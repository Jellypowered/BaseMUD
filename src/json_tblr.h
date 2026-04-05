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

#ifndef __ROM_JSON_TBLR_H
#define __ROM_JSON_TBLR_H

#include "merc.h"

/* dead-simple functions for reading JSON objects. */
DECLARE_JSON_READ_FUN (json_tblr_attack);
DECLARE_JSON_READ_FUN (json_tblr_clan);
DECLARE_JSON_READ_FUN (json_tblr_class);
DECLARE_JSON_READ_FUN (json_tblr_con_app);
DECLARE_JSON_READ_FUN (json_tblr_dam);
DECLARE_JSON_READ_FUN (json_tblr_day);
DECLARE_JSON_READ_FUN (json_tblr_dex_app);
DECLARE_JSON_READ_FUN (json_tblr_hp_cond);
DECLARE_JSON_READ_FUN (json_tblr_int_app);
DECLARE_JSON_READ_FUN (json_tblr_item);
DECLARE_JSON_READ_FUN (json_tblr_liq);
DECLARE_JSON_READ_FUN (json_tblr_month);
DECLARE_JSON_READ_FUN (json_tblr_pc_race);
DECLARE_JSON_READ_FUN (json_tblr_position);
DECLARE_JSON_READ_FUN (json_tblr_race);
DECLARE_JSON_READ_FUN (json_tblr_sex);
DECLARE_JSON_READ_FUN (json_tblr_size);
DECLARE_JSON_READ_FUN (json_tblr_skill);
DECLARE_JSON_READ_FUN (json_tblr_skill_group);
DECLARE_JSON_READ_FUN (json_tblr_sky);
DECLARE_JSON_READ_FUN (json_tblr_song);
DECLARE_JSON_READ_FUN (json_tblr_str_app);
DECLARE_JSON_READ_FUN (json_tblr_sun);
DECLARE_JSON_READ_FUN (json_tblr_weapon);
DECLARE_JSON_READ_FUN (json_tblr_wis_app);

#endif
