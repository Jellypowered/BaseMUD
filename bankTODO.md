This will be adapted and implemented at a later date. 

/***************************************************************************
 *        Port of ROM 2.4b6 to Windows by TAKA                             *
 *                                                                         *
 *  I call this "NEW ROM" Ghost since it has changed extensively.          *
 *  This version runs as a native console application                      *
 *  windows NT 4 service patch 3, Windows 95 software version release 2    *
 *  and windows 98.                                                        *
 *                                                                         *
 *  Ghost to my knowledge is the first ROM port to windows to be 100%      *
 *  stable.                                                                *
 *                                                                         *
 *  I compiled it using MS Vissual C++ 5.0                                 *
 *  THE FOLLOWING MODULES WERE MODIFIED TO ALLOW STABLE WINDOWS OPERATION  *
 *  - merc.h                                                               *
 *  - comm.c                                                               *
 *  - db.c                                                                 *
 *  BE SURE TO INCLUDE IN THE LINK LIST                                    *
 *  - wsock32.lib                                                          *
 *                                                                         *
 *  Other source files are straight from the ROM24b6 distribution.         *
 *                                                                         *
 *  ALL I ASK FOR MY WORK:                                                 *
 *  1) Is that you do not remove or modify this comment block.             *
 *  2) You must state on the entry screen that this is GHOST a ROM code    *
 *     base designed and ported to windows by TAKA    .                    *
 *  3) The latest version number from you source listing must be must be   *
 *     in the help file as well as my name TAKA                            *
 *     and my email a_ghost_dancer@excite.com                              *
 *     < < Simplist way is to leave my help entry supplied > >             *
 *  4) You must abid by all other ROM and MERC licences                    *
 *  5) Finally any improvements or snippets you make please forward them   *
 *     to me so we can imcorperate them into the latest version. I will    *
 *     post all snippets with the authors named and credited.              *
 *                                                                         *
 *  Last thank you to all the ROM amd MERC folks for this wounderful code  *
 *  base know as ROM.                                                      *
 *                                                                         *
 *          TAKA                                                           *
 *          a_ghost_dancer@excite.com                                      *
 *                                                                         *
 ***************************************************************************


 *
 * TAKA     banking/investment system.
 * modled in part after maniacs code.
 *
 */
#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "merc.h"


DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_save		);



void do_bank( CHAR_DATA *ch, char *argument )
{
	/*
	 * based on:
	 * Simple banking system. by -- Stephen --
	 *
	 */

	CHAR_DATA *mob;
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	if ( IS_NPC( ch ) )
	{
		send_to_char( "{RBanking Services are only available to players!{x\n\r", ch );
		return;
	}

	/* Check for mob with act->banker */
	for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
	{
		if (( IS_NPC(mob) && IS_SET(mob->act, ACT_BANKER ) )
			|| ( IS_NPC(mob) && IS_SET(mob->act, ACT_ATM )))
			break;
	}

	if ( mob == NULL )
	{
		send_to_char( "{RYou can't do that here.{x\n\r", ch );
		return;
	}

	if ((!IS_SET(mob->act, ACT_ATM)) || (BANK_ALLOW_ATM != 1))
	{
		if ((time_info.hour < 9) || (time_info.hour > 17))
		{
			send_to_char( "{RThe bank is closed, it is open from {W9am {Rto {W5pm{R.{x\n\r", ch);
			return;
		}
	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "{GBanking system by {WTAKA {Goptions:{x\n\r\n\r", ch );
		send_to_char( "{GBank balance		{M: {WDisplays your balance.{x\n\r", ch );
		send_to_char( "{GBank deposit		{M: {WDeposit gold into your account.{x\n\r", ch );
		send_to_char( "{GBank withdraw		{M: {WWithdraw gold from your account.{x\n\r", ch );
		if (BANK_DEPOSIT_SILVER == 1)
		{
			send_to_char("{GBank swithdraw		{M: {WWithdraw silver from your account.{x\n\r", ch );
			send_to_char("{GBank sdeposit		{M: {WDeposit silver into your account.{x\n\r", ch );
		}
		if (BANK_CONVERT_SILVER == 1)
			send_to_char( "{GBank convert		{M: {WConverts {D100 silver {Wto {y1 gold{W.{x\n\r", ch );
		#if defined BANK_TRANSFER 
			if (IS_SET(mob->act, ACT_BANKER))
			{
				send_to_char( "{GBank transfer	:	{M: {WTransfer  gold to  account.{x\n\r", ch);
				send_to_char( "{GBank buy1  #		{M: {WBuy # shares in type I stocks.{x\n\r", ch);
				send_to_char( "{GBank buy2  #		{M: {WBuy # shares in type II stocks.{x\n\r", ch);
				send_to_char( "{GBank buy3  #		{M: {WBuy # shares in type III stocks.{x\n\r", ch);
				send_to_char( "{GBank buy4  #		{M: {WBuy # shares in type IV stocks.{x\n\r", ch);

				send_to_char( "{GBank sell1  #		{M: {WSell # shares of type I stocks.{x\n\r", ch);
				send_to_char( "{GBank sell2  #		{M: {WSell # shares of type II stocks.{x\n\r", ch);
				send_to_char( "{GBank sell3  #		{M: {WSell # shares of type III stocks.{x\n\r", ch);
				send_to_char( "{GBank sell4  #		{M: {WSell # shares of type IV stocks.{x\n\r", ch);

				send_to_char( "{GBank check		{M: {WCheck the current rates of the shares.{x \n\r", ch);
			}
		#endif
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	/* Now work out what to do... */
	if ( !str_prefix( arg1, "balance" ) )
	{
		if (BANK_DEPOSIT_SILVER != 1)
			sprintf(buf,"{GYour current balance is: {y%ld GP{G.{x\n\r",ch->pcdata->balance );
		else
			sprintf(buf,"{GYour current balance is: {y%ld GP {Gand {D%ld SP{G.{x\n\r",
				ch->pcdata->balance, ch->pcdata->sbalance );
		do_say(mob, buf);
		return;
	}

	if ( !str_prefix( arg1, "deposit" ) )
	{
		int amount;
		if ( is_number ( arg2 ) )
		{
			amount = atoi( arg2 );

			if (amount > ch->gold )
			{
				sprintf( buf, "{RHow can you deposit {y%d GP {Rwhen you only have {y%ld{R?{x\n\r", amount, ch->gold );
				do_say(mob, buf );
				return;
			}

			if (amount < 0 )
			{
				do_say (mob, "{ROnly positive amounts allowed...{x");
				return;
			}

			ch->gold -= amount;
			ch->pcdata->balance += amount;
			sprintf ( buf, "{GYou deposit {y%d GP{G.  Your new balance is {y%ld GP{G.{x\n\r",
				amount, ch->pcdata->balance );
			send_to_char( buf, ch );

			do_save( ch, "" );
			return;
		}
	}

	if (( !str_prefix( arg1, "sdeposit" ) ) && (BANK_DEPOSIT_SILVER == 1))
	{
		int amount;
		if ( is_number ( arg2 ) )
		{
				amount = atoi( arg2 );

			if (amount > ch->silver )
			{
				sprintf( buf, "{RHow can you deposit {D%d SP {Rwhen you only have {D%ld{R?{x\n\r", amount, ch->silver );
				do_say(mob, buf );
				return;
			}

			if (amount < 0 )
			{
				do_say (mob, "{ROnly positive amounts allowed...{x");
				return;
			}

			ch->silver -= amount;
			ch->pcdata->sbalance += amount;
			sprintf ( buf, "{GYou deposit {D%d SP{G.  Your new balance is {D%ld SP{G.{x\n\r",
				amount, ch->pcdata->sbalance );
			send_to_char( buf, ch );

			do_save( ch, "" );
			return;
		}
	}

	/* We only allow transfers if this is true... so define it... */
	#if defined BANK_TRANSFER
		if ( !str_prefix( arg1, "transfer" ) )
		{
			int amount;
			CHAR_DATA *victim;

			if ( is_number ( arg2 ) )
			{
				amount = atoi( arg2 );

				if ( amount > ch->pcdata->balance )
				{
					sprintf( buf, "{RHow can you transfer {y%d GP {Gwhen your balance is {y%ld{R?{x\n\r",
						amount, ch->pcdata->balance );
					do_say( mob, buf);
					return;
				}

				if (amount < 0 )
				{
					do_say (mob, "{ROnly positive amounts allowed...{x");
					return;
				}

				if ( !( victim = get_char_world( ch, argument ) ) )
				{
					sprintf (buf, "{W%s {Rdoesn't have a bank account.{x\n\r", argument );
					do_say( mob, buf );
					return;
				}

				if (IS_NPC(victim))
				{
					do_say( mob, "{RYou can only transfer money to players.{x");
					return;
				}

				ch->pcdata->balance     -= amount;
				victim->pcdata->balance += amount;
				sprintf( buf, "{GYou transfer {y%d GP{G. Your new balance is {y%ld GP{G.{x\n\r",
					amount, ch->pcdata->balance );
				send_to_char( buf, ch );

				sprintf (buf, "{C[{WBANK{C] {W%s {Ghas transferred {y%d gold pieces {Gto your account.{x\n\r",
					ch->name, amount);
				send_to_char( buf, victim );

				do_save( ch, "" );
				do_save( victim, "");
				return;
			}
		}
	#endif

	if ( !str_prefix( arg1, "withdraw" ) )
	{
		int amount;

		if ( is_number ( arg2 ) )
		{
			amount = atoi( arg2 );
			if ( amount > ch->pcdata->balance )
			{
				sprintf( buf, "{RHow can you withdraw {y%d GP {Rwhen your balance is {y%ld{R?{x\n\r",
					amount, ch->pcdata->balance );
				do_say (mob, buf );
				return;
			}

			if (amount < 0 )
			{
				do_say( mob, "{ROnly positive amounts allowed...{x");
				return;
			}
			
			if (IS_SET(mob->act, ACT_ATM))
			{
				if ((amount >= (BANK_ALLOW_ATM_MAX + 1)) 
					|| ((ch->pcdata->dailylimit + amount) >= (BANK_ALLOW_ATM_MAX + 1)))
				{
					sprintf( buf, "{RYou can only withdraw {y%d GP {Rfrom an ATM daily.{x\n\r",
						BANK_ALLOW_ATM_MAX );
					do_say(mob, buf);
					return;
				}
				else
					ch->pcdata->dailylimit += amount;
			}


			ch->pcdata->balance -= amount;
			ch->gold += amount;
			sprintf( buf, "{GYou withdraw {y%d GP{G.  Your new balance is {y%ld GP{G.{x\n\r",
				amount, ch->pcdata->balance );
			send_to_char( buf, ch );
			do_save( ch, "" );
			return;
		}
	}

	if (( !str_prefix( arg1, "swithdraw" ) )  && (BANK_DEPOSIT_SILVER == 1))
	{
		int amount;

		if ( is_number ( arg2 ) )
		{
			amount = atoi( arg2 );
			if ( amount > ch->pcdata->sbalance )
			{
				sprintf( buf, "{RHow can you withdraw {D%d SP {Rwhen your balance is {D%ld{R?{x\n\r",
					amount, ch->pcdata->sbalance );
				do_say (mob, buf );
				return;
			}

			if (amount < 0 )
			{
				do_say( mob, "{ROnly positive amounts allowed...{x");
				return;
			}
			
			if (IS_SET(mob->act, ACT_ATM))
			{
				if ((amount >= ((BANK_ALLOW_ATM_MAX * 100) + 1))
					|| (((ch->pcdata->dailylimit * 100) + amount) >= ((BANK_ALLOW_ATM_MAX * 100) + 1)))
				{
					sprintf( buf, "{RYou can only withdraw {D%d SP{R from an ATM daily.{x\n\r",
						(BANK_ALLOW_ATM_MAX * 100) );
					do_say(mob, buf);
					return;
				}
				else
					ch->pcdata->dailylimit += (amount / 100);
			}

			ch->pcdata->sbalance -= amount;
			ch->silver += amount;
			sprintf( buf, "{GYou withdraw {D%d SP{G.  Your new balance is {D%ld SP{G.{x\n\r",
				amount, ch->pcdata->sbalance );
			send_to_char( buf, ch );
			do_save( ch, "" );
			return;
		}
	}

	if (( !str_prefix( arg1, "convert" ) ) && (BANK_CONVERT_SILVER == 1 ))
	{
		int amount;

		if ( is_number ( arg2 ) )
		{
			amount = atoi( arg2 );
			if ( amount > ch->pcdata->sbalance )
			{
				sprintf( buf, "{RHow can you convert {D%d SP {Rwhen your balance is {D%ld{R?{x\n\r",
					amount, ch->pcdata->sbalance );
				do_say (mob, buf );
				return;
			}

			if (amount < 0 )
			{
				do_say( mob, "{ROnly positive amounts allowed...{x");
				return;
			}

			if ((amount % 100) != 0)
			{
				do_say( mob, "{RYou can only convert silver in increaments of {D100{R....{x");
				return;
			}

			ch->pcdata->sbalance -= amount;
			ch->pcdata->balance += (amount / 100);
			sprintf( buf, "{GYou convert {D%d SP {Gto {y%d GP{G.  Your new balance is {y%ld GP{G.{x\n\r",
				amount, (amount / 100), ch->pcdata->balance );
			send_to_char( buf, ch );
			do_save( ch, "" );
			return;
		}
	}


	/* If you want to have an invest option... define BANK_INVEST */
	#if defined BANK_INVEST
		if ((time_info.hour < 9) || (time_info.hour > 17))
		{
			send_to_char( "{RThe stock market is closed, it is open from {W9am {Rto {W5pm{R.{x\n\r", ch);
			return;
		}

		if ( !str_prefix( arg1, "buy1" ) )
		{
			int amount;
			if ( is_number ( arg2 ) )
			{
				amount = atoi( arg2 );

				if ( (amount * share_value1) > ch->pcdata->balance )
				{
					sprintf( buf, "{W%d {Rtype I shares will cost you {y%d{R, get more money.{x\n\r",
						amount, (amount * share_value1) );
					do_say(mob, buf);
					return;
				}

				if (amount < 0 )
				{
					do_say(mob, "{RIf you want to sell shares you have to say so...{x");
					return;
				}

				ch->pcdata->balance -= (amount * share_value1);
				ch->pcdata->shares1  += amount;
				sprintf( buf, "{GYou buy {W%d {Gtype I shares for {y%d GP{G, you now have {W%d {Gtype I shares.{x\n\r",
					amount, (amount * share_value1), ch->pcdata->shares1 );
				do_say(mob, buf);
				do_save( ch, "" );
				return;
			}
		}
		if ( !str_prefix( arg1, "buy2" ) )
		{
			int amount;
			if ( is_number ( arg2 ) )
			{
				amount = atoi( arg2 );

				if ( (amount * share_value2) > ch->pcdata->balance )
				{
					sprintf( buf, "{W%d {Rtype II shares will cost you {y%d{R, get more money.{x\n\r",
						amount, (amount * share_value2) );
					do_say(mob, buf);
					return;
				}

				if (amount < 0 )
				{
					do_say(mob, "{RIf you want to sell shares you have to say so...{x");
					return;
				}

				ch->pcdata->balance -= (amount * share_value2);
				ch->pcdata->shares2  += amount;
				sprintf( buf, "{GYou buy {W%d {Gtype II shares for {y%d GP{G, you now have {W%d {Gtype II shares.{x\n\r",
					amount, (amount * share_value2), ch->pcdata->shares2 );
				do_say(mob, buf);
				do_save( ch, "" );
				return;
			}
		}
		if ( !str_prefix( arg1, "buy3" ) )
		{
			int amount;
			if ( is_number ( arg2 ) )
			{
				amount = atoi( arg2 );

				if ( (amount * share_value3) > ch->pcdata->balance )
				{
					sprintf( buf, "{W%d {Rtype III shares will cost you {y%d{R, get more money.{x\n\r",
						amount, (amount * share_value3) );
					do_say(mob, buf);
					return;
				}

				if (amount < 0 )
				{
					do_say(mob, "{RIf you want to sell shares you have to say so...{x");
					return;
				}

				ch->pcdata->balance -= (amount * share_value3);
				ch->pcdata->shares3  += amount;
				sprintf( buf, "{GYou buy {W%d {Gtype III shares for {y%d GP{G, you now have {W%d {Gtype III shares.{x\n\r",
					amount, (amount * share_value3), ch->pcdata->shares3 );
				do_say(mob, buf);
				do_save( ch, "" );
				return;
			}
		}
		if ( !str_prefix( arg1, "buy4" ) )
		{
			int amount;
			if ( is_number ( arg2 ) )
			{
				amount = atoi( arg2 );

				if ( (amount * share_value4) > ch->pcdata->balance )
				{
					sprintf( buf, "{W%d {Rtype IV shares will cost you {y%d{R, get more money.{x\n\r",
						amount, (amount * share_value4) );
					do_say(mob, buf);
					return;
				}

				if (amount < 0 )
				{
					do_say(mob, "{RIf you want to sell shares you have to say so...{x");
					return;
				}

				ch->pcdata->balance -= (amount * share_value4);
				ch->pcdata->shares4  += amount;
				sprintf( buf, "{GYou buy {W%d {Gtype IV shares for {y%d GP{G, you now have {W%d {Gtype IV shares.{x\n\r",
					amount, (amount * share_value4), ch->pcdata->shares4 );
				do_say(mob, buf);
				do_save( ch, "" );
				return;
			}
		}

		if ( !str_prefix( arg1, "sell1" ) )
		{
			int amount;

			if ( is_number ( arg2 ) )
			{
				amount = atoi( arg2 );

				if ( amount > ch->pcdata->shares1 )
				{
					sprintf( buf, "{RYou only have {W%d {Rtype I shares.{x\n\r", ch->pcdata->shares1 );
					do_say(mob, buf);
					return;
				}

				if (amount < 0 )
				{
					do_say (mob, "{RIf you want to buy shares you have to say so...{x");
					return;
				}

				ch->pcdata->balance += (amount * share_value1);
				ch->pcdata->shares1  -= amount;
				sprintf( buf, "{GYou sell {W%d {Gtype I shares for {y%d GP{G, you now have {W%d {Gtype I shares.{x\n\r",
					amount, (amount * share_value1), ch->pcdata->shares1 );
				do_say (mob, buf);
				do_save( ch, "" );
				return;
			}
		}
		if ( !str_prefix( arg1, "sell2" ) )
		{
			int amount;

			if ( is_number ( arg2 ) )
			{
				amount = atoi( arg2 );

				if ( amount > ch->pcdata->shares2 )
				{
					sprintf( buf, "{RYou only have {W%d {Rtype II shares.{x\n\r", ch->pcdata->shares2 );
					do_say(mob, buf);
					return;
				}

				if (amount < 0 )
				{
					do_say (mob, "{RIf you want to buy shares you have to say so...{x");
					return;
				}

				ch->pcdata->balance += (amount * share_value2);
				ch->pcdata->shares2  -= amount;
				sprintf( buf, "{GYou sell {W%d {Gtype II shares for {y%d GP{G, you now have {W%d {Gtype II shares.{x\n\r",
					amount, (amount * share_value2), ch->pcdata->shares2 );
				do_say (mob, buf);
				do_save( ch, "" );
				return;
			}
		}
		if ( !str_prefix( arg1, "sell3" ) )
		{
			int amount;

			if ( is_number ( arg2 ) )
			{
				amount = atoi( arg2 );

				if ( amount > ch->pcdata->shares3 )
				{
					sprintf( buf, "{RYou only have {W%d {Rtype III shares.{x\n\r", ch->pcdata->shares3 );
					do_say(mob, buf);
					return;
				}

				if (amount < 0 )
				{
					do_say (mob, "{RIf you want to buy shares you have to say so...{x");
					return;
				}

				ch->pcdata->balance += (amount * share_value3);
				ch->pcdata->shares3  -= amount;
				sprintf( buf, "{GYou sell {W%d {Gtype III shares for {y%d GP{G, you now have {W%d {Gtype III shares.{x\n\r",
					amount, (amount * share_value3), ch->pcdata->shares3 );
				do_say (mob, buf);
				do_save( ch, "" );
				return;
			}
		}
		if ( !str_prefix( arg1, "sell4" ) )
		{
			int amount;

			if ( is_number ( arg2 ) )
			{
				amount = atoi( arg2 );

				if ( amount > ch->pcdata->shares4 )
				{
					sprintf( buf, "{RYou only have {W%d {Rtype IV shares.{x\n\r", ch->pcdata->shares4 );
					do_say(mob, buf);
					return;
				}

				if (amount < 0 )
				{
					do_say (mob, "{RIf you want to buy shares you have to say so...{x");
					return;
				}

				ch->pcdata->balance += (amount * share_value4);
				ch->pcdata->shares4  -= amount;
				sprintf( buf, "{GYou sell {W%d {Gtype IV shares for {y%d GP{G, you now have {W%d {Gtype IV shares.{x\n\r",
					amount, (amount * share_value4), ch->pcdata->shares4 );
				do_say (mob, buf);
				do_save( ch, "" );
				return;
			}
		}

		if ( !str_prefix( arg1, "check" ) )
		{
			sprintf (buf, "{GThe current shareprice for type {WI {Gshares is {y%d GP{G.{x\n\r",share_value1);
			do_say(mob, buf);
			sprintf (buf, "{GThe current shareprice for type {WII {Gshares is {y%d GP{G.{x\n\r",share_value2);
			do_say(mob, buf);
			sprintf (buf, "{GThe current shareprice for type {WIII {Gshares is {y%d GP{G.{x\n\r",share_value3);
			do_say(mob, buf);
			sprintf (buf, "{GThe current shareprice for type {WIV {Gshares is {y%d GP{G.{x\n\r",share_value4);
			do_say(mob, buf);

			if ((ch->pcdata->shares1) || (ch->pcdata->shares2)
				|| (ch->pcdata->shares3) || (ch->pcdata->shares4))
			{
				sprintf (buf, "{GYou currently have {W%d {Gtype I shares, {C({W%d {Ga share{C) {Gworth a total of {y%d gold{G.{x\n\r",
					ch->pcdata->shares1, share_value1, (ch->pcdata->shares1 * share_value1) );
				do_say(mob, buf);
				sprintf (buf, "{GYou currently have {W%d {Gtype II shares, {C({W%d {Ga share{C) {Gworth a total of {y%d gold{G.{x\n\r",
					ch->pcdata->shares2, share_value2, (ch->pcdata->shares2 * share_value2) );
				do_say(mob, buf);
				sprintf (buf, "{GYou currently have {W%d {Gtype III shares, {C({W%d {Ga share{C) {Gworth a total of {y%d gold{G.{x\n\r",
					ch->pcdata->shares3, share_value4, (ch->pcdata->shares3 * share_value3) );
				do_say(mob, buf);
				sprintf (buf, "{GYou currently have {W%d {Gtype IV shares, {C({W%d {Ga share{C) {Gworth a total of {y%d gold{G.{x\n\r",
					ch->pcdata->shares4, share_value4, (ch->pcdata->shares4 * share_value4) );
				do_say(mob, buf);

			}

			return;
		}
	#endif

	do_say(mob, "{RI don't know what you mean{x");


	do_bank( ch, "" );

	/* Generate Instructions */
	return;
}