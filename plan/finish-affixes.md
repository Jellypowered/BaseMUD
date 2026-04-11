# Plan: Finish Remaining Affixes (Ancient, Cursed, Luminous)

## Overview
Implement logic for the 3 remaining affix types that currently have empty case statements in the mob spawn loop.

## Files to Modify
1. **src/pocket_dungeon.h**
   - Add new ext_mob flags for Cursed tracking

2. **src/pocket_dungeon.c**
   - Modify density calculation in `pd_generate_instance()` to handle Ancient affix
   - Add Cursed case: Set ext_mob flag for combat engine (MOB_CURSED_HEALING or similar)
   - Add Luminous case: Reduce armor values by 20% (worse dodging)
   - Apply Luminous to boss if present

## Implementation Details

### Ancient (+1 density per 2 rooms)
- Affects the `mob_count` calculation in the spawn loop
- After calculating base mob_count, check for Ancient affix
- If present: multiply mob_count by 1.5 (or add +50%)
- Still respect the B1 hard cap of 5 mobs/room
- Example: if mob_count would be 3, with Ancient it becomes 4

### Cursed (Healing reversed)
- Add ext_mob flag (check if one exists, or define MOB_CURSED)
- Set flag on affected mobs: `SET_BIT(mob->ext_mob, MOB_CURSED)`
- Combat engine will check this flag later to reverse healing effects
- No stat change needed now; flag-based for later integration

### Luminous (-20% dodge)
- Reduce AC values by 20% to make them worse (higher/less protective)
- All 4 armor slots affected: `mob->armor[i] += mob->armor[i] / 5` 
- Example: armor[0] = 100 → 120 (worse protection)
- Apply to both regular mobs and boss

## Risks
- Ancient might make dungeons too easy or too hard; cap mob_count at 5 to prevent overflow
- Cursed flag needs combat.c integration later (not done in this plan, staged)
- Luminous affects all armor types uniformly; may need tuning

## Status: NOT STARTED
