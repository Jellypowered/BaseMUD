# Extra Exits (NE/NW/SE/SW) Implementation Plan

Source: `Snippets/Pending/extraexits.c` (ROM 2.4 diagonal exit snippet by Carnage)

## Overview

Add four diagonal cardinal directions — northeast (NE), northwest (NW), southeast (SE), southwest (SW) — as fully functional exits (DIR_NE=6, DIR_NW=7, DIR_SE=8, DIR_SW=9), updating DIR_MAX from 6 to 10.

BaseMUD uses table-driven direction lookups, so most code that loops `for (i = 0; i < DIR_MAX; i++)` or calls `door_lookup()`/`door_get()` automatically handles new directions once the table and constants are updated.

---

## Files to Modify

### BaseMUD (C)

| File | Change |
|------|--------|
| `src/defs.h` | Add DIR_NE=6, DIR_NW=7, DIR_SE=8, DIR_SW=9; change DIR_MAX 6→10 |
| `src/tables.c` | Add 4 entries to `door_table[ ]` (NE, NW, SE, SW with correct reverses) |
| `src/act_move.c` | Add `do_northeast`, `do_northwest`, `do_southeast`, `do_southwest` |
| `src/act_move.h` | Add 4 `DECLARE_DO_FUN` declarations |
| `src/interp.c` | Add command table entries: ne/nw/se/sw + full-word aliases |
| `src/olc_redit.c` | Add 4 `REDIT` handlers for new directions |
| `src/olc_redit.h` | Add 4 `REDIT` forward declarations |
| `src/olc.c` | Add redit_table entries for ne/nw/se/sw (abbrev + full word) |
| `src/pocket_dungeon.c` | Extend `pd_dir_names[DIR_MAX]` with 4 new names |

### JSON / Data

| File | Change |
|------|--------|
| `json/config/doors.json` | Add entries for dir 6–9 (NE/NW/SE/SW with from/to phrases and reverses) |

### Documentation

| File | Change |
|------|--------|
| `doc/Json_Documentation.md` | Update the `dir` field enum list to include diagonal directions |

### MUDEditor (TypeScript) — Full Audit

MUDEditor already has all 10 directions in shared types, parsers, and map layout. Only the flag arrays need syncing.

| File | Change |
|------|--------|
| `web/client/src/components/FlagsField.tsx` | Add missing flags and tooltips (see below) |
| `web/client/src/hooks/useFlagsConfig.ts` | Sync FALLBACK arrays to match FlagsField.tsx |

#### FlagsField.tsx gaps (vs BaseMUD source truth):
- **ROOM_FLAGS** missing: `deathtrap`, `teleport`, `falling`
- **MOB_FLAGS** missing: `banker`, `atm` (after `noquest`), `restringer` (after `indoors`)
- **EXTRA_FLAGS** missing: `hidden`, `unidentified`
- **FLAG_TIPS** missing tooltips for all new flags above

#### useFlagsConfig.ts FALLBACK gaps:
- **room_flags** missing: `deathtrap`, `teleport`, `falling`
- **mob_flags** missing: `cursed`, `banker`, `atm`, `restringer`
- **extra_flags** missing: `hidden`, `unidentified`

---

## Diagonal Reverse Mappings

| Direction | Index | Reverse |
|-----------|-------|---------|
| NE | 6 | SW (9) |
| NW | 7 | SE (8) |
| SE | 8 | NW (7) |
| SW | 9 | NE (6) |

---

## Notes / Risks

- `door_table` is a sentinel-terminated static array sized `[DIR_MAX + 1]`. Increasing DIR_MAX automatically resizes it.
- `JSON_TBLR_START(DOOR_T, door, DIR_MAX, ...)` also uses DIR_MAX so the loader cap grows automatically.
- `exit[DIR_MAX]` in `struct room_index_data` auto-expands when DIR_MAX changes.
- `pocket_dungeon.c` has `pd_dir_names[DIR_MAX]` initialized with only 6 strings — must add 4 more to avoid partial-init (undefined elements mean NULL, which would crash if used).
- `update.c` uses `exit[5]` (hardcoded DIR_DOWN = 5) — no change needed; DOWN remains 5.
- MUDEditor direction handling (types, parser, exporter, map layout, room editor) is already complete for all 10 directions.
- `mobile_wander()` uses `number_bits(5)` (0–31) and checks `door < DIR_MAX` — with DIR_MAX=10, mobs will also wander diagonally when such exits exist, which is correct behavior.

## Status: COMPLETED
