/****************************************************************************
 *  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
 *  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   *
 *  code is allowed provided you add a credit line to the effect of:         *
 *  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
 *  of the standard diku/rom credits. If you use this or a modified version  *
 *  of this code, let me know via email: moongate@moongate.ams.com. Further  *
 *  updates will be posted to the rom mailing list. If you'd like to get     *
 *  the latest version of quest.c, please send a request to the above add-   *
 *  ress. Quest Code v2.03. Please do not remove this notice from this file. *
 ****************************************************************************/

#include "merc.h"

#include <string.h>
#include "act_comm.h"
#include "chars.h"
#include "comm.h"
#include "find.h"
#include "globals.h"
#include "interp.h"
#include "lookup.h"
#include "mobiles.h"
#include "objs.h"
#include "players.h"
#include "tables.h"
#include "utils.h"

/* Object vnums for object quest 'tokens' are now loaded from
   json/config/quest_tokens.json into quest_token_table[]. */

/* Local functions */

void generate_quest args((CHAR_T * ch, CHAR_T *questman));
void quest_give_reward args((CHAR_T *ch, CHAR_T *questman, int n_targets));
void quest_update args((void));
bool quest_level_diff args((int clevel, int mlevel));
bool chance args((int num));

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool chance(int num)
{
    if (number_range(1, 100) <= num)
        return TRUE;
    else
        return FALSE;
}

/* Award quest completion rewards: XP, gold, QP, rare practice, very rare train.
 * n_targets: 1 for classic quests, N for purge/collection multi-target quests.
 * Bonus: +25% per extra target beyond 1. */
void quest_give_reward(CHAR_T *ch, CHAR_T *questman, int n_targets)
{
    char buf[MAX_STRING_LENGTH];
    int n          = UMAX(1, n_targets);
    int divisor    = UMAX(1, quest_config.reward_level_divisor);
    int xp_pct     = number_range(
                         URANGE(1, quest_config.xp_reward_min_pct, 5),
                         URANGE(1, quest_config.xp_reward_max_pct, 5));
    int reward, pointreward, xpreward;

    reward      = number_range(quest_config.gold_min, quest_config.gold_max)
                      * UMAX(1, ch->level) / divisor;
    pointreward = number_range(quest_config.qp_min, quest_config.qp_max)
                      * UMAX(1, ch->level) / divisor;
    xpreward    = player_get_exp_per_level(ch) * xp_pct / 100;

    /* Multi-target bonus: +25% per extra target */
    if (n > 1)
    {
        reward      += reward      * (n - 1) / 4;
        pointreward += pointreward * (n - 1) / 4;
        xpreward    += xpreward    * (n - 1) / 4;
    }

    sprintf(buf, "Congratulations on completing your quest!");
    do_say(questman, buf);
    sprintf(buf, "As a reward, I give you %d quest points, %d gold, and %d experience.",
            pointreward, reward, xpreward);
    do_say(questman, buf);

    player_gain_exp(ch, xpreward);
    ch->gold        += reward;
    ch->questpoints += pointreward;

    /* Rare: bonus practice points */
    if (chance(quest_config.practice_chance))
    {
        int prac = number_range(quest_config.practice_min, quest_config.practice_max);
        sprintf(buf, "You also gain %d practice%s!\n\r", prac, prac == 1 ? "" : "s");
        send_to_char(buf, ch);
        ch->practice += prac;
    }

    /* Very rare: a training session */
    if (chance(quest_config.train_chance))
    {
        ch->train += 1;
        send_to_char("A moment of inspiration grants you a training session!\n\r", ch);
    }
}

/* The main quest function */

void do_quest(CHAR_T *ch, char *argument)
{
    CHAR_T *questman;
    OBJ_T *obj, *obj_next;
    OBJ_INDEX_T *questinfoobj;
    MOB_INDEX_T *questinfo;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        send_to_char("QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY.\n\r", ch);
        send_to_char("For more information, type 'HELP QUEST'.\n\r", ch);
        return;
    }
    if (!strcmp(arg1, "info"))
    {
        if (EXT_IS_SET(ch->ext_plr, PLR_QUESTOR))
        {
            if (ch->questmob == -1 && ch->questgiver && ch->questgiver->short_descr != NULL)
            {
                sprintf(buf, "Your quest is ALMOST complete!\n\rGet back to %s before your time runs out!\n\r", ch->questgiver->short_descr);
                send_to_char(buf, ch);
            }
            else if (ch->questobj > 0)
            {
                questinfoobj = obj_get_index(ch->questobj);
                if (questinfoobj != NULL)
                {
                    if (ch->questcount_max > 1)
                    {
                        int have = 0;
                        OBJ_T *scan;
                        for (scan = ch->content_first; scan != NULL; scan = scan->content_next)
                            if (scan->obj_index->vnum == ch->questobj) have++;
                        sprintf(buf, "You are on a quest to collect %d %s! (%d/%d in your inventory)\n\r",
                            ch->questcount_max, questinfoobj->name, have, ch->questcount_max);
                    }
                    else
                        sprintf(buf, "You are on a quest to recover the fabled %s!\n\r", questinfoobj->name);
                    send_to_char(buf, ch);
                }
                else
                    send_to_char("You aren't currently on a quest.\n\r", ch);
                return;
            }
            else if (ch->questmob > 0)
            {
                questinfo = mobile_get_index(ch->questmob);
                if (questinfo != NULL)
                {
                    if (ch->questcount_max > 1)
                        sprintf(buf, "You must slay %d %s! (%d/%d slain)\n\r",
                            ch->questcount_max, questinfo->short_descr,
                            ch->questcount, ch->questcount_max);
                    else
                        sprintf(buf, "You are on a quest to slay the dreaded %s!\n\r", questinfo->short_descr);
                    send_to_char(buf, ch);
                }
                else
                    send_to_char("You aren't currently on a quest.\n\r", ch);
                return;
            }
        }
        else
            send_to_char("You aren't currently on a quest.\n\r", ch);

        /* Always show XP progress toward next quest chance. */
        {
            int threshold = UMAX(1, player_get_exp_per_level(ch) / quest_config.xp_chance_divisor);
            sprintf(buf, "Quest chances: {C%d{x  ({Y%d{x / {Y%d{x xp to next)\n\r",
                ch->quest_chances, ch->quest_xp_prog, threshold);
            send_to_char(buf, ch);
        }
        return;
    }
    if (!strcmp(arg1, "points"))
    {
        sprintf(buf, "You have {Y%d{x quest point%s and {C%d{x quest chance%s.\n\r",
            ch->questpoints, ch->questpoints == 1 ? "" : "s",
            ch->quest_chances, ch->quest_chances == 1 ? "" : "s");
        send_to_char(buf, ch);
        return;
    }
    else if (!strcmp(arg1, "time"))
    {
        if (!EXT_IS_SET(ch->ext_plr, PLR_QUESTOR))
        {
            send_to_char("You aren't currently on a quest.\n\r", ch);
            if (ch->nextquest > 1)
            {
                sprintf(buf, "There are %d minutes remaining until you can go on another quest.\n\r", ch->nextquest);
                send_to_char(buf, ch);
            }
            else if (ch->nextquest == 1)
            {
                sprintf(buf, "There is less than a minute remaining until you can go on another quest.\n\r");
                send_to_char(buf, ch);
            }
        }
        else if (ch->countdown > 0)
        {
            sprintf(buf, "Time left for current quest: %d\n\r", ch->countdown);
            send_to_char(buf, ch);
        }
        return;
    }

    /* Checks for a character in the room with spec_questmaster set. This special
       procedure must be defined in special.c. You could instead use an
       ACT_QUESTMASTER flag instead of a special procedure. */

    for (questman = ch->in_room->people_first; questman != NULL; questman = questman->room_next)
    {
        if (!IS_NPC(questman))
            continue;
        if (questman->spec_fun == spec_lookup_function("spec_questmaster"))
            break;
    }

    if (questman == NULL || questman->spec_fun != spec_lookup_function("spec_questmaster"))
    {
        send_to_char("You can't do that here.\n\r", ch);
        return;
    }

    if (questman->fighting != NULL)
    {
        send_to_char("Wait until the fighting stops.\n\r", ch);
        return;
    }

    ch->questgiver = questman;

    /* And, of course, you will need to change the following lines for YOUR
       quest item information. Quest items on Moongate are unbalanced, very
       very nice items, and no one has one yet, because it takes awhile to
       build up quest points :> Make the item worth their while. */

    if (!strcmp(arg1, "list"))
    {
        int i;
        act("$n asks $N for a list of quest items.", ch, NULL, questman, TO_OTHERS);
        act("You ask $N for a list of quest items.", ch, NULL, questman, TO_CHAR);
        send_to_char("Current Quest Items available for Purchase:\n\r", ch);
        for (i = 0; i < quest_reward_count; i++)
        {
            const QUEST_REWARD_T *qr = &quest_reward_table[i];
            sprintf(buf, "{Y%5dqp{x.........%s\n\r", qr->cost, qr->label);
            send_to_char(buf, ch);
        }
        send_to_char("To buy an item, type 'QUEST BUY <item>'.\n\r", ch);
        return;
    }

    else if (!strcmp(arg1, "buy"))
    {
        int i;
        const QUEST_REWARD_T *qr = NULL;

        if (arg2[0] == '\0')
        {
            send_to_char("To buy an item, type 'QUEST BUY <item>'.\n\r", ch);
            return;
        }

        for (i = 0; i < quest_reward_count; i++)
        {
            if (str_in_namelist(arg2, quest_reward_table[i].keywords))
            {
                qr = &quest_reward_table[i];
                break;
            }
        }

        if (qr == NULL)
        {
            sprintf(buf, "I don't have that item, %s.", ch->name);
            do_say(questman, buf);
            return;
        }

        if (ch->questpoints < qr->cost)
        {
            sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", ch->name);
            do_say(questman, buf);
            return;
        }

        ch->questpoints -= qr->cost;

        if (!strcmp(qr->type, "object"))
        {
            OBJ_INDEX_T *idx = obj_get_index(qr->value);
            OBJ_T *obj = NULL;
            if (idx != NULL)
                obj = obj_create(idx, ch->level);
            if (obj != NULL)
            {
                if (IS_OBJ_STAT(obj, ITEM_REWARD))
                    obj_reward_scale(obj, ch->level);
                act("$N gives $p to $n.", ch, obj, questman, TO_OTHERS);
                act("$N gives you $p.", ch, obj, questman, TO_CHAR);
                obj_give_to_char(obj, ch);
            }
            else
            {
                send_to_char("The reward item could not be found - contact an immortal.\n\r", ch);
                ch->questpoints += qr->cost; /* refund */
            }
        }
        else if (!strcmp(qr->type, "practices"))
        {
            ch->practice += qr->value;
            act("$N gives practices to $n.", ch, NULL, questman, TO_OTHERS);
            sprintf(buf, "$N gives you %d practices.", qr->value);
            act(buf, ch, NULL, questman, TO_CHAR);
        }
        else if (!strcmp(qr->type, "gold"))
        {
            ch->gold += qr->value;
            act("$N transfers gold to $n.", ch, NULL, questman, TO_OTHERS);
            sprintf(buf, "$N transfers %d gold pieces to you.", qr->value);
            act(buf, ch, NULL, questman, TO_CHAR);
        }
        else if (!strcmp(qr->type, "quest_chance"))
        {
            ch->quest_chances += qr->value;
            act("$N grants $n an additional quest chance.", ch, NULL, questman, TO_OTHERS);
            sprintf(buf, "$N grants you %d additional quest chance%s.",
                qr->value, qr->value == 1 ? "" : "s");
            act(buf, ch, NULL, questman, TO_CHAR);
        }
        else
        {
            /* Unknown special reward type — log and refund. */
            sprintf(buf, "Unknown quest reward type '%s' - contact an immortal.", qr->type);
            send_to_char(buf, ch);
            ch->questpoints += qr->cost;
        }
        return;
    }
    else if (!strcmp(arg1, "request"))
    {
        act("$n asks $N for a quest.", ch, NULL, questman, TO_OTHERS);
        act("You ask $N for a quest.", ch, NULL, questman, TO_CHAR);
        if (EXT_IS_SET(ch->ext_plr, PLR_QUESTOR))
        {
            sprintf(buf, "But you're already on a quest!");
            do_say(questman, buf);
            return;
        }
        if (ch->nextquest > 0)
        {
            sprintf(buf, "You're very brave, %s, but let someone else have a chance.", ch->name);
            do_say(questman, buf);
            sprintf(buf, "Come back later.");
            do_say(questman, buf);
            return;
        }
        if (ch->quest_chances <= 0)
        {
            int threshold = UMAX(1, player_get_exp_per_level(ch) / quest_config.xp_chance_divisor);
            sprintf(buf, "You haven't earned a quest chance yet, %s.", ch->name);
            do_say(questman, buf);
            sprintf(buf, "Kill some monsters and come back! (%d / %d xp to next chance)",
                ch->quest_xp_prog, threshold);
            do_say(questman, buf);
            return;
        }
        ch->quest_chances--;

        sprintf(buf, "Thank you, brave %s!", ch->name);
        do_say(questman, buf);
        ch->questmob = 0;
        ch->questobj = 0;
        ch->questcount = 0;
        ch->questcount_max = 0;

        generate_quest(ch, questman);

        if (ch->questmob > 0 || ch->questobj > 0)
        {
            ch->countdown = number_range(quest_config.quest_timer_min, quest_config.quest_timer_max);
            EXT_SET(ch->ext_plr, PLR_QUESTOR);
            sprintf(buf, "You have %d minutes to complete this quest.", ch->countdown);
            do_say(questman, buf);
            sprintf(buf, "May the gods go with you!");
            do_say(questman, buf);
        }
        return;
    }
    else if (!strcmp(arg1, "complete"))
    {
        act("$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_OTHERS);
        act("You inform $N you have completed $s quest.", ch, NULL, questman, TO_CHAR);
        if (ch->questgiver != questman)
        {
            sprintf(buf, "I never sent you on a quest! Perhaps you're thinking of someone else.");
            do_say(questman, buf);
            return;
        }

        if (EXT_IS_SET(ch->ext_plr, PLR_QUESTOR))
        {
            if (ch->questmob == -1 && ch->countdown > 0)
            {
                quest_give_reward(ch, questman, UMAX(1, ch->questcount_max));
                EXT_UNSET(ch->ext_plr, PLR_QUESTOR);
                ch->questgiver = NULL;
                ch->countdown = 0;
                ch->questmob = 0;
                ch->questobj = 0;
                ch->questcount = 0;
                ch->questcount_max = 0;
                ch->nextquest = quest_config.cooldown_success;
                return;
            }
            else if (ch->questobj > 0 && ch->countdown > 0)
            {
                int needed = UMAX(1, ch->questcount_max);
                int have = 0;

                for (obj = ch->content_first; obj != NULL; obj = obj->content_next)
                    if (obj->obj_index->vnum == ch->questobj) have++;

                if (have >= needed)
                {
                    int to_extract = needed;

                    for (obj = ch->content_first; obj != NULL && to_extract > 0; obj = obj_next)
                    {
                        obj_next = obj->content_next;
                        if (obj->obj_index->vnum == ch->questobj)
                        {
                            act("You hand $p to $N.", ch, obj, questman, TO_CHAR);
                            act("$n hands $p to $N.", ch, obj, questman, TO_OTHERS);
                            obj_extract(obj);
                            to_extract--;
                        }
                    }

                    quest_give_reward(ch, questman, UMAX(1, ch->questcount_max));
                    EXT_UNSET(ch->ext_plr, PLR_QUESTOR);
                    ch->questgiver = NULL;
                    ch->countdown = 0;
                    ch->questmob = 0;
                    ch->questobj = 0;
                    ch->questcount = 0;
                    ch->questcount_max = 0;
                    ch->nextquest = quest_config.cooldown_success;
                    return;
                }
                else
                {
                    if (needed > 1)
                    {
                        sprintf(buf, "You still need %d more of those! (%d/%d collected)",
                            needed - have, have, needed);
                        do_say(questman, buf);
                    }
                    else
                    {
                        sprintf(buf, "You haven't completed the quest yet, but there is still time!");
                        do_say(questman, buf);
                    }
                    return;
                }
            }
            else if ((ch->questmob > 0 || ch->questobj > 0) && ch->countdown >
                                                                   0)
            {
                sprintf(buf, "You haven't completed the quest yet, but there is still time!");
                do_say(questman, buf);
                return;
            }
        }
        if (ch->nextquest > 0)
            sprintf(buf, "But you didn't complete your quest in time!");
        else
            sprintf(buf, "You have to REQUEST a quest first, %s.", ch->name);
        do_say(questman, buf);
        return;
    }

    send_to_char("QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY.\n\r", ch);
    send_to_char("For more information, type 'HELP QUEST'.\n\r", ch);
    return;
}

void generate_quest(CHAR_T *ch, CHAR_T *questman)
{
    CHAR_T *victim;
    ROOM_INDEX_T *room;
    OBJ_T *questitem = NULL;
    char buf[MAX_STRING_LENGTH];

    /*  Randomly selects a mob from the world mob list. If you don't
        want a mob to be selected, make sure it is immune to summon.
        Or, you could add a new mob flag called ACT_NOQUEST. The mob
        is selected for both mob and obj quests, even tho in the obj
        quest the mob is not used. This is done to assure the level
        of difficulty for the area isn't too great for the player. */

    /*  Uniformly selects a qualifying mob from the world mob list using
        reservoir sampling (size 1). Each qualifying mob has an equal chance
        of being chosen regardless of its position in the global list.
        To exclude a mob from quests, flag it MOB_NOQUEST or make it
        immune to summon. */

    {
        CHAR_T *cand;
        int n_candidates = 0;
        victim = NULL;
        for (cand = char_first; cand != NULL; cand = cand->global_next)
        {
            if (!IS_NPC(cand))
                continue;
            if (quest_level_diff(ch->level, cand->level) == TRUE && !IS_SET(cand->res_flags, RES_SUMMON) && cand->mob_index != NULL && cand->mob_index->shop == NULL && !EXT_IS_SET(cand->ext_mob, MOB_PET) && !EXT_IS_SET(cand->ext_mob, MOB_NOQUEST) && !IS_AFFECTED(cand, AFF_CHARM))
            {
                n_candidates++;
                if (number_range(1, n_candidates) == 1)
                    victim = cand;
            }
        }
    }

    if (victim == NULL)
    {
        do_say(questman, "I'm sorry, but I don't have any quests for you at this time.");
        do_say(questman, "Try again later.");
        ch->nextquest = quest_config.cooldown_none;
        return;
    }

    if ((room = find_location(ch, victim->name)) == NULL)
    {
        sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
        do_say(questman, buf);
        sprintf(buf, "Try again later.");
        do_say(questman, buf);
        ch->nextquest = quest_config.cooldown_none;
        return;
    }

    /*  40% chance it will send the player on a 'recover item' quest. */

    if (chance(quest_config.obj_quest_chance))
    {
        /* Scan obj_index_hash for a level-appropriate, takeable, non-special item
         * using reservoir sampling so every qualifying item has an equal chance. */
        OBJ_INDEX_T *chosen_idx = NULL;
        bool is_collection = chance(quest_config.collect_quest_chance);
        int n_items = is_collection
            ? number_range(quest_config.collect_count_min, quest_config.collect_count_max)
            : 1;

        {
            OBJ_INDEX_T *cand;
            int n_cand = 0, bucket;

            for (bucket = 0; bucket < MAX_KEY_HASH; bucket++)
            {
                for (cand = obj_index_hash[bucket]; cand != NULL; cand = cand->hash_next)
                {
                    /* Must be takeable */
                    if (!(cand->wear_flags & ITEM_TAKE))
                        continue;
                    /* Skip special-purpose or plot items */
                    if (cand->extra_flags &
                        (ITEM_NOPURGE | ITEM_INVENTORY | ITEM_REWARD | ITEM_NODROP | ITEM_NOLOCATE))
                        continue;
                    /* Skip unsuitable item types */
                    switch (cand->item_type)
                    {
                        case ITEM_TRASH:      case ITEM_CORPSE_NPC: case ITEM_CORPSE_PC:
                        case ITEM_MONEY:      case ITEM_FOUNTAIN:   case ITEM_PORTAL:
                        case ITEM_ROOM_KEY:   case ITEM_BOAT:       case ITEM_JUKEBOX:
                        case ITEM_DRINK_CON:  case ITEM_FOOD:       case ITEM_KEY:
                        case ITEM_WARP_STONE: case ITEM_MAP:        case ITEM_NONE:
                            continue;
                        default:
                            break;
                    }
                    /* Level check: item level 0 suits any character;
                     * otherwise must be within appropriate bands */
                    if (cand->level != 0 && !quest_level_diff(ch->level, cand->level))
                        continue;
                    /* Reservoir sample */
                    n_cand++;
                    if (number_range(1, n_cand) == 1)
                        chosen_idx = cand;
                }
            }
        }

        /* If no world item found and it's a single-item quest, fall back to
         * a treasury quest token. */
        if (chosen_idx == NULL && !is_collection)
        {
            int objvnum = 0;
            if (quest_token_count > 0)
                objvnum = quest_token_table[number_range(0, quest_token_count - 1)].vnum;
            chosen_idx = obj_get_index(objvnum);
        }

        if (chosen_idx != NULL)
        {
            int i;

            for (i = 0; i < n_items; i++)
            {
                questitem = obj_create(chosen_idx, ch->level);
                obj_give_to_room(questitem, room);
            }
            ch->questobj      = chosen_idx->vnum;
            ch->questcount    = 0;
            ch->questcount_max = n_items;

            if (n_items > 1)
            {
                sprintf(buf, "Reports say %d %s have gone missing in the region!",
                    n_items, chosen_idx->short_descr);
                do_say(questman, buf);
                do_say(questman, "Track them all down and bring them to me!");
            }
            else
            {
                sprintf(buf, "Vile pilferers have stolen %s! Find it and bring it back!",
                    chosen_idx->short_descr);
                do_say(questman, buf);
            }

            sprintf(buf, "Look in the general area of %s for %s!", room->area->name, room->name);
            do_say(questman, buf);
            return;
        }
        /* No suitable item found - fall through to mob quest */
    }

    /* Quest to kill a mob */

    {
        int n_kills = 1;
        if (chance(quest_config.purge_quest_chance))
            n_kills = number_range(quest_config.purge_count_min, quest_config.purge_count_max);

        ch->questcount = 0;
        ch->questcount_max = n_kills;
        ch->questmob = victim->mob_index->vnum;

        if (n_kills > 1)
        {
            switch (number_range(0, 1))
            {
            case 0:
                sprintf(buf, "%s and their ilk are overrunning the countryside!", victim->short_descr);
                do_say(questman, buf);
                sprintf(buf, "Slay %d of them to restore peace!", n_kills);
                do_say(questman, buf);
                break;
            case 1:
                sprintf(buf, "A pack of %s is terrorising the region!", victim->short_descr);
                do_say(questman, buf);
                sprintf(buf, "I need you to hunt down and slay %d of them!", n_kills);
                do_say(questman, buf);
                break;
            }
        }
        else
        {
            switch (number_range(0, 1))
            {
            case 0:
                sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.", victim->short_descr);
                do_say(questman, buf);
                sprintf(buf, "This threat must be eliminated!");
                do_say(questman, buf);
                break;

            case 1:
                sprintf(buf, "Rune's most heinous criminal, %s, has escaped from the dungeon!", victim->short_descr);
                do_say(questman, buf);
                sprintf(buf, "Since the escape, %s has murdered %d civillians!", victim->short_descr, number_range(2, 20));
                do_say(questman, buf);
                do_say(questman, "The penalty for this crime is death, and you are to deliver the sentence!");
                break;
            }
        }

        if (room->name != NULL)
        {
            sprintf(buf, "Seek %s out somewhere in the vicinity of %s!", victim->short_descr, room->name);
            do_say(questman, buf);

            /* I changed my area names so that they have just the name of the area
               and none of the level stuff. You may want to comment these next two
               lines. - Vassago */

            sprintf(buf, "That location is in the general area of %s.", room->area->name);
            do_say(questman, buf);
        }
    }
    return;
}

/* Level differences to search for. Moongate has 350
   levels, so you will want to tweak these greater or
   less than statements for yourself. - Vassago */

bool quest_level_diff(int clevel, int mlevel)
{
    if (clevel < 6 && mlevel < 24)
        return TRUE;
    else if (clevel > 6 && clevel < 20 && mlevel < 54)
        return TRUE;
    else if (clevel > 19 && clevel < 60 && mlevel > 45 && mlevel < 135)
        return TRUE;
    else if (clevel > 59 && clevel < 100 && mlevel > 120 && mlevel < 200)
        return TRUE;
    else if (clevel > 99 && clevel < 200 && mlevel > 180 && mlevel < 350)
        return TRUE;
    else if (clevel >= 200 && mlevel > 315)
        return TRUE;
    else
        return FALSE;
}

/* Called from update_handler() by pulse_area */

void quest_update(void)
{
    DESCRIPTOR_T *d;
    CHAR_T *ch;

    for (d = descriptor_first; d != NULL; d = d->global_next)
    {
        if (d->character != NULL && d->connected == CON_PLAYING)
        {

            ch = d->character;

            if (ch->nextquest > 0)
            {
                ch->nextquest--;
                if (ch->nextquest == 0)
                {
                    send_to_char("You may now quest again.\n\r", ch);
                    return;
                }
            }
            else if (EXT_IS_SET(ch->ext_plr, PLR_QUESTOR))
            {
                if (--ch->countdown <= 0)
                {
                    char buf[MAX_STRING_LENGTH];

                    ch->nextquest = quest_config.cooldown_success;
                    sprintf(buf, "You have run out of time for your quest!"
                                 "\n\rYou may quest again in %d minutes.\n\r",
                            ch->nextquest);
                    send_to_char(buf, ch);
                    EXT_UNSET(ch->ext_plr, PLR_QUESTOR);
                    ch->questgiver = NULL;
                    ch->countdown = 0;
                    ch->questmob = 0;
                    ch->questcount = 0;
                    ch->questcount_max = 0;
                }
                if (ch->countdown > 0 && ch->countdown < 6)
                {
                    send_to_char("Better hurry, you're almost out of time for your quest!\n\r", ch);
                    return;
                }
            }
        }
    }
    return;
}
