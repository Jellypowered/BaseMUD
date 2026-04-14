/*
 * Engage Combat code
 *
 *  Written by John Patrick (j.s.patrick@ieee.org)
 *   for Ansalon (ansalon.wolfpaw.net 8679)
 *
 * I'm very big in intellectual property rights.  In order to use
 * this code or any derivatives in your MUD, you must do the following:
 *
 *  - E-mail me at j.s.patrick@ieee.org
 *  - Have a helpfile for the item that includes a credit to me, with
 *    the above e-mail address in it.  (E.g. This code was written by
 *    John Patrick (j.s.patrick@ieee.org) and used with permission.)
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"

/* Extern calls.  */
extern void set_fighting(CHAR_DATA *ch, CHAR_DATA *victim);


/**************************************************************************/

/*
 * Engage skill, allows changing of targets in battle.
 * Increasing ability with Level.
 */

void do_engage(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  /* Check for skill.  */
  if (   (get_skill(ch,gsn_engage) == 0 )
      || (    !IS_NPC(ch)
          &&  (ch->level < skill_table[gsn_engage].skill_level[ch->class])))
    {
      send_to_char("Engage?  You're not even dating!\n\r",ch);  /* Humor. :)  */
      return;
    }

  /* Must be fighting.  */
  if (ch->fighting == NULL)
    {
      send_to_char("You're not fighting anyone.\n\r",ch);
      return;
    }

  one_argument( argument, arg );

  /* Check for argument.  */
  if (arg[0] == '\0')
    {
      send_to_char("Engage who?\n\r",ch);
      return;
    }

  /* Check for victim.  */
  if ((victim = get_char_room(ch,arg)) == NULL)
    {
      send_to_char("Shadowbox some other time.\n\r",ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char("Attacking yourself in combat isn't a smart thing.\n\r",ch);
      return;
    }

  if (ch->fighting == victim)
    {
      send_to_char("You're already pummelling that target as hard as you can!\n\r",ch);
      return;
    }

  /* Check for safe.  */
  if (is_safe(ch, victim))
    return;

  /* Check for charm.  */
  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
      act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
      return;
    }

  /* This lets higher-level characters engage someone that isn't already fighting them.
     Quite powerful.  Raise level as needed.  */
  if ((victim->fighting != ch) && (ch->level < LEVEL_YOU_WANT_TO_LET_THEM_DO_THIS))
    {
      send_to_char("But they're not fighting you!\n\r",ch);
      return;
    }

  /* Get chance of success, and allow max 95%.  */
  chance = get_skill(ch,gsn_engage);
  chance = UMIN(chance,95);

  if (number_percent() < chance)
    {
      /* It worked!  */
      stop_fighting(ch,FALSE);

      set_fighting(ch,victim);
      if (victim->fighting == NULL)
        set_fighting(victim,ch);

      check_improve(ch,gsn_engage,TRUE,3);
      act("$n has turned $s attacks toward you!",ch,NULL,victim,TO_VICT);
      act("You turn your attacks toward $N.",ch,NULL,victim,TO_CHAR);
      act("$n has turned $s attacks toward $N!",ch,NULL,victim,TO_NOTVICT);
    }
  else
    {
      /* It failed!  */
      send_to_char("You couldn't get your attack in.\n\r",ch);
      check_improve(ch,gsn_engage,FALSE,3);
    }
}
