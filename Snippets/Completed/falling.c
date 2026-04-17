/*
   Slash's Merc Snippet
	
   Falling objects

   Ok, what this does it causes objects in air sectors to fall.  This is
a real nice touch if you have lots of airy zones, like my Aarakocran city,
or the air above Midgaard.

   Everything falls.  This is pretty funny if you are standing in the
square in Midgaard below the air and corpses of flying monsters start
falling down.  It is also pretty funny if you have flying mobiles in air
zones who have treasure and as players loot the corpse the corpse is falling.

   Put this code in UPDATE.C in the function obj_update() where you see

        if ( obj->timer <= 0 || --obj->timer > 0 )
            continue;

   Put it before the continue and put braces around the whole thing and
the continue.

            if (obj->in_room &&
                obj->in_room->sector_type == SECT_AIR &&
                (obj->wear_flags & ITEM_TAKE) &&
                obj->in_room->exit[5] &&
                obj->in_room->exit[5]->to_room)
               {
               ROOM_INDEX_DATA *new_room = obj->in_room->exit[5]->to_room;

               if (( rch = obj->in_room->people ) != NULL )
                  {
                  act( "$p falls away.", rch, obj, NULL, TO_ROOM );
                  act( "$p falls away.", rch, obj, NULL, TO_CHAR );
                  }

               obj_from_room(obj);
               obj_to_room(obj, new_room);

               if (( rch = obj->in_room->people ) != NULL )
                  {
                  act( "$p floats by.", rch, obj, NULL, TO_ROOM );
                  act( "$p floats by.", rch, obj, NULL, TO_CHAR );
                  }
               }
*/

/*
 =============================================================================
/   ______ _______ ____   _____   ___ __    _ ______    ____  ____   _____   /
\  |  ____|__   __|  _ \ / ____\ / _ \| \  / |  ____|  / __ \|  _ \ / ____\  \
/  | |__     | |  | |_| | |     | |_| | |\/| | |___   | |  | | |_| | |       /
/  | ___|    | |  | ___/| |   __|  _  | |  | | ____|  | |  | |  __/| |   ___ \
\  | |       | |  | |   | |___| | | | | |  | | |____  | |__| | |\ \| |___| | /
/  |_|       |_|  |_|  o \_____/|_| |_|_|  |_|______|o \____/|_| \_|\_____/  \
\                                                                            /
 ============================================================================

------------------------------------------------------------------------------
ftp://ftp.game.org/pub/mud      FTP.GAME.ORG      http://www.game.org/ftpsite/
------------------------------------------------------------------------------

  This file came from FTP.GAME.ORG, the ultimate source for MUD resources.

------------------------------------------------------------------------------

*/
