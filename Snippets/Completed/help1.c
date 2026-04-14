/* This snippet is designed with ROM 2.4b6, but is glaringly simple and
could easily be tailored to other code bases.  It essentially tracks what
helps players are requesting that you don't currently have defined.  Very
customer service oriented, in the first three days we had it implemented
on Yrth, we've added approximately 20 new help files based on the data
collected in help.txt.

This was written by Absalom and Kiyo of Yrth.  Either can be contacted at
yrth@roscoe.mudservices.com. */

/* In act_info.c, as part of void do_help, define: */

    char nohelp[MAX_STRING_LENGTH];

/* Then, at the beginning of the function, add: */

    strcpy(nohelp, argument);

/* Change if(!found) to: */

    if(!found)
        {
    	send_to_char("No help on that word.\n\r", ch);
        append_file( ch, HELP_FILE, nohelp );
        }

/* In merc.h, define: */

#define HELP_FILE       "help.txt"   /* For undefined helps */

/* Recompile and reboot.  Start tracking help.txt in Rom24/area and
adjusting help.are appropriately. */
