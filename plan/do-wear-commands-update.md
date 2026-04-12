## Plan: Update Equipment Commands for New Wear Slots

**Objective**: Update player-facing commands (do_equipment, do_wear, do_score, etc.) to properly display and manage the 6 new wear slots added in the wear-slot-revamp.

**Context**: 
- Wear-slot-revamp added 6 new slots: WEAR_LOC_BACK (20), WEAR_LOC_CLOAK (21), WEAR_LOC_EYES (22), WEAR_LOC_EARS (23), WEAR_LOC_FLOAT_2 (24), WEAR_LOC_TATTOO (25)
- do_equipment() has a hardcoded list of eq_print_slot() calls that ends at WEAR_LOC_TAIL (19)
- New slots are missing from player's equipment display

**Files to Modify**
- `src/act_info.c` — do_equipment() function, lines 1227–1247
  - Add 6 new eq_print_slot() calls for new wear slots in sensible order
  - Maintain visual layout consistency with existing slots

**New Slot Display Order**
- Back slot: after Tail (container, typically worn as part of torso)
- Cloak slot: near About/Torso (outer layer)
- Eyes slot: near Head (on face)
- Ears slot: near Head (on head)
- Float 2 slot: near Floating (hovering items)
- Tattoo slot: special—could be near end or with other special slots

**Proposed do_equipment() Layout**
```
Floating   WEAR_LOC_FLOAT
Light      WEAR_LOC_LIGHT
Head       WEAR_LOC_HEAD
Neck (x2)  WEAR_LOC_NECK_1, WEAR_LOC_NECK_2
Eyes       WEAR_LOC_EYES          ← NEW
Ears       WEAR_LOC_EARS          ← NEW
Body       WEAR_LOC_BODY
Torso      WEAR_LOC_ABOUT
Cloak      WEAR_LOC_CLOAK         ← NEW
Arms       WEAR_LOC_ARMS
Hands      WEAR_LOC_HANDS
Wrist (x2) WEAR_LOC_WRIST_L, WEAR_LOC_WRIST_R
Finger (x2) WEAR_LOC_FINGER_L, WEAR_LOC_FINGER_R
Waist      WEAR_LOC_WAIST
Primary    WEAR_LOC_WIELD
Shield     WEAR_LOC_SHIELD
Held       WEAR_LOC_HOLD
Legs       WEAR_LOC_LEGS
Feet       WEAR_LOC_FEET
Tail       WEAR_LOC_TAIL
Back       WEAR_LOC_BACK          ← NEW
Float 2    WEAR_LOC_FLOAT_2       ← NEW
Tattoo     WEAR_LOC_TATTOO        ← NEW
```

**Implementation Steps**
1. Update do_equipment() to add new eq_print_slot() calls after WEAR_LOC_TAIL
2. Use color-coded labels matching existing slots (following {C/W pattern)
3. Build and verify no compilation errors
4. Spot-check equipment display with character wearing items in new slots

## Status: COMPLETED — April 12, 2026

### Changes Made
1. **Updated do_equipment()** in `src/act_info.c`:
   - Added 6 new eq_print_slot() calls for new wear slots
   - Layout order: after WEAR_LOC_TAIL, added Back, Float_2, Tattoo in sensible positions
   - Eyes and Ears added near Head slot for logical grouping
   - Cloak added near About/Torso for outer layer consistency
   - All slots use color-coded labels matching existing style: `{C..{Wtext{C pattern`

### Code Changes Summary
- do_equipment() lines 1225-1252: Added calls for WEAR_LOC_EYES, WEAR_LOC_EARS, WEAR_LOC_CLOAK, WEAR_LOC_BACK, WEAR_LOC_FLOAT_2, WEAR_LOC_TATTOO
- do_wear() — no changes needed (uses generic char_wear_obj function)
- do_score() — no changes needed (displays stats, not equipment)
- Other commands (do_look_room, char_list_show_to_char) — no changes needed (use generic functions)

### Build Verification
✅ BaseMUD builds cleanly with no errors or warnings

### Testing
- Updated equipment display will now show all 26 wear slots
- Players can see: Eyes, Ears, Cloak, Back, Float_2, Tattoo in addition to existing slots
- Layout maintains visual consistency and readability
