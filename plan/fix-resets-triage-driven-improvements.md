# Plan: fix_resets triage-driven improvements

## Scope
Improve BaseMUD's `fix_resets()` logic so its boot-time repair behavior better matches the issues exposed by the MUDEditor triage workflow. This plan is intentionally limited to investigation/design and does not implement code changes yet.

## Problem Summary
Current `fix_resets()` is a coarse fallback:
- It only checks whether the target wear location is compatible with the object's wear flags.
- When invalid, it picks the first allowed slot from a hardcoded priority list.
- It does not account for equip-slot collisions within the owning mob reset chain.
- It does not distinguish stronger intent cases such as `about` vs `cloak`, `body` vs armor/body-only semantics, or conflicts created by another equip reset already using the same slot group.
- It mutates live boot data in memory, but the replacement slot choice is only loosely related to the actual reset context.

The triage page has now shown several concrete failure modes:
- invalid slot but valid alternate slot exists on the same object
- invalid slot because object belongs to a different wear family (`wearcloak` vs `about`)
- candidate slot may still be unusable because the mob reset chain already consumes that slot/group
- paired slot groups (finger/neck/wrist/floating) should be treated differently from single-capacity groups
- some resets use cross-area object references, which exposes the need for better source-context reasoning when reporting/fixing

## Goals
1. Make `fix_resets()` choose a better fallback slot when it has to rewrite an invalid equip reset.
2. Prefer replacements that do not create immediate equip collisions within the owning mob block.
3. Preserve useful logging so the editor and humans can still identify what happened.
4. Keep boot resilient: if no safe slot exists, continue to degrade gracefully.

## Proposed Design

### 1. Factor slot selection into a helper
Create a dedicated helper for replacement wear-slot selection, for example:
- `static int fix_reset_choose_wear_loc(const RESET_T *equip_reset, const OBJ_INDEX_T *obj_index, const RESET_T *owner_reset)`

Responsibilities:
- Inspect object wear flags.
- Inspect sibling equip resets belonging to the same owning mob block.
- Prefer allowed slots that are not already occupied by another equip reset in the same slot group.
- Return `WEAR_LOC_NONE` / `WEAR_LOC_LIGHT` only as a last resort when no equip-capable solution exists.

### 2. Add mob-block-aware collision avoidance
Reuse the same conceptual grouping learned from triage:
- single-capacity groups: body, head, legs, feet, hands, arms, shield, about, waist, wield, hold, tail, back, cloak, eyes, ears, tattoo, light
- paired groups with limit 2 total: finger, neck, wrist, floating

For a candidate slot, scan forward/backward only within the owning mob block:
- find the nearest preceding `M` reset (already partly done for logging)
- scan subsequent resets until the next `M` or end of room reset chain
- count occupied slots/group usage by `E` resets in that block
- prefer unused single slots or under-capacity paired groups

### 3. Replace hardcoded fallback order with structured candidate ordering
Instead of one long `else if` chain, build candidate slots in priority order based on the object's actual wear flags.

Suggested policy:
- first: exact-family replacements tied to the object's available wear flags
- second: alternate slot within same paired family (`neck1/neck2`, `lfinger/rfinger`, etc.)
- third: other allowed slots only if they are semantically valid for the object
- final fallback: `WEAR_LOC_LIGHT` for light items with `ITEM_TAKE`, otherwise `WEAR_LOC_NONE`

This keeps behavior deterministic while still being more context-aware.

### 4. Improve logging to capture reason for chosen replacement
Extend the enriched log line to include the selected replacement wear location, for example:
- original invalid slot
- replacement slot chosen
- whether the choice avoided a collision or fell back to `none`

Example shape:
- `Warning: 'E' for object ... into unequippable location 'about' in room 7900, owned by mob 7900 (...). Reassigned to 'cloak'.`
- `Warning: ... no collision-free wear slot found; reassigned to 'none'.`

This would let triage distinguish:
- source-data-invalid-only
- invalid-and-auto-rewritten-cleanly
- invalid-and-still-degraded

### 5. Consider separating validation from mutation
Potential future split:
- `reset_equip_find_issue(...)`
- `reset_equip_choose_fix(...)`
- `reset_equip_apply_fix(...)`

That would make the behavior easier to unit-test and easier for the editor to mirror exactly.

## Files Likely To Modify In Implementation
1. `f:\Source\BaseMUD\src\db.c`
- Refactor `fix_resets()`
- Add helper(s) for owner mob block lookup and candidate slot selection
- Extend logging

2. Possibly supporting headers if helpers should be shared, though keeping them `static` in `db.c` is preferred unless reuse is required.

## Validation Plan
If implemented later:
1. Build BaseMUD with workspace build task.
2. Boot with known problematic resets (`about` vs `wearcloak`, `head` vs `body`, `hold`/`light`, paired-slot cases).
3. Verify logs show:
- room and owner mob context
- chosen replacement slot
- collision-aware decisions where relevant
4. Confirm the MUDEditor triage page can still locate the source reset from the enriched logs.

## Risks
- Behavior changes at boot may alter what equipment mobs actually spawn with in legacy areas.
- Collision avoidance logic could mask content errors too aggressively if it silently picks different but technically valid slots.
- If we mirror too much editor-side logic in C without shared abstractions, server/editor drift remains possible.
- Cross-area object references may still need special handling in log output if source data mixes local and external objects.

## Open Questions
1. Should `fix_resets()` prefer preserving apparent content intent (e.g. cloak-like items -> cloak) over simply avoiding collisions?
2. When every allowed slot is already occupied, should the function prefer `give`, `none`, or leave the reset unchanged and only log?
3. Should logs explicitly mark whether the fix was collision-free vs degraded fallback?
4. Do we want MUDEditor logic to become the behavioral spec, or should the editor be updated to mirror a new authoritative C-side algorithm?
