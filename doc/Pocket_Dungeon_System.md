# Pocket Dungeon System

Pocket Dungeon is BaseMUD's procedural instancing system for short-lived, themed dungeon runs. It exists to give groups repeatable dungeon content without hand-building a full static area for every run, while still letting builders author the flavor, encounter pools, and difficulty knobs in JSON.

The system is intentionally split across the MUD server and MUDEditor:
- the MUD owns generation, combat, instance lifecycle, and live snapshot output
- MUDEditor owns schema-aware editing of Pocket Dungeon config and seed JSON, plus a live read-only view of running instances
- shared TypeScript types keep the editor aligned with the C structs and snapshot layout

## Purpose

Pocket Dungeon is meant to provide:

- repeatable group content with themed variation
- a safe place to tune encounter density, boss difficulty, and loot pacing
- authored room flavor instead of one repeated generic room description
- procedural mob behavior through generated mobprogs
- a live inspection surface in MUDEditor so the output can be verified without logging into the game
- a testing mode for development and content iteration

It is not a separate world-building pipeline. It is a live runtime feature built on top of the normal room, mobile, object, and JSON systems already used by BaseMUD.

## End-to-End Flow

### 1. Boot and load the Pocket Dungeon data

At boot, BaseMUD loads the Pocket Dungeon defaults from JSON into global runtime structs:

- `src/globals.c` provides the fallback `pd_config` values
- `src/json_tblr.c` reads `json/config/pocket_dungeon_config.json` into `pd_config`
- `src/json_tblr.c` also reads `json/config/pocket_dungeon_seeds.json` into the linked list of `pd_seed` records
- `src/recycle.c` initializes and disposes seed objects so the pooled strings and arrays are managed safely

The config singleton has a dedicated reload helper in `src/json_import.c` (`json_reload_pd_config()`), so the MUD can reload `pocket_dungeon_config.json` without a full restart.

Pocket Dungeon seed data is loaded through the normal JSON import path. The current code does not expose a seed-specific reload helper, so seed edits written by MUDEditor become authoritative on disk immediately but are only reflected in memory when the server imports that JSON again.

### 2. A player uses the `dungeon` command

The player-facing entry point is `src/act_instance.c`, which implements `do_dungeon()`.

The command supports:

- `dungeon help`
- `dungeon list`
- `dungeon status`
- `dungeon enter`
- `dungeon create [theme]`
- `dungeon leave`
- `dungeon rejoin`

The important flows are:

- `dungeon enter` picks a random seed and creates a free instance
- `dungeon create [theme]` creates a themed instance and charges gold unless `testing_mode` is enabled
- `dungeon leave` returns the character to the original room or the temple fallback
- `dungeon rejoin` returns a player to an existing instance they were part of
- `dungeon list` shows the available seed names and titles

The command also enforces basic runtime limits before generation:

- it refuses to create a dungeon if the player is already inside one
- it counts active instances and blocks creation once `max_instances` is reached
- it gathers same-group members so a group can enter together
- it remembers the origin room so the return portal can go back to the correct place

### 3. The generator builds the instance

The core generator is `pd_generate_instance()` in `src/pocket_dungeon.c`.

That function:

- chooses the requested seed or a random seed if no theme was provided
- validates the seed with `pd_validate_seed()`
- finds a free instance slot and computes the vnum base for that slot
- allocates a live area named `pd-inst-<slot>`
- creates the room set for the instance
- spawns mobs, boss, sentinel, loot, and hidden cache content
- assigns affixes and boss powers
- generates mobprogs when the seed enables them
- writes a snapshot file for MUDEditor

The generated instance uses the reserved vnum range from `pd_config.vnum_base` and `pd_config.vnum_size`. The first room is the entry room; the later rooms use the same slot-relative vnum block.

### 4. Rooms are assembled from the seed's authoring pools

Room generation uses the seed's authored content instead of a single fixed sentence.

The room text path is:

- pick a room name from `room_names`
- capitalize the name for presentation
- pick a room description from `room_descs`
- append a layout-specific atmosphere clause so repeated rooms do not read identically
- attach room extra descriptions when the seed provides a `look_keyword` / `look_text` pair

If a seed has no `room_descs`, the generator falls back to a safe generic line and logs a warning. That keeps legacy or malformed seed data working, but the authored description pool is the intended path.

### 5. Encounters and loot are spawned from the pool data

Pocket Dungeon seeds control the active content pools for the instance:

- `mob_vnums` supply the base mobile templates used in normal rooms
- `item_vnums` supply loot templates used in chests, hidden caches, and floor drops
- `boss_vnum` supplies the boss template
- `sentinel_vnum` supplies the chest guardian template
- `container_vnum` supplies the visible treasure chest
- `hidden_container_vnum` supplies the hidden cache object
- `search_scroll_vnum` and `search_wand_vnum` support the search/detection side of the system
- `mobprog_enabled`, `mobprog_personality_override`, and `mobprog_difficulty_boost` control procedural mobile behavior

After each item is created, `pd_enhance_obj()` in `src/pd_loot.c` enhances it according to the spawn location tier and the active loot theme:

- **Floor drops** receive 1 affix (tier 1)
- **Chest loot** receives 2 affixes (tier 2)
- **Boss drops** receive 3 affixes (tier 3); if the theme defines a `boss_drop_vnum`, that authored heirloom item is also created unenhanced at `inst->level+5`

The stat and spell pools for each seed are defined in `json/config/pd_loot_themes.json`, keyed by seed name. A `"default"` entry covers seeds with no dedicated theme entry.

The mob and item pools are intentionally theme-specific. Expanding the number of variants per theme is part of the system's design, but only within the fields the schema already supports.

The generator also applies live runtime modifiers:

- affixes affect mob HP, haste, density, healing, dodge, and loot odds
- boss powers are assigned before the boss spawns
- mob difficulty scales with instance level and distance from the entry room
- the hidden-cache path can add extra room descriptions for search hints

### 6. Live instance data is written out as snapshot JSON

When generation completes, `pd_write_snapshot()` in `src/pocket_dungeon.c` writes a debug snapshot under the temp areas directory using a filename like:

- `temp/areas/pd-inst-<instance-id>-<timestamp>.json`

That snapshot contains the fields MUDEditor needs to render the live instance:

- slot and instance id
- seed/theme name
- instance level
- creation and empty timestamps
- entry vnum
- area name
- member list
- room list
- per-room descriptions
- room exits
- mob counts
- live mobs
- mobprog trigger summaries per mob
- objects and hidden-object status

Snapshots are written on creation and removed when the instance is destroyed.

### 7. MUDEditor reads and edits the same Pocket Dungeon data

MUDEditor treats Pocket Dungeon as three connected surfaces:

- the global config singleton
- the seed list
- the live instance snapshots

The client-side API wrappers are in `MUDEditor/web/client/src/lib/api.ts`:

- `pocketDungeonConfigApi` talks to `/api/config/pocket-dungeon-config`
- `pocketDungeonSeedsApi` talks to `/api/config/pocket-dungeon-seeds`
- `pocketDungeonInstancesApi` talks to `/api/pocket-dungeon/instances`

The server routes that back those endpoints live in `MUDEditor/web/server/src/routes/config.ts`:

- `pocketDungeonConfigRouter()` handles the singleton config object
- `pocketDungeonSeedsRouter()` handles named seed CRUD
- `pocketDungeonInstancesRouter()` serves read-only live snapshots from the temp areas directory

The shared TypeScript contract is in `MUDEditor/web/shared/types/index.ts`. It mirrors the C structs and snapshot layout so the editor can render the same fields the MUD writes.

## Key Data Surfaces

### Pocket Dungeon config

File: `json/config/pocket_dungeon_config.json`

This file feeds the global `pd_config` singleton. It controls the live runtime limits and debugging behavior.

| Field | Meaning |
| --- | --- |
| `autopurge` | Destroy empty instances automatically after the timeout |
| `empty_timeout_mins` | Minutes before an empty instance is purged |
| `max_instances` | Hard cap on simultaneously loaded instances |
| `vnum_base` | First vnum in the reserved live-instance range |
| `vnum_size` | Number of vnums reserved per instance slot |
| `max_members` | Maximum players allowed in one instance |
| `scaling_formula` | Difficulty scaling mode |
| `testing_mode` | Free entry mode for development and testing |
| `gold_cost_per_level` | Entry cost multiplier per character level |
| `show_room_vnums` | Gate for the room-vnum suffix in look output |

`show_room_vnums` is the explicit fix for the room-number visibility issue. When it is off, builders and immortals still see the room title, but the bracketed room-number suffix stays hidden.

### Pocket Dungeon loot themes

File: `json/config/pd_loot_themes.json`

Each `pd_loot_theme` record is keyed by `name` and matched to the running instance's seed at item-generation time. The `"default"` entry is the fallback for seeds with no dedicated entry.

| Field | Meaning |
| --- | --- |
| `name` | Matches the seed name (`inst->theme`); `"default"` = global fallback |
| `title` | Display label in MUDEditor |
| `spell_pool` | Named spells drawn from for potions, scrolls, pills, and wands |
| `stat_pool` | Named `APPLY_*` constants drawn from for equipment and jewelry affixes |
| `item_vnums` | Reserved for future per-theme item pool override |
| `wand_charges_min` / `wand_charges_max` | Charge range rolled for generated wands |
| `potion_level_min` / `potion_level_max` | Spell level clamped for generated potions |
| `boss_drop_vnum` | Authored heirloom vnum created at boss kill; 0 = none |

### Procedural item enhancement

All spawned items pass through `pd_enhance_obj()` immediately after creation. Enhancement is tier-driven by spawn location:

| Location | Tier | Affixes | Naming prefix |
| --- | --- | --- | --- |
| Floor drop | 1 | 1 | none |
| Chest / hidden cache | 2 | 2 | `"a fine "` |
| Boss drop | 3 | 3 | `"a rare "` |

Consumables (potions, scrolls, pills, wands, staves) are created with `ITEM_UNIDENTIFIED` set instead of receiving a naming prefix. While the flag is set, `look <item>` shows a generic label. Players can identify consumables through:

- `sip <potion>` — taste-tests a potion; chance to identify based on INT, WIS, and lore skill
- `lore <item>` — skill-gated identification that clears the flag at ≥60% knowledge
- `identify` spell — always clears the flag

### Pocket Dungeon seeds

File: `json/config/pocket_dungeon_seeds.json`

Each `pd_seed` record is a themed content pack. It is the main authoring surface for room flavor, themed encounter pools, and instance-specific tweaks.

Important fields include:

| Field | Meaning |
| --- | --- |
| `name` | Stable seed id used by `dungeon create <theme>` and editor selection |
| `title` | Display title for the seed/theme |
| `layout_style` | Room-flow style used by the generator |
| `sector_type` | Sector assignment for generated rooms |
| `outdoors` | Whether the area should be treated as outdoors |
| `room_names` | Pool of generated room names |
| `room_descs` | Pool of authored room descriptions |
| `mob_vnums` | Base mobile templates used for normal mob spawns |
| `item_vnums` | Base object templates used for loot |
| `room_count_min` / `room_count_max` | Room-count range for the instance |
| `mob_density` / `mob_density_min` / `mob_density_max` | How many mobs appear in rooms |
| `loot_density` | How much loot gets distributed |
| `boss_vnum` / `boss_level_add` | Boss template and level offset |
| `sentinel_vnum` / `sentinel_level_add` | Chest guardian template and level offset |
| `container_vnum` | Visible chest template |
| `hidden_container_vnum` | Hidden cache template |
| `search_scroll_vnum` / `search_wand_vnum` | Search/detection support items |
| `entry_room_name` / `boss_room_name` / `chest_room_name` | Special room name overrides |
| `mobprog_enabled` | Turns procedural mobprog generation on or off |
| `mobprog_personality_override` | Fixed mobprog personality or auto-select when null |
| `mobprog_difficulty_boost` | Extra difficulty applied when generating mob behavior |
| `hide_keywords` / `hide_look_texts` / `hide_hint_phrases` | Hidden-cache flavor and search hints |

The room description pool is the same idea as the room-name pool: it is a content authoring pool, not a computed field. The generator cycles through it and layers layout flavor on top.

### Live instance snapshots

The live snapshot format is not hand-authored. It is emitted by the MUD at runtime and consumed by MUDEditor as read-only inspection data.

Each snapshot room can include:

- `name`
- `description`
- `exits`
- `mob_count`
- `mobs[]`
- `objects[]`

Each live mob can include:

- vnum
- display name
- hp and max hp
- mobprog trigger summaries

That means MUDEditor is not just showing counts. It can show what is actually in the room, what the mobprogs are, and which objects are hidden.

## Relevant Files

### BaseMUD server

| File | Role |
| --- | --- |
| `src/act_instance.c` | Implements the `dungeon` command and player entry/exit flow |
| `src/pocket_dungeon.c` | Generates instances, rooms, encounters, loot, affixes, and snapshots |
| `src/pocket_dungeon_mobprog.c` | Generates and attaches procedural mobprogs |
| `src/pocket_dungeon.h` | Public Pocket Dungeon interface and affix / boss power enums |
| `src/pocket_dungeon_mobprog.h` | Mobprog helper declarations |
| `src/pd_loot.c` | Procedural item enhancement engine (`pd_enhance_obj`, theme lookup, tier bonuses) |
| `src/pd_loot.h` | Public loot enhancement declarations including `pd_loot_theme_get()` |
| `src/json_tblr.c` | Loads Pocket Dungeon config, seed, and loot theme JSON into runtime structs |
| `src/json_import.c` | Provides the `json_reload_pd_config()` helper |
| `src/recycle.c` | Initializes and disposes seed/runtime Pocket Dungeon data safely |
| `src/structs.h` | Defines `pd_config`, `pd_seed`, `pd_instance`, `PD_LOOT_THEME_T`, and related structs |
| `src/globals.c` | Supplies the fallback Pocket Dungeon defaults |
| `src/flags.h` | Defines `ITEM_UNIDENTIFIED` (BIT_28) |
| `src/act_obj.c` | Implements `do_sip` (stat-gated potion taste-test) |
| `src/act_info.c` | Controls room-header output, look description suppression for unidentified items, and `do_lore` |
| `src/spell_info.c` | `spell_identify_perform_seeded()` clears `ITEM_UNIDENTIFIED` at ≥60% knowledge |
| `src/fight.c` | Triggers boss loot when a Pocket Dungeon boss dies |

### Pocket Dungeon data files

| File | Role |
| --- | --- |
| `json/config/pocket_dungeon_config.json` | Global Pocket Dungeon tuning defaults |
| `json/config/pocket_dungeon_seeds.json` | Theme and content pools for generated instances |
| `json/config/pd_loot_themes.json` | Per-seed loot themes: spell/stat pools, charge ranges, boss heirloom vnums |
| `json/areas/pocketdungeon/` | Template mobs and objects referenced by seed vnums |

### MUDEditor

| File | Role |
| --- | --- |
| `MUDEditor/web/shared/types/index.ts` | Shared Pocket Dungeon TypeScript interfaces |
| `MUDEditor/web/client/src/pages/PocketDungeonPage.tsx` | Config, seeds, loot themes, and live instance UI |
| `MUDEditor/web/client/src/lib/api.ts` | Client wrappers for Pocket Dungeon API routes |
| `MUDEditor/web/server/src/routes/config.ts` | Reads/writes Pocket Dungeon JSON and serves snapshots |
| `MUDEditor/web/client/src/hooks/useFlagsConfig.ts` | Hardcoded extra_flags list — includes `unidentified` (BIT_28) |

### Reference docs

| File | Role |
| --- | --- |
| `doc/Json_Documentation.md` | Field-by-field JSON schema reference, including `pocket_dungeon_config` |

## How the MUD and MUDEditor interact

| Direction | Path | What happens |
| --- | --- | --- |
| MUDEditor -> filesystem | `pocketDungeonConfigApi`, `pocketDungeonSeedsApi`, `pocketDungeonLootThemesApi` | The editor saves schema-safe Pocket Dungeon JSON back to the config files |
| filesystem -> MUD | `json_tblr_pd_config`, `json_tblr_pd_seed`, `json_tblr_pd_loot_theme` | The MUD reads those JSON files into the runtime structs |
| MUD -> filesystem | `pd_write_snapshot()` | The MUD writes the current live instance snapshot into `temp/areas/` |
| filesystem -> MUDEditor | `pocketDungeonInstancesApi` | The editor reads the snapshots and shows rooms, mobs, objects, and mobprogs |

The important point is that MUDEditor is not inventing live state. It is editing source-of-truth JSON for config and seeds, then rendering the runtime snapshot the MUD already wrote.

## Player-Facing Behavior

Pocket Dungeon uses the normal MUD command layer, not a separate editor-only path.

The player flow is:

1. Use `dungeon list` to see the available themes
2. Use `dungeon enter` to enter a random theme for free
3. Use `dungeon create <theme>` to request a specific theme and pay the entry cost unless testing mode is on
4. Use `dungeon status` to inspect the active instance
5. Use `dungeon leave` to return to the origin room or the temple fallback
6. Use `dungeon rejoin` to return to an instance you were previously part of

Room names are presented as generated room titles. The room-vnum suffix is only shown when `show_room_vnums` is enabled, and only to the builder / immortal audience that already qualifies for it.

## Authoring and Extension Rules

If you are writing Pocket Dungeon content, keep these rules in mind:

- use existing JSON fields only
- expand `room_names`, `room_descs`, `mob_vnums`, `item_vnums`, and related existing pools instead of inventing new structures
- if a requested variant needs a field that does not exist, stop and ask for the schema/runtime change instead of guessing
- keep seed content theme-consistent so the generator can cycle through it without producing repetitive output
- remember that live instances are derived from the seed JSON and the generator code, not from MUDEditor state alone

For new content work, the safest pattern is:

1. author the JSON content in the seed files
2. verify the shared types and MUDEditor UI can represent it
3. confirm the MUD generator already reads or exports the field
4. only then extend the schema or code if absolutely required

## Reload and Deployment Notes

- `pocket_dungeon_config.json` is hot-reloadable through the dedicated config reload path
- seed JSON currently follows the normal JSON import path and should be treated as a persisted data source rather than a free-form runtime scratchpad
- live snapshots are transient and should be treated as inspection-only output
- if the generator changes, re-check both the MUD-side snapshot writer and the MUDEditor instance viewer so they stay in sync

## See Also

- `doc/Json_Documentation.md` for the JSON schema reference of `pocket_dungeon_config` and `pd_loot_theme`
- `MUDEditor/web/client/src/pages/PocketDungeonPage.tsx` for the current editor UI contract
- `src/pocket_dungeon.c` and `src/pocket_dungeon_mobprog.c` for the runtime generation logic
