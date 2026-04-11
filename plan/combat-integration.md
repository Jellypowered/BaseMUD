# Plan: Combat Integration (C3 Boss Loot + C4 Boss Powers)

## Overview
Hook boss loot and boss powers into the combat death flow so they trigger when a pocket dungeon boss dies.

## Files to Modify

### src/fight.c
1. Include `pocket_dungeon.h` header
2. After `char_die(victim)` returns corpse (~line 728), add:
   - Check if victim is in a pocket dungeon instance: `pd_find_instance_for_char((CHAR_T *)victim)`
   - If in instance and boss (last room), call `pd_trigger_boss_loot(inst, victim)`
   - Boss powers are already assigned to instance during generation; combat can check them during damage calculation (deferred)

## Implementation Details

### Boss Loot Trigger (C3)
- Function already guards with `boss_killed` flag (only triggers once per instance)
- Simple integration: after char_die() when IS_NPC, check instance and call trigger
- No need to identify specific "boss room" — function handles all logic
```c
/* After char_die(victim) returns corpse, around line 729: */
if (IS_NPC(victim)) {
    PD_INSTANCE_T *inst = pd_find_instance_for_char((CHAR_T *)victim);
    if (inst != NULL) {
        pd_trigger_boss_loot(inst, victim);  /* Safe: only triggers once if boss */
    }
}
```

### Boss Powers (C4)
- Already assigned to `inst->boss_powers[]` per-instance
- Powers stored but not yet integrated into combat damage/healing calculations
- Deferred to later pass (would require fight.c modifications to check/apply actual effects)
- For now: marked as staged/ready for integration

## Risks
- Need to ensure boss identification is correct (check last room or use a flag)
- Boss loot should only trigger once (inst->boss_killed flag prevents repeats)
- May need to import pocket_dungeon.h in fight.c (add to includes)

## Status: NOT STARTED
