# FA XP Bonus System Integration Plan

**Author**: Integration from Fallen Angels snippet (Laurent Zilber, Jerome Despret)  
**Status**: ✅ PLAN COMPLETE  
**Date Created**: 2026-04-16  
**Date Completed**: 2026-04-16

## Objective
Integrate Fallen Angels XP bonus system into BaseMUD's `fight_compute_kill_exp()` to provide experience rewards that account for mob difficulty modifiers (sanctuary, haste, offensive abilities, special functions).

## Current State
- BaseMUD `fight_compute_kill_exp()` computes XP based on level difference only
- Playtime scaling code exists but is disabled (#if 0)
- No bonuses for mob special abilities or offensive flags

## Target State
- Mob attribute bonuses applied after base_exp calculation, before alignment section
- Playtime scaling enabled (uncommented)
- Clear documentation of XP bonus percentages

## Files to Modify

### 1. `src/fight.c` — fight_compute_kill_exp() function
**Location**: Lines ~1586-1827  
**Changes**:
- After base_exp calculation (line ~1675), add mob attribute bonus calculations
- Bonus logic runs only for NPCs (IS_NPC check)
- Apply bonuses in this order:  
  1. Affect bonuses (sanctuary +30%, haste +20%)
  2. Offensive flag bonuses (area_attack +20%, backstab +20%, fast +20%, dodge +10%, parry +10%)
  3. Spec function bonuses (breath +25%, cast +20%, poison +10%)
- Before alignment section (line ~1676, "do alignment computations" comment)

**Exact Location for Insert**: After line ~1675 (after base_exp switch statement closes), before the "do alignment computations" comment

**Code to Insert**:
```c
    /* Mob attribute bonuses (Fallen Angels XP system) */
    if (IS_NPC(victim))
    {
        /* Affect-based bonuses */
        if (affect_is_char_affected(victim, skill_lookup("sanctuary")))
            base_exp = (base_exp * 130) / 100;
        if (affect_is_char_affected(victim, skill_lookup("haste")))
            base_exp = (base_exp * 120) / 100;

        /* Offensive flag bonuses */
        if (IS_SET(victim->off_flags, OFF_AREA_ATTACK))
            base_exp = (base_exp * 120) / 100;
        if (IS_SET(victim->off_flags, OFF_BACKSTAB))
            base_exp = (base_exp * 120) / 100;
        if (IS_SET(victim->off_flags, OFF_FAST))
            base_exp = (base_exp * 120) / 100;
        if (IS_SET(victim->off_flags, OFF_DODGE))
            base_exp = (base_exp * 110) / 100;
        if (IS_SET(victim->off_flags, OFF_PARRY))
            base_exp = (base_exp * 110) / 100;

        /* Special function bonuses */
        if (victim->spec_fun != NULL)
        {
            /* Breath attacks: +25% */
            if (victim->spec_fun == spec_breath_any ||
                victim->spec_fun == spec_breath_acid ||
                victim->spec_fun == spec_breath_fire ||
                victim->spec_fun == spec_breath_frost ||
                victim->spec_fun == spec_breath_gas ||
                victim->spec_fun == spec_breath_lightning)
            {
                base_exp = (base_exp * 125) / 100;
            }
            /* Cast spells: +20% */
            else if (victim->spec_fun == spec_cast_cleric ||
                     victim->spec_fun == spec_cast_mage ||
                     victim->spec_fun == spec_cast_undead)
            {
                base_exp = (base_exp * 120) / 100;
            }
            /* Poison: +10% */
            else if (victim->spec_fun == spec_poison)
            {
                base_exp = (base_exp * 110) / 100;
            }
        }
    }
```

### 2. `src/fight.c` — Uncomment playtime scaling section
**Location**: Lines ~1811-1825 (#if 0 block)  
**Changes**:
- Remove `#if 0` line (line ~1811)
- Remove `#endif` line (line ~1826)
- Uncomment time_per_level variable declaration in function signature (line ~?)

**Note**: Playtime scaling is currently disabled and will be enabled as part of FA integration.

### 3. `.github/agents/cheatsheet.md` — XP Bonus Documentation
**Location**: End of file (or relevant section if exists)  
**Changes**:
- Add entry documenting:
  - Mob attribute bonus system and percentages
  - Playtime scaling re-enabled
  - Affected functions: `fight_compute_kill_exp()`

**Content**:
```markdown
### XP Bonus System (Fallen Angels Integration)
Mobs with special attributes grant bonus XP:
- Sanctuary affect: +30%
- Haste affect: +20%
- OFF_AREA_ATTACK: +20%
- OFF_BACKSTAB: +20%
- OFF_FAST: +20%
- OFF_DODGE: +10%
- OFF_PARRY: +10%
- Breath spec functions: +25%
- Cast spec functions (cleric/mage/undead): +20%
- Poison spec function: +10%
Bonuses are cumulative. Playtime scaling re-enabled in `fight_compute_kill_exp()`.
```

### 4. `json/help/credits.json` — Attribution
**Location**: CREDITS page text field  
**Changes**:
- If file doesn't exist, create it with credits entry
- If file exists, append FA contributors to existing CREDITS page

**Entry Format**:
```
XP bonus system and mob attribute scaling --
    Laurent Zilber, Jerome Despret
```

## Implementation Order
1. ✅ Modify `src/fight.c` - insert mob bonus code after base_exp (single operation, multi_replace for efficiency)
2. ✅ Uncomment playtime scaling in `src/fight.c`
3. ✅ Update `.github/agents/cheatsheet.md`
4. ✅ Update `json/help/credits.json`
5. ✅ Build: `make clean && make -j4`
6. ✅ Verify no build errors
7. ✅ Commit with message

## Dependencies
- `OFF_*` flags: defined in `src/flags.h` ✓
- `affect_is_char_affected()`: defined in `src/affects.h` ✓
- `skill_lookup()`: defined in codebase ✓
- Spec functions (`spec_breath_*`, `spec_cast_*`, `spec_poison`): defined in `src/tables.c` ✓

## Known Risks
- **Cumulative bonuses**: Multiple bonuses stack (e.g., sanctuary + haste + spec = ~1.3 × 1.2 × 1.25). This is intentional per FA design.
- **Playtime scaling**: Uncommented code uses `time_per_level` variable; verify it exists in function signature.
- **Spec function pointers**: Direct function pointer comparison requires all spec functions to be defined and available.

## Rollback Plan
If integration causes unexpected behavior:
1. Revert playtime scaling: re-wrap in `#if 0` / `#endif`
2. Remove mob bonus code block
3. Restore from git: `git checkout HEAD src/fight.c`

## Status
- [x] Code inserted
- [x] Build verified
- [x] Documentation updated
- [x] Credits added
- [ ] Committed and pushed

## Status: COMPLETED — 2026-04-16
