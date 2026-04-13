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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_info.c,v 1.3 2000/12/01 10:48:33 ring0 Exp $ */

#include "act_info.h"

#include "act_comm.h"
#include "chars.h"
#include "comm.h"
#include "extra_descrs.h"
#include "find.h"
#include "globals.h"
#include "interp.h"
#include "items.h"
#include "lookup.h"
#include "memory.h"
#include "objs.h"
#include "players.h"
#include "recycle.h"
#include "rooms.h"
#include "spell_info.h"
#include "tables.h"
#include "update.h"
#include "utils.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define SCAN_ALL_DIRS -2

bool do_filter_blind(CHAR_T *ch)
{
    if (!IS_NPC(ch) && EXT_IS_SET(ch->ext_plr, PLR_HOLYLIGHT))
        return FALSE;
    FILTER(IS_AFFECTED(ch, AFF_BLIND),
           "You can't see a thing!\n\r", ch);
    return FALSE;
}

void do_scan_list(ROOM_INDEX_T *scan_room, CHAR_T *ch,
                  sh_int depth, sh_int door)
{
    CHAR_T *rch;

    if (scan_room == NULL)
        return;
    for (rch = scan_room->people_first; rch != NULL; rch = rch->room_next)
    {
        if (rch == ch)
            continue;
        if (!IS_NPC(rch) && rch->invis_level > char_get_trust(ch))
            continue;
        if (!char_can_see_anywhere(ch, rch))
            continue;
        do_scan_char(rch, ch, depth, door);
    }
}

static char *const scan_distance[8] = {
    "right here.",
    "nearby %s.",
    "not far %s.",
    "a bit far %s.",
    "far %s.",
    "very far %s.",
    "very, very far %s.",
    "extremely far %s.",
};

void do_scan_char(CHAR_T *victim, CHAR_T *ch, sh_int depth,
                  sh_int door)
{
    const DOOR_T *door_obj;
    char buf[MAX_INPUT_LENGTH];

    door_obj = door_get(door);
    sprintf(buf, scan_distance[depth], door_obj->to_phrase);

    printf_to_char(ch, "%s, %s\n\r", PERS_AW(victim, ch), buf);
}

void do_scan_real(CHAR_T *ch, char *argument, int max_depth)
{
    char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    int min_depth, i;
    ROOM_INDEX_T *scan_room;
    EXIT_T *ex;
    sh_int door, depth;

    argument = one_argument(argument, arg1);
    if (arg1[0] == '\0')
    {
        min_depth = 0;
        door = SCAN_ALL_DIRS;
        send_to_char("Looking around you see:\n\r", ch);
        act("$n looks all around.", ch, NULL, NULL, TO_NOTCHAR);
    }
    else if ((door = door_lookup(arg1)) >= 0)
    {
        min_depth = 1;
        act2("You peer intently $T.", "$n peers intently $T.",
             ch, NULL, door_table[door].name, 0, POS_RESTING);
        sprintf(buf, "Looking %s you see:\n\r", door_table[door].name);
        max_depth *= 2;
    }
    else
    {
        send_to_char("That's not a valid direction.\n\r", ch);
        return;
    }

    if (min_depth <= 0)
    {
        do_scan_list(ch->in_room, ch, 0, 0);
        min_depth = 1;
    }
    for (i = 0; i < DIR_MAX; i++)
    {
        if (door != SCAN_ALL_DIRS && door != i)
            continue;
        scan_room = ch->in_room;
        for (depth = 1; depth <= max_depth; depth++)
        {
            if (depth < min_depth)
                continue;
            if ((ex = scan_room->exit[i]) == NULL)
                break;
            if (IS_SET(ex->exit_flags, EX_CLOSED))
                break;
            if ((scan_room = ex->to_room) == NULL)
                break;
            if (!char_can_see_room(ch, scan_room))
                break;
            do_scan_list(scan_room, ch, depth, i);
        }
    }
}

void do_look_room(CHAR_T *ch, int is_auto)
{
    char sect_char = room_colour_char(ch->in_room);
    printf_to_char(ch, "{%c%s{x", sect_char, ch->in_room->name);

    if (pd_config.show_room_vnums &&
        ((IS_IMMORTAL(ch) && (IS_NPC(ch) || EXT_IS_SET(ch->ext_plr, PLR_HOLYLIGHT))) ||
         IS_BUILDER(ch, ch->in_room->area)))
        printf_to_char(ch, "{r [{RRoom %d{r]{x", ch->in_room->vnum);
    send_to_char("\n\r", ch);

    if (!is_auto || (!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF)))
        printf_to_char(ch, "  {S%s{x", ch->in_room->description);

    if (!IS_NPC(ch) && EXT_IS_SET(ch->ext_plr, PLR_AUTOEXIT))
        do_function(ch, &do_exits, "auto");

    obj_list_show_to_char(ch->in_room->content_first, ch, FALSE, FALSE);
    char_list_show_to_char(ch->in_room->people_first, ch);
}

void do_look_in(CHAR_T *ch, char *argument)
{
    OBJ_T *obj;

    BAIL_IF(argument[0] == '\0',
            "Look in what?\n\r", ch);
    BAIL_IF((obj = find_obj_here(ch, argument)) == NULL,
            "You do not see that here.\n\r", ch);
    BAIL_IF(!item_look_in(obj, ch),
            "That is not a container.\n\r", ch);
}

void do_look_direction(CHAR_T *ch, int door)
{
    EXIT_T *pexit;

    BAIL_IF((pexit = ch->in_room->exit[door]) == NULL,
            "Nothing special there.\n\r", ch);

    if (pexit->description != NULL && pexit->description[0] != '\0')
        send_to_char(pexit->description, ch);
    else
        send_to_char("Nothing special there.\n\r", ch);

    if (pexit->keyword != NULL && pexit->keyword[0] != '\0' &&
        pexit->keyword[0] != ' ')
    {
        if (IS_SET(pexit->exit_flags, EX_CLOSED))
            act("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
        else if (IS_SET(pexit->exit_flags, EX_ISDOOR))
            act("The $d is open.", ch, NULL, pexit->keyword, TO_CHAR);
    }
}

bool do_filter_description_remove_line(CHAR_T *ch)
{
    char buf[MAX_STRING_LENGTH];
    int len;
    bool found = FALSE;

    FILTER(ch->description == NULL || ch->description[0] == '\0',
           "No lines left to remove.\n\r", ch);

    strcpy(buf, ch->description);
    for (len = strlen(buf); len > 0; len--)
    {
        if (buf[len] == '\r')
        {
            if (!found)
            { /* back it up */
                if (len > 0)
                    len--;
                found = TRUE;
            }
            else
            { /* found the second one */
                buf[len + 1] = '\0';
                str_free(&(ch->description));
                ch->description = str_dup(buf);
                send_to_char("Your description is:\n\r", ch);
                send_to_char(ch->description ? ch->description : "(None).\n\r", ch);
                return FALSE;
            }
        }
    }

    buf[0] = '\0';
    str_free(&(ch->description));
    ch->description = str_dup(buf);
    send_to_char("Description cleared.\n\r", ch);
    return FALSE;
}

bool do_filter_description_append(CHAR_T *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';

    if (argument[0] == '+')
    {
        if (ch->description != NULL)
            strcat(buf, ch->description);
        argument++;
        while (isspace(*argument))
            argument++;
    }
    FILTER(strlen(buf) >= 1024,
           "Description too long.\n\r", ch);

    strcat(buf, argument);
    strcat(buf, "\n\r");

    str_replace_dup(&ch->description, buf);
    return FALSE;
}

bool do_filter_description_alter(CHAR_T *ch, char *argument)
{
    str_smash_tilde(argument);
    if (argument[0] == '-')
        return do_filter_description_remove_line(ch);
    else
        return do_filter_description_append(ch, argument);
}

/* RT Commands to replace news, motd, imotd, etc from ROM */
DEFINE_DO_FUN(do_motd)
{
    do_function(ch, &do_help, "motd");
}
DEFINE_DO_FUN(do_rules)
{
    do_function(ch, &do_help, "rules");
}
DEFINE_DO_FUN(do_story)
{
    do_function(ch, &do_help, "story");
}
/* do_wizlist is defined in wizlist.c */

/* Not-RT(?) commands that are similar */
DEFINE_DO_FUN(do_credits)
{
    do_function(ch, &do_help, "diku");
}

DEFINE_DO_FUN(do_look)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    OBJ_T *obj_list, *obj;
    char *pdesc1, *pdesc2;
    int i, door;
    int number, count;

    if (ch->desc == NULL)
        return;

    BAIL_IF(ch->position < POS_SLEEPING,
            "You can't see anything but stars!\n\r", ch);
    BAIL_IF(ch->position == POS_SLEEPING,
            "You can't see anything, you're sleeping!\n\r", ch);
    if (do_filter_blind(ch))
        return;

    if (!IS_NPC(ch) && !EXT_IS_SET(ch->ext_plr, PLR_HOLYLIGHT) &&
        room_is_dark(ch->in_room))
    {
        send_to_char("{DIt is pitch black ... {x\n\r", ch);
        char_list_show_to_char(ch->in_room->people_first, ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    number = number_argument(arg1, arg3);
    count = 0;

    /* "Auto" look shows the room. */
    if (arg1[0] == '\0' || !str_cmp(arg1, "auto"))
    {
        do_look_room(ch, arg1[0] != '\0');
        return;
    }

    /* Looking in something? */
    if (!str_cmp(arg1, "i") || !str_cmp(arg1, "in") || !str_cmp(arg1, "on"))
    {
        do_look_in(ch, arg2);
        return;
    }

    /* Looking at someone? */
    if ((victim = find_char_same_room(ch, arg1)) != NULL)
    {
        char_look_at_char(victim, ch);
        return;
    }

#define CHECK_LOOK(cond, str, with_crlf)  \
    if ((cond))                           \
    {                                     \
        if (++count == number)            \
        {                                 \
            send_to_char((str), ch);      \
            if ((with_crlf))              \
                send_to_char("\n\r", ch); \
            return;                       \
        }                                 \
        continue;                         \
    }

    /* Looking at any obj extra descriptions? */
    for (i = 0; i < 2; i++)
    {
        switch (i)
        {
        case 0:
            obj_list = ch->content_first;
            break;
        case 1:
            obj_list = ch->in_room->content_first;
            break;
        default:
            obj_list = NULL;
        }
        for (obj = obj_list; obj != NULL; obj = obj->content_next)
        {
            if (!char_can_see_obj(ch, obj))
                continue;
            pdesc1 = extra_descr_get_description(
                obj->extra_descr_first, arg3);
            pdesc2 = extra_descr_get_description(
                obj->obj_index->extra_descr_first, arg3);

            CHECK_LOOK(pdesc1 != NULL, pdesc1, FALSE);
            CHECK_LOOK(pdesc2 != NULL, pdesc2, FALSE);
            /* TODO[passive-lore-skill]: item_look_at currently shows basic
             * type info to all players. Gate behind a passive skill in the
             * future. Search: rg "TODO\[passive-lore-skill\]" */
            if (str_in_namelist(arg3, obj->name)) {
                if (++count == number) {
                    /* G3: unidentified consumables show a generic label. */
                    if (IS_SET (obj->extra_flags, ITEM_UNIDENTIFIED)) {
                        switch (obj->item_type) {
                            case ITEM_POTION:
                                send_to_char ("A murky liquid sloshes around inside.\n\r", ch);
                                break;
                            case ITEM_SCROLL:
                                send_to_char ("The writing on this scroll is difficult to make out.\n\r", ch);
                                break;
                            case ITEM_PILL:
                                send_to_char ("A small pill of indeterminate composition.\n\r", ch);
                                break;
                            case ITEM_WAND:
                            case ITEM_STAFF:
                                send_to_char ("You can't quite determine the purpose of this wand.\n\r", ch);
                                break;
                            default:
                                send_to_char (obj->description, ch);
                                send_to_char ("\n\r", ch);
                                break;
                        }
                    } else {
                        send_to_char(obj->description, ch);
                        send_to_char("\n\r", ch);
                    }
                    item_look_at(obj, ch);
                    return;
                }
                continue;
            }
        }
    }

    do
    {
        pdesc1 = extra_descr_get_description(
            ch->in_room->extra_descr_first, arg3);
        CHECK_LOOK(pdesc1 != NULL, pdesc1, FALSE);
    } while (0);

    /* Did we exceed the count? */
    if (count > 0 && count != number)
    {
        if (count == 1)
            printf_to_char(ch, "You only see one %s here.\n\r", arg3);
        else
            printf_to_char(ch, "You only see %d of those here.\n\r", count);
        return;
    }

    /* Check for a specific direction. */
    if ((door = door_lookup(arg1)) >= 0)
    {
        do_look_direction(ch, door);
        return;
    }

    /* All options failed. */
    send_to_char("You do not see that here.\n\r", ch);
}

/* RT added back for the hell of it */
DEFINE_DO_FUN(do_read)
{
    do_function(ch, &do_look, argument);
}

DEFINE_DO_FUN(do_examine)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;

    DO_REQUIRE_ARG(arg, "Examine what?\n\r");

    do_function(ch, &do_look, arg);
    if ((obj = find_obj_here(ch, arg)) == NULL)
        return;

    item_examine(obj, ch);
}

DEFINE_DO_FUN(do_lore)
{
    OBJ_T *obj;
    int skill = char_get_skill(ch, SN(LORE));

    BAIL_IF(skill == 0,
            "You haven't studied any lore.\n\r", ch);
    BAIL_IF(argument[0] == '\0',
            "Check the lore on what?\n\r", ch);
    BAIL_IF((obj = find_obj_here(ch, argument)) == NULL,
            "You can't find that here.\n\r", ch);
    spell_identify_perform(ch, obj, skill);
}

/* Thanks to Zrin for auto-exit part. */
DEFINE_DO_FUN(do_exits)
{
    char buf[MAX_STRING_LENGTH];
    bool auto_exits;
    int mode;

    auto_exits = !str_cmp(argument, "auto");
    if (do_filter_blind(ch))
        return;

    if (auto_exits)
        sprintf(buf, "{o[Exits: ");
    else if (IS_IMMORTAL(ch))
        sprintf(buf, "Obvious exits from room %d:\n\r", ch->in_room->vnum);
    else
        sprintf(buf, "Obvious exits:\n\r");
    send_to_char(buf, ch);

    mode = auto_exits ? EXITS_AUTO : EXITS_LONG;
    char_format_exit_string(ch, ch->in_room, mode, buf, sizeof(buf));
    if (auto_exits)
        printf_to_char(ch, "%s]{x\n\r", buf);
    else
        printf_to_char(ch, "%s", buf);
}

DEFINE_DO_FUN(do_worth)
{
    if (IS_NPC(ch))
    {
        printf_to_char(ch, "You have {Y%ld{x gold and {W%ld{x silver.\n\r",
                       ch->gold, ch->silver);
        return;
    }
    printf_to_char(ch,
                   "You have {Y%ld{x gold, {W%ld{x silver, "
                   "and {G%d{x experience ({R%d{x exp to level).\n\r",
                   ch->gold, ch->silver, ch->exp, player_get_exp_to_next_level(ch));
}

DEFINE_DO_FUN(do_score)
{
    char buf[MAX_STRING_LENGTH];
    int i;

    sprintf(buf, "{B================================================================={x\n\r");
    send_to_char(buf, ch);

    sprintf(buf, "{G%s%s.{x\n\r", ch->name, IS_NPC(ch) ? "" : ch->pcdata->title);
    send_to_char(buf, ch);

    sprintf(buf, "{B================================================================={x\n\r");
    send_to_char(buf, ch);

    sprintf(buf, "Level: {Y%d{x   Age: {Y%d{x   Played: {Y%d{x hours.\n\r", ch->level, char_get_age(ch), (ch->played + (int)(current_time - ch->logon)) / 3600);
    send_to_char(buf, ch);

    if (char_get_trust(ch) != ch->level)
    {
        sprintf(buf, "You are trusted at level %d.\n\r", char_get_trust(ch));
        send_to_char(buf, ch);
    }

    sprintf(buf, "Title: {Y%s{x\n\r", IS_NPC(ch) ? "" : ch->pcdata->title);
    send_to_char(buf, ch);

    sprintf(buf, "{B================================================================={x\n\r");
    send_to_char(buf, ch);

    sprintf(buf, "Race: {Y%s{x  Sex: {Y%s{x  Class: {Y%s{x\n\r", race_get_name(ch->race), sex_name(ch->sex), char_get_class_name(ch));
    send_to_char(buf, ch);

    sprintf(buf, "Str: {Y%d{x({R%d{x)  Int: {Y%d{x({R%d{x)  Wis: {Y%d{x({R%d{x)  Dex: {Y%d{x({R%d{x)  Con: {Y%d{x({R%d{x)\n\r",
            ch->perm_stat[STAT_STR], char_get_curr_stat(ch, STAT_STR),
            ch->perm_stat[STAT_INT], char_get_curr_stat(ch, STAT_INT),
            ch->perm_stat[STAT_WIS], char_get_curr_stat(ch, STAT_WIS),
            ch->perm_stat[STAT_DEX], char_get_curr_stat(ch, STAT_DEX),
            ch->perm_stat[STAT_CON], char_get_curr_stat(ch, STAT_CON));
    send_to_char(buf, ch);

    sprintf(buf, "{B================================================================={x\n\r");
    send_to_char(buf, ch);

    sprintf(buf, "HP: {Y%d{x of {R%d{x   MANA: {Y%d{x of {R%d{x   MOVE: {Y%d{x of {R%d{x\n\r", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
    send_to_char(buf, ch);

#ifdef BASEMUD_SHOW_RECOVERY_RATE
    sprintf(buf, "Your recovery rates are %+d hit, %+d mana, %+d movement.\n\r", hit_gain(ch, FALSE), mana_gain(ch, FALSE), move_gain(ch, FALSE));
    send_to_char(buf, ch);
#endif

    sprintf(buf, "Practices: {Y%d{x  Trains: {Y%d{x\n\r", ch->practice, ch->train);
    send_to_char(buf, ch);

    sprintf(buf, "{B================================================================={x\n\r");
    send_to_char(buf, ch);

    sprintf(buf, "Items: {Y%d{x of {R%d{x  Weight: {Y%ld{x of {Y%ld{x pounds.\n\r", ch->carry_number, char_get_max_carry_count(ch), char_get_carry_weight(ch) / 10, char_get_max_carry_weight(ch) / 10);
    send_to_char(buf, ch);

    sprintf(buf, "Gold: {Y%ld{x  Silver: {W%ld{x\n\r", ch->gold, ch->silver);
    send_to_char(buf, ch);

    sprintf(buf, "{B================================================================={x\n\r");
    send_to_char(buf, ch);

    // Experience to next level
    if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
    {
        sprintf(buf, "Experience: {Y%d{x   Next level: {Y%d{x\n\r", ch->exp, player_get_exp_to_next_level(ch));
        send_to_char(buf, ch);
    }

    if (ch->level >= 10)
    {
        sprintf(buf, "Alignment: {Y%d{x  ", ch->alignment);
        send_to_char(buf, ch);
    }

    send_to_char("You are ", ch);
    if (ch->alignment > 900)
        send_to_char("{Yangelic.{x\n\r", ch);
    else if (ch->alignment > 700)
        send_to_char("{Ysaintly.{x\n\r", ch);
    else if (ch->alignment > 350)
        send_to_char("{Ygood.{x\n\r", ch);
    else if (ch->alignment > 100)
        send_to_char("{Wkind.{x\n\r", ch);
    else if (ch->alignment > -100)
        send_to_char("{Wneutral.{x\n\r", ch);
    else if (ch->alignment > -350)
        send_to_char("{Wmean.{x\n\r", ch);
    else if (ch->alignment > -700)
        send_to_char("{Revil.{x\n\r", ch);
    else if (ch->alignment > -900)
        send_to_char("{Rdemonic.{x\n\r", ch);
    else
        send_to_char("{Rsatanic.{x\n\r", ch);

    sprintf(buf, "{B================================================================={x\n\r");
    send_to_char(buf, ch);

    sprintf(buf, "Wimpy: {Y%d{x hit points.\n\r", ch->wimpy);
    send_to_char(buf, ch);

    if (IS_DRUNK(ch))
        send_to_char("You are drunk.\n\r", ch);
    if (IS_THIRSTY(ch))
        send_to_char("You are thirsty.\n\r", ch);
    if (IS_HUNGRY(ch))
        send_to_char("You are hungry.\n\r", ch);

    send_to_char(char_get_position_str(ch, ch->position, ch->on, TRUE), ch);
    send_to_char("\n\r", ch);

    sprintf(buf, "{B================================================================={x\n\r");
    send_to_char(buf, ch);

    // Armor class
    if (ch->level >= 25)
    {
        sprintf(buf, "Armor: pierce: {Y%d{x  bash: {Y%d{x  slash: {Y%d{x  magic: {Y%d{x\n\r", GET_AC(ch, AC_PIERCE), GET_AC(ch, AC_BASH), GET_AC(ch, AC_SLASH), GET_AC(ch, AC_EXOTIC));
        send_to_char(buf, ch);
    }

    for (i = 0; i < 4; i++)
    {
        char *temp;
        switch (i)
        {
        case AC_PIERCE:
            temp = "piercing";
            break;
        case AC_BASH:
            temp = "bashing";
            break;
        case AC_SLASH:
            temp = "slashing";
            break;
        case AC_EXOTIC:
            temp = "magic";
            break;
        default:
            temp = "error";
            break;
        }
        send_to_char("You are ", ch);
        if (GET_AC(ch, i) >= 200)
            sprintf(buf, "hopelessly vulnerable to %s.\n\r", temp);
        else if (GET_AC(ch, i) >= 175)
            sprintf(buf, "defenseless against %s.\n\r", temp);
        else if (GET_AC(ch, i) >= 150)
            sprintf(buf, "barely protected from %s.\n\r", temp);
        else if (GET_AC(ch, i) >= 125)
            sprintf(buf, "slightly armored against %s.\n\r", temp);
        else if (GET_AC(ch, i) >= 75)
            sprintf(buf, "somewhat armored against %s.\n\r", temp);
        else if (GET_AC(ch, i) >= 0)
            sprintf(buf, "armored against %s.\n\r", temp);
        else if (GET_AC(ch, i) >= -50)
            sprintf(buf, "well-armored against %s.\n\r", temp);
        else if (GET_AC(ch, i) >= -100)
            sprintf(buf, "very well-armored against %s.\n\r", temp);
        else if (GET_AC(ch, i) >= -150)
            sprintf(buf, "heavily armored against %s.\n\r", temp);
        else if (GET_AC(ch, i) >= -200)
            sprintf(buf, "superbly armored against %s.\n\r", temp);
        else if (GET_AC(ch, i) >= -250)
            sprintf(buf, "almost invulnerable to %s.\n\r", temp);
        else
            sprintf(buf, "divinely armored against %s.\n\r", temp);
        send_to_char(buf, ch);
    }

    sprintf(buf, "{B================================================================={x\n\r");
    send_to_char(buf, ch);

    // Holy light and invis/immortal info
    if (IS_IMMORTAL(ch))
    {
        send_to_char("Holy Light: ", ch);
        if (EXT_IS_SET(ch->ext_plr, PLR_HOLYLIGHT))
            send_to_char("{Yon{x", ch);
        else
            send_to_char("{Roff{x", ch);
        if (ch->invis_level)
        {
            sprintf(buf, "  Invisible: level {Y%d{x", ch->invis_level);
            send_to_char(buf, ch);
        }
        if (ch->incog_level)
        {
            sprintf(buf, "  Incognito: level {Y%d{x", ch->incog_level);
            send_to_char(buf, ch);
        }
        send_to_char("\n\r", ch);
    }

    if (ch->level >= 15)
    {
        sprintf(buf, "Hitroll: {Y%d{x  Damroll: {Y%d{x\n\r", GET_HITROLL(ch), GET_DAMROLL(ch));
        send_to_char(buf, ch);
    }

    if (!IS_NPC(ch))
    {
        int threshold = UMAX(1, player_get_exp_per_level(ch) / 20);
        send_to_char("{B================================================================={x\n\r", ch);
        sprintf(buf, "[{1Quest{x]  Chances: {C%d{x   Points: {Y%d{x\n\r"
                     "          Progress: {Y%d{x / {Y%d{x xp to next chance\n\r"
                     "          Cooldown: ",
            ch->quest_chances, ch->questpoints,
            ch->quest_xp_prog, threshold);
        send_to_char(buf, ch);
        if (ch->nextquest > 1)
        {
            sprintf(buf, "{R%d min{x\n\r", ch->nextquest);
            send_to_char(buf, ch);
        }
        else if (ch->nextquest == 1)
            send_to_char("{Rless than 1 min{x\n\r", ch);
        else
            send_to_char("{GReady{x\n\r", ch);
    }

    if (!IS_NPC(ch))
    {
        sprintf(buf, "[{1PK Stats{x] Kills: {Y%d{x  Deaths: {Y%d{x\n\r",
            ch->pcdata->pkkills, ch->pcdata->pkdeaths);
        send_to_char(buf, ch);
        sprintf(buf, "{B================================================================={x\n\r");
        send_to_char(buf, ch);
    }

    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
        do_function(ch, &do_affects, "");
}

DEFINE_DO_FUN(do_affects)
{
    AFFECT_T *paf, *paf_last = NULL;

    BAIL_IF(ch->affect_first == NULL,
            "You are not affected by any spells.\n\r", ch);

    send_to_char("{YYou are affected by the following spells:{x\n\r", ch);
    for (paf = ch->affect_first; paf != NULL; paf = paf->on_next)
    {
        if (paf_last == NULL || paf->type != paf_last->type)
            printf_to_char(ch, "{YSpell: {M%-15s{x\n\r", skill_table[paf->type].name);
        else if (ch->level < 20)
            continue;

        if (ch->level >= 20)
        {
            if (paf->apply == APPLY_NONE)
                printf_to_char(ch, "   {Clasts {x");
            else
                printf_to_char(ch, "   {Cmodifies {W%s{x by {G%d{x ",
                               affect_apply_name(paf->apply), paf->modifier);
            if (paf->duration == -1)
                send_to_char("{Gpermanently{x\n\r", ch);
            else
                printf_to_char(ch, "{Bfor {W%d{x hours\n\r", paf->duration);
        }
        paf_last = paf;
    }
}

DEFINE_DO_FUN(do_time)
{
    const DAY_T *day_obj;
    const MONTH_T *month_obj;
    char *suf;
    int day = time_info.day + 1;

    /* Determine suffix */
    if (day > 4 && day < 20)
        suf = "th";
    else if (day % 10 == 1)
        suf = "st";
    else if (day % 10 == 2)
        suf = "nd";
    else if (day % 10 == 3)
        suf = "rd";
    else
        suf = "th";

    day_obj = day_get_current();
    month_obj = month_get_current();

    printf_to_char(ch,
                   "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r",
                   (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
                   time_info.hour >= 12 ? "pm" : "am",
                   day_obj->name, day, suf, month_obj->name);

    printf_to_char(ch, "ROM started up at %s.\n\rThe system time is %s.\n\r",
                   str_boot_time, (char *)ctime_fixed(&current_time));
}

DEFINE_DO_FUN(do_weather)
{
    const SKY_T *sky;
    char *change;

    BAIL_IF(!IS_OUTSIDE(ch),
            "You can't see the weather indoors.\n\r", ch);

    sky = sky_get_current();
    change = weather_info.change >= 0
                 ? "a warm southerly breeze blows"
                 : "a cold northern gust blows";

    printf_to_char(ch, "The sky is %s and %s.\n\r", sky->description, change);
}

DEFINE_DO_FUN(do_help)
{
    HELP_T *help;
    BUFFER_T *output;
    bool found = FALSE;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];
    int level, trust;

    output = buf_new();
    if (argument[0] == '\0')
        argument = "summary";

    /* This parts handles "help a b" so that it returns "help 'a b'" */
    argall[0] = '\0';
    while (argument[0] != '\0')
    {
        argument = one_argument(argument, argone);
        if (argall[0] != '\0')
            strcat(argall, " ");
        strcat(argall, argone);
    }

    trust = char_get_trust(ch);
    for (help = help_first; help != NULL; help = help->global_next)
    {
        level = (help->level < 0) ? -1 * help->level - 1 : help->level;
        if (level > trust)
            continue;

        if (str_in_namelist(argall, help->keyword))
        {
            /* add seperator if found */
            if (found)
                buf_cat(output,
                        "\n\r{g============================================================{x\n\r\n\r");
            if (help->level >= 0 && str_cmp(argall, "imotd"))
            {
                buf_cat(output, "{T");
                buf_cat(output, help->keyword);
                buf_cat(output, "{x\n\r");
            }

            /* Strip leading '.' to allow initial blanks. */
            if (help->text[0] == '.')
                buf_cat(output, help->text + 1);
            else
                buf_cat(output, help->text);
            found = TRUE;

            /* small hack :) */
            if (ch->desc != NULL && ch->desc->connected != CON_PLAYING && ch->desc->connected != CON_GEN_GROUPS)
                break;
        }
    }

    if (!found)
    {
        send_to_char("{YNo help on that word.{x\n\r", ch);
        /* Let's log unmet help requests so studious IMP's can improve their help files ;-)
         * But to avoid idiots, we will check the length of the help request, and trim to
         * a reasonable length (set it by redefining MAX_CMD_LEN in merc.h).  -- JR */
        if (strlen(argall) > MAX_CMD_LEN)
        {
            argall[MAX_CMD_LEN - 1] = '\0';
            log_f("Excessive command length: %s requested %s.", ch->name, argall);
            send_to_char("That was rude!\n\r", ch);
        }
        /* OHELPS_FILE is the "orphaned helps" files. Defined in merc.h -- JR */
        else
            append_file(ch, OHELPS_FILE, argall);
    }
    else
        page_to_char(buf_string(output), ch);

    buf_free(output);
}

/* whois command */
DEFINE_DO_FUN(do_whois)
{
    char arg[MAX_INPUT_LENGTH];
    BUFFER_T *output;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_T *d;
    bool found = FALSE;

    DO_REQUIRE_ARG(arg, "You must provide a name.\n\r");

    output = buf_new();
    for (d = descriptor_first; d != NULL; d = d->global_next)
    {
        CHAR_T *wch = CH(d);
        if (d->connected != CON_PLAYING)
            continue;
        if (!char_can_see_anywhere(ch, d->character))
            continue;
        if (!char_can_see_anywhere(ch, wch))
            continue;
        if (str_prefix(arg, wch->name))
            continue;

        found = TRUE;
        char_get_who_string(ch, wch, buf, sizeof(buf));
        buf_cat(output, buf);
        if (!IS_NPC(wch))
        {
            snprintf(buf, sizeof(buf), "[{1PK Stats{x] Kills: {Y%d{x  Deaths: {Y%d{x\n\r",
                wch->pcdata->pkkills, wch->pcdata->pkdeaths);
            buf_cat(output, buf);
        }
    }

    BAIL_IF(!found,
            "No one of that name is playing.\n\r", ch);

    page_to_char(buf_string(output), ch);
    buf_free(output);
}

/* Comparator for do_who: sort by level descending, then name ascending. */
static int who_entry_cmp (const void *a, const void *b)
{
    const CHAR_T *wa = *(const CHAR_T * const *) a;
    const CHAR_T *wb = *(const CHAR_T * const *) b;
    if (wb->level != wa->level)
        return wb->level - wa->level;
    return strcmp (wa->name, wb->name);
}

/* New 'who' command originally by Alander of Rivers of Mud. */
DEFINE_DO_FUN(do_who)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    BUFFER_T *output;
    DESCRIPTOR_T *d;
    CHAR_T *who_list[300];
    int i, level_lower, level_upper;
    int current_number, count_imm, count_mort, matches, who_count;
    bool restrict_class = FALSE;
    bool restrict_clan = FALSE;
    bool only_clan = FALSE;
    bool restrict_race = FALSE;
    bool only_immortal = FALSE;
    bool *show_class;
    bool *show_race;
    bool show_clan[CLAN_MAX];

    /* Set default arguments. */
    level_lower = 0;
    level_upper = MAX_LEVEL;
    show_class = calloc(class_count, sizeof(bool));
    show_race = calloc(race_count, sizeof(bool));
    for (i = 0; i < class_count; i++)
        show_class[i] = FALSE;
    for (i = 0; i < race_count; i++)
        show_race[i] = FALSE;
    for (i = 0; i < CLAN_MAX; i++)
        show_clan[i] = FALSE;

    /* Parse arguments. */
    current_number = 0;
    while (1)
    {
        char arg[MAX_STRING_LENGTH];
        argument = one_argument(argument, arg);
        if (arg[0] == '\0')
            break;

        /* Check for level arguments. */
        if (is_number(arg))
        {
            switch (++current_number)
            {
            case 1:
                level_lower = atoi(arg);
                break;
            case 2:
                level_upper = atoi(arg);
                break;
            default:
                send_to_char("Only two level numbers allowed.\n\r", ch);
                free(show_class);
                free(show_race);
                return;
            }
            continue;
        }

        /* Look for classes to turn on. */
        if (!str_prefix(arg, "immortals"))
        {
            only_immortal = TRUE;
            continue;
        }

        /* Check for explicit classes. */
        if ((i = class_lookup(arg)) >= 0)
        {
            restrict_class = TRUE;
            show_class[i] = TRUE;
            continue;
        }

        /* Check for explicit races. */
        i = pc_race_lookup(arg);
        if (i >= 1)
        {
            const PC_RACE_T *pc_race = pc_race_get(i);
            if ((i = race_lookup_exact(pc_race->name)) >= 1)
            {
                restrict_race = TRUE;
                show_race[i] = TRUE;
                continue;
            }
        }

        /* Check for anyone with a clan. */
        if (!str_prefix(arg, "clan"))
        {
            only_clan = TRUE;
            continue;
        }

        /* Check for specific clans. */
        if ((i = clan_lookup(arg)) >= 0)
        {
            restrict_clan = TRUE;
            show_clan[i] = TRUE;
            continue;
        }

        /* Unknown argument. */
        send_to_char("That's not a valid race, class, or clan.\n\r", ch);
        free(show_class);
        free(show_race);
        return;
    }

    /* Collect all matching chars into a list. */
    matches = 0;
    count_imm = 0;
    count_mort = 0;
    who_count = 0;
    output = buf_new();

    for (d = descriptor_first; d != NULL; d = d->global_next)
    {
        CHAR_T *wch = CH(d);

        if (d->connected != CON_PLAYING)
            continue;
        if (!char_can_see_anywhere(ch, d->character))
            continue;
        if (!char_can_see_anywhere(ch, wch))
            continue;
        if (wch->level < level_lower || wch->level > level_upper)
            continue;
        if (only_immortal && wch->level < LEVEL_IMMORTAL)
            continue;
        if (restrict_class && !show_class[wch->class])
            continue;
        if (restrict_race && !show_race[wch->race])
            continue;
        if (only_clan && !player_has_clan(wch))
            continue;
        if (restrict_clan && !show_clan[wch->clan])
            continue;

        if (who_count < 300)
            who_list[who_count++] = wch;
    }

    /* Sort: level descending, then name ascending. */
    qsort(who_list, who_count, sizeof(CHAR_T *), who_entry_cmp);

    /* Immortal section. */
    buf_cat(output, "\n\r{W[ Immortals ]{x\n\r");
    for (i = 0; i < who_count; i++)
    {
        CHAR_T *wch = who_list[i];
        if (wch->level < LEVEL_IMMORTAL)
            continue;
        count_imm++;
        matches++;
        char_get_who_string(ch, wch, buf, sizeof(buf));
        buf_cat(output, buf);
    }

    /* Mortal section (skip when only showing immortals). */
    if (!only_immortal)
    {
        buf_cat(output, "\n\r{W[ Mortals ]{x\n\r");
        for (i = 0; i < who_count; i++)
        {
            CHAR_T *wch = who_list[i];
            if (wch->level >= LEVEL_IMMORTAL)
                continue;
            count_mort++;
            matches++;
            char_get_who_string(ch, wch, buf, sizeof(buf));
            buf_cat(output, buf);
        }
    }

    sprintf(buf2,
            "\n\r{WPlayers found: {G%d {W({YImm: {G%d {W| {YMort: {G%d{W){x\n\r",
            matches, count_imm, count_mort);
    buf_cat(output, buf2);
    page_to_char(buf_string(output), ch);
    buf_free(output);
    free(show_class);
    free(show_race);
}

/* for keeping track of the player count */
static int max_on = 0;
DEFINE_DO_FUN(do_count)
{
    int count;
    DESCRIPTOR_T *d;

    count = 0;
    for (d = descriptor_first; d != NULL; d = d->global_next)
        if (d->connected == CON_PLAYING && char_can_see_anywhere(ch, d->character))
            count++;
    max_on = UMAX(count, max_on);

    if (max_on == count)
        printf_to_char(ch,
            "{GThere are {W%d{G characters on, the most so far today.{x\n\r",
            count);
    else
        printf_to_char(ch,
            "{GThere are {W%d{G characters on, the most on today was {W%d{G.{x\n\r",
            count, max_on);
}

DEFINE_DO_FUN(do_inventory)
{
    send_to_char("{CYou are carrying:{x\n\r", ch);
    obj_list_show_to_char(ch->content_first, ch, TRUE, TRUE);
}

static const char *eq_worn_str (CHAR_T *ch, int wear_loc)
{
    OBJ_T *obj = char_get_eq_by_wear_loc(ch, wear_loc);
    if (obj == NULL)
        return "";
    if (char_can_see_obj(ch, obj))
        return obj_format_to_char(obj, ch, TRUE);
    return "something";
}

static void eq_print_slot (CHAR_T *ch, const char *label, int wear_loc)
{
    const char *item = eq_worn_str(ch, wear_loc);
    if (item[0] == '\0')
        return;
    printf_to_char(ch, "%s{g:{x %s\n\r", label, item);
}

DEFINE_DO_FUN(do_equipment)
{
    send_to_char("\n\r{CYou are currently wearing:{x\n\r\n\r", ch);
    /* Head area */
    eq_print_slot(ch, "{CL{Wigh{Ct     ", WEAR_LOC_LIGHT);
    eq_print_slot(ch, "{CH{Wea{Cd      ", WEAR_LOC_HEAD);
    eq_print_slot(ch, "{CE{Wye{Cs      ", WEAR_LOC_EYES);
    eq_print_slot(ch, "{CE{Wae{Cr s    ", WEAR_LOC_EARS);
    eq_print_slot(ch, "{CN{Wec{Ck      ", WEAR_LOC_NECK_1);
    eq_print_slot(ch, "{CN{Wec{Ck      ", WEAR_LOC_NECK_2);
    /* Body area */
    eq_print_slot(ch, "{CT{Watto{Co    ", WEAR_LOC_TATTOO);
    eq_print_slot(ch, "{CB{Wod{Cy      ", WEAR_LOC_BODY);
    eq_print_slot(ch, "{CT{Wors{Co     ", WEAR_LOC_ABOUT);
    eq_print_slot(ch, "{CC{Wloa{Ck     ", WEAR_LOC_CLOAK);
    eq_print_slot(ch, "{CB{Wac{Ck      ", WEAR_LOC_BACK);
    /* Arms and hands */
    eq_print_slot(ch, "{CA{Wrm{Cs      ", WEAR_LOC_ARMS);
    eq_print_slot(ch, "{CW{Wris{Ct     ", WEAR_LOC_WRIST_L);
    eq_print_slot(ch, "{CW{Wris{Ct     ", WEAR_LOC_WRIST_R);
    eq_print_slot(ch, "{CH{Wand{Cs     ", WEAR_LOC_HANDS);
    eq_print_slot(ch, "{CF{Winge{Cr    ", WEAR_LOC_FINGER_L);
    eq_print_slot(ch, "{CF{Winge{Cr    ", WEAR_LOC_FINGER_R);
    /* Mid-body */
    eq_print_slot(ch, "{CW{Wais{Ct     ", WEAR_LOC_WAIST);
    eq_print_slot(ch, "{CT{Wai{Cl      ", WEAR_LOC_TAIL);
    /* Legs and feet */
    eq_print_slot(ch, "{CL{Weg{Cs      ", WEAR_LOC_LEGS);
    eq_print_slot(ch, "{CF{Wee{Ct      ", WEAR_LOC_FEET);
    /* Weapons and appendages */
    eq_print_slot(ch, "{CP{Wrimar{Cy   ", WEAR_LOC_WIELD);
    eq_print_slot(ch, "{CS{Whiel{Cd    ", WEAR_LOC_SHIELD);
    eq_print_slot(ch, "{CH{Wel{Cd      ", WEAR_LOC_HOLD);
    /* Special/floating */
    eq_print_slot(ch, "{CF{Wloatin{Cg  ", WEAR_LOC_FLOAT);
    eq_print_slot(ch, "{CF{Wloatin{Cg    ", WEAR_LOC_FLOAT_2);
}

DEFINE_DO_FUN(do_compare)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_T *obj1;
    OBJ_T *obj2;
    bool auto_found;
    const char *msg;
    AFFECT_T *paf;
    int apply1[APPLY_MAX + 1];
    int apply2[APPLY_MAX + 1];
    int i;
    bool any_affects;
    int adiff;
    const char *color;

    argument = one_argument(argument, arg1);
    BAIL_IF(arg1[0] == '\0',
            "Compare what to what?\n\r", ch);
    BAIL_IF((obj1 = find_obj_own_inventory(ch, arg1)) == NULL,
            "You do not have that item.\n\r", ch);

    auto_found = FALSE;
    argument = one_argument(argument, arg2);
    if (arg2[0] == '\0')
    {
        for (obj2 = ch->content_first; obj2 != NULL; obj2 = obj2->content_next)
        {
            if (obj2->wear_loc == WEAR_LOC_NONE)
                continue;
            if (!char_can_see_obj(ch, obj2))
                continue;
            if (obj1->item_type != obj2->item_type)
                continue;
            if ((obj1->wear_flags & (obj2->wear_flags & ~ITEM_TAKE)) == 0)
                continue;
            break;
        }
        BAIL_IF(obj2 == NULL,
                "You aren't wearing anything comparable.\n\r", ch);
        auto_found = TRUE;
    }
    else
    {
        BAIL_IF((obj2 = find_obj_own_inventory(ch, arg2)) == NULL,
                "You do not have that item.\n\r", ch);
    }

    if (obj1 == obj2)
    {
        act("You compare $p to itself.  It looks about the same.",
            ch, obj1, obj2, TO_CHAR);
        return;
    }

    if (obj1->item_type != obj2->item_type || !item_is_comparable(obj1))
    {
        act("You can't compare $p and $P.", ch, obj1, obj2, TO_CHAR);
        return;
    }

    if (auto_found)
        printf_to_char(ch, "Currently equipped: {W%s{x\n\r", obj2->short_descr);

    msg = NULL;
    if (obj1->item_type == ITEM_ARMOR)
    {
        int p1 = obj1->v.armor.vs_pierce, p2 = obj2->v.armor.vs_pierce;
        int b1 = obj1->v.armor.vs_bash,   b2 = obj2->v.armor.vs_bash;
        int s1 = obj1->v.armor.vs_slash,  s2 = obj2->v.armor.vs_slash;
        int m1 = obj1->v.armor.vs_magic,  m2 = obj2->v.armor.vs_magic;
        int dp = p1 - p2, db = b1 - b2, ds = s1 - s2, dm = m1 - m2;
        int dtotal = (p1 + b1 + s1 + m1) - (p2 + b2 + s2 + m2);

        send_to_char("{YArmor comparison:{x\n\r", ch);
        printf_to_char(ch, "  Pierce AC: {W%4d{x vs {W%4d{x  %s(%+d){x\n\r",
            p1, p2, (dp > 0 ? "{G" : dp < 0 ? "{R" : "{W"), dp);
        printf_to_char(ch, "  Bash   AC: {W%4d{x vs {W%4d{x  %s(%+d){x\n\r",
            b1, b2, (db > 0 ? "{G" : db < 0 ? "{R" : "{W"), db);
        printf_to_char(ch, "  Slash  AC: {W%4d{x vs {W%4d{x  %s(%+d){x\n\r",
            s1, s2, (ds > 0 ? "{G" : ds < 0 ? "{R" : "{W"), ds);
        printf_to_char(ch, "  Magic  AC: {W%4d{x vs {W%4d{x  %s(%+d){x\n\r",
            m1, m2, (dm > 0 ? "{G" : dm < 0 ? "{R" : "{W"), dm);

        msg = (dtotal == 0) ? "$p and $P look about the same." :
              (dtotal  > 0) ? "$p looks better than $P."       :
                              "$p looks worse than $P.";
    }
    else if (obj1->item_type == ITEM_WEAPON)
    {
        double avg1, avg2, wdiff;
        const char *wtype1, *wtype2;

        if (obj1->obj_index->new_format)
            avg1 = obj1->v.weapon.dice_num * (obj1->v.weapon.dice_size + 1) / 2.0;
        else
            avg1 = (obj1->v.weapon.dice_num + obj1->v.weapon.dice_size) / 2.0;

        if (obj2->obj_index->new_format)
            avg2 = obj2->v.weapon.dice_num * (obj2->v.weapon.dice_size + 1) / 2.0;
        else
            avg2 = (obj2->v.weapon.dice_num + obj2->v.weapon.dice_size) / 2.0;

        wdiff  = avg1 - avg2;
        wtype1 = str_if_null(weapon_get_name(obj1->v.weapon.weapon_type), "unknown");
        wtype2 = str_if_null(weapon_get_name(obj2->v.weapon.weapon_type), "unknown");

        send_to_char("{YWeapon comparison:{x\n\r", ch);
        printf_to_char(ch, "  Weapon type: {W%-10s{x  {W%s{x\n\r", wtype1, wtype2);
        printf_to_char(ch, "  Average dmg: {W%5.1f{x vs {W%5.1f{x  %s(%+.1f){x\n\r",
            avg1, avg2,
            (wdiff > 0.0 ? "{G" : wdiff < 0.0 ? "{R" : "{W"), wdiff);

        msg = (wdiff == 0.0) ? "$p and $P look about the same." :
              (wdiff  > 0.0) ? "$p looks better than $P."       :
                               "$p looks worse than $P.";
    }

    /* Collect affects from proto (unless enchanted) and instance for each item */
    memset(apply1, 0, sizeof(apply1));
    memset(apply2, 0, sizeof(apply2));

    if (!obj1->enchanted)
        for (paf = obj1->obj_index->affect_first; paf; paf = paf->on_next)
            if (paf->apply > APPLY_NONE && paf->apply <= APPLY_MAX)
                apply1[paf->apply] += paf->modifier;
    for (paf = obj1->affect_first; paf; paf = paf->on_next)
        if (paf->apply > APPLY_NONE && paf->apply <= APPLY_MAX)
            apply1[paf->apply] += paf->modifier;

    if (!obj2->enchanted)
        for (paf = obj2->obj_index->affect_first; paf; paf = paf->on_next)
            if (paf->apply > APPLY_NONE && paf->apply <= APPLY_MAX)
                apply2[paf->apply] += paf->modifier;
    for (paf = obj2->affect_first; paf; paf = paf->on_next)
        if (paf->apply > APPLY_NONE && paf->apply <= APPLY_MAX)
            apply2[paf->apply] += paf->modifier;

    any_affects = FALSE;
    for (i = 1; i <= APPLY_MAX; i++)
    {
        if (apply1[i] == 0 && apply2[i] == 0)
            continue;
        if (!any_affects)
        {
            send_to_char("{YAffects:{x\n\r", ch);
            any_affects = TRUE;
        }
        adiff = apply1[i] - apply2[i];
        color = (adiff > 0) ? "{G" : (adiff < 0) ? "{R" : "{W";
        printf_to_char(ch, "  %-18s {W%4d{x vs {W%4d{x  %s(%+d){x\n\r",
            affect_apply_name(i), apply1[i], apply2[i], color, adiff);
    }

    act(msg, ch, obj1, obj2, TO_CHAR);
}

DEFINE_DO_FUN(do_where)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    DESCRIPTOR_T *d;
    bool found;

    one_argument(argument, arg);

    printf_to_char(ch, "\n{WIn Area: {M%s{x\n\n\r", ch->in_room->area->name);

    if (arg[0] == '\0')
    {
        send_to_char("{GPlayers near you:{x\n\r", ch);
        found = FALSE;
        for (d = descriptor_first; d; d = d->global_next)
        {
            if (d->connected == CON_PLAYING && (victim = d->character) != NULL && !IS_NPC(victim) && victim->in_room != NULL && !IS_SET(victim->in_room->room_flags, ROOM_NOWHERE) && (room_is_owner(victim->in_room, ch) || !room_is_private(victim->in_room)) && victim->in_room->area == ch->in_room->area && char_can_see_anywhere(ch, victim))
            {
                found = TRUE;
                printf_to_char(ch, "{W%-28s {G%s{x\n\r",
                               victim->name, victim->in_room->name);
            }
        }
        if (!found)
            send_to_char("{GNone{x\n\r", ch);
    }
    else
    {
        found = FALSE;
        for (victim = char_first; victim != NULL; victim = victim->global_next)
        {
            if (victim->in_room != NULL && victim->in_room->area == ch->in_room->area && !IS_AFFECTED(victim, AFF_HIDE) && !IS_AFFECTED(victim, AFF_SNEAK) && char_can_see_anywhere(ch, victim) && str_in_namelist(arg, victim->name))
            {
                found = TRUE;
                printf_to_char(ch, "{W%-28s {G%s{x\n\r",
                               PERS_AW(victim, ch), victim->in_room->name);
                break;
            }
        }
        if (!found)
            act("{RYou didn't find any {w$T{R.{x", ch, NULL, arg, TO_CHAR);
    }
}

DEFINE_DO_FUN(do_title)
{
    int i;

    if (IS_NPC(ch))
        return;

    /* Changed this around a bit to do some sanitization first   *
     * before checking length of the title. Need to come up with *
     * a centralized user input sanitization scheme. FIXME!      *
     * JR -- 10/15/00                                            */

    if (strlen(argument) > 45)
        argument[45] = '\0';

    i = strlen(argument);
    if (argument[i - 1] == '{' && argument[i - 2] != '{')
        argument[i - 1] = '\0';

    BAIL_IF(argument[0] == '\0',
            "Change your title to what?\n\r", ch);

    str_smash_tilde(argument);
    player_set_title(ch, argument);
    printf_to_char(ch, "Your title is now:%s.\n\r", ch->pcdata->title);
}

DEFINE_DO_FUN(do_description)
{
    if (argument[0] != '\0')
        if (do_filter_description_alter(ch, argument))
            return;

    send_to_char("Your description is:\n\r", ch);
    send_to_char(ch->description ? ch->description : "(None).\n\r", ch);
}

DEFINE_DO_FUN(do_report)
{
    char buf[MAX_STRING_LENGTH];

    sprintf(buf,
            "{GYou say 'I have {R%d{G/{W%d {Ghp {R%d{G/{W%d {Gmana {R%d{G/{W%d {Gmv {M%d {Gxp.{x'\n\r",
            ch->hit, ch->max_hit,
            ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp);
    send_to_char(buf, ch);

    sprintf(buf, "{W$n {Gsays 'I have {R%d{G/{W%d {Ghp {R%d{G/{W%d {Gmana {R%d{G/{W%d {Gmv {M%d {Gxp.'{x",
            ch->hit, ch->max_hit,
            ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp);
    act(buf, ch, NULL, NULL, TO_NOTCHAR);
}

/* Contributed by Alander. */
DEFINE_DO_FUN(do_commands)
{
    int cmd, col;

    col = 0;
    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    {
        if (cmd_table[cmd].level < LEVEL_HERO &&
            cmd_table[cmd].level <= char_get_trust(ch) && cmd_table[cmd].show)
        {
            printf_to_char(ch, "%-12s", cmd_table[cmd].name);
            if (++col % 6 == 0)
                send_to_char("\n\r", ch);
        }
    }

    if (col % 6 != 0)
        send_to_char("\n\r", ch);
}

/* Comparator for do_areas: sort by low_range ascending, then high_range ascending. */
static int area_level_cmp(const void *a, const void *b)
{
    const AREA_T *a1 = *(const AREA_T **)a;
    const AREA_T *a2 = *(const AREA_T **)b;
    if (a1->low_range != a2->low_range)
        return a1->low_range - a2->low_range;
    return a1->high_range - a2->high_range;
}

/*
 * do_areas: sorted area list with optional level-range filter.
 *   areas           - all areas (mortals capped at LEVEL_IMMORTAL-1)
 *   areas 30        - areas whose range includes level 30
 *   areas 30 45     - areas with low_range >= 30 and high_range <= 45
 * Hidden areas are never shown to mortals.
 * Based on TAKA's Ghost Dancer MUD implementation, adapted for BaseMUD.
 */
DEFINE_DO_FUN(do_areas)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AREA_T *area;
    AREA_T **sorted;
    int count, lo_level, hi_level, col, i;
    bool found;
    bool is_immortal = (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL);

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    /* Determine filter range.
     * One arg: both lo and hi set to arg.
     * Two args: lo from arg1, hi from arg2.
     * No args:  lo=0, hi=MAX_LEVEL for immortals, LEVEL_IMMORTAL-1 for mortals. */
    lo_level = (arg1[0] != '\0' && is_number(arg1))
                   ? URANGE(1, atoi(arg1), MAX_LEVEL)
                   : 0;
    hi_level = (arg2[0] != '\0' && is_number(arg2))
                   ? URANGE(1, atoi(arg2), MAX_LEVEL)
               : (arg1[0] != '\0' && is_number(arg1))
                   ? lo_level
               : is_immortal
                   ? MAX_LEVEL
                   : LEVEL_IMMORTAL - 1;

    /* Collect matching areas (skip hidden unless immortal). */
    count = 0;
    for (area = area_first; area; area = area->global_next)
    {
        if (!is_immortal && IS_SET(area->area_flags, AREA_HIDDEN))
            continue;
        if (area->low_range  > hi_level && area->low_range  != 0)
            continue;
        if (area->high_range < lo_level && area->high_range != 0)
            continue;
        count++;
    }

    if (count == 0)
    {
        send_to_char("{RNo areas meeting those criteria.{x\n\r", ch);
        return;
    }

    sorted = calloc(count, sizeof(AREA_T *));
    i = 0;
    for (area = area_first; area; area = area->global_next)
    {
        if (!is_immortal && IS_SET(area->area_flags, AREA_HIDDEN))
            continue;
        if (area->low_range  > hi_level && area->low_range  != 0)
            continue;
        if (area->high_range < lo_level && area->high_range != 0)
            continue;
        sorted[i++] = area;
    }
    qsort(sorted, count, sizeof(AREA_T *), area_level_cmp);

    col = 0;
    found = FALSE;
    for (i = 0; i < count; i++)
    {
        area = sorted[i];
        found = TRUE;
        sprintf(buf, "{C[{M%3d{C] {C({G%-3d{W-{G%3d{C) {W%-18.18s {x",
                area->vnum,
                area->low_range,
                area->high_range,
                area->name);
        send_to_char(buf, ch);
        if (col)
            send_to_char_bw("\n\r", ch);
        col = !col;
    }
    if (col)
        send_to_char("\n\r", ch);

    free(sorted);

    if (!found)
        send_to_char("{RNo areas meeting those criteria.{x\n\r", ch);
}

DEFINE_DO_FUN(do_scan_short)
{
    do_scan_real(ch, argument, 1);
}
DEFINE_DO_FUN(do_scan_far)
{
    do_scan_real(ch, argument, 3);
}
