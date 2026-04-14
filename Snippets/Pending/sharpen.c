The sharpen skill was written for warriors so they had a skill that
other classes could hound on them for their services as well :) If you
have trouble installing this skill or have comments email me at
froboz@cyberdude.com.

-----------------------------------------------------------------------------
To begin this skill requires that a stone be created in an area file.
I like to define objects as such in the limbo.are but it can be placed in any
area file.  It should look something like this:

#38
sharpening stone~
a sharpening stone~
A sharpening stone has been dropped here.~
~
41 0 16385
0 0 0 0
3 650 65
---------------------------------------------------------------------------

Now to install this skill:

---------------------------------------------------------------------------
mud.h

under:
/*
 * Well known object virtual numbers.  
 * Defined in #OBJECTS.
 */
add:
#define OBJ_VNUM_STONE               38 /*Stone vnum for sharpen*/

under:
/*
 * These are skill_lookup return values for common skills and spells.
 */
add:
extern  sh_int  gsn_sharpen;

under:
/*
 * Command functions.
 * Defined in act_*.c (mostly).   
 */
add:

DECLARE_DO_FUN( do_sharpen      );

under:
/*
 * Extra flags.
 * Used in #OBJECTS.
 */
add:
#define ITEM_SHARP              BV31 /* open bv */
----------------------------------------------------------------------------
act_info.c

under:
char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
add:
    if ( IS_OBJ_STAT(obj, ITEM_SHARP)     )   strcat( buf, "(Sharp) "     );
-----------------------------------------------------------------------------
db.c

under:
/* weaponry */
add:
sh_int                  gsn_sharpen;

under:
    /*
     * Assign gsn's for skills which need them.
     */
add:
        ASSIGN_GSN( gsn_sharpen,        "sharpen" );

-----------------------------------------------------------------------------
handler.c

under:
char *extra_bit_name( int extra_flags )
add:
    if ( extra_flags & ITEM_SHARP        ) strcat( buf, " sharpened"    );

-----------------------------------------------------------------------------
skills.c

/* Sharpen skill by Froboz questions comments send to froboz@cyberdude.com */
void do_sharpen( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  OBJ_DATA *sobj;
  AFFECT_DATA *paf;
  char      arg [MAX_INPUT_LENGTH ];
  int       percent;

  if (! IS_NPC( ch )
  && ch->level < skill_table [gsn_sharpen]->skill_level [ch->class] )
    {
         send_to_char( "What do you think you are, a warrior?\n\r", ch );
         return;
    }

 one_argument ( argument, arg );

    if (arg[0] == '\0' )
    {
        send_to_char( "What are you trying to sharpen?\n\r", ch );
        return;
    }

    if ( ch->fighting )
    {
        send_to_char( "While you are fighting? Yea right!.\n\r", ch );
        return;
    }

    if ( ms_find_obj (ch) )
        return;

    if ( !( obj = get_obj_carry( ch, arg ) ) )
    {
        send_to_char( "You do not have that weapon.\n\r", ch );
        return;
    }

    if ( obj->item_type != ITEM_WEAPON )
    {
        send_to_char( "That item is not a weapon.\n\r", ch );
        return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_SHARP ) )
    {
        send_to_char( "That weapon is already sharpened.\n\r", ch );
        return;
    }

    /* Ok we have a sharpenable weapon but do we have the stone */ 
    for ( sobj = ch->first_carrying; sobj; sobj = sobj->next_content )
    {
        if ( sobj->pIndexData->vnum == OBJ_VNUM_STONE )
        break;
    }
    if ( !sobj )
    {
        send_to_char( "You do not have a sharpening stone.\n\r", ch );
        return;
    }

   if ( !IS_NPC( ch )
   && percent > ch->pcdata->learned[gsn_sharpen] )
   {
       set_char_color( AT_RED, ch );
       send_to_char( "You failed and slice your finger. Ouch!\n\r", ch );
       set_char_color( AT_GREY, ch );
       damage( ch, ch, ch->level, gsn_sharpen );
       act(AT_RED, "$n slices $s finger!", ch, NULL, NULL, TO_ROOM );
       learn_from_failure( ch, gsn_sharpen );
       return;
   }

separate_obj(obj);
       CREATE( paf, AFFECT_DATA, 1 );
       paf->type           = -1;
       paf->duration       = 0;
       paf->location       = APPLY_DAMROLL;
       paf->modifier       = number_fuzzy(6);
       paf->bitvector      = 0;
       LINK( paf, obj->first_affect, obj->last_affect, next, prev );

      act(AT_GREEN, "You sharpen $p.\n\r", ch, obj, NULL, TO_CHAR );
      act(AT_GREEN, "$n pulls out a piece of stone and begins sharpening $p.", ch, obj, NULL, TO_ROOM );  
      SET_BIT( obj->extra_flags, ITEM_SHARP );

      /* 
       *Sharp weapon control, if you don't mind surplus sharp weapons
       *just comment out the next line.
       */     
      obj->timer = 10 + ch->level;
      learn_from_success( ch, gsn_sharpen );
      return;
}
--------------------------------------------------------------------------------
Don't forget to define the command in tables.c, make clean recompile and
all that good stuff.

also command.dat should be something like this:
#SKILL
Name         sharpen~
Type         Skill
Flags        0
Target       4
Minpos       8
Rounds       12
Code         do_sharpen
Dammsg       ~  
Wearoff      !Sharpen!~
Minlevel     12
END





 