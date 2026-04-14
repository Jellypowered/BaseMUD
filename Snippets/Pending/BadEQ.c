/********************************************************************************
 * Fix Resets Code copyright 1999-2001                                          *
 * Markanth : markanth@hotpop.com or rjenn78@home.com                           *
 * Devil's Lament : spaceservices.net port 3778                                 *
 * Web Page : http://spaceservices.net/~markanth/                               *
 *                                                                              *
 * About 90% foolproof, only exception is when there is 2 wear locations        *
 * with the same wear flag.  (Which are done randomly here)                     *
 * This code pretty much requires OLC.                                          *
 *                                                                              *
 * All I ask in return is that you give me credit on your mud somewhere         *
 * or email me if you use it.                                                   *
 ********************************************************************************/


>>> DB2.C <<<

1) The main function. Put it at the bottom of the file.

void fix_resets(void)
{
     RESET_DATA *pReset;
     ROOM_INDEX_DATA *pRoom;
     OBJ_INDEX_DATA *pObj;
     int iHash;
     int rnum;
     
    /* Optional logging...
    unlink("resets.log"); */

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
        for (pRoom = room_index_hash[iHash]; pRoom != NULL; pRoom = pRoom->next)
        {
               rnum = 0;
                for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
                {
                      rnum++;
                      if (pReset->command == 'E') /* equip mob with object */
                      {
                           if ((pObj = get_obj_index(pReset->arg1)) != NULL)
                           {
                                 if (!CAN_WEAR(pObj, wear_bit(pReset->arg3)))  /* if object isn't wearable from the reset */
                                 {   
                                        /* Cycle through the wear flags and set the new reset value.
                                           Flags with 2 locations are done randomly (could be a problem)
                                           Modify to suit your needs =) */     


                                        sprintf(log_buf, "%s - %s %s (%d), Reset (%d), Room (%d)", 
                                        pRoom->area->name, pObj->short_descr, 
                                        wear_loc_strings[pReset->arg3], 
                                        pObj->vnum, rnum, pRoom->vnum);
                                        /* Optional logging to file...
										append_file(NULL, "resets.log", buf); */
                                        log_string(log_buf);
                                        if(CAN_WEAR(pObj, ITEM_WEAR_HEAD))
                                           pReset->arg3 = WEAR_HEAD;
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_NECK))
                                           pReset->arg3 = (number_range(1, 20) <= 10) ? WEAR_NECK_1 : WEAR_NEC$
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_BACK))
                                           pReset->arg3 = WEAR_BACK;
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_BODY))
                                           pReset->arg3 = WEAR_BODY;
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_ABOUT))
                                           pReset->arg3 = WEAR_ABOUT;
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_WAIST))
                                           pReset->arg3 = WEAR_WAIST;
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_ARMS))
                                           pReset->arg3 = WEAR_ARMS;
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_WRIST))
                                           pReset->arg3 = (number_range(1, 20) <= 10) ? WEAR_WRIST_L : WEAR_WR$
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_HANDS))
                                           pReset->arg3 = WEAR_HANDS;
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_SHIELD))
                                           pReset->arg3 = WEAR_SHIELD;
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_FINGER))
                                           pReset->arg3 = (number_range(1, 20) <= 10) ? WEAR_FINGER_L : WEAR_F$
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_LEGS))
                                           pReset->arg3 = WEAR_LEGS;
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_FEET))
                                           pReset->arg3 = WEAR_FEET;
                                        else if(CAN_WEAR(pObj, ITEM_WIELD))
                                           pReset->arg3 = WEAR_WIELD;
                                        else if(CAN_WEAR(pObj, ITEM_HOLD))
                                           pReset->arg3 = WEAR_HOLD;
                                        else if(CAN_WEAR(pObj, ITEM_WEAR_FLOAT))
                                           pReset->arg3 = WEAR_FLOAT;
                                        else if(pObj->wear_flags == ITEM_TAKE)
                                        {
                                                if(pObj->item_type == ITEM_LIGHT)
                                                        pReset->arg3 = WEAR_LIGHT;
                                                else
                                                        pReset->arg3 = WEAR_NONE;
                                        }
                                        else /* Log anything else */
                                        {
                                        sprintf(log_buf, "Bad wear flag - %s %s (%d), Reset (%d), Room (%d)", pObj->short_descr, wear_loc_strings[pReset->arg3], pObj->vnum, rnum, pRoom->vnum);
                                        /* Optional logging to file...
                                        append_file(NULL, "resets.log", buf); */
                                        log_string(buf);
                                        
                                        /* Set area changed */   
                                        SET_BIT(pRoom->area->area_flags, AREA_CHANGED);
                                 }
                           }
                      }
                }
          }
     }
}

>>> MERC.H or where you declare prototypes <<<

1) add under db.c

    void fix_resets args((void));
    long wear_bit args((int loc)); /* from olc_act.c */

>>> DB.C <<<

1) after fix_exits() add:

   logf("Fixing Resets..");
   fix_resets();

--------
Log on and enjoy...  I only had a few errors to fix cause i had mobs dual wieldeding
that were reset to wield. But other than that no complaints yet. =)

Markanth : markanth@spaceservices.net
Devil's Lament : spaceservices.net port 3778
Web Page : http://spaceservices.net/~markanth/
