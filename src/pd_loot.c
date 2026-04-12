/* pd_loot.c — Pocket Dungeon procedural item enhancer.
 * Fills consumable spell slots, sets wand/staff charges, adds combat affixes
 * to weapons/armor/jewelry, and applies tier-based naming prefixes.
 */

#include "globals.h"
#include "pd_loot.h"
#include "objs.h"
#include "lookup.h"
#include "affects.h"
#include "recycle.h"
#include "flags.h"
#include "utils.h"
#include "memory.h"
#include "skills.h"

#include <string.h>
#include <stdio.h>

/* Fallback spell pool used when no theme has a spell_pool. */
static const char *pd_default_spells[] = {
    "cure light", "cure serious", "cure critical", "heal",
    "refresh", "restore mana", "armor", "bless", "haste",
    "stone skin", "remove curse", "invisibility", NULL
};

/* Fallback stat pool used when no theme has a stat_pool. */
static const int pd_default_stats[] = {
    APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_WIS, APPLY_CON,
};
static const int pd_default_stats_count = 5;

/* Public: look up a loot theme by seed name. Returns NULL if not found. */
PD_LOOT_THEME_T *pd_loot_theme_get(const char *name)
{
    PD_LOOT_THEME_T *th;
    if (name == NULL || name[0] == '\0') return NULL;
    for (th = pd_loot_theme_first; th != NULL; th = th->global_next) {
        if (th->name && strcmp(th->name, name) == 0)
            return th;
    }
    return NULL;
}

/* Post-load: resolve spell name strings stored during JSON loading into
 * skill SNs now that skill_table has been populated by skills.json. */
void pd_loot_themes_reload_spells(void)
{
    PD_LOOT_THEME_T *th;
    int i;

    for (th = pd_loot_theme_first; th != NULL; th = th->global_next)
    {
        /* Skip already-resolved themes: spell_names[] are all NULL after the
         * first pass. This function is called on every skill_reload_mapping,
         * which fires for both skills.json and weapons.json. */
        {
            int has_names = 0;
            for (i = 0; i < th->spell_pool_count; i++)
                if (th->spell_names[i] != NULL) { has_names = 1; break; }
            if (!has_names)
                continue;
        }

        int count = th->spell_pool_count;
        th->spell_pool_count = 0;
        for (i = 0; i < count; i++)
        {
            int sn = skill_lookup_exact(th->spell_names[i]);
            if (sn >= 0)
                th->spell_pool[th->spell_pool_count++] = sn;
            else
                bugf("pd_loot_themes_reload_spells: Unknown spell '%s' in theme '%s'",
                     th->spell_names[i], th->name ? th->name : "?");
            str_free(&th->spell_names[i]);
        }
    }
}

/* Pick a random spell SN from theme pool, or fallback list. */
static int pd_pick_spell(PD_LOOT_THEME_T *th)
{
    if (th && th->spell_pool_count > 0)
        return th->spell_pool[number_range(0, th->spell_pool_count - 1)];

    /* Fallback: count NULL-terminated list, pick from it. */
    {
        int count = 0;
        const char **p = pd_default_spells;
        while (*p++) count++;
        if (count == 0) return -1;
        return skill_lookup_exact(pd_default_spells[number_range(0, count - 1)]);
    }
}

/* Pick a random APPLY_ constant from theme pool, or fallback array. */
static int pd_pick_stat(PD_LOOT_THEME_T *th, int slot)
{
    if (th && th->stat_pool_count > 0)
        return th->stat_pool[number_range(0, th->stat_pool_count - 1)];
    return pd_default_stats[slot % pd_default_stats_count];
}

static int pd_roll_charges(int tier)
{
    switch (tier) {
        case PD_QUALITY_FLOOR: return number_range(3, 8);
        case PD_QUALITY_CHEST: return number_range(6, 12);
        case PD_QUALITY_BOSS:  return number_range(10, 18);
        default: return number_range(3, 8);
    }
}

static void pd_add_affect(OBJ_T *obj, int level, int apply, int mod)
{
    AFFECT_T *af = affect_new();
    affect_init(af, AFF_TO_OBJECT, -1, level, -1, apply, mod, 0);
    affect_to_obj_back(af, obj);
    affect_modify_obj(af, obj);
}

void pd_enhance_obj(OBJ_T *obj, PD_INSTANCE_T *inst, int tier)
{
    char buf[MAX_STRING_LENGTH];
    PD_LOOT_THEME_T *th;

    if (obj == NULL || inst == NULL)
        return;

    th = pd_loot_theme_get(inst->theme);

    switch (obj->item_type) {
    case ITEM_POTION:
    case ITEM_SCROLL:
    case ITEM_PILL: {
        int sn;
        if (th && th->potion_level_max > 0)
            obj->v.value[0] = URANGE(th->potion_level_min, inst->level, th->potion_level_max);
        else
            obj->v.value[0] = UMAX(1, inst->level);
        /* Fill primary spell slot if missing. */
        if (obj->v.value[1] == 0) {
            sn = pd_pick_spell(th);
            if (sn > 0) obj->v.value[1] = sn;
        }
        /* For chest/boss, fill a second spell slot. */
        if (tier >= PD_QUALITY_CHEST && obj->v.value[2] == 0) {
            sn = pd_pick_spell(th);
            if (sn > 0) obj->v.value[2] = sn;
        }
        SET_BIT(obj->extra_flags, ITEM_UNIDENTIFIED);
        return; /* no naming prefix for consumables */
    }

    case ITEM_WAND:
    case ITEM_STAFF: {
        int charges;
        obj->v.value[0] = UMAX(1, inst->level);
        if (th && th->wand_charges_max > 0) {
            charges = number_range(UMAX(1, th->wand_charges_min),
                                   UMAX(1, th->wand_charges_max));
        } else {
            charges = pd_roll_charges(tier);
        }
        obj->v.value[1] = charges; /* max charges */
        obj->v.value[2] = charges; /* current charges */
        if (obj->v.value[3] == 0) {
            int sn = pd_pick_spell(th);
            if (sn > 0) obj->v.value[3] = sn;
        }
        SET_BIT(obj->extra_flags, ITEM_UNIDENTIFIED);
        return; /* no naming prefix for wands */
    }

    case ITEM_WEAPON: {
        int hit_mod = inst->level / 10 + tier;
        int dam_mod = inst->level / 10 + tier;
        pd_add_affect(obj, inst->level, APPLY_HITROLL, hit_mod);
        pd_add_affect(obj, inst->level, APPLY_DAMROLL, dam_mod);
        if (tier >= PD_QUALITY_BOSS) {
            int stat_apply = (number_percent() < 50) ? APPLY_STR : APPLY_DEX;
            int stat_mod   = UMAX(1, inst->level / 10 + 1);
            pd_add_affect(obj, inst->level, stat_apply, stat_mod);
        }
        break;
    }

    case ITEM_ARMOR: {
        int i;
        int ac_mod = tier * (inst->level / 10);
        for (i = 0; i < 4; i++)
            obj->v.value[i] += ac_mod;
        pd_add_affect(obj, inst->level, APPLY_AC, -(tier * (inst->level / 8 + 1)));
        if (tier >= PD_QUALITY_CHEST) {
            int stat_apply = (tier >= PD_QUALITY_BOSS) ? APPLY_STR : APPLY_CON;
            int stat_mod   = UMAX(1, inst->level / 10 + 1);
            pd_add_affect(obj, inst->level, stat_apply, stat_mod);
        }
        break;
    }

    case ITEM_JEWELRY:
    case ITEM_TREASURE: {
        int count = (tier == PD_QUALITY_FLOOR) ? 1 : (tier == PD_QUALITY_CHEST ? 2 : 3);
        int i;
        for (i = 0; i < count; i++) {
            int apply = pd_pick_stat(th, i);
            int mod   = UMAX(1, inst->level / 10 + tier);
            pd_add_affect(obj, inst->level, apply, mod);
        }
        break;
    }

    default:
        return; /* nothing to add, skip naming */
    }

    /* Apply naming prefix to equipment (not consumables — they return early above). */
    if (tier >= PD_QUALITY_BOSS)
        snprintf(buf, sizeof(buf), "a rare %s", obj->short_descr);
    else if (tier >= PD_QUALITY_CHEST)
        snprintf(buf, sizeof(buf), "a fine %s", obj->short_descr);
    else
        return; /* floor gear: no prefix */

    str_replace_dup(&obj->short_descr, buf);
}

