/*
 * "dump" command
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
#include "merc.h"
#include "tables.h"


void
do_dump(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *container, *into, *temp_obj, *temp_next;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  /* First, check to ensure they can dump.  */
  argument = one_argument( argument, arg1 );

  if (arg1[0] == '\0')
    {
      send_to_char("What do you want to dump out?\n\r",ch);
      return;
    }

  if ((container = get_obj_char(ch, arg1)) == NULL)
    {
      act("You don't have a $T.", ch, NULL, arg1, TO_CHAR);
      return;
    }

  if (container->item_type != ITEM_CONTAINER)
    {
      send_to_char("You can't dump that.\n\r",ch);
      return;
    }

  if (IS_SET(container->value[1], CONT_CLOSED))
    {
      act("$d is closed.", ch, NULL, container->short_descr, TO_CHAR);
      return;
    }

  /* Next, check to see if they want to dump into another container.  */
  argument = one_argument( argument, arg2 );

  if (!str_cmp(arg2,"in") || !str_cmp(arg2,"into") || !str_cmp(arg2,"on"))
    argument = one_argument(argument,arg2);

  if (arg2[0] != '\0')
    {
      /* Prefer obj in inventory other than object in room.  */
      if ((into = get_obj_char(ch, arg2)) == NULL)
        {
          if ((into = get_obj_here(ch, NULL, arg2)) == NULL)
            {
              send_to_char("You don't see that here.\n\r",ch);
              return;
            }
        }

      if (into->item_type != ITEM_CONTAINER)
        {
          send_to_char("You can't dump into that.\n\r",ch);
          return;
        }

      if (IS_SET(into->value[1], CONT_CLOSED))
        {
          act("$d is closed.", ch, NULL, into->short_descr, TO_CHAR);
          return;
        }

      act("You dump out the contents of $p into $P.",ch,container,into,TO_CHAR);
      act("$n dumps out the contents of $p into $P.",ch,container,into,TO_ROOM);

      for (temp_obj=container->contains; temp_obj != NULL; temp_obj=temp_next)
        {
          temp_next = temp_obj->next_content;

          if ((get_obj_weight(temp_obj) + get_true_weight(into)
                > (into->value[0] * 10))
              ||  (get_obj_weight(temp_obj) > (into->value[3] * 10)))
            act("$P won't fit into $p.",ch,into,temp_obj,TO_CHAR);

          else 
            {
              obj_from_obj(temp_obj);
              obj_to_obj(temp_obj, into);
            }
        }
    }

  /* Dumping to the floor.  */
  else
    {
      act("You dump out the contents of $p.",ch,container,NULL,TO_CHAR);
      act("$n dumps out the contents of $p.",ch,container,NULL,TO_ROOM);

      for (temp_obj=container->contains; temp_obj != NULL; temp_obj=temp_next)
        {
          temp_next = temp_obj->next_content;
          act("  ... $p falls out onto the ground.",ch,temp_obj,NULL,TO_CHAR);
          act("  ... $p falls out onto the ground.",ch,temp_obj,NULL,TO_ROOM);
  
          obj_from_obj(temp_obj);
          obj_to_room(temp_obj,ch->in_room);
  
          if (IS_OBJ_STAT(temp_obj,ITEM_MELT_DROP))
            {
              act("$p dissolves into smoke.",ch,temp_obj,NULL,TO_ROOM);
              act("$p dissolves into smoke.",ch,temp_obj,NULL,TO_CHAR);
              extract_obj(temp_obj);
            }
        }
    }
}
