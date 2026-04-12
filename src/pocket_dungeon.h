/***************************************************************************
 * Pocket Dungeon — instanced procedural area generator.
 * pocket_dungeon.h — public interface.
 ***************************************************************************/
#ifndef POCKET_DUNGEON_H
#define POCKET_DUNGEON_H

#include "merc.h"
#include "pocket_dungeon_mobprog.h"

/* C1: Affix types (random dungeon modifiers) */
enum pd_affix_enum {
    PD_AFFIX_NONE = 0,
    PD_AFFIX_STONY,       /* +20% mob HP */
    PD_AFFIX_CURSED,      /* healing reversed for 10 ticks */
    PD_AFFIX_SWIFT,       /* +25% mob haste */
    PD_AFFIX_ANCIENT,     /* +1 mob density per 2 rooms */
    PD_AFFIX_LUMINOUS,    /* -20% mob dodge, +10% item drop */
    PD_AFFIX_MAX
};

/* C4: Boss power types (special abilities) */
enum pd_boss_power_enum {
    PD_POWER_NONE = 0,
    PD_POWER_STRIKE,      /* Power Strike: stun 50% every 8 rounds */
    PD_POWER_AURA,        /* Healing Aura: +5 HP/round */
    PD_POWER_SUMMON,      /* Summon Guardian: spawn sentinel at 50% HP */
    PD_POWER_DODGE,       /* Dodge Stance: +30% dodge for 3 rounds */
    PD_POWER_DRAIN,       /* Life Drain: heal 20% damage */
    PD_POWER_MAX
};

/* Generate a pocket dungeon instance for the given group of players.
 * Picks a random seed if theme is NULL or empty.
 * Returns the instance on success, NULL on failure. */
PD_INSTANCE_T *pd_generate_instance(CHAR_T **members, int member_count,
                                     const char *theme);

/* Destroy a pocket dungeon instance: purge all mobs/objs from its rooms,
 * remove the return portal, free all rooms, and free the area. */
void pd_destroy_instance(PD_INSTANCE_T *inst);

/* Find a seed by name.  Returns NULL if not found. */
PD_SEED_T *pd_seed_get_by_name(const char *name);

/* Check all active instances for empty+timeout; destroy expired ones. */
void pd_update_all(void);

/* Return the active instance a character belongs to, or NULL. */
PD_INSTANCE_T *pd_find_instance_for_char(const CHAR_T *ch);

/* Scan all instances for one that lists the given player name as a member.
 * Used by 'dungeon rejoin' to let dying players re-enter their instance. */
PD_INSTANCE_T *pd_find_instance_by_member(const char *name);

/* B5: Validate that a seed has required data.
 * Returns TRUE if valid, FALSE if invalid (seed skipped with error log). */
bool pd_validate_seed(PD_SEED_T *seed);

/* Write / delete the debug snapshot file for an instance. */
void pd_write_snapshot(PD_INSTANCE_T *inst);
void pd_delete_snapshot(PD_INSTANCE_T *inst);
void pd_cleanup_orphaned_snapshots(void);

/* Scan for hidden objects in current room and reveal them based on skill roll.
 * Used by 'search' command and spell_detect_hidden. */
void pd_do_hidden_scan(CHAR_T *ch);

/* C1: Apply random affixes to instance (called during generation). */
void pd_apply_affixes(PD_INSTANCE_T *inst);

/* C2: Update difficulty based on rooms cleared (call during mob spawn). */
int pd_get_difficulty_bonus(PD_INSTANCE_T *inst);

/* C3: Trigger boss loot drops (call when boss dies). */
void pd_trigger_boss_loot(PD_INSTANCE_T *inst, ROOM_INDEX_T *room);

/* C4: Assign random powers to boss (call before boss spawns). */
void pd_assign_boss_powers(PD_INSTANCE_T *inst);

#endif /* POCKET_DUNGEON_H */
