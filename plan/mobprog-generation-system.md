# MobProg Generation System — Standalone Plan (Pocket Dungeon Instances)

**Date**: April 11, 2026
**Purpose**: Procedurally generate mob scripts (mobprogs) for dynamic AI behavior in pocket dungeon instances
**Scope**: **Pocket dungeon instances only** — AI generation specific to procedurally generated mobs within instance areas
**Status**: PLANNING

---

## Executive Summary

This plan designs a procedural mobprog generation system that creates AI behavior scripts at runtime for pocket dungeon mobs. Instead of hardcoded combat logic, instance mobs get dynamically generated scripts that define:
- Combat tactics (when to fight, flee, cast spells)
- Personality traits (aggressive, defensive, cowardly, tactical)
- Abilities (heal, summon allies, use special attacks)
- Reactions (fleeing at low HP, assisting allies, mocking players)

**Key Insight**: MobProg code is stored as C strings in a global pool (MPROG_CODE_T). Generation means building syntactically valid program scripts as text, embedding them in memory, then binding triggers to mobs. This approach is specific to pocket dungeon procedural mob generation and does NOT apply to static ROM mobprogs loaded from .are files.

---

## Part 1: MobProg System Capabilities Review

### Trigger Types Available (16 total)

| Type | Fired When | Parameter | Use Case |
|------|-----------|-----------|----------|
| TRIG_FIGHT | Mob starts combat | % chance | On-combat activation |
| TRIG_HPCNT | HP drops below % | threshold | Heal/flee logic |
| TRIG_RANDOM | Every pulse | % chance | Periodic AI (spell rotations) |
| TRIG_KILL | Mob kills someone | % chance | Victory taunts |
| TRIG_DEATH | Mob dies | % chance | Deathbed ability (summon help) |
| TRIG_GREET | Player enters idle | % chance | Initial greeting |
| TRIG_ENTRY | Player enters room | % chance | Alert mobs |
| TRIG_SURR | Mob surrounded | % chance | Desperate tactics |
| TRIG_SPEECH | Player says phrase | keyword | Behavior on keywords |
| TRIG_ACT | Action text matches | phrase | Reaction to events |
| TRIG_GIVE | Object given to mob | vnum | Reward/greeting if given item |
| TRIG_BRIBE | Gold given to mob | amount | Bribe-based behavior |
| TRIG_DELAY | Countdown fires | % chance | Scheduled actions |
| TRIG_EXIT | Player exits room | direction | Try to intercept |
| TRIG_EXALL | Player exits (always) | direction | Persistent exit attempts |
| TRIG_GRALL | Greet (always active) | % chance | Constant greeting |

### MobProg Commands Available (29 total)

**Combat:**
- `kill <target>` — engage combat
- `assist <target>` — join ally's fight
- `flee` — retreat from combat
- `damage <target> <amount>` — direct HP hit (unblockable)
- `cast <spell> <target>` — cast mobilized spells

**Communication:**
- `say <text>` — speak (supports $variable expansion)
- `emote <action>` — action text
- `echo <text>` — room message
- `echoat <$n> <text>` — message to one player
- `asound <text>` — adjacent rooms broadcast
- `zecho <text>` — entire zone broadcast

**Manipulation:**
- `mload <vnum> [count]` — spawn mob(s)
- `oload <vnum> [count]` — spawn obj(s)
- `junk <item>` — destroy item
- `purge [target]` — remove mob/obj/player

**Movement:**
- `goto <vnum>` — teleport self
- `transfer <target> <vnum>` — move player
- `gtransfer <vnum>` — move all players
- `otransfer <obj> <vnum>` — move object
- `force <target> <cmd>` — command player
- `gforce <cmd>` — command all players
- `vforce <vnum> <cmd>` — command all mobs of vnum

**Control:**
- `at <vnum> <cmd>` — execute at target room
- `call <mpcode_vnum>` — call another script (max 5 nesting)
- `delay <ticks>` — schedule TRIG_DELAY
- `cancel <ticks>` — cancel scheduled delay
- `remove <item>` — unequip item

**Data:**
- `remember <npc> <keyword>` — memorize players
- `forget <npc>` — clear memory

### Conditionals Available (51+ checks)

**Randomization:**
- `rand <percent>` — random success

**World state:**
- `mobhere <name|vnum>`, `objhere <name|vnum>`
- `mobexists <name|vnum>`, `objexists <name|vnum>`
- `people > N`, `players > N`, `mobs > N`, `clones > N`
- `hour > N`
- `room $n == vnum`

**Character checks (target $n, $t, $r, $q):**
- `ispc`, `isnpc`, `isgood`, `isevil`, `isneutral`, `isimmort`
- `isactive`, `isvisible`, `istarget`, `exists`, `ischarm`, `isfollow`, `isdelay`, `hastarget`
- `carries $n <vnum|name>`, `wears $n <vnum|name>`
- `has $n <type>`, `uses $n <type>`
- `name $n == keyword`, `class $n == mage`, `race $n == human`, `clan $n == guild`
- `pos $n == standing`, `level $n >= 10`, `align $n < -1000`, `money $n > 1000`
- `sex $n == 0|1`, `hpcnt $n < 30`, `vnum $n == 1000`
- `affected $n blind`, `act $n sentinel`, `off $n berserk`, `imm $n fire`
- `objtype $p == scroll`, `objval0 $p > 100`
- `grpsize $n > 5`

### Variables Supported

Expansion codes available in all text fields (`$variable` substitution):
- `$i` / `$I` — mob name / description
- `$n` / `$N` — target name / description
- `$t` / `$T` — second target name / description
- `$r` / `$R` — random person in room name / description
- `$q` / `$Q` — mob's current target (mprog_target) name / description
- `$o` / `$O`, `$p` / `$P` — object names
- `$j`, `$e`, `$E` — pronouns (he/she/it)
- `$k`, `$m` — pronouns (him/her/it)
- `$l`, `$s` — pronouns (his/her/its)

---

## Part 2: Generation Strategy

### Design Philosophy

**Three-tier mob AI system:**

1. **Passive Behavior** (TRIG_GREET, TRIG_ENTRY)
   - How mob greets/reacts to players entering
   - Non-combat personality

2. **Combat AI** (TRIG_FIGHT, TRIG_HPCNT, TRIG_RANDOM, TRIG_SURR)
   - Primary combat tactics
   - Spell rotation, healing, retreat
   - Ally coordination

3. **Advanced Personality** (TRIG_DEATH, TRIG_KILL, TRIG_SPEECH)
   - Victory/death messages
   - Special reactions to keywords
   - Memorable interactions

### Mob Personality Archetypes

Generate scripts based on mob type & difficulty:

| Archetype | Playstyle | Key Triggers | Abilities |
|-----------|-----------|--------------|-----------|
| **Brute** | Aggressive melee | FIGHT → kill, HPCNT (low %): flee | Direct damage, no tactics |
| **Tactician** | Smart caster | FIGHT → assess, RANDOM → spell rotation, HPCNT: heal self | Spell selection, positioning |
| **Guardian** | Def support | FIGHT → assist allies, ENTRY: threaten, GREET: warn | Buffs, protects group |
| **Coward** | Flee-happy | HPCNT (high %): flee, SPEECH (help): run, SURR: beg | Minimal confrontation |
| **Summoner** | Reinforcements | DEATH → mload, FIGHT (at 50% HP): call allies | Spawn mobs, crowd control |
| **Berserker** | High-risk | FIGHT → aggressive, KILL → celebrate, SURR → bonus dmg | Wild attacks, no restraint |
| **Assassin** | Surprise burst | GREET → pick target, FIGHT → focus target, KILL: vanish | Kill weak targets fast |

### Difficulty Scaling

Adjust generated scripts based on mob level:

- **Level 1–10** (Novice): Basic attacks only, flee at 50% HP
- **Level 11–25** (Apprentice): Add simple spell rotation, heal at 30% HP
- **Level 26–40** (Expert): Add tactical switches (curse on spellcasters), ally assists
- **Level 41–50** (Master): Full AI — spell rotation, positioning, emergency tactics

---

## Part 3: Generation Implementation

### Architecture

```
pd_mobprog_generator.c (NEW)
├── pd_generate_mobprog_script() — main entry point
├── pd_generate_passive_behavior() — TRIG_GREET, TRIG_ENTRY scripts
├── pd_generate_combat_ai() — TRIG_FIGHT, TRIG_RANDOM, TRIG_HPCNT scripts
├── pd_generate_personality() — TRIG_DEATH, TRIG_KILL, TRIG_SPEECH scripts
├── pd_attach_mobprog_to_mob() — bind generated triggers to mob instance
└── helper functions:
    ├── pd_select_personality() — pick archetype
    ├── pd_generate_spell_rotation() — build spell list for archetype + level
    ├── pd_generate_conditional_block() — build if/and/or/endif structure
    └── pd_buffer_append_safe() — safe string concatenation with bounds

pd_mobprog_generator.h (NEW)
└── Function declarations + enums for personalities
```

### Function Prototypes

```c
/* Main entry point */
void pd_generate_and_attach_mobprog(CHAR_T *mob, PD_INSTANCE_T *instance);
  /* Called during mob spawning in pd_generate_instance()
   * Takes mob, difficulty, instance context
   * Generates scripts, attaches triggers, returns */

/* Generation functions */
char *pd_generate_mobprog_script(CHAR_T *mob, int personality, int difficulty);
  /* Returns allocated C string containing full mobprog code
   * Combines passive + combat + personality behaviors */

char *pd_generate_combat_ai(CHAR_T *mob, int personality, int difficulty);
  /* Returns combat-specific triggers (FIGHT, HPCNT, RANDOM, SURR) */

char *pd_generate_passive_behavior(CHAR_T *mob, int difficulty);
  /* Returns greeting/entry behavior (GREET, ENTRY) */

char *pd_generate_personality_behavior(CHAR_T *mob, int personality);
  /* Returns DEATH, KILL, SPEECH behaviors (death messages, taunts, etc.) */

/* Personality selection */
int pd_select_personality(int mob_vnum);
  /* Deterministic based on mob vnum; ensures consistency
   * Returns one of: PERSONALITY_BRUTE, PERSONALITY_COWARD, etc. */

/* Helpers */
void pd_attach_mobprog_to_mob(CHAR_T *mob, const char *code, 
                              int trig_type, const char *trig_phrase);
  /* Binds MPROG_LIST_T to mob->mob_index->mprog_first
   * Uses existing `mprog_new()` + `LIST2_FRONT()` */

char *pd_describe_spell_for_personality(int personality, int level);
  /* Returns spell name suitable for archetype + level */
```

### Generation Flow (Pseudo-code)

```c
void pd_generate_and_attach_mobprog(CHAR_T *mob, PD_INSTANCE_T *instance) {
    int personality = pd_select_personality(mob->mob_index->vnum);
    int difficulty = instance->level;
    
    /* Generate three behavior layers */
    char *passive = pd_generate_passive_behavior(mob, difficulty);
    char *combat = pd_generate_combat_ai(mob, personality, difficulty);
    char *personality_script = pd_generate_personality_behavior(mob, personality);
    
    /* Combine into single script */
    char *full_script = malloc(...);
    sprintf(full_script, "%s\n%s\n%s", passive, combat, personality_script);
    
    /* Attach to mob using existing mobprog system */
    pd_attach_mobprog_to_mob(mob, full_script, TRIG_FIGHT, "100");
    pd_attach_mobprog_to_mob(mob, full_script, TRIG_HPCNT, "30");
    pd_attach_mobprog_to_mob(mob, full_script, TRIG_RANDOM, "20");
    
    /* Clean up */
    free(passive);
    free(combat);
    free(personality_script);
    free(full_script);
}
```

---

## Part 4: Script Examples

### Example 1: Brute Archetype, Level 20

```
* ===== PASSIVE BEHAVIOR =====
if greet
  mob say Prepare to fight, punk!
endif

* ===== COMBAT AI =====
if fight
  mob echo The brute charges forward!
  mob kill $n
endif

if hpcnt $i < 25
  if rand 40
    mob say I'll crush you!!!
    mob damage $n 15
  endif
endif

if surr
  mob echo The brute roars defiantly!
  mob kill $n
endif

if rand 15
  mob damage $n 10
endif

* ===== PERSONALITY =====
if death
  mob echo The brute falls heavily!
endif

if kill
  mob say Another weakling down!
endif
```

### Example 2: Tactician Archetype, Level 35

```
* ===== PASSIVE BEHAVIOR =====
if greet
  if ispc $n
    mob say Ah, a challenger. Let me assess you.
  endif
endif

* ===== COMBAT AI =====
if fight
  mob say Interesting. Let's see how you handle this.
  mob kill $n
  mob delay 5
endif

if hpcnt $i < 40
  if rand 60
    mob say I need tactical repositioning!
    mob cast heal $i
    mob delay 3
  endif
endif

if hpcnt $i < 20
  if rand 80
    mob say Time for a desperate measure!
    mob cast lightning bolt $n
  endif
endif

if rand 20
  if ispc $n
    mob cast curse $n
  endif
endif

* ===== PERSONALITY =====
if death
  mob say Curious... I did not anticipate this outcome...
endif

if kill
  mob say Your tactics were inferior.
endif
```

### Example 3: Summoner Archetype, Level 40

```
* ===== PASSIVE BEHAVIOR =====
if entry
  if ispc $n
    mob echo The summoner looks at you intently...
  endif
endif

* ===== COMBAT AI =====
if fight
  mob say Come forth, my allies!
  mob mload 19920 2
  mob kill $n
  mob delay 10
endif

if hpcnt $i < 50
  if rand 40
    mob echo The summoner gestures wildly!
    mob mload 19921 1
  endif
endif

if hpcnt $i < 25
  if rand 90
    mob say I am besieged!
    mob mload 19920 3
    mob cast heal $i 30
  endif
endif

if rand 25
  mob cast curse $n
endif

* ===== PERSONALITY =====
if death
  mob echo Impossible! My allies have failed me!
  mob mload 19920 2
endif

if kill
  mob say My servants have proven victorious!
endif
```

---

## Part 5: Personality & Difficulty Data

### Personality Enum

```c
#define PERSONALITY_BRUTE        0   /* Simple melee attacks */
#define PERSONALITY_TACTICIAN    1   /* Smart spell rotations */
#define PERSONALITY_GUARDIAN     2   /* Defensive, supports others */
#define PERSONALITY_COWARD       3   /* Flees easily */
#define PERSONALITY_SUMMONER     4   /* Calls reinforcements */
#define PERSONALITY_BERSERKER    5   /* Aggressive, high-risk */
#define PERSONALITY_ASSASSIN     6   /* Burst damage, focus targets */
#define PERSONALITY_COUNT        7
```

### Difficulty Tiers

```c
/* In pd_mobprog_generator.c */
static const char *difficulty_spells[5][10] = {
    { NULL, NULL, ... },                             /* Level 1-10: none, basic */
    { "cure light", "blindness", "poison", NULL },   /* Level 11-25: healing, debuffs */
    { "heal", "curse", "lightning bolt", NULL },     /* Level 26-40: powerful spells */
    { "greater heal", "chain lightning", NULL },     /* Level 41-50: master spells */
    { NULL }
};

static const int flee_thresholds[5] = {
    50,  /* Level 1-10: flee at 50% */
    35,  /* Level 11-25: flee at 35% */
    25,  /* Level 26-40: flee at 25% */
    15,  /* Level 41-50: flee at 15% (brave) */
};

static const int cast_frequency[5] = {
    0,   /* Level 1-10: no casting */
    10,  /* Level 11-25: cast 10% per pulse */
    20,  /* Level 26-40: cast 20% per pulse */
    35,  /* Level 41-50: cast 35% per pulse (aggressive) */
};
```

---

## Part 6: Integration Points

### Where to Hook In

**1. During mob spawning** (`src/pocket_dungeon.c`, line ~480):
```c
/* After mob is loaded and stats are scaled */
if (!mob->mob_index->mprog_first) {
    pd_generate_and_attach_mobprog(mob, instance);
}
```

**2. Attach to specific triggers** (called from `pd_generate_and_attach_mobprog()`):
```c
/* Use existing mobprog system */
void pd_attach_mobprog_to_mob(CHAR_T *mob, const char *code,
                              int trig_type, const char *trig_phrase) {
    MPROG_LIST_T *mp;
    
    mp = mprog_new();
    mp->vnum = /* use pocket dungeon code vnum bucket */;
    mp->code = code;  /* direct pointer to string */
    mp->trig_type = trig_type;
    mp->trig_phrase = str_dup(trig_phrase);
    
    LIST2_FRONT(mp, mob_prev, mob_next,
                mob->mob_index->mprog_first,
                mob->mob_index->mprog_last);
}
```

**3. Data storage** (in `json/config/pocket_dungeon_seeds.json`):
```json
{
  "name": "crypt",
  "mobprog_enabled": true,
  "mobprog_personality_override": null,  /* null = auto-select per mob vnum */
  "mobprog_difficulty_boost": 0           /* +/- from instance level */
}
```

---

## Part 7: Files to Modify/Create

### New Files
- [ ] `src/pocket_dungeon_mobprog.c` — NEW (generation + attachment logic)
- [ ] `src/pocket_dungeon_mobprog.h` — NEW (function declarations)

### Modified Files
- [ ] `src/pocket_dungeon.c` — MODIFY (call `pd_generate_and_attach_mobprog()` after loading each mob)
- [ ] `src/pocket_dungeon.h` — MODIFY (declare new function)
- [ ] `json/config/pocket_dungeon_seeds.json` — MODIFY (add `mobprog_enabled` flag)

---

## Part 8: Testing & Validation

### Test Cases

1. **Script Syntax Validation**
   - Generate script, log to file
   - Parse with mobprog parser (verify no syntax errors)
   - Ensure max nesting (12 levels) not exceeded
   - Check variable references ($n, $i, etc.) are valid

2. **Trigger Attachment Verification**
   - Confirm mob->mob_index->mprog_first is non-null
   - Verify each trigger type bound (FIGHT, HPCNT, RANDOM)
   - Check trig_phrase format (percentages are numeric)

3. **Behavior Observation**
   - Spawn mob in test instance; observe combat
   - Verify personality archetype behavior manifests
   - Check spell rotation matches difficulty tier
   - Confirm flee thresholds work

4. **Load Testing**
   - Generate 100 mobs with varied personalities
   - Monitor memory (no leaks in mprog string buffers)
   - Check CPU impact of script execution

### Success Criteria

- [ ] All 7 archetypes generate valid scripts (no syntax errors)
- [ ] Scripts execute without crashing
- [ ] Mobs demonstrate personality-appropriate behavior
- [ ] Memory allocated for scripts is cleaned up properly
- [ ] No conflicts with existing mobprog system
- [ ] Difficulty scaling produces observable behavior changes

---

## Part 9: Limitations & Future Enhancements

### Current Limitations (Phase 1)

- Scripts generated at runtime; no persistence (recreated on respawn)
- Limited personality customization (7 archetypes; could be expanded)
- No inter-mob communication (mobs don't chat, coordinate beyond combat)
- Simple spell selection (no tactical spell sequencing)
- No NPC memory system integration

### Phase 2 Enhancements

- **Persistence**: Save generated scripts for later replay
- **Dynamic personalities**: Allow seed JSON to specify custom personality blends
- **Mob communication**: Mobs could use `asound` to coordinate
- **Tactical sequences**: Build multi-turn spell combos
- **NPC memory**: Mobs remember players across sessions
- **Learning**: Mobs adapt tactics based on player behavior (aggressive vs. defensive)

---

## Part 10: Related Decisions

### Decision: Where to store generated code

**Options:**
1. **In-memory strings** (current plan) — Fast, no I/O, ephemeral
2. **Global MPROG_CODE_T pool** — Reusable across instances, persistent
3. **Instance-local pool** — Scoped to this instance, auto-cleanup on despawn

**Chosen**: Option 1 (in-memory) + Option 2 (global cache for common scripts)
- _Rationale_: Instant generation, low overhead; common archetypes cached for performance

### Decision: Personality selection method

**Options:**
1. **Deterministic** (based on mob vnum) — Consistent replays
2. **Random per instance** — More varied gameplay
3. **Seed-specified** — Builder chooses for each theme

**Chosen**: Deterministic + seed override
- _Rationale_: Predictable testing; builders can override via JSON if desired

---

## Status: FINALIZED — Ready for Phase 4

**Next Step**: Integrate this standalone plan into the main Phase 2 refinements plan. Mobprog generation will be Feature C5 in the implementation order.

