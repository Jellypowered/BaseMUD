/***************************************************************************
 * Pocket Dungeon — instanced procedural area generator.
 * pocket_dungeon.h — public interface.
 ***************************************************************************/
#ifndef POCKET_DUNGEON_H
#define POCKET_DUNGEON_H

#include "merc.h"

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

/* Write / delete the debug snapshot file for an instance. */
void pd_write_snapshot(PD_INSTANCE_T *inst);
void pd_delete_snapshot(PD_INSTANCE_T *inst);

#endif /* POCKET_DUNGEON_H */
