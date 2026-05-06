# ✅ PLAN COMPLETE

# do_areas centered range formatting and display caps

## Scope
- Restore centered-looking range display style in do_areas output.
- Render symbolic labels as uppercase `ALL` and `HERO`.
- Capitalize only the first letter of displayed area names without modifying stored data.

## Files to modify
- src/act_info.c
  - Adjust range formatting helper to produce centered/fixed-width range text.
  - Emit uppercase symbolic labels in that fixed-width field.
  - Add display-name helper that title-cases only first character at render time.

## Risks
- Spacing width choices may need minor tuning based on in-game font/client rendering.

## Validation
- Run workspace build task (`build`) and confirm success.

## Status: COMPLETED (2026-05-05)
