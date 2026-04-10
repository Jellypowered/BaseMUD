/***************************************************************************
 * Pocket Dungeon — player-facing command.
 * act_instance.c — `dungeon` command.
 ***************************************************************************/

#include "act_instance.h"

#include "chars.h"
#include "comm.h"
#include "globals.h"
#include "groups.h"
#include "interp.h"
#include "memory.h"
#include "pocket_dungeon.h"
#include "rooms.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>

/* Gather the player's current group members (PCs only, same room required
 * to co-enter; leader may be in a different room, which is fine). */
static int pd_collect_group(CHAR_T *ch, CHAR_T **out, int max)
{
    CHAR_T *gch;
    int count = 0;
    for (gch = char_first; gch != NULL; gch = gch->global_next) {
        if (count >= max)
            break;
        if (IS_NPC(gch))
            continue;
        if (!is_same_group(gch, ch))
            continue;
        out[count++] = gch;
    }
    return count;
}

void do_dungeon(CHAR_T *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PD_INSTANCE_T *inst;

    argument = one_argument(argument, arg1);
    one_argument(argument, arg2);

    if (IS_NPC(ch)) {
        send_to_char("Not for NPCs.\n\r", ch);
        return;
    }

    if (arg1[0] == '\0' || !str_cmp(arg1, "help")) {
        send_to_char("Pocket Dungeon commands:\n\r"
                     "  dungeon enter [theme]  - Enter a new pocket dungeon (creates one).\n\r"
                     "  dungeon leave          - Return to the temple from your dungeon.\n\r"
                     "  dungeon status         - Show your current dungeon info.\n\r"
                     "  dungeon list           - List available dungeon themes.\n\r",
                     ch);
        return;
    }

    /* LIST */
    if (!str_cmp(arg1, "list")) {
        PD_SEED_T *seed;
        if (pd_seed_first == NULL) {
            send_to_char("No dungeon themes are currently available.\n\r", ch);
            return;
        }
        send_to_char("Available dungeon themes:\n\r", ch);
        for (seed = pd_seed_first; seed != NULL; seed = seed->global_next) {
            char buf[256];
            snprintf(buf, sizeof(buf), "  %-15s %s\n\r",
                     seed->name ? seed->name : "(unnamed)",
                     seed->title ? seed->title : "");
            send_to_char(buf, ch);
        }
        return;
    }

    /* STATUS */
    if (!str_cmp(arg1, "status")) {
        inst = pd_find_instance_for_char(ch);
        if (inst == NULL) {
            send_to_char("You are not inside a pocket dungeon.\n\r", ch);
            return;
        }
        char buf[512];
        snprintf(buf, sizeof(buf),
                 "Dungeon: %s  (theme: %s, level: %d, slot: %d)\n\r"
                 "Members: %d  |  Entry vnum: %d\n\r",
                 inst->area_name,
                 inst->theme ? inst->theme : "unknown",
                 inst->level,
                 inst->vnum_slot,
                 inst->member_count,
                 inst->entry_vnum);
        send_to_char(buf, ch);
        return;
    }

    /* LEAVE */
    if (!str_cmp(arg1, "leave")) {
        inst = pd_find_instance_for_char(ch);
        if (inst == NULL) {
            send_to_char("You are not inside a pocket dungeon.\n\r", ch);
            return;
        }
        ROOM_INDEX_T *dest = room_get_index(ROOM_VNUM_TEMPLE);
        if (inst->origin_vnum != 0)
            dest = room_get_index(inst->origin_vnum);
        if (dest == NULL)
            dest = room_get_index(ROOM_VNUM_TEMPLE);
        char_from_room(ch);
        char_to_room(ch, dest);
        send_to_char("You step through the portal and leave the dungeon behind.\n\r", ch);
        act("$n steps through a shimmering portal and disappears.", ch, NULL, NULL, TO_NOTCHAR);
        return;
    }

    /* ENTER */
    if (!str_cmp(arg1, "enter")) {
        const char *theme = arg2[0] ? arg2 : NULL;

        if (pd_find_instance_for_char(ch) != NULL) {
            send_to_char("You are already inside a pocket dungeon. Use 'dungeon leave' first.\n\r", ch);
            return;
        }

        /* Count active instances */
        {
            int count = 0;
            PD_INSTANCE_T *scan;
            for (scan = pd_instance_first; scan != NULL; scan = scan->global_next)
                count++;
            if (count >= pd_config.max_instances) {
                send_to_char("All dungeon slots are currently in use. Try again later.\n\r", ch);
                return;
            }
        }

        /* Gather group */
        CHAR_T *members[MAX_INSTANCE_MEMBERS];
        int member_count = pd_collect_group(ch, members, MAX_INSTANCE_MEMBERS);
        if (member_count == 0) {
            members[0] = ch;
            member_count = 1;
        }

        /* Remember origin vnum */
        int origin_vnum = (ch->in_room != NULL) ? ch->in_room->vnum : ROOM_VNUM_TEMPLE;

        inst = pd_generate_instance(members, member_count, theme);
        if (inst == NULL) {
            send_to_char("Failed to generate a pocket dungeon. Please try again later.\n\r", ch);
            return;
        }

        inst->origin_vnum = origin_vnum;

        /* Update the return portal's to_vnum to send them back here */
        ROOM_INDEX_T *entry_room = room_get_index(inst->entry_vnum);
        if (entry_room != NULL) {
            OBJ_T *obj;
            for (obj = entry_room->content_first; obj != NULL; obj = obj->content_next) {
                if (obj->item_type == ITEM_PORTAL) {
                    obj->v.portal.to_vnum = origin_vnum;
                    break;
                }
            }
        }

        /* Teleport all group members into the dungeon */
        int i;
        for (i = 0; i < member_count; i++) {
            CHAR_T *m = members[i];
            if (m->in_room == NULL)
                continue;
            ROOM_INDEX_T *dest = room_get_index(inst->entry_vnum);
            if (dest == NULL)
                continue;
            act("A shimmering portal opens before $n.", m, NULL, NULL, TO_NOTCHAR);
            char_from_room(m);
            char_to_room(m, dest);
            send_to_char("You step through a shimmering portal into a pocket dungeon!\n\r", m);
            act("$n steps through a shimmering portal.", m, NULL, NULL, TO_NOTCHAR);
            /* do_look equivalent */
            /* (players will see the room on next prompt/look) */
        }
        return;
    }

    send_to_char("Unknown dungeon command. Type 'dungeon help' for a list.\n\r", ch);
}
