/* This function was written by Rox of Farside, Permission to
 * use is granted provided this header is retained 
*/

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

/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN(       spec_assassin           );

bool spec_assassin( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
         int rnd_say;

    if ( ch->fighting != NULL )
                return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
               /* this should kill mobs as well as players */
                        if (victim->class != 2)  /* thieves */
                                break;
    }

    if ( victim == NULL || victim == ch || IS_IMM(victim) )
        return FALSE;
    if ( victim->level > ch->level + 7 || IS_NPC(victim))
        return FALSE;

   rnd_say = number_range (1, 10);
                
   if ( rnd_say <= 5)
                sprintf( buf, "Death to is the true end...");
   else if ( rnd_say <= 6)
                sprintf( buf, "Time to die....");
   else if ( rnd_say <= 7)
                sprintf( buf, "Cabrone...."); 
   else if ( rnd_say <= 8)
                sprintf( buf, "Welcome to your fate....");
   else if ( rnd_say <= 9)
   else if ( rnd_say <= 10)
                sprintf( buf, "Ever dance with the devil...."); 

    do_say( ch, buf );
    multi_hit( ch, victim, gsn_backstab );
    return TRUE;
}


