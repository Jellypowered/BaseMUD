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

#include "json_tblr.h"

#include "json.h"
#include "json_import.h"
#include "lookup.h"
#include "magic.h"
#include "memory.h"
#include "spell_dispatch.h"
#include "tables.h"
#include "types.h"

#include <string.h>

#define JSON_TBLR_START(vtype, var, max, null_check) \
    vtype *var; \
    \
    int index = 0; \
    for (index = 0; index < max; index++) { \
        var = &(var ## _table[index]); \
        if (null_check) \
            break; \
    } \
    if (index == max) { \
        json_logf (json, "Too many '%s' objects.\n", obj_name); \
        return NULL; \
    } \
    var = &(var ## _table[index]);

DEFINE_JSON_READ_FUN (json_tblr_str_app) {
    int stat_val;
    STR_APP_T *str_app;

    if (!json_import_expect ("str_app", json,
            "stat", "hitroll_bonus", "damroll_bonus",
            "carry_bonus", "max_wield_weight", NULL))
        return NULL;
    stat_val = JGI("stat");
    if (stat_val < 0 || stat_val > ATTRIBUTE_HIGHEST) {
        json_logf (json, "str_app stat '%d' out of range.", stat_val);
        return NULL;
    }
    str_app = &str_app_table[stat_val];
    str_app->stat  = stat_val;
    str_app->tohit = JGI("hitroll_bonus");
    str_app->todam = JGI("damroll_bonus");
    str_app->carry = JGI("carry_bonus");
    str_app->wield = JGI("max_wield_weight");
    return str_app;
}

DEFINE_JSON_READ_FUN (json_tblr_int_app) {
    int stat_val;
    INT_APP_T *int_app;

    if (!json_import_expect ("int_app", json,
            "stat", "learn_rate", NULL))
        return NULL;
    stat_val = JGI("stat");
    if (stat_val < 0 || stat_val > ATTRIBUTE_HIGHEST) {
        json_logf (json, "int_app stat '%d' out of range.", stat_val);
        return NULL;
    }
    int_app = &int_app_table[stat_val];
    int_app->stat  = stat_val;
    int_app->learn = JGI("learn_rate");
    return int_app;
}

DEFINE_JSON_READ_FUN (json_tblr_wis_app) {
    int stat_val;
    WIS_APP_T *wis_app;

    if (!json_import_expect ("wis_app", json,
            "stat", "practices", NULL))
        return NULL;
    stat_val = JGI("stat");
    if (stat_val < 0 || stat_val > ATTRIBUTE_HIGHEST) {
        json_logf (json, "wis_app stat '%d' out of range.", stat_val);
        return NULL;
    }
    wis_app = &wis_app_table[stat_val];
    wis_app->stat     = stat_val;
    wis_app->practice = JGI("practices");
    return wis_app;
}

DEFINE_JSON_READ_FUN (json_tblr_dex_app) {
    int stat_val;
    DEX_APP_T *dex_app;

    if (!json_import_expect ("dex_app", json,
            "stat", "defense_bonus", NULL))
        return NULL;
    stat_val = JGI("stat");
    if (stat_val < 0 || stat_val > ATTRIBUTE_HIGHEST) {
        json_logf (json, "dex_app stat '%d' out of range.", stat_val);
        return NULL;
    }
    dex_app = &dex_app_table[stat_val];
    dex_app->stat      = stat_val;
    dex_app->defensive = JGI("defense_bonus");
    return dex_app;
}

DEFINE_JSON_READ_FUN (json_tblr_con_app) {
    int stat_val;
    CON_APP_T *con_app;

    if (!json_import_expect ("con_app", json,
            "stat", "level_hp", "shock", NULL))
        return NULL;
    stat_val = JGI("stat");
    if (stat_val < 0 || stat_val > ATTRIBUTE_HIGHEST) {
        json_logf (json, "con_app stat '%d' out of range.", stat_val);
        return NULL;
    }
    con_app = &con_app_table[stat_val];
    con_app->stat  = stat_val;
    con_app->hitp  = JGI("level_hp");
    con_app->shock = JGI("shock");
    return con_app;
}

DEFINE_JSON_READ_FUN (json_tblr_attack) {
    char buf[MAX_STRING_LENGTH];
    const char *dam_name;
    JSON_TBLR_START (ATTACK_T, attack, ATTACK_MAX, attack->name == NULL);
    if (!json_import_expect ("attack", json,
            "name", "noun", "*dam_type", NULL))
        return NULL;
    READ_PROP_STRP (attack->name, "name");
    READ_PROP_STRP (attack->noun, "noun");
    attack->dam_type = -1;
    if ((dam_name = JGS ("dam_type")) != NULL)
        attack->dam_type = dam_lookup_exact (dam_name);
    return attack;
}

DEFINE_JSON_READ_FUN (json_tblr_clan) {
    char buf[MAX_STRING_LENGTH];
    const char *s;
    JSON_TBLR_START (CLAN_T, clan, CLAN_MAX, clan->name == NULL);
    if (!json_import_expect ("clan", json,
            "name", "who_name", "hall", "independent", NULL))
        return NULL;
    s = JGS ("name");
    str_replace_dup (&clan->name,     s != NULL ? s : "");
    s = JGS ("who_name");
    str_replace_dup (&clan->who_name, s != NULL ? s : "");
    clan->hall        = JGI ("hall");
    clan->independent = JGB ("independent");
    return clan;
}

DEFINE_JSON_READ_FUN (json_tblr_item) {
    char buf[MAX_STRING_LENGTH];
    JSON_TBLR_START (ITEM_T, item, ITEM_MAX, item->name == NULL);
    if (!json_import_expect ("item", json, "type", "name", NULL))
        return NULL;
    item->type = JGI ("type");
    READ_PROP_STRP (item->name, "name");
    return item;
}

DEFINE_JSON_READ_FUN (json_tblr_sex) {
    char buf[MAX_STRING_LENGTH];
    JSON_TBLR_START (SEX_T, sex, SEX_MAX, sex->name == NULL);
    if (!json_import_expect ("sex", json, "sex", "name", NULL))
        return NULL;
    sex->sex = JGI ("sex");
    READ_PROP_STRP (sex->name, "name");
    return sex;
}

DEFINE_JSON_READ_FUN (json_tblr_size) {
    char buf[MAX_STRING_LENGTH];
    JSON_TBLR_START (SIZE_T, size, SIZE_MAX_R, size->name == NULL);
    if (!json_import_expect ("size", json, "size", "name", NULL))
        return NULL;
    size->size = JGI ("size");
    READ_PROP_STRP (size->name, "name");
    return size;
}

DEFINE_JSON_READ_FUN (json_tblr_day) {
    char buf[MAX_STRING_LENGTH];
    JSON_TBLR_START (DAY_T, day, DAY_MAX, day->name == NULL);
    if (!json_import_expect ("day", json, "index", "name", NULL))
        return NULL;
    day->type = JGI("index");
    READ_PROP_STRP (day->name, "name");
    return day;
}

DEFINE_JSON_READ_FUN (json_tblr_month) {
    char buf[MAX_STRING_LENGTH];
    JSON_TBLR_START (MONTH_T, month, MONTH_MAX, month->name == NULL);
    if (!json_import_expect ("month", json, "index", "name", NULL))
        return NULL;
    month->type = JGI("index");
    READ_PROP_STRP (month->name, "name");
    return month;
}

DEFINE_JSON_READ_FUN (json_tblr_sky) {
    char buf[MAX_STRING_LENGTH];
    JSON_TBLR_START (SKY_T, sky, SKY_MAX, sky->name == NULL);
    if (!json_import_expect ("sky", json,
            "index", "name", "description", "mmhg_min", "mmhg_max", NULL))
        return NULL;
    sky->type = JGI("index");
    READ_PROP_STRP (sky->name,        "name");
    READ_PROP_STRP (sky->description, "description");
    sky->mmhg_min = JGI("mmhg_min");
    sky->mmhg_max = JGI("mmhg_max");
    return sky;
}

DEFINE_JSON_READ_FUN (json_tblr_sun) {
    char buf[MAX_STRING_LENGTH];
    JSON_TBLR_START (SUN_T, sun, SUN_MAX, sun->name == NULL);
    if (!json_import_expect ("sun", json,
            "index", "name", "is_dark", "hour_start", "hour_end", "message", NULL))
        return NULL;
    sun->type       = JGI("index");
    READ_PROP_STRP (sun->name,    "name");
    READ_PROP_BOOL (sun->is_dark, "is_dark");
    sun->hour_start = JGI("hour_start");
    sun->hour_end   = JGI("hour_end");
    READ_PROP_STRP (sun->message, "message");
    return sun;
}

DEFINE_JSON_READ_FUN (json_tblr_skill) {
    char buf[MAX_STRING_LENGTH];
    JSON_T *array, *sub, *sub2;

    JSON_TBLR_START (SKILL_T, skill, SKILL_MAX, skill->name == NULL);

    if (!json_import_expect ("skill", json,
            "name", "*classes",
            "target", "min_position",
            "*slot", "*min_mana", "usage_beats",
            "*damage_noun", "*off_msg_char", "*off_msg_obj",
            "*spell_fun", NULL))
        return NULL;

    READ_PROP_STRP (skill->name, "name");

    if ((array = json_get (json, "classes")) != NULL) {
        for (sub = array->first_child; sub != NULL; sub = sub->next) {
            int num;
            if ((num = class_lookup_exact (sub->name)) < 0) {
                json_logf (json, "Unknown class '%s'", sub->name);
                continue;
            }
            if ((sub2 = json_get (sub, "level")) != NULL)
                skill->classes[num].level = json_value_as_int (sub2);
            if ((sub2 = json_get (sub, "effort")) != NULL)
                skill->classes[num].effort = json_value_as_int (sub2);
        }
    }

    READ_PROP_TYPE  (skill->target,           "target",       skill_target_types);
    READ_PROP_TYPE  (skill->minimum_position, "min_position", position_types);
    READ_PROP_INT   (skill->slot,             "slot");
    READ_PROP_INT   (skill->min_mana,         "min_mana");
    READ_PROP_INT   (skill->beats,            "usage_beats");

    /* Optional string fields — only present for spells */
    {
        JSON_T *node;
        if ((node = json_get (json, "damage_noun")) != NULL)
            str_replace_dup (&(skill->noun_damage),
                json_value_as_string (node, buf, sizeof (buf)));
        if ((node = json_get (json, "off_msg_char")) != NULL)
            str_replace_dup (&(skill->msg_off),
                json_value_as_string (node, buf, sizeof (buf)));
        if ((node = json_get (json, "off_msg_obj")) != NULL)
            str_replace_dup (&(skill->msg_obj),
                json_value_as_string (node, buf, sizeof (buf)));
    }

    /* Resolve spell function by name (NULL / absent → spell_null) */
    {
        JSON_T *node = json_get (json, "spell_fun");
        if (node != NULL) {
            json_value_as_string (node, buf, sizeof (buf));
            skill->spell_fun = spell_lookup_function (buf);
            if (skill->spell_fun == NULL)
                json_logf (json, "Unknown spell_fun '%s' for skill '%s'",
                    buf, skill->name);
        }
        if (skill->spell_fun == NULL)
            skill->spell_fun = spell_null;
    }

    return skill;
}

DEFINE_JSON_READ_FUN (json_tblr_class) {
    char buf[MAX_STRING_LENGTH];
    JSON_T *array, *sub;
    int i;

    JSON_TBLR_START (CLASS_T, class, CLASS_MAX, class->name == NULL);

    if (!json_import_expect ("class", json,
            "name", "who_name", "primary_stat",
            "weapon", "guild",
            "skill_adept", "thac0_00", "thac0_32",
            "hp_gain_min", "hp_gain_max",
            "gains_mana", "base_group", "default_group",
            "can_sneak_away", NULL))
        return NULL;

    READ_PROP_STRP (class->name,         "name");
    READ_PROP_STR  (class->who_name,     "who_name");
    READ_PROP_TYPE (class->attr_prime,   "primary_stat", stat_types);
    READ_PROP_INT  (class->weapon,       "weapon");
    if ((array = json_get (json, "guild")) != NULL) {
        i = 0;
        for (sub = array->first_child; sub != NULL && i < MAX_GUILD;
                sub = sub->next)
            class->guild[i++] = json_value_as_int (sub);
    }
    READ_PROP_INT  (class->skill_adept,  "skill_adept");
    READ_PROP_INT  (class->thac0_00,     "thac0_00");
    READ_PROP_INT  (class->thac0_32,     "thac0_32");
    READ_PROP_INT  (class->hp_min,       "hp_gain_min");
    READ_PROP_INT  (class->hp_max,       "hp_gain_max");
    READ_PROP_BOOL (class->gains_mana,   "gains_mana");
    READ_PROP_STRP (class->base_group,   "base_group");
    READ_PROP_STRP (class->default_group, "default_group");
    READ_PROP_BOOL (class->can_sneak_away, "can_sneak_away");

    return class;
}

DEFINE_JSON_READ_FUN (json_tblr_pc_race) {
    char buf[MAX_STRING_LENGTH];
    JSON_T *array, *sub;
    int i;

    JSON_TBLR_START (PC_RACE_T, pc_race, PC_RACE_MAX, pc_race->name == NULL);

    if (!json_import_expect ("pc_race", json,
            "name", "who_name", "creation_points",
            "base_stats", "max_stats", "size",
            "*class_exp", "*skills", "*bonus_max_stat", NULL))
        return NULL;

    READ_PROP_STRP (pc_race->name,     "name");
    READ_PROP_STR  (pc_race->who_name, "who_name");
    READ_PROP_INT  (pc_race->creation_points, "creation_points");

    for (i = 0; class_get (i) != NULL; i++)
        pc_race->class_mult[i] = 100;
    if ((array = json_get (json, "class_exp")) != NULL) {
        for (sub = array->first_child; sub != NULL; sub = sub->next) {
            int num;
            if ((num = class_lookup_exact (sub->name)) < 0) {
                json_logf (json, "Unknown class '%s'", sub->name);
                continue;
            }
            pc_race->class_mult[num] = json_value_as_int (sub);
        }
    }

    if ((array = json_get (json, "skills")) != NULL) {
        int num = 0;
        for (sub = array->first_child; sub != NULL; sub = sub->next)
            pc_race->skills[num++] = str_dup (
                json_value_as_string (sub, buf, sizeof (buf)));
    }

    array = json_get (json, "base_stats");
    for (sub = array->first_child; sub != NULL; sub = sub->next) {
        int stat;
        if ((stat = type_lookup_exact (stat_types, sub->name)) < 0) {
            json_logf (json, "Unknown stat '%s'", sub->name);
            continue;
        }
        pc_race->stats[stat] = json_value_as_int (sub);
    }

    array = json_get (json, "max_stats");
    for (sub = array->first_child; sub != NULL; sub = sub->next) {
        int stat;
        if ((stat = type_lookup_exact (stat_types, sub->name)) < 0) {
            json_logf (json, "Unknown stat '%s'", sub->name);
            continue;
        }
        pc_race->max_stats[stat] = json_value_as_int (sub);
    }

    READ_PROP_INT (pc_race->bonus_max, "bonus_max_stat");

    READ_PROP_STR (buf, "size");
    pc_race->size = lookup_func_backup (size_lookup_exact, buf,
        "Unknown size '%s'", SIZE_MEDIUM);

    return pc_race;
}

DEFINE_JSON_READ_FUN (json_tblr_race) {
    char buf[MAX_STRING_LENGTH];
    JSON_TBLR_START (RACE_T, race, RACE_MAX, race->name == NULL);

    if (!json_import_expect ("race", json,
            "name",
            "*mob_flags", "*affect_flags", "*offense_flags", "*immune_flags",
            "*res_flags", "*vuln_flags",   "*form",          "*parts",
            NULL))
        return NULL;

    READ_PROP_STRP (race->name, "name");
    READ_PROP_EXT_FLAGS (race->ext_mob, "mob_flags", mob_flags);
    READ_PROP_FLAGS (race->aff,   "affect_flags",  affect_flags);
    READ_PROP_FLAGS (race->off,   "offense_flags", off_flags);
    READ_PROP_FLAGS (race->imm,   "immune_flags",  res_flags);
    READ_PROP_FLAGS (race->res,   "res_flags",     res_flags);
    READ_PROP_FLAGS (race->vuln,  "vuln_flags",    res_flags);
    READ_PROP_FLAGS (race->form,  "form",          form_flags);
    READ_PROP_FLAGS (race->parts, "parts",         part_flags);

    return race;
}

DEFINE_JSON_READ_FUN (json_tblr_skill_group) {
    char buf[MAX_STRING_LENGTH];
    JSON_T *array, *sub, *cost_node;
    int i;

    JSON_TBLR_START (SKILL_GROUP_T, skill_group, SKILL_GROUP_MAX, skill_group->name == NULL);

    if (!json_import_expect ("skill_group", json,
            "name", "*classes", "*skills", NULL))
        return NULL;

    READ_PROP_STRP (skill_group->name, "name");

    /* initialize all class costs to -1 (unavailable) */
    for (i = 0; class_get (i) != NULL; i++)
        skill_group->classes[i].cost = -1;

    if ((array = json_get (json, "classes")) != NULL) {
        for (sub = array->first_child; sub != NULL; sub = sub->next) {
            int num;
            if ((num = class_lookup_exact (sub->name)) < 0) {
                json_logf (json, "Unknown class '%s'", sub->name);
                continue;
            }
            cost_node = json_get (sub, "cost");
            skill_group->classes[num].cost = (cost_node != NULL)
                ? json_value_as_int (cost_node) : 0;
        }
    }

    if ((array = json_get (json, "skills")) != NULL) {
        i = 0;
        for (sub = array->first_child; sub != NULL && i < MAX_IN_GROUP;
                sub = sub->next)
            skill_group->spells[i++] = str_dup (
                json_value_as_string (sub, buf, sizeof (buf)));
    }

    return skill_group;
}

DEFINE_JSON_READ_FUN (json_tblr_song) {
    char buf[256];
    JSON_T *array, *sub;

    JSON_TBLR_START (SONG_T, song, MAX_SONGS, song->name == NULL);

    if (!json_import_expect ("song", json,
            "name", "group", "lyrics", NULL))
        return NULL;

    READ_PROP_STRP (song->name,  "name");
    READ_PROP_STRP (song->group, "group");

    if ((array = json_get (json, "lyrics")) != NULL) {
        for (sub = array->first_child; sub != NULL; sub = sub->next) {
            if (song->lines == MAX_SONG_LINES)
                break;
            json_value_as_string (sub, buf, sizeof (buf));
            song->lyrics[song->lines] = str_dup (buf);
            song->lines++;
        }
    }

    return song;
}
