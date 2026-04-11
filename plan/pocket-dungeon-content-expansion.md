# ✅ PLAN COMPLETE
# Pocket Dungeon Content Expansion

## Status: COMPLETED

## Scope
- Expand the Pocket Dungeon JSON seed pools with more room names, room descriptions, and hidden-hint text for every existing theme.
- Add new Pocket Dungeon mob templates in `json/areas/pocketdungeon/mobiles.json` at unused vnums so the seed pools have more distinct encounter options.
- Add new Pocket Dungeon object templates in `json/areas/pocketdungeon/objects.json` at unused vnums so the seed pools have more loot, chest, cache, and search-support variation.
- Keep all changes within the existing schema; do not add new fields or change C code.

## Files To Modify
- `json/config/pocket_dungeon_seeds.json` - expand per-theme room-name and room-description pools, add hidden-cache hint arrays, and widen mob/item vnum arrays to reference the new templates.
- `json/areas/pocketdungeon/mobiles.json` - add new themed mob templates on unused Pocket Dungeon vnums.
- `json/areas/pocketdungeon/objects.json` - add new Pocket Dungeon containers, search-support items, and loot items on unused vnums.

## Risks / Notes
- The seed pools must stay within the current loader limits: room names <= 20, mob vnums <= 10, item vnums <= 10, room descriptions <= 15, hidden hints <= 5.
- Hidden caches only spawn when `hide_hint_count > 0`, so the new hint arrays must stay aligned across keywords, look text, and hint phrases.
- Support-object vnums must match the seed config exactly or the generator will fall back to missing content at runtime.
- No C/runtime changes are required because the JSON loader already supports the existing seed fields.

## Status: COMPLETED

2026-04-11
