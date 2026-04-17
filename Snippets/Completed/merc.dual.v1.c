/*

===========================================================================
This snippet was written by Erwin S. Andreasen, erwin@pip.dknet.dk. You may 
use this code freely, as long as you retain my name in all of the files. You
also have to mail me telling that you are using it. I am giving this,
hopefully useful, piece of source code to you for free, and all I require
from you is some feedback.

Please mail me if you find any bugs or have any new ideas or just comments.

All my snippets are publically available at:

http://pip.dknet.dk/~pip1773/

If you do not have WWW access, try ftp'ing to pip.dknet.dk and examine
the /pub/pip1773 directory.
===========================================================================

Note that function prototypes are not included in this code - remember to add
them to merc.h yourself - also add the 'second' command to interp.c.

Secondary weapon code prototype. You should probably do a clean compile on
this one, due to the change to the MAX_WEAR define.

Last update: Oct 10, 1995

Should work on : MERC2.2

Fixed since last update:

Know bugs and limitations yet to be fixed:

Comments:

*/


/* in the beginning of act_info.c, add a <secondary weapon> to the table: */
char *  const   where_name      [] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on body>      ",
    "<worn on head>      ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded>           ",
    "<held>              ",
    "<secondary weapon>  "  /* ADD THIS */

};


/* in act_obj.c, add this command */
/* also remember to add it to interp.c, declare it in merc.h etc. */

void do_second (CHAR_DATA *ch, char *argument)
/* wear object as a secondary weapon */
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH]; /* overkill, but what the heck */

    if (argument[0] == '\0') /* empty */
    {
        send_to_char ("Wear which weapon in your off-hand?\n\r",ch);
        return;
    }

    obj = get_obj_carry (ch, argument); /* find the obj withing ch's inventory */

    if (obj == NULL)
    {
        send_to_char ("You have no such thing in your backpack.\n\r",ch);
        return;
    }


    /* check if the char is using a shield or a held weapon */

    if ( (get_eq_char (ch,WEAR_SHIELD) != NULL) ||
         (get_eq_char (ch,WEAR_HOLD)   != NULL) )
    {
        send_to_char ("You cannot use a secondary weapon while using a shield or holding an item\n\r",ch);
        return;
    }


    if ( ch->level < obj->level )
    {
        sprintf( buf, "You must be level %d to use this object.\n\r",
            obj->level );
        send_to_char( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.",
            ch, obj, NULL, TO_ROOM );
        return;
    }

/* check that the character is using a first weapon at all */
    if (get_eq_char (ch, WEAR_WIELD) == NULL) /* oops - != here was a bit wrong :) */
    {
        send_to_char ("You need to wield a primary weapon, before using a secondary one!\n\r",ch);
        return;
    }


/* check for str - secondary weapons have to be lighter */
    if ( get_obj_weight( obj ) > ( str_app[get_curr_str(ch)].wield / 2) )
    {
        send_to_char( "This weapon is too heavy to be used as a secondary weapon by you.\n\r", ch );
        return;
    }

/* check if the secondary weapon is at least half as light as the primary weapon */
    if ( (get_obj_weight (obj)*2) > get_obj_weight(get_eq_char(ch,WEAR_WIELD)) )
    {
        send_to_char ("Your secondary weapon has to be considerably lighter than the primary one.\n\r",ch);
        return;
    }

/* at last - the char uses the weapon */

    if (!remove_obj(ch, WEAR_SECONDARY, TRUE)) /* remove the current weapon if any */
        return;                                /* remove obj tells about any no_remove */

/* char CAN use the item! that didn't take long at aaall */

    act ("$n wields $p in $s off-hand.",ch,obj,NULL,TO_ROOM);
    act ("You wield $p in your off-hand.",ch,obj,NULL,TO_CHAR);
    equip_char ( ch, obj, WEAR_SECONDARY);
    return;
}

/* in the wear_obj function, replace the shield and wield sections with this: */

/* shield section */
    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {

        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("You cannot use a shield while using 2 weapons.\n\r",ch);
            return;
        }

        if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
            return;
        act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
        act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_SHIELD );
        return;
    }


/* wield section */

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("You cannot hold an item while using 2 weapons.\n\r",ch);
            return;
        }

        if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
            return;
        act( "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
        act( "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_HOLD );
        return;
    }




/* in merc.h, extend the WEAR_ constants: */

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */

/* current #define bla bla bla */
#define WEAR_SECONDARY               18
#define MAX_WEAR                     19


/* and most important, in fight.c: */

/* change the prototype in the beginning of the file: */
void    one_hit     args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ) );


/* in multi_hit, add this right after the first hit is resolved */
    if (get_eq_char (ch, WEAR_SECONDARY))
    {
        one_hit( ch, victim, dt, TRUE );
        if ( ch->fighting != victim )
            return;
    }

/* also, add an extra parameter, FALSE, to all of the one_hit calls */


/* finally, change one_hit: */
void one_hit ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary )

/* .... */

    /*
     * Figure out the type of damage message.
     * if secondary == true, use the second weapon.
     */
    if (!secondary)
        wield = get_eq_char( ch, WEAR_WIELD );
    else
        wield = get_eq_char( ch, WEAR_SECONDARY );

