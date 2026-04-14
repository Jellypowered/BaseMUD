/***************************************************************************  
 * New version of do_emote by Midboss.                                     *  
 *  ---------------------------------------------------------------------- *  
 * This code may be used freely within any non-commercial MUD, all I ask   *  
 * is that these comments remain in tact and that you give me any feedback *  
 * or bug reports you come up with.  Credit in a helpfile might be nice,   *  
 * too.                             -- Midboss (eclipsing.souls@gmail.com) *  
 ***************************************************************************/  
void do_emote (CHAR_DATA * ch, char * argument)  
{  
    char output[MSL];  
    int npos = 0, pos = 0;  
    bool fSay = FALSE, addname = FALSE;  
   
    if (argument[0] == '\0')  
    {  
        send_to_char ("Syntax: emote <text>\n\r", ch);  
        return;  
    }  
   
    if (strstr (argument, ch->name) == NULL)  
		addname = TRUE;  
   
    if (strstr (argument, "{") != NULL)  
    {  
        send_to_char ("Please do not use color codes in your emotes.\n\r", ch);  
        return;  
    }  
   
    while (*argument != '\0')  
    {  
        if (npos > (MSL - 4))  
            break;  
   
        switch(*argument)  
        {  
            default:  
                output[npos++] = *argument++;  
                break;  
            case '@':  
                for (pos = 0; ch->name[pos] != '\0'; pos++)  
                    output[npos++] = ch->name[pos];  
                argument++;  
                break;  
            case '\"':  
                if (!fSay)  
                {  
                    fSay = TRUE;  
                    output[npos++] = '\"';  
                    output[npos++] = '{';  
                    output[npos++] = 'G';  
					argument++;  
                    break;  
                }  
                else  
                {  
                    fSay = FALSE;  
                    output[npos++] = '{';  
                    output[npos++] = 'g';  
                    output[npos++] = '\"';  
					argument++;  
                    break;  
                }  
                break;  
        }  
    }  
    output[npos] = '\0';  
   
	//Simply remove the MOBtrigger lines if your MUD doesn't have progs.  
    MOBtrigger = FALSE;  
	if (addname)  
	{  
		act ("{g$n $T {D({x$n{D){x", ch, NULL, output, TO_CHAR);  
		act ("{g$n $T {D({x$n{D){x", ch, NULL, output, TO_ROOM);  
	}  
	else  
	{  
		act ("{g$T {D({x$n{D){x", ch, NULL, output, TO_CHAR);  
		act ("{g$T {D({x$n{D){x", ch, NULL, output, TO_ROOM);  
	}  
    MOBtrigger = TRUE;  
}  