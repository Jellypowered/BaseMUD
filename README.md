# BaseMUD — Jelly's Fork

**A ROM 2.4b6 / QuickMUD codebase rebuilt for clarity, JSON-driven content, and extensibility.**

This is a personal fork of [Synival's BaseMUD](https://github.com/Synival/BaseMUD), itself a thorough code review and overhaul of [QuickMUD](https://github.com/avinson/rom24-quickmud). New systems added in this fork include a full banking system, a wear-slot expansion, and a procedural group-instancing feature (Pocket Dungeon) with a companion web editor.

**Branch**: [`Dungeon`](https://github.com/Jellypowered/BaseMUD/tree/Dungeon)  
**Upstream**: [github.com/Synival/BaseMUD](https://github.com/Synival/BaseMUD)

---

## Quick Start

### Native (MSYS2 / MinGW on Windows, or GCC on Linux)

```
make
./run.sh
```

### Docker

```
docker-compose build
docker-compose up
```

Mounts `player/`, `log/`, and `json/areas/` for live editing.

```
telnet localhost 4000
```

A level-60 Implementor account is included: name `Admin`, password `Admin`. **Change the password immediately.**

---

## Legal / Credits

BaseMUD inherits from the following codebases, each with their own license requirements:

| License | File |
|---------|------|
| DikuMUD | `doc/license.doc` |
| Merc 2.1 | `doc/license.txt` |
| ROM 2.4 | `doc/rom.license` |

```
ROM 2.4 copyright (c) 1993-1995 Russ Taylor
MERC 2.1 by Hatchet, Furey, and Kahn
DikuMUD by Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert, and Sebastian Hammer
```

Integrated add-ons (as with QuickMUD):

- OLC 1.81
- Lope's Color 2.0
- Erwin's Copyover
- Erwin's Noteboard
- Color Login

---

## What Synival's BaseMUD Added

Synival's work transformed the stock ROM/QuickMUD codebase before this fork existed. Key contributions:

- **Full JSON import/export** — all areas, rooms, mobs, objects, resets, portals, and config tables are stored in `json/` and loaded at boot. The `.are` files are a secondary backup written by OLC save; JSON is the live format.
- **Area-number (anum) system** — within-area relative addressing so areas can be rearranged without vnum conflicts. Cross-area references use `{ "area": "name", "anum": N }`.
- **Portal system** — named two-way and one-way exits defined in `json/config/portals.json`, decoupling cross-area connections from raw vnum references.
- **Thorough code cleanup** — tabs → spaces everywhere, redundant comments removed, tables (spells, races, classes) laid out as readable tabular data, variables and functions renamed for clarity.
- **`#define BASEMUD_*` feature flags** — optional gameplay changes in `src/basemud.h`:
  - `BASEMUD_SHOW_DOORS` — open/closed doors visible in auto exits
  - `BASEMUD_SHOW_ARRIVAL_DIRECTIONS` — direction of arrival shown on movement
  - `BASEMUD_GRADUAL_RECOVERY` — HP/mana/move regenerates every pulse instead of on ticks
  - `BASEMUD_CAP_JOINED_AFFECTS` — stacking affects cap at strongest modifier instead of adding
  - `BASEMUD_ALLOW_STUNNED_MOBS` — mobs can be incapacitated/mortally wounded
  - `BASEMUD_MOBS_SAY_SPELLS` — caster mobs announce their spells like players
  - `BASEMUD_MATERIALS_COMMAND` — `materials` command shows object/character materials
  - `BASEMUD_DISENGAGE_COMMAND` — `disengage` lets spellcasters stop fighting to recover mana
  - `BASEMUD_PIXIE_RACE` — adds the Pixie race (flying, tiny, detect good/evil, frail)
  - Several damage display, condition display, and QoL flags
- **Bash/trip rework** — knockdown states have cooldowns; standing back up is no longer instant.
- **`lore` command** — works like `identify` but reveals information based on skill %, seeded by player name + vnum.
- **`dump` command** — `dump stats`, `dump world raw`, `dump world json` write to `dump/`.
- **OLC fixes** — stock zones save cleanly round-trip; confirmed via diff against `.dump` files.
- Various ROM bug fixes (fire/ice breath hunger bug, heat metal dex inversion, scan visibility, door lock sync, etc.)

See `Readmes/SynivalREADME.md` for the full upstream changelog.

---

## Features Added in This Fork

### Banking System

A full in-game bank ported from TAKA's ROM 2.4b6 banking snippet and integrated natively.

- `balance` — check gold and silver account balances
- `deposit` / `withdraw` — transfer between inventory and bank account
- `transfer <player> <amount>` — player-to-player gold transfers
- ATM mobs (`MOB_ATM` flag) bypass business hours; standard Banker mobs (`MOB_BANKER`) observe opening/closing hours configured in `json/config/banking_config.json`
- Configurable daily ATM withdrawal limits, silver deposit/conversion enable flags, and transfer enable flag
- Balances persist in the player save file (`balance` and `sbalance` fields on `PC_DATA`)

**Config**: `json/config/banking_config.json`

---

### Wear Slot Expansion

Six new equipment slots added to the wear-location system, with the carry-cap decoupled from slot count so adding slots does not silently change inventory limits.

| New Slot | Purpose |
|----------|---------|
| `back` | Containers only (backpacks, satchels). Contents excluded from carry count/weight while worn. |
| `cloak` | Cloaks, capes, shawls (separated from body armor). |
| `eyes` | Goggles, spectacles, monocles, patches. |
| `ears` | Earrings, ear cuffs. |
| `float2` | Second floating object slot. |
| `tattoo` | New `tattoo` item type for permanent-feeling cosmetic/magical effects. |

Area objects in the stock world (cloaks, backpacks, eyewear, earrings, orbs) were audited and migrated to the appropriate new slots across 20+ area files. MUDEditor's equipment modal, room-reset dropdown, wear-flag picker, and object-editor item-type list were all updated to match.

---

### Pocket Dungeon (Procedural Group Instancing)

The largest feature added to the fork. Pocket Dungeon generates private, temporary dungeon instances on demand for a player or group, themed and scaled to the group's gear level.

#### Player Commands

```
dungeon list              — show available themes
dungeon create [theme]    — create a themed dungeon (costs gold unless testing_mode)
dungeon enter             — enter a waiting instance
dungeon leave             — return to your origin room
dungeon rejoin            — re-enter an instance you were part of (e.g. after death)
dungeon status            — view your current instance info
dungeon help              — command summary
```

#### How It Works

1. **Gear-based scaling** — the instance level is calculated from the group's equipped gear score, not character level. This makes instancing fair for twinks and veterans alike.
2. **Procedural layout** — five layout algorithms (`linear`, `spiral`, `hub`, `ruins`, `cavern`) produce distinct dungeon shapes based on the seed's `layout_style`.
3. **Themed content** — each seed (`crypt`, `cave`, `ruins`, `mountain_frost`, etc.) carries its own pool of room names, room descriptions, mob vnums, item vnums, boss, sentinel, and hidden-cache configuration, all authored in JSON.
4. **Three loot tiers**:
   - Floor drops (every Nth room) — `PD_QUALITY_FLOOR`
   - Guarded chest (mid-dungeon with sentinel) — `PD_QUALITY_CHEST`
   - Boss drop (final room, with optional authored heirloom vnum) — `PD_QUALITY_BOSS`
5. **Procedural item enhancement** — `pd_enhance_obj()` applies tier-appropriate affixes, charges, spell levels, and armor/weapon bonuses to template objects at generation time.
6. **Loot themes** — spell pools and stat-apply pools for generated consumables and jewelry are defined in `json/config/pd_loot_themes.json` and resolved at boot. Each theme links to the matching seed.
7. **Mob affixes** — generated mobs can receive modifiers: `Swift` (+speed), `Armored` (+AC), `Ancient` (+density), `Cursed`, `Luminous`, etc.
8. **Procedural mobprogs** — instance mobs receive runtime-generated AI scripts (fight tactics, HP-threshold healing, flee logic, taunts) based on their type and role.
9. **Hidden caches** — `ITEM_HIDDEN` objects placed in side rooms, findable with `search` or `detect hidden`. Seeds carry hint-keyword pools for immersive clue text.
10. **`search` skill** — new player skill to find hidden objects. Wands/scrolls of search are spawned in instances.
11. **`sip` command** — sipping a potion identifies only that item (removes `ITEM_HIDDEN`), without revealing the rest of the room's hidden content.
12. **Instance lifecycle** — instances auto-purge on a configurable timer once empty. If a PC corpse is in the instance, purge is suppressed until the corpse is retrieved.

#### JSON Data Files

| File | Purpose |
|------|---------|
| `json/config/pocket_dungeon_config.json` | Global limits: max instances, vnum range, testing mode, etc. |
| `json/config/pocket_dungeon_seeds.json` | Per-theme seed data: layout, rooms, mobs, boss, loot, hints |
| `json/config/pd_loot_themes.json` | Per-theme spell/stat pools and boss heirloom vnum |
| `json/areas/pocketdungeon/` | Mob and object templates used by all instances |

#### MUDEditor Integration

The companion web editor at [github.com/Jellypowered/BaseMUD/tree/Dungeon](https://github.com/Jellypowered/BaseMUD/tree/Dungeon) (under `MUDEditor/`) surfaces:

- **Pocket Dungeon page** — seed editor, config editor, loot-theme editor, live instance snapshot viewer with minimap
- **Live instances tab** — reads the server's `/api/pocket-dungeon/instances` snapshot; shows active rooms, mobs, exits, and group members without needing to log in-game
- **Seed editor** — full CRUD for seeds: room pools, mob pools, boss/sentinel config, layout style, sector, hidden hints

---

### Boot Crash Fixes

Three boot-time crash causes were identified and fixed:

1. **`skill_clear_mapping` OOB write** — loop used `SKILL_MAX` (300) instead of `skill_count` on a dynamically-sized array, writing past the end.
2. **CRLF JSON files on Windows** — `fopen("r")` translated `\r\n→\n` in text mode, causing a `ftell`/`fread` size mismatch that silently discarded JSON config files (including `skills.json`). Fixed by using `fopen("rb")`.
3. **Load-order crash (`pd_loot_themes.json` before `skills.json`)** — `pd_loot_themes.json` sorts alphabetically before `skills.json`, so `skill_lookup_exact()` was called while `skill_table` was still NULL. Fixed by storing spell names as strings during parse and resolving them in a post-load hook (`pd_loot_themes_reload_spells()`) called from `skill_reload_mapping`.

---

### Other Changes

- **Greeting variants** — four distinct login screen greetings in `json/config/greetings.json` instead of four copies of the same message.
- **`BASEMUD_LOG_FILES_LOADED`** — `.are` files already covered by JSON are logged as "Ignoring loaded area" at boot (cosmetic; can be silenced by undefining the flag in `src/basemud.h`).
- **`boot.log` excluded** from git tracking.

---

## Project Layout

```
src/           C source files
json/          JSON content (loaded at boot — this is the live data)
  areas/       Area-local rooms, mobs, objects, resets
  config/      Config tables, pocket dungeon, banking, portals, skills, etc.
  help/        Help text entries
area/          Legacy .are files (written by OLC save as backup; not loaded if JSON exists)
MUDEditor/     Companion web editor (React + Express + TypeScript)
doc/           Design docs: Pocket_Dungeon_System.md, Json_Documentation.md, etc.
plan/          Active work-in-progress plans
CompletedPlans/ Completed implementation plans (audit trail)
bin/           Compiled executable
```

---

## MUDEditor Web UI

The companion area/config editor lives under `MUDEditor/web/`. It is a separate npm workspace:

```
web/client/     React + Vite + Tailwind SPA
web/server/     Express API + static file server
web/shared/     TypeScript types shared by client and server
```

### Running it

```
cd MUDEditor/web
npm install
cd shared && npm run build
cd ../server && node dist/index.js   # or use Docker
```

Or via Docker Compose inside `MUDEditor/web/`:

```
docker-compose up
```

The editor is designed to be served behind a Caddy reverse proxy at a sub-path (e.g. `/editor/`), with `X-Forwarded-Prefix` header handling for correct asset and API routing.

---

## Optional Feature Flags (`src/basemud.h`)

All `#define BASEMUD_*` flags from upstream are preserved. Flags specific to this fork:

| Flag | Effect |
|------|--------|
| `BASEMUD_LOG_FILES_LOADED` | Log "Ignoring loaded area" for .are files skipped due to JSON coverage |

---

## License

This project inherits all license requirements from DikuMUD, Merc 2.1, and ROM 2.4. See `doc/` for the full license texts. You must comply with all three if you use this code.
