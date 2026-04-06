/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
 *  ROM has been brought to you by the ROM consortium                      *
 *      Russ Taylor (rtaylor@hypercube.org)                                *
 *      Gabrielle Taylor (gtaylor@hypercube.org)                           *
 *      Brian Moore (zump@rom.org)                                         *
 *  By using this code, you have agreed to follow the terms of the         *
 *  ROM license, in the file Rom24/doc/rom.license                         *
 ***************************************************************************/

#ifndef __ROM_TABLES_H
#define __ROM_TABLES_H

#include "merc.h"

#include "defs.h"
#include "types.h"

/* A table containing every table of every type. */
extern const TABLE_T master_table[TABLE_MAX + 1];

/* All tables. */
extern CLAN_T clan_table[CLAN_MAX + 1];
extern POSITION_T position_table[POS_MAX + 1];
extern SEX_T sex_table[SEX_MAX + 1];
extern SIZE_T size_table[SIZE_MAX_R + 1];
extern ITEM_T item_table[ITEM_MAX + 1];
extern DAM_T dam_table[DAM_MAX + 1];
extern ATTACK_T attack_table[ATTACK_MAX + 1];
extern PC_RACE_T *pc_race_table;
extern int pc_race_count, pc_race_cap;
extern CLASS_T *class_table;
extern int class_count, class_cap;
extern STR_APP_T str_app_table[ATTRIBUTE_HIGHEST + 2];
extern INT_APP_T int_app_table[ATTRIBUTE_HIGHEST + 2];
extern WIS_APP_T wis_app_table[ATTRIBUTE_HIGHEST + 2];
extern DEX_APP_T dex_app_table[ATTRIBUTE_HIGHEST + 2];
extern CON_APP_T con_app_table[ATTRIBUTE_HIGHEST + 2];
extern LIQ_T *liq_table;
extern int liq_count, liq_cap;
extern SKILL_GROUP_T *skill_group_table;
extern int skill_group_count, skill_group_cap;
extern SECTOR_T sector_table[SECT_MAX + 1];
extern DOOR_T door_table[DIR_MAX + 1];
extern SPEC_T spec_table[SPEC_MAX + 1];
extern WEAR_LOC_T wear_loc_table[WEAR_LOC_MAX + 2];
extern MATERIAL_T *material_table;
extern int material_count, material_cap;
extern COLOUR_SETTING_T colour_setting_table[COLOUR_SETTING_MAX + 1];
extern DAY_T day_table[DAY_MAX + 1];
extern MONTH_T month_table[MONTH_MAX + 1];
extern SKY_T sky_table[SKY_MAX + 1];
extern SUN_T sun_table[SUN_MAX + 1];
extern HP_COND_T hp_cond_table[HP_COND_MAX + 1];
extern COLOUR_T colour_table[COLOUR_MAX + 1];
extern POSE_T *pose_table;
extern int pose_count, pose_cap;
extern WEAPON_T weapon_table[WEAPON_MAX + 1];
extern SKILL_T *skill_table;
extern int skill_count, skill_cap;
extern BOARD_T board_table[BOARD_MAX + 1];
extern SONG_T song_table[MAX_SONGS + 1];
extern RACE_T *race_table;
extern int race_count, race_cap;

/* Internal tables. */
extern const AFFECT_BIT_T affect_bit_table[AFF_TO_MAX + 1];
extern const EFFECT_T effect_table[EFFECT_MAX + 1];
extern const FURNITURE_BITS_T furniture_table[POS_MAX + 1];
extern const MAP_LOOKUP_TABLE_T map_flags_table[MAP_LOOKUP_MAX + 1];
extern const MAP_LOOKUP_TABLE_T map_lookup_table[MAP_LOOKUP_MAX + 1];
extern const NANNY_HANDLER_T nanny_table[NANNY_MAX + 1];
extern const OBJ_MAP_T obj_map_table[ITEM_MAX + 1];
extern SKILL_MAP_T skill_map_table[SKILL_MAP_MAX + 1];
extern RECYCLE_T recycle_table[RECYCLE_MAX + 1];
extern const WIZNET_T wiznet_table[WIZNET_MAX + 1];
extern COND_T cond_table[COND_MAX + 1];
extern const TRAIN_STAT_T train_stat_table[TRAIN_STAT_MAX + 1];
extern const HEAL_SPELL_T heal_spell_table[HEAL_SPELL_MAX + 1];

/* Other tables. */
extern char *const title_table[CLASS_MAX][MAX_LEVEL + 1][2];

/* Table management functions. */
void table_dispose_all(void);
void table_dispose(const TABLE_T *table);

/* Init / disposal functions. */
DECLARE_DISPOSE_FUN(attack_dispose);
DECLARE_DISPOSE_FUN(board_dispose);
DECLARE_DISPOSE_FUN(clan_dispose);
DECLARE_DISPOSE_FUN(class_dispose);
DECLARE_DISPOSE_FUN(colour_dispose);
DECLARE_DISPOSE_FUN(colour_setting_dispose);
DECLARE_DISPOSE_FUN(cond_dispose);
DECLARE_DISPOSE_FUN(dam_dispose);
DECLARE_DISPOSE_FUN(day_dispose);
DECLARE_DISPOSE_FUN(door_dispose);
DECLARE_DISPOSE_FUN(hp_cond_dispose);
DECLARE_DISPOSE_FUN(item_dispose);
DECLARE_DISPOSE_FUN(liq_dispose);
DECLARE_DISPOSE_FUN(material_dispose);
DECLARE_DISPOSE_FUN(month_dispose);
DECLARE_DISPOSE_FUN(pc_race_dispose);
DECLARE_DISPOSE_FUN(pose_dispose);
DECLARE_DISPOSE_FUN(position_dispose);
DECLARE_DISPOSE_FUN(race_dispose);
DECLARE_DISPOSE_FUN(sector_dispose);
DECLARE_DISPOSE_FUN(sex_dispose);
DECLARE_DISPOSE_FUN(size_dispose);
DECLARE_DISPOSE_FUN(skill_dispose);
DECLARE_DISPOSE_FUN(skill_group_dispose);
DECLARE_DISPOSE_FUN(spec_dispose);
DECLARE_DISPOSE_FUN(spec_dispose);
DECLARE_DISPOSE_FUN(sky_dispose);
DECLARE_DISPOSE_FUN(song_dispose);
DECLARE_DISPOSE_FUN(sun_dispose);
DECLARE_DISPOSE_FUN(weapon_dispose);
DECLARE_DISPOSE_FUN(wear_loc_dispose);

DECLARE_POST_LOAD_FUN(cond_reload_mapping);
DECLARE_POST_LOAD_FUN(spec_reload_mapping);

#endif
