# Persistent Cartography System Plan

**Date:** 2026-04-17
**Scope:** Redesign map from stateless wilderness-only display to persistent, area-based cartography with items and progress tracking.

---

## Summary

Implement a persistent mapping system that works in all rooms, tracks player discovery per area across sessions, supports shareable snapshot map items, and uses skill-based map quality/accuracy.

This plan intentionally supersedes wilderness-only behavior in `do_map` for mortals.

---

## Confirmed Requirements (User Decisions)

- Mapping is allowed in **all rooms** (not wilderness-only).
- Mapping persists per player and per area, and continues when re-entering an area.
- Storage uses **separate per-player map files** (outside core player save file).
- Blank map workflow:
  - Blank map is purchased (Midgaard general store, cost 5 silver).
  - Blank map is consumed when first writing a map for an area.
  - Result is a written map item that can be read/traded.
- Mapping toggle exists (`map toggle`) and controls whether recording occurs.
- If mapping is on but no blank map is available for the area, show a **one-time message** per player+area.
- Skill quality affects:
  - Display radius
  - Diagonal (NE/SE/SW/NW) visibility
  - Sector accuracy (low skill can misidentify sectors)
- Completion indicator required: `mapped/total` and `Incomplete` title until 100%.
- Diagonal directions should render as diagonal offsets.
- U/D exits are acknowledged as difficult in 2D and will be represented in legend/metadata, while still counting for discovery/completion.
- Add thematic sector colors (brown dirt, blue water, etc.).
- Map item behavior:
  - Item stores room-vnum snapshot in `extra_descr` (`map_data`).
  - Trading a map gives a snapshot (not live-sync updates).
  - Multiple maps for same area merge when in one inventory.
- Command UX:
  - `map` uses player's persistent discovery data for current area.
  - `read map` shows held map item's snapshot.

---

## Files To Modify

### `src/act_map.c`
- Replace wilderness gate with all-area logic.
- Add persistent discovery-aware rendering path (current area only).
- Add skill-tier effects:
  - Radius by skill tier.
  - Diagonal visibility unlock by skill tier.
  - Sector misidentification chance at low skill.
- Add completion title formatting (`Incomplete` + mapped/total).
- Add U/D indicator handling in legend text.
- Add `map toggle` subcommand support and one-time missing-blank warning integration.
- Add themed color palette mapping by sector.

### `src/act_map.h`
- Add declarations for new cartography helpers exposed to movement/save layers.

### `src/act_move.c`
- Hook movement success path to record room discovery when mapping is enabled.
- Ensure recording runs for all movement directions including diagonals and U/D.

### `src/interp.c`
- Ensure command table supports `map` subcommands and `read map`/equivalent reader command routing.

### `src/flags.h`
- Add/assign player extended flag for mapping toggle (`PLR_MAPPING` or equivalent ext flag).

### `src/flags.c`
- Register mapping flag name for display/edit where appropriate.

### `src/structs.h`
- Add in-memory structs for per-area discovery sets and one-time warning state cache.
- Add pointers/containers on player character struct.

### `src/save.c`
- Load/save per-player mapping state to dedicated map file path.
- Persist discovered rooms by area and one-time warning state as needed.

### `src/players.c`
- Initialize/cleanup mapping structures on login/logout/character extraction.

### `src/lookup.h` and/or parser helpers (`src/json*.c` as needed)
- Add helper interfaces for map file read/write and area-room totals.

### `src/act_obj.c` (or relevant item command handler)
- Implement `read map` behavior for map items.
- Parse/validate `map_data` extra description payload.
- Merge duplicate area map items in inventory.

### `json/config/skills.json`
- Tune `map` skill progression/effect assumptions to match new tier model.

### Area/Shop data under `json/` (Midgaard)
- Add purchasable blank map object.
- Add object to Midgaard general store inventory with 5 silver price.

### `json/help/map.json`
- Update help for toggle, blank map workflow, progress/completion, and map reading.

### `.github/agents/cheatsheet.md`
- Record verified implementation details and pitfalls discovered during integration.

### `json/help/credits.json`
- Confirm/update contribution credit entry if this redesign includes notable sourced behavior updates.

---

## Files To Create

### `map/<player>.json` runtime data files (new persisted data domain)
Per-player cartography save files containing:
- Area identifier
- Discovered room vnums
- Optional last render metadata
- One-time warning state (no blank map warning per area)

### `src/cartography.c` and `src/cartography.h` (if needed for separation)
Optional extraction if `act_map.c` becomes too large; contains persistence, merge, and query helpers.

---

## Data Model (Planned)

Per-player map file schema (JSON, conceptual):
- `version`
- `areas[]`
  - `name` (area key)
  - `rooms[]` (unique room vnums discovered)
  - `warned_no_blank` (bool)

Map item payload schema in `extra_descr` keyword `map_data`:
- `area=<name>;vnums=1,2,3,...`

Note: validate max length and split into multiple chunks if needed to avoid extra description overflow.

---

## Skill Tier Behavior (Planned)

- Tier 1 (low skill): small radius, no diagonals, higher sector error chance.
- Tier 2 (mid skill): medium radius, no diagonals, reduced error chance.
- Tier 3 (high skill): large radius, diagonals enabled, near-accurate sectors.
- Tier 4 (master): largest radius, diagonals enabled, fully accurate sectors.

Exact thresholds and percentages to be finalized during implementation based on `char_get_skill` values.

---

## Color Theme Plan

Use themed ANSI color mapping by sector for readability:
- field: light green
- forest: green
- hills/mountain: yellow/brown variants
- desert/dirt: brown/yellow
- water_swim/water_noswim: blue/light blue
- city/inside: white/gray
- air: cyan
- unknown/unmapped: blank
- player marker: red

Ensure compatibility with existing BaseMUD color token system (`{x`, `{g`, `{G`, `{y`, `{B`, `{b`, `{W`, `{D`, `{C`, `{R`).

---

## Risks

- Item `extra_descr` size limits may constrain large-area snapshots.
- Area total-room counting must align with authoritative area room index source.
- Movement hook must avoid performance regression on every room transition.
- Diagonal/U/D rendering interactions can create ambiguous map topology.
- Save/load path must be robust for missing/corrupt per-player map files.

---

## Verification Plan

- Build with workspace `build` task.
- Functional checks:
  - Toggle on/off behavior and one-time warning.
  - Blank map consume/create flow.
  - Discovery persists across logout/login.
  - Area re-entry continues prior map progress.
  - Progress text and `Incomplete` title state.
  - Diagonal visibility unlock by skill tier.
  - Color rendering by sector type.
  - `read map` snapshot trade behavior and same-area merge.

---

## Status: IN PROGRESS
