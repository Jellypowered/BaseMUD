# Plan: Pocket Dungeon Refinements (Full Scope)

**Status:** PHASE 1 COMPLETE - Foundation building blocks ready. Phases E-J pending implementation.

---

## COMPLETED PHASES

### ✅ Phase A — Foundation
- vnum_size: 100 → 105 in config
- All 3 seeds: room_count 4-8 → 25-100
- `pd_calc_gear_score()`: Sums equipped item levels for mixed-gear group scaling
- `pd_calc_instance_level()`: Uses gear scores + scaling_formula (average or max)
- Instance creation now uses real gear-based level instead of `members[0]->level`
- **Status**: Builds cleanly ✓

### ✅ Phase B — Layout Variety
- Added `layout_style` field to `struct pd_seed`
- Implemented `pd_build_layout()` dispatcher with 5 algorithms:
  - `linear`: North chain + east alcoves (crypt)
  - `spiral`: Cardinal directions + UP/DOWN stairs (cave) 
  - `hub`: Central hub with 4 radii + branches (for future themes)
  - `ruins`: Grid with cross-connects and vertical depth (ruins)
  - `cavern`: Organic tree from spine with branches (default)
- Replaces hardwired room linking in pd_generate_instance()
- All 3 existing seeds assigned layout styles in JSON
- JSON loading updated to read layout_style
- **Status**: Builds cleanly ✓

### ✅ Phase C+D — Mob Density Scaling & Expanded pd_seed
- **Struct expansions (`struct pd_seed`):**
  - `mob_density_min`, `mob_density_max` (replaces flat `mob_density`)
  - `sector_type`: SECT_INSIDE, SECT_FOREST, etc.
  - `outdoors`: bool to control ROOM_INDOORS flag
  - Boss fields: `boss_vnum`, `boss_level_add`, `boss_room_name`
  - Sentinel fields: `sentinel_vnum`, `sentinel_level_add`
  - Container fields: `container_vnum`, `hidden_container_vnum`
  - Search items: `search_scroll_vnum`, `search_wand_vnum`
  - Room descriptions: `struct pd_room_desc[15]`, `room_descs`, `room_desc_count`
  - Entry/chest room name overrides
  - Hidden object hints: `hide_keywords[]`, `hide_look_texts[]`, `hide_hint_phrases[]`, `hide_hint_count`
- Updated JSON loader (`json_tblr.c`) to read all new fields from JSON
- **Status**: Builds cleanly ✓
- **Ready for**: Theme expansion with all new seed data

### ✅ Phase F (partial) — ITEM_HIDDEN Flag
- Renamed `ITEM_UNUSED_FLAG_2` → `ITEM_HIDDEN` (BIT_24) in `src/flags.h`
- Updated flag entry in `src/flags.c` with name "hidden" and TRUE
- Updated `char_can_see_obj()` in `src/chars.c` to check `ITEM_HIDDEN` against `AFF_DETECT_HIDDEN`
- Hidden objects now invisible unless viewer has detect_hidden buff
- **Status**: Builds cleanly ✓
- **Ready for**: Loot spawning with hidden objects, do_search command

---

## PENDING PHASES (Implementation order)

### Phase E — Boss Placement (NEXT)
- **Goal**: Spawn boss mobs in deepest rooms with unique stats
- **Tasks**:
  - Add logic to pd_generate_instance() to place boss mob in rooms[room_count-1]
  - Boss level = instance_level + seed->boss_level_add
  - Scale boss using pd_scale_mob_to_level()
  - Add boss_room_name override to generated room
  - Exclude boss room from regular mob density spawning

### Phase F (completion) — Loot & Hidden Objects  
- **Goal**: Spawn tiered loot, hidden caches with sentinels
- **Tasks**:
  - Create chest guardians (sentinel mobs) in dedicated room
  - Spawn treasure chests (container objects) with items inside
  - Spawn hidden caches (flagged with ITEM_HIDDEN) with search keywords
  - Implement 3-tier loot: open chests, sentinel-guarded, hidden + per-room floor drops
  - Add hint phrases to room descriptions for hidden loot discovery

### Phase G — do_search Command
- **Goal**: MUD-wide search skill to find hidden objects
- **Tasks**:
  - Add SKILL_MAP_SEARCH to src/defs.h (bump SKILL_MAP_MAX to 54)
  - Register in tables.c skill_map_table[]
  - Implement do_search() in act_move.c (pattern: follows do_hide logic)
  - Add skill JSON entry to skills.json
  - Modify spell_detect_hidden to reveal hidden items via pd_do_hidden_scan()

### Phase H — 14 Themes JSON Expansion
- **Goal**: Add 11 new themes + expand 3 existing to full seed data
- **Tasks**:
  - Expand pocket_dungeon_seeds.json with all new fields for each seed
  - Create 11 new themes: mountain, stronghold, castle, forest, den, marsh, island, plains, skyreach, temple, dungeon_depths
  - Define theme-specific boss mobs (19906-19919), sentinel mobs, containers, search items
  - Add unique room descriptions, hidden object keywords, and loot variance
  - All 14 themes → distinct layout + sector + boss + loot profile

### Phase I — MUDEditor Snapshot Updates  
- **Goal**: Show real mob names + objects in Instances tab
- **Tasks**:
  - Update snapshot JSON to include full mob/object data (not just counts)
  - Modify pd_write_snapshot() to serialize mob names + HP, object names + flags
  - Update MUDEditor types to include `PocketDungeonInstanceMob` and `PocketDungeonInstanceObject`
  - Update Instances tab UI to display real data with object HIDDEN badge

### Phase J — MUDEditor Seed Editor UI
- **Goal**: Full expandable seed editor with all new fields
- **Tasks**:
  - Expand `PocketDungeonSeed` type with all new struct fields
  - Create sectioned seed editor (Identity, Layout, Mobs, Boss, Loot, Room Descs, etc.)
  - Multi-entry list editor for room descriptions, hidden hints
  - Update EMPTY_SEED constant with sensible defaults
  - CheatSheet documentation for new fields

---

---

## Phase A — Foundation

### A1 — vnum_size + room count

| File | Change |
|---|---|
| `json/config/pocket_dungeon_config.json` | `vnum_size: 100` → `105` |
| `src/pocket_dungeon.c` | Change local `rooms[100]` array → `rooms[105]`; replace hardcoded cap with `UMIN(pd_config.vnum_size - 1, 104)` |
| `json/config/pocket_dungeon_seeds.json` | `room_count_min: 25, room_count_max: 100` on all three existing seeds |

### A2 — Gear score + real group scaling

New static functions in `src/pocket_dungeon.c`:

- `pd_calc_gear_score(CHAR_T *ch)` — iterates worn objects via `ch->content_first`, sums `obj->level` for equipped items + 1 per `AFFECT_T` on each item; returns effective level clamped to `[ch->level, ch->level * 2]`
- `pd_calc_instance_level(CHAR_T **members, int count)` — per-member gear score, then applies `pd_config.scaling_formula` (0 = average, 1 = max). Replaces the current `members[0]->level` hardcode.

---

## Phase B — Layout Variety Per Theme

### B1 — New `layout_style` field in pd_seed

Add `char *layout_style` to `struct pd_seed` in `src/structs.h`.

### B2 — `pd_build_layout()` dispatcher in `src/pocket_dungeon.c`

Replaces the current hard-wired north-chain + east-branch loop. Dispatches by `layout_style`:

| Style | Themes | Description |
|---|---|---|
| `"linear"` | crypt, tower, catacombs | N/S main chain; dead-end E alcoves for side rooms. Deterministic, easy to navigate. |
| `"spiral"` | mountain, cave | Uses all 4 cardinal dirs + occasional UP/DOWN stair rooms. Winds through the level. |
| `"hub"` | stronghold, castle | Central hub room with N/E/S/W radial branches, each branch a short corridor. |
| `"ruins"` | ruins, island, plains | Sprawling grid with frequent horizontal cross-connects and occasional UP/DOWN (second floors). |
| `"cavern"` | cavern, marsh, forest, den | Organic tree layout — branches split from a main spine, many UP/DOWN, winding paths. |

Spatial consistency rules (all styles):
- Exits are always paired: N↔S, E↔W, U↔D bidirectionally
- No exit mismatches (east from A to B always means west from B to A)
- `rooms[0]` = entry/foyer; `rooms[main_count-1]` = boss room
- No exit loops back to the same room

### B3 — Sector types and ROOM_INDOORS

Outdoor themes (`ruins`, forest/marsh/mountain/island/plains/cavern) — drop `ROOM_INDOORS`, set `sector_type` from seed field:

| Theme | sector_type constant |
|---|---|
| forest | `SECT_FOREST` (3) |
| mountain | `SECT_MOUNTAIN` (5) |
| island | `SECT_WATER_SWIM` (6) |
| marsh, plains, ruins | `SECT_FIELD` (2) |
| cavern, den, cave, crypt, tower, catacombs, stronghold, castle | `SECT_INSIDE` (0) |

New `bool outdoors` field in `struct pd_seed` — if TRUE, skip `SET_BIT(room_flags, ROOM_INDOORS)`.

---

## Phase C — Mob Density Scaling

### C1 — New seed fields

Add to `struct pd_seed` in `src/structs.h`:
```c
int mob_density_min;   /* mobs per room at level 1 (default 1) */
int mob_density_max;   /* mobs per room at max level (default 3) */
/* existing mob_density retained for backwards-compat but ignored */
```

### C2 — Scaled spawn loop in `src/pocket_dungeon.c`

Per-room density uses `int_interpolate(inst->level, 1, 50)` scaled between `mob_density_min` and `mob_density_max`.

Room skip chance: `if (inst->level < 20 && number_percent() < (20 - inst->level))` → skip mobs this room entirely.

Boss room and chest sentinel room are excluded from the density loop.

### C2a — Per-room depth scaling

As you progress deeper (farther from entry), mobs gain a difficulty bonus:

- Calculate room depth: `int depth = pathfind_distance(rooms[0], rooms[i])` (BFS from entry)
- Depth multiplier: `float depth_mult = 1.0 + (depth / (float)room_count * 0.5)` → ranges [1.0, 1.5]
- Apply to all mobs spawned in that room: `mob->level = (int)(base_level * depth_mult + 0.5)`

Boss room mobs are always scaled to boss level; depth scaling does not apply there.

### C3 — seeds JSON update

Add `mob_density_min: 1, mob_density_max: 3` to all seeds in `json/config/pocket_dungeon_seeds.json`.

---

## Phase D — Greatly Expanded pd_seed

### D1 — New struct fields in `src/structs.h`

```c
/* Room variety */
#define PD_MAX_ROOM_DESCS  15
struct pd_room_desc {
    char *text;           /* room description body text */
    char *look_keyword;   /* optional "look <keyword>" trigger (NULL if none) */
    char *look_text;      /* text seen when looking at keyword */
};
struct pd_room_desc room_descs[PD_MAX_ROOM_DESCS];
int room_desc_count;

/* Special room overrides */
char *boss_room_name;
char *entry_room_name;
char *chest_room_name;

/* Layout */
char *layout_style;
int  sector_type;
bool outdoors;

/* Density */
int mob_density_min;
int mob_density_max;

/* Boss */
int boss_vnum;
int boss_level_add;

/* Sentinel (chest guard — tougher than regular, weaker than boss) */
int sentinel_vnum;
int sentinel_level_add;

/* Loot containers */
int container_vnum;
int hidden_container_vnum;

/* Hidden cache hints (parallel arrays) */
#define PD_MAX_HIDE_HINTS 5
char *hide_keywords[PD_MAX_HIDE_HINTS];
char *hide_look_texts[PD_MAX_HIDE_HINTS];
char *hide_hint_phrases[PD_MAX_HIDE_HINTS];
int  hide_hint_count;

/* Magic search items */
int search_scroll_vnum;
int search_wand_vnum;
```

### D2 — Generator changes in `src/pocket_dungeon.c`

Room creation loop:
- Pick `room_descs[number_range(0, room_desc_count-1)]` instead of hardcoded description
- If desc has `look_keyword`: allocate `EXTRA_DESCR_T` via `extra_descr_new()`, set keyword/description, call `extra_descr_to_room_index_back(ed, room)`
- `rooms[0]->name` = `seed->entry_room_name` (fallback "dungeon foyer")
- `rooms[main_count-1]->name` = `seed->boss_room_name` (fallback "inner sanctum")

### D3 — JSON loading in `src/json_tblr.c`

Parse all new fields. `room_descs` is a JSON array of objects with `text`, optional `look_keyword`, optional `look_text`.

### D4 — Seed JSON content

Each of the 14 seeds gets ≥8 `room_descs` entries, at least 3 with `look_keyword`/`look_text`, themed to their environment.

---

## Phase E — Boss System

### E1 — Boss placement in `pd_generate_instance()`

If `seed->boss_vnum != 0`:
- Create boss mob in `rooms[main_count-1]`
- `pd_scale_mob_to_level(boss, inst->level + seed->boss_level_add)`
- `SET_BIT(boss->off_flags, OFF_SENTINEL)` — stays in room
- Override room name to `seed->boss_room_name`
- Regular mob density loop skips `rooms[main_count-1]`

### E2 — Boss mob prototypes

One new mob prototype per theme added to the template area JSON.
All 14 boss mobs carry unique `short_descr`/`long_descr`; stat values are overridden at runtime by `pd_scale_mob_to_level`.
All boss mobs have `ACT_SENTINEL` in the template.

Vnum allocation: **19906–19919** (14 bosses × 1)

| Theme | Vnum | Boss name |
|---|---|---|
| crypt | 19906 | The Ancient Crypt Keeper |
| cave | 19907 | The Cave Troll Warchief |
| ruins | 19908 | The Ruined Warlord |
| forest | 19909 | The Ancient Treant |
| marsh | 19910 | The Bog Witch |
| plains | 19911 | The Warlord of the Plains |
| stronghold | 19912 | The Iron Warden |
| castle | 19913 | The Undead Lord |
| cavern | 19914 | The Deep Horror |
| den | 19915 | The Alpha Predator |
| island | 19916 | The Pirate Captain |
| mountain | 19917 | The Mountain Giant |
| tower | 19918 | The Archmage |
| catacombs | 19919 | The Lich |

---

## Phase F — Loot Containers, Hidden Chests, and Chest Sentinels

### F1 — ITEM_HIDDEN flag (prerequisite for F2, G, I)

| File | Change |
|---|---|
| `src/flags.h` | Rename `ITEM_UNUSED_FLAG_2` (BIT_24) → `ITEM_HIDDEN` |
| `src/flags.c` | Entry `"unused_extra_2"` → `"hidden"`, set `TRUE` (builder-accessible) |
| `src/chars.c` | In `char_can_see_obj()` after the `ITEM_INVIS` block (~line 508): add `if (IS_SET(obj->extra_flags, ITEM_HIDDEN) && !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)) return FALSE;` |

### F2 — Three loot tiers in `pd_generate_instance()`

**Tier 1 — Open loot chest** (1 per dungeon, at `rooms[room_count/4]`):
- `obj_create(seed->container_vnum, inst->level)` — create the chest
- Place `loot_density` loot items inside via `obj_give_to_obj(item, chest)`
- `obj_give_to_room(chest, room)`

**Chest sentinel** (if `seed->sentinel_vnum != 0`):
- `mobile_create()` → `pd_scale_mob_to_level(sentinel, inst->level + seed->sentinel_level_add)`
- `SET_BIT(sentinel->off_flags, OFF_SENTINEL)`
- `char_to_room(sentinel, chest_room)`
- Regular mob density loop skips the chest room

**Tier 2 — Hidden cache** (1 per dungeon, random side room, if `seed->hidden_container_vnum != 0`):
- `obj_create(hidden_container_vnum, inst->level)` → `SET_BIT(obj->extra_flags, ITEM_HIDDEN)`
- Pick random index from `seed->hide_hint_count`; use `hide_keywords[h]`, `hide_look_texts[h]`, `hide_hint_phrases[h]`
- Append hint phrase to room description via `str_replace_dup` concat
- Allocate `EXTRA_DESCR_T` with the keyword/text; `extra_descr_to_room_index_back(ed, side_room)`
- Put loot items inside hidden container; `obj_give_to_room(hidden_container, side_room)`

**Tier 3 — Floor drops** (existing loop, every 5th room instead of every 3rd):
- Keep cycling `item_vnums[]`, drop directly to room

### F3 — Container object prototypes (template area)

| Vnum | Description | Flags |
|---|---|---|
| 19960 | "a worn wooden chest" | `ITEM_CONTAINER`, `CONT_CLOSEABLE\|CONT_CLOSED` |
| 19961 | "a hidden cache" | `ITEM_CONTAINER` (spawned with `ITEM_HIDDEN` by generator) |

All 14 seeds get `container_vnum: 19960, hidden_container_vnum: 19961`.

---

## Phase G — `do_search` Command (MUD-Wide) + Magical Search Items

### G1 — Skill registration

| File | Change |
|---|---|
| `src/defs.h` | Add `#define SKILL_MAP_SEARCH 53`, bump `SKILL_MAP_MAX` to `54` |
| `src/tables.c` | Add `{SKILL_MAP_SEARCH, "search"}` to `skill_map_table[]` |

### G2 — `do_search` implementation in `src/act_move.c` (after `do_hide`)

- NPC bail
- Skill < 1 bail: "You search around making lots of noise.\n\r"
- `WAIT_STATE(ch, 24)` — prevents spamming
- Roll `number_percent() < char_get_skill(ch, SN(SEARCH))`
- On success: iterate `ch->in_room->content_first → content_next`; for each `ITEM_HIDDEN` obj: print "You reveal [short_descr]!", `REMOVE_BIT(obj->extra_flags, ITEM_HIDDEN)`, set `found = TRUE`
- Success with nothing found: "You searched everywhere but found nothing unusual.\n\r"
- Failure: "You search carefully but uncover nothing unusual.\n\r" + fail-improve

Also:
- `src/act_move.h`: `DECLARE_DO_FUN(do_search);`
- `src/interp.c`: `{"search", do_search, POS_RESTING, 0, LOG_NORMAL, 1}` (alphabetical, after "say", before "sell")

### G3 — Skill class levels in `json/config/skills.json`

Add entry (alphabetical between "scroll" and "shield block"):
- thief: level 5, effort 4
- warrior: level 15, effort 6
- mage: level 53, effort 0 (effectively unavailable — they use items)
- cleric: level 53, effort 0

### G4 — Magical search items (for mages/clerics)

`spell_detect_hidden` already exists in `src/spell_aff.c`. These items use it:

- `obj vnum 19989` — "a scroll of detection" (`ITEM_SCROLL`, `spell_fun: "spell_detect_hidden"`)
- `obj vnum 19990` — "a wand of seeking" (`ITEM_WAND`, charges 3–5, `spell_fun: "spell_detect_hidden"`)

Additionally, modify `spell_detect_hidden` in `src/spell_aff.c`: after applying the `AFF_DETECT_HIDDEN` affect, call `pd_do_hidden_scan(ch)` — a new shared helper in `pocket_dungeon.c/h` that rolls `number_percent() < UMAX(50, char_get_skill(ch, SN(SEARCH)))` and reveals hidden objects in the room. Characters with the search skill get their skill% chance; others get 50%.

`pd_do_hidden_scan()` is also called internally by `do_search` to share the reveal logic.

### G5 — Help and JSON

- `json/help/search.json` — new help entry for SEARCH command
- Both search items added to relevant seeds' `item_vnums[]` pools and `search_scroll_vnum`/`search_wand_vnum` fields

---

## Phase H — 14 Themes (11 New)

Full theme table:

| Theme | layout_style | sector_type | outdoors | Boss vnum | Sentinel vnum | Notes |
|---|---|---|---|---|---|---|
| crypt | linear | 0 (INSIDE) | no | 19906 | ? | Dark, bone-strewn, undead |
| cave | cavern | 0 (INSIDE) | no | 19907 | ? | Wet rock, fungi, underground |
| ruins | ruins | 2 (FIELD) | yes | 19908 | ? | Collapsed walls, overgrown |
| forest | cavern | 3 (FOREST) | yes | 19909 | ? | Dense undergrowth, clearings |
| marsh | cavern | 2 (FIELD) | yes | 19910 | ? | Mud, mist, reed beds |
| plains | ruins | 2 (FIELD) | yes | 19911 | ? | Open grassland, earthworks |
| stronghold | hub | 0 (INSIDE) | no | 19912 | ? | Military compound |
| castle | hub | 0 (INSIDE) | no | 19913 | ? | Great hall, towers, throne |
| cavern | cavern | 0 (INSIDE) | no | 19914 | ? | Deep underground, pools |
| den | linear | 0 (INSIDE) | no | 19915 | ? | Beast lair, nesting |
| island | ruins | 6 (WATER_SWIM) | yes | 19916 | ? | Beach, cove, jungle |
| mountain | spiral | 5 (MOUNTAIN) | yes | 19917 | ? | Cliffs, passes, caves |
| tower | linear | 0 (INSIDE) | no | 19918 | ? | Ascending floors, labs |
| catacombs | linear | 0 (INSIDE) | no | 19919 | ? | Sealed crypts, ritual rooms |

Sentinel mob vnums TBD — new mobs to be created in template area alongside boss mobs.
Regular mob pairs (2 per theme): vnums 19920–19947 (11 new themes × 2), supplementing existing 19900–19905.

---

## Phase I — MUDEditor Snapshot Fixes

### I1 — C: Add mob names + objects to snapshot (`src/pocket_dungeon.c` — `pd_write_snapshot()`)

Replace the scalar `mob_count` block with a `mobs` array:
```c
"mobs": [{"vnum": N, "name": "short_descr", "hp": N, "max_hp": N}, ...]
"mob_count": N  // retained for backwards compat
```

Add a new `objects` array per room:
```c
"objects": [{"vnum": N, "name": "short_descr", "hidden": true/false}, ...]
```

### I2 — TypeScript: New types in `web/shared/types/index.ts`

```ts
interface PocketDungeonInstanceMob { vnum: number; name: string; hp: number; max_hp: number; }
interface PocketDungeonInstanceObject { vnum: number; name: string; hidden: boolean; }
```

Update `PocketDungeonInstanceRoom`:
```ts
mobs: PocketDungeonInstanceMob[];       // new
objects: PocketDungeonInstanceObject[];  // new
mob_count: number;                       // kept for compat
```

### I3 — React: Update `InstancesTab` in `web/client/src/pages/PocketDungeonPage.tsx`

**Mobiles section:**
- Replace room-name placeholders with real data: `selected.rooms.flatMap(r => r.mobs ?? [])`
- Tree entries: show `mob.name` with HP indicator
- Detail table columns: Vnum | Name | HP | Room

**Objects section:**
- Replace hardcoded "Not tracked in snapshot" with real list: `selected.rooms.flatMap(r => r.objects ?? [])`
- Tree count badge uses actual count
- Show `obj.name` with amber "HIDDEN" pill for hidden objects

Defend against old snapshots: `r.mobs ?? []` and `r.objects ?? []`.

---

## Phase J — MUDEditor Seed Editor Expansion

### J1 — Expanded `PocketDungeonSeed` type in `web/shared/types/index.ts`

```ts
interface PocketDungeonSeedRoomDesc {
  text: string;
  look_keyword?: string;
  look_text?: string;
}

interface PocketDungeonSeed {
  // existing fields
  name: string; title: string; room_names: string[];
  mob_vnums: number[]; item_vnums: number[];
  room_count_min: number; room_count_max: number;
  mob_density: number; loot_density: number;
  // new fields
  room_descs: PocketDungeonSeedRoomDesc[];
  layout_style: string;
  sector_type: number;
  outdoors: boolean;
  boss_vnum: number; boss_level_add: number;
  boss_room_name: string; entry_room_name: string; chest_room_name: string;
  mob_density_min: number; mob_density_max: number;
  sentinel_vnum: number; sentinel_level_add: number;
  container_vnum: number; hidden_container_vnum: number;
  search_scroll_vnum: number; search_wand_vnum: number;
}
```

### J2 — Sectioned seed editor in `web/client/src/pages/PocketDungeonPage.tsx`

Replace the current flat form with labeled sections:

| Section | Fields |
|---|---|
| **Identity** | name (slug), title |
| **Layout** | layout_style (dropdown), sector_type (number), outdoors (checkbox), room_count_min/max |
| **Mobs** | mob_vnums, mob_density_min/max, sentinel_vnum, sentinel_level_add |
| **Boss** | boss_vnum, boss_level_add, boss_room_name, entry_room_name, chest_room_name |
| **Loot** | item_vnums, loot_density, container_vnum, hidden_container_vnum, search_scroll_vnum, search_wand_vnum |
| **Room Descriptions** | Multi-entry list: each row has text textarea + optional look_keyword + look_text inputs; add/remove buttons |
| **Room Names (legacy)** | Existing room_names textarea |

Update `EMPTY_SEED` constant with all new fields and sensible defaults.
Update CheatSheet with entries for new fields.

### J3 — Server note

`pocketDungeonSeedsRouter` uses the generic `makeNameRouter` (JSON passthrough). No server changes required — it saves whatever the client sends.

---

## All Files Modified / Created

### BaseMUD (C)

| File | Change |
|---|---|
| `src/structs.h` | `pd_room_desc` struct; all new `pd_seed` fields (see Phase D) |
| `src/pocket_dungeon.c` | `pd_build_layout()`, `pd_calc_gear_score()`, `pd_calc_instance_level()`, `pd_do_hidden_scan()`; boss, sentinel, container, hidden-chest logic; room variety; expanded snapshot JSON |
| `src/pocket_dungeon.h` | Declare `pd_do_hidden_scan()` |
| `src/flags.h` | `ITEM_UNUSED_FLAG_2` → `ITEM_HIDDEN` |
| `src/flags.c` | Flag name + TRUE |
| `src/chars.c` | `char_can_see_obj()` — add ITEM_HIDDEN check vs AFF_DETECT_HIDDEN |
| `src/spell_aff.c` | `spell_detect_hidden` — call `pd_do_hidden_scan(ch)` after applying buff |
| `src/defs.h` | `SKILL_MAP_SEARCH 53`, `SKILL_MAP_MAX 54` |
| `src/tables.c` | Add `search` to `skill_map_table[]` |
| `src/act_move.c` | `do_search` implementation |
| `src/act_move.h` | `DECLARE_DO_FUN(do_search)` |
| `src/interp.c` | Register `search` command |
| `src/json_tblr.c` | Load all new seed fields from JSON |
| `json/config/pocket_dungeon_config.json` | `vnum_size: 105` |
| `json/config/pocket_dungeon_seeds.json` | Full overhaul — 14 seeds, all new fields |
| `json/config/skills.json` | Add `search` skill entry |
| Template area JSON | Boss mobs 19906–19919; new theme mobs 19920–19947; sentinel mobs; chest 19960; hidden cache 19961; scroll 19989; wand 19990 |
| `json/help/search.json` | New — SEARCH command help |
| `json/help/dungeon.json` | Update — new themes, boss, hidden loot, search mechanic |
| `json/help/credits.json` | New credit entry |

### MUDEditor (TypeScript/React)

| File | Change |
|---|---|
| `web/shared/types/index.ts` | Expanded `PocketDungeonSeed`; new `PocketDungeonInstanceMob`, `PocketDungeonInstanceObject`; updated `PocketDungeonInstanceRoom` |
| `web/client/src/pages/PocketDungeonPage.tsx` | Sectioned seed editor; real mob/object data in Instances tab |

---

## Verification Checklist

1. Build → zero errors/warnings
2. `dungeon enter cave` at level 1 → 25–100 rooms; MUDEditor map view shows organic layout
3. Each theme produces a visually distinct layout (hub vs linear vs cavern)
4. Level 1 char with level-20 gear → effective instance level raises above 1
5. Chest room has sentinel mob guarding it; opening chest reveals loot items inside
6. Hidden cache room has hint text in description; `search` reveals the container; `detect hidden` shows it
7. Boss in deepest room: unique name, unique appearance, significantly harder than regular mobs
8. No regular mobs in boss room or chest room
9. Mobs deeper into dungeon have higher levels than mobs near entry (depth scaling visible)
10. `search` in plain room → "nothing unusual" + skill improve roll
11. Scroll of detection: use → scan roll + persistent detect_hidden buff
11. MUDEditor Seeds tab shows all new fields in organized sections; round-trip save works
12. MUDEditor Instances tab Mobiles list shows mob names + HP; Objects list shows items with HIDDEN badge
13. All 14 themes listed in `dungeon enter` command with unique titles
14. `help search` returns correct help text

---

## Phase Dependencies

```
A (gear score, vnum size) ──────────────────── ALL depend on A
F1 (ITEM_HIDDEN flag) ──────────────────────── F2, G, I depend on F1
B (layout algorithms) ────── independent of C/D/E/F
C (mob density) ──────────── depends on A
D (pd_seed expansion) ───────── depends on A; B's layout_style is part of D
E (boss) ─────────────────── depends on A, D
F2/F3 (loot) ─────────────── depends on A, D, F1
G (do_search) ─────────────── depends on F1
H (14 themes) ─────────────── depends on B, D, E, F
I1 (snapshot C) ──────────── independent
I2/I3 (snapshot UI) ─────── depends on I1
J (seed editor UI) ──────── depends on D types
```

---

## Scope Boundaries (Deferred)

- Boss-specific guaranteed loot table
- `do_search` revealing hidden characters (AFF_HIDE system) — objects only for now
- Splitting `detect_hidden` / `detect_invis` into fully separate spell mechanics
