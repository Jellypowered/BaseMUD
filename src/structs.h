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

#ifndef __ROM_STRUCTS_H
#define __ROM_STRUCTS_H

#include "merc.h"

/* Extended flags. */
/* NOTE: the type used should be determined by EXT_FLAGS_ELEMENT_SIZE,
 *       defined in 'defs.h'. If you want to use anything other than
 *       8-bit unsigned chars, please upgrade! */
struct ext_flags_type
{
    unsigned char bits[EXT_FLAGS_ARRAY_LENGTH];
};

struct ext_init_flags_type
{
    int *bits;
};

/* Objects that can be instantiated, freed, recycled, and catalogued. */
struct recycle_type
{
    int type;
    char *name;
    size_t size;
    size_t obj_data_off;
    size_t obj_name_off;
    INIT_FUN *const init_fun;
    DISPOSE_FUN *const dispose_fun;

    /* internal - do not set in definition table! */
    int top, list_count, free_count;
    OBJ_RECYCLE_T *list_first, *list_last;
    OBJ_RECYCLE_T *free_first, *free_last;
};

/* Data stored in individual objects to make recycling work. */
struct obj_recycle_data
{
    void *obj;
    OBJ_RECYCLE_T *prev, *next;
    bool valid;
};

/* Structures, ahoy!*/
struct ban_data
{
    BAN_T *global_next, *global_prev;
    flag_t ban_flags;
    sh_int level;
    char *name;
    OBJ_RECYCLE_T rec_data;
};

struct wiz_data
{
    WIZ_T *global_next, *global_prev;
    sh_int level;
    char *name;
    OBJ_RECYCLE_T rec_data;
};

struct buf_type
{
    sh_int state; /* error state of the buffer */
    int size;     /* size in bytes */
    char *string; /* buffer's string */
    OBJ_RECYCLE_T rec_data;
};

struct time_info_data
{
    int hour;
    int day;
    int month;
    int year;
};

struct weather_data
{
    int mmhg;
    int change;
    int sky;
    int sunlight;
};

/* Descriptor (channel) structure. */
struct descriptor_data
{
    DESCRIPTOR_T *global_next, *global_prev;
    DESCRIPTOR_T *snoop_by;
    CHAR_T *character;
    CHAR_T *original;
    bool ansi;
    char *host;
    sh_int descriptor;
    sh_int connected;
    bool fcommand;
    char inbuf[4 * MAX_INPUT_LENGTH];
    char incomm[MAX_INPUT_LENGTH];
    char inlast[MAX_INPUT_LENGTH];
    int repeat;
    char *outbuf;
    int outsize;
    int outtop;
    char *showstr_head;
    char *showstr_point;
    int lines_written;  /* for the pager */
    void *olc_edit;     /* OLC */
    char **string_edit; /* OLC */
    int editor;         /* OLC */
    OBJ_RECYCLE_T rec_data;
};

/* Attribute bonus structures. */
struct str_app_type
{
    int stat;
    sh_int tohit;
    sh_int todam;
    sh_int carry;
    sh_int wield;
    OBJ_RECYCLE_T rec_data;
};

struct int_app_type
{
    int stat;
    sh_int learn;
};

struct wis_app_type
{
    int stat;
    sh_int practice;
};

struct dex_app_type
{
    int stat;
    sh_int defensive;
};

struct con_app_type
{
    int stat;
    sh_int hitp;
    sh_int shock;
};

/* Help table types. */
struct help_data
{
    HELP_T *global_next, *global_prev;
    HELP_AREA_T *had;
    HELP_T *had_next, *had_prev;
    sh_int level;
    char *keyword;
    char *text;
    OBJ_RECYCLE_T rec_data;
};

struct help_area_data
{
    HELP_AREA_T *global_next, *global_prev;
    AREA_T *area;
    HELP_AREA_T *area_next, *area_prev;
    HELP_T *help_first, *help_last;
    char *area_str;
    char *filename;
    char *name;
    bool changed;
    OBJ_RECYCLE_T rec_data;
};

struct shop_data
{
    SHOP_T *global_next, *global_prev;
    sh_int keeper;      /* Vnum of shop keeper mob     */
    sh_int *buy_type;   /* heap, buy_count entries */
    int buy_count;      /* number of item types */
    sh_int profit_buy;  /* Cost multiplier for buying  */
    sh_int profit_sell; /* Cost multiplier for selling */
    sh_int open_hour;   /* First opening hour          */
    sh_int close_hour;  /* First closing hour          */
    OBJ_RECYCLE_T rec_data;
};

struct class_type
{
    char *name;          /* the full name of the class  */
    char who_name[4];    /* Three-letter name for 'who' */
    sh_int attr_prime;   /* Prime attribute             */
    sh_int weapon;       /* First weapon                */
    sh_int *guild;       /* Vnum of guild rooms (heap, guild_count entries) */
    int guild_count;     /* Number of guild entries */
    sh_int skill_adept;  /* Maximum skill level         */
    sh_int thac0_00;     /* Thac0 for level  0          */
    sh_int thac0_32;     /* Thac0 for level 32          */
    sh_int hp_min;       /* Min hp gained on leveling   */
    sh_int hp_max;       /* Max hp gained on leveling   */
    bool gains_mana;     /* Class gains mana on level   */
    char *base_group;    /* base skills gained          */
    char *default_group; /* default skills gained       */
    bool can_sneak_away; /* Can sneak away when fleeing */
    char **titles[2];    /* heap, [0]=male [1]=female, MAX_LEVEL+1 entries each */
};

struct item_type
{
    int type;
    char *name;
};

struct weapon_type
{
    sh_int type;
    char *name;
    char *skill;
    sh_int newbie_vnum;
    int skill_index; /* dynamically set */
};

struct wiznet_type
{
    flag_t bit;
    char *name;
    int level;
};

struct effect_type
{
    int type;
    char *name;
    EFFECT_FUN *effect_fun;
};

struct dam_type
{
    int type;
    char *name;
    flag_t res;
    int effect;
    flag_t dam_flags;
};

struct attack_type
{
    char *name;   /* name          */
    char *noun;   /* message       */
    int dam_type; /* type of DAM_T */
};

struct race_type
{
    char *name;          /* call name of the race          */
    EXT_FLAGS_T ext_mob; /* act bits for the race   */
    flag_t aff;          /* aff bits for the race          */
    flag_t off;          /* off bits for the race          */
    flag_t imm;          /* imm bits for the race          */
    flag_t res;          /* res bits for the race          */
    flag_t vuln;         /* vuln bits for the race         */
    flag_t form;         /* default form flag for the race */
    flag_t parts;        /* default parts for the race     */
};

struct pc_race_type
{               /* additional data for pc races    */
    char *name; /* MUST be in race_type            */
    char who_name[8];
    sh_int creation_points;     /* cost in points of the race      */
    sh_int *class_mult;         /* exp multiplier for class, * 100 (heap, class_count entries) */
    char **skills;              /* bonus skills for the race (heap, pc_race_skill_count entries) */
    sh_int stats[STAT_MAX];     /* starting stats                  */
    sh_int max_stats[STAT_MAX]; /* maximum stats                   */
    sh_int size;                /* aff bits for the race           */
    sh_int bonus_max;           /* bonus to maximum stats          */
};

struct spec_type
{
    char *name;         /* special function name */
    SPEC_FUN *function; /* the function          */
};

/* Data structure for notes. */
struct note_data
{
    BOARD_T *board;
    NOTE_T *board_next, *board_prev;
    sh_int type;
    char *sender;
    char *date;
    char *to_list;
    char *subject;
    char *text;
    time_t date_stamp;
    time_t expire;
    OBJ_RECYCLE_T rec_data;
};

/* An affect.  */
struct affect_data
{
    void *parent;
    int parent_type;
    AFFECT_T *on_next, *on_prev;
    sh_int bit_type;
    sh_int type;
    sh_int level;
    sh_int duration;
    sh_int apply;
    sh_int modifier;
    flag_t bits;
    OBJ_RECYCLE_T rec_data;
};

/* A kill structure (indexed by level). */
struct kill_data
{
    sh_int number;
    sh_int killed;
};

struct flag_type
{
    char *name;
    flag_t bit;
    bool settable;
};

struct ext_flag_def_type
{
    char *name;
    int bit;
    bool settable;
};

struct type_type
{
    char *name;
    type_t type;
    bool settable;
};

struct sector_type
{
    int type;
    char *name;
    int move_loss;
    char colour_char;
};

struct clan_type
{
    char *name;
    char *who_name;
    sh_int hall;
    bool independent; /* true for loners */
};

struct hp_cond_type
{
    int hp_percent;
    char *message;
};

struct position_type
{
    int pos;
    char *long_name;
    char *name;
    char *room_msg;
    char *room_msg_furniture;
};

struct sex_type
{
    int sex;
    char *name;
};

struct size_type
{
    int size;
    char *name;
};

struct door_type
{
    int dir;
    char *name;
    char *from_phrase;
    char *to_phrase;
    int reverse;
    char *short_name;
};

struct dice_type
{
    sh_int number;
    sh_int size;
    sh_int bonus;
};

/* Prototype for a mob.
 * This is the in-memory version of #MOBILES. */
struct mob_index_data
{
    MOB_INDEX_T *hash_next, *hash_prev;
    AREA_T *area; /* OLC */
    MOB_INDEX_T *area_next, *area_prev;
    SPEC_FUN *spec_fun;
    SHOP_T *shop;
    MPROG_LIST_T *mprog_first, *mprog_last;
    CHAR_T *mob_first, *mob_last;
    char *area_str;
    sh_int vnum, anum;
    sh_int group;
    bool new_format;
    sh_int mob_count;
    sh_int killed;
    char *name;
    char *short_descr;
    char *long_descr;
    char *description;
    sh_int alignment;
    sh_int level;
    sh_int hitroll;
    DICE_T hit;
    DICE_T mana;
    DICE_T damage;
    sh_int ac[AC_MAX];
    sh_int attack_type;
    sh_int start_pos;
    sh_int default_pos;
    sh_int sex;
    sh_int race;
    long wealth;
    sh_int size;
    sh_int material;
    flag_t mprog_flags;
    EXT_FLAGS_T ext_mob_plus, ext_mob_final, ext_mob_minus;
    flag_t affected_by_plus, affected_by_final, affected_by_minus;
    flag_t off_flags_plus, off_flags_final, off_flags_minus;
    flag_t imm_flags_plus, imm_flags_final, imm_flags_minus;
    flag_t res_flags_plus, res_flags_final, res_flags_minus;
    flag_t vuln_flags_plus, vuln_flags_final, vuln_flags_minus;
    flag_t form_plus, form_final, form_minus;
    flag_t parts_plus, parts_final, parts_minus;
    OBJ_RECYCLE_T rec_data;
};

/* memory for mobs */
struct mem_data
{
    int id;
    int reaction;
    time_t when;
    OBJ_RECYCLE_T rec_data;
};

/* One character (PC or NPC). */
struct char_data
{
    CHAR_T *global_next, *global_prev;
    CHAR_T *room_next, *room_prev;
    CHAR_T *master;
    CHAR_T *leader;
    CHAR_T *fighting;
    CHAR_T *reply;
    CHAR_T *pet;
    CHAR_T *mprog_target;
    MEM_T *memory;
    SPEC_FUN *spec_fun;
    MOB_INDEX_T *mob_index;
    CHAR_T *mob_index_prev, *mob_index_next;
    DESCRIPTOR_T *desc;
    AFFECT_T *affect_first, *affect_last;
    OBJ_T *content_first, *content_last;
    OBJ_T *on;
    ROOM_INDEX_T *in_room;
    ROOM_INDEX_T *was_in_room;
    AREA_T *area;
    PC_T *pcdata;
    GEN_T *gen_data;
    char *name;
    long id;
    sh_int version;
    char *short_descr;
    char *long_descr;
    char *description;
    char *prompt;
    char *prefix;
    sh_int group;
    sh_int clan;
    sh_int sex;
    sh_int class;
    sh_int race;
    sh_int level;
    sh_int trust;
    int played;
    int lines; /* for the pager */
    time_t logon;
    sh_int timer;
    sh_int wait;
    sh_int daze;
    sh_int hit;
    sh_int max_hit;
    sh_int mana;
    sh_int max_mana;
    sh_int move;
    sh_int max_move;
    long gold;
    long silver;
    int exp;
    EXT_FLAGS_T ext_mob;
    EXT_FLAGS_T ext_plr;
    flag_t comm;   /* RT added to pad the vector */
    flag_t wiznet; /* wiz stuff */
    flag_t imm_flags;
    flag_t res_flags;
    flag_t vuln_flags;
    sh_int invis_level;
    sh_int incog_level;
    flag_t affected_by;
    sh_int position;
    sh_int practice;
    sh_int train;
    sh_int carry_weight;
    sh_int carry_number;
    sh_int saving_throw;
    sh_int alignment;
    sh_int hitroll;
    sh_int damroll;
    sh_int armor[4];

    sh_int wimpy;

    /* quest system (Vassago) */
    CHAR_T *questgiver;
    int questpoints;
    sh_int nextquest;
    sh_int countdown;
    sh_int questobj;
    sh_int questmob;
    sh_int questcount;     /* kills done / items collected toward questcount_max */
    sh_int questcount_max; /* total needed for this quest (1 = classic single-target) */
    int  quest_chances;   /* stored quest chances (spend to request) */
    int  quest_xp_prog;   /* xp accumulated toward next chance */

    /* stats */
    sh_int perm_stat[STAT_MAX];
    sh_int mod_stat[STAT_MAX];

    /* parts stuff */
    flag_t form;
    flag_t parts;
    sh_int size;
    sh_int material;

    /* mobile stuff */
    flag_t off_flags;
    DICE_T damage;
    sh_int attack_type;
    sh_int start_pos;
    sh_int default_pos;
    sh_int mprog_delay;

    /* temporary data for per-second stat regen */
    sh_int gain_hit_remainder;
    sh_int gain_mana_remainder;
    sh_int gain_move_remainder;
    OBJ_RECYCLE_T rec_data;
};

/* Colour settings */
struct colour_setting_type
{
    int index;
    char *name;
    char act_char;
    flag_t default_colour;
};

/* Colour definition */
struct colour_type
{
    flag_t mask;
    flag_t code;
    char *name;
};

/* Lookup information */
struct map_lookup_table
{
    int index;
    char *name;
    const FLAG_T *flags;
};

/* Data which only PC's have. */
struct pc_data
{
    BUFFER_T *buffer;
    char *pwd;
    char *bamfin;
    char *bamfout;
    char *title;
    sh_int perm_hit;
    sh_int perm_mana;
    sh_int perm_move;
    sh_int true_sex;
    int last_level;
    sh_int *cond_hours;  /* COND_MAX entries (heap) */
    sh_int *learned;     /* skill_count entries (heap) */
    sh_int *skill_known; /* skill_count entries (heap) */
    sh_int *group_known; /* skill_group_count entries (heap) */
    sh_int creation_points;
    bool confirm_delete;
    char **alias;      /* MAX_ALIAS entries (heap) */
    char **alias_sub;  /* MAX_ALIAS entries (heap) */
    BOARD_T *board;    /* The current board        */
    time_t *last_note; /* BOARD_MAX entries (heap) */
    NOTE_T *in_progress;
    int security;   /* OLC - Builder security */
    flag_t *colour;       /* COLOUR_SETTING_MAX entries (heap) */
    char *colour_theme;    /* active colour theme name, or "custom" */
    int pkkills;    /* PK kills */
    int pkdeaths;   /* PK deaths */

#ifdef IMC
    IMC_CHARDATA *imcchardata;
#endif
    OBJ_RECYCLE_T rec_data;
};

/* Data for generating characters -- only used during generation */
struct gen_data
{
    bool *skill_chosen; /* skill_count entries (heap) */
    bool *group_chosen; /* skill_group_count entries (heap) */
    OBJ_RECYCLE_T rec_data;
};

struct liq_type
{
    char *name;
    char *color;
    sh_int cond[COND_MAX];
    sh_int serving_size;
};

/* Quest reward record - loaded from json/config/quest_rewards.json at boot. */
struct quest_reward_type
{
    char *id;        /* identifier / primary keyword for 'quest buy <id>' */
    char *label;     /* display text shown in 'quest list' */
    char *keywords;  /* full namelist for 'quest buy' matching */
    int   cost;      /* quest point cost */
    char *type;      /* "object", "gold", "practices" (open-ended) */
    int   value;     /* object vnum, gold amount, or practice count */
};

struct quest_token_type
{
    int   vnum;      /* object vnum for this quest token type */
};

/* Quest system parameters - loaded from json/config/quest_config.json at boot.
 * All fields fall back to hard-coded defaults if the file is absent. */
struct quest_config_type
{
    int quest_timer_min;      /* minimum quest time limit (minutes) */
    int quest_timer_max;      /* maximum quest time limit (minutes) */
    int cooldown_success;     /* cooldown after successful quest (minutes) */
    int cooldown_none;        /* cooldown when no quest available (minutes) */
    int gold_min;             /* minimum gold reward */
    int gold_max;             /* maximum gold reward */
    int qp_min;               /* minimum quest point reward */
    int qp_max;               /* maximum quest point reward */
    int practice_chance;      /* % chance of bonus practices on completion */
    int practice_min;         /* minimum bonus practices */
    int practice_max;         /* maximum bonus practices */
    int obj_quest_chance;     /* % chance of object quest vs mob kill quest */
    int xp_chance_divisor;    /* divisor for XP-to-quest-chance threshold */
    int reward_level_divisor; /* "baseline" level for reward scaling (reward * level / divisor) */
    int purge_quest_chance;  /* % of mob-kill quests that become purge (multi-kill) quests */
    int purge_count_min;     /* min kills required in a purge quest */
    int purge_count_max;     /* max kills required in a purge quest */
    int collect_quest_chance;/* % of obj-recovery quests that become collection (multi-item) quests */
    int collect_count_min;   /* min items needed in a collection quest */
    int collect_count_max;   /* max items needed in a collection quest */
    int xp_reward_min_pct;   /* quest XP: minimum % of a level awarded (e.g. 1 = 1%) */
    int xp_reward_max_pct;   /* quest XP: maximum % of a level awarded (e.g. 2 = 2%, capped at 5%) */
    int train_chance;       /* % chance of awarding 1 training session on quest completion */
};

/* Pocket Dungeon global config. */
struct pd_config {
    bool autopurge;            /* if TRUE, empty instances are purged automatically */
    int  empty_timeout_mins;   /* minutes before an empty instance is purged (default 120) */
    int  max_instances;        /* maximum simultaneously loaded instances (default 50) */
    int  vnum_base;            /* first vnum of the instance reserved range (default 20000) */
    int  vnum_size;            /* vnums per instance slot (default 100) */
    int  max_members;          /* max members that can share one instance (default 10) */
    int  scaling_formula;      /* 0 = average group level, 1 = max group level */
    bool testing_mode;         /* if TRUE, dungeon entry is free (default FALSE) */
    int  gold_cost_per_level;  /* gold per level for dungeon entry (default 100) */
};

/* Pocket Dungeon seed / theme. */
#define PD_MAX_ROOM_NAMES 20
#define PD_MAX_MOB_VNUMS  10
#define PD_MAX_ITEM_VNUMS 10
#define PD_MAX_ROOM_DESCS 15
#define PD_MAX_HIDE_HINTS 5

struct pd_room_desc {
    char           *text;           /* room description body */
    char           *look_keyword;   /* optional "look" trigger keyword */
    char           *look_text;      /* text for "look <keyword>" */
};

struct pd_seed {
    PD_SEED_T      *global_next, *global_prev;
    char           *name;                          /* theme identifier, e.g. "undead_crypts" */
    char           *title;                         /* color-code display name */
    char           *layout_style;                  /* "linear", "spiral", "hub", "ruins", "cavern" */
    int             sector_type;                   /* SECT_INSIDE, SECT_FOREST, etc. */
    bool            outdoors;                      /* if FALSE, set ROOM_INDOORS */
    
    char           *room_names[PD_MAX_ROOM_NAMES]; /* name pool for generated rooms */
    int             room_name_count;
    int             mob_vnums[PD_MAX_MOB_VNUMS];   /* template mob vnums to spawn */
    int             mob_vnum_count;
    int             item_vnums[PD_MAX_ITEM_VNUMS];  /* loot item vnums */
    int             item_vnum_count;
    
    int             room_count_min;   /* minimum rooms to generate */
    int             room_count_max;   /* maximum rooms to generate */
    int             mob_density;      /* legacy; use mob_density_min/max */
    int             mob_density_min;  /* mobs per room at level 1 */
    int             mob_density_max;  /* mobs per room at max level */
    int             loot_density;     /* % chance each room gets a loot item */
    
    struct pd_room_desc room_descs[PD_MAX_ROOM_DESCS];  /* varied room descriptions */
    int             room_desc_count;
    
    char           *entry_room_name;  /* name override for foyer */
    char           *boss_room_name;   /* name override for boss room */
    char           *chest_room_name;  /* name override for treasure chest room */
    
    int             boss_vnum;        /* boss mob template vnum */
    int             boss_level_add;   /* boss level = instance_level + this */
    
    int             sentinel_vnum;    /* chest guardian mob vnum */
    int             sentinel_level_add;  /* sentinel level = instance_level + this */
    
    int             container_vnum;   /* treasure chest object vnum */
    int             hidden_container_vnum;  /* hidden cache object vnum */
    
    char           *hide_keywords[PD_MAX_HIDE_HINTS];  /* "look" hooks for hidden objects */
    char           *hide_look_texts[PD_MAX_HIDE_HINTS];  /* descriptions when looking at hidden hints */
    char           *hide_hint_phrases[PD_MAX_HIDE_HINTS];  /* flavor text when searching finds hints */
    int             hide_hint_count;  /* number of hints available */
    
    int             search_scroll_vnum;  /* Detection scroll vnum */
    int             search_wand_vnum;    /* Detection wand vnum */

    bool            mobprog_enabled;     /* enable procedural mobprog generation */
    int             mobprog_personality_override; /* -1 = auto-select */
    int             mobprog_difficulty_boost;    /* extra levels for mobprog scaling */
    
    OBJ_RECYCLE_T   rec_data;
};

/* Pocket Dungeon live instance. */
#define MAX_INSTANCE_MEMBERS 10
struct pd_instance {
    PD_INSTANCE_T  *global_next, *global_prev;
    int             id;                                  /* unique monotonic ID */
    char           *members[MAX_INSTANCE_MEMBERS];       /* char names (str_dup'd) */
    int             member_count;
    char           *theme;                               /* seed name used */
    int             level;                               /* scaling level at creation */
    time_t          created_at;
    time_t          last_empty_at;   /* 0 while players are inside */
    int             entry_vnum;      /* vnum of the foyer room (room 0) */
    int             origin_vnum;     /* room vnum where 'dungeon enter' was used */
    int             vnum_slot;       /* 0-based slot in AREA_INSTANCE_BASE_VNUM range */
    AREA_T         *area;            /* live area pointer; NULL after purge */
    char            area_name[64];   /* e.g. "pd_instance_007" */
    
    /* C1: Dungeon affixes (random modifiers) */
    int             affixes[5];       /* affix IDs applied to this instance (0=none) */
    int             affix_count;
    
    /* C2: Progressive difficulty (rooms cleared tracker) */
    int             rooms_cleared;    /* incremented each time a room becomes empty of mobs */
    
    /* C3: Boss tracking */
    bool            boss_killed;      /* set to TRUE when boss dies (triggers loot drops) */
    
    /* C4: Boss powers */
    int             boss_powers[5];   /* assigned power IDs for this instance's boss */
    int             boss_power_count;
    
    OBJ_RECYCLE_T   rec_data;
};

/* Extra description data for a room or object. */
struct extra_descr_data
{
    void *parent;
    int parent_type;
    EXTRA_DESCR_T *on_next, *on_prev;
    char *keyword;     /* Keyword in look/examine */
    char *description; /* What to see             */
    OBJ_RECYCLE_T rec_data;
};

/* Object values for all item types. */
struct obj_values_weapon
{
    flag_t weapon_type;
    flag_t dice_num;
    flag_t dice_size;
    flag_t attack_type;
    flag_t flags;
};

struct obj_values_container
{
    flag_t capacity;
    flag_t flags;
    flag_t key;
    flag_t max_weight;
    flag_t weight_mult;
};

struct obj_values_drink_con
{
    flag_t capacity;
    flag_t filled;
    flag_t liquid;
    flag_t poisoned;
    flag_t _value5;
};

struct obj_values_fountain
{
    flag_t capacity;
    flag_t filled;
    flag_t liquid;
    flag_t poisoned;
    flag_t _value5;
};

struct obj_values_wand
{
    flag_t level;
    flag_t recharge;
    flag_t charges;
    flag_t skill;
    flag_t _value5;
};

struct obj_values_staff
{
    flag_t level;
    flag_t recharge;
    flag_t charges;
    flag_t skill;
    flag_t _value5;
};

struct obj_values_food
{
    flag_t hunger;
    flag_t fullness;
    flag_t _value3;
    flag_t poisoned;
    flag_t _value5;
};

struct obj_values_money
{
    flag_t silver;
    flag_t gold;
    flag_t _value_3;
    flag_t _value_4;
    flag_t _value_5;
};

struct obj_values_armor
{
    flag_t vs_pierce;
    flag_t vs_bash;
    flag_t vs_slash;
    flag_t vs_magic;
    flag_t _value_5;
};

struct obj_values_potion
{
    flag_t level;
    flag_t skill[POTION_SKILL_MAX];
};

struct obj_values_pill
{
    flag_t level;
    flag_t skill[PILL_SKILL_MAX];
};

struct obj_values_scroll
{
    flag_t level;
    flag_t skill[SCROLL_SKILL_MAX];
};

struct obj_values_map
{
    flag_t persist;
    flag_t _value_2;
    flag_t _value_3;
    flag_t _value_4;
    flag_t _value_5;
};

struct obj_values_furniture
{
    flag_t max_people;
    flag_t max_weight;
    flag_t flags;
    flag_t heal_rate;
    flag_t mana_rate;
};

struct obj_values_light
{
    flag_t _value_1;
    flag_t _value_2;
    flag_t duration;
    flag_t _value_4;
    flag_t _value_5;
};

struct obj_values_portal
{
    flag_t charges;
    flag_t exit_flags;
    flag_t gate_flags;
    flag_t to_vnum;
    flag_t key;
};

struct obj_values_jukebox
{
    flag_t line;
    flag_t song;
    flag_t queue[JUKEBOX_QUEUE_MAX];
};

union obj_value_type
{
    flag_t value[OBJ_VALUE_MAX];
    struct obj_values_weapon weapon;
    struct obj_values_container container;
    struct obj_values_drink_con drink_con;
    struct obj_values_fountain fountain;
    struct obj_values_wand wand;
    struct obj_values_staff staff;
    struct obj_values_food food;
    struct obj_values_money money;
    struct obj_values_armor armor;
    struct obj_values_potion potion;
    struct obj_values_pill pill;
    struct obj_values_scroll scroll;
    struct obj_values_map map;
    struct obj_values_furniture furniture;
    struct obj_values_light light;
    struct obj_values_portal portal;
    struct obj_values_jukebox jukebox;
};

/* Reset values for all reset types. */
struct reset_values_mob
{
    sh_int _value1;
    sh_int mob_vnum;
    sh_int global_limit;
    sh_int room_vnum;
    sh_int room_limit;
};

struct reset_values_obj
{
    sh_int room_limit;
    sh_int obj_vnum;
    sh_int global_limit;
    sh_int room_vnum;
    sh_int _value5;
};

struct reset_values_give
{
    sh_int _value1;
    sh_int obj_vnum;
    sh_int global_limit;
    sh_int _value4;
    sh_int _value5;
};

struct reset_values_equip
{
    sh_int _value1;
    sh_int obj_vnum;
    sh_int global_limit;
    sh_int wear_loc;
    sh_int _value5;
};

struct reset_values_put
{
    sh_int _value1;
    sh_int obj_vnum;
    sh_int global_limit;
    sh_int into_vnum;
    sh_int put_count;
};

struct reset_values_door
{
    sh_int _value1;
    sh_int room_vnum;
    sh_int dir;
    sh_int locks;
    sh_int _value5;
};

struct reset_values_randomize
{
    sh_int _value1;
    sh_int room_vnum;
    sh_int dir_count;
    sh_int _value4;
    sh_int _value5;
};

union reset_value_type
{
    sh_int value[RESET_VALUE_MAX];
    struct reset_values_mob mob;
    struct reset_values_obj obj;
    struct reset_values_give give;
    struct reset_values_equip equip;
    struct reset_values_put put;
    struct reset_values_randomize randomize;
    struct reset_values_door door;
};

/* Prototype for an object. */
struct obj_index_data
{
    OBJ_INDEX_T *hash_next, *hash_prev;
    AREA_T *area; /* OLC */
    OBJ_INDEX_T *area_next, *area_prev;
    EXTRA_DESCR_T *extra_descr_first, *extra_descr_last;
    AFFECT_T *affect_first, *affect_last;
    OBJ_T *obj_first, *obj_last;
    char *area_str;
    bool new_format;
    char *name;
    char *short_descr;
    char *description;
    sh_int vnum, anum;
    sh_int reset_num;
    sh_int material;
    sh_int item_type;
    flag_t extra_flags;
    flag_t wear_flags;
    sh_int level;
    sh_int condition;
    sh_int obj_count;
    sh_int weight;
    int cost;
    OBJ_RECYCLE_T rec_data;
    OBJ_VALUE_T v;
};

/* Object stat <-> value[] mapping. */
struct obj_map_value
{
    int index;
    flag_t default_value;
    char *name;
    int type, sub_type;
};

struct obj_map
{
    int item_type;
    const struct obj_map_value values[OBJ_VALUE_MAX];
};

/* One object. */
struct obj_data
{
    OBJ_T *global_next, *global_prev;
    CHAR_T *carried_by;
    OBJ_T *in_obj;
    OBJ_T *content_next, *content_prev;
    OBJ_T *content_first, *content_last;
    OBJ_T *on;
    EXTRA_DESCR_T *extra_descr_first, *extra_descr_last;
    AFFECT_T *affect_first, *affect_last;
    OBJ_INDEX_T *obj_index;
    OBJ_T *obj_index_prev, *obj_index_next;
    ROOM_INDEX_T *in_room;
    bool enchanted;
    char *owner;
    char *name;
    char *short_descr;
    char *description;
    sh_int item_type;
    flag_t extra_flags;
    flag_t wear_flags;
    sh_int wear_loc;
    sh_int weight;
    int cost;
    sh_int level;
    sh_int condition;
    sh_int material;
    sh_int timer;
    sh_int pd_saved_timer; /* preserved timer for pocket dungeon corpse freeze; restored on pickup */
    OBJ_RECYCLE_T rec_data;
    OBJ_VALUE_T v;
};

/* Exit data. */
struct exit_data
{
    ROOM_INDEX_T *from_room, *to_room;
    sh_int to_vnum, to_anum, to_area_vnum;
    flag_t exit_flags;
    sh_int key;
    char *keyword;
    char *description;
    flag_t rs_flags; /* OLC */
    int orig_door;   /* OLC */
    PORTAL_EXIT_T *portal;
    OBJ_RECYCLE_T rec_data;
};

/* Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list) */

/* Area-reset definition. */
struct reset_data
{
    AREA_T *area;
    RESET_T *area_next, *area_prev;
    ROOM_INDEX_T *room;
    RESET_T *room_next, *room_prev;
    char command;
    int room_vnum;
    RESET_VALUE_T v;
    OBJ_RECYCLE_T rec_data;
};

/* Area definition.  */
struct area_data
{
    AREA_T *global_next, *global_prev;
    HELP_AREA_T *had_first, *had_last;
    MOB_INDEX_T *mob_first, *mob_last;
    OBJ_INDEX_T *obj_first, *obj_last;
    ROOM_INDEX_T *room_first, *room_last;
    MPROG_LIST_T *mprog_first, *mprog_last;
    MPROG_CODE_T *mpcode_first, *mpcode_last;
    RESET_T *reset_first, *reset_last;
    char *name;
    char *filename;
    char *title;
    char *credits;
    sh_int age;
    sh_int nplayer;
    sh_int low_range;
    sh_int high_range;
    sh_int min_vnum;
    sh_int max_vnum;
    bool had_players;
    char *builders;    /* OLC - Listing of */
    char *repop_msg;   /* OLC - Repop message */
    int vnum;          /* OLC - Area vnum  */
    flag_t area_flags; /* OLC              */
    int security;      /* OLC - Value 1-9  */
    OBJ_RECYCLE_T rec_data;
};

/* Room type. */
struct room_index_data
{
    ROOM_INDEX_T *hash_next, *hash_prev;
    AREA_T *area;
    ROOM_INDEX_T *area_next, *area_prev;
    CHAR_T *people_first, *people_last;
    OBJ_T *content_first, *content_last;
    RESET_T *reset_first, *reset_last;
    EXTRA_DESCR_T *extra_descr_first, *extra_descr_last;
    EXIT_T *exit[DIR_MAX];
    char *area_str;
    char *name;
    char *description;
    char *owner;
    sh_int vnum, anum;
    flag_t room_flags;
    sh_int light;
    sh_int sector_type;
    sh_int heal_rate;
    sh_int mana_rate;
    sh_int clan;
    PORTAL_EXIT_T *portal;
    OBJ_RECYCLE_T rec_data;
};

struct skill_class_type
{
    sh_int level;  /* Level needed by class       */
    sh_int effort; /* How hard it is to learn     */
};

/* Skills include spells as a particular case. */
struct skill_type
{
    char *name;              /* Name of skill               */
    SKILL_CLASS_T *classes;  /* Restrictions based on class (heap, class_count entries) */
    SPELL_FUN *spell_fun;    /* Spell pointer (for spells)  */
    sh_int target;           /* Legal targets               */
    sh_int minimum_position; /* Position for caster / user  */
    sh_int slot;             /* Slot for #OBJECT loading    */
    sh_int min_mana;         /* Minimum mana used           */
    sh_int beats;            /* Waiting time after use      */
    char *noun_damage;       /* Damage message              */
    char *msg_off;           /* Wear off message            */
    char *msg_obj;           /* Wear off message for obects */
    int map_index;           /* Dynamically set             */
    int weapon_index;        /* Dynamically set             */
};

struct skill_group_class_type
{
    sh_int cost; /* How hard it is to learn */
};

struct skill_group_type
{
    char *name;
    SKILL_GROUP_CLASS_T *classes; /* heap, class_count entries */
    char **spells;                /* heap, spell_count entries (null-terminated) */
    int spell_count;              /* number of spells in group */
};

struct skill_map_type
{
    int map_index;
    char *name;
    int skill_index; /* dynamically set */
};

struct mprog_list
{
    AREA_T *area;
    MPROG_LIST_T *area_next, *area_prev;
    MPROG_LIST_T *mob_next, *mob_prev;
    int trig_type;
    char *trig_phrase;
    sh_int vnum, anum;
    char *code;
    OBJ_RECYCLE_T rec_data;
};

struct mprog_code
{
    AREA_T *area;
    MPROG_CODE_T *global_next, *global_prev;
    MPROG_CODE_T *area_next, *area_prev;
    sh_int vnum, anum;
    char *code;
    OBJ_RECYCLE_T rec_data;
};

struct nanny_handler
{
    int state;
    char *name;
    NANNY_FUN *const action;
};

struct furniture_bits
{
    int position;
    char *name;
    flag_t bit_at;
    flag_t bit_on;
    flag_t bit_in;
};

/* Structure for a social in the socials table. */
struct social_type
{
    char *name;
    char *char_no_arg;
    char *others_no_arg;
    char *char_found;
    char *others_found;
    char *vict_found;
    char *char_not_found;
    char *char_auto;
    char *others_auto;
    int min_pos;
    OBJ_RECYCLE_T rec_data;
};

/* Data about a board */
struct board_data
{
    char *name;      /* Max 8 chars */
    char *long_name; /* Explanatory text, should be no more than 40 ? chars */
    int read_level;  /* minimum level to see board */
    int write_level; /* minimum level to post notes */
    char *names;     /* Default recipient */
    int force_type;  /* Default action (DEF_XXX) */
    int purge_days;  /* Default expiration */

    /* Non-constant data */
    BOARD_T *global_next, *global_prev;
    NOTE_T *note_first, *note_last; /* pointer to board's first note */
    bool changed;                   /* currently unused */
};

/* Things we can wear, wield, hold, etc. */
struct wear_loc_type
{
    int type;
    char *name;
    char *phrase;
    char *look_msg;
    flag_t wear_flag;
    int ac_bonus;
    char *msg_wear_self, *msg_wear_room;
};

/* Material types - currently unused. */
struct material_type
{
    int type;
    char *name;
    char color;
};

struct flag_stat_type
{
    const FLAG_T *structure;
    bool stat;
};

struct table_type
{
    const void *table;
    const char *name;
    int type;
    const char *description;

    size_t type_size, table_length;
    const char *obj_name;
    const char *json_path;
    JSON_WRITE_FUN *json_write_func;
    JSON_READ_FUN *json_read_func;
    DISPOSE_FUN *dispose_fun;
    POST_LOAD_FUN *post_load_fun;

    /* Dynamic-table support. NULL for static tables. */
    void **table_pp;                  /* e.g. (void **)&race_table */
    int *count_p;                     /* &race_count  */
    int *cap_p;                       /* &race_cap    */
    void (*invalidate_max_fun)(void); /* e.g. race_invalidate_max */
};

struct portal_exit_type
{
    ROOM_INDEX_T *room;
    EXIT_T *exit;
    char *name;
    OBJ_RECYCLE_T rec_data;
};

struct portal_type
{
    char *name_from, *name_to;
    bool two_way;
    PORTAL_EXIT_T *from, *to;
    OBJ_RECYCLE_T rec_data;
};

struct affect_bit_type
{
    char *name;
    int type;
    const FLAG_T *flags;
    char *help;
};

struct day_type
{
    int type;
    char *name;
};

struct month_type
{
    int type;
    char *name;
};

struct sky_type
{
    int type;
    char *name;
    char *description;
    int mmhg_min;
    int mmhg_max;
};

struct sun_type
{
    int type;
    char *name;
    bool is_dark;
    int hour_start;
    int hour_end;
    char *message;
};

struct mob_cmd_type
{
    const char *name;
    DO_FUN *do_fun;
};

struct anum_type
{
    int type;
    sh_int *vnum_ref;
    char *area_str;
    int anum;
    ANUM_T *global_prev, *global_next;
};

/* Structure for a command in the command lookup table. */
struct cmd_type
{
    char *const name;
    DO_FUN *do_fun;
    sh_int position;
    sh_int level;
    sh_int log;
    sh_int show;
};

/* Structure for an OLC editor command. */
struct olc_cmd_type
{
    char *const name;
    OLC_FUN *olc_fun;
};

/* Structure for an OLC editor startup command. */
struct editor_cmd_type
{
    char *const name;
    DO_FUN *do_fun;
};

/* All the posing stuff. */
struct pose_type
{
    char *class_name;
    char *message[MAX_LEVEL * 2 + 2];
};

/* Music stuff. */
struct song_type
{
    char *group;
    char *name;
    char **lyrics; /* heap, MAX_SONG_LINES slots (null-terminated) */
    int lines;
};

/* Conditions like hunger/thirst. */
struct cond_type
{
    int type;
    char *name;
    COND_FUN *good_fun, *bad_fun;
    char *msg_good, *msg_bad;
    char *msg_better, *msg_worse;
};

/* Stats for training. */
struct train_stat_type
{
    const char *keyword;
    const char *name;
    TRAIN_STAT_FUN *cost_func;
    TRAIN_STAT_FUN *can_func;
    TRAIN_STAT_FUN *do_func;
    int func_param;
};

/* Login greeting. */
struct greeting_t
{
    char *text;
};

/* Spells for the 'heal' command. */
struct heal_spell_type
{
    char *keywords;
    const char *description;
    SPELL_FUN *spell_func;
    const char *skill_name;
    int cost_gold;
};

#endif
