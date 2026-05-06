# ✅ PLAN COMPLETE
# Portal Relink By Name At Boot

## Goal
Fix cases where configured portal links (including up/down) exist in JSON but do not appear in runtime exits because the in-memory links were not re-applied.

## Files To Modify
- src/portals.c
  - Change boot-time link pass to re-apply links by name for all portal records, not only unassigned ones.
  - Keep logging clear for success/failure counts.
- .github/agents/cheatsheet.md
  - Add a verified note about portal relink behavior and troubleshooting command usage.

## Risks
- Re-linking all portals may override ad-hoc in-memory portal pointer mutations made before boot completes. This is acceptable because JSON should be canonical at boot.
- Needs validation by build to ensure no regressions/warnings.

## Status: COMPLETED (2026-05-06)
