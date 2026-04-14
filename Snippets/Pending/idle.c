/* This meager bit of code is, oddly enough, copyrighted by 
 * Ferric of MelmothMUD.  Feel free to use it to your hearts content;
 * it's something you could have come up on your own easily enough.
 * If you do use this, or the idea, drop me a line at ferric@uwyo.edu,
 * just so I know my contributions (however small :) are worth something!
 * Enhanced by Dennis Reichel
 */
void do_idle(CHAR_DATA *ch, char *argument)
{

    CHAR_DATA *vch;
    char	buf[MAX_INPUT_LENGTH];
    char	status[MAX_INPUT_LENGTH];

    send_to_char("Players     Idle  Hours  HpL Position  Status  Host\n\r-----------------------------------------------------------\n\r",ch);
    for(vch=char_list; vch != NULL; vch = vch->next)
    {         
        if ( IS_NPC( vch ) || !can_see( ch, vch ) )
	    continue;
	 
	if ( vch->desc && vch->desc->editor )
	    sprintf( status, olc_ed_name( vch ) ); 
	else    
	    sprintf( status, vch->countdown > 0 ? "Quest" :"" );
	
        sprintf(buf,"%-12s%4d%7d%5.1f %-10s%-8s%-30.30s\n\r",vch->name, vch->timer,
            ( vch->played + (int) (current_time - vch->logon) ) / 3600, 
            (float)( vch->played + (int) (current_time - vch->logon) )/(3600*vch->level), 
                    position_flags[vch->position].name, status,
                    vch->desc ? vch->desc->host : "No descriptor." );
        send_to_char(buf,ch);
    }
    if (number_percent() == 1 ) 
       send_to_char( "You have become better at idleness!\n\r", ch );
    send_to_char( "\n\r", ch );
}

