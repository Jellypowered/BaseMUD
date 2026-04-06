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

#include "tables.h"

#include <stdlib.h>
#include "act_skills.h"
#include "board.h"
#include "chars.h"
#include "colour.h"
#include "effects.h"
#include "json_tblr.h"
#include "json_tblw.h"
#include "lookup.h"
#include "magic.h"
#include "memory.h"
#include "nanny.h"
#include "recycle.h"
#include "skills.h"
#include "special.h"
#include "spell_dispatch.h"
#include "spell_aff.h"
#include "spell_create.h"
#include "spell_cure.h"
#include "spell_info.h"
#include "spell_misc.h"
#include "spell_move.h"
#include "spell_npc.h"
#include "spell_off.h"
#include "utils.h"
#include "globals.h"

#include <stddef.h>

/* Useful macros for defining rows in our master table. */
#define TFLAGS(table, desc)                \
    {table, #table, TABLE_FLAGS, desc,     \
     sizeof(FLAG_T), TABLE_LENGTH_DYNAMIC, \
     NULL, "meta/flags", json_tblw_flag}
#define TXFLAGS(table, desc)                       \
    {table, #table, TABLE_EXT_FLAGS, desc,         \
     sizeof(EXT_FLAG_DEF_T), TABLE_LENGTH_DYNAMIC, \
     NULL, "meta/ext_flags", json_tblw_ext_flag}
#define TTYPES(table, desc)                \
    {table, #table, TABLE_TYPES, desc,     \
     sizeof(TYPE_T), TABLE_LENGTH_DYNAMIC, \
     NULL, "meta/types", json_tblw_type}
#define TTABLE(table, name, desc, obj_name, json_path, jwrite, jread, \
               dispose)                                               \
    {table, name, TABLE_UNIQUE, desc,                                 \
     sizeof(table[0]), sizeof(table) / sizeof(table[0]),              \
     obj_name, json_path, jwrite, jread, dispose, NULL}
#define TTABLE_POSTLOAD(table, name, desc, obj_name, json_path, jwrite, jread, \
                        dispose, post_load)                                    \
    {table, name, TABLE_UNIQUE, desc,                                          \
     sizeof(table[0]), sizeof(table) / sizeof(table[0]),                       \
     obj_name, json_path, jwrite, jread, dispose, post_load}
#define TTABLE_INTERNAL(table, name, desc) \
    {table, name, TABLE_INTERNAL, desc,    \
     sizeof(table[0]), sizeof(table) / sizeof(table[0])}
/* Dynamic table: table_pp points to the global pointer variable; count/cap
 * track current population and allocated capacity respectively. */
#define TTABLE_DYNAMIC(tbl_pp, name, desc, obj_name, json_path, jwrite, jread, \
                       dispose, count_p, cap_p, inv_fun)                       \
    {NULL, name, TABLE_UNIQUE, desc,                                           \
     sizeof(**(tbl_pp)), TABLE_LENGTH_DYNAMIC,                                 \
     obj_name, json_path, jwrite, jread, dispose, NULL,                        \
     (void **)(tbl_pp), count_p, cap_p, inv_fun}

#define TTABLE_DYNAMIC_POSTLOAD(tbl_pp, name, desc, obj_name, json_path, \
                                jwrite, jread, dispose, post_load,       \
                                count_p, cap_p, inv_fun)                 \
    {NULL, name, TABLE_UNIQUE, desc,                                     \
     sizeof(**(tbl_pp)), TABLE_LENGTH_DYNAMIC,                           \
     obj_name, json_path, jwrite, jread, dispose, post_load,             \
     (void **)(tbl_pp), count_p, cap_p, inv_fun}

const TABLE_T master_table[TABLE_MAX + 1] = {
    /* from flags.h */
    TFLAGS(affect_flags, "Mobile affects."),
    TFLAGS(area_flags, "Area attributes."),
    TFLAGS(comm_flags, "Communication channel flags."),
    TFLAGS(container_flags, "Container status."),
    TFLAGS(dam_flags, "Attributes for damage types."),
    TFLAGS(exit_flags, "Exit types."),
    TFLAGS(extra_flags, "Object attributes."),
    TFLAGS(form_flags, "Mobile body form."),
    TFLAGS(furniture_flags, "Flags for furniture."),
    TFLAGS(gate_flags, "Portal gate flags."),
    TFLAGS(mprog_flags, "MobProgram flags."),
    TFLAGS(off_flags, "Mobile offensive behaviour."),
    TFLAGS(part_flags, "Mobile body parts."),
    TFLAGS(res_flags, "Mobile immunity."),
    TFLAGS(room_flags, "Room attributes."),
    TFLAGS(weapon_flags, "Special weapon type."),
    TFLAGS(wear_flags, "Types of wear locations."),

    /* from ext_flags.h */
    TXFLAGS(mob_flags, "Mobile flags."),
    TXFLAGS(plr_flags, "Player flags."),

    /* from types.h */
    TTYPES(ac_types, "AC for different attacks."),
    TTYPES(affect_apply_types, "Affect apply types."),
    TTYPES(board_def_types, "Types of boards."),
    TTYPES(door_reset_types, "Door reset types."),
    TTYPES(item_types, "Types of objects."),
    TTYPES(position_types, "Mobile positions."),
    TTYPES(sector_types, "Sector types, terrain."),
    TTYPES(sex_types, "Sexes."),
    TTYPES(size_types, "Mobile sizes."),
    TTYPES(skill_target_types, "Targets for skills and spells."),
    TTYPES(stat_types, "Available stats for characters."),
    TTYPES(weapon_types, "Weapon classes."),

    /* tables that are currently supported for reading. */
    TTABLE_DYNAMIC(&pc_race_table, "pc_races", "Playable race data.", "player_race", "config", json_tblw_pc_race, json_tblr_pc_race, pc_race_dispose, &pc_race_count, &pc_race_cap, pc_race_invalidate_max),
    TTABLE_DYNAMIC(&race_table, "races", "Races and statistics.", "race", "config", json_tblw_race, json_tblr_race, race_dispose, &race_count, &race_cap, race_invalidate_max),
    TTABLE(song_table, "songs", "Songs for jukeboxes.", "song", "config", json_tblw_song, json_tblr_song, song_dispose),

    /* tables that are written but not yet read. */
    /* TODO: read all of these! */
    TTABLE(attack_table, "attacks", "Attack types and properties.", "attack", "config", json_tblw_attack, json_tblr_attack, attack_dispose),
    TTABLE(board_table, "boards", "Discussion boards.", "board", "config", json_tblw_board, json_tblr_board, board_dispose),
    TTABLE(clan_table, "clans", "Player clans.", "clan", "config", json_tblw_clan, json_tblr_clan, clan_dispose),
    TTABLE_DYNAMIC(&class_table, "classes", "Classes and statistics.", "class", "config", json_tblw_class, json_tblr_class, class_dispose, &class_count, &class_cap, class_invalidate_max),
    TTABLE(colour_setting_table, "color_settings", "Configurable colours.", "color_setting", "config", json_tblw_colour_setting, json_tblr_colour_setting, colour_setting_dispose),
    TTABLE(colour_table, "colors", "Colour values.", "color", "config", json_tblw_colour, json_tblr_colour, colour_dispose),
    TTABLE(con_app_table, "con_app", "Con apply table.", "con_app", "config", json_tblw_con_app, json_tblr_con_app, NULL),
    TTABLE_POSTLOAD(cond_table, "conds", "Conditions like thirst/hunger.", "cond", "config", json_tblw_cond, json_tblr_cond, cond_dispose, cond_reload_mapping),
    TTABLE(dam_table, "dam_types", "Damage types and properties.", "dam_type", "config", json_tblw_dam, json_tblr_dam, dam_dispose),
    TTABLE(day_table, "days", "Days of the week.", "day", "config", json_tblw_day, json_tblr_day, day_dispose),
    TTABLE(dex_app_table, "dex_app", "Dex apply table.", "dex_app", "config", json_tblw_dex_app, json_tblr_dex_app, NULL),
    TTABLE(door_table, "doors", "Exit names.", "door", "config", json_tblw_door, json_tblr_door, door_dispose),
    TTABLE_DYNAMIC_POSTLOAD(&greeting_table, "greetings", "Login greeting messages.", "greeting", "config", json_tblw_greeting, json_tblr_greeting, greeting_dispose, greeting_reload_mapping, &greeting_count, &greeting_cap, NULL),
    TTABLE(hp_cond_table, "hp_conds", "Messages based on % of hp.", "hp_cond", "config", json_tblw_hp_cond, json_tblr_hp_cond, hp_cond_dispose),
    TTABLE(int_app_table, "int_app", "Int apply table.", "int_app", "config", json_tblw_int_app, json_tblr_int_app, NULL),
    TTABLE(item_table, "items", "Item types and properties.", "item", "config", json_tblw_item, json_tblr_item, item_dispose),
    TTABLE_DYNAMIC(&liq_table, "liquids", "Liquid types.", "liquid", "config", json_tblw_liq, json_tblr_liq, liq_dispose, &liq_count, &liq_cap, liq_invalidate_max),
    TTABLE_DYNAMIC(&material_table, "materials", "Material properties", "material", "config", json_tblw_material, json_tblr_material, material_dispose, &material_count, &material_cap, NULL),
    TTABLE(month_table, "months", "Months of the year.", "month", "config", json_tblw_month, json_tblr_month, month_dispose),
    TTABLE_DYNAMIC(&pose_table, "pose", "Poses based on class and level", "pose", "config", json_tblw_pose, json_tblr_pose, pose_dispose, &pose_count, &pose_cap, NULL),
    TTABLE(position_table, "positions", "Character positions.", "position", "config", json_tblw_position, json_tblr_position, position_dispose),
    TTABLE(sector_table, "sectors", "Sector/terrain properties.", "sector", "config", json_tblw_sector, json_tblr_sector, sector_dispose),
    TTABLE(sex_table, "sexes", "Gender settings.", "sex", "config", json_tblw_sex, json_tblr_sex, sex_dispose),
    TTABLE(size_table, "sizes", "Character sizes.", "size", "config", json_tblw_size, json_tblr_size, size_dispose),
    TTABLE_DYNAMIC(&skill_group_table, "skill_groups", "Groups of skills table.", "skill_group", "config", json_tblw_skill_group, json_tblr_skill_group, skill_group_dispose, &skill_group_count, &skill_group_cap, skill_group_invalidate_max),
    TTABLE_DYNAMIC_POSTLOAD(&skill_table, "skills", "Master skill table.", "skill", "config", json_tblw_skill, json_tblr_skill, skill_dispose, skill_reload_mapping, &skill_count, &skill_cap, skill_invalidate_max),
    TTABLE(sky_table, "skies", "Skies based on the weather.", "sky", "config", json_tblw_sky, json_tblr_sky, sky_dispose),
    TTABLE_POSTLOAD(spec_table, "specs", "Specialized mobile behavior.", "spec", "config", json_tblw_spec, json_tblr_spec, spec_dispose, spec_reload_mapping),
    TTABLE(str_app_table, "str_app", "Str apply table.", "str_app", "config", json_tblw_str_app, json_tblr_str_app, NULL),
    TTABLE(sun_table, "suns", "Positions of the sun.", "sun", "config", json_tblw_sun, json_tblr_sun, sun_dispose),
    TTABLE_POSTLOAD(weapon_table, "weapons", "Weapon types and properties.", "weapon", "config", json_tblw_weapon, json_tblr_weapon, weapon_dispose, skill_reload_mapping),
    TTABLE(wear_loc_table, "wear_locs", "Wearable item table.", "wear_loc", "config", json_tblw_wear_loc, json_tblr_wear_loc, wear_loc_dispose),
    TTABLE(wis_app_table, "wis_app", "Wis apply table.", "wis_app", "config", json_tblw_wis_app, json_tblr_wis_app, NULL),

    /* constant tables that are internal only. */
    TTABLE_INTERNAL(affect_bit_table, "affect_bits", "Affect bit vector types."),
    TTABLE_INTERNAL(effect_table, "effects", "Damage effects and breaths."),
    TTABLE_INTERNAL(furniture_table, "furnitures", "Furniture flags for positions."),
    TTABLE_INTERNAL(map_flags_table, "map_flags", "Flags for object mappings."),
    TTABLE_INTERNAL(map_lookup_table, "map_lookups", "Types for object mappings."),
    TTABLE_INTERNAL(nanny_table, "nannies", "Descriptor 'Nanny' table."),
    TTABLE_INTERNAL(obj_map_table, "obj_maps", "Obj type-values[] mappings."),
    TTABLE_INTERNAL(recycle_table, "recyclables", "Recycleable object types."),
    TTABLE_INTERNAL(skill_map_table, "skill_maps", "Internal mappings of skills."),
    TTABLE_INTERNAL(train_stat_table, "train_stats", "List of trainable stats."),
    TTABLE_INTERNAL(wiznet_table, "wiznets", "Wiznet channels."),
    {0}};

void table_dispose_all(void)
{
    const TABLE_T *table;
    int i;

    for (i = 0; master_table[i].name != NULL; i++)
    {
        table = &(master_table[i]);
        if (!table->dispose_fun)
            continue;
        log_f("Freeing table '%s'...", table->name);
        table_dispose(table);
    }
}

void table_dispose(const TABLE_T *table)
{
    int i, count;
    void *obj;

    if (table->dispose_fun == NULL)
        return;

    if (table->table_pp != NULL)
    {
        obj = *table->table_pp;
        count = *table->count_p;
    }
    else
    {
        obj = (void *)table->table;
        count = (int)table->table_length;
    }

    for (i = 0; i < count; i++)
    {
        table->dispose_fun(obj);
        obj += table->type_size;
    }
};

/* for clans */
CLAN_T clan_table[CLAN_MAX + 1] = {
    /* name, who entry, death-transfer room, independent */
    /* independent should be FALSE if is a real clan */
    {"", "", ROOM_VNUM_ALTAR, TRUE},
    {"loner", "[ Loner ] ", ROOM_VNUM_ALTAR, TRUE},
    {"rom", "[  ROM  ] ", ROOM_VNUM_ALTAR, FALSE},
    {0},
};

HP_COND_T hp_cond_table[HP_COND_MAX + 1] = {
#ifdef BASEMUD_MORE_PRECISE_CONDITIONS
    {100, "$1 is in excellent condition."},
    {90, "$1 has a few scratches."},
    {80, "$1 has a few bruises."},
    {70, "$1 has some small wounds and bruises."},
    {60, "$1 has some large wounds."},
    {50, "$1 has quite a large few wounds."},
    {40, "$1 has some big nasty wounds and scratches."},
    {30, "$1 looks seriously wounded."},
    {20, "$1 looks pretty hurt."},
    {10, "$1 is in awful condition."},
    {1, "$1 is in critical condition."},
#else
    {100, "$1 is in excellent condition."},
    {90, "$1 has a few scratches."},
    {75, "$1 has some small wounds and bruises."},
    {50, "$1 has quite a few wounds."},
    {30, "$1 has some big nasty wounds and scratches."},
    {15, "$1 looks pretty hurt."},
    {1, "$1 is in awful condition."},
    {-100, "$1 is bleeding to death."},
#endif
    {-999, NULL}};

/* for position */
POSITION_T position_table[POS_MAX + 1] = {
    {POS_DEAD, "dead", "dead", "$1 is lying here, DEAD!!", NULL},
    {POS_MORTAL, "mortally wounded", "mort", "$1 is lying here, mortally wounded.", NULL},
    {POS_INCAP, "incapacitated", "incap", "$1 is lying here, incapacitated.", NULL},
    {POS_STUNNED, "stunned", "stun", "$1 is lying here, stunned.", NULL},
    {POS_SLEEPING, "sleeping", "sleep", "$1 is sleeping here.", "$1 is sleeping $2 $3."},
    {POS_RESTING, "resting", "rest", "$1 is resting here.", "$1 is resting $2 $3."},
    {POS_SITTING, "sitting", "sit", "$1 is sitting here.", "$1 is sitting $2 $3."},
    {POS_FIGHTING, "fighting", "fight", "$1 is here, fighting $2$3", NULL},
    {POS_STANDING, "standing", "stand", "$1 is standing here.", " is standing $2 $3."},
    {0},
};

/* for sex */
SEX_T sex_table[SEX_MAX + 1] = {
    {SEX_NEUTRAL, "neutral"},
    {SEX_MALE, "male"},
    {SEX_FEMALE, "female"},
    {SEX_EITHER, "either"},
    {0},
};

/* for sizes */
SIZE_T size_table[SIZE_MAX_R + 1] = {
    {SIZE_TINY, "tiny"},
    {SIZE_SMALL, "small"},
    {SIZE_MEDIUM, "medium"},
    {SIZE_LARGE, "large"},
    {
        SIZE_HUGE,
        "huge",
    },
    {SIZE_GIANT, "giant"},
    {0},
};

/* item type list */
ITEM_T item_table[ITEM_MAX + 1] = {
    {ITEM_LIGHT, "light"},
    {ITEM_SCROLL, "scroll"},
    {ITEM_WAND, "wand"},
    {ITEM_STAFF, "staff"},
    {ITEM_WEAPON, "weapon"},
    {ITEM_UNUSED_1, "unused_item_1"},
    {ITEM_UNUSED_2, "unused_item_2"},
    {ITEM_TREASURE, "treasure"},
    {ITEM_ARMOR, "armor"},
    {ITEM_POTION, "potion"},
    {ITEM_CLOTHING, "clothing"},
    {ITEM_FURNITURE, "furniture"},
    {ITEM_TRASH, "trash"},
    {ITEM_UNUSED_3, "unused_item_3"},
    {ITEM_CONTAINER, "container"},
    {ITEM_UNUSED_4, "unused_item_4"},
    {ITEM_DRINK_CON, "drink"},
    {ITEM_KEY, "key"},
    {ITEM_FOOD, "food"},
    {ITEM_MONEY, "money"},
    {ITEM_UNUSED_5, "unused_item_5"},
    {ITEM_BOAT, "boat"},
    {ITEM_CORPSE_NPC, "npc_corpse"},
    {ITEM_CORPSE_PC, "pc_corpse"},
    {ITEM_FOUNTAIN, "fountain"},
    {ITEM_PILL, "pill"},
    {ITEM_PROTECT, "protect"},
    {ITEM_MAP, "map"},
    {ITEM_PORTAL, "portal"},
    {ITEM_WARP_STONE, "warp_stone"},
    {ITEM_ROOM_KEY, "room_key"},
    {ITEM_GEM, "gem"},
    {ITEM_JEWELRY, "jewelry"},
    {ITEM_JUKEBOX, "jukebox"},
    {0}};

/* weapon selection table */
WEAPON_T weapon_table[WEAPON_MAX + 1] = {
    {WEAPON_SWORD, "sword", "sword", OBJ_VNUM_SCHOOL_SWORD},
    {WEAPON_MACE, "mace", "mace", OBJ_VNUM_SCHOOL_MACE},
    {WEAPON_DAGGER, "dagger", "dagger", OBJ_VNUM_SCHOOL_DAGGER},
    {WEAPON_AXE, "axe", "axe", OBJ_VNUM_SCHOOL_AXE},
    {WEAPON_SPEAR, "staff", "staves", OBJ_VNUM_SCHOOL_STAFF},
    {WEAPON_FLAIL, "flail", "flail", OBJ_VNUM_SCHOOL_FLAIL},
    {WEAPON_WHIP, "whip", "whip", OBJ_VNUM_SCHOOL_WHIP},
    {WEAPON_POLEARM, "polearm", "polearm", OBJ_VNUM_SCHOOL_POLEARM},
    {-1, NULL, 0}};

DAM_T dam_table[DAM_MAX + 1] = {
    /* TODO: reference effects by index, not by function directly. */
    {DAM_NONE, "none", 0, EFFECT_NONE, 0},
    {DAM_BASH, "bash", RES_BASH, EFFECT_NONE, 0},
    {DAM_PIERCE, "pierce", RES_PIERCE, EFFECT_NONE, 0},
    {DAM_SLASH, "slash", RES_SLASH, EFFECT_NONE, 0},
    {DAM_FIRE, "fire", RES_FIRE, EFFECT_FIRE, DAM_MAGICAL},
    {DAM_COLD, "cold", RES_COLD, EFFECT_COLD, DAM_MAGICAL},
    {DAM_LIGHTNING, "lightning", RES_LIGHTNING, EFFECT_SHOCK, DAM_MAGICAL},
    {DAM_ACID, "acid", RES_ACID, EFFECT_ACID, DAM_MAGICAL},
    {DAM_POISON, "poison", RES_POISON, EFFECT_POISON, DAM_MAGICAL},
    {DAM_NEGATIVE, "negative", RES_NEGATIVE, EFFECT_NONE, DAM_MAGICAL},
    {DAM_HOLY, "holy", RES_HOLY, EFFECT_NONE, DAM_MAGICAL},
    {DAM_ENERGY, "energy", RES_ENERGY, EFFECT_NONE, DAM_MAGICAL},
    {DAM_MENTAL, "mental", RES_MENTAL, EFFECT_NONE, DAM_MAGICAL},
    {DAM_DISEASE, "disease", RES_DISEASE, EFFECT_NONE, DAM_MAGICAL},
    {DAM_DROWNING, "drowning", RES_DROWNING, EFFECT_NONE, DAM_MAGICAL},
    {DAM_LIGHT, "light", RES_LIGHT, EFFECT_NONE, DAM_MAGICAL},
    {DAM_OTHER, "other", 0, EFFECT_NONE, DAM_MAGICAL},
    {DAM_HARM, "harm", 0, EFFECT_NONE, DAM_MAGICAL},
    {DAM_CHARM, "charm", RES_CHARM, EFFECT_NONE, DAM_MAGICAL},
    {DAM_SOUND, "sound", RES_SOUND, EFFECT_NONE, DAM_MAGICAL},
    {0}};

/* attack table  -- not very organized :( */
ATTACK_T attack_table[ATTACK_MAX + 1] = {
    {"none", "hit", -1}, /*  0 */
    {"slice", "slice", DAM_SLASH},
    {"stab", "stab", DAM_PIERCE},
    {"slash", "slash", DAM_SLASH},
    {"whip", "whip", DAM_SLASH},
    {"claw", "claw", DAM_SLASH}, /*  5 */
    {"blast", "blast", DAM_BASH},
    {"pound", "pound", DAM_BASH},
    {"crush", "crush", DAM_BASH},
    {"grep", "grep", DAM_SLASH},
    {"bite", "bite", DAM_PIERCE}, /* 10 */
    {"pierce", "pierce", DAM_PIERCE},
    {"suction", "suction", DAM_BASH},
    {"beating", "beating", DAM_BASH},
    {"digestion", "digestion", DAM_ACID},
    {"charge", "charge", DAM_BASH}, /* 15 */
    {"slap", "slap", DAM_BASH},
    {"punch", "punch", DAM_BASH},
    {"wrath", "wrath", DAM_ENERGY},
    {"magic", "magic", DAM_ENERGY},
    {"divine", "divine power", DAM_HOLY}, /* 20 */
    {"cleave", "cleave", DAM_SLASH},
    {"scratch", "scratch", DAM_PIERCE},
    {"peck", "peck", DAM_PIERCE},
    {"peckb", "peck", DAM_BASH},
    {"chop", "chop", DAM_SLASH}, /* 25 */
    {"sting", "sting", DAM_PIERCE},
    {"smash", "smash", DAM_BASH},
    {"shbite", "shocking bite", DAM_LIGHTNING},
    {"flbite", "flaming bite", DAM_FIRE},
    {"frbite", "freezing bite", DAM_COLD}, /* 30 */
    {"acbite", "acidic bite", DAM_ACID},
    {"chomp", "chomp", DAM_PIERCE},
    {"drain", "life drain", DAM_NEGATIVE},
    {"thrust", "thrust", DAM_PIERCE},
    {"slime", "slime", DAM_ACID}, /* 35 */
    {"shock", "shock", DAM_LIGHTNING},
    {"thwack", "thwack", DAM_BASH},
    {"flame", "flame", DAM_FIRE},
    {"chill", "chill", DAM_COLD},
    {NULL, NULL, -1}};

RACE_T *race_table = NULL;
int race_count = 0, race_cap = 0;
PC_RACE_T *pc_race_table = NULL;
int pc_race_count = 0, pc_race_cap = 0;

/* Class table - loaded from JSON at boot. */
CLASS_T *class_table = NULL;
int class_count = 0, class_cap = 0;

/* REMOVED: title_table moved to classes.json -> CLASS_T.titles[2] */
#if 0
char *const title_table[CLASS_MAX][MAX_LEVEL + 1][2] = {
    {{"Man", "Woman"},

     {"Apprentice of Magic", "Apprentice of Magic"},
     {"Spell Student", "Spell Student"},
     {"Scholar of Magic", "Scholar of Magic"},
     {"Delver in Spells", "Delveress in Spells"},
     {"Medium of Magic", "Medium of Magic"},

     {"Scribe of Magic", "Scribess of Magic"},
     {"Seer", "Seeress"},
     {"Sage", "Sage"},
     {"Illusionist", "Illusionist"},
     {"Abjurer", "Abjuress"},

     {"Invoker", "Invoker"},
     {"Enchanter", "Enchantress"},
     {"Conjurer", "Conjuress"},
     {"Magician", "Witch"},
     {"Creator", "Creator"},

     {"Savant", "Savant"},
     {"Magus", "Craftess"},
     {"Wizard", "Wizard"},
     {"Warlock", "War Witch"},
     {"Sorcerer", "Sorceress"},

     {"Elder Sorcerer", "Elder Sorceress"},
     {"Grand Sorcerer", "Grand Sorceress"},
     {"Great Sorcerer", "Great Sorceress"},
     {"Golem Maker", "Golem Maker"},
     {"Greater Golem Maker", "Greater Golem Maker"},

     {
         "Maker of Stones",
         "Maker of Stones",
     },
     {
         "Maker of Potions",
         "Maker of Potions",
     },
     {
         "Maker of Scrolls",
         "Maker of Scrolls",
     },
     {
         "Maker of Wands",
         "Maker of Wands",
     },
     {
         "Maker of Staves",
         "Maker of Staves",
     },

     {"Demon Summoner", "Demon Summoner"},
     {"Greater Demon Summoner", "Greater Demon Summoner"},
     {"Dragon Charmer", "Dragon Charmer"},
     {"Greater Dragon Charmer", "Greater Dragon Charmer"},
     {"Master of all Magic", "Master of all Magic"},

     {"Master Mage", "Master Mage"},
     {"Master Mage", "Master Mage"},
     {"Master Mage", "Master Mage"},
     {"Master Mage", "Master Mage"},
     {"Master Mage", "Master Mage"},

     {"Master Mage", "Master Mage"},
     {"Master Mage", "Master Mage"},
     {"Master Mage", "Master Mage"},
     {"Master Mage", "Master Mage"},
     {"Master Mage", "Master Mage"},

     {"Master Mage", "Master Mage"},
     {"Master Mage", "Master Mage"},
     {"Master Mage", "Master Mage"},
     {"Master Mage", "Master Mage"},
     {"Master Mage", "Master Mage"},

     {"Mage Hero", "Mage Heroine"},
     {"Avatar of Magic", "Avatar of Magic"},
     {"Angel of Magic", "Angel of Magic"},
     {"Demigod of Magic", "Demigoddess of Magic"},
     {"Immortal of Magic", "Immortal of Magic"},
     {"God of Magic", "Goddess of Magic"},
     {"Deity of Magic", "Deity of Magic"},
     {"Supremity of Magic", "Supremity of Magic"},
     {"Creator", "Creator"},
     {"Implementor", "Implementress"}},

    {{"Man", "Woman"},

     {"Believer", "Believer"},
     {"Attendant", "Attendant"},
     {"Acolyte", "Acolyte"},
     {"Novice", "Novice"},
     {"Missionary", "Missionary"},

     {"Adept", "Adept"},
     {"Deacon", "Deaconess"},
     {"Vicar", "Vicaress"},
     {"Priest", "Priestess"},
     {"Minister", "Lady Minister"},

     {"Canon", "Canon"},
     {"Levite", "Levitess"},
     {"Curate", "Curess"},
     {"Monk", "Nun"},
     {"Healer", "Healess"},

     {"Chaplain", "Chaplain"},
     {"Expositor", "Expositress"},
     {"Bishop", "Bishop"},
     {"Arch Bishop", "Arch Lady of the Church"},
     {"Patriarch", "Matriarch"},

     {"Elder Patriarch", "Elder Matriarch"},
     {"Grand Patriarch", "Grand Matriarch"},
     {"Great Patriarch", "Great Matriarch"},
     {"Demon Killer", "Demon Killer"},
     {"Greater Demon Killer", "Greater Demon Killer"},

     {"Cardinal of the Sea", "Cardinal of the Sea"},
     {"Cardinal of the Earth", "Cardinal of the Earth"},
     {"Cardinal of the Air", "Cardinal of the Air"},
     {"Cardinal of the Ether", "Cardinal of the Ether"},
     {"Cardinal of the Heavens", "Cardinal of the Heavens"},

     {"Avatar of an Immortal", "Avatar of an Immortal"},
     {"Avatar of a Deity", "Avatar of a Deity"},
     {"Avatar of a Supremity", "Avatar of a Supremity"},
     {"Avatar of an Implementor", "Avatar of an Implementor"},
     {"Master of all Divinity", "Mistress of all Divinity"},

     {"Master Cleric", "Master Cleric"},
     {"Master Cleric", "Master Cleric"},
     {"Master Cleric", "Master Cleric"},
     {"Master Cleric", "Master Cleric"},
     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},
     {"Master Cleric", "Master Cleric"},
     {"Master Cleric", "Master Cleric"},
     {"Master Cleric", "Master Cleric"},
     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},
     {"Master Cleric", "Master Cleric"},
     {"Master Cleric", "Master Cleric"},
     {"Master Cleric", "Master Cleric"},
     {"Master Cleric", "Master Cleric"},

     {"Holy Hero", "Holy Heroine"},
     {"Holy Avatar", "Holy Avatar"},
     {"Angel", "Angel"},
     {
         "Demigod",
         "Demigoddess",
     },
     {"Immortal", "Immortal"},
     {"God", "Goddess"},
     {"Deity", "Deity"},
     {"Supreme Master", "Supreme Mistress"},
     {"Creator", "Creator"},
     {"Implementor", "Implementress"}},

    {{"Man", "Woman"},

     {"Pilferer", "Pilferess"},
     {"Footpad", "Footpad"},
     {"Filcher", "Filcheress"},
     {"Pick-Pocket", "Pick-Pocket"},
     {"Sneak", "Sneak"},

     {"Pincher", "Pincheress"},
     {"Cut-Purse", "Cut-Purse"},
     {"Snatcher", "Snatcheress"},
     {"Sharper", "Sharpress"},
     {"Rogue", "Rogue"},

     {"Robber", "Robber"},
     {"Magsman", "Magswoman"},
     {"Highwayman", "Highwaywoman"},
     {"Burglar", "Burglaress"},
     {"Thief", "Thief"},

     {"Knifer", "Knifer"},
     {"Quick-Blade", "Quick-Blade"},
     {"Killer", "Murderess"},
     {"Brigand", "Brigand"},
     {"Cut-Throat", "Cut-Throat"},

     {"Spy", "Spy"},
     {"Grand Spy", "Grand Spy"},
     {"Master Spy", "Master Spy"},
     {"Assassin", "Assassin"},
     {"Greater Assassin", "Greater Assassin"},

     {"Master of Vision", "Mistress of Vision"},
     {"Master of Hearing", "Mistress of Hearing"},
     {"Master of Smell", "Mistress of Smell"},
     {"Master of Taste", "Mistress of Taste"},
     {"Master of Touch", "Mistress of Touch"},

     {"Crime Lord", "Crime Mistress"},
     {"Infamous Crime Lord", "Infamous Crime Mistress"},
     {"Greater Crime Lord", "Greater Crime Mistress"},
     {"Master Crime Lord", "Master Crime Mistress"},
     {"Godfather", "Godmother"},

     {"Master Thief", "Master Thief"},
     {"Master Thief", "Master Thief"},
     {"Master Thief", "Master Thief"},
     {"Master Thief", "Master Thief"},
     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},
     {"Master Thief", "Master Thief"},
     {"Master Thief", "Master Thief"},
     {"Master Thief", "Master Thief"},
     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},
     {"Master Thief", "Master Thief"},
     {"Master Thief", "Master Thief"},
     {"Master Thief", "Master Thief"},
     {"Master Thief", "Master Thief"},

     {"Assassin Hero", "Assassin Heroine"},
     {
         "Avatar of Death",
         "Avatar of Death",
     },
     {"Angel of Death", "Angel of Death"},
     {"Demigod of Assassins", "Demigoddess of Assassins"},
     {"Immortal Assasin", "Immortal Assassin"},
     {
         "God of Assassins",
         "God of Assassins",
     },
     {"Deity of Assassins", "Deity of Assassins"},
     {"Supreme Master", "Supreme Mistress"},
     {"Creator", "Creator"},
     {"Implementor", "Implementress"}},

    {{"Man", "Woman"},

     {"Swordpupil", "Swordpupil"},
     {"Recruit", "Recruit"},
     {"Sentry", "Sentress"},
     {"Fighter", "Fighter"},
     {"Soldier", "Soldier"},

     {"Warrior", "Warrior"},
     {"Veteran", "Veteran"},
     {"Swordsman", "Swordswoman"},
     {"Fencer", "Fenceress"},
     {"Combatant", "Combatess"},

     {"Hero", "Heroine"},
     {"Myrmidon", "Myrmidon"},
     {"Swashbuckler", "Swashbuckleress"},
     {"Mercenary", "Mercenaress"},
     {"Swordmaster", "Swordmistress"},

     {"Lieutenant", "Lieutenant"},
     {"Champion", "Lady Champion"},
     {"Dragoon", "Lady Dragoon"},
     {"Cavalier", "Lady Cavalier"},
     {"Knight", "Lady Knight"},

     {"Grand Knight", "Grand Knight"},
     {"Master Knight", "Master Knight"},
     {"Paladin", "Paladin"},
     {"Grand Paladin", "Grand Paladin"},
     {"Demon Slayer", "Demon Slayer"},

     {"Greater Demon Slayer", "Greater Demon Slayer"},
     {"Dragon Slayer", "Dragon Slayer"},
     {"Greater Dragon Slayer", "Greater Dragon Slayer"},
     {"Underlord", "Underlord"},
     {"Overlord", "Overlord"},

     {"Baron of Thunder", "Baroness of Thunder"},
     {"Baron of Storms", "Baroness of Storms"},
     {"Baron of Tornadoes", "Baroness of Tornadoes"},
     {"Baron of Hurricanes", "Baroness of Hurricanes"},
     {"Baron of Meteors", "Baroness of Meteors"},

     {"Master Warrior", "Master Warrior"},
     {"Master Warrior", "Master Warrior"},
     {"Master Warrior", "Master Warrior"},
     {"Master Warrior", "Master Warrior"},
     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},
     {"Master Warrior", "Master Warrior"},
     {"Master Warrior", "Master Warrior"},
     {"Master Warrior", "Master Warrior"},
     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},
     {"Master Warrior", "Master Warrior"},
     {"Master Warrior", "Master Warrior"},
     {"Master Warrior", "Master Warrior"},
     {"Master Warrior", "Master Warrior"},

     {"Knight Hero", "Knight Heroine"},
     {"Avatar of War", "Avatar of War"},
     {"Angel of War", "Angel of War"},
     {"Demigod of War", "Demigoddess of War"},
     {"Immortal Warlord", "Immortal Warlord"},
     {"God of War", "God of War"},
     {"Deity of War", "Deity of War"},
     {"Supreme Master of War", "Supreme Mistress of War"},
     {"Creator", "Creator"},
     {"Implementor", "Implementress"}}};
#endif

/* Attribute bonus tables. */
STR_APP_T str_app_table[ATTRIBUTE_HIGHEST + 2] = {
    /* stat, tohit, todam, carry, wield */
    {0, -5, -4, 0, 0}, /* 0 */
    {1, -4, -3, 3, 1},
    {2, -3, -2, 6, 2},
    {3, -3, -1, 10, 3},
    {4, -2, -1, 25, 4},
    {5, -2, -1, 55, 5}, /* 5 */
    {6, -1, 0, 80, 6},
    {7, -1, 0, 90, 7},
    {8, 0, 0, 100, 8},
    {9, 0, 0, 107, 9},
    {10, 0, 0, 115, 10}, /* 10 */
    {11, 0, 0, 123, 11},
    {12, 0, 0, 130, 12},
    {13, 0, 0, 137, 13},
    {14, 0, 1, 143, 14},
    {15, 1, 1, 150, 15}, /* 15 */
    {16, 1, 2, 165, 16},
    {17, 2, 3, 180, 22},
    {18, 2, 3, 200, 25},
    {19, 3, 4, 225, 30},
    {20, 3, 5, 250, 35}, /* 20 */
    {21, 4, 6, 300, 40},
    {22, 4, 6, 350, 45},
    {23, 5, 7, 400, 50},
    {24, 5, 8, 450, 55},
    {25, 6, 9, 500, 60}, /* 25 */

    {-999},
};

INT_APP_T int_app_table[ATTRIBUTE_HIGHEST + 2] = {
    /* stat, learn */
    {0, 3}, /* 0 */
    {1, 5},
    {2, 7},
    {3, 8},
    {4, 9},
    {5, 10}, /* 5 */
    {6, 11},
    {7, 12},
    {8, 13},
    {9, 15},
    {10, 17}, /* 10 */
    {11, 19},
    {12, 22},
    {13, 25},
    {14, 28},
    {15, 31}, /* 15 */
    {16, 34},
    {17, 37},
    {18, 40},
    {19, 44},
    {20, 49}, /* 20 */
    {21, 55},
    {22, 60},
    {23, 70},
    {24, 80},
    {25, 85}, /* 25 */

    {-999}};

WIS_APP_T wis_app_table[ATTRIBUTE_HIGHEST + 2] = {
    /* stat, practice */
    {0, 0}, /* 0 */
    {1, 0},
    {2, 0},
    {3, 0},
    {4, 0},
    {5, 1}, /* 5 */
    {6, 1},
    {7, 1},
    {8, 1},
    {9, 1},
    {10, 1}, /* 10 */
    {11, 1},
    {12, 1},
    {13, 1},
    {14, 1},
    {15, 2}, /* 15 */
    {16, 2},
    {17, 2},
    {18, 3},
    {19, 3},
    {20, 3}, /* 20 */
    {21, 3},
    {22, 4},
    {23, 4},
    {24, 4},
    {25, 5}, /* 25 */

    {-999}};

DEX_APP_T dex_app_table[ATTRIBUTE_HIGHEST + 2] = {
    /* stat, defensive */
    {0, 60}, /* 0 */
    {1, 50},
    {2, 50},
    {3, 40},
    {4, 30},
    {5, 20}, /* 5 */
    {6, 10},
    {7, 0},
    {8, 0},
    {9, 0},
    {10, 0}, /* 10 */
    {11, 0},
    {12, 0},
    {13, 0},
    {14, 0},
    {15, -10}, /* 15 */
    {16, -15},
    {17, -20},
    {18, -30},
    {19, -40},
    {20, -50}, /* 20 */
    {21, -60},
    {22, -75},
    {23, -90},
    {24, -105},
    {25, -120}, /* 25 */

    {-999}};

CON_APP_T con_app_table[ATTRIBUTE_HIGHEST + 2] = {
    /* stat, hitp, shock */
    {0, -4, 20}, /* 0 */
    {1, -3, 25},
    {2, -2, 30},
    {3, -2, 35},
    {4, -1, 40},
    {5, -1, 45}, /* 5 */
    {6, -1, 50},
    {7, 0, 55},
    {8, 0, 60},
    {9, 0, 65},
    {10, 0, 70}, /* 10 */
    {11, 0, 75},
    {12, 0, 80},
    {13, 0, 85},
    {14, 0, 88},
    {15, 1, 90}, /* 15 */
    {16, 2, 95},
    {17, 2, 97},
    {18, 3, 99},
    {19, 3, 99},
    {20, 4, 99}, /* 20 */
    {21, 4, 99},
    {22, 5, 99},
    {23, 6, 99},
    {24, 7, 99},
    {25, 8, 99}, /* 25 */

    {-999},
};

/* Liquid properties - loaded from JSON. */
LIQ_T *liq_table = NULL;
int liq_count = 0, liq_cap = 0;

/* Login greetings - loaded from JSON. */
GREETING_T *greeting_table = NULL;
int greeting_count = 0, greeting_cap = 0;

/* Skill table - loaded from JSON at boot. */
SKILL_T *skill_table = NULL;
int skill_count = 0, skill_cap = 0;

/* Skill group table - loaded from JSON at boot. */
SKILL_GROUP_T *skill_group_table = NULL;
int skill_group_count = 0, skill_group_cap = 0;

SECTOR_T sector_table[SECT_MAX + 1] = {
    {SECT_INSIDE, "inside", 1, 'C'},
    {SECT_CITY, "city", 2, 'c'},
    {SECT_FIELD, "field", 2, 'G'},
    {SECT_FOREST, "forest", 3, 'g'},
    {SECT_HILLS, "hills", 4, 'y'},
    {SECT_MOUNTAIN, "mountain", 6, 'R'},
    {SECT_WATER_SWIM, "swim", 4, 'B'},
    {SECT_WATER_NOSWIM, "noswim", 1, 'b'},
    {SECT_UNUSED, "unused_sect", 6, 'w'},
    {SECT_AIR, "air", 10, 'W'},
    {SECT_DESERT, "desert", 6, 'Y'},
    {0}};

/* for doors */
DOOR_T door_table[DIR_MAX + 1] = {
    {DIR_NORTH, "north", "from the north", "to the north", DIR_SOUTH, "N"},
    {DIR_EAST, "east", "from the east", "to the east", DIR_WEST, "E"},
    {DIR_SOUTH, "south", "from the south", "to the south", DIR_NORTH, "S"},
    {DIR_WEST, "west", "from the west", "to the west", DIR_EAST, "W"},
    {DIR_UP, "up", "from above", "above you", DIR_DOWN, "U"},
    {DIR_DOWN, "down", "from below", "below you", DIR_UP, "D"},
    {0},
};

/* the function table */
SPEC_T spec_table[SPEC_MAX + 1] = {
    {"spec_breath_any", spec_breath_any},
    {"spec_breath_acid", spec_breath_acid},
    {"spec_breath_fire", spec_breath_fire},
    {"spec_breath_frost", spec_breath_frost},
    {"spec_breath_gas", spec_breath_gas},
    {"spec_breath_lightning", spec_breath_lightning},
    {"spec_cast_adept", spec_cast_adept},
    {"spec_cast_cleric", spec_cast_cleric},
    {"spec_cast_judge", spec_cast_judge},
    {"spec_cast_mage", spec_cast_mage},
    {"spec_cast_undead", spec_cast_undead},
    {"spec_executioner", spec_executioner},
    {"spec_fido", spec_fido},
    {"spec_guard", spec_guard},
    {"spec_janitor", spec_janitor},
    {"spec_mayor", spec_mayor},
    {"spec_poison", spec_poison},
    {"spec_thief", spec_thief},
    {"spec_nasty", spec_nasty},
    {"spec_troll_member", spec_troll_member},
    {"spec_ogre_member", spec_ogre_member},
    {"spec_patrolman", spec_patrolman},
    {"spec_questmaster", spec_questmaster}, /* Vassago */
    {"spec_assassin", spec_assassin},       /* Rox of Farside */
    {0}};

COLOUR_SETTING_T colour_setting_table[COLOUR_SETTING_MAX + 1] = {
    {COLOUR_TEXT, "text", 't', CC_BACK_DEFAULT | CC_WHITE},
    {COLOUR_AUCTION, "auction", 'a', CC_BACK_DEFAULT | CC_BRIGHT_YELLOW},
    {COLOUR_AUCTION_TEXT, "auction_text", 'A', CC_BACK_DEFAULT | CC_BRIGHT_WHITE},
    {COLOUR_GOSSIP, "gossip", 'd', CC_BACK_DEFAULT | CC_MAGENTA},
    {COLOUR_GOSSIP_TEXT, "gossip_text", '9', CC_BACK_DEFAULT | CC_BRIGHT_MAGENTA},
    {COLOUR_MUSIC, "music", 'e', CC_BACK_DEFAULT | CC_RED},
    {COLOUR_MUSIC_TEXT, "music_text", 'E', CC_BACK_DEFAULT | CC_BRIGHT_RED},
    {COLOUR_QUESTION, "question", 'q', CC_BACK_DEFAULT | CC_BRIGHT_YELLOW},
    {COLOUR_QUESTION_TEXT, "question_text", 'Q', CC_BACK_DEFAULT | CC_BRIGHT_WHITE},
    {COLOUR_ANSWER, "answer", 'f', CC_BACK_DEFAULT | CC_BRIGHT_YELLOW},
    {COLOUR_ANSWER_TEXT, "answer_text", 'F', CC_BACK_DEFAULT | CC_BRIGHT_WHITE},
    {COLOUR_QUOTE, "quote", 'h', CC_BACK_DEFAULT | CC_YELLOW},
    {COLOUR_QUOTE_TEXT, "quote_text", 'H', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_IMMTALK_TEXT, "immtalk_text", 'i', CC_BACK_DEFAULT | CC_CYAN},
    {COLOUR_IMMTALK_TYPE, "immtalk_type", 'I', CC_BACK_DEFAULT | CC_YELLOW},
    {COLOUR_INFO, "info", 'j', CC_BACK_DEFAULT | CC_BRIGHT_YELLOW | CB_BEEP},
    {COLOUR_SAY, "say", '6', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_SAY_TEXT, "say_text", '7', CC_BACK_DEFAULT | CC_BRIGHT_GREEN},
    {COLOUR_TELL, "tell", 'k', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_TELL_TEXT, "tell_text", 'K', CC_BACK_DEFAULT | CC_BRIGHT_GREEN},
    {COLOUR_REPLY, "reply", 'l', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_REPLY_TEXT, "reply_text", 'L', CC_BACK_DEFAULT | CC_BRIGHT_GREEN},
    {COLOUR_GTELL_TEXT, "gtell_text", 'n', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_GTELL_TYPE, "gtell_type", 'N', CC_BACK_DEFAULT | CC_RED},
    {COLOUR_WIZNET, "wiznet", 'B', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_ROOM_TITLE, "room_title", 's', CC_BACK_DEFAULT | CC_CYAN},
    {COLOUR_ROOM_TEXT, "room_text", 'S', CC_BACK_DEFAULT | CC_WHITE},
    {COLOUR_ROOM_EXITS, "room_exits", 'o', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_ROOM_THINGS, "room_things", 'O', CC_BACK_DEFAULT | CC_CYAN},
    {COLOUR_PROMPT, "prompt", 'p', CC_BACK_DEFAULT | CC_CYAN},
    {COLOUR_FIGHT_DEATH, "fight_death", '1', CC_BACK_DEFAULT | CC_BRIGHT_RED},
    {COLOUR_FIGHT_YHIT, "fight_yhit", '2', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_FIGHT_OHIT, "fight_ohit", '3', CC_BACK_DEFAULT | CC_YELLOW},
    {COLOUR_FIGHT_THIT, "fight_thit", '4', CC_BACK_DEFAULT | CC_RED},
    {COLOUR_FIGHT_SKILL, "fight_skill", '5', CC_BACK_DEFAULT | CC_BRIGHT_WHITE},
    {0}};

COLOUR_T colour_table[] = {
    /* All forecolors */
    {CM_FORECOLOUR, CC_DEFAULT, "none"},
    {CM_FORECOLOUR, CC_BLACK, "black"},
    {CM_FORECOLOUR, CC_RED, "red"},
    {CM_FORECOLOUR, CC_GREEN, "green"},
    {CM_FORECOLOUR, CC_YELLOW, "yellow"},
    {CM_FORECOLOUR, CC_BLUE, "blue"},
    {CM_FORECOLOUR, CC_MAGENTA, "magenta"},
    {CM_FORECOLOUR, CC_CYAN, "cyan"},
    {CM_FORECOLOUR, CC_WHITE, "white"},
    {CM_FORECOLOUR, CC_DARK_GREY, "grey"},
    {CM_FORECOLOUR, CC_BRIGHT_RED, "hi-red"},
    {CM_FORECOLOUR, CC_BRIGHT_GREEN, "hi-green"},
    {CM_FORECOLOUR, CC_BRIGHT_YELLOW, "hi-yellow"},
    {CM_FORECOLOUR, CC_BRIGHT_BLUE, "hi-blue"},
    {CM_FORECOLOUR, CC_BRIGHT_MAGENTA, "hi-magenta"},
    {CM_FORECOLOUR, CC_BRIGHT_CYAN, "hi-cyan"},
    {CM_FORECOLOUR, CC_BRIGHT_WHITE, "hi-white"},

    /* All backcolors */
    {CM_BACKCOLOUR, CC_BACK_DEFAULT, "back-none"},
    {CM_BACKCOLOUR, CC_BACK_BLACK, "back-black"},
    {CM_BACKCOLOUR, CC_BACK_RED, "back-red"},
    {CM_BACKCOLOUR, CC_BACK_GREEN, "back-green"},
    {CM_BACKCOLOUR, CC_BACK_YELLOW, "back-yellow"},
    {CM_BACKCOLOUR, CC_BACK_BLUE, "back-blue"},
    {CM_BACKCOLOUR, CC_BACK_MAGENTA, "back-magenta"},
    {CM_BACKCOLOUR, CC_BACK_CYAN, "back-cyan"},
    {CM_BACKCOLOUR, CC_BACK_WHITE, "back-white"},

    /* non-standard; let's not support it. */
    /*
        {CM_BACKCOLOUR, CC_BACK_DARK_GREY,      "back-grey"},
        {CM_BACKCOLOUR, CC_BACK_BRIGHT_RED,     "back-hi-red"},
        {CM_BACKCOLOUR, CC_BACK_BRIGHT_GREEN,   "back-hi-green"},
        {CM_BACKCOLOUR, CC_BACK_BRIGHT_YELLOW,  "back-hi-yellow"},
        {CM_BACKCOLOUR, CC_BACK_BRIGHT_BLUE,    "back-hi-blue"},
        {CM_BACKCOLOUR, CC_BACK_BRIGHT_MAGENTA, "back-hi-magenta"},
        {CM_BACKCOLOUR, CC_BACK_BRIGHT_CYAN,    "back-hi-cyan"},
        {CM_BACKCOLOUR, CC_BACK_BRIGHT_WHITE,   "back-hi-white"},
    */

    /* Individual bits */
    {CM_BEEP, CB_BEEP, "beep"},
    {CM_BEEP, 0x00, "nobeep"},

    /* End */
    {0}};

/* We use WEAR_LOC_MAX+2 to account for 'none' and a blank wear location. */
WEAR_LOC_T wear_loc_table[WEAR_LOC_MAX + 2] = {
    {WEAR_LOC_NONE, "none", "in the inventory", "<in inventory>", 0, 0, "You wear $p nowhere (??).", "$n wears $p nowhere (??)."},
    {WEAR_LOC_LIGHT, "light", "as a light", "<used as light>", ITEM_WEAR_LIGHT, 0, "You light $p and hold it.", "$n lights $p and holds it."},
    {WEAR_LOC_FINGER_L, "lfinger", "on the left finger", "<worn on L-finger>", ITEM_WEAR_FINGER, 0, "You wear $p on your left finger.", "$n wears $p on $s left finger."},
    {WEAR_LOC_FINGER_R, "rfinger", "on the right finger", "<worn on R-finger>", ITEM_WEAR_FINGER, 0, "You wear $p on your right finger.", "$n wears $p on $s right finger."},
    {WEAR_LOC_NECK_1, "neck1", "around the neck (1)", "<worn around neck 1>", ITEM_WEAR_NECK, 100, "You wear $p around your neck.", "$n wears $p around $s neck."},
    {WEAR_LOC_NECK_2, "neck2", "around the neck (2)", "<worn around neck 2>", ITEM_WEAR_NECK, 100, "You wear $p around your neck.", "$n wears $p around $s neck."},
    {WEAR_LOC_BODY, "body", "on the torso", "<worn on torso>", ITEM_WEAR_BODY, 300, "You wear $p on your torso.", "$n wears $p on $s torso."},
    {WEAR_LOC_HEAD, "head", "over the head", "<worn on head>", ITEM_WEAR_HEAD, 200, "You wear $p on your head.", "$n wears $p on $s head."},
    {WEAR_LOC_LEGS, "legs", "on the legs", "<worn on legs>", ITEM_WEAR_LEGS, 200, "You wear $p on your legs.", "$n wears $p on $s legs."},
    {WEAR_LOC_FEET, "feet", "on the feet", "<worn on feet>", ITEM_WEAR_FEET, 100, "You wear $p on your feet.", "$n wears $p on $s feet."},
    {WEAR_LOC_HANDS, "hands", "on the hands", "<worn on hands>", ITEM_WEAR_HANDS, 100, "You wear $p on your hands.", "$n wears $p on $s hands."},
    {WEAR_LOC_ARMS, "arms", "on the arms", "<worn on arms>", ITEM_WEAR_ARMS, 100, "You wear $p on your arms.", "$n wears $p on $s arms."},
    {WEAR_LOC_SHIELD, "shield", "as a shield", "<worn as shield>", ITEM_WEAR_SHIELD, 100, "You wear $p as a shield.", "$n wears $p as a shield."},
    {WEAR_LOC_ABOUT, "about", "about the body", "<worn about body>", ITEM_WEAR_ABOUT, 200, "You wear $p about your torso.", "$n wears $p about $s torso."},
    {WEAR_LOC_WAIST, "waist", "around the waist", "<worn about waist>", ITEM_WEAR_WAIST, 100, "You wear $p about your waist.", "$n wears $p about $s waist."},
    {WEAR_LOC_WRIST_L, "lwrist", "on the left wrist", "<worn around L-wrist>", ITEM_WEAR_WRIST, 100, "You wear $p around your left wrist.", "$n wears $p around $s left wrist."},
    {WEAR_LOC_WRIST_R, "rwrist", "on the right wrist", "<worn around R-wrist>", ITEM_WEAR_WRIST, 100, "You wear $p around your right wrist.", "$n wears $p around $s right wrist."},
    {WEAR_LOC_WIELD, "wielded", "wielded", "<wielded>", ITEM_WIELD, 0, "You wield $p.", "$n wields $p."},
    {WEAR_LOC_HOLD, "hold", "held in the hands", "<held>", ITEM_HOLD, 100, "You hold $p in your hand.", "$n holds $p in $s hand."},
    {WEAR_LOC_FLOAT, "floating", "floating nearby", "<floating nearby>", ITEM_WEAR_FLOAT, 0, "You release $p and it floats next to you.", "$n releases $p to float next to $m."},
    {0},
};

/* Material table - loaded from JSON. */
MATERIAL_T *material_table = NULL;
int material_count = 0, material_cap = 0;

/* Technically not const, but this is a good place to have it! */
BOARD_T board_table[BOARD_MAX + 1] = {
    {"General", "General discussion", 0, 2, "all", DEF_INCLUDE, 21, NULL, FALSE},
    {"Ideas", "Suggestion for improvement", 0, 2, "all", DEF_NORMAL, 60, NULL, FALSE},
    {"Announce", "Announcements from Immortals", 0, L_IMM, "all", DEF_NORMAL, 60, NULL, FALSE},
    {"Bugs", "Typos, bugs, errors", 0, 1, "imm", DEF_NORMAL, 60, NULL, FALSE},
    {"Personal", "Personal messages", 0, 1, "all", DEF_EXCLUDE, 28, NULL, FALSE},
    {0}};

DAY_T day_table[DAY_MAX + 1] = {
    {DAY_MOON, "the Moon"},
    {DAY_BULL, "the Bull"},
    {DAY_DECEPTION, "Deception"},
    {DAY_THUNDER, "Thunder"},
    {DAY_FREEDOM, "Freedom"},
    {DAY_GREAT_GODS, "the Great Gods"},
    {DAY_SUN, "the Sun"},
    {-1, NULL}};

MONTH_T month_table[MONTH_MAX + 1] = {
    {MONTH_WINTER, "Winter"},
    {MONTH_WINTER_WOLF, "the Winter Wolf"},
    {MONTH_FROST_GIANT, "the Frost Giant"},
    {MONTH_OLD_FORCES, "the Old Forces"},
    {MONTH_GRAND_STRUGGLE, "the Grand Struggle"},
    {MONTH_SPRING, "the Spring"},
    {MONTH_NATURE, "Nature"},
    {MONTH_FUTILITY, "Futility"},
    {MONTH_DRAGON, "the Dragon"},
    {MONTH_SUN, "the Sun"},
    {MONTH_HEAT, "the Heat"},
    {MONTH_BATTLE, "the Battle"},
    {MONTH_DARK_SHADES, "the Dark Shades"},
    {MONTH_SHADOWS, "the Shadows"},
    {MONTH_LONG_SHADOWS, "the Long Shadows"},
    {MONTH_ANCIENT_DARKNESS, "the Ancient Darkness"},
    {MONTH_GREAT_EVIL, "the Great Evil"},
    {-1, NULL}};

SKY_T sky_table[SKY_MAX + 1] = {
    {SKY_CLOUDLESS, "cloudless", "cloudless", 1020, -1},
    {SKY_CLOUDY, "cloudy", "cloudy", 1000, 1020},
    {SKY_RAINING, "raining", "rainy", 980, 1000},
    {SKY_LIGHTNING, "lightning", "lit by flashes of lightning", -1, 980},
    {-1, NULL, NULL},
};

SUN_T sun_table[SUN_MAX + 1] = {
    {SUN_DARK, "dark", TRUE, 0, 5, "The night has begun."},
    {SUN_RISE, "rise", FALSE, 5, 6, "The sun rises in the east."},
    {SUN_LIGHT, "light", FALSE, 6, 19, "The day has begun."},
    {SUN_SET, "set", TRUE, 19, 20, "The sun slowly disappears in the west."},
    {-1, NULL, 0}};

/* Pose table - loaded from JSON at boot. */
POSE_T *pose_table = NULL;
int pose_count = 0, pose_cap = 0;

SONG_T song_table[MAX_SONGS + 1];

DEFINE_DISPOSE_FUN(attack_dispose)
{
    ATTACK_T *attack = obj;
    str_free(&(attack->name));
    str_free(&(attack->noun));
}

DEFINE_DISPOSE_FUN(clan_dispose)
{
    CLAN_T *clan = obj;
    str_free(&(clan->name));
    str_free(&(clan->who_name));
}

DEFINE_DISPOSE_FUN(item_dispose)
{
    ITEM_T *item = obj;
    str_free(&(item->name));
}

DEFINE_DISPOSE_FUN(sex_dispose)
{
    SEX_T *sex = obj;
    str_free(&(sex->name));
}

DEFINE_DISPOSE_FUN(size_dispose)
{
    SIZE_T *size = obj;
    str_free(&(size->name));
}

DEFINE_DISPOSE_FUN(day_dispose)
{
    DAY_T *day = obj;
    str_free(&(day->name));
}

DEFINE_DISPOSE_FUN(greeting_dispose)
{
    GREETING_T *greeting = obj;
    str_free(&(greeting->text));
}

DEFINE_DISPOSE_FUN(month_dispose)
{
    MONTH_T *month = obj;
    str_free(&(month->name));
}

DEFINE_DISPOSE_FUN(sky_dispose)
{
    SKY_T *sky = obj;
    str_free(&(sky->name));
    str_free(&(sky->description));
}

DEFINE_DISPOSE_FUN(sun_dispose)
{
    SUN_T *sun = obj;
    str_free(&(sun->name));
    str_free(&(sun->message));
}

DEFINE_DISPOSE_FUN(dam_dispose)
{
    DAM_T *dam = obj;
    str_free(&(dam->name));
}

DEFINE_DISPOSE_FUN(hp_cond_dispose)
{
    HP_COND_T *cond = obj;
    str_free(&(cond->message));
}

DEFINE_DISPOSE_FUN(liq_dispose)
{
    LIQ_T *liq = obj;
    str_free(&(liq->name));
    str_free(&(liq->color));
}

DEFINE_DISPOSE_FUN(position_dispose)
{
    POSITION_T *pos = obj;
    str_free(&(pos->name));
    str_free(&(pos->long_name));
    str_free(&(pos->room_msg));
    str_free(&(pos->room_msg_furniture));
}

DEFINE_DISPOSE_FUN(weapon_dispose)
{
    WEAPON_T *weapon = obj;
    str_free(&(weapon->name));
    str_free(&(weapon->skill));
}

DEFINE_DISPOSE_FUN(colour_dispose)
{
    COLOUR_T *colour = obj;
    str_free(&(colour->name));
}

DEFINE_DISPOSE_FUN(colour_setting_dispose)
{
    COLOUR_SETTING_T *cs = obj;
    str_free(&(cs->name));
}

DEFINE_DISPOSE_FUN(door_dispose)
{
    DOOR_T *door = obj;
    str_free(&(door->name));
    str_free(&(door->short_name));
    str_free(&(door->from_phrase));
    str_free(&(door->to_phrase));
}

DEFINE_DISPOSE_FUN(material_dispose)
{
    MATERIAL_T *material = obj;
    str_free(&(material->name));
}

DEFINE_DISPOSE_FUN(sector_dispose)
{
    SECTOR_T *sector = obj;
    str_free(&(sector->name));
}

DEFINE_DISPOSE_FUN(wear_loc_dispose)
{
    WEAR_LOC_T *wear_loc = obj;
    str_free(&(wear_loc->name));
    str_free(&(wear_loc->phrase));
    str_free(&(wear_loc->look_msg));
    str_free(&(wear_loc->msg_wear_self));
    str_free(&(wear_loc->msg_wear_room));
}

DEFINE_DISPOSE_FUN(class_dispose)
{
    CLASS_T *class = obj;
    int i;

    str_free(&(class->name));
    str_free(&(class->base_group));
    str_free(&(class->default_group));
    free(class->guild);
    class->guild = NULL;
    if (class->titles[0] != NULL)
    {
        for (i = 0; i <= MAX_LEVEL; i++)
            str_free(&(class->titles[0][i]));
        free(class->titles[0]);
        class->titles[0] = NULL;
    }
    if (class->titles[1] != NULL)
    {
        for (i = 0; i <= MAX_LEVEL; i++)
            str_free(&(class->titles[1][i]));
        free(class->titles[1]);
        class->titles[1] = NULL;
    }
}

DEFINE_DISPOSE_FUN(pc_race_dispose)
{
    PC_RACE_T *pc_race = obj;
    int i;

    str_free(&(pc_race->name));
    if (pc_race->skills != NULL)
    {
        for (i = 0; pc_race->skills[i] != NULL; i++)
            str_free(&(pc_race->skills[i]));
        free(pc_race->skills);
        pc_race->skills = NULL;
    }
    free(pc_race->class_mult);
    pc_race->class_mult = NULL;
}

DEFINE_DISPOSE_FUN(race_dispose)
{
    RACE_T *race = obj;
    str_free(&(race->name));
}

DEFINE_DISPOSE_FUN(skill_dispose)
{
    SKILL_T *skill = obj;

    str_free(&(skill->name));
    str_free(&(skill->noun_damage));
    str_free(&(skill->msg_off));
    str_free(&(skill->msg_obj));
    free(skill->classes);
    skill->classes = NULL;
}

DEFINE_DISPOSE_FUN(skill_group_dispose)
{
    SKILL_GROUP_T *group = obj;
    int i;

    str_free(&(group->name));
    for (i = 0; i < group->spell_count; i++)
        str_free(&(group->spells[i]));
    free(group->spells);
    group->spells = NULL;
    free(group->classes);
    group->classes = NULL;
}

DEFINE_DISPOSE_FUN(song_dispose)
{
    SONG_T *song = obj;
    int i;

    str_free(&(song->group));
    str_free(&(song->name));
    for (i = 0; i < song->lines; i++)
        str_free(&(song->lyrics[i]));
    free(song->lyrics);
    song->lyrics = NULL;
    song->lines = 0;
}

DEFINE_DISPOSE_FUN(board_dispose)
{
    BOARD_T *board = obj;
    str_free(&(board->name));
    str_free(&(board->long_name));
    str_free(&(board->names));
}

DEFINE_DISPOSE_FUN(cond_dispose)
{
    COND_T *cond = obj;
    str_free(&(cond->name));
    str_free(&(cond->msg_good));
    str_free(&(cond->msg_bad));
    str_free(&(cond->msg_better));
    str_free(&(cond->msg_worse));
}

DEFINE_DISPOSE_FUN(pose_dispose)
{
    POSE_T *pose = obj;
    int i;
    str_free(&(pose->class_name));
    for (i = 0; i < MAX_LEVEL * 2 + 2; i++)
        str_free(&(pose->message[i]));
}

DEFINE_DISPOSE_FUN(spec_dispose)
{
    SPEC_T *spec = obj;
    str_free(&(spec->name));
    spec->function = NULL;
}

static SPEC_FUN *spec_dispatch_lookup_static(const char *name)
{
    static const struct
    {
        const char *name;
        SPEC_FUN *fun;
    } dispatch[] = {
        {"spec_breath_any", spec_breath_any},
        {"spec_breath_acid", spec_breath_acid},
        {"spec_breath_fire", spec_breath_fire},
        {"spec_breath_frost", spec_breath_frost},
        {"spec_breath_gas", spec_breath_gas},
        {"spec_breath_lightning", spec_breath_lightning},
        {"spec_cast_adept", spec_cast_adept},
        {"spec_cast_cleric", spec_cast_cleric},
        {"spec_cast_judge", spec_cast_judge},
        {"spec_cast_mage", spec_cast_mage},
        {"spec_cast_undead", spec_cast_undead},
        {"spec_executioner", spec_executioner},
        {"spec_fido", spec_fido},
        {"spec_guard", spec_guard},
        {"spec_janitor", spec_janitor},
        {"spec_mayor", spec_mayor},
        {"spec_poison", spec_poison},
        {"spec_thief", spec_thief},
        {"spec_nasty", spec_nasty},
        {"spec_troll_member", spec_troll_member},
        {"spec_ogre_member", spec_ogre_member},
        {"spec_patrolman", spec_patrolman},
        {"spec_questmaster", spec_questmaster},
        {NULL, NULL}};
    int i;
    if (name == NULL)
        return NULL;
    for (i = 0; dispatch[i].name != NULL; i++)
        if (!str_cmp(dispatch[i].name, name))
            return dispatch[i].fun;
    return NULL;
}

void spec_reload_mapping(void)
{
    int i;
    for (i = 0; i < SPEC_MAX; i++)
    {
        SPEC_T *spec = &spec_table[i];
        if (spec->name == NULL)
            break;
        spec->function = spec_dispatch_lookup_static(spec->name);
        if (spec->function == NULL)
            bugf("spec_reload_mapping: Unknown spec '%s'", spec->name);
    }
}

void cond_reload_mapping(void)
{
    int i;
    for (i = 0; i < COND_MAX; i++)
    {
        COND_T *cond = &cond_table[i];
        if (cond->name == NULL)
            break;
        switch (cond->type)
        {
        case COND_DRUNK:
            cond->good_fun = char_is_sober;
            cond->bad_fun = char_is_drunk;
            break;
        case COND_FULL:
            cond->good_fun = NULL;
            cond->bad_fun = char_is_full;
            break;
        case COND_THIRST:
            cond->good_fun = char_is_quenched;
            cond->bad_fun = char_is_thirsty;
            break;
        case COND_HUNGER:
            cond->good_fun = char_is_fed;
            cond->bad_fun = char_is_hungry;
            break;
        default:
            bugf("cond_reload_mapping: Unknown cond type %d", cond->type);
            break;
        }
    }
}

void greeting_reload_mapping(void)
{
    help_greeting  = (greeting_count > 0) ? greeting_table[0].text : NULL;
    help_greeting1 = (greeting_count > 1) ? greeting_table[1].text : NULL;
    help_greeting2 = (greeting_count > 2) ? greeting_table[2].text : NULL;
    help_greeting3 = (greeting_count > 3) ? greeting_table[3].text : NULL;
}

const AFFECT_BIT_T affect_bit_table[AFF_TO_MAX + 1] = {
    {"affects", AFF_TO_AFFECTS, affect_flags, "affect_flags"},
    {"object", AFF_TO_OBJECT, extra_flags, "extra_flags"},
    {"immune", AFF_TO_IMMUNE, res_flags, "res_flags"},
    {"resist", AFF_TO_RESIST, res_flags, "res_flags"},
    {"vuln", AFF_TO_VULN, res_flags, "res_flags"},
    {"weapon", AFF_TO_WEAPON, weapon_flags, "weapon_flags"},
    {NULL, 0, NULL},
};

const EFFECT_T effect_table[EFFECT_MAX + 1] = {
    {EFFECT_NONE, "none", effect_empty},
    {EFFECT_FIRE, "fire", effect_fire},
    {EFFECT_COLD, "cold", effect_cold},
    {EFFECT_SHOCK, "shock", effect_shock},
    {EFFECT_ACID, "acid", effect_acid},
    {EFFECT_POISON, "poison", effect_poison},
    {0}};

const FURNITURE_BITS_T furniture_table[POS_MAX + 1] = {
    {POS_STANDING, "standing", STAND_AT, STAND_ON, STAND_IN},
    {POS_SITTING, "sitting", SIT_AT, SIT_ON, SIT_IN},
    {POS_RESTING, "resting", REST_AT, REST_ON, REST_IN},
    {POS_SLEEPING, "sleeping", SLEEP_AT, SLEEP_ON, SLEEP_IN},
    {-1, NULL, 0, 0}};

const MAP_LOOKUP_TABLE_T map_flags_table[MAP_LOOKUP_MAX + 1] = {
    {MAP_FLAGS_WEAPON, "weapon", weapon_flags},
    {MAP_FLAGS_CONT, "container", container_flags},
    {MAP_FLAGS_FURNITURE, "furniture", furniture_flags},
    {MAP_FLAGS_EXIT, "exit", exit_flags},
    {MAP_FLAGS_GATE, "gate", gate_flags},
    {-1, NULL, NULL},
};

const MAP_LOOKUP_TABLE_T map_lookup_table[MAP_LOOKUP_MAX + 1] = {
    {MAP_LOOKUP_WEAPON_TYPE, "weapon_type", NULL},
    {MAP_LOOKUP_ATTACK_TYPE, "attack_type", NULL},
    {MAP_LOOKUP_LIQUID, "liquid", NULL},
    {MAP_LOOKUP_SKILL, "skill", NULL},
    {-1, NULL, NULL},
};

const NANNY_HANDLER_T nanny_table[NANNY_MAX + 1] = {
    {CON_ANSI, "ansi", nanny_ansi},
    {CON_GET_NAME, "get_player_name", nanny_get_player_name},
    {CON_GET_OLD_PASSWORD, "get_old_password", nanny_get_old_password},
    {CON_BREAK_CONNECT, "break_connect", nanny_break_connect},
    {CON_CONFIRM_NEW_NAME, "confirm_new_name", nanny_confirm_new_name},
    {CON_GET_NEW_PASSWORD, "get_new_password", nanny_get_new_password},
    {CON_CONFIRM_PASSWORD, "confirm_new_password", nanny_confirm_new_password},
    {CON_GET_NEW_RACE, "get_new_race", nanny_get_new_race},
    {CON_GET_NEW_SEX, "get_new_sex", nanny_get_new_sex},
    {CON_GET_NEW_CLASS, "get_new_class", nanny_get_new_class},
    {CON_GET_ALIGNMENT, "get_alignment", nanny_get_alignment},
    {CON_DEFAULT_CHOICE, "default_choice", nanny_default_choice},
    {CON_PICK_WEAPON, "pick_weapon", nanny_pick_weapon},
    {CON_GEN_GROUPS, "gen_groups", nanny_gen_groups},
    {CON_READ_IMOTD, "read_imotd", nanny_read_imotd},
    {CON_READ_MOTD, "read_motd", nanny_read_motd},

    /* states for new note system, (c)1995-96 erwin@pip.dknet.dk */
    /* ch MUST be PC here; have nwrite check for PC status! */
    {CON_NOTE_TO, "note_to", handle_con_note_to},
    {CON_NOTE_SUBJECT, "note_subject", handle_con_note_subject},
    {CON_NOTE_EXPIRE, "note_expire", handle_con_note_expire},
    {CON_NOTE_TEXT, "note_text", handle_con_note_text},
    {CON_NOTE_FINISH, "note_finish", handle_con_note_finish},

    {-1, NULL}};

const OBJ_MAP_T obj_map_table[ITEM_MAX + 1] = {
    {ITEM_WEAPON, {
                      {0, -1, "weapon_type", MAP_LOOKUP, MAP_LOOKUP_WEAPON_TYPE},
                      {1, 0, "dice_num", MAP_INTEGER, 0},
                      {2, 0, "dice_size", MAP_INTEGER, 0},
                      {3, -1, "attack_type", MAP_LOOKUP, MAP_LOOKUP_ATTACK_TYPE},
                      {4, 0, "flags", MAP_FLAGS, MAP_FLAGS_WEAPON},
                  }},
    {ITEM_CONTAINER, {{0, 0, "capacity", MAP_INTEGER, 0}, {1, 0, "flags", MAP_FLAGS, MAP_FLAGS_CONT}, {2, 0, "key", MAP_INTEGER, 0}, {3, 0, "max_weight", MAP_INTEGER, 0}, {4, 0, "weight_mult", MAP_INTEGER, 0}}},
    {ITEM_DRINK_CON, {{0, 0, "capacity", MAP_INTEGER, 0}, {1, 0, "filled", MAP_INTEGER, 0}, {2, -1, "liquid", MAP_LOOKUP, MAP_LOOKUP_LIQUID}, {3, 0, "poisoned", MAP_BOOLEAN, 0}, {4, 0, NULL, MAP_IGNORE, 0}}},
    {ITEM_FOUNTAIN, {{0, 0, "capacity", MAP_INTEGER, 0}, {1, 0, "filled", MAP_INTEGER, 0}, {2, -1, "liquid", MAP_LOOKUP, MAP_LOOKUP_LIQUID}, {3, 0, "poisoned", MAP_BOOLEAN, 0}, {4, 0, NULL, MAP_IGNORE, 0}}},
    {ITEM_WAND, {{0, 0, "level", MAP_INTEGER, 0}, {1, 0, "recharge", MAP_INTEGER, 0}, {2, 0, "charges", MAP_INTEGER, 0}, {3, -1, "skill", MAP_LOOKUP, MAP_LOOKUP_SKILL}, {4, 0, NULL, MAP_IGNORE, 0}}},
    {ITEM_STAFF, {{0, 0, "level", MAP_INTEGER, 0}, {1, 0, "recharge", MAP_INTEGER, 0}, {2, 0, "charges", MAP_INTEGER, 0}, {3, -1, "skill", MAP_LOOKUP, MAP_LOOKUP_SKILL}, {4, 0, NULL, MAP_IGNORE, 0}}},
    {ITEM_FOOD, {{0, 0, "hunger", MAP_INTEGER, 0}, {1, 0, "fullness", MAP_INTEGER, 0}, {2, 0, NULL, MAP_IGNORE, 0}, {3, 0, "poisoned", MAP_BOOLEAN, 0}, {4, 0, NULL, MAP_IGNORE, 0}}},
    {ITEM_MONEY, {{0, 0, "silver", MAP_INTEGER, 0}, {1, 0, "gold", MAP_INTEGER, 0}, {2, 0, NULL, MAP_IGNORE, 0}, {3, 0, NULL, MAP_IGNORE, 0}, {4, 0, NULL, MAP_IGNORE, 0}}},
    {ITEM_ARMOR, {{0, 0, "vs_pierce", MAP_INTEGER, 0}, {1, 0, "vs_bash", MAP_INTEGER, 0}, {2, 0, "vs_slash", MAP_INTEGER, 0}, {3, 0, "vs_magic", MAP_INTEGER, 0}, {4, 0, NULL, MAP_IGNORE, 0}}},
    {ITEM_POTION, {
                      {0, 0, "level", MAP_INTEGER, 0},
                      {1, -1, "skill1", MAP_LOOKUP, MAP_LOOKUP_SKILL},
                      {2, -1, "skill2", MAP_LOOKUP, MAP_LOOKUP_SKILL},
                      {3, -1, "skill3", MAP_LOOKUP, MAP_LOOKUP_SKILL},
                      {4, -1, "skill4", MAP_LOOKUP, MAP_LOOKUP_SKILL},
                  }},
    {ITEM_PILL, {
                    {0, 0, "level", MAP_INTEGER, 0},
                    {1, -1, "skill1", MAP_LOOKUP, MAP_LOOKUP_SKILL},
                    {2, -1, "skill2", MAP_LOOKUP, MAP_LOOKUP_SKILL},
                    {3, -1, "skill3", MAP_LOOKUP, MAP_LOOKUP_SKILL},
                    {4, -1, "skill4", MAP_LOOKUP, MAP_LOOKUP_SKILL},
                }},
    {ITEM_SCROLL, {
                      {0, 0, "level", MAP_INTEGER, 0},
                      {1, -1, "skill1", MAP_LOOKUP, MAP_LOOKUP_SKILL},
                      {2, -1, "skill2", MAP_LOOKUP, MAP_LOOKUP_SKILL},
                      {3, -1, "skill3", MAP_LOOKUP, MAP_LOOKUP_SKILL},
                      {4, -1, "skill4", MAP_LOOKUP, MAP_LOOKUP_SKILL},
                  }},
    {ITEM_MAP, {
                   {0, 0, "persist", MAP_BOOLEAN, 0},
                   {1, 0, NULL, MAP_IGNORE, 0},
                   {2, 0, NULL, MAP_IGNORE, 0},
                   {3, 0, NULL, MAP_IGNORE, 0},
                   {4, 0, NULL, MAP_IGNORE, 0},
               }},
    {ITEM_FURNITURE, {
                         {0, 0, "max_people", MAP_INTEGER, 0},
                         {1, 0, "max_weight", MAP_INTEGER, 0},
                         {2, 0, "flags", MAP_FLAGS, MAP_FLAGS_FURNITURE},
                         {3, 0, "heal_rate", MAP_INTEGER, 0},
                         {4, 0, "mana_rate", MAP_INTEGER, 0},
                     }},
    {ITEM_LIGHT, {
                     {0, 0, NULL, MAP_IGNORE, 0},
                     {1, 0, NULL, MAP_IGNORE, 0},
                     {2, 0, "duration", MAP_INTEGER, 0},
                     {3, 0, NULL, MAP_IGNORE, 0},
                     {4, 0, NULL, MAP_IGNORE, 0},
                 }},
    {ITEM_PORTAL, {
                      {0, 0, "charges", MAP_INTEGER, 0},
                      {1, 0, "exit_flags", MAP_FLAGS, MAP_FLAGS_EXIT},
                      {2, 0, "gate_flags", MAP_FLAGS, MAP_FLAGS_GATE},
                      {3, 0, "to_vnum", MAP_INTEGER, 0},
                      {4, 0, "key", MAP_INTEGER, 0},
                  }},

#define OBJ_MAP_NO_VALUES(type)                                           \
    {                                                                     \
        type,                                                             \
        {                                                                 \
            {0, 0, NULL, MAP_IGNORE, 0}, {1, 0, NULL, MAP_IGNORE, 0},     \
                {2, 0, NULL, MAP_IGNORE, 0}, {3, 0, NULL, MAP_IGNORE, 0}, \
                {4, 0, NULL, MAP_IGNORE, 0}                               \
        }                                                                 \
    }

    OBJ_MAP_NO_VALUES(ITEM_TRASH),
    OBJ_MAP_NO_VALUES(ITEM_GEM),
    OBJ_MAP_NO_VALUES(ITEM_TREASURE),
    OBJ_MAP_NO_VALUES(ITEM_KEY),
    OBJ_MAP_NO_VALUES(ITEM_JEWELRY),
    OBJ_MAP_NO_VALUES(ITEM_CLOTHING),
    OBJ_MAP_NO_VALUES(ITEM_BOAT),
    OBJ_MAP_NO_VALUES(ITEM_CORPSE_NPC),
    OBJ_MAP_NO_VALUES(ITEM_CORPSE_PC),
    OBJ_MAP_NO_VALUES(ITEM_JUKEBOX),
    OBJ_MAP_NO_VALUES(ITEM_WARP_STONE),

    OBJ_MAP_NO_VALUES(-1),
};

/* Globals. */
SKILL_MAP_T skill_map_table[SKILL_MAP_MAX + 1] = {
    {SKILL_MAP_BACKSTAB, "backstab"},
    {SKILL_MAP_DODGE, "dodge"},
    {SKILL_MAP_ENVENOM, "envenom"},
    {SKILL_MAP_HIDE, "hide"},
    {SKILL_MAP_PEEK, "peek"},
    {SKILL_MAP_PICK_LOCK, "pick lock"},
    {SKILL_MAP_SNEAK, "sneak"},
    {SKILL_MAP_STEAL, "steal"},

    {SKILL_MAP_DISARM, "disarm"},
    {SKILL_MAP_ENHANCED_DAMAGE, "enhanced damage"},
    {SKILL_MAP_KICK, "kick"},
    {SKILL_MAP_PARRY, "parry"},
    {SKILL_MAP_RESCUE, "rescue"},
    {SKILL_MAP_SECOND_ATTACK, "second attack"},
    {SKILL_MAP_THIRD_ATTACK, "third attack"},

    {SKILL_MAP_BLINDNESS, "blindness"},
    {SKILL_MAP_CHARM_PERSON, "charm person"},
    {SKILL_MAP_CURSE, "curse"},
    {SKILL_MAP_INVIS, "invisibility"},
    {SKILL_MAP_MASS_INVIS, "mass invis"},
    {SKILL_MAP_POISON, "poison"},
    {SKILL_MAP_PLAGUE, "plague"},
    {SKILL_MAP_SLEEP, "sleep"},
    {SKILL_MAP_SANCTUARY, "sanctuary"},
    {SKILL_MAP_FLY, "fly"},

    {SKILL_MAP_AXE, "axe"},
    {SKILL_MAP_DAGGER, "dagger"},
    {SKILL_MAP_FLAIL, "flail"},
    {SKILL_MAP_MACE, "mace"},
    {SKILL_MAP_POLEARM, "polearm"},
    {SKILL_MAP_SHIELD_BLOCK, "shield block"},
    {SKILL_MAP_SPEAR, "spear"},
    {SKILL_MAP_SWORD, "sword"},
    {SKILL_MAP_WHIP, "whip"},

    {SKILL_MAP_BASH, "bash"},
    {SKILL_MAP_BERSERK, "berserk"},
    {SKILL_MAP_DIRT, "dirt kicking"},
    {SKILL_MAP_HAND_TO_HAND, "hand to hand"},
    {SKILL_MAP_TRIP, "trip"},

    {SKILL_MAP_FAST_HEALING, "fast healing"},
    {SKILL_MAP_HAGGLE, "haggle"},
    {SKILL_MAP_LORE, "lore"},
    {SKILL_MAP_MEDITATION, "meditation"},

    {SKILL_MAP_SCROLLS, "scrolls"},
    {SKILL_MAP_STAVES, "staves"},
    {SKILL_MAP_WANDS, "wands"},
    {SKILL_MAP_RECALL, "recall"},
    {SKILL_MAP_FRENZY, "frenzy"},
    {SKILL_MAP_BUTCHER, "butcher"},
    {SKILL_MAP_HUNT, "hunt"},
    {SKILL_MAP_CRITICAL_STRIKE, "critical strike"},
    {SKILL_MAP_SILENCE, "silence"},

    {0}};

#define RE_NULL

#define RECYCLE_REAL_ENTRY(rtype, rname, vtype, name_off, init, dispose) { \
    rtype,                                                                 \
    #rname,                                                                \
    sizeof(vtype),                                                         \
    GET_OFFSET(vtype, rec_data),                                           \
    name_off,                                                              \
    init,                                                                  \
    dispose}

#define RECYCLE_ENTRY(rtype, rname, vtype, init, dispose) \
    RECYCLE_REAL_ENTRY(rtype, rname, vtype, -1, init, dispose)

#define RECYCLE_N_ENTRY(rtype, rname, vtype, name, init, dispose) \
    RECYCLE_REAL_ENTRY(rtype, rname, vtype, GET_OFFSET(vtype, name), init, dispose)

#define BLANK -1

RECYCLE_T recycle_table[RECYCLE_MAX + 1] = {
    RECYCLE_N_ENTRY(RECYCLE_BAN_T, ban, BAN_T, name, ban_init, ban_dispose),
    RECYCLE_N_ENTRY(RECYCLE_AREA_T, area, AREA_T, name, area_init, area_dispose),
    RECYCLE_ENTRY(RECYCLE_EXTRA_DESCR_T, extra_descr, EXTRA_DESCR_T, extra_descr_init, extra_descr_dispose),
    RECYCLE_ENTRY(RECYCLE_EXIT_T, exit, EXIT_T, exit_init, exit_dispose),
    RECYCLE_N_ENTRY(RECYCLE_ROOM_INDEX_T, room_index, ROOM_INDEX_T, name, room_index_init, room_index_dispose),
    RECYCLE_N_ENTRY(RECYCLE_OBJ_INDEX_T, obj_index, OBJ_INDEX_T, name, obj_index_init, obj_index_dispose),
    RECYCLE_ENTRY(RECYCLE_SHOP_T, shop, SHOP_T, shop_init, shop_dispose),
    RECYCLE_ENTRY(RECYCLE_MOB_INDEX_T, mob_index, MOB_INDEX_T, mob_index_init, mob_index_dispose),
    RECYCLE_ENTRY(RECYCLE_RESET_T, reset_data, RESET_T, reset_data_init, reset_data_dispose),
    RECYCLE_N_ENTRY(RECYCLE_HELP_T, help, HELP_T, keyword, NULL, help_dispose),
    RECYCLE_ENTRY(RECYCLE_MPROG_CODE_T, mpcode, MPROG_CODE_T, mpcode_init, mpcode_dispose),
    RECYCLE_ENTRY(RECYCLE_DESCRIPTOR_T, descriptor, DESCRIPTOR_T, descriptor_init, descriptor_dispose),
    RECYCLE_ENTRY(RECYCLE_GEN_T, gen_data, GEN_T, gen_data_init, gen_data_dispose),
    RECYCLE_ENTRY(RECYCLE_AFFECT_T, affect, AFFECT_T, NULL, affect_dispose),
    RECYCLE_ENTRY(RECYCLE_OBJ_T, obj, OBJ_T, NULL, obj_dispose),
    RECYCLE_ENTRY(RECYCLE_CHAR_T, char, CHAR_T, char_init, char_dispose),
    RECYCLE_ENTRY(RECYCLE_PC_T, pcdata, PC_T, pcdata_init, pcdata_dispose),
    RECYCLE_ENTRY(RECYCLE_MEM_T, mem_data, MEM_T, NULL, NULL),
    RECYCLE_ENTRY(RECYCLE_BUFFER_T, buf, BUFFER_T, buf_init, buf_dispose),
    RECYCLE_ENTRY(RECYCLE_MPROG_LIST_T, mprog, MPROG_LIST_T, mprog_init, mprog_dispose),
    RECYCLE_N_ENTRY(RECYCLE_HELP_AREA_T, had, HELP_AREA_T, name, NULL, had_dispose),
    RECYCLE_ENTRY(RECYCLE_NOTE_T, note, NOTE_T, NULL, note_dispose),
    RECYCLE_N_ENTRY(RECYCLE_SOCIAL_T, social, SOCIAL_T, name, social_init, social_dispose),
    RECYCLE_N_ENTRY(RECYCLE_PORTAL_EXIT_T, portal_exit, PORTAL_EXIT_T, name, NULL, portal_exit_dispose),
    RECYCLE_ENTRY(RECYCLE_PORTAL_T, portal, PORTAL_T, NULL, portal_dispose),
    RECYCLE_N_ENTRY(RECYCLE_WIZ_T, wiz, WIZ_T, name, wiz_init, wiz_dispose),
    {0}};

/* wiznet table and prototype for future flag setting */
const WIZNET_T wiznet_table[WIZNET_MAX + 1] = {
    {WIZ_ON, "on", IM},
    {WIZ_PREFIX, "prefix", IM},
    {WIZ_TICKS, "ticks", IM},
    {WIZ_LOGINS, "logins", IM},
    {WIZ_SITES, "sites", L4},
    {WIZ_LINKS, "links", L7},
    {WIZ_NEWBIE, "newbies", IM},
    {WIZ_SPAM, "spam", L5},
    {WIZ_DEATHS, "deaths", IM},
    {WIZ_RESETS, "resets", L4},
    {WIZ_MOBDEATHS, "mobdeaths", L4},
    {WIZ_FLAGS, "flags", L5},
    {WIZ_PENALTIES, "penalties", L5},
    {WIZ_SACCING, "saccing", L5},
    {WIZ_LEVELS, "levels", IM},
    {WIZ_LOAD, "load", L2},
    {WIZ_RESTORE, "restore", L2},
    {WIZ_SNOOPS, "snoops", L2},
    {WIZ_SWITCHES, "switches", L2},
    {WIZ_SECURE, "secure", L1},
    {-1, NULL, 0}};

COND_T cond_table[COND_MAX + 1] = {
    /* type,        name,     good_fun,         bad_fun,         msg_good,                       msg_bad,                msg_better,                       msg_worse */
    {COND_DRUNK, "drunk", char_is_sober, char_is_drunk, "You are sober.\n\r", "You feel drunk.\n\r", NULL, "You feel a little tispy...\n\r"},
    {COND_FULL, "full", NULL, char_is_full, NULL, "You are full.\n\r", NULL, NULL},
    {COND_THIRST, "thirst", char_is_quenched, char_is_thirsty, "Your thirst is quenched.\n\r", "You are thirsty.\n\r", "You are no longer thirsty.\n\r", NULL},
    {COND_HUNGER, "hunger", char_is_fed, char_is_hungry, "You feel well-fed.\n\r", "You are hungry.\n\r", "You are no longer hungry.\n\r", NULL},
    {0}};

const TRAIN_STAT_T train_stat_table[TRAIN_STAT_MAX + 1] = {
    {"str", "strength", train_stat_cost_stat, train_stat_can_stat, train_stat_do_stat, STAT_STR},
    {"int", "intelligence", train_stat_cost_stat, train_stat_can_stat, train_stat_do_stat, STAT_INT},
    {"wis", "wisdom", train_stat_cost_stat, train_stat_can_stat, train_stat_do_stat, STAT_WIS},
    {"dex", "dexterity", train_stat_cost_stat, train_stat_can_stat, train_stat_do_stat, STAT_DEX},
    {"con", "constitution", train_stat_cost_stat, train_stat_can_stat, train_stat_do_stat, STAT_CON},
    {"hp", "durability", NULL, NULL, train_stat_do_hp_mana, 0},
    {"mana", "power", NULL, NULL, train_stat_do_hp_mana, 1},
    {0},
};

const HEAL_SPELL_T heal_spell_table[HEAL_SPELL_MAX + 1] = {
    {"light", "cure light wounds", spell_cure_light, "cure light", 10},
    {"serious", "cure serious wounds", spell_cure_serious, "cure serious", 15},
    {"critic", "cure critical wounds", spell_cure_critical, "cure critical", 25},
    {"heal", "healing spell", spell_heal, "heal", 50},
    {"blind", "cure blindness", spell_cure_blindness, "cure blindness", 20},
    {"disease", "cure disease", spell_cure_disease, "cure disease", 15},
    {"poison", "cure poison", spell_cure_poison, "cure poison", 25},
    {"uncurse curse", "remove curse", spell_remove_curse, "remove curse", 50},
    {"refresh moves", "restore movement", spell_refresh, "refresh", 5},
    {"mana energize", "restore mana", spell_restore_mana, "restore mana", 10},
    {0},
};
