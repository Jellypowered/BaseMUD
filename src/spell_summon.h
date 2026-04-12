/***************************************************************************
 *  BaseMUD - Original Code                                                 *
 *                                                                         *
 *  Summoning spells: Find Familiar, Mount, Animate Dead.                  *
 *  Implemented based on the John Lin spell list.                          *
 ***************************************************************************/

#ifndef __ROM_SPELL_SUMMON_H
#define __ROM_SPELL_SUMMON_H

#include "merc.h"

/* Vnum ranges for summoned creatures (area: summons, min_vnum 30000). */
#define MOB_VNUM_MOUNT_MIN      30001
#define MOB_VNUM_MOUNT_MAX      30005
#define MOB_VNUM_FAMILIAR_MIN   30010
#define MOB_VNUM_FAMILIAR_MAX   30017

/* Object vnums for spell constructs. */
#define OBJ_VNUM_MAGIC_MOUTH    30025
#define OBJ_VNUM_EXPLOSIVE_RUNE 30026

/* Number of familiar types available. */
#define FAMILIAR_TYPE_COUNT 8

DECLARE_SPELL_FUN (spell_find_familiar);
DECLARE_SPELL_FUN (spell_mount);
DECLARE_SPELL_FUN (spell_animate_dead);

#endif
