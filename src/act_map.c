/***************************************************************************
 *  ASCII Automap                                                           *
 *  Original snippet by mlkesl@stthomas.edu                                *
 *  Adapted for BaseMUD by the BaseMUD project.                            *
 ***************************************************************************/

#include "act_map.h"

#include "chars.h"
#include "comm.h"
#include "interp.h"
#include "lookup.h"
#include "players.h"
#include "rooms.h"
#include "tables.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>

/* Map grid size.  A 40x40 grid comfortably fits the largest wilderness
 * regions without excessive recursion depth.  SECT_MAX acts as the
 * "unvisited" sentinel; SECT_MAX+1 is one-way/maze; SECT_MAX+2 is the
 * player's current position. */
#define MAX_MAP     40
#define MAX_MAP_DIR 4   /* cardinal directions only for 2D rendering */

/* Offsets: [N, E, S, W] */
static int offsets[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };

/* Global map grid, reset before each use. */
static int map[MAX_MAP][MAX_MAP];

/* -------------------------------------------------------------------------
 * MapArea - recursive flood fill that populates the map grid.
 * ------------------------------------------------------------------------- */
static void MapArea (ROOM_INDEX_T *room, CHAR_T *ch,
                     int x, int y, int min, int max)
{
    ROOM_INDEX_T *prospect;
    EXIT_T *pexit;
    int door;

    /* Mark this cell as visited with the room's sector type. */
    map[x][y] = room->sector_type;

    for (door = 0; door < MAX_MAP_DIR; door++) {
        if ((pexit = room->exit[door]) == NULL)
            continue;
        if (pexit->to_room == NULL)
            continue;
        if (!char_can_see_room (ch, pexit->to_room))
            continue;
        if (IS_SET (pexit->exit_flags, EX_CLOSED))
            continue;

        /* Boundary check. */
        if (x < min || y < min || x > max || y > max)
            return;

        prospect = pexit->to_room;

        /* One-way or maze room: mark and stop this branch. */
        if (prospect->exit[REV_DIR (door)] == NULL ||
            prospect->exit[REV_DIR (door)]->to_room != room)
        {
            map[x][y] = SECT_MAX + 1;
            return;
        }

        /* Sectors that block sight (city / inside stop recursion into them). */
        if (prospect->sector_type == SECT_INSIDE ||
            prospect->sector_type == SECT_CITY)
        {
            map[x + offsets[door][0]][y + offsets[door][1]] =
                prospect->sector_type;
            continue;
        }

        /* Only recurse into unvisited cells. */
        if (map[x + offsets[door][0]][y + offsets[door][1]] == SECT_MAX) {
            MapArea (prospect, ch,
                     x + offsets[door][0], y + offsets[door][1],
                     min, max);
        }
    }
}

/* -------------------------------------------------------------------------
 * ShowMap - print the full map grid with color codes.
 * The run-length trick: only emit the color prefix when the sector changes;
 * repeat cells on the same row with the same sector use the bare glyph.
 * ------------------------------------------------------------------------- */
static void ShowMap (CHAR_T *ch, int min, int max)
{
    int x, y;
    bool new_sector;

    for (x = min; x < max; x++) {
        for (y = min; y < max; y++) {
            new_sector = (y == min || map[x][y - 1] != map[x][y]);

            switch (map[x][y]) {
                case SECT_MAX:        send_to_char (" ",    ch); break;
                case SECT_FOREST:     send_to_char (new_sector ? "{g@" : "@",  ch); break;
                case SECT_FIELD:      send_to_char (new_sector ? "{G\"" : "\"", ch); break;
                case SECT_HILLS:      send_to_char (new_sector ? "{y^" : "^",  ch); break;
                case SECT_MOUNTAIN:   send_to_char (new_sector ? "{R^" : "^",  ch); break;
                case SECT_WATER_SWIM: send_to_char (new_sector ? "{B~" : "~",  ch); break;
                case SECT_WATER_NOSWIM: send_to_char (new_sector ? "{b~" : "~", ch); break;
                case SECT_UNUSED:     send_to_char (new_sector ? "{DX" : "X",  ch); break;
                case SECT_AIR:        send_to_char (new_sector ? "{C%" : "%",  ch); break;
                case SECT_DESERT:     send_to_char (new_sector ? "{Y=" : "=",  ch); break;
                case SECT_INSIDE:     send_to_char (new_sector ? "{W%" : "%",  ch); break;
                case SECT_CITY:       send_to_char (new_sector ? "{W#" : "#",  ch); break;
                case SECT_MAX + 1:    send_to_char (new_sector ? "{D?" : "?",  ch); break;
                default:              send_to_char (new_sector ? "{R*" : "*",  ch); break;
            }
        }
        send_to_char ("{x\n\r", ch);
    }
}

/* -------------------------------------------------------------------------
 * ShowHalfMap - compressed view, sampling every other cell.
 * Used by do_smallmap (immortal command).
 * ------------------------------------------------------------------------- */
static void ShowHalfMap (CHAR_T *ch, int min, int max)
{
    int x, y;
    bool new_sector;

    for (x = min; x < max; x += 2) {
        for (y = min; y < max; y += 2) {
            new_sector = (y == min || map[x][y - 2] != map[x][y]);

            switch (map[x][y]) {
                case SECT_MAX:        send_to_char (" ",    ch); break;
                case SECT_FOREST:     send_to_char (new_sector ? "{g@" : "@",  ch); break;
                case SECT_FIELD:      send_to_char (new_sector ? "{G\"" : "\"", ch); break;
                case SECT_HILLS:      send_to_char (new_sector ? "{y^" : "^",  ch); break;
                case SECT_MOUNTAIN:   send_to_char (new_sector ? "{R^" : "^",  ch); break;
                case SECT_WATER_SWIM: send_to_char (new_sector ? "{B~" : "~",  ch); break;
                case SECT_WATER_NOSWIM: send_to_char (new_sector ? "{b~" : "~", ch); break;
                case SECT_UNUSED:     send_to_char (new_sector ? "{DX" : "X",  ch); break;
                case SECT_AIR:        send_to_char (new_sector ? "{C%" : "%",  ch); break;
                case SECT_DESERT:     send_to_char (new_sector ? "{Y=" : "=",  ch); break;
                case SECT_INSIDE:     send_to_char (new_sector ? "{W%" : "%",  ch); break;
                case SECT_CITY:       send_to_char (new_sector ? "{W#" : "#",  ch); break;
                case SECT_MAX + 1:    send_to_char (new_sector ? "{D?" : "?",  ch); break;
                default:              send_to_char (new_sector ? "{R*" : "*",  ch); break;
            }
        }
        send_to_char ("{x\n\r", ch);
    }
}

/* -------------------------------------------------------------------------
 * do_map - player command.
 *
 * Mortals: requires the 'map' skill and ROOM_WILDERNESS flag. Shows a fixed
 * 11x11 area centered on the player.
 *
 * Immortals: accepts an optional size argument; defaults to 13.
 * ------------------------------------------------------------------------- */
DEFINE_DO_FUN (do_map)
{
    int size, center, x, y, min, max;
    char arg1[MAX_INPUT_LENGTH];
    int skill;

    one_argument (argument, arg1);

    /* Mortal checks. */
    if (!IS_IMMORTAL (ch)) {
        skill = char_get_skill (ch, SN (MAP));
        if (skill == 0) {
            send_to_char ("You don't know how to read the terrain.\n\r", ch);
            return;
        }
        if (!IS_SET (ch->in_room->room_flags, ROOM_WILDERNESS)) {
            send_to_char ("You can only map wilderness areas.\n\r", ch);
            return;
        }
        if (room_is_dark (ch->in_room)) {
            send_to_char (
                "It is too dark to map your surroundings.\n\r", ch);
            return;
        }
    }

    /* Determine view size. */
    if (arg1[0] != '\0')
        size = atoi (arg1);
    else
        size = IS_IMMORTAL (ch) ? 13 : 11;

    size   = URANGE (6, size, MAX_MAP - 2);
    center = MAX_MAP / 2;
    min    = center - size / 2;
    max    = center + size / 2;

    /* Reset the grid. */
    for (x = 0; x < MAX_MAP; x++)
        for (y = 0; y < MAX_MAP; y++)
            map[x][y] = SECT_MAX;

    /* Flood-fill from the player's room. */
    MapArea (ch->in_room, ch, center, center, min - 1, max - 1);

    /* Mark the player's position (default: '*' via switch default). */
    map[center][center] = SECT_MAX + 2;

    ShowMap (ch, min, max);

    /* Skill improvement for mortals. */
    if (!IS_IMMORTAL (ch))
        player_try_skill_improve (ch, SN (MAP), TRUE, 4);
}

/* -------------------------------------------------------------------------
 * do_smallmap - immortal-only compressed map view.
 * ------------------------------------------------------------------------- */
DEFINE_DO_FUN (do_smallmap)
{
    int size, center, x, y, min, max;
    char arg1[MAX_INPUT_LENGTH];

    one_argument (argument, arg1);

    if (!IS_IMMORTAL (ch)) {
        send_to_char ("Huh?\n\r", ch);
        return;
    }

    if (arg1[0] != '\0')
        size = atoi (arg1);
    else
        size = 20;

    size   = URANGE (6, size, MAX_MAP - 2);
    center = MAX_MAP / 2;
    min    = center - size / 2;
    max    = center + size / 2;

    for (x = 0; x < MAX_MAP; x++)
        for (y = 0; y < MAX_MAP; y++)
            map[x][y] = SECT_MAX;

    MapArea (ch->in_room, ch, center, center, min - 1, max - 1);
    map[center][center] = SECT_MAX + 2;

    ShowHalfMap (ch, min, max);
}
