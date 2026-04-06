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

/***************************************************************************
 *  Wizlist - ported from ROT 1.4 to BaseMUD                               *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wizlist.h"

#include "chars.h"
#include "comm.h"
#include "fread.h"
#include "fwrite.h"
#include "globals.h"
#include "interp.h"
#include "memory.h"
#include "recycle.h"
#include "utils.h"

static const char *const wiz_titles[] = {
    "Implementors",
    "Creators    ",
    "Supremacies ",
    "Deities     ",
    "Gods        ",
    "Immortals   ",
    "DemiGods    ",
    "Knights     ",
    "Squires     "};

/* Local prototypes */
static void wizlist_change(CHAR_T *ch, bool add, int level, char *argument);

void wizlist_save_all(void)
{
    WIZ_T *pwiz;
    FILE *fp;
    bool found = FALSE;

    fclose(reserve_file);
    if ((fp = fopen(WIZ_FILE, "w")) == NULL)
    {
        perror(WIZ_FILE);
        reserve_file = fopen(NULL_FILE, "r");
        return;
    }

    for (pwiz = wiz_first; pwiz != NULL; pwiz = pwiz->global_next)
    {
        found = TRUE;
        fprintf(fp, "%s %d\n", pwiz->name, pwiz->level);
    }

    fclose(fp);
    reserve_file = fopen(NULL_FILE, "r");
    if (!found)
        unlink(WIZ_FILE);
}

void wizlist_load_all(void)
{
    FILE *fp;

    if ((fp = fopen(WIZ_FILE, "r")) == NULL)
        return;

    for (;;)
    {
        WIZ_T *pwiz;

        if (feof(fp))
        {
            fclose(fp);
            return;
        }

        pwiz = wiz_new();
        fread_word_replace(fp, &pwiz->name);
        pwiz->level = fread_number(fp);
        fread_to_eol(fp);

        LIST2_BACK(pwiz, global_prev, global_next, wiz_first, wiz_last);
    }
}

void do_wizlist(CHAR_T *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char title[80];
    BUFFER_T *buffer;
    int level;
    WIZ_T *pwiz;
    int lngth;
    int amt;
    bool found;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg1[0] != '\0' && ch->level == MAX_LEVEL)
    {
        if (!str_prefix(arg1, "add"))
        {
            if (!is_number(arg2) || arg3[0] == '\0')
            {
                send_to_char("Syntax: wizlist add <level> <name>\n\r", ch);
                return;
            }
            level = atoi(arg2);
            wizlist_change(ch, TRUE, level, arg3);
            return;
        }
        if (!str_prefix(arg1, "delete"))
        {
            if (arg2[0] == '\0')
            {
                send_to_char("Syntax: wizlist delete <name>\n\r", ch);
                return;
            }
            wizlist_change(ch, FALSE, 0, arg2);
            return;
        }
        send_to_char("Syntax:\n\r", ch);
        send_to_char("       wizlist delete <name>\n\r", ch);
        send_to_char("       wizlist add <level> <name>\n\r", ch);
        return;
    }

    if (wiz_first == NULL)
    {
        send_to_char("No immortals listed at this time.\n\r", ch);
        return;
    }

    buffer = buf_new();
    sprintf(title, "The Immortals");
    sprintf(buf, "{x  ___________________________________________________________________________\n\r");
    buf_cat(buffer, buf);
    sprintf(buf, "{x /\\_\\%70s\\_\\\n\r", " ");
    buf_cat(buffer, buf);
    lngth = (70 - (int)strlen(title)) / 2;
    for (; lngth >= 0; lngth--)
        strcat(title, " ");
    sprintf(buf, "|/\\\\_\\{W%70s{x\\_\\\n\r", title);
    buf_cat(buffer, buf);
    sprintf(buf, "{x\\_/_|_|%69s|_|\n\r", " ");
    buf_cat(buffer, buf);

    for (level = IMPLEMENTOR; level > HERO; level--)
    {
        found = FALSE;
        amt = 0;
        for (pwiz = wiz_first; pwiz != NULL; pwiz = pwiz->global_next)
        {
            if (pwiz->level == level)
            {
                amt++;
                found = TRUE;
            }
        }
        if (!found)
        {
            if (level == HERO + 1)
            {
                sprintf(buf, "{x ___|_|%69s|_|\n\r", " ");
                buf_cat(buffer, buf);
            }
            continue;
        }
        sprintf(buf, "{x    |_|{R%37s {B[%d]{x%26s|_|\n\r",
                wiz_titles[IMPLEMENTOR - level], level, " ");
        buf_cat(buffer, buf);
        sprintf(buf, "{x    |_|{Y%25s******************{x%26s|_|\n\r",
                " ", " ");
        buf_cat(buffer, buf);

        lngth = 0;
        for (pwiz = wiz_first; pwiz != NULL; pwiz = pwiz->global_next)
        {
            if (pwiz->level != level)
                continue;
            if (lngth == 0)
            {
                if (amt > 2)
                {
                    sprintf(buf, "{x    |_|{%s%12s%-17s ",
                            level >= DEMI ? "G" : "C", " ", pwiz->name);
                    buf_cat(buffer, buf);
                    lngth = 1;
                }
                else if (amt > 1)
                {
                    sprintf(buf, "{x    |_|{%s%21s%-17s ",
                            level >= DEMI ? "G" : "C", " ", pwiz->name);
                    buf_cat(buffer, buf);
                    lngth = 1;
                }
                else
                {
                    sprintf(buf, "{x    |_|{%s%30s%-39s{x|_|\n\r",
                            level >= DEMI ? "G" : "C", " ", pwiz->name);
                    buf_cat(buffer, buf);
                    lngth = 0;
                }
            }
            else if (lngth == 1)
            {
                if (amt > 2)
                {
                    sprintf(buf, "%-17s ", pwiz->name);
                    buf_cat(buffer, buf);
                    lngth = 2;
                }
                else
                {
                    sprintf(buf, "%-30s{x|_|\n\r", pwiz->name);
                    buf_cat(buffer, buf);
                    lngth = 0;
                }
            }
            else
            {
                sprintf(buf, "%-21s{x|_|\n\r", pwiz->name);
                buf_cat(buffer, buf);
                lngth = 0;
                amt -= 3;
            }
        }

        if (level == HERO + 1)
            sprintf(buf, "{x ___|_|%69s|_|\n\r", " ");
        else
            sprintf(buf, "{x    |_|%69s|_|\n\r", " ");
        buf_cat(buffer, buf);
    }

    sprintf(buf, "{x/ \\ |_|%69s|_|\n\r", " ");
    buf_cat(buffer, buf);
    sprintf(buf, "{x|\\//_/%70s/_/\n\r", " ");
    buf_cat(buffer, buf);
    sprintf(buf, "{x \\/_/______________________________________________________________________/_/\n\r");
    buf_cat(buffer, buf);
    page_to_char(buf_string(buffer), ch);
    buf_free(buffer);
}

void wizlist_update(CHAR_T *ch, int level)
{
    WIZ_T *curr;

    if (IS_NPC(ch))
        return;

    /* Remove existing entry for this character, if any. */
    for (curr = wiz_first; curr != NULL; curr = curr->global_next)
    {
        if (!str_cmp(ch->name, curr->name))
        {
            LIST2_REMOVE(curr, global_prev, global_next, wiz_first, wiz_last);
            wiz_free(curr);
            wizlist_save_all();
            break;
        }
    }

    if (level <= HERO)
        return;

    curr = wiz_new();
    str_free(&curr->name);
    curr->name = str_dup(ch->name);
    curr->level = level;
    LIST2_BACK(curr, global_prev, global_next, wiz_first, wiz_last);
    wizlist_save_all();
}

static void wizlist_change(CHAR_T *ch, bool add, int level, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    WIZ_T *curr;

    one_argument(argument, arg);
    if (arg[0] == '\0')
    {
        send_to_char("Syntax:\n\r", ch);
        if (!add)
            send_to_char("    wizlist delete <name>\n\r", ch);
        else
            send_to_char("    wizlist add <level> <name>\n\r", ch);
        return;
    }

    if (add && (level <= HERO || level > MAX_LEVEL))
    {
        send_to_char("Syntax:\n\r", ch);
        send_to_char("    wizlist add <level> <name>\n\r", ch);
        return;
    }

    if (!add)
    {
        for (curr = wiz_first; curr != NULL; curr = curr->global_next)
        {
            if (!str_cmp(str_capitalized(arg), curr->name))
            {
                LIST2_REMOVE(curr, global_prev, global_next,
                             wiz_first, wiz_last);
                wiz_free(curr);
                wizlist_save_all();
                return;
            }
        }
    }
    else
    {
        curr = wiz_new();
        str_free(&curr->name);
        curr->name = str_dup(str_capitalized(arg));
        curr->level = level;
        LIST2_BACK(curr, global_prev, global_next, wiz_first, wiz_last);
        wizlist_save_all();
    }
}
