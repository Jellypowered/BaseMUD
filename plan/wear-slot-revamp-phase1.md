# Phase 1: C Code Implementation — Wear Slot Revamp

## Overview
Add new wear location constants, item types, and the back-container exemption logic. This phase implements the server-side model that subsequent phases (JSON config, area migrations, MUDEditor) depend on.

## New Wear Locations
- **WEAR_LOC_BACK** = 20 (back slot for containers/backpacks)
- **WEAR_LOC_CLOAK** = 21 (outer garments, cloaks, capes)
- **WEAR_LOC_EYES** = 22 (glasses, goggles, spectacles, monocles, patches)
- **WEAR_LOC_EARS** = 23 (earrings, ear cuffs)
- **WEAR_LOC_FLOAT_2** = 24 (second floating item, for orbs/globes/discs)
- **WEAR_LOC_TATTOO** = 25 (tattoo slot for stat-bearing marks)

WEAR_LOC_MAX changes from 20 to 26.

## New Item Type
- **ITEM_TATTOO** = 35 (treated like ITEM_TREASURE but uses tattoo slot; can be worn and give affects/spells)

ITEM_MAX changes from 35 to 36.

## New Wear Flags
Use available BIT numbers (BIT_20 through BIT_25 are free in wear context):
- **ITEM_WEAR_BACK** = BIT_20
- **ITEM_WEAR_CLOAK** = BIT_21
- **ITEM_WEAR_EYES** = BIT_22
- **ITEM_WEAR_EARS** = BIT_23
- **ITEM_WEAR_FLOAT_2** = BIT_24
- **ITEM_WEAR_TATTOO** = BIT_25

## Carry Capacity Baseline
Create a new constant to decouple carry-count cap from WEAR_LOC_MAX:
- **BASE_CARRY_COUNT_CAP** = 20 (the current WEAR_LOC_MAX, preserved for backwards compatibility)

This ensures adding 6 new wear slots doesn't automatically grant +6 to every character's inventory cap.

## Back-Container Exemption Logic
When a container is worn on WEAR_LOC_BACK:
- The container's own weight counts toward carry weight
- The container's own size/count counts toward inventory limits
- **Items inside the worn back container do NOT count toward carry weight or item count**

This applies only when the container is actively worn on the back; if dropped or unequipped, contents count normally.

### Implementation Strategy for Exemption
1. Add a function `is_item_exempt_from_carry(const OBJ_T *obj, const CHAR_T *ch)` that checks:
   - Is `obj` inside a container? → get the container
   - Is the container worn on `WEAR_LOC_BACK` by this character? → return TRUE
   - Is the back-worn container a `ITEM_CONTAINER`? → return TRUE (only containers qualify)
2. Update carry-count and carry-weight calculations to check exemption before adding an item
3. Update equip/unequip flows to recalculate carry limits when back item changes

## Files to Modify

### 1. src/types.h
- Add WEAR_LOC_BACK through WEAR_LOC_TATTOO (20-25)
- Update WEAR_LOC_MAX to 26
- Add ITEM_TATTOO = 35
- Update ITEM_MAX to 36

### 2. src/types.c
- Add `{"tattoo", ITEM_TATTOO, TRUE}` entry to `item_types[]` table

### 3. src/flags.h
- Add ITEM_WEAR_BACK through ITEM_WEAR_TATTOO (BIT_20–BIT_25)

### 4. src/flags.c
- Add new wear flag entries to `wear_flags[]` table:
  - `{"wearback", ITEM_WEAR_BACK, TRUE}`
  - `{"wearcloak", ITEM_WEAR_CLOAK, TRUE}`
  - `{"weareyes", ITEM_WEAR_EYES, TRUE}`
  - `{"wearears", ITEM_WEAR_EARS, TRUE}`
  - `{"wearfloat2", ITEM_WEAR_FLOAT_2, TRUE}`
  - `{"weartattoo", ITEM_WEAR_TATTOO, TRUE}`

### 5. src/tables.c
- Add 6 rows to `wear_loc_table[]` for the new slots with appropriate messages and AC bonus values:
  - back: 100 AC bonus (weight penalty for backpack)
  - cloak: 200 AC bonus (like ABOUT)
  - eyes: 0 AC bonus (no armor value)
  - ears: 0 AC bonus (no armor value)
  - float_2: 0 AC bonus (like first float)
  - tattoo: 0 AC bonus (cosmetic/stat-only)

### 6. src/chars.c
- Define `#define BASE_CARRY_COUNT_CAP 20` (decouple from WEAR_LOC_MAX)
- Update `char_get_max_carry_count()` to use BASE_CARRY_COUNT_CAP instead of WEAR_LOC_MAX
- Add `is_item_exempt_from_carry()` helper function
- Update any code that iterates over wear slots or checks WEAR_LOC_MAX to handle new slots correctly
- Add back-container exemption checks in carry weight/count accounting

### 7. src/items.c
- Add tattoo to the no-values item type map (if it exists and allows for custom behavior)
- Ensure back-worn containers remain container-type only (add validation if needed)

### 8. src/act_info.c
- Update equipment display to show new wear slots in proper order

### Note on JSON Files
- Do NOT edit JSON files in this phase
- Phase 2 will migrate `json/config/wear_locs.json`, wear_flags, and item_types
- Phase 3 will handle area-specific migrations

## Verification Goals
- C code compiles without warnings
- `WEAR_LOC_MAX` correctly reflects 26 slots
- `BASE_CARRY_COUNT_CAP` is used in carry calculations
- Character equipment display shows all 6 new slots
- Back-container exemption logic doesn't break existing equip/unequip flows

## Status
**Draft** — Ready for implementation once user confirms Phase 1 approach.
