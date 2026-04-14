/* Original Code by Jason Huang (god@sure.net).                       */
/* Permission to use this code is granted provided this header is     */
/* retained and unaltered.                                            */ 

void spell_fear( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;

    if (       victim == ch
	|| !victim->in_room
	|| IS_SET( victim->in_room->room_flags, ROOM_SAFE      )
	|| IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
	|| IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
	|| IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
	|| victim->level >= level 
	|| victim->in_room->area != ch->in_room->area
        |	|| ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    do_flee( victim, "" );   
    return;
}