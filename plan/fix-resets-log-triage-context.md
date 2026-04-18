✅ PLAN COMPLETE

# Plan: fix_resets log context + triage parser support

## Scope
Enhance BaseMUD boot logging for invalid equip resets, add MUDEditor parser support for enriched lines, and improve triage candidate lookup by room context.

## Files To Modify

1. f:\Source\BaseMUD\src\db.c
- Update `fix_resets()` warning output for unequippable equip resets to include room vnum and owning mob context.
- Determine owning mob by scanning backward in the current room reset chain for the nearest preceding `M` reset.
- Emit enriched warning format with room and mob name when owner exists.
- Emit enriched warning format with room and `no owning mob reset` when no owner exists.
- Keep existing old-style warning output path as fallback when room context is unavailable.

2. f:\Source\mudeditor\web\client\src\lib\logTriage.ts
- In `parseBootLogIssues`, add a new regex branch before legacy `unequippable_equip` parsing for:
  - room+owned-mob enriched line format
  - room+no-owning-mob enriched line format
- Populate parsed issue fields (`roomVnum`, `mobVnum`, `mobName`) from captures.
- Use dedupe key format: `unequippable_equip:${objectVnum}:${mobVnum ?? 'none'}:${wearLoc}`.
- Keep existing fallback regex branch below the new branches unchanged for legacy lines.

3. f:\Source\mudeditor\web\client\src\pages\LogTriagePage.tsx
- Update equip-candidate collection flow to prioritize room-scoped lookup when `issue.roomVnum` exists.
- Resolve room via `areasApi.resolveVnum(issue.roomVnum, 'room')` and search that exact room first.
- If no candidates found, fall back to existing broader area/full scan behavior.

## Validation
- Run TypeScript check in MUDEditor client:
  - `npx tsc --noEmit` from `web/client`.

## Risks / Notes
- Reset ordering assumptions: owning mob lookup depends on nearest prior `M` reset in room chain; malformed/orphaned data should log `no owning mob reset`.
- Parser ordering is important: enriched regexes must be placed before legacy fallback to avoid short-circuiting.
- Candidate lookup changes must preserve current behavior when `roomVnum` is absent.

## Status: COMPLETED (2026-04-17)