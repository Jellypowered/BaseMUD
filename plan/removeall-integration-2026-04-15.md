# ✅ PLAN COMPLETE - Remove All Integration Plan (2026-04-15)

## Scope
Integrate the `remove all` snippet behavior into BaseMUD `do_remove`, extended to match existing object command patterns by supporting both `remove all` and `remove all.<keyword>`.

## Files to modify
- `src/act_obj.c`
  - Update `do_remove` to parse remove targets as single/all/all-of.
  - Implement iterative worn-item removal for `all` and `all.<keyword>` using BaseMUD object iteration and visibility/name checks.
  - Preserve existing single-object behavior for `remove <object>`.
- `json/help/help.json`
  - Update the `HOLD REMOVE WEAR WIELD` help page syntax and text to document `remove all` and `remove all.<keyword>`.
- `json/help/credits.json`
  - Append credits entry for this snippet integration (contributor: TAKA).
- `.github/agents/cheatsheet.md`
  - Add verified note about `do_remove` supporting all/all.<keyword> behavior.

## Order of changes
1. Implement command logic in `src/act_obj.c`.
2. Update player help text in `json/help/help.json`.
3. Update contributor attribution in `json/help/credits.json`.
4. Record integration finding in `.github/agents/cheatsheet.md`.

## Risks / Notes
- Unequipping while iterating equipment can invalidate traversal if done incorrectly; iterate over `ch->content_first` with `obj_next` captured before removal.
- Existing remove messaging should remain stable for single-object usage.
- Help and credits JSON formatting must stay valid to avoid startup parse issues.

## Status: COMPLETED (2026-04-15)
