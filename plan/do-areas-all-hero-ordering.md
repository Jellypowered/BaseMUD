# ✅ PLAN COMPLETE

# do_areas All/Numeric/Hero ordering and vnum visibility

## Scope
- Update do_areas output buckets so entries tagged as All display first, numeric ranges display next in sorted order, and Hero display last.
- Keep column spacing consistent with current output format.
- Hide area vnum for non-immortals while preserving immortal output.

## Files to modify
- src/act_info.c
  - Add helper logic to classify area range display from area credits tags (`{ All }`, `{ Hero }`, `{None}`) with numeric fallback.
  - Apply filtering and bucket collection so output order is: All, numeric(sorted), Hero.
  - Keep range column width stable using fixed-width formatted range text.
  - Make vnum column conditional on immortal status.

## Risks
- Credits parsing may miss unexpected brace formats if malformed.
- Hero visibility behavior depends on level filter intersection assumptions.

## Validation
- Run workspace build task (`build`) and confirm success.

## Status: COMPLETED (2026-05-05)
