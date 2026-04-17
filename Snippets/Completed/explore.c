/*
To get this thing working, add the following on top of handler.c somewhere
under local procedures remark:
   
    void 	set_bit		args( ( char *explored, int index) );

in handler.c in char_to_room just below pRoomIndex->people = ch; add this:
 
if (!IS_NPC(ch)) set_bit(ch->pcdata->explored,pRoomIndex->vnum);

then in save.c add this 2 lines somewhere on the top of the file:
    
    void	fwrite_rle	args((char *explored, FILE *fp));
    void	fread_rle	args((char *explored, FILE *fp));

in load_char_obj add this 2 lines below all the structure initializations:
    
    for (index = 0; index < 32768; index++)
    	ch->pcdata->explored[index] = '\0';


in fwrite_char add this line anywhere before fprintf( fp, "End\n\n" );

    fwrite_rle(ch->pcdata->explored, fp);

in fread_char in "case 'R':" add this 6 lines

	    if ( !str_cmp( word, "RoomRLE" ) ) 
	    {
		fread_rle(ch->pcdata->explored, fp);
	        fMatch = TRUE;
		break;
	    }

and finally in merc.h in pcdata structure add this line:

    char                explored[4096];

you can add something like this in your do_score, in act_info.c to display
the current status:

    if (!IS_NPC(ch))
    {
	int    rcnt    = roomcount(ch);
	double rooms   = top_room,
	       percent = (double)rcnt / (rooms / 100);
	
	printf_to_char( ch, "You explored {Y%d{x from {Y%d{x places, that is {Y%.2f%%{x of the world.\n\r",rcnt,top_room,percent);
    }

don't forget to add explore.o in your O_FILES in Makefile :) do clean compile
and you should be set. Thanks goes to Edwin and guys on irc.acestar.org/#Rom 
channel for the ideas and hammering out the details. 

If you like it, send a postcard to my dog, and if you don't like it, send him
one anyway it will make him happy :)
*/

#include <stdio.h>
#include <stdlib.h>
#include "merc.h"

/*
 * bitcount is used in roomcount procedure, everything what it does is that it
 * returns amount of bits set in the 8bit variable
 */
int bitcount(char c)
{
    int count = 0;

    if (c & A) count++;
    if (c & B) count++;
    if (c & C) count++;
    if (c & D) count++;
    if (c & E) count++;
    if (c & F) count++;
    if (c & G) count++;
    if (c & H) count++;
    
    return count;
}

/*
 * roomcount is called from do_score in act_info.c, it returns number of
 * explored rooms
 */
int roomcount(CHAR_DATA * ch)
{
    int index = 0;
    int count = 0;

    if (IS_NPC(ch)) return top_room;

    for ( index = 0; index < 4096; index++)
	count += bitcount(ch->pcdata->explored[index]);

    return count;
}

/*
 * getbit returns 1 if the specific bit is set, 0 otherwise, it is used by
 * fwrite_rle procedure.
 */
int getbit(char *explored, int index)
{
    int bit = 0;

    switch (index % 8)
    {
	case 0:	 bit = A; break;
	case 1:	 bit = B; break;
        case 2:	 bit = C; break;
        case 3:	 bit = D; break;
        case 4:	 bit = E; break;
        case 5:	 bit = F; break;
        case 6:	 bit = G; break;
        case 7:	 bit = H; break;
        default: bit = 0; break;
    }

    if (IS_SET(explored[index/8], bit)) return 1;
    else return 0;
    
}

/*
 * setbit sets the specific bit pointed on by the index [vnum] of the room,
 * there is no need for remove bit at this time. setbit is used in char_to_room
 * and also in fread_rle to restore the array from pfile.
 */
void setbit(char *explored, int index)
{
    int bit = 0;

    switch (index % 8)
     {
	case 0:	 bit = A; break;
	case 1:	 bit = B; break;
        case 2:	 bit = C; break;
        case 3:	 bit = D; break;
	case 4:	 bit = E; break;
        case 5:	 bit = F; break;
	case 6:	 bit = G; break;
        case 7:	 bit = H; break;
	default: bit = 0; break;
    }

    SET_BIT(explored[index/8], bit);
    return;
}

/* fwrite_rle saves the status of the array in already open file it doesn't 
 * check for NPCs or anything, however it uses simple RLE-like bit compression
 * to save some space on the drive. this procedure is called from fwrite_char
 */
void fwrite_rle(char *explored, FILE *fp)
{
    int index;
    int bit = getbit(explored,0);
    int count = 0;

    fprintf(fp,"RoomRLE  ");

    for (index = 0; index < 32768; index++)
    {
	if (getbit(explored,index) == bit) count++;
	else
	{
	    fprintf(fp," %d %d",count,bit);
	    count = 1;
	    bit = getbit(explored,index);
	}
    }
    fprintf(fp," %d %d -1\n",count,bit);
    return;
}

/* fread_rle reads the data back in the bit array, it automatically
 * decompresses them to their original state, the bit compression was hammered
 * out on #rom channel with Peter Vidler :)
 */
void fread_rle(char *explored, FILE *fp)
{
    int index;
    int bit   = 0;
    int count = 0;
    int pos   = 0;

    index = 0;

    for (;;)
    {
	count = fread_number(fp);
	if (count < 0) break;

	bit = fread_number(fp);

	do 
	{
	    if (bit == 1) setbit(explored,index);
	    index++;
	} while (index < pos+count);
	pos = index;
    }
    return;
}

/* 
i know the code is nothing much but it works and there is no crashes,
if you improve it, please send me the changes to dingo@pdragon.inetsolve.com
so i can stick them in my own mud ;) There is no credits or anything, just
have some fun for change ;)
*/
