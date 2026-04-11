# Deep Dive Findings Summary — CORRECTED

**Date**: April 11, 2026
**Purpose**: Clarify pocket dungeon instance despawn + document procedural mobprog generation for instances

---

## Finding 1: Instance Despawn with Corpse Protection — Already Implemented ✅

### Actual Implementation

**Location**: `src/pocket_dungeon.c` in `pd_update_all()` function (lines 756–762)

**How it works**:
```c
for (inst = pd_instance_first; inst != NULL; inst = inst_next) {
    inst_next = inst->global_next;

    /* Check if any PCs are still inside */
    bool occupied = FALSE;
    bool has_corpse = FALSE;
    
    /* Scan rooms for living PCs and PC corpses */
    for (room = inst->area->room_first; room != NULL; room = room->area_next) {
        for (ch = room->people_first; ch != NULL; ch = ch->room_next) {
            if (!IS_NPC(ch)) {
                occupied = TRUE;
                break;
            }
        }
        /* Freeze corpses so they don't decay */
        for (cobj = room->content_first; cobj != NULL; cobj = cobj->content_next) {
            if (cobj->item_type == ITEM_CORPSE_PC) {
                has_corpse = TRUE;
                pd_freeze_obj_recursive(cobj);
            }
        }
    }

    if (occupied) {
        inst->last_empty_at = 0;
    }
    else if (has_corpse) {
        /* ✅ KEEP INSTANCE ALIVE WHILE CORPSES PRESENT */
        inst->last_empty_at = 0;
    }
    else {
        /* Check timeout; if empty > timeout_secs, despawn */
        if (inst->last_empty_at == 0)
            inst->last_empty_at = now;
        else if (now - inst->last_empty_at >= timeout_secs)
            pd_destroy_instance(inst);
    }
}
```

**What this does**:
- Scans all rooms in the instance for living players and corpses
- If players inside: don't despawn (reset empty timer)
- **If player corpses detected: DON'T despawn** (reset empty timer; they're protected)
- If empty with no corpses: start timeout; despawn when timeout expires
- Corpses are frozen (`timer = -1`) so they don't naturally decay

**Status**: ✅ **Feature C6 is COMPLETE. No changes needed.**

---

## Finding 2: General Area Despawn System (NOT needed for pocket dungeons)

**Note**: The standard ROM area reset system was researched but is NOT relevant to pocket dungeon instances, which use their own custom despawn logic (see Finding 1 above).

For reference (if building other features):
- Standard areas reset after ~1 hour via `area_should_update()` in `src/areas.c`
- Controlled by `AREA_RESET_ALWAYS_AGE = 31 ticks` (~62 minutes)
- Called from `update_handler()` every PULSE_AREA (120 seconds)

**Pocket dungeons**: Use independent timeout system in `pd_update_all()`, not the ROM area system.

---

## Finding 3: Procedural MobProg Generation System (Pocket Dungeon Instances Only)

### MobProg Capabilities Review

BaseMUD has a **complete, flexible mobprog system** (ROM standard). Key facts for procedural generation:

**Architecture**:
- **MPROG_CODE_T** — Global pool of reusable script strings
- **MPROG_LIST_T** — Links triggers to individual mobs
- **Execution**: Stack-based VM via `program_flow()` (max 12 nesting, 5 mpcall depth)

**Trigger types** (16 available):
- Combat: TRIG_FIGHT, TRIG_HPCNT, TRIG_RANDOM, TRIG_SURR, TRIG_KILL, TRIG_DEATH
- Entry: TRIG_ENTRY, TRIG_GREET, TRIG_EXALL
- Other: TRIG_SPEECH, TRIG_ACT, TRIG_BRIBE, TRIG_GIVE, TRIG_EXIT, TRIG_DELAY, TRIG_GRALL

**Commands** (29+ available):
- Combat: kill, assist, flee, damage, cast
- Communication: say, emote, echo, echoat, asound, zecho, gecho
- Manipulation: mload, oload, junk, purge
- Movement: goto, transfer, gtransfer, otransfer, force, gforce, vforce
- Control: at, call, delay, cancel, remember, forget, remove

**Conditionals** (51+ checks):
- rand %, mobhere, objhere, people > N, hour > N
- ispc, isnpc, level >=, hpcnt <, carries, wears, name ==, class ==, race ==
- affected, act (flags), imm (immunity), off (offense)

**Variable expansion**:
- `$i` (mob name), `$n` (target), `$t` (2nd target), `$r` (random), `$q` (combat target)
- Pronouns: `$j/$k/$l` (he/him/his), `$e/$m/$s` (she/her/her), `$E/$M/$S` (it/it/its)

### Implementation for C5 (Pocket Dungeon Instances)

**Scope**: **POCKET DUNGEON INSTANCES ONLY** (not a general ROM mobprog architecture)

**Design**: 7 personality archetypes (Brute, Tactician, Guardian, Coward, Summoner, Berserker, Assassin) with difficulty-based spell tiers and trigger configurations.

**Architecture**:
1. **Generation function** (`pd_generate_and_attach_mobprog()`) — called during mob spawning
2. **Personality selection** — deterministic based on mob vnum
3. **Script generation** — Combines passive (GREET/ENTRY), combat (FIGHT/HPCNT/RANDOM), and personality (DEATH/KILL) triggers
4. **Attachment** — Uses existing `mprog_new()` + `LIST2_FRONT()` to bind to mob

**Integration point** (in `pd_generate_instance()` after mob is loaded and scaled):
```c
/* After pd_scale_mob_to_level(mob, final_level) */
if (!mob->mob_index->mprog_first) {
    pd_generate_and_attach_mobprog(mob, instance);
}
```

**Files to create**:
- `src/pocket_dungeon_mobprog.c` — Generation logic + attachment
- `src/pocket_dungeon_mobprog.h` — Declarations + personality enums

**Why this approach**:
- Reuses existing ROM mobprog system (no new interpreter)
- Scripts are human-readable (easy to debug/log)
- Per-mob generation → consistent personality per instance
- Can mix with static ROM mobprogs from .are files

---

## Summary — What's Complete, What's Remaining

### ✅ Already Done (No work needed)
- **Feature C6** (corpse-protected despawn): Fully implemented in `pd_update_all()`

### ⚠️ Still To Do (Phase 4 Implementation)
| Item | Type | Status |
|------|------|--------|
| Bug 1 | Density overflow cap | Not started |
| Bug 2 | Depth multiplier runaway | Not started |
| Bug 3 | Hidden item search exploit | Not started |
| Bug 4 | Loot drop loops validation | Not started |
| Bug 5 | Theme data validation | Not started |
| Bug 6 | Skill registration checks | Not started |
| Bug 7 | Snapshot race conditions | Not started |
| L3 | Mob level floor audit | Not started |
| C1 | Affixes (5 modifiers) | Not started |
| C2 | Progressive difficulty | Not started |
| C3 | Boss loot tables | Not started |
| C4 | Boss powers (5 abilities) | Not started |
| **C5** | **Mobprog generation (7 archetypes)** | **Not started** |
| C7 | Dungeon setup command + MUDEditor | Not started |



