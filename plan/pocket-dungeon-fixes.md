# Plan: Pocket Dungeon — Map View, Scaling, Re-entry, Corpse Hold

## Scope

Four issues reported after live testing:

1. **Map view** — Instances tab shows rooms table but not the area minimap.
2. **Mob scaling** — Mobs use their template level (too high for level 1); need level scaling.
3. **Re-entry** — Player who died inside cannot rejoin to retrieve their corpse.
4. **Corpse hold** — Instance should not auto-purge while a PC corpse is present; the corpse itself must not decay while in-instance.

---

## Files to Modify

### C — BaseMUD

| File | Change |
|---|---|
| `src/pocket_dungeon.c` | `pd_write_snapshot`: add `mob_count` per room. Add `pd_scale_mob_to_level()` helper; call in mob-spawn loop. Extend `pd_update_all`: scan for PC corpses → freeze timers + block purge. |
| `src/pocket_dungeon.h` | Declare `pd_find_instance_by_member()`. |
| `src/act_instance.c` | Add `dungeon rejoin` subcommand using `pd_find_instance_by_member()`. |
| `src/structs.h` | Add `has_pc_corpse: bool` field to `struct pd_instance` (optional flag cached each update tick — avoids rescanning in snapshot). |

### MUDEditor — Client

| File | Change |
|---|---|
| `web/shared/types/index.ts` | Add `mob_count: number` to `PocketDungeonInstanceRoom`. |
| `web/client/src/pages/PocketDungeonPage.tsx` | Add `convertInstanceToFullArea()` helper; replace rooms table with `<AreaMiniMap>` (read-only, no room selection action). Add `selectedVnum` state so clicking a room highlights it and shows its name in the detail panel. |

---

## Detailed Design

### 1. Map View

**Problem:** `AreaMiniMap` expects a `FullArea` (with `rooms[].anum` as area-local offset and `doors[].to` as anum). The snapshot delivers vnums. Rooms count shows 0 because the Rooms table rendered `selected.rooms.length` which was populated, but the visual was a plain table — not a map.

**Fix (client-only):**
- Add `convertInstanceToFullArea(inst: PocketDungeonInstance): FullArea` inside `PocketDungeonPage.tsx`:
  - `anum = room.vnum - inst.entry_vnum`
  - `doors = Object.entries(room.exits).map(([dir, destVnum]) => ({ dir, to: destVnum - inst.entry_vnum }))`
  - `sector_type = 'inside'` for all rooms
  - If `room.mob_count > 0`: add `resets: [{ command: 'mobile', values: {} }]` so map colors the room orange
- Replace the `<table>` with `<AreaMiniMap area={syntheticArea} selectedAnum={selectedAnum} onSelectRoom={setSelectedAnum} />` in a fixed-height container

**C side (snapshot):**
- In `pd_write_snapshot`, for each room, count live `CHAR_T` NPCs present: `"mob_count": N`

### 2. Mob Scaling

**Problem:** `mobile_create()` stamps `mob->level = mob_index->level`. Template mobs may be level 10+. Player enters at level 1 → instant death.

**Fix:**
- Add `static void pd_scale_mob_to_level(CHAR_T *mob, int target_level)` in `pocket_dungeon.c`:
  - If `target_level == mob->level` → no-op
  - Set `mob->level = target_level`
  - Recompute `mob->max_hit` using old-format level formula: `(level*8 + number_range(level*level/4, level*level)) * 9/10`
  - Set `mob->hit = mob->max_hit`
  - Recompute `mob->hitroll = target_level / 5`
  - Recompute `mob->damroll = target_level / 4`
  - Recompute `mob->armor[i]` via `int_interpolate(level, 100, -100)` for i < 3; `armor[3] = int_interpolate(level, 100, 0)`
  - Recompute stats: `perm_stat[i] = UMIN(25, 11 + level/4)`
- Call `pd_scale_mob_to_level(mob, inst->level)` immediately after `char_to_room(mob, rooms[i])` in the spawn loop

### 3. Re-entry (dungeon rejoin)

**Problem:** Player who died was ejected to the temple. `pd_find_instance_for_char()` returns NULL (correct — they're not in the instance's rooms). But `dungeon enter` creates a new instance. There's no way to go back to their existing one.

**Fix:**
- Add `PD_INSTANCE_T *pd_find_instance_by_member(const char *name)` to `pocket_dungeon.c`:
  - Iterates `pd_instance_first`; checks `inst->members[i]` for a case-insensitive match
  - Returns first match
- Declare in `pocket_dungeon.h`
- Add `dungeon rejoin` in `act_instance.c`:
  - Calls `pd_find_instance_by_member(ch->name)`
  - If found and `inst->area != NULL`: teleport `ch` to `inst->entry_vnum`
  - If already inside: "You are already inside a pocket dungeon."
  - If no instance found: "You have no dungeon to rejoin."

### 4. Corpse Hold + Freeze Decay

**Problem A:** Empty instance has a PC corpse → instance should not auto-purge until corpse is gone.
**Problem B:** PC corpse timer counts down → disappears before player can retrieve it.

**Fix — `pd_update_all`:**
After existing "occupied" scan, add a second scan for PC corpses:

```c
bool has_corpse = FALSE;
OBJ_T *cobj;
for (room = inst->area->room_first; ...) {
    for (cobj = room->content_first; cobj != NULL; cobj = cobj->content_next) {
        if (cobj->item_type == ITEM_CORPSE_PC) {
            /* Freeze decay: set timer to -1 so obj_update skips it */
            cobj->timer = -1;
            has_corpse = TRUE;
        }
    }
}
```

In the empty-timeout logic:
```c
if (occupied || has_corpse) {
    if (!occupied)   /* keep alive but reset empty clock */
        inst->last_empty_at = 0; 
} else {
    /* normal timeout path */
}
```

Note: `timer = -1` is safe because `obj_update` checks `if (obj->timer > 0 ...)` — negative values are skipped. When the instance eventually destroys (all corpses looted + players left), `area_free()` cascades `obj_extract()` on all remaining objects — the freeze is irrelevant at that point.

---

## Risks / Dependencies

- `pd_scale_mob_to_level` must be called AFTER `mobile_create` AND `char_to_room` so it doesn't interfere with position setup. Actually can be called before `char_to_room` — just after `mobile_create`.
- `AreaMiniMap` uses `computeLayout` which BFS from lowest anum. Since our anums are 0..N-1, this is well-ordered.
- `AreaMiniMap` has a lazy-load toggle (`loaded` state, set by IntersectionObserver or a manual "load map" button). Worth checking whether the map renders on first show or needs a trigger.
- `ITEM_CORPSE_PC` is defined in `defs.h` line 265 (vnum 11) → `item_type == ITEM_CORPSE_PC` is the correct check (same used in `fight.c` and `act_skills.c`).

---

## Out of Scope

- Changing the seed `mob_density` field semantics (value stays as "mobs per room")
- Adding new help entries for `dungeon rejoin` (covered in Phase 7)

---

## Deviations from Plan

- `structs.h` field `has_pc_corpse` was listed as optional — not added. Equivalent logic achieved inline in `pd_update_all` scan loop via `pd_freeze_obj_recursive()` helper.
- Freeze is applied recursively to corpse contents (added `pd_freeze_obj_recursive()`) which was stronger than the plan's flat freeze.

## Status: COMPLETED (2025-07-18)
- Full mob stat sheet in the Instances map tooltip

## Status: PENDING
