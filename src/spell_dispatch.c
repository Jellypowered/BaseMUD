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

#include "spell_dispatch.h"

#include "magic.h"
#include "spell_aff.h"
#include "spell_create.h"
#include "spell_cure.h"
#include "spell_info.h"
#include "spell_misc.h"
#include "spell_move.h"
#include "spell_npc.h"
#include "spell_off.h"
#include "spell_summon.h"

#include <string.h>

typedef struct spell_entry_type
{
    const char *name;
    SPELL_FUN *fun;
} SPELL_ENTRY_T;

static const SPELL_ENTRY_T spell_dispatch_table[] = {
    /* spell_aff.h */
    {"spell_armor", spell_armor},
    {"spell_bless", spell_bless},
    {"spell_bless_char", spell_bless_char},
    {"spell_bless_object", spell_bless_object},
    {"spell_blindness", spell_blindness},
    {"spell_blindness_quiet", spell_blindness_quiet},
    {"spell_calm", spell_calm},
    {"spell_change_sex", spell_change_sex},
    {"spell_charm_person", spell_charm_person},
    {"spell_curse", spell_curse},
    {"spell_curse_char", spell_curse_char},
    {"spell_curse_char_quiet", spell_curse_char_quiet},
    {"spell_curse_object", spell_curse_object},
    {"spell_deter", spell_deter},
    {"spell_detect_evil", spell_detect_evil},
    {"spell_detect_good", spell_detect_good},
    {"spell_detect_hidden", spell_detect_hidden},
    {"spell_detect_invis", spell_detect_invis},
    {"spell_detect_magic", spell_detect_magic},
    {"spell_enchant_armor", spell_enchant_armor},
    {"spell_enchant_weapon", spell_enchant_weapon},
    {"spell_faerie_fire", spell_faerie_fire},
    {"spell_faerie_fog", spell_faerie_fog},
    {"spell_fireproof", spell_fireproof},
    {"spell_fly", spell_fly},
    {"spell_frenzy", spell_frenzy},
    {"spell_giant_strength", spell_giant_strength},
    {"spell_haste", spell_haste},
    {"spell_infravision", spell_infravision},
    {"spell_invis", spell_invis},
    {"spell_invis_char", spell_invis_char},
    {"spell_invis_object", spell_invis_object},
    {"spell_mass_invis", spell_mass_invis},
    {"spell_pass_door", spell_pass_door},
    {"spell_plague", spell_plague},
    {"spell_poison", spell_poison},
    {"spell_poison_char", spell_poison_char},
    {"spell_poison_object", spell_poison_object},
    {"spell_protection_evil", spell_protection_evil},
    {"spell_protection_good", spell_protection_good},
    {"spell_sanctuary", spell_sanctuary},
    {"spell_shield", spell_shield},
    {"spell_sleep", spell_sleep},
    {"spell_slow", spell_slow},
    {"spell_stone_skin", spell_stone_skin},
    {"spell_weaken", spell_weaken},
    {"spell_silence", spell_silence},
    {"spell_cure_mute", spell_cure_mute},
    {"spell_quench", spell_quench},
    {"spell_sate", spell_sate},
    {"spell_resurrect", spell_resurrect},
    {"spell_light", spell_light},
    {"spell_deafness", spell_deafness},
    {"spell_glitterdust", spell_glitterdust},

    /* spell_create.h */
    {"spell_continual_light", spell_continual_light},
    {"spell_create_food", spell_create_food},
    {"spell_create_rose", spell_create_rose},
    {"spell_create_spring", spell_create_spring},
    {"spell_create_water", spell_create_water},
    {"spell_floating_disc", spell_floating_disc},

    /* spell_cure.h */
    {"spell_cure_blindness", spell_cure_blindness},
    {"spell_cure_critical", spell_cure_critical},
    {"spell_cure_disease", spell_cure_disease},
    {"spell_cure_light", spell_cure_light},
    {"spell_cure_poison", spell_cure_poison},
    {"spell_cure_serious", spell_cure_serious},
    {"spell_heal", spell_heal},
    {"spell_mass_healing", spell_mass_healing},
    {"spell_refresh", spell_refresh},
    {"spell_remove_curse", spell_remove_curse},
    {"spell_remove_curse_char", spell_remove_curse_char},
    {"spell_remove_curse_object", spell_remove_curse_object},
    {"spell_restore_mana", spell_restore_mana},

    /* spell_info.h */
    {"spell_detect_poison", spell_detect_poison},
    {"spell_farsight", spell_farsight},
    {"spell_identify", spell_identify},
    {"spell_know_alignment", spell_know_alignment},
    {"spell_locate_object", spell_locate_object},
    {"spell_detect_undead", spell_detect_undead},
    {"spell_locate_person", spell_locate_person},
    {"spell_legend_lore", spell_legend_lore},

    /* spell_misc.h */
    {"spell_cancellation", spell_cancellation},
    {"spell_control_weather", spell_control_weather},
    {"spell_dispel_magic", spell_dispel_magic},
    {"spell_recharge", spell_recharge},
    {"spell_ventriloquate", spell_ventriloquate},
    {"spell_vampiric_touch", spell_vampiric_touch},
    {"spell_magic_mouth", spell_magic_mouth},
    {"spell_explosive_runes", spell_explosive_runes},
    {"spell_wish", spell_wish},

    /* spell_move.h */
    {"spell_fear", spell_fear},
    {"spell_gate", spell_gate},
    {"spell_nexus", spell_nexus},
    {"spell_portal", spell_portal},
    {"spell_summon", spell_summon},
    {"spell_teleport", spell_teleport},
    {"spell_word_of_recall", spell_word_of_recall},

    /* spell_npc.h */
    {"spell_acid_breath", spell_acid_breath},
    {"spell_fire_breath", spell_fire_breath},
    {"spell_frost_breath", spell_frost_breath},
    {"spell_gas_breath", spell_gas_breath},
    {"spell_general_purpose", spell_general_purpose},
    {"spell_high_explosive", spell_high_explosive},
    {"spell_lightning_breath", spell_lightning_breath},

    /* spell_off.h */
    {"spell_acid_blast", spell_acid_blast},
    {"spell_acid_rain", spell_acid_rain},
    {"spell_burning_hands", spell_burning_hands},
    {"spell_call_lightning", spell_call_lightning},
    {"spell_cause_critical", spell_cause_critical},
    {"spell_cause_light", spell_cause_light},
    {"spell_cause_serious", spell_cause_serious},
    {"spell_chain_lightning", spell_chain_lightning},
    {"spell_chill_touch", spell_chill_touch},
    {"spell_colour_spray", spell_colour_spray},
    {"spell_demonfire", spell_demonfire},
    {"spell_dispel_evil", spell_dispel_evil},
    {"spell_dispel_good", spell_dispel_good},
    {"spell_earthquake", spell_earthquake},
    {"spell_energy_drain", spell_energy_drain},
    {"spell_fireball", spell_fireball},
    {"spell_flamestrike", spell_flamestrike},
    {"spell_harm", spell_harm},
    {"spell_heat_metal", spell_heat_metal},
    {"spell_holy_word", spell_holy_word},
    {"spell_lightning_bolt", spell_lightning_bolt},
    {"spell_magic_missile", spell_magic_missile},
    {"spell_ray_of_truth", spell_ray_of_truth},
    {"spell_shocking_grasp", spell_shocking_grasp},
    {"spell_acid_arrow", spell_acid_arrow},
    {"spell_flame_arrow", spell_flame_arrow},

    /* spell_summon.h */
    {"spell_find_familiar", spell_find_familiar},
    {"spell_mount", spell_mount},
    {"spell_animate_dead", spell_animate_dead},

    /* magic.h */
    {"spell_null", spell_null},

    {NULL, NULL}};

SPELL_FUN *spell_lookup_function(const char *name)
{
    const SPELL_ENTRY_T *e;
    for (e = spell_dispatch_table; e->name != NULL; e++)
        if (strcmp(e->name, name) == 0)
            return e->fun;
    return NULL;
}

const char *spell_function_name(SPELL_FUN *fun)
{
    const SPELL_ENTRY_T *e;
    if (fun == NULL || fun == spell_null)
        return NULL;
    for (e = spell_dispatch_table; e->name != NULL; e++)
        if (e->fun == fun)
            return e->name;
    return NULL;
}
