/***************************************************************************
 * Pocket Dungeon — immortal command.
 * wiz_instance.c — `pocketdungeon` command.
 ***************************************************************************/

#include "wiz_instance.h"

#include "chars.h"
#include "comm.h"
#include "globals.h"
#include "interp.h"
#include "pocket_dungeon.h"
#include "rooms.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void do_pocketdungeon(CHAR_T *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    one_argument(argument, arg2);

    if (arg1[0] == '\0') {
        send_to_char("Syntax: pocketdungeon <list|create|destroy|info>\n\r", ch);
        return;
    }

    /* LIST */
    if (!str_cmp(arg1, "list")) {
        PD_INSTANCE_T *inst;
        if (pd_instance_first == NULL) {
            send_to_char("No active pocket dungeon instances.\n\r", ch);
            return;
        }
        char buf[256];
        send_to_char("Active pocket dungeon instances:\n\r", ch);
        for (inst = pd_instance_first; inst != NULL; inst = inst->global_next) {
            snprintf(buf, sizeof(buf),
                     "  [%d] theme=%-10s level=%-3d members=%-2d entry=%d\n\r",
                     inst->vnum_slot,
                     inst->theme ? inst->theme : "?",
                     inst->level,
                     inst->member_count,
                     inst->entry_vnum);
            send_to_char(buf, ch);
        }
        return;
    }

    /* CREATE */
    if (!str_cmp(arg1, "create")) {
        CHAR_T *members[1];
        members[0] = ch;
        PD_INSTANCE_T *inst = pd_generate_instance(members, 1,
                                                    arg2[0] ? arg2 : NULL);
        if (inst == NULL) {
            send_to_char("Failed to create pocket dungeon instance.\n\r", ch);
            return;
        }
        inst->origin_vnum = (ch->in_room != NULL) ? ch->in_room->vnum
                                                   : ROOM_VNUM_TEMPLE;
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "Pocket dungeon created: slot %d, entry vnum %d.\n\r",
                 inst->vnum_slot, inst->entry_vnum);
        send_to_char(buf, ch);
        return;
    }

    /* DESTROY */
    if (!str_cmp(arg1, "destroy")) {
        if (arg2[0] == '\0') {
            send_to_char("Syntax: pocketdungeon destroy <slot>\n\r", ch);
            return;
        }
        int slot = atoi(arg2);
        PD_INSTANCE_T *inst;
        for (inst = pd_instance_first; inst != NULL; inst = inst->global_next) {
            if (inst->vnum_slot == slot)
                break;
        }
        if (inst == NULL) {
            send_to_char("No instance with that slot number.\n\r", ch);
            return;
        }
        pd_destroy_instance(inst);
        send_to_char("Pocket dungeon destroyed.\n\r", ch);
        return;
    }

    /* INFO */
    if (!str_cmp(arg1, "info")) {
        if (arg2[0] == '\0') {
            send_to_char("Syntax: pocketdungeon info <slot>\n\r", ch);
            return;
        }
        int slot = atoi(arg2);
        PD_INSTANCE_T *inst;
        for (inst = pd_instance_first; inst != NULL; inst = inst->global_next) {
            if (inst->vnum_slot == slot)
                break;
        }
        if (inst == NULL) {
            send_to_char("No instance with that slot number.\n\r", ch);
            return;
        }
        char buf[512];
        snprintf(buf, sizeof(buf),
                 "Slot:      %d\n\r"
                 "Theme:     %s\n\r"
                 "Level:     %d\n\r"
                 "Members:   %d\n\r"
                 "Entry:     %d\n\r"
                 "Origin:    %d\n\r"
                 "Vnums:     %d - %d\n\r",
                 inst->vnum_slot,
                 inst->theme ? inst->theme : "?",
                 inst->level,
                 inst->member_count,
                 inst->entry_vnum,
                 inst->origin_vnum,
                 pd_config.vnum_base + inst->vnum_slot * pd_config.vnum_size,
                 pd_config.vnum_base + inst->vnum_slot * pd_config.vnum_size
                     + pd_config.vnum_size - 1);
        send_to_char(buf, ch);
        return;
    }

    send_to_char("Syntax: pocketdungeon <list|create [theme]|destroy <slot>|info <slot>>\n\r", ch);
}
