/* -------------------------------------------
 * Critical Strike for ROM24, 1.00 - 3/28/98
 * -------------------------------------------
 * The critical strike is an automatic skill
 * that allows players with 100% skill in the
 * weapon they are using to get a little bonus
 * out of it once in a while.
 *
 * As with all else, please email me if you use
 * this code so I know my time is not going
 * to waste. :)
 *
 * Brian Babey (aka Varen)
 * [bribe@erols.com]
 * ------------------------------------------- */


/* -----------------------------------------------
 * In const.c, insert this into the skill table */

    {
        "critical strike",            { 53, 53, 40, 35 },     { 0, 0, 8, 8},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_critical,          SLOT( 0),       0,      0,
        "",                     "!Critical Strike!",   ""
    },


/* ----------------------------------------------
 * In merc.h, under Game Parameters, increment
 * the variable MAX_SKILL by 1. */


/* ----------------------------------------------
 * In merc.h, under the new gsns section, insert
 * this line */

extern sh_int  gsn_critical;


/* ----------------------------------------------
 * In db.c, under new gsns, insert this line: */

sh_int                  gsn_critical;


/* ----------------------------------------------
 * In fight.c, in the local function declarations,
 * insert this line. */

bool    check_critical  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );


/* ---------------------------------------------
 * In fight.c, insert this function wherever
 * it belongs alphabetically (or wherever you'd
 * like it to be - it really doesn't matter) */

bool check_critical(CHAR_DATA *ch, CHAR_DATA *victim)
{
        OBJ_DATA *obj;

        obj = get_eq_char(ch,WEAR_WIELD);

        if (
             ( get_eq_char(ch,WEAR_WIELD) == NULL ) || 
             ( get_skill(ch,gsn_critical)  <  1 ) ||
             ( get_weapon_skill(ch,get_weapon_sn(ch))  !=  100 ) ||
             ( number_range(0,100) > get_skill(ch,gsn_critical) )
           )
                return FALSE;


        if ( number_range(0,100) > 25 )
                return FALSE;


        /* Now, if it passed all the tests... */

        act("$p CRITICALLY STRIKES $n!",victim,obj,NULL,TO_NOTVICT);
        act("CRITICAL STRIKE!",ch,NULL,victim,TO_VICT);
        check_improve(ch,gsn_critical,TRUE,6);
        return TRUE;
}

/* -------------------------------------------
 * In fight.c, in the function one_hit, insert
 * this line into the bonus section, where
 * enhanced damage is factored in. */

    if ( check_critical(ch,victim) )
        dam *= 1.40;
