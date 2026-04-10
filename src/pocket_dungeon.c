/***************************************************************************
 * Pocket Dungeon — instanced procedural area generator.
 * pocket_dungeon.c — implementation.
 ***************************************************************************/

#include "globals.h"
#include "pocket_dungeon.h"

#include "chars.h"
#include "comm.h"
#include "db.h"
#include "defs.h"
#include "memory.h"
#include "mobiles.h"
#include "objs.h"
#include "recycle.h"
#include "rooms.h"
#include "tables.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

/* Scale a mobile's stats to a target level.
 * Used after mobile_create() so instance mobs match the group's level
 * with minor ±2 variance rather than the fixed template level. */
static void pd_scale_mob_to_level(CHAR_T *mob, int target_level)
{
    int i;

    if (mob == NULL)
        return;
    if (target_level < 1)
        target_level = 1;
    if (target_level == mob->level)
        return;

    mob->level = target_level;

    /* Recompute HP with the standard level formula (format-agnostic). */
    mob->max_hit = (target_level * 8
        + number_range(target_level * target_level / 4,
                       target_level * target_level)) * 9 / 10;
    if (mob->max_hit < 1)
        mob->max_hit = 1;
    mob->hit = mob->max_hit;

    /* Recompute mana */
    mob->max_mana = 100 + dice(target_level, 10);
    mob->mana     = mob->max_mana;

    /* Combat rolls */
    mob->hitroll = target_level / 5;
    mob->damroll  = target_level / 4;

    /* Armour class */
    for (i = 0; i < 3; i++)
        mob->armor[i] = int_interpolate(target_level, 100, -100);
    mob->armor[3] = int_interpolate(target_level, 100, 0);

    /* Stats */
    for (i = 0; i < STAT_MAX; i++)
        mob->perm_stat[i] = UMIN(25, 11 + target_level / 4);
}

/* Find the first free vnum slot among AREA_INSTANCE_MAX possible slots.
 * Returns -1 if none available. */
static int pd_find_free_slot(void)
{
    int slot;
    PD_INSTANCE_T *inst;

    for (slot = 0; slot < pd_config.max_instances; slot++) {
        bool used = FALSE;
        for (inst = pd_instance_first; inst != NULL; inst = inst->global_next) {
            if (inst->vnum_slot == slot) {
                used = TRUE;
                break;
            }
        }
        if (!used)
            return slot;
    }
    return -1;
}

/* Build a two-way bidirectional exit between two rooms. */
static void pd_link_rooms(ROOM_INDEX_T *from, int dir, ROOM_INDEX_T *to)
{
    EXIT_T *ex_fwd, *ex_rev;
    int rev = door_table[dir].reverse;

    if (from->exit[dir] == NULL) {
        ex_fwd = room_create_exit(from, dir);
        exit_to_room_index_to(ex_fwd, to);
    }
    if (to->exit[rev] == NULL) {
        ex_rev = room_create_exit(to, rev);
        exit_to_room_index_to(ex_rev, from);
    }
}

/* Pick a room name from the seed, cycling to avoid repeats. */
static const char *pd_pick_room_name(PD_SEED_T *seed, int index)
{
    if (seed->room_name_count <= 0)
        return "dungeon chamber";
    return seed->room_names[index % seed->room_name_count];
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

PD_SEED_T *pd_seed_get_by_name(const char *name)
{
    PD_SEED_T *seed;
    if (name == NULL || name[0] == '\0')
        return NULL;
    for (seed = pd_seed_first; seed != NULL; seed = seed->global_next)
        if (seed->name != NULL && !str_cmp(seed->name, name))
            return seed;
    return NULL;
}

PD_INSTANCE_T *pd_find_instance_for_char(const CHAR_T *ch)
{
    PD_INSTANCE_T *inst;
    if (ch == NULL || ch->in_room == NULL || ch->in_room->area == NULL)
        return NULL;
    for (inst = pd_instance_first; inst != NULL; inst = inst->global_next)
        if (inst->area == ch->in_room->area)
            return inst;
    return NULL;
}

PD_INSTANCE_T *pd_find_instance_by_member(const char *name)
{
    PD_INSTANCE_T *inst;
    int i;
    if (name == NULL || name[0] == '\0')
        return NULL;
    for (inst = pd_instance_first; inst != NULL; inst = inst->global_next) {
        for (i = 0; i < inst->member_count; i++) {
            if (inst->members[i] != NULL && !str_cmp(inst->members[i], name))
                return inst;
        }
    }
    return NULL;
}

PD_INSTANCE_T *pd_generate_instance(CHAR_T **members, int member_count,
                                     const char *theme)
{
    PD_SEED_T *seed;
    PD_INSTANCE_T *inst;
    AREA_T *area;
    ROOM_INDEX_T *rooms[100]; /* guard against room_count_max > 100 */
    int room_count, i, slot;
    int vnum_base;
    char buf[256];

    /* Select seed */
    if (theme != NULL && theme[0] != '\0')
        seed = pd_seed_get_by_name(theme);
    else
        seed = NULL;

    if (seed == NULL) {
        /* pick random seed */
        int count = 0;
        PD_SEED_T *s;
        for (s = pd_seed_first; s != NULL; s = s->global_next)
            count++;
        if (count == 0) {
            bugf("pd_generate_instance: no seeds loaded");
            return NULL;
        }
        int pick = number_range(0, count - 1);
        seed = pd_seed_first;
        for (i = 0; i < pick && seed != NULL; i++)
            seed = seed->global_next;
        if (seed == NULL)
            seed = pd_seed_first;
    }

    /* Find a free slot */
    slot = pd_find_free_slot();
    if (slot < 0) {
        bugf("pd_generate_instance: no free instance slots");
        return NULL;
    }
    vnum_base = pd_config.vnum_base + slot * pd_config.vnum_size;

    /* Create the instance area */
    area = area_new();
    str_free(&area->name);
    snprintf(buf, sizeof(buf), "pd-inst-%d", slot);
    area->name = str_dup(buf);

    str_free(&area->title);
    snprintf(buf, sizeof(buf), "%s (Instance)", seed->title ? seed->title : "Pocket Dungeon");
    area->title = str_dup(buf);

    str_free(&area->filename);
    area->filename = str_dup("");

    str_free(&area->builders);
    area->builders = str_dup("None");

    area->min_vnum   = vnum_base;
    area->max_vnum   = vnum_base + pd_config.vnum_size - 1;
    area->area_flags = AREA_INSTANCE | AREA_HIDDEN;
    area->security   = 0;

    /* Determine room count for this instance */
    room_count = seed->room_count_min;
    if (seed->room_count_max > seed->room_count_min)
        room_count = number_range(seed->room_count_min, seed->room_count_max);
    if (room_count < 1)
        room_count = 4;
    if (room_count > pd_config.vnum_size - 1)
        room_count = pd_config.vnum_size - 1;
    if (room_count > 100)
        room_count = 100;

    /* Create rooms */
    memset(rooms, 0, sizeof(rooms));
    for (i = 0; i < room_count; i++) {
        ROOM_INDEX_T *room = room_index_new();
        int vnum = vnum_base + i;

        str_replace_dup(&room->name, pd_pick_room_name(seed, i));
        str_replace_dup(&room->description,
            "Dark stone walls press in around you. Torchlight flickers against damp rock.\n\r");

        room->area    = area;
        room->vnum    = vnum;
        room->anum    = i;
        room->sector_type = SECT_INSIDE;
        SET_BIT(room->room_flags, ROOM_INDOORS);

        room_to_area(room, area);
        room_index_to_hash(room);

        rooms[i] = room;
    }

    /* Link rooms in a simple chain with occasional branches */
    for (i = 0; i + 1 < room_count; i++)
        pd_link_rooms(rooms[i], DIR_NORTH, rooms[i + 1]);

    /* Add a couple of east-west branches for rooms 2+ */
    for (i = 2; i + 2 < room_count; i += 3)
        pd_link_rooms(rooms[i], DIR_EAST, rooms[i + 1]);

    /* entry_vnum is the first room; place entry portal there */
    int entry_vnum = vnum_base;

    /* Create the instance record */
    inst                = pd_instance_new();
    inst->vnum_slot     = slot;
    inst->area          = area;
    inst->entry_vnum    = entry_vnum;
    inst->created_at    = (long)time(NULL);
    inst->last_empty_at = 0;
    inst->level         = (member_count > 0 && members[0] != NULL)
                            ? members[0]->level : 1;

    str_replace_dup(&inst->theme, seed->name ? seed->name : "");
    snprintf(inst->area_name, sizeof(inst->area_name), "%s", area->name);

    /* Copy members */
    inst->member_count = 0;
    for (i = 0; i < member_count && i < MAX_INSTANCE_MEMBERS; i++) {
        if (members[i] == NULL || IS_NPC(members[i]))
            continue;
        str_replace_dup(&inst->members[inst->member_count],
                        members[i]->name);
        inst->member_count++;
    }

    /* Register in global list */
    LIST2_BACK(inst, global_prev, global_next,
               pd_instance_first, pd_instance_last);

    /* Spawn mobs */
    if (seed->mob_vnum_count > 0 && seed->mob_density > 0) {
        for (i = 1; i < room_count; i++) {
            int mob_count = seed->mob_density;
            int m;
            for (m = 0; m < mob_count; m++) {
                int vnum_idx = (i + m) % seed->mob_vnum_count;
                MOB_INDEX_T *midx = mobile_get_index(seed->mob_vnums[vnum_idx]);
                if (midx != NULL) {
                    CHAR_T *mob = mobile_create(midx);
                    /* Scale to instance level ±2 so there is variance but
                     * fights are never impossible at level 1. */
                    pd_scale_mob_to_level(mob,
                        UMAX(1, inst->level + number_range(-2, 2)));
                    char_to_room(mob, rooms[i]);
                }
            }
        }
    }

    /* Place loot consumables */
    if (seed->item_vnum_count > 0 && seed->loot_density > 0) {
        for (i = 1; i < room_count; i += (2 / seed->loot_density + 1)) {
            int vnum_idx = i % seed->item_vnum_count;
            OBJ_INDEX_T *oidx = obj_get_index(seed->item_vnums[vnum_idx]);
            if (oidx != NULL) {
                OBJ_T *obj = obj_create(oidx, inst->level);
                obj_give_to_room(obj, rooms[i]);
            }
        }
    }

    /* Place the entry/return portal in the first room (permanent, so
     * GATE_PERMANENT keeps it from decaying).  to_vnum points back to the
     * character's original room; that will be set by the caller. */
    {
        OBJ_T *portal = obj_create(obj_get_index(OBJ_VNUM_PORTAL), 0);
        if (portal != NULL) {
            portal->timer = -1;
            SET_BIT(portal->v.portal.gate_flags, GATE_PERMANENT);
            portal->v.portal.to_vnum = ROOM_VNUM_TEMPLE; /* caller overrides */
            str_replace_dup(&portal->short_descr, "a shimmering exit portal");
            str_replace_dup(&portal->name, "portal exit");
            obj_give_to_room(portal, rooms[0]);
        }
    }

    pd_write_snapshot(inst);
    return inst;
}

/* -------------------------------------------------------------------------
 * Snapshot helpers — write/delete json/temp/areas/pd-inst-N.json
 * ---------------------------------------------------------------------- */

/* Recursively freeze an object (and its contents) so obj_update won't
 * extract it.  Used to preserve PC corpses while they are in an instance. */
static void pd_freeze_obj_recursive(OBJ_T *obj)
{
    OBJ_T *child;
    if (obj == NULL)
        return;
    /* Only freeze objects that are actively decaying (timer > 0).
     * Skip already-permanent items (timer <= 0) and already-frozen ones. */
    if (obj->timer > 0 && obj->pd_saved_timer == 0) {
        obj->pd_saved_timer = obj->timer;
        obj->timer = -1;
    }
    for (child = obj->content_first; child != NULL; child = child->content_next)
        pd_freeze_obj_recursive(child);
}

static const char *pd_dir_names[DIR_MAX] = {
    "north", "east", "south", "west", "up", "down"
};

/* Write a single character c to fp, JSON-encoding where necessary. */
static void pd_fputc_json(FILE *fp, char c)
{
    if      (c == '"')  fputs("\\\"", fp);
    else if (c == '\\') fputs("\\\\", fp);
    else if (c == '\n') fputs("\\n",  fp);
    else if (c == '\r') ; /* skip */
    else                fputc(c, fp);
}

static void pd_fputs_json(FILE *fp, const char *s)
{
    if (s == NULL) return;
    for (; *s; s++)
        pd_fputc_json(fp, *s);
}

void pd_write_snapshot(PD_INSTANCE_T *inst)
{
    FILE *fp;
    char filepath[256];
    ROOM_INDEX_T *room;
    int i, first_room, first_exit;

    if (inst == NULL || inst->area == NULL)
        return;

    snprintf(filepath, sizeof(filepath), "%spd-inst-%d.json",
             PD_TEMP_DIR, inst->vnum_slot);

    fp = fopen(filepath, "w");
    if (fp == NULL) {
        bugf("pd_write_snapshot: cannot open '%s' for writing", filepath);
        return;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"slot\": %d,\n",          inst->vnum_slot);
    fprintf(fp, "  \"id\": %d,\n",             inst->id);
    fprintf(fp, "  \"theme\": \"");
    pd_fputs_json(fp, inst->theme ? inst->theme : "");
    fprintf(fp, "\",\n");
    fprintf(fp, "  \"level\": %d,\n",          inst->level);
    fprintf(fp, "  \"created_at\": %ld,\n",    (long)inst->created_at);
    fprintf(fp, "  \"last_empty_at\": %ld,\n", (long)inst->last_empty_at);
    fprintf(fp, "  \"entry_vnum\": %d,\n",     inst->entry_vnum);
    fprintf(fp, "  \"area_name\": \"");
    pd_fputs_json(fp, inst->area_name);
    fprintf(fp, "\",\n");

    fprintf(fp, "  \"members\": [");
    for (i = 0; i < inst->member_count; i++) {
        if (i > 0) fprintf(fp, ", ");
        fprintf(fp, "\"");
        pd_fputs_json(fp, inst->members[i] ? inst->members[i] : "");
        fprintf(fp, "\"");
    }
    fprintf(fp, "],\n");

    fprintf(fp, "  \"rooms\": [\n");
    first_room = 1;
    for (room = inst->area->room_first; room != NULL; room = room->area_next) {
        if (!first_room)
            fprintf(fp, ",\n");
        first_room = 0;

        fprintf(fp, "    {\n");
        fprintf(fp, "      \"vnum\": %d,\n", room->vnum);
        fprintf(fp, "      \"name\": \"");
        pd_fputs_json(fp, room->name);
        fprintf(fp, "\",\n");

        fprintf(fp, "      \"exits\": {");
        first_exit = 1;
        for (i = 0; i < DIR_MAX; i++) {
            if (room->exit[i] != NULL && room->exit[i]->to_room != NULL) {
                if (!first_exit)
                    fprintf(fp, ", ");
                first_exit = 0;
                fprintf(fp, "\"%s\": %d",
                        pd_dir_names[i], room->exit[i]->to_room->vnum);
            }
        }
        fprintf(fp, "},\n");

        /* Count live NPCs in room for the map overlay */
        {
            int mob_count = 0;
            CHAR_T *ch;
            for (ch = room->people_first; ch != NULL; ch = ch->room_next)
                if (IS_NPC(ch))
                    mob_count++;
            fprintf(fp, "      \"mob_count\": %d\n", mob_count);
        }
        fprintf(fp, "    }");
    }
    fprintf(fp, "\n  ]\n");
    fprintf(fp, "}\n");

    fclose(fp);
}

void pd_delete_snapshot(PD_INSTANCE_T *inst)
{
    char filepath[256];
    if (inst == NULL)
        return;
    snprintf(filepath, sizeof(filepath), "%spd-inst-%d.json",
             PD_TEMP_DIR, inst->vnum_slot);
    remove(filepath);
}

void pd_destroy_instance(PD_INSTANCE_T *inst)
{
    AREA_T *area;
    ROOM_INDEX_T *room;
    CHAR_T *mob, *mob_next;
    OBJ_T *obj, *obj_next;

    if (inst == NULL)
        return;
    pd_delete_snapshot(inst);
    area = inst->area;

    if (area != NULL) {
        /* Evacuate any remaining PCs to temple before cleanup */
        for (room = area->room_first; room != NULL; room = room->area_next) {
            CHAR_T *ch, *ch_next;
            for (ch = room->people_first; ch != NULL; ch = ch_next) {
                ch_next = ch->room_next;
                if (!IS_NPC(ch)) {
                    char_from_room(ch);
                    char_to_room(ch, room_get_index(ROOM_VNUM_TEMPLE));
                    send_to_char("The dungeon collapses, sending you back to the temple!\n\r", ch);
                }
            }
        }

        /* Purge all mobs so room_index_dispose does not char_free live chars */
        for (room = area->room_first; room != NULL; room = room->area_next) {
            for (mob = room->people_first; mob != NULL; mob = mob_next) {
                mob_next = mob->room_next;
                if (IS_NPC(mob))
                    char_extract(mob);
            }
        }

        /* Purge all loose objects; area_free will also do this but be explicit */
        for (room = area->room_first; room != NULL; room = room->area_next) {
            for (obj = room->content_first; obj != NULL; obj = obj_next) {
                obj_next = obj->content_next;
                obj_extract(obj);
            }
        }

        /* area_free cascades: frees rooms (via room_index_free), which removes
         * them from the hash table and area list via room_index_dispose. */
        area_free(area);
        inst->area = NULL;
    }

    /* Remove from global list and free */
    pd_instance_free(inst);
}

void pd_update_all(void)
{
    PD_INSTANCE_T *inst, *inst_next;
    long now = (long)time(NULL);
    long timeout_secs = (long)pd_config.empty_timeout_mins * 60L;

    if (!pd_config.autopurge)
        return;

    for (inst = pd_instance_first; inst != NULL; inst = inst_next) {
        inst_next = inst->global_next;

        /* Check if any PCs are still inside */
        bool occupied = FALSE;
        bool has_corpse = FALSE;
        ROOM_INDEX_T *room;
        OBJ_T *cobj;

        if (inst->area != NULL) {
            for (room = inst->area->room_first; room != NULL; room = room->area_next) {
                CHAR_T *ch;
                for (ch = room->people_first; ch != NULL; ch = ch->room_next) {
                    if (!IS_NPC(ch)) {
                        occupied = TRUE;
                        break;
                    }
                }
                /* Scan for PC corpses and freeze them so they don't decay */
                for (cobj = room->content_first; cobj != NULL; cobj = cobj->content_next) {
                    if (cobj->item_type == ITEM_CORPSE_PC) {
                        has_corpse = TRUE;
                        pd_freeze_obj_recursive(cobj);
                    }
                }
            }
        }

        if (occupied) {
            inst->last_empty_at = 0;
        }
        else if (has_corpse) {
            /* Keep the instance alive while a player corpse is present.
             * Reset empty clock so the timeout won't fire until after the
             * last corpse is looted/removed. */
            inst->last_empty_at = 0;
        }
        else {
            if (inst->last_empty_at == 0)
                inst->last_empty_at = now;
            else if (now - inst->last_empty_at >= timeout_secs)
                pd_destroy_instance(inst);
        }
    }
}
