# ✅ PLAN COMPLETE
# Plan: do_donate mass donation by pattern

## Scope
- Keep existing single-item `donate <item>` behavior unchanged.
- Add `donate all <pattern>` support in `do_donate` only.

## Files To Modify
- `src/act_obj.c`
  - Extend argument parsing in `do_donate` to detect `all` prefix and require a non-empty pattern.
  - Iterate inventory for matches with `str_in_namelist(pattern, obj->name)` when using `all` mode.
  - Apply existing per-item restrictions unchanged (fighting check, ownership, droppable unless immortal, no corpses, no timer).
  - Donate matching items one by one using existing altar/pit behavior.
  - Preserve alignment gating logic and adjust silver reward to ~10% of item worth (`obj->cost`) with safe minimum and guards.
  - Aggregate player-facing donate output by identical item identity/name (same index + short description), e.g. `You donate (N) of ...`.

## Files To Create
- `plan/donate-all-pattern.md` (this file)

## Risks / Notes
- Grouped messaging must avoid stale pointers after moving/extracting objects; message grouping should store only stable identity keys and counts.
- `str_in_namelist` pattern behavior is token-based; multi-word pattern semantics follow existing name-list matching conventions.
- Keep anti-alignment checks exactly as current logic; only payout ratio changes from 50% to ~10%.

## Status: COMPLETED (2026-04-12)
