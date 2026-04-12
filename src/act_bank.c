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

/*
 * Banking system.
 * Adapted from TAKA's banking system.
 */

#include "act_bank.h"

#include "act_comm.h"
#include "chars.h"
#include "comm.h"
#include "find.h"
#include "globals.h"
#include "interp.h"
#include "memory.h"
#include "mobiles.h"
#include "players.h"
#include "save.h"
#include "tables.h"
#include "utils.h"

#include <stdlib.h>

DEFINE_DO_FUN(do_bank)
{
    CHAR_T *banker;
    CHAR_T *victim;
    char buf[MAX_STRING_LENGTH], arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int amount;
    bool is_atm;

    /* Only players can use the bank. */
    if (IS_NPC(ch)) {
        send_to_char("Banking services are not available to NPCs.\n\r", ch);
        return;
    }

    /* Find a banker or ATM in the room. */
    banker = NULL;
    is_atm = FALSE;
    for (banker = ch->in_room->people_first; banker; banker = banker->room_next) {
        if (IS_NPC(banker)) {
            if (ext_flags_is_set(banker->ext_mob, MOB_BANKER)) {
                is_atm = FALSE;
                break;
            }
            if (ext_flags_is_set(banker->ext_mob, MOB_ATM)) {
                is_atm = TRUE;
                break;
            }
        }
    }

    BAIL_IF(banker == NULL, "You can't do that here.\n\r", ch);

    /* Get arguments. */
    if (argument[0] == '\0') {
        send_to_char("Banking system options:\n\r\n\r", ch);
        send_to_char("  bank balance        - Display your account balance\n\r", ch);
        send_to_char("  bank deposit <amt>  - Deposit gold into your account\n\r", ch);
        send_to_char("  bank withdraw <amt> - Withdraw gold from your account\n\r", ch);

        if (banking_config.silver_deposit_enabled) {
            send_to_char("  bank sdeposit <amt> - Deposit silver into your account\n\r", ch);
            send_to_char("  bank swithdraw <amt> - Withdraw silver from your account\n\r", ch);
        }

        if (banking_config.silver_convert_enabled) {
            send_to_char("  bank convert <amt>  - Convert silver to gold (100 silver = 1 gold)\n\r", ch);
        }

        if (!is_atm && banking_config.transfer_enabled) {
            send_to_char("  bank transfer <who> <amt> - Transfer gold to another player\n\r", ch);
        }

        return;
    }

    /* Check business hours (unless ATM with bypass). */
    if (!is_atm || !banking_config.atm_allow_bypass) {
        if (time_info.hour < banking_config.bank_open_hour ||
            time_info.hour >= banking_config.bank_close_hour)
        {
            sprintf(buf, "The bank is closed. It is open from %d:00 to %d:00.\n\r",
                    banking_config.bank_open_hour, banking_config.bank_close_hour);
            send_to_char(buf, ch);
            return;
        }
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    /* Balance command. */
    if (!str_prefix(arg1, "balance")) {
        if (banking_config.silver_deposit_enabled) {
            sprintf(buf, "Your current balance is: {Y%ld{x gold and {W%ld{x silver.\n\r",
                    ch->pcdata->balance, ch->pcdata->sbalance);
        } else {
            sprintf(buf, "Your current balance is: {Y%ld{x gold.\n\r", ch->pcdata->balance);
        }
        send_to_char(buf, ch);
        return;
    }

    /* Deposit gold. */
    if (!str_prefix(arg1, "deposit")) {
        BAIL_IF(arg2[0] == '\0' || !is_number(arg2),
                "Usage: bank deposit <amount>\n\r", ch);

        amount = atoi(arg2);
        BAIL_IF(amount > ch->gold,
                "You don't have that much gold.\n\r", ch);
        BAIL_IF(amount <= 0,
                "You must deposit a positive amount.\n\r", ch);

        ch->gold -= amount;
        ch->pcdata->balance += amount;
        sprintf(buf, "You deposit {Y%d{x gold into your account. New balance: {Y%ld{x gold.\n\r",
                amount, ch->pcdata->balance);
        send_to_char(buf, ch);

        act2("You deposit gold into your account.",
             "$n deposits gold into $s account.", ch, NULL, NULL, 0, POS_RESTING);

        save_char_obj(ch);
        return;
    }

    /* Withdraw gold. */
    if (!str_prefix(arg1, "withdraw")) {
        BAIL_IF(arg2[0] == '\0' || !is_number(arg2),
                "Usage: bank withdraw <amount>\n\r", ch);

        amount = atoi(arg2);
        BAIL_IF(amount > ch->pcdata->balance,
                "You don't have that much in your account.\n\r", ch);
        BAIL_IF(amount <= 0,
                "You must withdraw a positive amount.\n\r", ch);

        /* Check ATM daily limit. */
        if (is_atm && banking_config.atm_daily_limit > 0) {
            if (amount > banking_config.atm_daily_limit) {
                sprintf(buf, "ATM daily limit is {Y%d{x gold. You can withdraw up to {Y%d{x.\n\r",
                        banking_config.atm_daily_limit,
                        banking_config.atm_daily_limit);
                send_to_char(buf, ch);
                return;
            }
        }

        ch->pcdata->balance -= amount;
        ch->gold += amount;
        sprintf(buf, "You withdraw {Y%d{x gold from your account. New balance: {Y%ld{x gold.\n\r",
                amount, ch->pcdata->balance);
        send_to_char(buf, ch);

        act2("You withdraw gold from your account.",
             "$n withdraws gold from $s account.", ch, NULL, NULL, 0, POS_RESTING);

        save_char_obj(ch);
        return;
    }

    /* Transfer gold to another player. */
    if (!str_prefix(arg1, "transfer")) {
        BAIL_IF(!banking_config.transfer_enabled,
                "Transfers are not available.\n\r", ch);
        BAIL_IF(is_atm,
                "Transfers are not available at ATMs.\n\r", ch);

        argument = one_argument(argument, arg2);
        BAIL_IF(arg2[0] == '\0' || argument[0] == '\0',
                "Usage: bank transfer <player> <amount>\n\r", ch);
        BAIL_IF(!is_number(argument),
                "Usage: bank transfer <player> <amount>\n\r", ch);

        amount = atoi(argument);
        BAIL_IF(amount > ch->pcdata->balance,
                "You don't have that much in your account.\n\r", ch);
        BAIL_IF(amount <= 0,
                "You must transfer a positive amount.\n\r", ch);

        victim = find_char_world(ch, arg2);
        BAIL_IF(victim == NULL,
                "That player is not online.\n\r", ch);
        BAIL_IF(IS_NPC(victim),
                "You can only transfer money to players.\n\r", ch);

        ch->pcdata->balance -= amount;
        victim->pcdata->balance += amount;

        sprintf(buf, "You transfer {Y%d{x gold to %s. New balance: {Y%ld{x gold.\n\r",
                amount, victim->name, ch->pcdata->balance);
        send_to_char(buf, ch);

        sprintf(buf, "%s has transferred {Y%d{x gold to your account.\n\r", ch->name, amount);
        send_to_char(buf, victim);

        save_char_obj(ch);
        save_char_obj(victim);
        return;
    }

    /* Silver deposit. */
    if (!str_prefix(arg1, "sdeposit")) {
        BAIL_IF(!banking_config.silver_deposit_enabled,
                "Silver deposits are not available.\n\r", ch);
        BAIL_IF(arg2[0] == '\0' || !is_number(arg2),
                "Usage: bank sdeposit <amount>\n\r", ch);

        amount = atoi(arg2);
        BAIL_IF(amount > ch->silver,
                "You don't have that much silver.\n\r", ch);
        BAIL_IF(amount <= 0,
                "You must deposit a positive amount.\n\r", ch);

        ch->silver -= amount;
        ch->pcdata->sbalance += amount;
        sprintf(buf, "You deposit {W%d{x silver into your account. New balance: {W%ld{x silver.\n\r",
                amount, ch->pcdata->sbalance);
        send_to_char(buf, ch);

        act2("You deposit silver into your account.",
             "$n deposits silver into $s account.", ch, NULL, NULL, 0, POS_RESTING);

        save_char_obj(ch);
        return;
    }

    /* Silver withdraw. */
    if (!str_prefix(arg1, "swithdraw")) {
        BAIL_IF(!banking_config.silver_deposit_enabled,
                "Silver withdrawals are not available.\n\r", ch);
        BAIL_IF(arg2[0] == '\0' || !is_number(arg2),
                "Usage: bank swithdraw <amount>\n\r", ch);

        amount = atoi(arg2);
        BAIL_IF(amount > ch->pcdata->sbalance,
                "You don't have that much silver in your account.\n\r", ch);
        BAIL_IF(amount <= 0,
                "You must withdraw a positive amount.\n\r", ch);

        /* Check ATM daily limit for silver. */
        if (is_atm && banking_config.atm_daily_limit_silver > 0) {
            if (amount > banking_config.atm_daily_limit_silver) {
                sprintf(buf, "ATM daily limit for silver is {W%d{x. You can withdraw up to {W%d{x.\n\r",
                        banking_config.atm_daily_limit_silver,
                        banking_config.atm_daily_limit_silver);
                send_to_char(buf, ch);
                return;
            }
        }

        ch->pcdata->sbalance -= amount;
        ch->silver += amount;
        sprintf(buf, "You withdraw {W%d{x silver from your account. New balance: {W%ld{x silver.\n\r",
                amount, ch->pcdata->sbalance);
        send_to_char(buf, ch);

        act2("You withdraw silver from your account.",
             "$n withdraws silver from $s account.", ch, NULL, NULL, 0, POS_RESTING);

        save_char_obj(ch);
        return;
    }

    /* Convert silver to gold. */
    if (!str_prefix(arg1, "convert")) {
        BAIL_IF(!banking_config.silver_convert_enabled,
                "Silver conversions are not available.\n\r", ch);
        BAIL_IF(arg2[0] == '\0' || !is_number(arg2),
                "Usage: bank convert <amount> (must be 100+ silver)\n\r", ch);

        amount = atoi(arg2);
        BAIL_IF(amount > ch->pcdata->sbalance,
                "You don't have that much silver in your account.\n\r", ch);
        BAIL_IF(amount <= 0,
                "You must convert a positive amount.\n\r", ch);
        BAIL_IF(amount % 100 != 0,
                "You can only convert silver in increments of 100.\n\r", ch);

        ch->pcdata->sbalance -= amount;
        ch->pcdata->balance += (amount / 100);
        sprintf(buf, "You convert {W%d{x silver to {Y%d{x gold. New balance: {Y%ld{x gold, {W%ld{x silver.\n\r",
                amount, amount / 100, ch->pcdata->balance, ch->pcdata->sbalance);
        send_to_char(buf, ch);

        save_char_obj(ch);
        return;
    }

    /* Unknown subcommand. */
    send_to_char("Unknown banking command. Type 'bank' for help.\n\r", ch);
}
