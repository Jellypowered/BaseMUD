# ✅ PLAN COMPLETE — ASCII Automap Integration Plan

**Source:** `Snippets/Pending/asciimap.c` — by mlkesl@stthomas.edu  
**Date:** 2026-04-17

---

## Summary

Integrate the ASCII automap snippet into BaseMUD as a new `map` command skill assigned
to the Ranger class at level 1 (all other classes at level 49). The feature renders a
color-coded text map of surrounding wilderness rooms based on sector type.

---

## Investigation Findings (Summary)

### Sector Types
BaseMUD has 11 sector types (SECT_MAX=11). Missing from BaseMUD vs snippet:
SECT_ROAD, SECT_ENTER, SECT_ROCK_MOUNTAIN, SECT_SNOW_MOUNTAIN, SECT_SWAMP, SECT_JUNGLE, SECT_RUINS.
All missing cases removed from switch statements; default case handles them.

### Type and Function Renames
- `CHAR_DATA*` → `CHAR_T*`
- `ROOM_INDEX_DATA*` → `ROOM_INDEX_T*`
- `EXIT_DATA*` → `EXIT_T*`
- `pexit->u1.to_room` → `pexit->to_room`
- `can_see_room(ch, r)` → `char_can_see_room(ch, r)`
- `room_is_dark(room, ch)` → `room_is_dark(room)` (single parameter)
- `rev_dir[door]` → `REV_DIR(door)`
- `get_room_index(vnum)` → `room_get_index(vnum)`
- `format_string(str)` → `room->description = format_string(room->description)` (must capture return)

### New Flag
`ROOM_WILDERNESS (BIT_21)` — added after `ROOM_NOWHERE (BIT_20)`.

### Skill Map Slot
`SKILL_MAP_MAP = 56`; `SKILL_MAP_MAX` bumped from 56 to 57.

### Skipped Functions
- `do_printmap` — hardcoded char name + fpReserve
- `do_set_wilderness_all` — hardcoded vnum ranges

---

## Files to Modify

### 1. `src/flags.h`
Add `#define ROOM_WILDERNESS (BIT_21)` after `ROOM_NOWHERE`.

### 2. `src/flags.c`
Add `{"wilderness", ROOM_WILDERNESS, TRUE}` to `room_flags[]` after "nowhere".

### 3. `src/defs.h`
- Add `#define SKILL_MAP_MAP  56` before `SKILL_MAP_MAX`
- Change `#define SKILL_MAP_MAX  56` → `57`

### 4. `src/tables.c`
Add `{SKILL_MAP_MAP, "map"}` to `skill_map_table[]` before the terminating `{0}`.

### 5. `src/players.c`
Add `if (learned[SN(MAP)] < 50) learned[SN(MAP)] = 50;` to `player_set_default_skills()`
(Ranger's native skill defaults to 50%; only fires if character's class level allows it.)

### 6. `src/interp.c`
- Add `#include "act_map.h"` in includes
- Register `{"map", do_map, POS_STANDING, 0, LOG_NORMAL, 1}` in player section
- Register `{"smallmap", do_smallmap, POS_DEAD, IM, LOG_NORMAL, 1}` in immortal section

## Files to Create

### 7. `src/act_map.c`
New source file containing adapted functions:
- `#define MAX_MAP 160`, `#define MAX_MAP_DIR 4`
- `static int map[MAX_MAP][MAX_MAP]` — global grid
- `static void MapArea(room, ch, x, y, min, max)` — flood fill
- `static void ShowMap(ch, min, max)` — full map
- `static void ShowHalfMap(ch, min, max)` — compressed map (every 2 cells)
- `static void ShowRoom(ch, min, max)` — small map + room name/desc
- `static const char *get_sector_name(int sector)` — prose sector name
- `static int *get_exit_sectors(int *exit_sectors)` — neighboring sector types
- `static void set_wilderness(ch, argument)` — OLC builder command for default names/descs
- `DEFINE_DO_FUN(do_map)` — with skill check for mortals
- `DEFINE_DO_FUN(do_smallmap)` — immortal compressed map

### 8. `src/act_map.h`
New header declaring `do_map` and `do_smallmap`.

## Files to Modify (JSON/Doc)

### 9. `json/config/skills.json`
Add entry at end of array for "map" with class assignments:
- ranger: level 1, effort 2
- mage/cleric/thief/warrior/druid/paladin/vampire: level 49, effort 5

### 10. `json/help/map.json`
New help entry for MAP / NOMAP.

### 11. `json/help/credits.json`
Update credits for the snippet author.

### 12. `.github/agents/cheatsheet.md`
Record: SKILL_MAP_MAP=56, ROOM_WILDERNESS=BIT_21, sector type list confirmed.

---

## Risks

- `ROOM_WILDERNESS` flag is new — existing area files don't have it set, so `do_map` will
  correctly block mortals in non-wilderness areas until builders set the flag via OLC.
- The 160×160 `map[]` grid is a global `static int` array (102,400 bytes = ~100 KB). 
  Acceptable memory footprint; single-threaded MUD so no race conditions.
- `set_wilderness` uses `free_string`/`str_dup` for name/description modification.
  Must ensure `string.h` is included in `act_map.c` for `format_string`.

---

## Status: IN PROGRESS

## Status: COMPLETED
**Date:** 2026-04-17
