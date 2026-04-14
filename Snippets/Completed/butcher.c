void do_butcher(CHAR_DATA *ch, char *argument)
{
	 /* Butcher skill, created by Argawal                   */
	 /* Original Idea taken fom Carrion Fields Mud          */
	 /* If you have an interest in this skill, feel free    */
	 /* to use it in your mud if you so desire.             */
	 /* All I ask is that Argawal is credited with creating */
	 /* this skill, as I wrote it from scratch.             */

	 char buf[MAX_STRING_LENGTH];
	 char arg[MAX_STRING_LENGTH];
	 int numst = 0;
	 OBJ_DATA *steak;
	 OBJ_DATA *obj;

	 one_argument(argument, arg);

	 if(get_skill(ch,gsn_butcher)==0)
	 {
		 send_to_char("{wButchering is beyond your skills.{x\n\r",ch);
		 return;
	 }

	 if(arg[0]=='\0')
	 {
		 send_to_char("{wButcher what?{x\n\r",ch);
		 return;
	 }

	 obj = get_obj_list( ch, arg, ch->in_room->contents );

	 if ( obj == NULL )
	 {
		  send_to_char( "{wIt's not here.{x\n\r", ch );
		  return;
	 }

	 if( (obj->item_type != ITEM_CORPSE_NPC)
		  && (obj->item_type!=ITEM_CORPSE_PC) )
	 {
		  send_to_char( "{wYou can only butcher corpses.{x\n\r", ch );
		  return;
	 }

	 /* create and rename the steak */
	 buf[0]='\0';
	 strcat(buf,"A steak of ");
	 strcat(buf,str_dup(obj->short_descr));
	 strcat(buf," is here.");
	 steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
	 steak->description=str_dup(buf);
	 steak->value[0] = ch->level / 2;
	 steak->value[1] = ch->level;
	 buf[0]='\0';
	 strcat(buf,"A steak of ");
	 strcat(buf,str_dup(obj->short_descr));
	 steak->short_descr=str_dup(buf);

	 /* Check the skill roll, and put a random ammount of steaks here. */

	 if(number_percent( ) < get_skill(ch,gsn_butcher))
	 {
		 numst = dice(1,4);
		 switch(numst)
		 {
		 case 1:

			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			act( "{w$n butchers a corpse and creates a steak.{x\n\r", ch, steak, NULL, TO_ROOM );
			act( "{wYou butcher a corpse and create a steak.{x\n\r",  ch, steak, NULL, TO_CHAR );
			break;

		 case 2:

			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			act( "{w$n butchers a corpse and creates two steaks.{x\n\r", ch, steak, NULL, TO_ROOM );
			act( "{wYou butcher a corpse and create two steaks.{x\n\r",  ch, steak, NULL, TO_CHAR );
			break;

		 case 3:

			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			act( "{w$n butchers a corpse and creates three steaks.{x\n\r", ch, steak, NULL, TO_ROOM );
			act( "{wYou butcher a corpse and create three steaks.{x\n\r",  ch, steak, NULL, TO_CHAR );
			break;

		 case 4:

			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			act( "{w$n butchers a corpse and creates four steaks.{x\n\r", ch, steak, NULL, TO_ROOM );
			act( "{wYou butcher a corpse and create four steaks.{x\n\r",  ch, steak, NULL, TO_CHAR );
			break;

		}

		check_improve(ch,gsn_butcher,TRUE,1);

	 }
	 else
	 {
		 act( "{w$n fails to butcher a corpse, and destroys it.{x\n\r", ch, steak, NULL, TO_ROOM );
		 act( "{wYou fail to butcher a corpse, and destroy it.{\n\r",   ch, steak, NULL, TO_CHAR );
		 check_improve(ch,gsn_butcher,FALSE,1);
	 }

	 /* dump items caried */
	 /* Taken from the original ROM code and added into here. */
	 if ( obj->item_type == ITEM_CORPSE_PC )
	 {   /* save the contents */
		 {
				OBJ_DATA *t_obj, *next_obj;
				for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
				{
					 next_obj = t_obj->next_content;
					 obj_from_obj(t_obj);
					 if (obj->in_obj) /* in another object */
						  obj_to_obj(t_obj,obj->in_obj);
					 else if (obj->carried_by) /* carried */
						  if (obj->wear_loc == WEAR_FLOAT)
								if (obj->carried_by->in_room == NULL)
									 extract_obj(t_obj);
								else
									 obj_to_room(t_obj,obj->carried_by->in_room);
						  else
								obj_to_char(t_obj,obj->carried_by);
					else if (obj->in_room == NULL) /* destroy it */
						  extract_obj(t_obj);
					 else /* to a room */
						  obj_to_room(t_obj,obj->in_room);
			  }
		}
  }
	 if ( obj->item_type == ITEM_CORPSE_NPC )
	 {
		 {
				OBJ_DATA *t_obj, *next_obj;
				for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
				{
					 next_obj = t_obj->next_content;
					 obj_from_obj(t_obj);
					 if (obj->in_obj) /* in another object */
						  obj_to_obj(t_obj,obj->in_obj);
					 else if (obj->carried_by) /* carried */
						  if (obj->wear_loc == WEAR_FLOAT)
								if (obj->carried_by->in_room == NULL)
									 extract_obj(t_obj);
								else
									 obj_to_room(t_obj,obj->carried_by->in_room);
						  else
								obj_to_char(t_obj,obj->carried_by);
					 else if (obj->in_room == NULL) /* destroy it */
						  extract_obj(t_obj);
					 else /* to a room */
						  obj_to_room(t_obj,obj->in_room);
			}
	  }
  }
	 /* Now remove the corpse */
	 extract_obj(obj);
	 return;
}
