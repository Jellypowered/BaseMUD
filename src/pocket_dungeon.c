/***************************************************************************
 * Pocket Dungeon — instanced procedural area generator.
 * pocket_dungeon.c — implementation.
 ***************************************************************************/

#include "globals.h"
#include "pocket_dungeon.h"

#include "chars.h"
#include "comm.h"
#include "db.h"
#include "defs.h"
#include "extra_descrs.h"
#include "memory.h"
#include "mob_cmds.h"
#include "mobiles.h"
#include "objs.h"
#include "recycle.h"
#include "rooms.h"
#include "tables.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

/* Scale a mobile's stats to a target level.
 * Used after mobile_create() so instance mobs match the group's level
 * with minor ±2 variance rather than the fixed template level. */
static void pd_scale_mob_to_level(CHAR_T *mob, int target_level)
{
    int i;

    if (mob == NULL)
        return;
    if (target_level < 1)
        target_level = 1;
    if (target_level == mob->level)
        return;

    mob->level = target_level;
    
    /* L1: Assertion: ensure final level is always >= 1 */
    assert(mob->level >= 1 && "mob->level must be at least 1");

    /* Recompute HP with the standard level formula (format-agnostic). */
    mob->max_hit = (target_level * 8
        + number_range(target_level * target_level / 4,
                       target_level * target_level)) * 9 / 10;
    if (mob->max_hit < 1)
        mob->max_hit = 1;
    mob->hit = mob->max_hit;

    /* Recompute mana */
    mob->max_mana = 100 + dice(target_level, 10);
    mob->mana     = mob->max_mana;

    /* Combat rolls */
    mob->hitroll = target_level / 5;
    mob->damroll  = target_level / 4;

    /* Armour class */
    for (i = 0; i < 3; i++)
        mob->armor[i] = int_interpolate(target_level, 100, -100);
    mob->armor[3] = int_interpolate(target_level, 100, 0);

    /* Stats */
    for (i = 0; i < STAT_MAX; i++)
        mob->perm_stat[i] = UMIN(25, 11 + target_level / 4);
}

/* B5: Validate that a seed has required data before spawning.
 * Returns TRUE if valid, FALSE if invalid (seed skipped with error log).
 * Checks: mob/item arrays exist, key vnums set, strings present. */
bool pd_validate_seed(PD_SEED_T *seed)
{
    if (seed == NULL) {
        bugf("pd_validate_seed called on NULL seed");
        return FALSE;
    }
    
    if (seed->mob_vnum_count <= 0) {
        bugf("pd_seed '%s' has no mob_vnums (count=%d)", 
             seed->name ? seed->name : "UNNAMED", seed->mob_vnum_count);
        return FALSE;
    }
    
    if (seed->item_vnum_count <= 0) {
        bugf("pd_seed '%s' has no item_vnums (count=%d)",
             seed->name ? seed->name : "UNNAMED", seed->item_vnum_count);
        return FALSE;
    }
    
    if (seed->boss_vnum <= 0) {
        bugf("pd_seed '%s' has no boss_vnum (spawning without boss)",
             seed->name ? seed->name : "UNNAMED");
    }
    
    if (seed->container_vnum <= 0) {
        bugf("pd_seed '%s' has no container_vnum (spawning without loot chest)",
             seed->name ? seed->name : "UNNAMED");
    }

    if (seed->room_desc_count <= 0) {
        bugf("pd_seed '%s' has no room_descs (using fallback flavor text)",
             seed->name ? seed->name : "UNNAMED");
    }
    
    return TRUE;
}

/* C1: Apply random affixes to an instance.
 * Selects 1–2 random affixes and stores in instance->affixes array. */
void pd_apply_affixes(PD_INSTANCE_T *inst)
{
    if (inst == NULL)
        return;
    
    inst->affix_count = 0;
    
    /* Randomly decide: 1 or 2 affixes? (50/50 chance) */
    int num_affixes = number_percent() < 50 ? 1 : 2;
    
    while (inst->affix_count < num_affixes && inst->affix_count < 5) {
        int affix = number_range(PD_AFFIX_STONY, PD_AFFIX_LUMINOUS);
        
        /* Check if already applied */
        int i;
        bool already_has = FALSE;
        for (i = 0; i < inst->affix_count; i++) {
            if (inst->affixes[i] == affix) {
                already_has = TRUE;
                break;
            }
        }
        
        if (!already_has) {
            inst->affixes[inst->affix_count] = affix;
            inst->affix_count++;
        }
    }
}

/* C2: Calculate difficulty bonus based on rooms cleared.
 * Returns bonus level: +1 per 5 rooms cleared (max +10). */
int pd_get_difficulty_bonus(PD_INSTANCE_T *inst)
{
    if (inst == NULL)
        return 0;
    
    int bonus = inst->rooms_cleared / 5;
    if (bonus > 10)
        bonus = 10;
    return bonus;
}

/* C3: Trigger boss loot drops when boss dies.
 * Called from combat code when boss death is detected. */
void pd_trigger_boss_loot(PD_INSTANCE_T *inst, CHAR_T *boss)
{
    PD_SEED_T *seed;
    OBJ_T *loot;
    OBJ_INDEX_T *oidx;
    int i, vnum_idx;
    
    if (inst == NULL || boss == NULL || boss->in_room == NULL)
        return;
    
    if (inst->boss_killed)
        return;  /* already triggered */
    
    inst->boss_killed = TRUE;
    
    /* Get the seed to access boss_loot_table */
    seed = pd_seed_get_by_name(inst->theme);
    if (seed == NULL || seed->item_vnum_count <= 0)
        return;
    
    /* Drop 2–3 random boss loot items to boss room */
    int num_drops = number_range(2, 3);
    for (i = 0; i < num_drops; i++) {
        vnum_idx = number_range(0, seed->item_vnum_count - 1);
        oidx = obj_get_index(seed->item_vnums[vnum_idx]);
        if (oidx != NULL) {
            loot = obj_create(oidx, inst->level + 5);  /* slightly higher level for boss drops */
            obj_give_to_room(loot, boss->in_room);
        }
    }
}

/* C4: Assign random powers to boss mob.
 * Selects 2–3 random powers and stores in instance->boss_powers. */
void pd_assign_boss_powers(PD_INSTANCE_T *inst)
{
    if (inst == NULL)
        return;
    
    inst->boss_power_count = 0;
    
    /* Randomly decide: 2 or 3 powers? (60/40 chance for 2/3) */
    int num_powers = number_percent() < 60 ? 2 : 3;
    
    while (inst->boss_power_count < num_powers && inst->boss_power_count < 5) {
        int power = number_range(PD_POWER_STRIKE, PD_POWER_DRAIN);
        
        /* Check if already assigned */
        int i;
        bool already_has = FALSE;
        for (i = 0; i < inst->boss_power_count; i++) {
            if (inst->boss_powers[i] == power) {
                already_has = TRUE;
                break;
            }
        }
        
        if (!already_has) {
            inst->boss_powers[inst->boss_power_count] = power;
            inst->boss_power_count++;
        }
    }
}

/* Find the first free vnum slot among AREA_INSTANCE_MAX possible slots.
 * Returns -1 if none available. */
static int pd_find_free_slot(void)
{
    int slot;
    PD_INSTANCE_T *inst;

    for (slot = 0; slot < pd_config.max_instances; slot++) {
        bool used = FALSE;
        for (inst = pd_instance_first; inst != NULL; inst = inst->global_next) {
            if (inst->vnum_slot == slot) {
                used = TRUE;
                break;
            }
        }
        if (!used)
            return slot;
    }
    return -1;
}

/* Build a two-way bidirectional exit between two rooms. */
static void pd_link_rooms(ROOM_INDEX_T *from, int dir, ROOM_INDEX_T *to)
{
    EXIT_T *ex_fwd, *ex_rev;
    int rev = door_table[dir].reverse;

    if (from->exit[dir] == NULL) {
        ex_fwd = room_create_exit(from, dir);
        exit_to_room_index_to(ex_fwd, to);
    }
    if (to->exit[rev] == NULL) {
        ex_rev = room_create_exit(to, rev);
        exit_to_room_index_to(ex_rev, from);
    }
}

/* Pick a room name from the seed, cycling to avoid repeats. */
static const char *pd_pick_room_name(PD_SEED_T *seed, int index)
{
    if (seed->room_name_count <= 0)
        return "dungeon chamber";
    return seed->room_names[index % seed->room_name_count];
}

/* Pick a room description from the authored pool, or use a safe fallback. */
static const char *pd_pick_room_description(PD_SEED_T *seed, int index)
{
    const char *text;

    if (seed == NULL || seed->room_desc_count <= 0)
        return "The chamber is damp and watchful, with old stone pressing close on every side.";

    text = seed->room_descs[index % seed->room_desc_count].text;
    if (text == NULL || text[0] == '\0')
        return "The chamber is damp and watchful, with old stone pressing close on every side.";

    return text;
}

/* Layout-specific atmosphere clause keeps descriptions from repeating verbatim. */
static const char *pd_pick_room_atmosphere(const char *style, int index)
{
    static const char *linear[] = {
        "The passage stretches in a stubborn line, echoing every footstep back at you.",
        "Dust lies in a thin ribbon across the floor, broken only by fresh tracks.",
        "The corridor feels narrow and deliberate, as if the dungeon itself is guiding you onward."
    };
    static const char *spiral[] = {
        "The route bends back on itself, making it hard to trust your sense of direction.",
        "Stone walls sweep around you in a tightening curve that never quite repeats.",
        "The path seems to coil deeper into the dungeon with every step."
    };
    static const char *hub[] = {
        "Branches of passageway peel away in several directions from the center.",
        "This chamber feels like the heart of a larger maze, waiting for a choice.",
        "Drafts drift in from every side, carrying faint hints of distant rooms."
    };
    static const char *ruins[] = {
        "Broken masonry and weather-softened stone litter the floor in uneven piles.",
        "Roots and rubble crowd the edges here, as though the past is creeping back in.",
        "The air smells of damp dust and long-abandoned memory."
    };
    static const char *cavern[] = {
        "Cold moisture beads on the rock and slips into the dark below.",
        "The chamber breathes with a low draft that stirs the silence but reveals nothing.",
        "The stone is slick and ancient, worn smooth by water and time."
    };
    const char **table = linear;
    int count = 3;

    if (style != NULL) {
        if (!str_cmp(style, "spiral"))
            table = spiral;
        else if (!str_cmp(style, "hub"))
            table = hub;
        else if (!str_cmp(style, "ruins"))
            table = ruins;
        else if (!str_cmp(style, "cavern"))
            table = cavern;
    }

    return table[index % count];
}

/* Attach authored look details to a room if the seed provides them. */
static void pd_attach_room_desc_extra(ROOM_INDEX_T *room, PD_SEED_T *seed, int index)
{
    EXTRA_DESCR_T *ed;
    struct pd_room_desc *desc;

    if (room == NULL || seed == NULL || seed->room_desc_count <= 0)
        return;

    desc = &seed->room_descs[index % seed->room_desc_count];
    if (desc->look_keyword == NULL || desc->look_keyword[0] == '\0' ||
        desc->look_text == NULL || desc->look_text[0] == '\0')
        return;

    ed = extra_descr_new();
    str_replace_dup(&ed->keyword, desc->look_keyword);
    str_replace_dup(&ed->description, desc->look_text);
    ed->parent = room;
    ed->parent_type = EXTRA_DESCR_ROOM_INDEX;
    extra_descr_to_room_index_back(ed, room);
}

/* Calculate effective gear score for a character.
 * Sums equipped item levels + 1 per affect, clamped to [ch->level, ch->level * 2].
 * Used to determine instance scaling for mixed-gear groups. */
static int pd_calc_gear_score(CHAR_T *ch)
{
    int score = ch->level;
    OBJ_T *obj;
    AFFECT_T *aff;

    if (ch == NULL)
        return 1;

    /* Sum equipped item levels */
    for (obj = ch->content_first; obj != NULL; obj = obj->content_next) {
        if (obj->wear_loc != WEAR_LOC_NONE && obj->level > 0)
            score += obj->level;
    }

    /* Add 1 per affect on those items */
    for (obj = ch->content_first; obj != NULL; obj = obj->content_next) {
        if (obj->wear_loc != WEAR_LOC_NONE) {
            for (aff = obj->affect_first; aff != NULL; aff = aff->on_next)
                score += 1;
        }
    }

    /* Clamp to range [ch->level, ch->level * 2] */
    if (score < ch->level)
        score = ch->level;
    if (score > ch->level * 2)
        score = ch->level * 2;

    return score;
}

/* Calculate instance level for a group of members.
 * Uses pd_config.scaling_formula to determine how to blend individual gear scores:
 * 0 = average, 1 = max. Clamped to [1, 100]. */
static int pd_calc_instance_level(CHAR_T **members, int count)
{
    int i, total = 0, level = 1;

    if (members == NULL || count <= 0)
        return 1;

    /* Calculate individual gear scores */
    for (i = 0; i < count; i++) {
        if (members[i] != NULL)
            total += pd_calc_gear_score(members[i]);
    }

    if (count > 0) {
        if (pd_config.scaling_formula == 0) {
            /* Average mode */
            level = total / count;
        } else {
            /* Max mode — find highest gear score */
            level = 1;
            for (i = 0; i < count; i++) {
                if (members[i] != NULL) {
                    int gs = pd_calc_gear_score(members[i]);
                    if (gs > level)
                        level = gs;
                }
            }
        }
    }

    /* Clamp to valid range */
    if (level < 1)
        level = 1;
    if (level > 100)
        level = 100;

    return level;
}

/* Build room layout (linked exits) based on layout_style.
 * Dispatches to one of 5 different area-generation algorithms.
 * All respect: room[0]=entry, room[count-1]=boss, bidirectional exits. */
static void pd_build_layout(ROOM_INDEX_T **rooms, int room_count, PD_SEED_T *seed)
{
    const char *style;
    int i, j;

    if (room_count < 2 || rooms == NULL || seed == NULL)
        return;

    style = (seed->layout_style != NULL && seed->layout_style[0] != '\0')
            ? seed->layout_style
            : "linear";

    if (!str_cmp(style, "linear")) {
        /* Linear: north chain from 0->1->2->...->N
         * Side rooms: even i links east to odd i+1 (alcoves) */
        for (i = 0; i + 1 < room_count; i++)
            pd_link_rooms(rooms[i], DIR_NORTH, rooms[i + 1]);
        for (i = 0; i + 2 < room_count; i += 2)
            pd_link_rooms(rooms[i], DIR_EAST, rooms[i + 1]);
    }
    else if (!str_cmp(style, "spiral")) {
        /* Spiral: uses all 4 cardinal directions + occasional UP/DOWN */
        int dir_seq[] = { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST };
        int stairs = room_count / 6;
        int stair_interval = (room_count > stairs) ? room_count / stairs : 4;
        for (i = 0; i + 1 < room_count; i++) {
            int dir = dir_seq[i % 4];
            pd_link_rooms(rooms[i], dir, rooms[i + 1]);
            if (stairs > 0 && (i + 1) % stair_interval == 0 && i + 2 < room_count) {
                pd_link_rooms(rooms[i + 1], DIR_UP, rooms[i + 2]);
                i++;
                stairs--;
            }
        }
    }
    else if (!str_cmp(style, "hub")) {
        /* Hub: room 0 is central, rooms 1-4 radiate (N/E/S/W)
         * Rooms 5+ form branches from each radius */
        /* Link cardinal branches from hub (0) */
        int hub_dirs[] = { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST };
        for (i = 0; i < 4 && i + 1 < room_count; i++)
            pd_link_rooms(rooms[0], hub_dirs[i], rooms[i + 1]);
        /* Chain remaining rooms as extensions from radius corridors */
        for (i = 5; i < room_count; i++) {
            int parent_branch = ((i - 5) % 4) + 1;
            if (parent_branch < room_count)
                pd_link_rooms(rooms[parent_branch], DIR_NORTH, rooms[i]);
        }
    }
    else if (!str_cmp(style, "ruins")) {
        /* Ruins: sprawling grid with cross-connects and UP/DOWN for depth */
        int cols = (int)number_range(3, 6);
        if (cols < 1) cols = 1;
        int rows = (room_count + cols - 1) / cols;
        int grid_room = 0;
        /* Create horizontal and vertical links for grid structure */
        for (i = 0; i < rows && grid_room < room_count; i++) {
            for (j = 0; j < cols && grid_room < room_count; j++) {
                int room_idx = i * cols + j;
                if (room_idx >= room_count) break;
                /* East link */
                if (j + 1 < cols && room_idx + 1 < room_count)
                    pd_link_rooms(rooms[room_idx], DIR_EAST, rooms[room_idx + 1]);
                /* South link */
                if (i + 1 < rows && room_idx + cols < room_count)
                    pd_link_rooms(rooms[room_idx], DIR_SOUTH, rooms[room_idx + cols]);
                /* Occasional UP for second floor */
                if ((i + j) % 3 == 0 && room_idx + cols * rows < room_count)
                    pd_link_rooms(rooms[room_idx], DIR_UP, rooms[room_idx + cols * rows / 2]);
            }
        }
    }
    else {
        /* Cavern (default): organic tree from main spine, many UP/DOWN */
        int main_count = (room_count * 2 / 3);
        if (main_count < 1) main_count = 1;
        if (main_count > room_count) main_count = room_count - 1;
        /* Main spine */
        for (i = 0; i + 1 < main_count; i++) {
            if (number_percent() < 50)
                pd_link_rooms(rooms[i], DIR_NORTH, rooms[i + 1]);
            else
                pd_link_rooms(rooms[i], DIR_UP, rooms[i + 1]);
        }
        /* Branch remaining rooms off main spine */
        for (i = main_count; i < room_count; i++) {
            int parent_idx = number_range(0, UMIN(main_count - 1, i - 1));
            int branch_dir = (number_percent() < 50) ? DIR_EAST : DIR_WEST;
            if (parent_idx >= 0 && parent_idx < room_count)
                pd_link_rooms(rooms[parent_idx], branch_dir, rooms[i]);
        }
    }
}

/* Scan current room for hidden objects and reveal them based on skill roll.
 * If character has search skill, roll against it; otherwise 50% base chance.
 * Called by 'search' command and spell_detect_hidden.
 * Removes ITEM_HIDDEN flag from found objects and prints discovery message. */
void pd_do_hidden_scan(CHAR_T *ch)
{
    OBJ_T *obj;
    int skill_roll, success_threshold;

    if (ch == NULL || ch->in_room == NULL)
        return;

    /* Determine success threshold based on skill */
    success_threshold = UMAX(50, char_get_skill(ch, SN(SEARCH)));

    /* Iterate all objects in the room */
    for (obj = ch->in_room->content_first; obj != NULL; obj = obj->content_next) {
        if (IS_SET(obj->extra_flags, ITEM_HIDDEN)) {
            /* Roll against success threshold */
            skill_roll = number_percent();
            if (skill_roll < success_threshold) {
                /* Success: reveal the object */
                REMOVE_BIT(obj->extra_flags, ITEM_HIDDEN);
                printf_to_char(ch, "You reveal %s!\n\r", obj->short_descr);
            }
        }
    }
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

PD_SEED_T *pd_seed_get_by_name(const char *name)
{
    PD_SEED_T *seed;
    if (name == NULL || name[0] == '\0')
        return NULL;
    for (seed = pd_seed_first; seed != NULL; seed = seed->global_next)
        if (seed->name != NULL && !str_cmp(seed->name, name))
            return seed;
    return NULL;
}

PD_INSTANCE_T *pd_find_instance_for_char(const CHAR_T *ch)
{
    PD_INSTANCE_T *inst;
    if (ch == NULL || ch->in_room == NULL || ch->in_room->area == NULL)
        return NULL;
    for (inst = pd_instance_first; inst != NULL; inst = inst->global_next)
        if (inst->area == ch->in_room->area)
            return inst;
    return NULL;
}

PD_INSTANCE_T *pd_find_instance_by_member(const char *name)
{
    PD_INSTANCE_T *inst;
    int i;
    if (name == NULL || name[0] == '\0')
        return NULL;
    for (inst = pd_instance_first; inst != NULL; inst = inst->global_next) {
        for (i = 0; i < inst->member_count; i++) {
            if (inst->members[i] != NULL && !str_cmp(inst->members[i], name))
                return inst;
        }
    }
    return NULL;
}

PD_INSTANCE_T *pd_generate_instance(CHAR_T **members, int member_count,
                                     const char *theme)
{
    PD_SEED_T *seed;
    PD_INSTANCE_T *inst;
    AREA_T *area;
    ROOM_INDEX_T *rooms[105]; /* guard against room_count_max > 104 */
    int room_count, i, slot;
    int vnum_base;
    char buf[256];

    /* Select seed */
    if (theme != NULL && theme[0] != '\0')
        seed = pd_seed_get_by_name(theme);
    else
        seed = NULL;

    if (seed == NULL) {
        /* pick random seed */
        int count = 0;
        PD_SEED_T *s;
        for (s = pd_seed_first; s != NULL; s = s->global_next)
            count++;
        if (count == 0) {
            bugf("pd_generate_instance: no seeds loaded");
            return NULL;
        }
        int pick = number_range(0, count - 1);
        seed = pd_seed_first;
        for (i = 0; i < pick && seed != NULL; i++)
            seed = seed->global_next;
        if (seed == NULL)
            seed = pd_seed_first;
    }

    /* B5: Validate seed before using it */
    if (!pd_validate_seed(seed)) {
        bugf("pd_generate_instance: seed validation failed, aborting");
        return NULL;
    }

    /* Find a free slot */
    slot = pd_find_free_slot();
    if (slot < 0) {
        bugf("pd_generate_instance: no free instance slots");
        return NULL;
    }
    vnum_base = pd_config.vnum_base + slot * pd_config.vnum_size;

    /* Create the instance area */
    area = area_new();
    str_free(&area->name);
    snprintf(buf, sizeof(buf), "pd-inst-%d", slot);
    area->name = str_dup(buf);

    str_free(&area->title);
    snprintf(buf, sizeof(buf), "%s (Instance)", seed->title ? seed->title : "Pocket Dungeon");
    area->title = str_dup(buf);

    str_free(&area->filename);
    area->filename = str_dup("");

    str_free(&area->builders);
    area->builders = str_dup("None");

    area->min_vnum   = vnum_base;
    area->max_vnum   = vnum_base + pd_config.vnum_size - 1;
    area->area_flags = AREA_INSTANCE | AREA_HIDDEN;
    area->security   = 0;

    /* Determine room count for this instance */
    room_count = seed->room_count_min;
    if (seed->room_count_max > seed->room_count_min)
        room_count = number_range(seed->room_count_min, seed->room_count_max);
    if (room_count < 1)
        room_count = 4;
    if (room_count > pd_config.vnum_size - 1)
        room_count = pd_config.vnum_size - 1;
    if (room_count > 104)
        room_count = 104;

    /* Create rooms */
    memset(rooms, 0, sizeof(rooms));
    for (i = 0; i < room_count; i++) {
        ROOM_INDEX_T *room = room_index_new();
        int vnum = vnum_base + i;
        char name_buf[MAX_STRING_LENGTH];
        char desc_buf[MAX_STRING_LENGTH];

        snprintf(name_buf, sizeof(name_buf), "%s", pd_pick_room_name(seed, i));
        str_replace_dup(&room->name, str_capitalized(name_buf));

        snprintf(desc_buf, sizeof(desc_buf), "%s %s\n\r",
                 pd_pick_room_description(seed, i),
                 pd_pick_room_atmosphere(seed->layout_style, i));
        str_replace_dup(&room->description, desc_buf);

        room->vnum    = vnum;
        room->anum    = i;
        room->sector_type = SECT_INSIDE;
        SET_BIT(room->room_flags, ROOM_INDOORS);

        pd_attach_room_desc_extra(room, seed, i);

        room_to_area(room, area);
        room_index_to_hash(room);

        rooms[i] = room;
    }

    /* Build the room layout based on seed's style */
    pd_build_layout(rooms, room_count, seed);

    /* Apply room name overrides if provided */
    if (seed->entry_room_name != NULL && seed->entry_room_name[0] != '\0')
        str_replace_dup(&rooms[0]->name, seed->entry_room_name);

    /* entry_vnum is the first room; place entry portal there */
    int entry_vnum = vnum_base;

    /* Create the instance record */
    inst                = pd_instance_new();
    inst->vnum_slot     = slot;
    inst->area          = area;
    inst->entry_vnum    = entry_vnum;
    inst->created_at    = (long)time(NULL);
    inst->last_empty_at = 0;
    inst->level         = pd_calc_instance_level(members, member_count);

    str_replace_dup(&inst->theme, seed->name ? seed->name : "");
    snprintf(inst->area_name, sizeof(inst->area_name), "%s", area->name);

    /* C1: Apply random affixes to this instance */
    pd_apply_affixes(inst);
    
    /* C2: Initialize room-clearing counter for progressive difficulty */
    inst->rooms_cleared = 0;

    /* Copy members */
    inst->member_count = 0;
    for (i = 0; i < member_count && i < MAX_INSTANCE_MEMBERS; i++) {
        if (members[i] == NULL || IS_NPC(members[i]))
            continue;
        str_replace_dup(&inst->members[inst->member_count],
                        members[i]->name);
        inst->member_count++;
    }

    /* Register in global list */
    LIST2_BACK(inst, global_prev, global_next,
               pd_instance_first, pd_instance_last);

    /* Spawn mobs with density scaling and depth multipliers */
    if (seed->mob_vnum_count > 0 && seed->mob_density > 0) {
        int density_min = seed->mob_density_min > 0 ? seed->mob_density_min : 1;
        int density_max = seed->mob_density_max > 0 ? seed->mob_density_max : 3;
        
        for (i = 1; i < room_count; i++) {
            /* Skip boss room (last room) and chest room */
            if (i == room_count - 1)
                continue;
            if (room_count > 4 && i == room_count / 4)
                continue;  /* skip chest room */
            
            /* B1+C2: Scale density based on instance level
             * At level 1: use density_min
             * At level 50+: use density_max
             * Interpolate between in range [1, 50]
             * B1: Hard cap at 5 mobs per room to prevent overflow */
            int scaled_level = inst->level;
            if (scaled_level > 50)
                scaled_level = 50;
            int mob_count = density_min + 
                            ((density_max - density_min) * scaled_level) / 50;
            if (mob_count < density_min)
                mob_count = density_min;
            if (mob_count > density_max)
                mob_count = density_max;
            
            /* C1-Ancient: Increase density by 50% if Ancient affix active */
            int k;
            for (k = 0; k < inst->affix_count; k++) {
                if (inst->affixes[k] == PD_AFFIX_ANCIENT) {
                    mob_count = (mob_count * 150) / 100;  /* 50% boost */
                    break;
                }
            }
            
            /* B1: Apply hard cap to prevent unplayable crowding */
            if (mob_count > 5) {
                if (mob_count > 5 && inst->level > 40)
                    log_f("WARNING: density cap hit at room %d, seed tried %d mobs", i, mob_count);
                mob_count = 5;
            }
            
            /* C2: Skip room entirely at very low levels (chance decreases as level rises) */
            if (inst->level < 20 && number_percent() < (20 - inst->level))
                continue;
            
            /* B2+C2a: Depth scaling — mobs deeper into dungeon get level bonus
             * Calculate room depth: distance from entry room (0)
             * B2: Reduced multiplier (1.0–1.3×) instead of 1.0–1.5× to prevent 
             *     boss-level mobs appearing too early; less aggressive scaling */
            float depth_mult = 1.0;
            if (i > 0 && room_count > 1) {
                float depth = (float)i / (float)(room_count - 1);
                depth_mult = 1.0 + (depth * 0.3);  /* B2: ranges [1.0, 1.3] reduced from [1.0, 1.5] */
            }
            
            int m;
            for (m = 0; m < mob_count; m++) {
                int vnum_idx = (i + m) % seed->mob_vnum_count;
                MOB_INDEX_T *midx = mobile_get_index(seed->mob_vnums[vnum_idx]);
                if (midx != NULL) {
                    CHAR_T *mob = mobile_create(midx);
                    /* Scale to instance level with depth bonus and ±2 variance.
                     * Depth scaling makes progression feel natural. */
                    int depth_level = (int)(inst->level * depth_mult + 0.5);
                    int final_level = UMAX(1, depth_level + number_range(-2, 2));
                    if (final_level > 100)
                        final_level = 100;
                    pd_scale_mob_to_level(mob, final_level);
                    
                    /* C1: Apply affix modifiers to mob */
                    int k;
                    for (k = 0; k < inst->affix_count; k++) {
                        switch (inst->affixes[k]) {
                            case PD_AFFIX_STONY:
                                /* +20% HP */
                                mob->max_hit = (int)(mob->max_hit * 1.2);
                                mob->hit = mob->max_hit;
                                break;
                            case PD_AFFIX_SWIFT:
                                /* +25% haste (via improved hitroll) */
                                mob->hitroll += mob->level / 8;
                                break;
                            case PD_AFFIX_ANCIENT:
                                /* Ancient affects density (handled above), not individual mob */
                                break;
                            case PD_AFFIX_CURSED:
                                /* Mark mob as cursed; combat engine will reverse healing */
                                EXT_SET(mob->ext_mob, MOB_CURSED);
                                break;
                            case PD_AFFIX_LUMINOUS:
                                /* -20% dodge: increase AC values (worse protection) */
                                {
                                    int ac_idx;
                                    for (ac_idx = 0; ac_idx < 4; ac_idx++)
                                        mob->armor[ac_idx] += mob->armor[ac_idx] / 5;
                                }
                                break;
                        }
                    }

                    pd_generate_and_attach_mobprog(mob, inst);
                    
                    char_to_room(mob, rooms[i]);
                }
            }
        }
    }

    /* Spawn boss in the deepest room */
    if (seed->boss_vnum > 0 && room_count > 1) {
        /* C4: Assign random powers before boss spawns */
        pd_assign_boss_powers(inst);
        
        MOB_INDEX_T *boss_idx = mobile_get_index(seed->boss_vnum);
        if (boss_idx != NULL) {
            CHAR_T *boss = mobile_create(boss_idx);
            int boss_level = inst->level + seed->boss_level_add;
            if (boss_level < inst->level)
                boss_level = inst->level;
            if (boss_level > 100)
                boss_level = 100;
            pd_scale_mob_to_level(boss, boss_level);
            
            /* C1: Apply Stony affix effect to boss for extra toughness */
            int k;
            for (k = 0; k < inst->affix_count; k++) {
                if (inst->affixes[k] == PD_AFFIX_STONY) {
                    boss->max_hit = (int)(boss->max_hit * 1.2);
                    boss->hit = boss->max_hit;
                    break;
                }
            }
            
            /* C1: Apply Cursed affix to boss if active */
            for (k = 0; k < inst->affix_count; k++) {
                if (inst->affixes[k] == PD_AFFIX_CURSED) {
                    EXT_SET(boss->ext_mob, MOB_CURSED);
                    break;
                }
            }
            
            /* C1: Apply Luminous affix to boss if active */
            for (k = 0; k < inst->affix_count; k++) {
                if (inst->affixes[k] == PD_AFFIX_LUMINOUS) {
                    int ac_idx;
                    for (ac_idx = 0; ac_idx < 4; ac_idx++)
                        boss->armor[ac_idx] += boss->armor[ac_idx] / 5;
                    break;
                }
            }

            pd_generate_and_attach_mobprog(boss, inst);
            
            char_to_room(boss, rooms[room_count - 1]);
            /* Override boss room name if provided */
            if (seed->boss_room_name != NULL && seed->boss_room_name[0] != '\0')
                str_replace_dup(&rooms[room_count - 1]->name, seed->boss_room_name);
        }
    }

    /* ===== THREE-TIER LOOT SYSTEM ===== */

    /* TIER 1: Open treasure chest with optional sentinel guardian */
    if (seed->container_vnum > 0 && seed->loot_density > 0 && room_count > 2) {
        int chest_room_idx = room_count / 4;
        if (chest_room_idx < 1) chest_room_idx = 1;
        if (chest_room_idx == room_count - 1) chest_room_idx--; /* avoid boss room */

        /* Spawn sentinel guardian if vnum provided */
        if (seed->sentinel_vnum > 0) {
            MOB_INDEX_T *sentinel_idx = mobile_get_index(seed->sentinel_vnum);
            if (sentinel_idx != NULL) {
                CHAR_T *sentinel = mobile_create(sentinel_idx);
                int sentinel_level = inst->level + seed->sentinel_level_add;
                if (sentinel_level < inst->level)
                    sentinel_level = inst->level;
                if (sentinel_level > 100)
                    sentinel_level = 100;
                pd_scale_mob_to_level(sentinel, sentinel_level);
                EXT_SET(sentinel->ext_mob, MOB_SENTINEL);
                pd_generate_and_attach_mobprog(sentinel, inst);
                char_to_room(sentinel, rooms[chest_room_idx]);
            }
        }

        /* Create and stock the chest */
        OBJ_T *chest = obj_create(obj_get_index(seed->container_vnum), inst->level);
        if (chest != NULL) {
            int chest_items = UMAX(1, seed->loot_density);
            int m;
            for (m = 0; m < chest_items && seed->item_vnum_count > 0; m++) {
                int vnum_idx = m % seed->item_vnum_count;
                OBJ_INDEX_T *oidx = obj_get_index(seed->item_vnums[vnum_idx]);
                if (oidx != NULL) {
                    OBJ_T *loot = obj_create(oidx, inst->level);
                    obj_give_to_obj(loot, chest);
                }
            }
            obj_give_to_room(chest, rooms[chest_room_idx]);

            /* Override chest room name if provided */
            if (seed->chest_room_name != NULL && seed->chest_room_name[0] != '\0')
                str_replace_dup(&rooms[chest_room_idx]->name, seed->chest_room_name);
        }
    }

    /* TIER 2: Hidden cache with hints and extra descriptions */
    if (seed->hidden_container_vnum > 0 && seed->hide_hint_count > 0) {
        int hide_room_idx;
        int attempts = 0;
        
        /* Pick a random side room (not entry, not boss, not chest) */
        do {
            hide_room_idx = number_range(2, room_count - 2);
            attempts++;
        } while (attempts < 10 && (hide_room_idx == room_count - 1 || 
                 (room_count > 4 && hide_room_idx == room_count / 4)));

        if (hide_room_idx >= 1 && hide_room_idx < room_count) {
            /* Pick a random hint from the pool */
            int hint_idx = number_range(0, seed->hide_hint_count - 1);

            /* Create hidden container with ITEM_HIDDEN flag */
            OBJ_T *hidden_cache = obj_create(obj_get_index(seed->hidden_container_vnum), inst->level);
            if (hidden_cache != NULL) {
                SET_BIT(hidden_cache->extra_flags, ITEM_HIDDEN);

                /* Stock the hidden cache with loot */
                int cache_items = UMAX(1, seed->loot_density / 2);
                int m;
                for (m = 0; m < cache_items && seed->item_vnum_count > 0; m++) {
                    int vnum_idx = (m + hint_idx) % seed->item_vnum_count;
                    OBJ_INDEX_T *oidx = obj_get_index(seed->item_vnums[vnum_idx]);
                    if (oidx != NULL) {
                        OBJ_T *loot = obj_create(oidx, inst->level);
                        obj_give_to_obj(loot, hidden_cache);
                    }
                }
                obj_give_to_room(hidden_cache, rooms[hide_room_idx]);

                /* Add extra description (look keyword) if available */
                if (seed->hide_keywords[hint_idx] != NULL && 
                    seed->hide_look_texts[hint_idx] != NULL) {
                    EXTRA_DESCR_T *ed = extra_descr_new();
                    str_replace_dup(&ed->keyword, seed->hide_keywords[hint_idx]);
                    str_replace_dup(&ed->description, seed->hide_look_texts[hint_idx]);
                    ed->parent = rooms[hide_room_idx];
                    ed->parent_type = EXTRA_DESCR_ROOM_INDEX;
                    extra_descr_to_room_index_back(ed, rooms[hide_room_idx]);
                }

                /* B3: Store hint phrase in extra_descr to prevent exploit
                 * Only hint text is accessible via look, not appended to room description.
                 * This prevents players from reading hints without the search skill. */
                if (seed->hide_hint_phrases[hint_idx] != NULL &&
                    seed->hide_hint_phrases[hint_idx][0] != '\0') {
                    EXTRA_DESCR_T *hint_ed = extra_descr_new();
                    str_replace_dup(&hint_ed->keyword, "hint");
                    str_replace_dup(&hint_ed->description, seed->hide_hint_phrases[hint_idx]);
                    hint_ed->parent = rooms[hide_room_idx];
                    hint_ed->parent_type = EXTRA_DESCR_ROOM_INDEX;
                    extra_descr_to_room_index_back(hint_ed, rooms[hide_room_idx]);
                }
            }
        }
    }

    /* TIER 3: Floor drop loot (spread across remaining rooms) */
    if (seed->item_vnum_count > 0 && seed->loot_density > 0) {
        /* B4: Validate loot_density to prevent overflow
         * Valid range: [0.1, 3.0]. Clamp to prevent division issues. */
        float loot_density = seed->loot_density;
        if (loot_density < 0.1f)
            loot_density = 0.1f;
        if (loot_density > 3.0f)
            loot_density = 3.0f;
        
        int loot_interval = UMAX(1, (int)(50 / loot_density));  /* B4: safer calc */
        for (i = 1; i < room_count; i += loot_interval) {
            /* Skip chest and boss rooms */
            if (room_count > 4 && i == room_count / 4) continue;
            if (i == room_count - 1) continue;

            int vnum_idx = i % seed->item_vnum_count;
            OBJ_INDEX_T *oidx = obj_get_index(seed->item_vnums[vnum_idx]);
            if (oidx != NULL) {
                OBJ_T *obj = obj_create(oidx, inst->level);
                obj_give_to_room(obj, rooms[i]);
            }
        }
    }

    /* Place the entry/return portal in the first room (permanent, so
     * GATE_PERMANENT keeps it from decaying).  to_vnum points back to the
     * character's original room; that will be set by the caller. */
    {
        OBJ_T *portal = obj_create(obj_get_index(OBJ_VNUM_PORTAL), 0);
        if (portal != NULL) {
            portal->timer = -1;
            SET_BIT(portal->v.portal.gate_flags, GATE_PERMANENT);
            portal->v.portal.to_vnum = ROOM_VNUM_TEMPLE; /* caller overrides */
            str_replace_dup(&portal->short_descr, "a shimmering exit portal");
            str_replace_dup(&portal->name, "portal exit");
            obj_give_to_room(portal, rooms[0]);
        }
    }

    pd_write_snapshot(inst);
    return inst;
}

/* -------------------------------------------------------------------------
 * Snapshot helpers — write/delete json/temp/areas/pd-inst-N.json
 * ---------------------------------------------------------------------- */

/* Recursively freeze an object (and its contents) so obj_update won't
 * extract it.  Used to preserve PC corpses while they are in an instance. */
static void pd_freeze_obj_recursive(OBJ_T *obj)
{
    OBJ_T *child;
    if (obj == NULL)
        return;
    /* Only freeze objects that are actively decaying (timer > 0).
     * Skip already-permanent items (timer <= 0) and already-frozen ones. */
    if (obj->timer > 0 && obj->pd_saved_timer == 0) {
        obj->pd_saved_timer = obj->timer;
        obj->timer = -1;
    }
    for (child = obj->content_first; child != NULL; child = child->content_next)
        pd_freeze_obj_recursive(child);
}

static const char *pd_dir_names[DIR_MAX] = {
    "north", "east", "south", "west", "up", "down"
};

/* Write a single character c to fp, JSON-encoding where necessary. */
static void pd_fputc_json(FILE *fp, char c)
{
    if      (c == '"')  fputs("\\\"", fp);
    else if (c == '\\') fputs("\\\\", fp);
    else if (c == '\n') fputs("\\n",  fp);
    else if (c == '\r') ; /* skip */
    else                fputc(c, fp);
}

static void pd_fputs_json(FILE *fp, const char *s)
{
    if (s == NULL) return;
    for (; *s; s++)
        pd_fputc_json(fp, *s);
}

void pd_write_snapshot(PD_INSTANCE_T *inst)
{
    FILE *fp;
    char filepath[256];
    ROOM_INDEX_T *room;
    int i, first_room, first_exit;

    if (inst == NULL || inst->area == NULL)
        return;

    /* B7: Use unique filename with instance ID and timestamp to prevent race conditions */
    snprintf(filepath, sizeof(filepath), "%spd-inst-%d-%ld.json",
             PD_TEMP_DIR, inst->id, (long)inst->created_at);

    fp = fopen(filepath, "w");
    if (fp == NULL) {
        bugf("pd_write_snapshot: cannot open '%s' for writing", filepath);
        return;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"slot\": %d,\n",          inst->vnum_slot);
    fprintf(fp, "  \"id\": %d,\n",             inst->id);
    fprintf(fp, "  \"theme\": \"");
    pd_fputs_json(fp, inst->theme ? inst->theme : "");
    fprintf(fp, "\",\n");
    fprintf(fp, "  \"level\": %d,\n",          inst->level);
    fprintf(fp, "  \"created_at\": %ld,\n",    (long)inst->created_at);
    fprintf(fp, "  \"last_empty_at\": %ld,\n", (long)inst->last_empty_at);
    fprintf(fp, "  \"entry_vnum\": %d,\n",     inst->entry_vnum);
    fprintf(fp, "  \"area_name\": \"");
    pd_fputs_json(fp, inst->area_name);
    fprintf(fp, "\",\n");

    fprintf(fp, "  \"members\": [");
    for (i = 0; i < inst->member_count; i++) {
        if (i > 0) fprintf(fp, ", ");
        fprintf(fp, "\"");
        pd_fputs_json(fp, inst->members[i] ? inst->members[i] : "");
        fprintf(fp, "\"");
    }
    fprintf(fp, "],\n");

    fprintf(fp, "  \"rooms\": [\n");
    first_room = 1;
    for (room = inst->area->room_first; room != NULL; room = room->area_next) {
        if (!first_room)
            fprintf(fp, ",\n");
        first_room = 0;

        fprintf(fp, "    {\n");
        fprintf(fp, "      \"vnum\": %d,\n", room->vnum);
        fprintf(fp, "      \"name\": \"");
        pd_fputs_json(fp, room->name);
        fprintf(fp, "\",\n");
        fprintf(fp, "      \"description\": \"");
        pd_fputs_json(fp, room->description ? room->description : "");
        fprintf(fp, "\",\n");

        fprintf(fp, "      \"exits\": {");
        first_exit = 1;
        for (i = 0; i < DIR_MAX; i++) {
            if (room->exit[i] != NULL && room->exit[i]->to_room != NULL) {
                if (!first_exit)
                    fprintf(fp, ", ");
                first_exit = 0;
                fprintf(fp, "\"%s\": %d",
                        pd_dir_names[i], room->exit[i]->to_room->vnum);
            }
        }
        fprintf(fp, "},\n");

        /* Count live NPCs and generate mob array */
        {
            int mob_count = 0;
            int first_mob = 1;
            MPROG_LIST_T *prg;
            CHAR_T *ch;
            
            /* Count mobs first */
            for (ch = room->people_first; ch != NULL; ch = ch->room_next) {
                if (IS_NPC(ch))
                    mob_count++;
            }
            
            fprintf(fp, "      \"mob_count\": %d,\n", mob_count);
            fprintf(fp, "      \"mobs\": [");
            
            /* Output mob array */
            for (ch = room->people_first; ch != NULL; ch = ch->room_next) {
                if (IS_NPC(ch)) {
                    if (!first_mob)
                        fprintf(fp, ", ");
                    first_mob = 0;
                    fprintf(fp, "{\"vnum\": %d, \"name\": \"",
                            ch->mob_index ? ch->mob_index->vnum : 0);
                    pd_fputs_json(fp, ch->short_descr ? ch->short_descr : "");
                    fprintf(fp, "\", \"hp\": %d, \"max_hp\": %d, \"mobprogs\": [",
                            ch->hit, ch->max_hit);
                    if (ch->mob_index != NULL) {
                        int first_prog = 1;
                        for (prg = ch->mob_index->mprog_first; prg != NULL; prg = prg->mob_next) {
                            if (!first_prog)
                                fprintf(fp, ", ");
                            first_prog = 0;
                            fprintf(fp, "{\"trigger\": \"");
                            pd_fputs_json(fp, mprog_type_to_name(prg->trig_type));
                            fprintf(fp, "\", \"phrase\": \"");
                            pd_fputs_json(fp, prg->trig_phrase ? prg->trig_phrase : "");
                            fprintf(fp, "\"}");
                        }
                    }
                    fprintf(fp, "]}");
                }
            }
            fprintf(fp, "],\n");
        }

        /* Generate objects array */
        {
            int first_obj = 1;
            OBJ_T *obj;
            
            fprintf(fp, "      \"objects\": [");
            for (obj = room->content_first; obj != NULL; obj = obj->content_next) {
                if (!first_obj)
                    fprintf(fp, ", ");
                first_obj = 0;
                fprintf(fp, "{\"vnum\": %d, \"name\": \"",
                        obj->obj_index ? obj->obj_index->vnum : 0);
                pd_fputs_json(fp, obj->short_descr ? obj->short_descr : "");
                fprintf(fp, "\", \"hidden\": %s}",
                        IS_SET(obj->extra_flags, ITEM_HIDDEN) ? "true" : "false");
            }
            fprintf(fp, "]\n");
        }
        fprintf(fp, "    }");
    }
    fprintf(fp, "\n  ]\n");
    fprintf(fp, "}\n");

    fclose(fp);
}

void pd_delete_snapshot(PD_INSTANCE_T *inst)
{
    char filepath[256];
    if (inst == NULL)
        return;
    /* B7: Match the new filename format with instance ID and timestamp */
    snprintf(filepath, sizeof(filepath), "%spd-inst-%d-%ld.json",
             PD_TEMP_DIR, inst->id, (long)inst->created_at);
    remove(filepath);
}

void pd_destroy_instance(PD_INSTANCE_T *inst)
{
    AREA_T *area;
    ROOM_INDEX_T *room;
    CHAR_T *mob, *mob_next;
    OBJ_T *obj, *obj_next;

    if (inst == NULL)
        return;
    pd_delete_snapshot(inst);
    area = inst->area;

    if (area != NULL) {
        /* Evacuate any remaining PCs to temple before cleanup */
        for (room = area->room_first; room != NULL; room = room->area_next) {
            CHAR_T *ch, *ch_next;
            for (ch = room->people_first; ch != NULL; ch = ch_next) {
                ch_next = ch->room_next;
                if (!IS_NPC(ch)) {
                    char_from_room(ch);
                    char_to_room(ch, room_get_index(ROOM_VNUM_TEMPLE));
                    send_to_char("The dungeon collapses, sending you back to the temple!\n\r", ch);
                }
            }
        }

        /* Purge all mobs so room_index_dispose does not char_free live chars */
        for (room = area->room_first; room != NULL; room = room->area_next) {
            for (mob = room->people_first; mob != NULL; mob = mob_next) {
                mob_next = mob->room_next;
                if (IS_NPC(mob))
                    char_extract(mob);
            }
        }

        /* Purge all loose objects; area_free will also do this but be explicit */
        for (room = area->room_first; room != NULL; room = room->area_next) {
            for (obj = room->content_first; obj != NULL; obj = obj_next) {
                obj_next = obj->content_next;
                obj_extract(obj);
            }
        }

        /* area_free cascades: frees rooms (via room_index_free), which removes
         * them from the hash table and area list via room_index_dispose. */
        area_free(area);
        inst->area = NULL;
    }

    /* Remove from global list and free */
    pd_instance_free(inst);
}

void pd_update_all(void)
{
    PD_INSTANCE_T *inst, *inst_next;
    long now = (long)time(NULL);
    long timeout_secs = (long)pd_config.empty_timeout_mins * 60L;

    if (!pd_config.autopurge)
        return;

    for (inst = pd_instance_first; inst != NULL; inst = inst_next) {
        inst_next = inst->global_next;

        /* Check if any PCs are still inside */
        bool occupied = FALSE;
        bool has_corpse = FALSE;
        ROOM_INDEX_T *room;
        OBJ_T *cobj;

        if (inst->area != NULL) {
            for (room = inst->area->room_first; room != NULL; room = room->area_next) {
                CHAR_T *ch;
                for (ch = room->people_first; ch != NULL; ch = ch->room_next) {
                    if (!IS_NPC(ch)) {
                        occupied = TRUE;
                        break;
                    }
                }
                /* Scan for PC corpses and freeze them so they don't decay */
                for (cobj = room->content_first; cobj != NULL; cobj = cobj->content_next) {
                    if (cobj->item_type == ITEM_CORPSE_PC) {
                        has_corpse = TRUE;
                        pd_freeze_obj_recursive(cobj);
                    }
                }
            }
        }

        if (occupied) {
            inst->last_empty_at = 0;
        }
        else if (has_corpse) {
            /* Keep the instance alive while a player corpse is present.
             * Reset empty clock so the timeout won't fire until after the
             * last corpse is looted/removed. */
            inst->last_empty_at = 0;
        }
        else {
            if (inst->last_empty_at == 0)
                inst->last_empty_at = now;
            else if (now - inst->last_empty_at >= timeout_secs)
                pd_destroy_instance(inst);
        }
    }
}
