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

#include "spell_misc.h"

#include "affects.h"
#include "chars.h"
#include "comm.h"
#include "fight.h"
#include "globals.h"
#include "interp.h"
#include "items.h"
#include "lookup.h"
#include "magic.h"
#include "objs.h"
#include "spell_summon.h"
#include "utils.h"

DEFINE_SPELL_FUN (spell_cancellation) {
    CHAR_T *victim = (CHAR_T *) vo;
    bool found = FALSE;

    level += 2;
    BAIL_IF (IS_NPC (ch) && !IS_NPC (victim),
        "You failed, try dispel magic.\n\r", ch);
    BAIL_IF (!IS_NPC (ch) && IS_NPC (victim) &&
             !(IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim),
        "You failed, try dispel magic.\n\r", ch);

    /* unlike dispel magic, the victim gets NO save */

    /* begin running through the spells */
    found += check_dispel_quick (level, victim, "armor",           NULL);
    found += check_dispel_quick (level, victim, "bless",           NULL);
    found += check_dispel_quick (level, victim, "blindness",       "$n is no longer blinded.");
    found += check_dispel_quick (level, victim, "calm",            "$n no longer looks so peaceful...");
    found += check_dispel_quick (level, victim, "change sex",      "$n looks more like $mself again.");
    found += check_dispel_quick (level, victim, "charm person",    "$n regains $s free will.");
    found += check_dispel_quick (level, victim, "chill touch",     "$n looks warmer.");
    found += check_dispel_quick (level, victim, "curse",           NULL);
    found += check_dispel_quick (level, victim, "detect evil",     NULL);
    found += check_dispel_quick (level, victim, "detect good",     NULL);
    found += check_dispel_quick (level, victim, "detect hidden",   NULL);
    found += check_dispel_quick (level, victim, "detect invis",    NULL);
    found += check_dispel_quick (level, victim, "detect magic",    NULL);
    found += check_dispel_quick (level, victim, "faerie fire",     "$n's outline fades.");
    found += check_dispel_quick (level, victim, "fly",             "$n falls to the ground!");
    found += check_dispel_quick (level, victim, "frenzy",          "$n no longer looks so wild.");
    found += check_dispel_quick (level, victim, "giant strength",  "$n no longer looks so mighty.");
    found += check_dispel_quick (level, victim, "haste",           "$n is no longer moving so quickly.");
    found += check_dispel_quick (level, victim, "infravision",     NULL);
    found += check_dispel_quick (level, victim, "invis",           "$n fades into existance.");
    found += check_dispel_quick (level, victim, "mass invis",      "$n fades into existance.");
    found += check_dispel_quick (level, victim, "pass door",       NULL);
    found += check_dispel_quick (level, victim, "protection evil", NULL);
    found += check_dispel_quick (level, victim, "protection good", NULL);
    found += check_dispel_quick (level, victim, "sanctuary",       "The white aura around $n's body vanishes.");
    found += check_dispel_quick (level, victim, "shield",          "The shield protecting $n vanishes.");
    found += check_dispel_quick (level, victim, "sleep",           NULL);
    found += check_dispel_quick (level, victim, "slow",            "$n is no longer moving so slowly.");
    found += check_dispel_quick (level, victim, "stone skin",      "$n's skin regains its normal texture.");
    found += check_dispel_quick (level, victim, "weaken",          "$n looks stronger.");

    printf_to_char(ch, found ? "Success!\n\r" : "Spell failed.\n\r");
}

DEFINE_SPELL_FUN (spell_control_weather) {
    if (!str_cmp (target_name, "better")) {
        weather_info.change += dice (level / 3, 4);
        send_to_char ("You sense the clouds clearing up...\n\r", ch);
    }
    else if (!str_cmp (target_name, "worse")) {
        weather_info.change -= dice (level / 3, 4);
        send_to_char ("You sense a storm gathering...\n\r", ch);
    }
    else
        send_to_char ("Do you want it to get better or worse?\n\r", ch);
}

/* modified for enhanced use */
DEFINE_SPELL_FUN (spell_dispel_magic) {
    CHAR_T *victim = (CHAR_T *) vo;
    bool found = FALSE;

    if (saves_spell (level, victim, DAM_OTHER)) {
        send_to_char ("You feel a brief tingling sensation.\n\r", victim);
        send_to_char ("You notice no change.\n\r", ch);
        return;
    }

    /* begin running through the spells */
    found += check_dispel_quick (level, victim, "armor",           NULL);
    found += check_dispel_quick (level, victim, "bless",           NULL);
    found += check_dispel_quick (level, victim, "blindness",       "$n is no longer blinded.");
    found += check_dispel_quick (level, victim, "calm",            "$n no longer looks so peaceful...");
    found += check_dispel_quick (level, victim, "change sex",      "$n looks more like $mself again.");
    found += check_dispel_quick (level, victim, "charm person",    "$n regains $s free will.");
    found += check_dispel_quick (level, victim, "chill touch",     "$n looks warmer.");
    found += check_dispel_quick (level, victim, "curse",           NULL);
    found += check_dispel_quick (level, victim, "detect evil",     NULL);
    found += check_dispel_quick (level, victim, "detect good",     NULL);
    found += check_dispel_quick (level, victim, "detect hidden",   NULL);
    found += check_dispel_quick (level, victim, "detect invis",    NULL);
    found += check_dispel_quick (level, victim, "detect magic",    NULL);
    found += check_dispel_quick (level, victim, "faerie fire",     "$n's outline fades.");
    found += check_dispel_quick (level, victim, "fly",             "$n falls to the ground!");
    found += check_dispel_quick (level, victim, "frenzy",          "$n no longer looks so wild.");
    found += check_dispel_quick (level, victim, "giant strength",  "$n no longer looks so mighty.");
    found += check_dispel_quick (level, victim, "haste",           "$n is no longer moving so quickly.");
    found += check_dispel_quick (level, victim, "infravision",     NULL);
    found += check_dispel_quick (level, victim, "invis",           "$n fades into existance.");
    found += check_dispel_quick (level, victim, "mass invis",      "$n fades into existance.");
    found += check_dispel_quick (level, victim, "pass door",       NULL);
    found += check_dispel_quick (level, victim, "protection evil", NULL);
    found += check_dispel_quick (level, victim, "protection good", NULL);
    found += check_dispel_quick (level, victim, "sanctuary",       "The white aura around $n's body vanishes.");
    found += check_dispel_quick (level, victim, "shield",          "The shield protecting $n vanishes.");
    found += check_dispel_quick (level, victim, "sleep",           NULL);
    found += check_dispel_quick (level, victim, "slow",            "$n is no longer moving so slowly.");
    found += check_dispel_quick (level, victim, "stone skin",      "$n's skin regains its normal texture.");
    found += check_dispel_quick (level, victim, "weaken",          "$n looks stronger.");

    if (IS_AFFECTED (victim, AFF_SANCTUARY)
        && !saves_dispel (level, victim->level, -1)
        && !affect_is_char_affected (victim, skill_lookup ("sanctuary")))
    {
        REMOVE_BIT (victim->affected_by, AFF_SANCTUARY);
        act ("The white aura around $n's body vanishes.",
             victim, NULL, NULL, TO_NOTCHAR);
        found = TRUE;
    }

    printf_to_char(ch, found ? "Success!\n\r" : "Spell failed.\n\r");
}

DEFINE_SPELL_FUN (spell_recharge) {
    OBJ_T *obj = (OBJ_T *) vo;
    int chance, percent;
    flag_t wlevel, *recharge_ptr, *charges_ptr;

    BAIL_IF (!item_get_recharge_values (obj, &wlevel, &recharge_ptr, &charges_ptr),
        "That item does not carry charges.\n\r", ch);
    BAIL_IF (wlevel >= 3 * level / 2,
        "Your skills are not great enough for that.\n\r", ch);
    BAIL_IF (*recharge_ptr == 0,
        "That item has already been recharged once.\n\r", ch);

    chance = 40 + 2 * level;
    chance -= wlevel; /* harder to do high-level spells */
    chance -= (*recharge_ptr - *charges_ptr) *
              (*recharge_ptr - *charges_ptr);
    chance = UMAX (level / 2, chance);

    percent = number_percent ();
    if (percent < chance / 2) {
        act ("$p glows softly.", ch, obj, NULL, TO_ALL);
        *charges_ptr  = UMAX (*recharge_ptr, *charges_ptr);
        *recharge_ptr = 0;
        return;
    }
    else if (percent <= chance) {
        int chargeback, chargemax;
        act ("$p glows softly for a moment.", ch, obj, NULL, TO_ALL);

        chargemax = *recharge_ptr - *charges_ptr;
        if (chargemax > 0)
            chargeback = UMAX (1, chargemax * percent / 100);
        else
            chargeback = 0;

        *charges_ptr += chargeback;
        *recharge_ptr = 0;
        return;
    }
    else if (percent <= UMIN (95, 3 * chance / 2)) {
        send_to_char ("Nothing seems to happen.\n\r", ch);
        if (*recharge_ptr > 0)
            (*recharge_ptr)--;
        return;
    }
    else { /* whoops! */
        act ("$p glows brightly and explodes!", ch, obj, NULL, TO_ALL);
        obj_extract (obj);
    }
}

DEFINE_SPELL_FUN (spell_ventriloquate) {
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_T *vch;

    target_name = one_argument (target_name, speaker);
    sprintf (buf1, "%s says '%s'.\n\r", speaker, target_name);
    sprintf (buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name);
    buf1[0] = UPPER (buf1[0]);

    for (vch = ch->in_room->people_first; vch != NULL; vch = vch->room_next)
        if (!str_in_namelist_exact (speaker, vch->name) && IS_AWAKE (vch))
            send_to_char (saves_spell (level, vch, DAM_OTHER) ? buf2 : buf1, vch);
}

DEFINE_SPELL_FUN (spell_vampiric_touch) {
    CHAR_T *victim = (CHAR_T *) vo;
    int dam, drain;

    if (saves_spell (level, victim, DAM_NEGATIVE)) {
        act ("$N resists your vampiric touch.", ch, NULL, victim, TO_CHAR);
        act ("You resist $n's vampiric touch!", ch, NULL, victim, TO_VICT);
        return;
    }
    dam   = dice (1, 4) * level / 5;
    drain = dam / 2;
    act ("You draw life from $N with a vampiric touch!", ch, NULL, victim, TO_CHAR);
    act ("$n drains your life with a vampiric touch!", ch, NULL, victim, TO_VICT);
    act ("$n drains life from $N with a vampiric touch!", ch, NULL, victim, TO_NOTCHAR);
    ch->hit = UMIN (ch->max_hit, ch->hit + drain);
    damage_visible (ch, victim, dam, sn, DAM_NEGATIVE, NULL);
}

DEFINE_SPELL_FUN (spell_magic_mouth) {
    OBJ_INDEX_T *ward_index;
    OBJ_T *ward;

    ward_index = obj_get_index (OBJ_VNUM_MAGIC_MOUTH);
    BAIL_IF (ward_index == NULL, "The magic mouth ward cannot be created.\n\r", ch);
    ward = obj_create (ward_index, level);
    ward->timer = level + 10;
    obj_give_to_room (ward, ch->in_room);
    act ("You conjure a magic mouth to ward this room.", ch, NULL, NULL, TO_CHAR);
    act ("$n conjures a magic mouth to ward this room.", ch, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_explosive_runes) {
    OBJ_INDEX_T *rune_index;
    OBJ_T *rune;

    rune_index = obj_get_index (OBJ_VNUM_EXPLOSIVE_RUNE);
    BAIL_IF (rune_index == NULL, "The explosive rune cannot be created.\n\r", ch);
    rune = obj_create (rune_index, level);
    rune->timer = level;
    obj_give_to_char (rune, ch);
    act ("You inscribe explosive runes onto a parchment.", ch, NULL, NULL, TO_CHAR);
    act ("$n inscribes runes onto a parchment.", ch, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_wish) {
    AFFECT_T af;
    int roll = number_range (0, 7);
    int stat;

    switch (roll) {
        case 0:
            ch->hit = UMIN (ch->max_hit, ch->hit + ch->max_hit * 9 / 10);
            send_to_char ("Your wish grants you vitality!\n\r", ch);
            break;
        case 1:
            ch->mana = UMIN (ch->max_mana, ch->mana + ch->max_mana * 3 / 4);
            send_to_char ("Your wish restores your magical energy!\n\r", ch);
            break;
        case 2:
            check_dispel_quick (level, ch, "blindness", NULL);
            check_dispel_quick (level, ch, "curse",     NULL);
            check_dispel_quick (level, ch, "poison",    NULL);
            check_dispel_quick (level, ch, "plague",    NULL);
            check_dispel_quick (level, ch, "weaken",    NULL);
            send_to_char ("Your wish cleanses you of ailments!\n\r", ch);
            break;
        case 3:
            stat = number_range (APPLY_STR, APPLY_CON);
            affect_init (&af, AFF_TO_AFFECTS, sn, level, 50, stat, 2, 0);
            affect_copy_to_char (&af, ch);
            send_to_char ("Your wish grants you a surge of power!\n\r", ch);
            break;
        case 4:
            send_to_char ("Your wish fades unanswered.\n\r", ch);
            break;
        case 5:
            affect_init (&af, AFF_TO_AFFECTS, skill_lookup ("curse"), level,
                         dice (3, 6), APPLY_HITROLL, -4, AFF_CURSE);
            affect_copy_to_char (&af, ch);
            send_to_char ("Your wish backfires - you are cursed!\n\r", ch);
            break;
        case 6:
            ch->hit = UMAX (1, ch->hit - dice (2, 50) - 50);
            send_to_char ("Your wish backfires - misfortune strikes you!\n\r", ch);
            break;
        default: /* 7 */
            stat = number_range (APPLY_STR, APPLY_CON);
            affect_init (&af, AFF_TO_AFFECTS, sn, level, 50, stat, -2, 0);
            affect_copy_to_char (&af, ch);
            send_to_char ("Your wish backfires - you are weakened!\n\r", ch);
            break;
    }
}
