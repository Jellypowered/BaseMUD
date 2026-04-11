# Pocket Dungeon — Phase 2 Refinements (Bugs, Features, UX)

**Date**: April 11, 2026
**Phase**: Planning (Phase 2)
**Status**: IN PLANNING

---

## Executive Summary

This refinement pass addresses 7 identified bugs, fixes 1 limitation, implements 5 future improvements, and adds a player-facing "Dungeon Setup" command for gold-based dungeon customization.

---

## Part A: Bug Fixes

### Bug 1: Density Overflow at High Levels
**File**: `src/pocket_dungeon.c`
**Issue**: If `mob_density_max` is set too high (e.g., >5), dungeons become unplayably crowded at level 50.
**Root Cause**: No upper bound on mob count per room.
**Fix Strategy**:
- Add hard cap: `mob_count = UMIN(mob_count, 5)` after density calculation
- Log warning if seed tries to exceed cap
- Document in skills.json that `mob_density_max` should not exceed 5

**Lines to modify**: `src/pocket_dungeon.c` ~ line 520 (density calculation loop)

### Bug 2: Depth Multiplier Runaway  
**File**: `src/pocket_dungeon.c`
**Issue**: Boss mobs at depth 1.5× multiplier can hit level 100 cap too early, making end-game unfair.
**Root Cause**: Depth multiplier (1.0–1.5×) applied directly to mob level without considering final caps.
**Fix Strategy**:
- Reduce depth multiplier to 1.0–1.3× (less aggressive)
- OR: Apply depth multiplier BEFORE level variance (not after), so clamping is predictable
- Add comment explaining multiplier design choice

**Lines to modify**: `src/pocket_dungeon.c` ~ lines 535–545 (depth_mult calculation)

### Bug 3: Hidden Item Search Exploit
**File**: `json/config/pocket_dungeon_seeds.json` + `src/pocket_dungeon.c`
**Issue**: If the hint phrase appears verbatim in room description, observant players can spot it without using `search` skill.
**Root Cause**: Hints are directly appended to room descriptions during generation; no obfuscation.
**Fix Strategy**:
- Store hint separately in room `extra_descr` (not room `description`)
- Only show extra_descr hint if `ITEM_HIDDEN` flag is still present
- Alternatively: Use a "hint pool" with hint numbers instead of text
- Validate all 15 themes have varied, non-obvious hint phrases

**Files to validate**: `json/config/pocket_dungeon_seeds.json` (all hide_hint_phrases entries)

### Bug 4: Loot Drop Loops
**File**: `src/pocket_dungeon.c`
**Issue**: If `loot_density` is miscalculated, nearly every room could drop loot or almost none would.
**Root Cause**: Loot spawn loop uses `i += max(1, 5 / seed->loot_density)` which can overflow if density is 0 or very small.
**Fix Strategy**:
- Validate `loot_density` is in range [0.1, 3.0] during seed initialization
- Use UMAX/UMIN to clamp: `i += UMAX(1, (5 * 10) / seed->loot_density)`
- Add debug output for loot room selection

**Lines to modify**: `src/pocket_dungeon.c` ~ line 580+ (floor loot loop)

### Bug 5: Theme Data Missing
**File**: `json/config/pocket_dungeon_seeds.json`
**Issue**: If a seed is missing `mob_vnums[]` or `item_vnums[]`, spawning could crash or drop null items.
**Root Cause**: No validation during seed load; spawning assumes arrays exist.
**Fix Strategy**:
- Add validation function `pd_validate_seed()` in `src/pocket_dungeon.c`
  - Check: `mob_vnum_count > 0`, `item_vnum_count > 0`
  - Check: `boss_vnum > 0`, `sentinel_vnum > 0`
  - Check: `container_vnum > 0`, `hidden_container_vnum > 0`
  - Check: All required string fields are non-null
- Call `pd_validate_seed()` during seed load in JSON parser
- Log errors and skip invalid seeds with descriptive message

**Files to create/modify**: 
- `src/pocket_dungeon.c` — add `pd_validate_seed(pd_seed_t *seed)` function
- `src/pocket_dungeon.h` — declare above function
- `src/json_tblr.c` — call validation after loading each seed

### Bug 6: Skill Registration Typos
**File**: `src/tables.c`, `src/interp.c`, `json/config/skills.json`
**Issue**: If `SKILL_MAP_SEARCH` typo or mismatch, `do_search` silently fails.
**Root Cause**: Three-way registration (defs.h → tables.c → interp.c) easy to misalign.
**Fix Strategy**:
- Add compile-time assertion: ensure `SKILL_MAP_SEARCH` is < `SKILL_MAP_MAX`
- Add startup validation in `init_skills()` that `skill_map_table[SN(MAP_SEARCH)]` exists
- Add to cheatsheet: search skill registration checklist

**Files to modify**:
- `src/defs.h` — add `#error` check
- `src/tables.c` — add startup log message
- `.github/agents/cheatsheet.md` — document registration pattern

### Bug 7: Race Condition on Snapshots
**File**: `src/pocket_dungeon.c` (snapshot writing)
**Issue**: If two players spawn the same seed instance simultaneously, snapshot files could overwrite each other.
**Root Cause**: No file locking; snapshot filename based only on instance vnum.
**Fix Strategy**:
- Ensure snapshot filename uses instance ID (already unique per instance)
- Add temp file pattern: write to `.tmp` then atomic rename
- OR: Lock file during write with `flock()` (Unix) / `LockFile()` (Windows)
- For now (quick fix): Use instance->vnum + instance creation timestamp in filename

**Lines to modify**: `src/pocket_dungeon.c` ~ line 650+ (pd_write_snapshot)

---

## Part B: Limitation Fixes

### Limitation 3: Mob Level Floor = 1
**File**: `src/pocket_dungeon.c`
**Issue**: Mobs can sometimes end up at level 0 or negative due to variance `±2` before clamping.
**Current Code**: 
```c
int final_level = UMAX(1, depth_level + number_range(-2, 2));
```
**Status**: Already correct! But verify everywhere level is set.
**Fix Strategy**:
- Audit all places where mob level is assigned
- Ensure all use `UMAX(1, ...)` before scaling
- Add assertion in `pd_scale_mob_to_level()` that final level >= 1

**Lines to audit**: `src/pocket_dungeon.c` lines 515, 540, 560, etc.

---

## Part C: Future Improvements

### Feature C1: Dungeon Affixes (Random Modifiers)
**Files to modify**:
- `src/pocket_dungeon.h` — add affix enum
- `src/pocket_dungeon.c` — add `pd_apply_affixes()` function
- `json/config/pocket_dungeon_seeds.json` — add `affix_list[]`
- `src/pocket_dungeon.c` (pd_generate_instance) — call affix application

**Affixes** (start with 5):
1. **Stony** (+20% mob HP)
2. **Cursed** (healing reversed for 10 ticks after damage)
3. **Swift** (+25% mob haste)
4. **Ancient** (+1 mob density for every 2 rooms)
5. **Luminous** (–20% mob dodge, +10% item drop rate)

**Implementation**:
- Randomly select 1–2 affixes during instance generation
- Store `instance->affixes[]` array
- In mob scaling: apply HP += 20% if STONY, etc.
- Display affix names in UI and snapshot

### Feature C2: Progressive Difficulty (Scaling from Entry)
**File**: `src/pocket_dungeon.c`
**Concept**: The farther players progress without leaving, the harder mobs get (mini-progression bonus).
**Implementation**:
- Add counter: `instance->rooms_cleared` (increment when room is empty of mobs)
- On mob spawn: apply bonus level = `+1 per 5 rooms cleared` (up to +10)
- On room exit: clear counter (reward for leaving)
- This creates a risk/reward: stay → gain loot&XP but face harder fights; leave → reset

**Lines to add**: `src/pocket_dungeon.c` (new function `pd_update_difficulty_momentum()`)

### Feature C3: Boss Loot Tables
**File**: `src/pocket_dungeon.c` + `json/config/pocket_dungeon_seeds.json`
**Concept**: Boss mobs drop guaranteed tier loot + special drops.
**Implementation**:
- Add to seed: `boss_loot_table[]` array of item vnums (5–10 items)
- On boss death: check `instance->boss_killed` flag
- If true: drop 2–3 items from boss_loot_table (guaranteed unique)
- Log in snapshot: "Boss loot: [item1], [item2], [item3]"

**Files**:
- `src/pocket_dungeon.c` — add `pd_boss_death_drop()` function (called from combat code)
- `json/config/pocket_dungeon_seeds.json` — add `boss_loot_table[]` to each seed

### Feature C4: Randomized Boss Powers
**File**: `src/pocket_dungeon.c` + `src/mob_progs.c` (NEW mobprog system)
**Concept**: Bosses get random abilities (aura, AoE, heal phase).
**Implementation** (see Limitation Fix C5 below for mobprog generation):
- Create `pd_assign_boss_powers()` function
- Randomly select 2–3 powers from pool:
  - **Power Strike**: Every 8 rounds, 50% chance to stun target for 2 rounds
  - **Healing Aura**: Boss heals +5 HP/round (first 5 rounds, then fades)
  - **Summon Guardian**: Boss summons sentinel mob if health < 50%
  - **Dodge Stance**: Boss gains +30% dodge for 3 rounds (1x per fight)
  - **Life Drain**: Boss attk restores 20% dmg as healing
- Store selected powers in `instance->boss_powers[]`
- Engine reads powers during combat calculations

**Files**: 
- `src/pocket_dungeon.c` — add power assignment logic
- `src/combat.c` — modify damage/healing logic to check boss_powers

### Feature C5: Procedural Mobprog Generation
**Scope**: Not limited to bosses; all pocket dungeon mobs can have generated personalities
**Concept**: Generate ROM mobprog scripts at runtime for dynamic AI behavior (combat tactics, spell rotations, personality traits).
**Architecture**: 7 personality archetypes (Brute, Tactician, Guardian, Coward, Summoner, Berserker, Assassin) with difficulty scaling (levels 1–50).

**Integration**:
- Hook into mob spawning in `pd_generate_instance()`
- Generate and attach mobprogs using existing ROM mobprog system (MPROG_LIST_T, program_flow VM)
- Scripts leverage 16 trigger types (TRIG_FIGHT, TRIG_HPCNT, TRIG_RANDOM, TRIG_DEATH, etc.)
- Mobs execute 29+ commands (kill, cast, say, mload, damage, etc.) as AI behavior

**Files to create**: 
- `src/pocket_dungeon_mobprog.c` — NEW (generation + attachment functions)
- `src/pocket_dungeon_mobprog.h` — NEW (declarations + personality enums)
- `plan/mobprog-generation-system.md` — NEW (standalone deep-dive plan with examples, trigger catalog, architecture)

**Key Functions**:
- `pd_generate_and_attach_mobprog(mob, instance)` — main entry point; generates + attaches scripts
- `pd_generate_combat_ai(mob, personality, difficulty)` — combat behavior (FIGHT, HPCNT, RANDOM)
- `pd_generate_passive_behavior(mob, difficulty)` — greeting/entry behavior (GREET, ENTRY)
- `pd_generate_personality_behavior(mob, personality)` — personality (DEATH, KILL, SPEECH)
- `pd_select_personality(mob_vnum)` — deterministic personality selection
- `pd_attach_mobprog_to_mob(mob, code, trig_type, trig_phrase)` — bind trigger to mob using existing mobprog system

### Feature C6: Instance Despawn with Corpse Protection
**Status**: ✅ **ALREADY IMPLEMENTED**

**Location**: `src/pocket_dungeon.c` in `pd_update_all()` (lines 756–762)

**How it works**:
- `pd_update_all()` is called periodically (part of game pulse cycle)
- For each instance: checks if any players inside OR if any PC corpses in rooms
- If occupied (players inside): reset empty timer
- **If corpses detected**: DO NOT despawn; reset empty timer so timeout won't fire
- If empty AND no corpses: start/check timeout; if timeout exceeded, call `pd_destroy_instance()`
- Corpses are frozen (timer = -1) so they don't decay while instance is alive

**Code snippet** (already present):
```c
if (occupied) {
    inst->last_empty_at = 0;
}
else if (has_corpse) {
    /* Keep the instance alive while a player corpse is present. */
    inst->last_empty_at = 0;
}
else {
    if (inst->last_empty_at == 0)
        inst->last_empty_at = now;
    else if (now - inst->last_empty_at >= timeout_secs)
        pd_destroy_instance(inst);
}
```

**No changes needed** — feature complete.

### Feature C7: Difficulty Selector ("Dungeon Setup" Command)
**Player-Facing Command**: `dungeon_setup`
**Concept**: Menu-driven customization with gold cost (free during testing, gated during production).
**Testing Mode**: Controlled by `testing_mode` config flag (settable in MUDEditor UI; default = true).
**Implementation**:
- New command in `src/act_move.c`: `do_dungeon_setup()`
- Menu loop:
  ```
  === Dungeon Setup Menu ===
  1. Select Theme      [Current: %s]
  2. Set Difficulty    [Current: %s]
  3. Enable Affixes    [Current: %s]
  4. Review & Spawn    [Cost: %d gold] %s
  5. Cancel
  
  (If testing_mode=true, display: "[TEST MODE - FREE]" next to cost)
  ```
- **Themes**: All 15 seeds (display with min_level hint to guide player choices)
- **Difficulty**: Easy (L-10), Normal (L), Hard (L+10), Nightmare (L+20)
- **Affixes**: Enable/disable toggle (multiple allowed; each affix costs extra gold)
- **Costs** (only charged if testing_mode=false):
  - Easy: 50g, Normal: 0g (free), Hard: 100g, Nightmare: 250g
  - +20g per affix enabled
  - Display shows 0g if testing_mode=true
- On confirmation: 
  - If testing_mode=false AND player.gold < cost → refuse with "Insufficient gold"
  - Else: Deduct gold (if testing_mode=false), create instance with customized parameters
- Log to snapshot: setup_cost, difficulty_selection, affixes_enabled, theme_selected

**Files**:
- `src/act_move.c` — add `do_dungeon_setup()` command with menu loop
- `src/act_move.h` — declare command
- `src/interp.c` — register command
- `src/pocket_dungeon.c` — add `pd_create_custom_instance(theme, difficulty, affixes[], testing_mode)` function
- `src/pocket_dungeon.h` — declare function, add instance struct fields for setup info
- `json/config/pocket_dungeon_seeds.json` — add `min_level` hint to each theme ("Recommended for levels 1-15" etc)

---

## Part D: Implementation Order

1. **Bug fixes first** (Bugs 1–7): Low risk, high confidence
2. **Limitation fix** (L3): Verify + audit existing code
3. **Affixes (C1)**: Moderate complexity, good visible impact
4. **Progressive difficulty (C2)**: Easy add to spawning loop
5. **Boss loot tables (C3)**: Straightforward data structure
6. **Boss powers (C4)**: Requires careful design; tie to mobprog system
7. **Mobprog generation (C5)**: New subsystem; implement core only (Phase 1 = generation, Phase 2 = engine integration)
8. **Difficulty selector (C7)**: Command framework + menu loop + MUDEditor UI (good end-user feature)

---

## Part E: Files to Modify/Create

### BaseMUD C
- [ ] `src/pocket_dungeon.c` — MODIFY (all bugs 1–4, L3, features C1–C5, C7)
- [ ] `src/pocket_dungeon.h` — MODIFY (new function declarations, new struct fields for affixes/powers/setup_cost)
- [ ] `src/pocket_dungeon_mobprog.c` — CREATE (C5: mobprog generation)
- [ ] `src/pocket_dungeon_mobprog.h` — CREATE (C5: function declarations)
- [ ] `src/act_move.c` — MODIFY (add `do_dungeon_setup()` for C7)
- [ ] `src/act_move.h` — MODIFY (declare `do_dungeon_setup()`)
- [ ] `src/interp.c` — MODIFY (register dungeon_setup command)
- [ ] `src/json_tblr.c` — MODIFY (add seed validation call, bug 5; add testing_mode flag load)
- [ ] `src/combat.c` — MODIFY (boss powers execution, feature C4)
- [ ] `.github/agents/cheatsheet.md` — MODIFY (add skill registration checklist, bug 6)
- [ ] `json/config/pocket_dungeon_seeds.json` — MODIFY (affixes, boss loot, min_level hints, bug 3 hint validation)
- [ ] `json/config/pocket_dungeon_config.json` — MODIFY (add `testing_mode` boolean flag, default = true for development)

### MUDEditor
- [ ] `web/shared/types/index.ts` — MODIFY (expose `testing_mode` boolean in config types)
- [ ] `web/client/src/pages/ConfigPage.tsx` or similar — MODIFY (add toggle for `testing_mode` in config editor, visible for admins)

---

## Part G: Risks & Dependencies (Revised)

---

## Part F: Clarifying Questions — USER RESPONSES

1. **Difficulty Selector Behavior**: ✅ **Entry Only** - Setup menu available only at dungeon entry; cannot re-customize during run.

2. **Gold Cost During Testing**: ✅ **Show Cost with Testing Flag** - Display cost message but don't deduct gold. Testing mode flag is settable via MUDEditor config (user will expose toggle in MUDEditor settings UI).
   - Implementation note: Need to add `testing_mode` boolean to pocket_dungeon_config.json, loadable from MUDEditor

3. **Instance Despawn**: ✅ **2hr Idle Timeout (Like Current System)** - Instances persist like regular areas, unload after 2 hours empty BUT only if **no player corpses** remain inside (prevents item loss).
   - Implementation note: During despawn check, scan instance rooms for `item->item_type == ITEM_CORPSE_PC`; if found, skip despawn

4. **Boss Powers Fallback**: ✅ **Yes, Hardcode Fallback** - Boss powers apply as simple combat checks if mobprog engine unavailable.

5. **Affix Probability**: ✅ **Equal Probability** - All affixes equally likely (no weighted rarity).

---

## Part G: Risks & Dependencies

| Risk | Mitigation |
|------|-----------|
| Boss multiplier scaling causes instant level 100 | Cap depth multiplier to 1.3×; test at level 50 |
| Loot loop spin → infinite loop | Validate loot_density [0.1, 3.0]; add loop guard |
| Mobprog generation creates invalid syntax | Generate to `.tmp` file; validate before storing |
| Gold cost stored incorrectly | Store in `instance->setup_cost`; verify on creation |
| Affixes interact badly (e.g., Swift + Power Strike = broken) | Document affixes as independent; test combinations |
| Testing mode flag conflicts between C and TypeScript | Store flag in both pocket_dungeon_config.json AND instance struct; sync on load |
| Player corpse detection misses edge cases | Use robust scan: `for_each_obj_in_rooms(instance, obj) { if (obj->item_type == ITEM_CORPSE_PC) }` |

---

## Part H: Success Criteria

- [ ] All 7 bugs fixed and verified (unit tests or manual checks)
- [ ] Mob level floor always >= 1 (audit complete)
- [ ] Instance despawn check detects player corpses and preserves instances correctly
- [ ] Affixes apply correctly (3+ affixes spawn correctly in instances)
- [ ] Progressive difficulty counter increments and applies level bonus
- [ ] Boss loot table items drop on boss death
- [ ] Boss powers apply without crashing
- [ ] Mobprog generation compiles valid syntax (logged to file)
- [ ] Testing mode flag toggles gold cost on/off in both C and MUDEditor
- [ ] Dungeon setup command opens menu, accepts selections, verifies costs, creates instance
- [ ] Build passes with no errors or warnings
- [ ] All MUDEditor TypeScript compiles with no errors

---

## Status: PLANNING FINALIZED (Ready for Phase 4 — Implementation)

**Plan Date**: April 11, 2026
**User Confirmations**: All 5 clarifying questions answered; scope finalized
**Ready to Proceed**: YES ✅

**Next Step**: Phase 4 — Implementation. Execute bugs (1–7) + Limitation (L3) + Features (C1–C7) in order.

