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

/*  Info broadcast system, adapted from Voltec's 1998 system.
 *  Allows players to subscribe to different categories of server broadcasts.
 *  Based on wiznet but accessible to all players.
 */

#include "info.h"

#include "chars.h"
#include "comm.h"
#include "descs.h"
#include "globals.h"
#include "lookup.h"
#include "memory.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

/* Info categories table. Similar to wiznet but for player broadcasts. */
const struct info_type info_table[] = {
    {"on", INFO_ON, 0},
    {"levels", INFO_LEVELS, 4},
    {"consent", INFO_CONSENT, 12},
    {"deaths", INFO_DEATHS, 0},
    {"logins", INFO_LOGINS, 0},
    {"quests", INFO_QUESTS, 0},
    {NULL, 0, 0}};

/* Command handler for 'news' command (and aliases 'broadcast', 'system'). */
DEFINE_DO_FUN(do_news)
{
    char buf[MAX_STRING_LENGTH];
    int i;
    flag_t flag;

    /* No argument: toggle main info on/off */
    if (argument[0] == '\0')
    {
        if (IS_SET(ch->info, INFO_ON))
        {
            send_to_char("You will no longer see info messages.\n\r", ch);
            REMOVE_BIT(ch->info, INFO_ON);
        }
        else
        {
            send_to_char("You will now see info messages.\n\r", ch);
            SET_BIT(ch->info, INFO_ON);
        }
        return;
    }

    /* "on" argument: explicitly turn on */
    if (!str_prefix(argument, "on"))
    {
        send_to_char("You will now see info messages.\n\r", ch);
        SET_BIT(ch->info, INFO_ON);
        return;
    }

    /* "off" argument: explicitly turn off */
    if (!str_prefix(argument, "off"))
    {
        send_to_char("You will no longer see info messages.\n\r", ch);
        REMOVE_BIT(ch->info, INFO_ON);
        return;
    }

    /* "status" argument: show current info settings */
    if (!str_prefix(argument, "status"))
    {
        buf[0] = '\0';

        if (!IS_SET(ch->info, INFO_ON))
            strcat(buf, "off ");

        for (i = 0; info_table[i].name != NULL; i++)
            if (IS_SET(ch->info, info_table[i].flag))
            {
                strcat(buf, info_table[i].name);
                strcat(buf, " ");
            }

        strcat(buf, "\n\r");

        send_to_char("You receive the following items of information:\n\r", ch);
        send_to_char(buf, ch);
        return;
    }

    /* "show" argument: list all available info options */
    if (!str_prefix(argument, "show"))
    {
        buf[0] = '\0';

        for (i = 0; info_table[i].name != NULL; i++)
        {
            if (info_table[i].level <= get_trust(ch))
            {
                strcat(buf, info_table[i].name);
                strcat(buf, " ");
            }
        }

        strcat(buf, "\n\r");

        send_to_char("The following info options are available to you:\n\r", ch);
        send_to_char(buf, ch);
        return;
    }

    /* Try to toggle a specific info category */
    flag = info_lookup(argument);

    if (flag == 0 || get_trust(ch) < info_table[flag].level)
    {
        send_to_char("No such option.\n\r", ch);
        return;
    }

    if (IS_SET(ch->info, info_table[flag].flag))
    {
        sprintf(buf, "You will no longer see %s on the info channel.\n\r",
                info_table[flag].name);
        send_to_char(buf, ch);
        REMOVE_BIT(ch->info, info_table[flag].flag);
        return;
    }
    else
    {
        sprintf(buf, "You will now see %s on the info channel.\n\r",
                info_table[flag].name);
        send_to_char(buf, ch);
        SET_BIT(ch->info, info_table[flag].flag);
        return;
    }
}

/* Broadcast a message to all players subscribed to this info category.
 * 
 * Parameters:
 *   string    - The message to broadcast
 *   ch        - The character sending the broadcast (can be excluded from recv)
 *   obj       - Optional object context (for act_new)
 *   flag      - Required info flag to receive this message (0 = no flag requirement)
 *   flag_skip - Skip recipients with this flag set (0 = no skip)
 *   min_level - Minimum trust level required to receive this message
 */
void news(const char *string, CHAR_T *ch, OBJ_T *obj,
          flag_t flag, flag_t flag_skip, int min_level)
{
    DESCRIPTOR_T *d;

    for (d = descriptor_first; d != NULL; d = d->global_next)
    {
        if (d->connected == CON_PLAYING
            && IS_SET(d->character->info, INFO_ON)
            && (!flag || IS_SET(d->character->info, flag))
            && (!flag_skip || !IS_SET(d->character->info, flag_skip))
            && get_trust(d->character) >= min_level
            && d->character != ch)
        {
            send_to_char("{mINFO:{x ", d->character);
            act_new(string, d->character, obj, ch, TO_CHAR, POS_DEAD);
        }
    }
}

/* Lookup an info category by name.
 * Returns the index into info_table[], or 0 if not found or access denied.
 */
flag_t info_lookup(const char *name)
{
    int flag;

    for (flag = 0; info_table[flag].name != NULL; flag++)
    {
        if (LOWER(name[0]) == LOWER(info_table[flag].name[0])
            && !str_prefix(name, info_table[flag].name))
            return flag;
    }

    return 0;
}
