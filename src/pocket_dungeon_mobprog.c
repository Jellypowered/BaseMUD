/***************************************************************************
 * Pocket Dungeon mobprog generation and attachment.
 ***************************************************************************/

#include "pocket_dungeon.h"

#include "chars.h"
#include "globals.h"
#include "mob_prog.h"
#include "mobiles.h"
#include "memory.h"
#include "recycle.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pd_text_buffer {
    char *data;
    size_t length;
    size_t capacity;
} PD_TEXT_BUFFER_T;

static bool pd_text_buffer_init(PD_TEXT_BUFFER_T *buffer)
{
    if (buffer == NULL)
        return FALSE;

    buffer->capacity = 512;
    buffer->length = 0;
    buffer->data = malloc(buffer->capacity);
    if (buffer->data == NULL)
        return FALSE;

    buffer->data[0] = '\0';
    return TRUE;
}

static void pd_text_buffer_free(PD_TEXT_BUFFER_T *buffer)
{
    if (buffer == NULL)
        return;

    free(buffer->data);
    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}

static bool pd_text_buffer_reserve(PD_TEXT_BUFFER_T *buffer, size_t extra)
{
    size_t needed;
    char *new_data;

    if (buffer == NULL || buffer->data == NULL)
        return FALSE;

    needed = buffer->length + extra + 1;
    if (needed <= buffer->capacity)
        return TRUE;

    while (buffer->capacity < needed)
        buffer->capacity *= 2;

    new_data = realloc(buffer->data, buffer->capacity);
    if (new_data == NULL)
        return FALSE;

    buffer->data = new_data;
    return TRUE;
}

static bool pd_text_buffer_appendf(PD_TEXT_BUFFER_T *buffer, const char *fmt, ...)
{
    char temp[MAX_STRING_LENGTH];
    va_list args;
    int written;

    if (buffer == NULL || fmt == NULL)
        return FALSE;

    va_start(args, fmt);
    written = vsnprintf(temp, sizeof(temp), fmt, args);
    va_end(args);

    if (written < 0 || (size_t)written >= sizeof(temp))
        return FALSE;

    if (!pd_text_buffer_reserve(buffer, (size_t)written))
        return FALSE;

    memcpy(buffer->data + buffer->length, temp, (size_t)written);
    buffer->length += (size_t)written;
    buffer->data[buffer->length] = '\0';
    return TRUE;
}

static char *pd_text_buffer_take(PD_TEXT_BUFFER_T *buffer)
{
    char *result;

    if (buffer == NULL)
        return NULL;

    result = buffer->data;
    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
    return result;
}

static int pd_level_tier(int level)
{
    if (level <= 10)
        return 0;
    if (level <= 25)
        return 1;
    if (level <= 40)
        return 2;
    return 3;
}

static void pd_free_generated_text(char **text)
{
    if (text == NULL || *text == NULL)
        return;

    free(*text);
    *text = NULL;
}

static const char *pd_personality_label(int personality)
{
    switch (personality) {
    case PD_MOBPROG_BRUTE:     return "brute";
    case PD_MOBPROG_TACTICIAN: return "tactician";
    case PD_MOBPROG_GUARDIAN:  return "guardian";
    case PD_MOBPROG_COWARD:    return "coward";
    case PD_MOBPROG_SUMMONER:  return "summoner";
    case PD_MOBPROG_BERSERKER: return "berserker";
    case PD_MOBPROG_ASSASSIN:  return "assassin";
    default:                   return "mob";
    }
}

static const char *pd_pick_attack_spell(int personality, int tier)
{
    switch (personality) {
    case PD_MOBPROG_TACTICIAN:
        switch (tier) {
        case 0: return "blindness";
        case 1: return "curse";
        case 2: return "lightning bolt";
        default: return "chain lightning";
        }
    case PD_MOBPROG_GUARDIAN:
        switch (tier) {
        case 0: return "shield";
        case 1: return "sanctuary";
        case 2: return "heal";
        default: return "greater heal";
        }
    case PD_MOBPROG_SUMMONER:
        switch (tier) {
        case 0: return "curse";
        case 1: return "blindness";
        case 2: return "lightning bolt";
        default: return "energy drain";
        }
    case PD_MOBPROG_BERSERKER:
        switch (tier) {
        case 0: return "weaken";
        case 1: return "poison";
        case 2: return "fireball";
        default: return "acid blast";
        }
    case PD_MOBPROG_ASSASSIN:
        switch (tier) {
        case 0: return "poison";
        case 1: return "blindness";
        case 2: return "curse";
        default: return "energy drain";
        }
    default:
        return NULL;
    }
}

static const char *pd_pick_support_spell(int personality, int tier)
{
    switch (personality) {
    case PD_MOBPROG_TACTICIAN:
        switch (tier) {
        case 0: return "armor";
        case 1: return "heal";
        case 2: return "greater heal";
        default: return "sanctuary";
        }
    case PD_MOBPROG_GUARDIAN:
        switch (tier) {
        case 0: return "armor";
        case 1: return "shield";
        case 2: return "sanctuary";
        default: return "greater heal";
        }
    case PD_MOBPROG_SUMMONER:
        switch (tier) {
        case 0: return "armor";
        case 1: return "shield";
        case 2: return "sanctuary";
        default: return "heal";
        }
    case PD_MOBPROG_ASSASSIN:
        switch (tier) {
        case 0: return "blindness";
        case 1: return "curse";
        case 2: return "slow";
        default: return "dispel magic";
        }
    default:
        return NULL;
    }
}

static int pd_base_fight_chance(int personality, int tier)
{
    switch (personality) {
    case PD_MOBPROG_BRUTE:
        return 30 + tier * 10;
    case PD_MOBPROG_TACTICIAN:
        return 20 + tier * 8;
    case PD_MOBPROG_GUARDIAN:
        return 18 + tier * 7;
    case PD_MOBPROG_COWARD:
        return 12 + tier * 4;
    case PD_MOBPROG_SUMMONER:
        return 25 + tier * 8;
    case PD_MOBPROG_BERSERKER:
        return 28 + tier * 9;
    case PD_MOBPROG_ASSASSIN:
        return 24 + tier * 8;
    default:
        return 20;
    }
}

static int pd_base_random_chance(int personality, int tier)
{
    switch (personality) {
    case PD_MOBPROG_BRUTE:
        return 10 + tier * 3;
    case PD_MOBPROG_TACTICIAN:
        return 15 + tier * 4;
    case PD_MOBPROG_GUARDIAN:
        return 12 + tier * 3;
    case PD_MOBPROG_COWARD:
        return 8 + tier * 2;
    case PD_MOBPROG_SUMMONER:
        return 18 + tier * 4;
    case PD_MOBPROG_BERSERKER:
        return 14 + tier * 4;
    case PD_MOBPROG_ASSASSIN:
        return 16 + tier * 4;
    default:
        return 10;
    }
}

static int pd_base_entry_chance(int personality, int tier)
{
    switch (personality) {
    case PD_MOBPROG_BRUTE:
        return 20 + tier * 5;
    case PD_MOBPROG_TACTICIAN:
        return 15 + tier * 5;
    case PD_MOBPROG_GUARDIAN:
        return 25 + tier * 5;
    case PD_MOBPROG_COWARD:
        return 12 + tier * 4;
    case PD_MOBPROG_SUMMONER:
        return 18 + tier * 5;
    case PD_MOBPROG_BERSERKER:
        return 18 + tier * 5;
    case PD_MOBPROG_ASSASSIN:
        return 15 + tier * 5;
    default:
        return 15;
    }
}

static int pd_base_greet_chance(int personality, int tier)
{
    int chance = pd_base_entry_chance(personality, tier) - 5;
    return UMAX(5, chance);
}

static int pd_base_hp_threshold(int personality, int tier)
{
    switch (personality) {
    case PD_MOBPROG_BRUTE:
        return (int[]){ 50, 35, 25, 15 }[tier];
    case PD_MOBPROG_TACTICIAN:
        return (int[]){ 40, 32, 24, 18 }[tier];
    case PD_MOBPROG_GUARDIAN:
        return (int[]){ 45, 35, 30, 25 }[tier];
    case PD_MOBPROG_COWARD:
        return (int[]){ 70, 60, 50, 40 }[tier];
    case PD_MOBPROG_SUMMONER:
        return (int[]){ 45, 38, 30, 24 }[tier];
    case PD_MOBPROG_BERSERKER:
        return (int[]){ 35, 30, 20, 15 }[tier];
    case PD_MOBPROG_ASSASSIN:
        return (int[]){ 40, 32, 24, 18 }[tier];
    default:
        return 30;
    }
}

static int pd_base_delay(int personality, int tier)
{
    switch (personality) {
    case PD_MOBPROG_TACTICIAN:
        return 2 + tier;
    case PD_MOBPROG_SUMMONER:
        return 2 + (tier / 2);
    case PD_MOBPROG_ASSASSIN:
        return 1 + (tier / 2);
    default:
        return 0;
    }
}

static int pd_summon_vnum(const PD_SEED_T *seed, int fallback_vnum)
{
    int i;

    if (seed == NULL || seed->mob_vnum_count <= 0)
        return fallback_vnum;

    for (i = 0; i < seed->mob_vnum_count; i++) {
        if (seed->mob_vnums[i] != fallback_vnum)
            return seed->mob_vnums[i];
    }

    return seed->mob_vnums[0];
}

static char *pd_build_entry_behavior(CHAR_T *mob, int personality, int difficulty)
{
    PD_TEXT_BUFFER_T buffer;
    int tier = pd_level_tier(difficulty);
    const char *label = pd_personality_label(personality);

    (void)mob;

    if (!pd_text_buffer_init(&buffer))
        return NULL;

    if (!pd_text_buffer_appendf(&buffer, "* Pocket dungeon entry behavior (%s)\n", label))
        goto fail;

    if (!pd_text_buffer_appendf(&buffer, "if ispc $n\n"))
        goto fail;

    switch (personality) {
    case PD_MOBPROG_BRUTE:
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The brute sizes you up and cracks its knuckles.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  say I will break you.\n"))
            goto fail;
        break;
    case PD_MOBPROG_TACTICIAN:
        if (!pd_text_buffer_appendf(&buffer, "  say An intruder. Let us assess the odds.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The tactician watches your stance closely.\n"))
            goto fail;
        break;
    case PD_MOBPROG_GUARDIAN:
        if (!pd_text_buffer_appendf(&buffer, "  say Stay back. This place is under my protection.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The guardian squares its shoulders.\n"))
            goto fail;
        break;
    case PD_MOBPROG_COWARD:
        if (!pd_text_buffer_appendf(&buffer, "  say Oh no... not again.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The coward starts looking for an exit.\n"))
            goto fail;
        break;
    case PD_MOBPROG_SUMMONER:
        if (!pd_text_buffer_appendf(&buffer, "  say You have stepped into my domain.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The summoner traces a pattern in the air.\n"))
            goto fail;
        break;
    case PD_MOBPROG_BERSERKER:
        if (!pd_text_buffer_appendf(&buffer, "  say Blood in the halls. Good.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The berserker twitches with barely contained rage.\n"))
            goto fail;
        break;
    case PD_MOBPROG_ASSASSIN:
        if (!pd_text_buffer_appendf(&buffer, "  say You should have stayed hidden.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The assassin studies your blind spots.\n"))
            goto fail;
        break;
    }

    if (!pd_text_buffer_appendf(&buffer, "endif\n"))
        goto fail;

    if (tier >= 2) {
        if (!pd_text_buffer_appendf(&buffer, "if rand %d\n", UMAX(5, pd_base_greet_chance(personality, tier) / 2)))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The %s keeps a careful watch on the room.\n", label))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "endif\n"))
            goto fail;
    }

    return pd_text_buffer_take(&buffer);

fail:
    pd_text_buffer_free(&buffer);
    return NULL;
}

static char *pd_build_greet_behavior(CHAR_T *mob, int personality, int difficulty)
{
    PD_TEXT_BUFFER_T buffer;
    int tier = pd_level_tier(difficulty);

    (void)mob;

    if (!pd_text_buffer_init(&buffer))
        return NULL;

    if (!pd_text_buffer_appendf(&buffer, "* Pocket dungeon greet behavior\n"))
        goto fail;
    if (!pd_text_buffer_appendf(&buffer, "if ispc $n\n"))
        goto fail;

    switch (personality) {
    case PD_MOBPROG_BRUTE:
        if (!pd_text_buffer_appendf(&buffer, "  say Prepare to fight, worm.\n"))
            goto fail;
        break;
    case PD_MOBPROG_TACTICIAN:
        if (!pd_text_buffer_appendf(&buffer, "  say Interesting. Let us see your method.\n"))
            goto fail;
        break;
    case PD_MOBPROG_GUARDIAN:
        if (!pd_text_buffer_appendf(&buffer, "  say Turn around. You are not welcome here.\n"))
            goto fail;
        break;
    case PD_MOBPROG_COWARD:
        if (!pd_text_buffer_appendf(&buffer, "  say Please, keep moving.\n"))
            goto fail;
        break;
    case PD_MOBPROG_SUMMONER:
        if (!pd_text_buffer_appendf(&buffer, "  say The dead hear my call.\n"))
            goto fail;
        break;
    case PD_MOBPROG_BERSERKER:
        if (!pd_text_buffer_appendf(&buffer, "  say Good. I was hoping for a challenge.\n"))
            goto fail;
        break;
    case PD_MOBPROG_ASSASSIN:
        if (!pd_text_buffer_appendf(&buffer, "  say One step closer to the grave.\n"))
            goto fail;
        break;
    }

    if (!pd_text_buffer_appendf(&buffer, "endif\n"))
        goto fail;

    if (tier >= 1) {
        if (!pd_text_buffer_appendf(&buffer, "if rand %d\n", UMAX(5, pd_base_greet_chance(personality, tier))))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The %s quietly sizes up the intruder.\n", pd_personality_label(personality)))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "endif\n"))
            goto fail;
    }

    return pd_text_buffer_take(&buffer);

fail:
    pd_text_buffer_free(&buffer);
    return NULL;
}

static char *pd_build_fight_behavior(CHAR_T *mob, int personality, int difficulty, const PD_SEED_T *seed, int source_vnum)
{
    PD_TEXT_BUFFER_T buffer;
    int tier = pd_level_tier(difficulty);
    const char *attack_spell = pd_pick_attack_spell(personality, tier);
    const char *support_spell = pd_pick_support_spell(personality, tier);
    int fight_chance = UMIN(100, pd_base_fight_chance(personality, tier));
    int damage = UMAX(4, (difficulty / 2) + 6);

    (void)mob;

    if (!pd_text_buffer_init(&buffer))
        return NULL;

    if (!pd_text_buffer_appendf(&buffer, "* Pocket dungeon fight behavior\n"))
        goto fail;

    if (personality == PD_MOBPROG_COWARD) {
        if (!pd_text_buffer_appendf(&buffer, "if ispc $n\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  say No, no, keep back!\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "endif\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "mob flee\n"))
            goto fail;
        return pd_text_buffer_take(&buffer);
    }

    if (!pd_text_buffer_appendf(&buffer, "if hastarget $i\n"))
        goto fail;

    switch (personality) {
    case PD_MOBPROG_BRUTE:
        if (!pd_text_buffer_appendf(&buffer, "  say Crushing you now.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob kill $n\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob damage $q %d\n", damage))
            goto fail;
        break;
    case PD_MOBPROG_TACTICIAN:
        if (!pd_text_buffer_appendf(&buffer, "  say Your form is already failing.\n"))
            goto fail;
        if (attack_spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $q\n", attack_spell))
                goto fail;
        }
        if (!pd_text_buffer_appendf(&buffer, "  mob delay %d\n", UMAX(1, pd_base_delay(personality, tier))))
            goto fail;
        break;
    case PD_MOBPROG_GUARDIAN:
        if (!pd_text_buffer_appendf(&buffer, "  say I will not let you pass.\n"))
            goto fail;
        if (support_spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $i\n", support_spell))
                goto fail;
        }
        if (!pd_text_buffer_appendf(&buffer, "  mob assist $q\n"))
            goto fail;
        break;
    case PD_MOBPROG_SUMMONER:
        if (!pd_text_buffer_appendf(&buffer, "  say Rise and fight for me!\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob mload %d %d\n",
            pd_summon_vnum(seed, source_vnum),
                UMAX(1, 1 + tier / 2)))
            goto fail;
        if (attack_spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $q\n", attack_spell))
                goto fail;
        }
        break;
    case PD_MOBPROG_BERSERKER:
        if (!pd_text_buffer_appendf(&buffer, "  say RAAAAAAGH!\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob damage $q %d\n", damage + 5))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob kill $n\n"))
            goto fail;
        break;
    case PD_MOBPROG_ASSASSIN:
        if (!pd_text_buffer_appendf(&buffer, "  say You will not see me coming.\n"))
            goto fail;
        if (attack_spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $q\n", attack_spell))
                goto fail;
        }
        if (!pd_text_buffer_appendf(&buffer, "  mob damage $q %d\n", damage + 3))
            goto fail;
        break;
    }

    if (!pd_text_buffer_appendf(&buffer, "endif\n"))
        goto fail;

    if (!pd_text_buffer_appendf(&buffer, "if hpcnt $i < %d\n", pd_base_hp_threshold(personality, tier)))
        goto fail;

    switch (personality) {
    case PD_MOBPROG_BRUTE:
        if (!pd_text_buffer_appendf(&buffer, "  mob say You'll need more than that!\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob damage $q %d\n", damage + 4))
            goto fail;
        break;
    case PD_MOBPROG_TACTICIAN:
        if (!pd_text_buffer_appendf(&buffer, "  mob say Repositioning.\n"))
            goto fail;
        if (support_spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $i\n", support_spell))
                goto fail;
        }
        break;
    case PD_MOBPROG_GUARDIAN:
        if (!pd_text_buffer_appendf(&buffer, "  mob say I will hold this line.\n"))
            goto fail;
        if (support_spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $i\n", support_spell))
                goto fail;
        }
        break;
    case PD_MOBPROG_SUMMONER:
        if (!pd_text_buffer_appendf(&buffer, "  mob say More allies!\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob mload %d %d\n",
            pd_summon_vnum(seed, source_vnum),
                UMAX(1, 2 + tier / 2)))
            goto fail;
        if (support_spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $i\n", support_spell))
                goto fail;
        }
        break;
    case PD_MOBPROG_BERSERKER:
        if (!pd_text_buffer_appendf(&buffer, "  mob say Your blood is running thin.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob damage $q %d\n", damage + 8))
            goto fail;
        break;
    case PD_MOBPROG_ASSASSIN:
        if (!pd_text_buffer_appendf(&buffer, "  mob say You are already dead.\n"))
            goto fail;
        if (support_spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $q\n", support_spell))
                goto fail;
        }
        if (!pd_text_buffer_appendf(&buffer, "  mob flee\n"))
            goto fail;
        break;
    case PD_MOBPROG_COWARD:
        if (!pd_text_buffer_appendf(&buffer, "  mob flee\n"))
            goto fail;
        break;
    }

    if (!pd_text_buffer_appendf(&buffer, "endif\n"))
        goto fail;

    if (!pd_text_buffer_appendf(&buffer, "if rand %d\n", fight_chance))
        goto fail;
    if (!pd_text_buffer_appendf(&buffer, "  if hastarget $i\n"))
        goto fail;

    switch (personality) {
    case PD_MOBPROG_BRUTE:
        if (!pd_text_buffer_appendf(&buffer, "    mob damage $q %d\n", UMAX(4, damage / 2)))
            goto fail;
        break;
    case PD_MOBPROG_TACTICIAN:
        if (attack_spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "    mob cast %s $q\n", attack_spell))
                goto fail;
        }
        break;
    case PD_MOBPROG_GUARDIAN:
        if (support_spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "    mob cast %s $i\n", support_spell))
                goto fail;
        }
        break;
    case PD_MOBPROG_SUMMONER:
        if (attack_spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "    mob cast %s $q\n", attack_spell))
                goto fail;
        }
        break;
    case PD_MOBPROG_BERSERKER:
        if (!pd_text_buffer_appendf(&buffer, "    mob damage $q %d\n", damage))
            goto fail;
        break;
    case PD_MOBPROG_ASSASSIN:
        if (attack_spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "    mob cast %s $q\n", attack_spell))
                goto fail;
        }
        break;
    case PD_MOBPROG_COWARD:
        if (!pd_text_buffer_appendf(&buffer, "    mob flee\n"))
            goto fail;
        break;
    }

    if (!pd_text_buffer_appendf(&buffer, "  endif\n"))
        goto fail;
    if (!pd_text_buffer_appendf(&buffer, "endif\n"))
        goto fail;

    return pd_text_buffer_take(&buffer);

fail:
    pd_text_buffer_free(&buffer);
    return NULL;
}

static char *pd_build_random_behavior(CHAR_T *mob, int personality, int difficulty)
{
    PD_TEXT_BUFFER_T buffer;
    int tier = pd_level_tier(difficulty);
    const char *spell = pd_pick_support_spell(personality, tier);
    int chance = UMIN(80, pd_base_random_chance(personality, tier));
    int damage = UMAX(3, difficulty / 3 + 4);

    (void)mob;

    if (!pd_text_buffer_init(&buffer))
        return NULL;

    if (!pd_text_buffer_appendf(&buffer, "* Pocket dungeon random behavior\n"))
        goto fail;

    if (!pd_text_buffer_appendf(&buffer, "if rand %d\n", chance))
        goto fail;

    switch (personality) {
    case PD_MOBPROG_BRUTE:
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The brute stamps the floor and roars.\n"))
            goto fail;
        break;
    case PD_MOBPROG_TACTICIAN:
        if (!pd_text_buffer_appendf(&buffer, "  if hastarget $i\n"))
            goto fail;
        if (spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "    mob cast %s $q\n", spell))
                goto fail;
        }
        if (!pd_text_buffer_appendf(&buffer, "  else\n"))
            goto fail;
        if (spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "    mob cast %s $i\n", spell))
                goto fail;
        }
        if (!pd_text_buffer_appendf(&buffer, "  endif\n"))
            goto fail;
        break;
    case PD_MOBPROG_GUARDIAN:
        if (spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $i\n", spell))
                goto fail;
        }
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The guardian reinforces its warding.\n"))
            goto fail;
        break;
    case PD_MOBPROG_COWARD:
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The coward keeps glancing for escape routes.\n"))
            goto fail;
        break;
    case PD_MOBPROG_SUMMONER:
        if (!pd_text_buffer_appendf(&buffer, "  if hastarget $i\n"))
            goto fail;
        if (spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "    mob cast %s $q\n", spell))
                goto fail;
        }
        if (!pd_text_buffer_appendf(&buffer, "  endif\n"))
            goto fail;
        break;
    case PD_MOBPROG_BERSERKER:
        if (!pd_text_buffer_appendf(&buffer, "  if hastarget $i\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "    mob damage $q %d\n", damage))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  endif\n"))
            goto fail;
        break;
    case PD_MOBPROG_ASSASSIN:
        if (!pd_text_buffer_appendf(&buffer, "  if hastarget $i\n"))
            goto fail;
        if (spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "    mob cast %s $q\n", spell))
                goto fail;
        }
        if (!pd_text_buffer_appendf(&buffer, "  endif\n"))
            goto fail;
        break;
    }

    if (!pd_text_buffer_appendf(&buffer, "endif\n"))
        goto fail;

    return pd_text_buffer_take(&buffer);

fail:
    pd_text_buffer_free(&buffer);
    return NULL;
}

static char *pd_build_hp_behavior(CHAR_T *mob, int personality, int difficulty, const PD_SEED_T *seed, int source_vnum)
{
    PD_TEXT_BUFFER_T buffer;
    int tier = pd_level_tier(difficulty);
    const char *spell = pd_pick_support_spell(personality, tier);
    int threshold = pd_base_hp_threshold(personality, tier);
    int summon_vnum = pd_summon_vnum(seed, source_vnum);

    (void)mob;

    if (!pd_text_buffer_init(&buffer))
        return NULL;

    if (!pd_text_buffer_appendf(&buffer, "* Pocket dungeon low-health behavior\n"))
        goto fail;

    switch (personality) {
    case PD_MOBPROG_BRUTE:
        if (!pd_text_buffer_appendf(&buffer, "mob say I am not finished yet.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "mob damage $q %d\n", UMAX(4, difficulty / 2 + 4)))
            goto fail;
        break;
    case PD_MOBPROG_TACTICIAN:
        if (!pd_text_buffer_appendf(&buffer, "if hastarget $i\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob say Repositioning to a better angle.\n"))
            goto fail;
        if (spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $i\n", spell))
                goto fail;
        }
        if (!pd_text_buffer_appendf(&buffer, "endif\n"))
            goto fail;
        break;
    case PD_MOBPROG_GUARDIAN:
        if (!pd_text_buffer_appendf(&buffer, "mob say I will endure.\n"))
            goto fail;
        if (spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "mob cast %s $i\n", spell))
                goto fail;
        }
        break;
    case PD_MOBPROG_COWARD:
        if (!pd_text_buffer_appendf(&buffer, "mob say Mercy!\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "mob flee\n"))
            goto fail;
        break;
    case PD_MOBPROG_SUMMONER:
        if (!pd_text_buffer_appendf(&buffer, "mob say My servants, aid me!\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "mob mload %d %d\n", summon_vnum, UMAX(1, 2 + tier / 2)))
            goto fail;
        if (spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "mob cast %s $i\n", spell))
                goto fail;
        }
        break;
    case PD_MOBPROG_BERSERKER:
        if (!pd_text_buffer_appendf(&buffer, "mob say You cannot stop this fury.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "mob damage $q %d\n", UMAX(4, difficulty / 2 + 8)))
            goto fail;
        break;
    case PD_MOBPROG_ASSASSIN:
        if (!pd_text_buffer_appendf(&buffer, "if hastarget $i\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob say The shadows still own me.\n"))
            goto fail;
        if (spell != NULL) {
            if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $q\n", spell))
                goto fail;
        }
        if (!pd_text_buffer_appendf(&buffer, "endif\n"))
            goto fail;
        break;
    }

    if (personality == PD_MOBPROG_COWARD) {
        if (!pd_text_buffer_appendf(&buffer, "if rand %d\n", UMIN(45, UMAX(10, threshold / 2))))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob flee\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "endif\n"))
            goto fail;
    }

    return pd_text_buffer_take(&buffer);

fail:
    pd_text_buffer_free(&buffer);
    return NULL;
}

static char *pd_build_kill_behavior(CHAR_T *mob, int personality, int difficulty)
{
    PD_TEXT_BUFFER_T buffer;
    int tier = pd_level_tier(difficulty);

    (void)mob;

    if (!pd_text_buffer_init(&buffer))
        return NULL;

    if (!pd_text_buffer_appendf(&buffer, "* Pocket dungeon kill behavior\n"))
        goto fail;

    if (!pd_text_buffer_appendf(&buffer, "if ispc $n\n"))
        goto fail;

    switch (personality) {
    case PD_MOBPROG_BRUTE:
        if (!pd_text_buffer_appendf(&buffer, "  say Another weakling falls.\n"))
            goto fail;
        break;
    case PD_MOBPROG_TACTICIAN:
        if (!pd_text_buffer_appendf(&buffer, "  say Your pattern was predictable.\n"))
            goto fail;
        break;
    case PD_MOBPROG_GUARDIAN:
        if (!pd_text_buffer_appendf(&buffer, "  say The line remains unbroken.\n"))
            goto fail;
        break;
    case PD_MOBPROG_COWARD:
        if (!pd_text_buffer_appendf(&buffer, "  say Stay down. Please.\n"))
            goto fail;
        break;
    case PD_MOBPROG_SUMMONER:
        if (!pd_text_buffer_appendf(&buffer, "  say My servants will applaud this.\n"))
            goto fail;
        break;
    case PD_MOBPROG_BERSERKER:
        if (!pd_text_buffer_appendf(&buffer, "  say Yes! More blood!\n"))
            goto fail;
        break;
    case PD_MOBPROG_ASSASSIN:
        if (!pd_text_buffer_appendf(&buffer, "  say Too slow.\n"))
            goto fail;
        break;
    }

    if (!pd_text_buffer_appendf(&buffer, "endif\n"))
        goto fail;

    if (tier >= 2) {
        if (!pd_text_buffer_appendf(&buffer, "if rand %d\n", UMAX(5, pd_base_random_chance(personality, tier))))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "  mob echo The %s savors the victory.\n", pd_personality_label(personality)))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "endif\n"))
            goto fail;
    }

    return pd_text_buffer_take(&buffer);

fail:
    pd_text_buffer_free(&buffer);
    return NULL;
}

static char *pd_build_speech_behavior(CHAR_T *mob, int personality)
{
    PD_TEXT_BUFFER_T buffer;

    (void)mob;

    if (!pd_text_buffer_init(&buffer))
        return NULL;

    if (!pd_text_buffer_appendf(&buffer, "* Pocket dungeon speech behavior\n"))
        goto fail;
    if (!pd_text_buffer_appendf(&buffer, "if ispc $n\n"))
        goto fail;

    switch (personality) {
    case PD_MOBPROG_BRUTE:
        if (!pd_text_buffer_appendf(&buffer, "  say Fight me properly, coward.\n"))
            goto fail;
        break;
    case PD_MOBPROG_TACTICIAN:
        if (!pd_text_buffer_appendf(&buffer, "  say Strategy is wasted on the doomed.\n"))
            goto fail;
        break;
    case PD_MOBPROG_GUARDIAN:
        if (!pd_text_buffer_appendf(&buffer, "  say The sentinel does not answer to words.\n"))
            goto fail;
        break;
    case PD_MOBPROG_COWARD:
        if (!pd_text_buffer_appendf(&buffer, "  say Please leave me alone.\n"))
            goto fail;
        break;
    case PD_MOBPROG_SUMMONER:
        if (!pd_text_buffer_appendf(&buffer, "  say Your voice will feed the dead.\n"))
            goto fail;
        break;
    case PD_MOBPROG_BERSERKER:
        if (!pd_text_buffer_appendf(&buffer, "  say Blood. Give me blood.\n"))
            goto fail;
        break;
    case PD_MOBPROG_ASSASSIN:
        if (!pd_text_buffer_appendf(&buffer, "  say Whisper softer.\n"))
            goto fail;
        break;
    }

    if (!pd_text_buffer_appendf(&buffer, "endif\n"))
        goto fail;

    return pd_text_buffer_take(&buffer);

fail:
    pd_text_buffer_free(&buffer);
    return NULL;
}

static char *pd_build_surrender_behavior(CHAR_T *mob, int personality)
{
    PD_TEXT_BUFFER_T buffer;

    (void)mob;

    if (!pd_text_buffer_init(&buffer))
        return NULL;

    if (!pd_text_buffer_appendf(&buffer, "* Pocket dungeon surrender behavior\n"))
        goto fail;

    switch (personality) {
    case PD_MOBPROG_COWARD:
        if (!pd_text_buffer_appendf(&buffer, "say Mercy, and maybe I will run.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "mob flee\n"))
            goto fail;
        break;
    case PD_MOBPROG_ASSASSIN:
        if (!pd_text_buffer_appendf(&buffer, "say You found only a shadow.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "mob flee\n"))
            goto fail;
        break;
    case PD_MOBPROG_GUARDIAN:
        if (!pd_text_buffer_appendf(&buffer, "say I stand my post until the end.\n"))
            goto fail;
        break;
    default:
        if (!pd_text_buffer_appendf(&buffer, "say No surrender.\n"))
            goto fail;
        break;
    }

    return pd_text_buffer_take(&buffer);

fail:
    pd_text_buffer_free(&buffer);
    return NULL;
}

static char *pd_build_death_behavior(CHAR_T *mob, int personality, int difficulty, const PD_SEED_T *seed, int source_vnum)
{
    PD_TEXT_BUFFER_T buffer;
    int tier = pd_level_tier(difficulty);
    int summon_vnum = pd_summon_vnum(seed, source_vnum);

    (void)mob;

    if (!pd_text_buffer_init(&buffer))
        return NULL;

    if (!pd_text_buffer_appendf(&buffer, "* Pocket dungeon death behavior\n"))
        goto fail;

    switch (personality) {
    case PD_MOBPROG_BRUTE:
        if (!pd_text_buffer_appendf(&buffer, "mob echo The brute crashes to the stone floor.\n"))
            goto fail;
        break;
    case PD_MOBPROG_TACTICIAN:
        if (!pd_text_buffer_appendf(&buffer, "say This calculation... was incomplete...\n"))
            goto fail;
        break;
    case PD_MOBPROG_GUARDIAN:
        if (!pd_text_buffer_appendf(&buffer, "mob echo The guardian's watch finally ends.\n"))
            goto fail;
        break;
    case PD_MOBPROG_COWARD:
        if (!pd_text_buffer_appendf(&buffer, "say I never wanted this...\n"))
            goto fail;
        break;
    case PD_MOBPROG_SUMMONER:
        if (!pd_text_buffer_appendf(&buffer, "mob echo The summoner's fingers twitch one last time.\n"))
            goto fail;
        if (!pd_text_buffer_appendf(&buffer, "mob mload %d %d\n", summon_vnum, UMAX(1, 1 + tier / 2)))
            goto fail;
        break;
    case PD_MOBPROG_BERSERKER:
        if (!pd_text_buffer_appendf(&buffer, "mob echo The berserker falls with a final, furious roar.\n"))
            goto fail;
        break;
    case PD_MOBPROG_ASSASSIN:
        if (!pd_text_buffer_appendf(&buffer, "say The shadows... remember...\n"))
            goto fail;
        break;
    }

    return pd_text_buffer_take(&buffer);

fail:
    pd_text_buffer_free(&buffer);
    return NULL;
}

static MOB_INDEX_T *pd_clone_mob_index_for_instance(const MOB_INDEX_T *source, AREA_T *area)
{
    MOB_INDEX_T *clone;

    if (source == NULL || area == NULL)
        return NULL;

    clone = mob_index_new();
    if (clone == NULL)
        return NULL;

    mob_index_to_area(clone, area);

    clone->spec_fun = source->spec_fun;
    clone->shop = NULL;
    clone->vnum = source->vnum;
    clone->anum = source->anum;
    clone->group = source->group;
    clone->new_format = source->new_format;
    clone->mob_count = 0;
    clone->killed = 0;

    str_replace_dup(&clone->area_str, source->area_str ? source->area_str : "");
    str_replace_dup(&clone->name, source->name ? source->name : "no name");
    str_replace_dup(&clone->short_descr, source->short_descr ? source->short_descr : "(no short description)");
    str_replace_dup(&clone->long_descr, source->long_descr ? source->long_descr : "(no long description)\n\r");
    str_replace_dup(&clone->description, source->description ? source->description : "");

    clone->alignment = source->alignment;
    clone->level = source->level;
    clone->hitroll = source->hitroll;
    clone->hit = source->hit;
    clone->mana = source->mana;
    clone->damage = source->damage;
    clone->ac[0] = source->ac[0];
    clone->ac[1] = source->ac[1];
    clone->ac[2] = source->ac[2];
    clone->ac[3] = source->ac[3];
    clone->attack_type = source->attack_type;
    clone->start_pos = source->start_pos;
    clone->default_pos = source->default_pos;
    clone->sex = source->sex;
    clone->race = source->race;
    clone->wealth = source->wealth;
    clone->size = source->size;
    clone->material = source->material;
    clone->mprog_flags = 0;

    clone->ext_mob_plus = source->ext_mob_plus;
    clone->ext_mob_final = source->ext_mob_final;
    clone->ext_mob_minus = source->ext_mob_minus;
    clone->affected_by_plus = source->affected_by_plus;
    clone->affected_by_final = source->affected_by_final;
    clone->affected_by_minus = source->affected_by_minus;
    clone->off_flags_plus = source->off_flags_plus;
    clone->off_flags_final = source->off_flags_final;
    clone->off_flags_minus = source->off_flags_minus;
    clone->imm_flags_plus = source->imm_flags_plus;
    clone->imm_flags_final = source->imm_flags_final;
    clone->imm_flags_minus = source->imm_flags_minus;
    clone->res_flags_plus = source->res_flags_plus;
    clone->res_flags_final = source->res_flags_final;
    clone->res_flags_minus = source->res_flags_minus;
    clone->vuln_flags_plus = source->vuln_flags_plus;
    clone->vuln_flags_final = source->vuln_flags_final;
    clone->vuln_flags_minus = source->vuln_flags_minus;
    clone->form_plus = source->form_plus;
    clone->form_final = source->form_final;
    clone->form_minus = source->form_minus;
    clone->parts_plus = source->parts_plus;
    clone->parts_final = source->parts_final;
    clone->parts_minus = source->parts_minus;

    return clone;
}

static bool pd_attach_mprog(MOB_INDEX_T *mob_index, int trig_type, int chance,
                            const char *code, const char *phrase, int vnum)
{
    MPROG_LIST_T *mprog;
    char phrase_buf[32];

    if (mob_index == NULL || code == NULL || code[0] == '\0')
        return TRUE;

    mprog = mprog_new();
    if (mprog == NULL)
        return FALSE;

    mprog_to_area(mprog, mob_index->area);
    mprog->trig_type = trig_type;
    mprog->vnum = vnum;
    mprog->anum = 0;

    if (phrase != NULL)
        str_replace_dup(&mprog->trig_phrase, phrase);
    else {
        snprintf(phrase_buf, sizeof(phrase_buf), "%d", chance);
        str_replace_dup(&mprog->trig_phrase, phrase_buf);
    }

    str_replace_dup(&mprog->code, code);
    LIST2_BACK(mprog, mob_prev, mob_next, mob_index->mprog_first, mob_index->mprog_last);
    SET_BIT(mob_index->mprog_flags, trig_type);
    return TRUE;
}

static bool pd_attach_generated_scripts(MOB_INDEX_T *mob_index,
                                        const PD_SEED_T *seed,
                                        int personality,
                                        int difficulty,
                                        int source_vnum)
{
    char *entry = NULL;
    char *greet = NULL;
    char *fight = NULL;
    char *hp = NULL;
    char *random = NULL;
    char *kill = NULL;
    char *speech = NULL;
    char *surr = NULL;
    char *death = NULL;
    char *delay = NULL;
    bool ok = TRUE;
    int tier = pd_level_tier(difficulty);

    entry = pd_build_entry_behavior(NULL, personality, difficulty);
    greet = pd_build_greet_behavior(NULL, personality, difficulty);
    fight = pd_build_fight_behavior(NULL, personality, difficulty, seed, source_vnum);
    hp = pd_build_hp_behavior(NULL, personality, difficulty, seed, source_vnum);
    random = pd_build_random_behavior(NULL, personality, difficulty);
    kill = pd_build_kill_behavior(NULL, personality, difficulty);
    speech = pd_build_speech_behavior(NULL, personality);
    surr = pd_build_surrender_behavior(NULL, personality);
    death = pd_build_death_behavior(NULL, personality, difficulty, seed, source_vnum);

    if (personality == PD_MOBPROG_TACTICIAN || personality == PD_MOBPROG_ASSASSIN) {
        PD_TEXT_BUFFER_T buffer;

        if (pd_text_buffer_init(&buffer)) {
            if (pd_text_buffer_appendf(&buffer, "* Pocket dungeon delay behavior\n")) {
                if (!pd_text_buffer_appendf(&buffer, "if hastarget $i\n"))
                    ok = FALSE;
                else {
                    if (personality == PD_MOBPROG_TACTICIAN) {
                        if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $q\n",
                                pd_pick_attack_spell(personality, tier)))
                            ok = FALSE;
                    } else {
                        const char *spell = pd_pick_attack_spell(personality, tier);
                        if (spell != NULL) {
                            if (!pd_text_buffer_appendf(&buffer, "  mob cast %s $q\n", spell))
                                ok = FALSE;
                        }
                    }
                    if (ok && !pd_text_buffer_appendf(&buffer, "endif\n"))
                        ok = FALSE;
                }
            } else {
                ok = FALSE;
            }

            if (ok)
                delay = pd_text_buffer_take(&buffer);
            else
                pd_text_buffer_free(&buffer);
        }
    }

    if (entry == NULL || greet == NULL || fight == NULL || hp == NULL ||
        random == NULL || kill == NULL || speech == NULL || surr == NULL ||
        death == NULL || ((personality == PD_MOBPROG_TACTICIAN ||
                           personality == PD_MOBPROG_ASSASSIN) && delay == NULL)) {
        ok = FALSE;
    }

    if (entry != NULL && !pd_attach_mprog(mob_index, TRIG_ENTRY, pd_base_entry_chance(personality, tier), entry, NULL, source_vnum))
        ok = FALSE;
    if (ok && greet != NULL && !pd_attach_mprog(mob_index, TRIG_GREET, pd_base_greet_chance(personality, tier), greet, NULL, source_vnum))
        ok = FALSE;
    if (ok && fight != NULL && !pd_attach_mprog(mob_index, TRIG_FIGHT, pd_base_fight_chance(personality, tier), fight, NULL, source_vnum))
        ok = FALSE;
    if (ok && hp != NULL && !pd_attach_mprog(mob_index, TRIG_HPCNT, pd_base_hp_threshold(personality, tier), hp, NULL, source_vnum))
        ok = FALSE;
    if (ok && random != NULL && !pd_attach_mprog(mob_index, TRIG_RANDOM, pd_base_random_chance(personality, tier), random, NULL, source_vnum))
        ok = FALSE;
    if (ok && kill != NULL && !pd_attach_mprog(mob_index, TRIG_KILL, 100, kill, NULL, source_vnum))
        ok = FALSE;
    if (ok && speech != NULL && !pd_attach_mprog(mob_index, TRIG_SPEECH, 100,
            speech,
            (personality == PD_MOBPROG_COWARD) ? "help" :
            (personality == PD_MOBPROG_GUARDIAN) ? "protect" :
            (personality == PD_MOBPROG_SUMMONER) ? "serve" :
            (personality == PD_MOBPROG_BERSERKER) ? "blood" :
            (personality == PD_MOBPROG_ASSASSIN) ? "silent" :
            (personality == PD_MOBPROG_TACTICIAN) ? "strategy" : "fight",
            source_vnum))
        ok = FALSE;
    if (ok && surr != NULL && !pd_attach_mprog(mob_index, TRIG_SURR, 100, surr, NULL, source_vnum))
        ok = FALSE;
    if (ok && death != NULL && !pd_attach_mprog(mob_index, TRIG_DEATH, 100, death, NULL, source_vnum))
        ok = FALSE;
    if (ok && delay != NULL && !pd_attach_mprog(mob_index, TRIG_DELAY, 100, delay, NULL, source_vnum))
        ok = FALSE;

    pd_free_generated_text(&entry);
    pd_free_generated_text(&greet);
    pd_free_generated_text(&fight);
    pd_free_generated_text(&hp);
    pd_free_generated_text(&random);
    pd_free_generated_text(&kill);
    pd_free_generated_text(&speech);
    pd_free_generated_text(&surr);
    pd_free_generated_text(&death);
    pd_free_generated_text(&delay);

    return ok;
}

char *pd_generate_passive_behavior(CHAR_T *mob, int difficulty)
{
    int personality;

    if (mob == NULL || mob->mob_index == NULL)
        return NULL;

    personality = pd_select_personality(mob->mob_index->vnum);
    return pd_build_entry_behavior(mob, personality, difficulty);
}

char *pd_generate_combat_ai(CHAR_T *mob, int personality, int difficulty)
{
    if (mob == NULL || mob->mob_index == NULL)
        return NULL;
    return pd_build_fight_behavior(mob, personality, difficulty, NULL, mob->mob_index->vnum);
}

char *pd_generate_personality_behavior(CHAR_T *mob, int personality)
{
    return pd_build_kill_behavior(mob, personality, 1);
}

char *pd_generate_mobprog_script(CHAR_T *mob, int personality, int difficulty)
{
    PD_TEXT_BUFFER_T buffer;
    char *entry;
    char *fight;
    char *personality_code;
    char *result;

    if (mob == NULL || mob->mob_index == NULL)
        return NULL;

    entry = pd_build_entry_behavior(mob, personality, difficulty);
    fight = pd_build_fight_behavior(mob, personality, difficulty, NULL, mob->mob_index->vnum);
    personality_code = pd_build_kill_behavior(mob, personality, difficulty);

    if (!pd_text_buffer_init(&buffer)) {
        pd_free_generated_text(&entry);
        pd_free_generated_text(&fight);
        pd_free_generated_text(&personality_code);
        return NULL;
    }

    if (entry != NULL)
        pd_text_buffer_appendf(&buffer, "%s\n", entry);
    if (fight != NULL)
        pd_text_buffer_appendf(&buffer, "%s\n", fight);
    if (personality_code != NULL)
        pd_text_buffer_appendf(&buffer, "%s\n", personality_code);

    result = pd_text_buffer_take(&buffer);

    pd_free_generated_text(&entry);
    pd_free_generated_text(&fight);
    pd_free_generated_text(&personality_code);

    return result;
}

const char *pd_describe_spell_for_personality(int personality, int level)
{
    return pd_pick_attack_spell(personality, pd_level_tier(level));
}

int pd_select_personality(int mob_vnum)
{
    unsigned int value;

    if (mob_vnum < 0)
        mob_vnum = -mob_vnum;

    value = (unsigned int)mob_vnum;
    value ^= value >> 7;
    value ^= value >> 13;
    value ^= value >> 17;
    return (int)(value % PD_MOBPROG_PERSONALITY_MAX);
}

void pd_generate_and_attach_mobprog(CHAR_T *mob, PD_INSTANCE_T *instance)
{
    const PD_SEED_T *seed;
    MOB_INDEX_T *source;
    MOB_INDEX_T *clone;
    int personality;
    int difficulty;

    if (mob == NULL || instance == NULL || mob->mob_index == NULL)
        return;

    seed = pd_seed_get_by_name(instance->theme);
    if (seed == NULL || !seed->mobprog_enabled)
        return;

    source = mob->mob_index;
    if (source->mprog_first != NULL || source->shop != NULL)
        return;

    difficulty = instance->level + seed->mobprog_difficulty_boost;
    if (difficulty < 1)
        difficulty = 1;
    if (difficulty > 50)
        difficulty = 50;

    if (seed->mobprog_personality_override >= 0 &&
        seed->mobprog_personality_override < PD_MOBPROG_PERSONALITY_MAX)
        personality = seed->mobprog_personality_override;
    else
        personality = pd_select_personality(source->vnum);

    clone = pd_clone_mob_index_for_instance(source, instance->area);
    if (clone == NULL)
        return;

    if (!pd_attach_generated_scripts(clone, seed, personality, difficulty, source->vnum)) {
        mob_index_free(clone);
        return;
    }

    mobile_to_mob_index(mob, clone);
}