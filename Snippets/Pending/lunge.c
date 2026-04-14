void do_lunge( CHAR_DATA *ch, char *argument)
{
        CHAR_DATA *victim;
        int chance, dam;
        char arg[MAX_INPUT_LENGTH];
        OBJ_DATA *wield;
        int multiplier;
	
	dam = 0;

        one_argument(argument,arg);

        if ((chance = get_skill(ch,gsn_lunge) ) == 0
        || (ch->level < skill_table[gsn_lunge].skill_level[ch->class]) )
        {
        send_to_char("Lunging? What's that?\n\r",ch);
        return;
        }
        if (arg[0] == '\0')
        {
        victim = ch->fighting;
        if (victim == NULL)
                {
                send_to_char("But you aren't fighting anyone!\n\r",ch);
                return;
                }
        }
        else if ( (victim = get_char_room(ch,arg)) == NULL )
        {
                send_to_char("They aren't here.\n\r",ch);
                return;
        }
        if (victim == ch)
        {
        send_to_char("You can't lunge at yourself!\n\r",ch);
        return;
        }

        wield = get_eq_char(ch,WEAR_WIELD);
        if ((wield == NULL)
        || (( wield->value[0] != WEAPON_SWORD)
        && ( wield->value[0] != WEAPON_SPEAR)
        && ( wield->value[0] != WEAPON_POLEARM) ))
        {
        send_to_char("You must be wielding a sword, spear or polearm to lunge.\n\r",ch);
        return;
        }

        if (is_safe(ch,victim) )
        return;

        chance += ch->carry_weight/25;
        chance -= victim->carry_weight/20;
        chance += (ch->size - victim->size)*20;
        chance -= get_curr_stat(victim,STAT_DEX);
        chance += get_curr_stat(ch,STAT_STR)/3;
        chance += get_curr_stat(ch,STAT_DEX)/2;
        if (IS_AFFECTED(ch,AFF_HASTE) )
                chance += 10;
        if (IS_AFFECTED(victim,AFF_HASTE) )
                chance = 20;
        chance += (ch->drain_level + ch->level - victim->level - victim->drain_level);

        act("$n attempts to impale $N with a quick lunge!",ch,0,victim,TO_NOTVICT);
        act("You attempt to impale $N with a quick lunge!",ch,0,victim,TO_CHAR);
        act("$n attempts to impale you with a quick lunge!",ch,0,victim,TO_VICT);

        if (number_percent() < chance)
        {
        check_improve(ch,gsn_lunge,TRUE,1);
        WAIT_STATE(ch,skill_table[gsn_lunge].beats);
        if (wield->pIndexData->new_format)
                dam = dice(wield->value[1],wield->value[2]);
        else
                dam = number_range(wield->value[1],wield->value[2]);

        if (get_skill(ch,gsn_enhanced_damage) > 0 )
        {
        if (number_percent() <= get_skill(ch,gsn_enhanced_damage) )
                {
                check_improve(ch,gsn_enhanced_damage,TRUE,1);
                dam += dam*(number_range(50,100)/100) * ch->pcdata->learned[gsn_enhanced_damage]/100;
                }
        }

        dam += GET_DAMROLL(ch);
        dam *= ch->pcdata->learned[gsn_lunge];
	dam /= 100;
        multiplier = number_range((ch->drain_level + ch->level)/8,(ch->drain_level + ch->level)/4);
        multiplier /= 10;
	multiplier += 5/4;
        dam *= multiplier;
        
        if (dam <= 0)
                dam = 1;
        damage(ch,victim,dam,gsn_lunge,DAM_PIERCE,TRUE);
        }
        else
        {
        damage(ch,victim,dam,gsn_lunge,DAM_PIERCE,TRUE);
        check_improve(ch,gsn_lunge,FALSE,1);
        WAIT_STATE(ch,skill_table[gsn_lunge].beats);
        }
        return;
}
