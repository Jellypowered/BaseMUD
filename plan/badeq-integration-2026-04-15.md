# PLAN - BadEQ Reset Fixer Integration (2026-04-15)

## Scope
Integrate the BadEQ reset fixer into BaseMUD by implementing `fix_resets()` and wiring it into `boot_db()`. The fixer will detect invalid 'E' equipment resets and correct their wear location argument to a valid slot for the object.

## Files to modify
- `src/db.c`
  - Add `fix_resets()` definition.
  - Call `fix_resets()` in `boot_db()` after `room_link_exits_by_vnum_all()` / `room_fix_two_way_exits_all()` or after `fix_exits()` if needed.
- `json/help/credits.json`
  - Add contributor attribution for the BadEQ reset fixer.
- `.github/agents/cheatsheet.md`
  - Document the new `fix_resets()` helper and its purpose.

## Optional file operations
- Move `Snippets/Pending/BadEQ.c` to `Snippets/Completed/BadEQ.c` once merged.

## Risks / Notes
- Must use BaseMUD helpers such as `obj_index_can_wear_flag()` and `wear_loc_get_flag()` instead of ROM macros.
- The fixer must not crash if it encounters an invalid object or room.
- `reset_commit_all()` runs later, so `fix_resets()` should run after area loading but before reset checking/committing if it operates on the loaded reset structures.

## Status: IN PROGRESS
