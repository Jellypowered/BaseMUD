/***************************************************************************
 *  BaseMUD JSON hot-reload system.                                        *
 *                                                                         *
 *  File-watching: stat() poll every HOTRELOAD_SCAN_PULSES pulses.         *
 *  Debounce: only reload after HOTRELOAD_DEBOUNCE_SECS of stability.      *
 *  Safety: defers reload if any PC in the area is currently in combat.    *
 *  Immunity: HOTRELOAD_IMMUNITY_PULSES of zero-damage after a reload.     *
 ***************************************************************************/

#include "json_hotreload.h"

#ifdef BASEMUD_JSON_HOTRELOAD

#include "areas.h"
#include "chars.h"
#include "comm.h"
#include "fight.h"
#include "globals.h"
#include "json.h"
#include "json_import.h"
#include "lookup.h"
#include "objs.h"
#include "recycle.h"
#include "resets.h"
#include "rooms.h"
#include "utils.h"

#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* -----------------------------------------------------------------------
 * Module-private state
 * --------------------------------------------------------------------- */
static HOTRELOAD_ENTRY_T hotreload_table[HOTRELOAD_MAX_AREAS];
static int hotreload_count = 0;

/* Used inside hotreload_execute() to track displaced PCs. */
typedef struct
{
    CHAR_T *ch;
    int old_vnum;
} DISPLACED_PC_T;
/* -----------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------- */

/* Return the highest mtime of all *.json files directly under dir_path. */
static time_t hotreload_scan_dir_mtime(const char *dir_path)
{
    struct dirent **files;
    struct stat sb;
    char fbuf[1024];
    time_t max_mtime = 0;
    int count, i, len;
    const char *ext;

    count = scandir(dir_path, &files, NULL, alphasort);
    if (count < 0)
        return 0;

    for (i = 0; i < count; i++)
    {
        ext = strrchr(files[i]->d_name, '.');
        if (ext != NULL && strcasecmp(ext, ".json") == 0)
        {
            len = snprintf(fbuf, sizeof(fbuf), "%s%s",
                           dir_path, files[i]->d_name);
            if (len < (int)sizeof(fbuf) && stat(fbuf, &sb) == 0)
            {
                if (sb.st_mtime > max_mtime)
                    max_mtime = sb.st_mtime;
            }
        }
        free(files[i]);
    }
    free(files);
    return max_mtime;
}

/* -----------------------------------------------------------------------
 * Phase 2: Safe area teardown
 * --------------------------------------------------------------------- */

/* Unload one area from the live world.
 *
 * Steps performed (in order):
 *  1. Evacuate and extract all NPCs in area rooms.
 *  2. Stop fighting for all PCs in area rooms, move them to safe room.
 *  3. Extract all objects in area room contents.
 *  4. Null 'to_room' on exits from OTHER areas pointing INTO this area
 *     (preserves to_vnum for re-link after reload).
 *  5. area_free() — cascades safely through rooms/mobs/objs/etc.
 *
 * On return the caller's AREA_T pointer is dangling; it must be replaced
 * with the newly loaded AREA_T from json_import_area(). */
static void hotreload_unload_area(AREA_T *area)
{
    ROOM_INDEX_T *room, *room_next;
    CHAR_T *ch, *ch_next;
    OBJ_T *obj, *obj_next;
    EXIT_T *exit;
    ROOM_INDEX_T *safe_room;
    int dir;

    /* Find the safe room to evacuate PCs to. */
    safe_room = room_get_index(ROOM_VNUM_TEMPLE);
    if (safe_room == NULL)
        safe_room = room_get_index(ROOM_VNUM_LIMBO);

    /* Step 1 + 2 + 3: for every room in the area, clear inhabitants and
     * contents.  Snapshot room_next before the loop because area->room_first
     * is modified by char_from_room / obj_extract. */
    for (room = area->room_first; room != NULL; room = room_next)
    {
        room_next = room->area_next;

        /* 1a: Extract every NPC — this also stops their fights. */
        for (ch = room->people_first; ch != NULL; ch = ch_next)
        {
            ch_next = ch->room_next;
            if (!IS_NPC(ch))
                continue;
            stop_fighting(ch, TRUE);
            char_extract(ch);
        }

        /* 1b: Move every PC to the safe room. */
        for (ch = room->people_first; ch != NULL; ch = ch_next)
        {
            ch_next = ch->room_next;
            /* IS_NPC check: any remaining character is a PC. */
            stop_fighting(ch, TRUE);
            char_from_room(ch);
            if (safe_room != NULL)
                char_to_room(ch, safe_room);
            send_to_char(
                "\n\r{YNOTE{x: The area is being reloaded; stand by.\n\r",
                ch);
        }

        /* 3: Extract all objects in the room. */
        for (obj = room->content_first; obj != NULL; obj = obj_next)
        {
            obj_next = obj->content_next;
            obj_extract(obj);
        }
    }

    /* Step 4: Null 'to_room' on exits from areas outside this one that point
     * INTO this area's vnum range.  Preserve to_vnum for re-link after
     * reload. */
    {
        ROOM_INDEX_T *r;
        for (r = room_index_get_first(); r != NULL;
             r = room_index_get_next(r))
        {
            if (r->area == area)
                continue; /* own exits are freed with the area below */
            for (dir = 0; dir < DIR_MAX; dir++)
            {
                exit = r->exit[dir];
                if (exit == NULL || exit->to_room == NULL)
                    continue;
                if (exit->to_vnum >= area->min_vnum &&
                    exit->to_vnum <= area->max_vnum)
                {
                    /* Null only the pointer; preserve to_vnum. */
                    exit->to_room = NULL;
                }
            }
        }
    }

    /* Step 5: Free the area struct — cascades through rooms/mobs/objs. */
    area_free(area);
}

/* -----------------------------------------------------------------------
 * Phase 3: Reload execution
 * --------------------------------------------------------------------- */

static void hotreload_execute(HOTRELOAD_ENTRY_T *entry)
{
    AREA_T *new_area;
    CHAR_T *ch;
    ROOM_INDEX_T *old_room;
    time_t t_start;
    int imported = 0;

    /* ---- Player safety: defer if any PC in the area is in combat ---- */
    for (ch = char_first; ch != NULL; ch = ch->global_next)
    {
        if (IS_NPC(ch))
            continue;
        if (ch->in_room == NULL || ch->in_room->area != entry->area)
            continue;
        if (ch->fighting != NULL || ch->position == POS_FIGHTING)
        {
            log_f("[hotreload] Deferring reload of '%s' — PC '%s' "
                  "is in combat.",
                  entry->area_name, ch->name);
            /* Restart the debounce clock so we wait a full interval again. */
            entry->mtime_first_changed = (time_t)current_time;
            return;
        }
    }

    /* ---- Record each PC's current room vnum before the unload ---- */
    DISPLACED_PC_T displaced[256];
    int displaced_count = 0;

    for (ch = char_first; ch != NULL; ch = ch->global_next)
    {
        if (IS_NPC(ch) || ch->in_room == NULL)
            continue;
        if (ch->in_room->area == entry->area)
        {
            if (displaced_count < 256)
            {
                displaced[displaced_count].ch = ch;
                displaced[displaced_count].old_vnum = ch->in_room->vnum;
                displaced_count++;
            }
        }
    }

    /* ---- Notify immortals ---- */
    t_start = time(NULL);
    wiznetf(NULL, NULL, 0, 0, 0,
            "[hotreload] Reloading area '{W%s{x'...", entry->area_name);
    log_f("[hotreload] Reloading area '%s'...", entry->area_name);

    /* ---- Unload the old area ---- */
    hotreload_unload_area(entry->area);
    entry->area = NULL; /* dangling — must not dereference */

    /* ---- Load JSON files for this area ---- */
    json_import_area(entry->dir_path, &imported);

    /* ---- Find the freshly created AREA_T ---- */
    new_area = area_get_by_name_exact(entry->area_name);
    if (new_area == NULL)
    {
        log_f("[hotreload] ERROR: area '%s' failed to reload!",
              entry->area_name);
        entry->reload_pending = FALSE;
        entry->last_mtime = hotreload_scan_dir_mtime(entry->dir_path);
        return;
    }
    entry->area = new_area;

    /* ---- Link entities, exits, and commit initial resets ---- */
    json_import_link_one_area(new_area);
    reset_commit_area(new_area);
    area_reset(new_area);

    /* ---- Re-validate displaced PC room positions ---- */
    for (int i = 0; i < displaced_count; i++)
    {
        ch = displaced[i].ch;
        /* Character was moved to safe room by hotreload_unload_area(). */
        old_room = room_get_index(displaced[i].old_vnum);
        if (old_room != NULL)
        {
            /* Their old room still exists in the new version. */
            char_from_room(ch);
            char_to_room(ch, old_room);
        }
        else
        {
            send_to_char(
                "{YNOTE{x: Your room no longer exists — you have been "
                "relocated.\n\r",
                ch);
            /* ch is already in the safe room from hotreload_unload_area */
        }
    }

    /* ---- Grant post-reload immunity window ---- */
    reload_immunity_pulses = HOTRELOAD_IMMUNITY_PULSES;

    /* ---- Log and notify ---- */
    {
        long elapsed = (long)(time(NULL) - t_start);
        log_f("[hotreload] Area '%s' reloaded in %lds. "
              "%d object(s) imported.",
              new_area->name, elapsed, imported);
        wiznetf(NULL, NULL, 0, 0, 0,
                "[hotreload] Area '{W%s{x' reloaded (%lds, %d object(s)).",
                new_area->name, elapsed, imported);
    }

    /* ---- Update entry state ---- */
    entry->reload_pending = FALSE;
    entry->last_mtime = hotreload_scan_dir_mtime(entry->dir_path);
}

/* -----------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------- */

void hotreload_init(void)
{
    AREA_T *area;
    time_t mtime;

    hotreload_count = 0;
    for (area = area_first; area != NULL; area = area->global_next)
    {
        if (hotreload_count >= HOTRELOAD_MAX_AREAS)
        {
            bugf("hotreload_init: too many areas (max %d)",
                 HOTRELOAD_MAX_AREAS);
            break;
        }
        HOTRELOAD_ENTRY_T *e = &hotreload_table[hotreload_count];
        e->area = area;
        snprintf(e->area_name, sizeof(e->area_name), "%s", area->name);
        snprintf(e->dir_path, sizeof(e->dir_path), "%s%s/",
                 JSON_AREAS_DIR, area->name);
        mtime = hotreload_scan_dir_mtime(e->dir_path);
        e->last_mtime = mtime;
        e->mtime_first_changed = 0;
        e->reload_pending = FALSE;
        log_f("[hotreload] Watching '%s' (mtime=%ld)",
              e->dir_path, (long)mtime);
        hotreload_count++;
    }
    log_f("[hotreload] Watching %d area director%s.",
          hotreload_count, hotreload_count == 1 ? "y" : "ies");
}

void hotreload_scan(void)
{
    static long scan_count = 0;
    int i;
    time_t new_mtime;
    time_t now = (time_t)current_time;

    scan_count++;

    /* Periodic heartbeat every ~5 minutes so we can confirm scans run. */
    if (scan_count % 60 == 1)
        log_f("[hotreload] Scan #%ld running (%d areas watched).",
              scan_count, hotreload_count);

    for (i = 0; i < hotreload_count; i++)
    {
        HOTRELOAD_ENTRY_T *e = &hotreload_table[i];
        if (e->area == NULL)
            continue; /* area failed to reload previously */

        new_mtime = hotreload_scan_dir_mtime(e->dir_path);

        if (new_mtime != e->last_mtime)
            log_f("[hotreload] '%s': mtime %ld -> %ld%s",
                  e->area_name, (long)e->last_mtime, (long)new_mtime,
                  new_mtime > e->last_mtime ? " (changed)" : " (reverted?)");

        if (new_mtime > e->last_mtime)
        {
            if (!e->reload_pending)
            {
                e->reload_pending = TRUE;
                log_f("[hotreload] Change detected in area '%s'.",
                      e->area_name);
            }
            /* Reset the timer on every new change so we wait DEBOUNCE_SECS
             * after the LAST edit, not the first. */
            e->mtime_first_changed = now;
            e->last_mtime = new_mtime;
        }

        if (e->reload_pending)
        {
            long age = (long)(now - e->mtime_first_changed);
            if (age >= HOTRELOAD_DEBOUNCE_SECS)
                hotreload_execute(e);
        }
    }
}

#endif /* BASEMUD_JSON_HOTRELOAD */
