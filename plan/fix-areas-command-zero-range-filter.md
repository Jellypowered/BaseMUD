# ✅ PLAN COMPLETE

# Fix do_areas zero-range filtering

## Scope
- Exclude areas with both low_range and high_range set to 0 from do_areas output.

## Files to modify
- src/act_info.c
  - In both matching loops inside do_areas, skip entries where low_range == 0 and high_range == 0.

## Risks
- Minimal; change only affects displayed list contents.

## Validation
- Build BaseMUD with workspace build task.

## Status: COMPLETED (2026-05-05)
