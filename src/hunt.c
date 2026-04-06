/* hunt.c — BFS path-finding for the 'hunt' skill.
 *
 * Original hunt/track code from SillyMUD Distribution V1.1b (c) 1993.
 * Ported to Merc 2.1 by Rip, to Merc 2.2 by Turtle (07-Nov-94),
 * to ROM 2.4 by Baxter for Deadland MUD (01-Feb-97).
 * Hidayet Dogan (hdogan@flipper.bitav.org.tr)
 * Adapted for BaseMUD: CHAR_T/OBJ_T types, EXIT_T/exit_flags, door_table,
 * char_first/global_next, char_get_skill, player_try_skill_improve,
 * intptr_t casts for 64-bit compatibility, memset/memmove for bcopy/bzero.
 */

#include "hunt.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "chars.h"
#include "comm.h"
#include "find.h"
#include "globals.h"
#include "interp.h"
#include "players.h"
#include "rooms.h"
#include "tables.h"
#include "utils.h"

/* -----------------------------------------------------------------------
 * Minimal open-addressing hash table used to track visited rooms and their
 * BFS ancestors during path-finding.  Values are stored as void * but are
 * actually small integers cast to pointer via intptr_t.
 * ----------------------------------------------------------------------- */

struct hash_link {
    int               key;
    struct hash_link *next;
    void             *data;
};

struct hash_header {
    int               rec_size;
    int               table_size;
    int              *keylist;
    int               klistsize;
    int               klistlen;
    struct hash_link **buckets;
};

#define HASH_KEY(ht, key) (((unsigned int)(key) * 17u) % (unsigned int)(ht)->table_size)

static void init_hash_table (struct hash_header *ht, int rec_size,
                              int table_size)
{
    ht->rec_size   = rec_size;
    ht->table_size = table_size;
    ht->buckets    = calloc ((size_t) table_size, sizeof (struct hash_link *));
    ht->keylist    = malloc (128 * sizeof (*ht->keylist));
    ht->klistsize  = 128;
    ht->klistlen   = 0;
}

static void donothing (void *data)
{
    (void) data;
}

static void destroy_hash_table (struct hash_header *ht,
                                 void (*gman) (void *))
{
    int               i;
    struct hash_link *scan, *tmp;

    for (i = 0; i < ht->table_size; i++)
        for (scan = ht->buckets[i]; scan;) {
            tmp = scan->next;
            gman (scan->data);
            free (scan);
            scan = tmp;
        }
    free (ht->buckets);
    free (ht->keylist);
}

static void *hash_find (struct hash_header *ht, int key)
{
    struct hash_link *scan = ht->buckets[HASH_KEY (ht, key)];
    while (scan && scan->key != key)
        scan = scan->next;
    return scan ? scan->data : NULL;
}

static void _hash_enter (struct hash_header *ht, int key, void *data)
{
    struct hash_link *temp;
    int               i;

    temp        = malloc (sizeof *temp);
    temp->key   = key;
    temp->next  = ht->buckets[HASH_KEY (ht, key)];
    temp->data  = data;
    ht->buckets[HASH_KEY (ht, key)] = temp;

    if (ht->klistlen >= ht->klistsize) {
        ht->klistsize *= 2;
        ht->keylist = realloc (ht->keylist,
                               (size_t) ht->klistsize * sizeof (*ht->keylist));
    }
    for (i = ht->klistlen; i > 0; i--) {
        if (ht->keylist[i - 1] < key) {
            ht->keylist[i] = key;
            break;
        }
        ht->keylist[i] = ht->keylist[i - 1];
    }
    if (i == 0)
        ht->keylist[0] = key;
    ht->klistlen++;
}

static int hash_enter (struct hash_header *ht, int key, void *data)
{
    if (hash_find (ht, key))
        return 0;
    _hash_enter (ht, key, data);
    return 1;
}

/* -----------------------------------------------------------------------
 * BFS queue node.
 * ----------------------------------------------------------------------- */

struct room_q {
    int            room_nr;
    struct room_q *next_q;
};

/* -----------------------------------------------------------------------
 * find_path — breadth-first search from in_room_vnum to out_room_vnum.
 *
 * Returns the first direction to take from in_room, or -1 if unreachable.
 * depth < 0  → pass through closed doors; actual depth = -depth.
 * in_zone    → restrict search to the same area as the starting room.
 * ----------------------------------------------------------------------- */

static int find_path (int in_room_vnum, int out_room_vnum, CHAR_T *ch,
                      int depth, int in_zone)
{
    struct room_q      *tmp_q, *q_head, *q_tail;
    struct hash_header  x_room;
    int                 i, tmp_room, count = 0, thru_doors;
    ROOM_INDEX_T       *herep, *startp;
    EXIT_T             *exitp;

    (void) ch; /* reserved for future visibility checks */

    if (depth < 0) {
        thru_doors = TRUE;
        depth      = -depth;
    }
    else {
        thru_doors = FALSE;
    }

    startp = room_get_index (in_room_vnum);
    if (startp == NULL)
        return -1;

    init_hash_table (&x_room, sizeof (int), 2048);
    hash_enter (&x_room, in_room_vnum, (void *) (intptr_t) -1);

    q_head           = malloc (sizeof *q_head);
    q_tail           = q_head;
    q_tail->room_nr  = in_room_vnum;
    q_tail->next_q   = NULL;

    while (q_head) {
        herep = room_get_index (q_head->room_nr);
        if (herep == NULL) {
            tmp_q  = q_head->next_q;
            free (q_head);
            q_head = tmp_q;
            continue;
        }

        if (!in_zone || herep->area == startp->area) {
            for (i = 0; i < DIR_MAX; i++) {
                exitp = herep->exit[i];
                if (exitp == NULL || exitp->to_room == NULL)
                    continue;
                if (!thru_doors && IS_SET (exitp->exit_flags, EX_CLOSED))
                    continue;

                tmp_room = exitp->to_room->vnum;

                if (tmp_room != out_room_vnum) {
                    if (!hash_find (&x_room, tmp_room) && count < depth) {
                        count++;
                        tmp_q           = malloc (sizeof *tmp_q);
                        tmp_q->room_nr  = tmp_room;
                        tmp_q->next_q   = NULL;
                        q_tail->next_q  = tmp_q;
                        q_tail          = tmp_q;

                        /* Propagate the first-step direction from the start. */
                        hash_enter (&x_room, tmp_room,
                            ((intptr_t) hash_find (&x_room, q_head->room_nr) == -1)
                                ? (void *) (intptr_t) (i + 1)
                                : hash_find (&x_room, q_head->room_nr));
                    }
                }
                else {
                    /* Found the destination.  Determine first direction. */
                    intptr_t ancestor;
                    int      dir;

                    tmp_room = q_head->room_nr;
                    for (; q_head; q_head = tmp_q) {
                        tmp_q = q_head->next_q;
                        free (q_head);
                    }
                    ancestor = (intptr_t) hash_find (&x_room, tmp_room);
                    dir = (ancestor == -1) ? i : (int) ancestor - 1;
                    destroy_hash_table (&x_room, donothing);
                    return dir;
                }
            }
        }

        tmp_q  = q_head->next_q;
        free (q_head);
        q_head = tmp_q;
    }

    destroy_hash_table (&x_room, donothing);
    return -1;
}

/* -----------------------------------------------------------------------
 * find_char_area — like find_char_world but restricted to the same area.
 * Checks the current room first so exact matches are preferred.
 * ----------------------------------------------------------------------- */

static CHAR_T *find_char_area (CHAR_T *ch, const char *argument)
{
    CHAR_T *ach;
    char    arg[MAX_INPUT_LENGTH];
    int     number, count;

    if ((ach = find_char_same_room (ch, argument)) != NULL)
        return ach;

    number = number_argument (argument, arg);
    count  = 0;
    for (ach = char_first; ach != NULL; ach = ach->global_next) {
        if (ach->in_room == NULL || ach->in_room->area != ch->in_room->area)
            continue;
        if (!char_can_see_anywhere (ch, ach))
            continue;
        if (!str_in_namelist (arg, ach->name))
            continue;
        if (++count == number)
            return ach;
    }
    return NULL;
}

/* -----------------------------------------------------------------------
 * do_hunt — track a character by name and report the direction to travel.
 * ----------------------------------------------------------------------- */

DEFINE_DO_FUN (do_hunt) {
    char    arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    int     direction;
    bool    fArea;

    one_argument (argument, arg);

    BAIL_IF (char_get_skill (ch, SN (HUNT)) == 0,
        "Huh?\n\r", ch);
    DO_REQUIRE_ARG (arg, "Whom are you trying to hunt?\n\r");

    /* Only high-trust characters can hunt across areas. */
    fArea = (char_get_trust (ch) < MAX_LEVEL);

    if (IS_NPC (ch))
        victim = find_char_world (ch, arg);
    else if (fArea)
        victim = find_char_area (ch, arg);
    else
        victim = find_char_world (ch, arg);

    BAIL_IF (victim == NULL,
        "No one around by that name.\n\r", ch);

    if (ch->in_room == victim->in_room) {
        act ("$N is here!", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (ch->move > 2)
        ch->move -= 3;
    else {
        send_to_char ("You're too exhausted to hunt anyone!\n\r", ch);
        return;
    }

    act ("$n carefully sniffs the air.", ch, NULL, NULL, TO_NOTCHAR);
    WAIT_STATE (ch, skill_table[SN (HUNT)].beats);

    direction = find_path (ch->in_room->vnum, victim->in_room->vnum,
                           ch, -40000, fArea);

    BAIL_IF (direction == -1,
        "You couldn't find a path to your quarry from here.\n\r", ch);

    if (direction < 0 || direction >= DIR_MAX) {
        send_to_char ("Hmm... Something seems to be wrong.\n\r", ch);
        return;
    }

    /* Random direction on a failed roll. */
    if (!IS_NPC (ch) && number_percent () > char_get_skill (ch, SN (HUNT))) {
        do {
            direction = number_door ();
        } while (ch->in_room->exit[direction] == NULL ||
                 ch->in_room->exit[direction]->to_room == NULL);
    }

    {
        char buf[MAX_STRING_LENGTH];
        snprintf (buf, sizeof buf, "$N is %s from here.",
                  door_table[direction].name);
        act (buf, ch, NULL, victim, TO_CHAR);
    }
    player_try_skill_improve (ch, SN (HUNT), TRUE, 1);
}
