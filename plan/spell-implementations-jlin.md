# Implementation Plan: 20 Missing Spells from John Lin

**Status**: Planning Phase

---

## Overview

This plan covers implementation of 20 missing wizard spells from the John Lin spell list. 36 spells already exist in BaseMUD; these 20 are new additions that will be implemented in three tiers of complexity.

---

## Slot Number Assignments

**New spell slots to be assigned (500+ range to avoid conflicts with stock merc slots):**

| Spell Name             | Slot | Category      |
|------------------------|------|---------------|
| Detect Undead          | 525  | Detection     |
| Find Familiar          | 526  | Summoning     |
| Light                  | 527  | Utility       |
| Mount                  | 528  | Summoning     |
| Deafness               | 529  | Debuff        |
| Glitterdust            | 530  | Utility/Offensive |
| Locate Person          | 531  | Detection     |
| Magic Mouth            | 532  | Trap/Alert    |
| Acid Arrow             | 533  | Offensive     |
| Explosive Runes        | 534  | Trap          |
| Flame Arrow            | 535  | Offensive     |
| Mass Invisibility      | 536  | Group Buff    |
| Protection from Evil   | 537  | Buff          |
| Protection from Good   | 538  | Buff          |
| Vampiric Touch         | 539  | Drain         |
| Stoneskin              | 540  | Buff          |
| Animate Dead           | 541  | Summoning     |
| Legend Lore            | 542  | Detection     |
| Demon Fire             | 543  | High-level Offensive |
| Wish                   | 544  | Godly/Special |

---

## Files to Be Modified

### 1. **json/config/skills.json**
- **Change**: Add 20 new spell entries
- **Location**: Maintain alphabetical order within the file
- **Structure for each spell**:
  ```json
  {
    "skill": {
      "name": "<spell_name>",
      "classes": {
        "mage": { "level": <int>, "effort": <int> },
        "cleric": { "level": <int>, "effort": <int> },
        "thief": { "level": <int>, "effort": <int> },
        "warrior": { "level": <int>, "effort": <int> }
      },
      "target": "<target_type>",
      "min_position": "standing|fighting",
      "slot": <int>,
      "min_mana": <int>,
      "usage_beats": <int>,
      "off_msg_char": "!<Spell Name>!",
      "spell_fun": "spell_<name_lowercase>"
    }
  }
  ```
- **Default class levels**: Mage 99 (primary), Cleric 99, Thief 99, Warrior 99 (all unassigned; user assigns later via editor)
- **Risks**: File may be truncated; verify closing `}` and `]` after edit.

### 2. **src/magic.h**
- **Change**: Add `DECLARE_SPELL_FUN` declarations for all 20 new spells
- **Location**: After existing spell declarations
- **Format**: `DECLARE_SPELL_FUN(spell_<name>);`

### 3. **src/spell_off.c** (Offensive/Damage Spells)
- **Spells to add**:
  - `spell_acid_arrow()` — 2d4 + level damage, may damage equipment
  - `spell_flame_arrow()` — 4d6 damage
  - `spell_demon_fire()` — High-level damage (300-500 range random)
- **Pattern**: Use `damage_visible()`, check saves with `saves_spell()`, use `act3()` for messages
- **New includes needed**: None (existing headers sufficient)

### 4. **src/spell_aff.c** (Affection/Buff Spells)
- **Spells to add**:
  - `spell_light()` — Creates room light; use AFF flag or room flag (check Continual Light pattern)
  - `spell_deafness()` — Applies AFF_DEAF; blocks say/tell/whisper (player sees emotes only)
  - `spell_glitterdust()` — Reveals invisibles; may blind victims (1d4 ticks) if save fails
  - `spell_protection_from_evil()` — Reduces damage from evil creatures to 90%
  - `spell_protection_from_good()` — Reduces damage from good creatures to 90%
  - `spell_stoneskin()` — Self-only AC -20
  - `spell_mass_invisibility()` — Affects all creatures in room
- **Pattern**: Use `affect_init()`, `affect_copy_to_char()`, check `affect_is_char_affected_with_act()` for duplicates
- **Flags needed**: Check if AFF_DEAF exists; if not, add to `src/flags.h`

### 5. **src/spell_misc.c** (Miscellaneous Spells)
- **Spells to add**:
  - `spell_detect_undead()` — Detects undead in room; message-based
  - `spell_locate_person()` — Finds player location; check `no summon` flag
  - `spell_legend_lore()` — Advanced identify; outputs complete item metadata
- **Pattern**: Direct detection/search logic (similar to existing detect spells)
- **Considerations**: Locate Person must verify `no summon` flag on target

### 6. **src/spell_summon.c** (or new file; summon-related spells)
- **Spells to add**:
  - `spell_mount()` — Summons mount based on caster level (Pony → Elephant/Griffin)
  - `spell_find_familiar()` — Complex; summons pet with AC bonus, special powers, HP contribution, CON penalty on death
  - `spell_animate_dead()` — Summons zombie from corpse in room
- **Complexity**: These require mob creation, level-based selection, special handling for familiar death
- **Risks**: Need to verify mob vnum availability and pet tracking mechanism

### 7. **src/spell_trap.c** (or spell_misc.c; trap spells)
- **Spells to add**:
  - `spell_magic_mouth()` — Creates alarm trigger; warns when enemies 4 rooms away (or configurable distance)
  - `spell_explosive_runes()` — Creates trap on object/floor; detonates (4d6+6) when read
- **Complexity**: Requires persistent trap object or room flag mechanism
- **Risks**: Need to determine how traps are stored/triggered

### 8. **src/spell_dispatch.c**
- **Change**: Register all 20 new spell functions in `spell_dispatch_table[]`
- **Format**: 
  ```c
  {"spell_<name>", spell_<name>},
  ```
- **Location**: Maintain alphabetical order

### 9. **src/spell_off.h** (if needed)
- **Change**: May need to add new damage type constants if new damage types used (e.g., DAM_VAMPIRIC)

### 10. **src/flags.h & src/flags.c**
- **Change**: If new AFF flags needed (e.g., AFF_DEAF, AFF_MAGIC_MOUTH):
  - Add flag to `enum affect_bits` in `src/flags.h`
  - Register in `flag_table[]` in `src/flags.c` with display name
- **Authority**: C is authoritative for AFF flags; `json/meta/flags/` is generated export

---

## JSON Help Entries (json/help/)

- **File**: Create/update relevant `.json` help files in `json/help/`
- **Format**: Array of `{"help": {...}}` objects with `keyword`, `text`, optional `hide_keywords`
- **Entries needed**:
  - One per spell: keyword = spell name (uppercase), secondary = "NO<SPELL_NAME>"
  - Help text should describe effect, duration, saving throw if applicable, special notes
  - Use `|` delimiters for multi-line text

---

## JSON Documentation

- **File**: `doc/Json_Documentation.md`
- **Change**: If new AFF flags added, document in the flags section
- **Update**: Any affected skill/type table sections if applicable

---

## Credits Update

- **File**: `json/help/credits.json`
- **Change**: Add entry crediting John Lin for the spell collection
- **Format**: 
  ```json
  "feature": "20 wizard spells (Detect Undead, Find Familiar, Light, Mount, etc.)",
  "contributor": "John Lin"
  ```

---

## Cheatsheet Update

- **File**: `.github/agents/cheatsheet.md`
- **Change**: Add section documenting:
  - New slot numbers (525–544)
  - New AFF flags (if any)
  - Unit test patterns expected for complex spells (Find Familiar, Mount, Animate Dead)
  - Notes on Light vs Continual Light distinction

---

## Implementation Tiers & Risks

### **Tier 1: Simple Damage Spells** (Low Risk)
**Spells**: Acid Arrow, Flame Arrow
**Effort**: ~30 min each
**Risk**: Low — similar to existing spells (acidblast, fireball)
**Spike damage tracking**: Confirm equipment damage mechanic exists

### **Tier 2: Detection & Buff Spells** (Medium Risk)
**Spells**: Detect Undead, Deafness, Glitterdust, Protection from Evil/Good, Stoneskin, Mass Invisibility
**Effort**: ~1 hour total
**Risk**: Medium — requires AFF flag management, deafness requires communication hook, mass invisibility requires room iteration
**Deafness complication**: Must verify player comm channels respect AFF_DEAF flag
**Protection complexity**: Damage reduction calc integrates into fight.c combat loop

### **Tier 3: Complex Mechanics** (High Risk)
**Spells**: Mount, Find Familiar, Animate Dead, Explosive Runes, Magic Mouth, Legend Lore, Wish
**Effort**: ~3–4 hours total
**Risk**: High
- **Mount/Familiar/Animate Dead**: Require mob creation, vnum validation, pet tracking
- **Find Familiar**: CON penalty on death is destructive (permanent effect); needs careful rollback design
- **Explosive Runes/Magic Mouth**: May require new object flags or room trigger mechanism not yet in place
- **Legend Lore**: Must enumerate all object properties; scope depends on object model complexity
- **Wish**: Needs immortal-level confirmation or random effect table; scope undefined

**Mitigation**:
- Verify pet/summon tracking exists in codebase before implementing Mount/Familiar
- Test Familiar death scenario manually
- Confirm trap mechanism (runes, mouth) integration point before coding
- Define Wish effect table with user before final implementation

---

## Build & Verification Strategy

1. **Compile all new .c code** with `make` before JSON updates
2. **Verify spell_dispatch_table entries** register correctly
3. **Test each spell in-game**: cast, save, effect, wear-off (if applicable)
4. **Test JSON loading**: Verify skills.json reloads on boot without truncation
5. **Integration test**: Verify spell in magical objects (wands, scrolls, potions)

---

## Known Assumptions & Dependencies

- **Familiar mechanic**: Assume pet/companion system exists or will be created for Find Familiar
- **Equipment damage**: Assume item degradation system exists for Acid Arrow / Flame Arrow
- **Deafness channels**: Assume communication channels (say, tell, whisper) check AFF_DEAF flag
- **Trap persistence**: Assume object/room flags can track active traps (Magic Mouth, Explosive Runes)
- **Demon Fire damage**: Assume random() function available for 300–500 range

---

## Rollback Notes

If implementation for a tier fails during build or testing:

1. **Tier 1 (Damage)**: Revert json/config/skills.json entries, remove spell_off.c functions, remove declarations from magic.h, remove dispatcher entries
2. **Tier 2 (Buffs)**: Same as Tier 1 plus revert any AFF flag additions to flags.h/flags.c
3. **Tier 3 (Complex)**: Same as above; additionally verify no persistence (pet objects, trap markers) remains in area/save data

---

## Clarifications (User Responses)

### 1. Pet/Familiar System
**Answer**: No existing system. Implement lightweight mechanics:
- **Mount**: Create temporary mob at caster location; mob follows caster; despawns at spell end or caster death
- **Find Familiar**: Similar but with persistence until familiar dies; familiar death inflicts permanent -1 CON on caster
- **Animate Dead**: Summon zombie from corpse in room; follows caster; despawns at spell end
- **Implementation**: Use mob creation + simple follow flag; track familiar with caster->familiar_vnum or similar
- **Rollback risk**: Familiar death mod is permanent; ensure caster consent or make it avoidable

### 2. Deafness Spell
**Answer**: Block ALL communication channels (say, tell, whisper, chat, emote restrictions)
- **Scope**: Affects player's ability to send messages on any channel
- **Visible effect**: Player sees "[You are deaf and cannot hear this]" or similar
- **Exception**: Emotes may still be visible to others (player actions still broadcast)
- **Implementation**: Add AFF_DEAF check in `act_comm.c` (do_say, do_tell, do_whisper, do_chat) and return early with error
- **Duration**: 1d6 + level ticks (spell spec)

### 3. Trap Spells (Magic Mouth, Explosive Runes)
**Answer**: Use persistent objects/flags
- **Magic Mouth**: Create invisible trap object in room; triggers when hostile NPC/PC enters within 4 squares; broadcasts warning
  - Object: Holds trap state (armed/triggered), last trigger time
  - Delete on spell duration end or when triggered too many times
- **Explosive Runes**: Attach to an object or floor; triggers on read; applies damage to reader (4d6+6 no save)
  - Object-based: Wand/scroll holder; hidden flag
  - Floor-based: Room flag + timer object
  - **Choice: Object-based**; easier to manage lifecycle
- **Implementation**: Create special object types with trap flags; add trigger logic to room/object handlers

### 4. Wish Spell
**Answer**: 50/50 success/failure with random beneficial effects
- **Success path (50%)**: Randomly select one effect:
  - +5 to random stat for 1 hour
  - Heal 100 HP
  - Grant 1 free spell cast (no mana cost for next spell, 10 min duration)
  - Restore 50% mana
  - Remove one negative affect (curse, poison, etc.)
  - Teleport to safe room
- **Failure path (50%)**: Random bad effect:
  - Curse caster for 3d6 ticks
  - Reduce random stat by 2 for 1 hour (not below 3)
  - Summon hostile mob (random undead/demon)
  - Drain 100 HP
  - Reduce mana to 10%
- **Implementation**: Pick random int 0–9; map to effect table; execute via affect/damage/summon helpers

### 5. Light Spell
**Answer**: Room-only, temporary; no save with character
- **Behavior**: Creates bright light in room; fades when caster leaves room
- **Track method**: Add flag to room being lit (ROOM_LIGHT or similar); check caster->in_room; remove flag when caster exits
- **Duration**: Stays active as long as caster remains in room; ends on teleport/death/recall
- **Distinction from Continual Light**: Continual Light is permanent room blessing (item-like); Light is temporary caster aura
- **Implementation**: Use room flag + caster's current location check; clean up on room exit

---

## MUD Area Editor (mudeditor) Updates

The MUD Editor must be updated to handle the new spells and flags:

### 1. **web/shared/types/index.ts** (Shared Types)
- **Change**: Add new spell names to skill name union type if it exists
- **Change**: Add new AFF flags to flag metadata if AFF_DEAF is added
- **No changes needed** if types are dynamically generated from skills.json at runtime (likely case)

### 2. **web/server/src/routes/config/** (Config Data Routes)
- **No changes**: Skills are auto-loaded from json/config/skills.json on server startup
- **Verification**: Ensure POST/PUT endpoints reject attempts to create invalid skill entries
- **Risk**: If skills.json is truncated during edit, server may fail to parse; validate JSON integrity after edit

### 3. **web/client/src/pages/ConfigPages/** (Client UI)
- **Skills Config Page**: Auto-displays all skills including new 20 (no code change needed if SKU dynamically loads)
- **Flags Config Page**: If new AFF_DEAF, glitterdust-blind, or persist-trap flags added:
  - Verify they appear in flag metadata (auto-loaded from server)
  - UI sorts alphabetically; should auto-position new flags
- **Testing**: Open SkillsPage and SkillsConfigPage in editor UI; verify all 20 spells appear and are editable

### 4. **web/client/src/components/FlagsField.tsx**
- **No changes** if flag UI is data-driven (flags loaded from API)
- **Verification**: FlagsField components auto-render flags from server metadata

### 5. **web/server/src/parsers/** (JSON Parsers)
- **sanitizeJsonControlChars()**: Already handles area file parsing; no new logic needed
- **Verification**: Ensure skills.json with new entries parses without errors

### 6. **Dockerization**: No code changes; image rebuild will load new skills.json on start

---

## Implementation Tiers — Refined

### **Tier 1: Simple Damage Spells** (Low Risk)
**Spells**: Acid Arrow, Flame Arrow, Demon Fire
**C Changes**:
- Add functions to `src/spell_off.c`
- Add declarations to `src/magic.h`
- Register in `src/spell_dispatch.c`

**JSON Changes**:
- Add 3 entries to `json/config/skills.json` (slots 533, 535, 543)

**Testing**:
- Cast on NPC; verify damage message and save mechanic
- Acid Arrow: Verify equipment damage (if system exists; otherwise skip)
- Demon Fire: Test high-level caster; confirm 300–500 damage range

**Effort**: ~45 min

---

### **Tier 2: Detection, Buffs & Affections** (Medium Risk)
**Spells**: Detect Undead, Glitterdust, Protection from Evil/Good, Stoneskin, Deafness, Mass Invisibility, Legend Lore, Locate Person, Light

**C Changes**:
- Add functions to `src/spell_aff.c` (7 spells), `src/spell_misc.c` (3 spells)
- Add declarations to `src/magic.h`
- Register in `src/spell_dispatch.c`
- **Deafness**: Add AFF_DEAF to `src/flags.h` enum, register in `src/flags.c`, add checks in `src/act_comm.c` (do_say, do_tell, do_whisper, do_chat)
- **Light**: Add ROOM_LIGHT flag to `src/flags.h` (room_flags), register in `src/flags.c`, add room cleanup logic in appropriate handler (e.g., when caster leaves room)

**JSON Changes**:
- Add 9 entries to `json/config/skills.json` (slots 525, 529, 530, 531, 537, 538, 540, 542, 527)
- If new flags: update `json/meta/flags/` (auto-generated after rebuild; verify on next boot)

**Additional Files**:
- `json/help/` — Create help entries for Deafness, Light if not auto-generated

**Testing**:
- Detect Undead: Cast in room with undead; verify message
- Glitterdust: Cast on invisible NPC; verify reveal + blind effect (1d4 ticks)
- Protection spells: Take damage from aligned NPC; verify 90% of damage reduction
- Stoneskin: Check AC; verify -20 bonus; test on self only (reject on others)
- Deafness: Cast on NPC; try to use says/tells; verify blocked
- Mass Invisibility: Cast; verify all creatures in room go invisible
- Legend Lore: Identify item; verify all fields displayed
- Locate Person: Cast; verify room title output or failure (e.g., no-summon flag)
- Light: Cast; leave room; verify light fades

**Effort**: ~2–3 hours

---

### **Tier 3: Complex Mechanics** (High Risk)
**Spells**: Mount, Find Familiar, Animate Dead, Explosive Runes, Magic Mouth, Wish

**C Changes**:

**Mount** (`src/spell_summon.c` or new file):
- Create temp mob based on caster level
- Mob follows caster; caster can mount (ride) it
- Despawn on duration end or caster death
- **New files/changes**:
  - Add mount mob vnums to constants or config
  - Create mob in `create_summon()` helper
  - Add follow flag & mount tracking to caster struct (or use existing familiar-slot if available)

**Find Familiar** (similar to Mount but more complex):
- Summon pet; various types (cat, hawk, owl, toad, etc.) by random %
- Pet grants AC -5 bonus, special powers (infravision, detect magic, regen, etc.)
- Pet contributes its HP to caster's max HP
- **On familiar death**: Permanent -1 CON on caster; spell locked for 5 levels
- **New files/changes**:
  - Add familiar mob vnums (8 types)
  - Create familiar in `create_familiar()` helper
  - Track familiar lifespan; on death, apply CON penalty
  - Add spell cooldown check (gsn_find_familiar set)

**Animate Dead** (mid-complexity):
- Find corpse in room
- Create zombie mob from corpse
- Zombie follows caster; despawn on duration end
- **New files/changes**:
  - Corpse search logic
  - Zombie mob creation from corpse template
  - Despawn handler

**Explosive Runes** (mid-complexity):
- Create invisible trap object
- Place in caster's inventory or room
- Trigger on proximity/read (TBD based on object model)
- Damage (4d6+6) to triggerer; no save
- Trap persists until triggered or duration end
- **New files/changes**:
  - Add ITEM_EXPLOSIVE_RUNES type or flag
  - Trigger check in object read/examine handler
  - Damage + destruction on trigger

**Magic Mouth** (similar to Explosive Runes):
- Create invisible alarm object
- Place in room
- Trigger when hostile NPC/PC enters (4 sq range, or room-based for simplicity)
- Broadcast warning message
- Persist until triggered or duration end
- **New files/changes**:
  - Add ITEM_MAGIC_MOUTH type or flag
  - Trigger check in `do_enter` or room entry handler

**Wish** (low complexity but magic-heavy):
- Random function: pick 0–9
- Effect table: map to benefit or curse
- Execute via existing affect/damage/summon helpers
- **New files/changes**:
  - Effect table in spell_misc.c or separate file
  - Dispatch logic to call appropriate helper

**JSON Changes**:
- Add 6 entries to `json/config/skills.json` (slots 526, 528, 541, 534, 532, 544)

**Additional Files**:
- May need new src/spell_summon.c if splitting out mount/familiar/animate
- May need new damage type DAM_EXPLOSION if not reusing DAM_FIRE (check Fireball)

**Testing**:
- Mount: Cast; verify mob creation; ride indicator; despawn
- Familiar: Cast; verify type chosen; AC bonus active; test on-death scenario (death by dmg or spell)
- Animate Dead: Cast with corpse; verify zombie creation; despawn
- Explosive Runes: Create trap; read in room; verify damage + destruction
- Magic Mouth: Create trap; move hostile NPC into room; verify alarm
- Wish: Cast 20 times; verify 50/50 split; log effects for sanity check

**Effort**: ~3–4 hours

---

## Build & Verification Strategy — Refined

1. **Tier 1**: `make clean && make` — verify no errors
2. **Tier 2**: Add AFF_DEAF flag → `make` — verify flag registrations
3. **Tier 3**: Create spell_summon.c → `make` — verify function linking
4. **Integration**: Boot server → cast each spell in MUD → verify behavior
5. **MUD Editor**: Open web UI → navigate SkillsPage → verify all 20 spells appear + are editable
6. **Persistence**: Save area with spell-enchanted objects → reload → verify spells still references correctly

---

## Documentation Updates — Refined

### 1. **json/help/** — Spell Help Entries
Create one file per spell or combine into thematic files:
- `json/help/spells_detect.json` — Detect Undead, Locate Person, Legend Lore
- `json/help/spells_damage.json` — Acid Arrow, Flame Arrow, Demon Fire
- `json/help/spells_buff.json` — Light, Stoneskin, Protection from Evil/Good, Mass Invisibility
- `json/help/spells_debuff.json` — Deafness, Glitterdust, Wish
- `json/help/spells_summon.json` — Mount, Find Familiar, Animate Dead
- `json/help/spells_trap.json` — Explosive Runes, Magic Mouth

Each entry:
```json
{
  "help": {
    "name": "<spell_name>",
    "keyword": "<SPELL_NAME> [ALIAS]",
    "text": "Multi-line description. Use |.\n|...",
    "hide_keywords": true
  }
}
```

### 2. **doc/Json_Documentation.md**
- Add new AFF flags to AFF_FLAGS section (if any)
- Add new ROOM_FLAGS to ROOM_FLAGS section (if any)
- Add spell slot table update with new slots 525–544

### 3. **.github/agents/cheatsheet.md**
- Add "Recent Spell Integrations" section:
  ```
  ### Recent Spell Implementations (John Lin)
  
  **New spell slots**: 525–544
  - Detect Undead (525), Find Familiar (526), Light (527), Mount (528), ...
  
  **New AFF flags**: AFF_DEAF (if added)
  
  **New ROOM flags**: ROOM_LIGHT (if added)
  
  **Pattern notes**:
  - Light spell: Room-based, not saved; cleanup on caster room exit
  - Familiar: Pet death inflicts permanent -1 CON
  - Deafness: Blocks all comm channels (say, tell, whisper, chat)
  - Wish: 50/50 random benefit/curse from effect table
  - Traps: Persistent objects; trigger on proximity/read
  ```

### 4. **json/help/credits.json**
- Add entry for John Lin with spell list

---

## Rollback Contingency

If any tier fails:

**Tier 1 rollback**: Remove acid arrow, flame arrow, demon fire from skills.json + spell_off.c functions + declarations + dispatcher
**Tier 2 rollback**: Revert deafness AFF_DEAF flag from flags.h/flags.c/act_comm.c; remove all 9 spells
**Tier 3 rollback**: Remove all 6 spells + any new src/spell_summon.c; revert trap object flags

---

## Known Unknowns (To Be Determined During Implementation)

1. **Familiar tracking field**: Does caster struct have a familiar_vnum or similar? Or use GSN + object tracking?
2. **Equipment damage system**: Does BaseMUD have item degradation? Check obj.h for durability fields.
3. **Room exit hook**: Where is the best place to hook caster room-exit to clean up Light spell?
4. **Trap trigger mechanism**: Should triggers check room->people loop or use a pulse? Check existing trap code.
5. **Wish effect targets**: Should Wish effects apply to caster only, or allow targeting? (Spec unclear; assume self-only for v1)

---

## Next Steps

1. **Plan approved** (current status)
2. **Phase 4: Implementation** — Start Tier 1 (Damage Spells)
3. Build, test, move to Tier 2
4. Tier 2 complete, move to Tier 3
5. Full build + MUD Editor verification
6. Help/cheatsheet/credits documentation
7. Commit + push

**Status**: Ready for implementation.
