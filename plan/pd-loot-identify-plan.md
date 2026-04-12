✅ PLAN COMPLETE — Pocket Dungeon: PD Loot Enhancer + Identify/Sip Integration

## Summary
Add a conservative procedural enhancer for Pocket Dungeon template items so generated objects become usable at instance creation, plus server-side identify/unidentified behavior and a small player-facing `sip` command to reveal a consumed item's identity. Provide JSON-configurable loot themes and editor support in MUDEditor.

## Files Modified / Created
- Created: `src/pd_loot.h` — public pd_enhance_obj() API and quality constants.
- Created: `src/pd_loot.c` — procedural enhancer implementation (fills consumables, sets wand/staff charges, adds affixes to armor/weapon/jewelry).
- Modified: `src/pocket_dungeon.c` — call `pd_enhance_obj()` at generation hooks (floor, chest, hidden cache, boss drops).
- Modified/Added types: `src/typedefs.h`, `src/structs.h` — `PD_LOOT_THEME_T` declarations.
- Modified: `src/globals.h` / `src/globals.c` — `pd_loot_theme_first` / `pd_loot_theme_last` globals.
- Modified: `src/defs.h` — recycle constant registration.
- Modified/Added recycle helpers: `src/recycle.h` / `src/recycle.c` / `src/tables.c` — register `PD_LOOT_THEME_T` recycler and init/dispose.
- Modified/Added JSON reader: `src/json_tblr.h` / `src/json_tblr.c` — `json_tblr_pd_loot_theme()` to import `json/config/pd_loot_themes.json` into runtime theme list.
- Modified: `src/json_import.c` — registered the pd_loot_theme import path.
- Created: `json/config/pd_loot_themes.json` — example/default theme.

- Editor (MUDEditor) changes:
  - Modified: `mudeditor/web/server/src/routes/config.ts` — added `pocketDungeonLootThemesRouter()` CRUD for `pd_loot_themes.json`.
  - Modified: `mudeditor/web/server/src/index.ts` — registered `/api/config/pocket-dungeon-loot-themes` route.
  - Modified: `mudeditor/web/client/src/lib/api.ts` — added `pocketDungeonLootThemesApi` wrappers.
  - Modified: `mudeditor/web/client/src/pages/PocketDungeonPage.tsx` — added `LootTab` UI and integrated tab.

- Identify / do_sip integration (server-side):
  - Modified: `src/spell_info.c` — `spell_identify_perform_seeded()` now clears `ITEM_HIDDEN` for the identified object when knowledge >= 60% and prints an identification message.
  - Modified: `src/act_obj.h` — declared `do_sip`.
  - Modified: `src/interp.c` — registered the `"sip"` command.
  - Modified: `src/act_obj.c` — implemented `do_sip()`; sips a drink/potion from inventory, applies quaff effects, and only identifies the sipped item (removes `ITEM_HIDDEN`), without revealing other hidden objects.

## Implementation Steps Performed
1. Designed conservative enhancement rules (tier-based charges/levels, small affixes only, stack on top of template affects).
2. Implemented `pd_enhance_obj()` and called it from `pd_generate_instance()` at all appropriate object-creation points.
3. Added `pd_loot_theme` runtime structure and JSON import so themes are editable via `json/config/pd_loot_themes.json`.
4. Added MUDEditor server route and client UI to CRUD themes.
5. Implemented identify behavior to reveal `ITEM_HIDDEN` on successful identify/lore.
6. Implemented `do_sip` command that reveals only the consumed item (no room-wide reveal).
7. Built the project using the repository `build` task; verified successful compile and `bin/basemud.exe` generation.

## Risks & Mitigations
- Risk: Procedural changes could unexpectedly change balance (weapon/armor affixes, potion levels).
  - Mitigation: Enhancer makes conservative, small changes; configurable via `pd_loot_themes.json` and can be disabled by omitting themes or removing calls.
- Risk: JSON loader errors for malformed config files causing boot failures.
  - Mitigation: `json_tblr_*` reader uses defensive parsing; editors should validate before saving. Provide example theme file.
- Risk: `ITEM_HIDDEN` semantics may affect other systems (visibility, looting).
  - Mitigation: `ITEM_HIDDEN` is only applied to generated consumables and removed only by explicit identify/lore or `do_sip`. `pd_do_hidden_scan()` (search/detect) still reveals caches as designed.

## Testing / Verification Plan
- Manual runtime test (requires running server):
  1. Start server and load JSON config containing a `default` theme.
  2. Create or spawn a Pocket Dungeon instance (use existing dungeon commands).
  3. Inspect objects in a chest/floor: consumables should have `ITEM_HIDDEN` set (appear hidden in client UI where appropriate).
  4. Use `lore`/`identify` on a generated consumable — on success (>=60% knowledge) the item should become identified and print an identification message.
  5. Use `sip <potion>`: it should identify only the consumed potion (remove `ITEM_HIDDEN`) and apply the potion effect, but it must NOT reveal other `ITEM_HIDDEN` objects in the room.
  6. Use `search` or cast `detect hidden` to confirm `pd_do_hidden_scan()` reveals hidden caches per skill/threshold.

- CI/Local checks already performed:
  - TypeScript checks: `mudeditor/web/{shared,server,client}` — passed after minor client fix.
  - Build task: `make` via workspace `build` task — produced `bin/basemud.exe` successfully.

## Next Actions / Recommendations
- Runtime verification on your deployed environment (you indicated you'll run remotely). I can provide a short command checklist to run on the server to validate everything.
- Optional tuning: add additional fields to `pd_loot_themes.json` (rarity weights, specific vnum pools, per-item overrides).
- Optional: add logging around `pd_enhance_obj()` for early live testing (debug build) to inspect values applied.

## Status
✅ COMPLETED — 2026-04-12

---
If you want, I can now prepare and paste the exact runtime test commands to run on the deployed server, or add debug logging into `pd_enhance_obj()` to emit applied values at instance creation.
