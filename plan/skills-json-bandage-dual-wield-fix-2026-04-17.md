✅ PLAN COMPLETE

# Skills JSON Bandage And Dual Wield Fix Plan

## Scope
Resolve the `skills.json` import failure caused by an unexpected `cmd_fun` property on the `bandage` skill entry, then verify that the post-load skill mapping for `bandage` and `dual wield` succeeds once the skill table loads cleanly.

## Files To Modify
- `json/config/skills.json`
  - Remove the unsupported `cmd_fun` property from the `bandage` skill entry so it matches the current `json_tblr_skill()` schema.
- `.github/agents/cheatsheet.md`
  - Add a verified note that command-based skills still use `interp.c` and must not serialize `cmd_fun` in `json/config/skills.json`.

## Files To Verify
- `src/json_tblr.c`
  - Confirm the accepted `skill` JSON properties and ensure the fix matches the loader schema.
- `src/skills.c`
  - Confirm the missing-skill bug originates from failed skill import rather than a mapping table typo.
- `src/tables.c`
  - Confirm `bandage` and `dual wield` remain present in `skill_map_table[]`.

## Risks
- If another stale property exists later in `skills.json`, the file could still fail to load after removing `cmd_fun` from `bandage`.
- If runtime mapping errors persist after a clean import, a second issue may exist in skill naming or load order.

## Status: COMPLETED 2026-04-17