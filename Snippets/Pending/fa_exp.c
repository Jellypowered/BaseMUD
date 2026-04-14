
/***************************************************************************
*       FALLEN ANGELS mud is protected by french law of intelectual        *
*                            property. we share freely that code           *
*                            provided this message isn't removed from the  *
*                            files and you respect the name of all the     *
*                            coders,area builders, and all the Diku,Merc,  *
*                            Rom licences.                                 *
*                                                                          *
*                   Thank to : ROM consortium .                            *
*                   Big thank to : Gary Gygax !!!!!!!!!!!!!!               *
*                                                                          *
*       Fallen Angel project by : Loran      ( laurent zilber  )           *
*                                 Silfen or                                *
*                                 Gwendoline ( jerome despret  )           *
*                                                                          *
*       despret@ecoledoc.lip6.fr ...                                       *
***************************************************************************/

/***************************************************************************
*                                                                          *
*  To use this snipet you must set the following line in the "check" help  *
*                                                                          *
*    Coded for Fallen Angels by : Zilber laurent,Despret jerome.           *
*                                                                          *
*  And send a mail to say you use it ( feel free to comment ) at :         *
*                                                                          *
*  [despret@ecoledoc.lip6.fr] or/and at [lzilber@hotmail.com]              *
****************************************************************************/

/***************************************************************************
*                                                                          *
* If you want to put this snipet on your web site you are allowed to but   *
*  the file must remain unchanged and you have to send us a mail at :      *
*                                                                          *
*  [despret@ecoledoc.lip6.fr] or/and at [lzilber@hotmail.com]              *
*  with the site URL.                                                      *
*                                                                          *
***************************************************************************/


/* -------  INSTALATION PROCEDURE ---------------------------------

   Edit "fight.c" file, replace your old xp_compute function by this
   one.

   Ok all is done now ... it should work... please if you have any problem
   mail us.
---------------------------------------------------------------------*/


/***************************************************************************

 This new xp_compute function try to give xp from mob considering their
 real strengh, and not only the level... It's obvious that a standart
 level X mob is harder to kill with "sanctuary" than without. 
 In standart code both mobs would worth the same xps, this code try
 to fix that.

 The mob have                            xp bonus given by this code
 ------------                            ---------------------------
 Sanctuary                                        +30%
 Haste                                            +20%
 Area attack                                      +20%
 Backstab                                         +20%
 Fast                                             +20%
 Dodge                                            +10%
 Parry                                            +10%

 Spec breath                                      +25%
 Spec cast spell                                  +20%
 Spec poison                                      +10%
 

****************************************************************************/


/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 *
 *  Fallen Angels version.
 */

int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
{
    int xp,base_exp;
    int align,level_range;
    int change;
    int time_per_level;

    level_range = victim->level - gch->level;
 
    /* compute the base exp */
    switch (level_range)
    {
 	default : 	base_exp =   0;		break;
	case -9 :	base_exp =   1;		break;
	case -8 :	base_exp =   2;		break;
	case -7 :	base_exp =   5;		break;
	case -6 : 	base_exp =   9;		break;
	case -5 :	base_exp =  11;		break;
	case -4 :	base_exp =  22;		break;
	case -3 :	base_exp =  33;		break;
	case -2 :	base_exp =  50;		break;
	case -1 :	base_exp =  66;		break;
	case  0 :	base_exp =  83;		break;
	case  1 :	base_exp =  99;		break;
	case  2 :	base_exp = 121;		break;
	case  3 :	base_exp = 143;		break;
	case  4 :	base_exp = 165;		break;
    } 
    
    if (level_range > 4)
	base_exp = 160 + 20 * (level_range - 4);

    /* jerome : base xp addition ---------------------- */

      if(IS_NPC(victim) )/* at max a mob with all worth 5 level above his own*/
       {
         if( is_perm_affected( victim, skill_lookup("sanctuary") ) )
            base_exp = (base_exp * 130) / 100;
         if( is_perm_affected( victim, skill_lookup("haste") ) )
            base_exp = (base_exp * 120) / 100;
         if( IS_SET(victim->off_flags,OFF_AREA_ATTACK) ) 
            base_exp = (base_exp * 120) / 100;
         if( IS_SET(victim->off_flags,OFF_BACKSTAB) ) 
            base_exp = (base_exp * 120) / 100;
         if( IS_SET(victim->off_flags,OFF_FAST) )
            base_exp = (base_exp * 120) / 100;
         if( IS_SET(victim->off_flags,OFF_DODGE) )
            base_exp = (base_exp * 110) / 100;
         if( IS_SET(victim->off_flags,OFF_PARRY) )
            base_exp = (base_exp * 110) / 100;

         if( victim->spec_fun != 0 )
          {
           if(   !str_cmp(spec_name(victim->spec_fun),"spec_breath_any")
              || !str_cmp(spec_name(victim->spec_fun),"spec_breath_acid")
              || !str_cmp(spec_name(victim->spec_fun),"spec_breath_fire")
              || !str_cmp(spec_name(victim->spec_fun),"spec_breath_frost")
              || !str_cmp(spec_name(victim->spec_fun),"spec_breath_gas")
              || !str_cmp(spec_name(victim->spec_fun),"spec_breath_lightning")
             )
             base_exp = (base_exp * 125) / 100;
           
           else if(   !str_cmp(spec_name(victim->spec_fun),"spec_cast_cleric")
                   || !str_cmp(spec_name(victim->spec_fun),"spec_cast_mage")
                   || !str_cmp(spec_name(victim->spec_fun),"spec_cast_undead")
                  ) 
                base_exp = (base_exp * 120) / 100;
         
           else if( !str_cmp(spec_name(victim->spec_fun),"spec_poison") )
                base_exp = (base_exp * 110) / 100;
          }
       }
    /*    back to normal code    -------------------- */

    /* do alignment computations */
   
    align = victim->alignment - gch->alignment;

    if (IS_SET(victim->act,ACT_NOALIGN))
    {
	/* no change */
    }

    else if (align > 500) /* monster is more good than slayer */
    {
	change = (align - 500) * base_exp / 500 * gch->level/total_levels; 
	change = UMAX(1,change);
        gch->alignment = UMAX(-1000,gch->alignment - change);
    }

    else if (align < -500) /* monster is more evil than slayer */
    {
	change =  ( -1 * align - 500) * base_exp/500 * gch->level/total_levels;
	change = UMAX(1,change);
	gch->alignment = UMIN(1000,gch->alignment + change);
    }

    else /* improve this someday */
    {
	change =  gch->alignment * base_exp/500 * gch->level/total_levels;  
	gch->alignment -= change;
    }
    
    /* calculate exp multiplier */
    if (IS_SET(victim->act,ACT_NOALIGN))
	xp = base_exp;

    else if (gch->alignment > 500)  /* for goodie two shoes */
    {
	if (victim->alignment < -750)
	    xp = (base_exp *4)/3;
   
 	else if (victim->alignment < -500)
	    xp = (base_exp * 5)/4;

        else if (victim->alignment > 750)
	    xp = base_exp / 4;

   	else if (victim->alignment > 500)
	    xp = base_exp / 2;

        else if (victim->alignment > 250)
	    xp = (base_exp * 3)/4; 

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -500) /* for baddies */
    {
	if (victim->alignment > 750)
	    xp = (base_exp * 5)/4;
	
  	else if (victim->alignment > 500)
	    xp = (base_exp * 11)/10; 

   	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < -500)
	    xp = (base_exp * 3)/4;

	else if (victim->alignment < -250)
	    xp = (base_exp * 9)/10;

	else
	    xp = base_exp;
    }

    else if (gch->alignment > 200)  /* a little good */
    {

	if (victim->alignment < -500)
	    xp = (base_exp * 6)/5;

 	else if (victim->alignment > 750)
	    xp = base_exp/2;

	else if (victim->alignment > 0)
	    xp = (base_exp * 3)/4; 
	
	else
	    xp = base_exp;
    }

    else if (gch->alignment < -200) /* a little bad */
    {
	if (victim->alignment > 500)
	    xp = (base_exp * 6)/5;
 
	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < 0)
	    xp = (base_exp * 3)/4;

	else
	    xp = base_exp;
    }

    else /* neutral */
    {

	if (victim->alignment > 500 || victim->alignment < -500)
	    xp = (base_exp * 4)/3;

	else if (victim->alignment < 200 && victim->alignment > -200)
	    xp = base_exp/2;

 	else
	    xp = base_exp;
    }

    /* more exp at the low levels */
    if (gch->level < 6)
    	xp = 10 * xp / (gch->level + 4);

    /* less at high */
    if (gch->level > 35 )
	xp =  15 * xp / (gch->level - 25 );

    /* reduce for playing time */
    
    {
	/* compute quarter-hours per level */
	time_per_level = 4 *
			 (gch->played + (int) (current_time - gch->logon))/3600
			 / gch->level;

	time_per_level = URANGE(2,time_per_level,12);
	if (gch->level < 15)
                          /* make it a curve */
	    time_per_level = UMAX(time_per_level,(15 - gch->level));
	xp = xp * time_per_level / 12;
    }
   
    /* randomize the rewards */
    xp = number_range (xp * 3/4, xp * 5/4);

    /* adjust for grouping */
    xp = xp * gch->level/( UMAX(1,total_levels -1) );

    return xp;
}


