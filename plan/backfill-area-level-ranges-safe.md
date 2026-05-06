# ✅ PLAN COMPLETE

# Backfill area low/high ranges safely

## Scope
- Populate missing `low_range` / `high_range` in `json/areas/*/area.json`.
- Keep JSON structure/encoding valid and untouched except range fields.
- Skip credits ranges that are non-numeric (`{All}`, `{None}`).

## Files to modify
- json/areas/*/area.json
  - Only set `low_range` and `high_range` when currently missing or both zero and a numeric pair exists in `credits`.

## Risks
- Bulk update may touch many files; must validate JSON parseability after rewrite.
- Ensure UTF-8 without BOM is preserved.

## Validation
- Parse all `json/areas/*/area.json` files with Python JSON loader.
- Verify no BOM in `json/areas/*/area.json` files.
- Build BaseMUD with workspace build task.

## Status: COMPLETED (2026-05-05)
