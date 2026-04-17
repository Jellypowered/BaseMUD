# Death Trap, Teleport, Falling Objects Integration Plan

## Overview
Three room-based hazard systems with separate pulse handlers. All use available room flags.

---

## 1. DEATH TRAP (ROOM_DEATHTRAP)

**Room Flag:** `ROOM_DEATHTRAP` (settable=TRUE in flags.c)
**Pulse:** `PULSE_DEATHTRAP = 15 * PULSE_PER_SECOND` (~3.75 seconds)

**Mechanics:**
- Triggers on PCs only (non-immortals)
- Random chance each pulse (player could potentially escape)
- **Tick 1:** Halves HP, changes position to resting, warning message
- **Tick 2:** Reduces HP to 1, stronger warning
- **Tick 3+:** Death (raw_kill)
- Extra description "dt" customizes message per room

**Files to modify:**
- `src/defs.h` - Add PULSE_DEATHTRAP constant
- `src/flags.c` - Renamed ROOM_UNUSED_FLAG_1 → ROOM_DEATHTRAP
- `src/update.c` - Add `dtrap_update()` function + pulse handler
- `src/globals.h` - Declare update function
- No new files needed

---

## 2. TELEPORT ROOM (ROOM_TELEPORT)

**Room Flag:** `ROOM_TELEPORT` (settable=TRUE in flags.c)
**Pulse:** `PULSE_TELEPORT = 20 * PULSE_PER_SECOND` (~5 seconds)
**Room struct field:** `int tele_dest` (stores destination vnum or 0 for random)

**Mechanics:**
- PC-only
- Interrupts combat (forces char_from_room/char_to_room)
- No cooldown/immunity
- random rooms if `tele_dest == 0`; otherwise goes to specified vnum
- Extra description "tele" customizes message per room
- Fast players can move away before trigger

**Files to modify:**
- `src/defs.h` - Add PULSE_TELEPORT constant
- `src/flags.c` - Renamed ROOM_UNUSED_FLAG_2 → ROOM_TELEPORT
- `src/structs.h` - Add `int tele_dest` to ROOM_INDEX_T
- `src/db.c` - Parse tele_dest from area file (reuse zero field)
- `src/update.c` - Add `tele_update()` function + pulse handler
- `src/globals.h` - Declare update function

---

## 3. FALLING OBJECTS (ROOM_FALLING)

**Room Flag:** `ROOM_FALLING_OBJECTS` (settable=TRUE in flags.c)
**Pulse:** `PULSE_FALLING = 30 * PULSE_PER_SECOND` (~7.5 seconds, batches obj_update)
**Air Sectors:** SECT_AIR (value=9) defined in types.h

**Mechanics:**
- Objects in air sectors fall down (exit[5] = DOWN direction)
- Must have DOWN exit to fall
- Display messages when falling
- Example locations:
  - Canyon area (high elevations)
  - Sewer area (Upper levels)
  - Any custom air zones
- Falls instantly to room below; can accumulate if stacked

**Files to modify:**
- `src/defs.h` - Add PULSE_FALLING constant
- `src/flags.c` - Renamed ROOM_UNUSED_FLAG_3 → ROOM_FALLING_OBJECTS
- `src/update.c` - Add `falling_update()` function + pulse handler
- `src/globals.h` - Declare update function

---

## Implementation Order

1. Add flag renames + pulse constants
2. Add struct field (tele_dest)
3. Implement update functions in update.c
4. Add pulse handlers to update_handler()
5. Update area file parser (tele_dest reuse)
6. Test each system independently

---

## Status: COMPLETED (2026-04-16)

All three room hazard systems successfully implemented, tested, and pushed.

**Commit:** 7308b6b "Integrate three room hazard systems: death traps, teleport rooms, and falling objects"
**Build:** Clean compilation with -Werror flag verified
**Files modified:** 11 files, 268 insertions
**Snippets moved:** merc-l.deathtraps.txt, teleport.c, falling.c → Completed/
