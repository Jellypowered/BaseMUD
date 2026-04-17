/* Voltecs info system, based on wiznet!  Nice and simple!(?)  *
 *  Version 1.0       Voltec -98			       */
 
 Make the following changes to the specified files, and then decide what bits of
 info you want your players to see, and add a call to info() (same format as a 
 wiznet call) and add a flag to the list / flag table.  This isn't the most
 straight foreward info code I've ever seen, but it is very versatile, as players
 can select what items they receive.  I sugest setting new players so that they receive
 all info reports, and then letting them turn off the reports that annoy them.  (makes
 sense if you ask me, not that you did...)
 
 It should be possible to make wiznet a special subset of info by re-organising the
 flag tables for the 2 systems, and crowbaring it into the code!  Watch out for
 version 2.0!

 Also rather than having all these additions scattered all over the place, prehaps
 putting them in a file called info.c would be sensible...  Hmm...  Might just do that!
 
 
MERC.H
======
add the following lines to merc.h just after the corresponding wiznet lines

(Arround line 2100)
extern	const	struct	info_type	info_table	[];

(Arround line 350)
struct info_type
{
    char *	name;
    long 	flag;
    int		level;
};

add a line to the char_data declaration

	long	info;

and a section of...

/* INFO flags */
#define INFO_ON			(A)
#define INFO_LEVELS		(B)
#define INFO_CONSENT		(C)

INTERP.C - INTERP.H
===================

Add the usual procedure/command declarations...

SAVE.C
======

Add the following arround line 226 (In the function fwrite_char() 
Just after the Wiznet entry...)

    if (ch->info)
    	fprintf( fp, "Info %s\n",   print_flags(ch->info));
		
in fread_char()	in the I section of the case statement add the following line..

	    KEY( "Info",	ch->info,		fread_flag( fp ) );
		

CUT OUT AND KEEP THANG...
------------------------------------------------------------------
/******************************************************************
 *                     INFO.C Version 1.1                         *
 *        Written by Voltec (Voltec@cyberdude.com) 1998           *
 *      For Empire Of The Night (eotn.oaktree.co.uk 4100)         *
 *       All the usual DIKU/MERC/ROM/EOTN Licences apply          *
 *  If you use this code, drop me a line so I can see if people   *
 *                 Actually find this usefull.                    *
 *                         REMEMBER                               *
 *      Share and Enjoy!                      -Voltec 98          *
 ******************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/* Local functions... */
void do_info(CHAR_DATA *ch, char *argument);
void info(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	             long flag, long flag_skip, int min_level);
long info_lookup (const char *name);

/* info table and prototype for future flag setting 	*
 * Add more flags as you see fit...			*
 * this works the same way as wiznet... Voltec -98	*/
 
const   struct info_type info_table[] =
  {

/*      NAME			FLAG		  LEVEL */

   {    "on",           INFO_ON,        0  },
   {    "levels",	INFO_LEVELS,	4  },
   {    "consent",      INFO_CONSENT,   12 },
   {	NULL,		0,		0  }
   };



/* Voltecs Info Channel */
void do_info( CHAR_DATA *ch, char *argument )
{
    int flag;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
      	if (IS_SET(ch->info,INFO_ON))
      	{
            send_to_char("You will no longer see info messages.\n\r",ch);
            REMOVE_BIT(ch->info,INFO_ON);
      	}
      	else
      	{
            send_to_char("You will now see info messages.\n\r",ch);
            SET_BIT(ch->info,INFO_ON);
      	}
      	return;
    }

    if (!str_prefix(argument,"on"))
    {
	send_to_char("You will now see info messages.\n\r",ch);
	SET_BIT(ch->info,INFO_ON);
	return;
    }

    if (!str_prefix(argument,"off"))
    {
	send_to_char("You will no longer see info messages.\n\r",ch);
	REMOVE_BIT(ch->info,INFO_ON);
	return;
    }

	/* Right, thats the basic on off switch done, now for the options! */

    /* show info status */
    if (!str_prefix(argument,"status")) 
    {
	buf[0] = '\0';

	if (!IS_SET(ch->info,INFO_ON))
	    strcat(buf,"off ");

	for (flag = 0; info_table[flag].name != NULL; flag++) /*Cycle through the info flags*/
	    if (IS_SET(ch->info,info_table[flag].flag))
	    {
		strcat(buf,info_table[flag].name);		/* If the char has the flag enabled, tell them! */
		strcat(buf," ");
	    }

	strcat(buf,"\n\r");

	send_to_char("You receive the following items of information:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }

/* Now to see what options are available to a player... */

    if (!str_prefix(argument,"show"))
    /* list of all info options */
    {
	buf[0] = '\0';

	for (flag = 0; info_table[flag].name != NULL; flag++) /*Cycle through the info table*/
	{
	    if (info_table[flag].level <= get_trust(ch)) /* If the char is allowed the option, tell them! */
	    {
	    	strcat(buf,info_table[flag].name);
	    	strcat(buf," ");
	    }
	}

	strcat(buf,"\n\r");

	send_to_char("The following info options are available to you:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }

/* Toggle an option! */

    flag = info_lookup(argument);

    if (flag == -1 || get_trust(ch) < info_table[flag].level)
    {
	send_to_char("No such option.\n\r",ch);
	return;
    }
   
    if (IS_SET(ch->info,info_table[flag].flag))
    {
	sprintf(buf,"You will no longer see %s on the info channel.\n\r",
	        info_table[flag].name);
	send_to_char(buf,ch);
	REMOVE_BIT(ch->info,info_table[flag].flag);
    	return;
    }
    else
    {
    	sprintf(buf,"You will now see %s on the info channel.\n\r",
		        info_table[flag].name);
	send_to_char(buf,ch);
    SET_BIT(ch->info,info_table[flag].flag);
	return;
    }

}

/* Voltecs Info Channel */
void info(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	             long flag, long flag_skip, int min_level) 
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->connected == CON_PLAYING 
	&&  IS_SET(d->character->info, INFO_ON) 
	&&  (!flag || IS_SET(d->character->info,flag))
	&&  (!flag_skip || !IS_SET(d->character->info,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  d->character != ch)
        {
	  	send_to_char("{mINFO:{x ",d->character);
            act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD);
        }
    }
    return;
}

/* returns a flag for Voltecs info channel */
long info_lookup (const char *name)
{
    int flag;

    for (flag = 0; info_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(info_table[flag].name[0])
	&& !str_prefix(name,info_table[flag].name))
	    return flag;
    }

    return -1;
}

-----------------------------<END OF INFO.C>----------------------------------
Add the following help to your help files...

0 INFO~

INFO
Syntax: info 
	info show
	info status
	info <field>

Info is sort of a news service, to show important events to
anyone who cares to listen.  Info by itself turns info on and off, 
info show lists all settable flags available to you (they are not 
detailed here),  info status shows your current info settings, and 
info <field> toggles a field on and off.  The events should be self-explanatory, if they are not, fiddle with them a while.  
More events are available at higher levels.

Info Version 1.1 was written by Voltec (Voltec@cyberdude.com) for EotN.
~   

