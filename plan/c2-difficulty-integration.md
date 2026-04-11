# Plan: C2 Difficulty Progression Integration

## Overview
Implement room-clear detection and progressive difficulty scaling. Players get +1 difficulty level per 5 rooms cleared (max +10).

## Key Questions
1. **When is a room "cleared"?** — When last mob in room dies
2. **Where to hook?** — fight.c after damage/death, increment instances[].rooms_cleared
3. **How to apply bonus?** — Use pd_get_difficulty_bonus() when spawning new mobs (deferred to future feature)

## Files to Modify

### src/fight.c
- After NPC death, check if room now has zero hostile NPCs
- If yes: find instance and increment inst->rooms_cleared
- Can be done alongside C3 boss loot integration, or as separate pass

### src/pocket_dungeon.c (future)
- When respawning mobs or spawning reinforcements, call:
  ```c
  int bonus = pd_get_difficulty_bonus(inst);
  /* Apply +bonus to spawn levels */
  ```
- Currently only spawns at generation time; would need respawn logic

## Implementation Details

### Room Clear Detection (Phase 3)
```c
/* In fight.c, after NPC death */
if (IS_NPC(victim)) {
    PD_INSTANCE_T *inst = pd_find_instance_for_char((CHAR_T *)victim);
    if (inst != NULL && victim->in_room) {
        /* Check if room now has zero hostile NPCs */
        CHAR_T *ch;
        bool room_has_hostiles = FALSE;
        for (ch = victim->in_room->people_first; ch != NULL; ch = ch->room_next) {
            if (IS_NPC(ch) && !ch->fighting == NULL) {
                room_has_hostiles = TRUE;
                break;
            }
        }
        
        /* If room is now clear and not already counted */
        if (!room_has_hostiles) {
            inst->rooms_cleared++;
        }
    }
}
```

## Risks
- Room clearing counter could double-count if multiple mobs die in same room
- Need to track which rooms have been counted (add tracker to instance?)
- Deferred application of difficulty bonus (not applied to existing spawns)

## Status: NOT STARTED (design phase)

## Note
This feature is complex because spawning is one-time during instance generation. Progressive difficulty would require:
- Enemy respawning system (not yet implemented)
- Or difficulty applied to reinforcements/special spawns
- Current design only increments counter; actual difficulty effect deferred
