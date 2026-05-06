# ✅ PLAN COMPLETE
# Hotreload Portal Refresh Safety

## Goal
Ensure area hot-reload also refreshes portal links so runtime portal connectivity stays in sync with JSON without requiring a full reboot.

## Files To Modify
- src/json_hotreload.c
  - After successful area reload/link/reset, run portal refresh passes used at boot.
  - Add a concise comment explaining why this is done during hot-reload.
- .github/agents/cheatsheet.md
  - Record that hot-reload now triggers portal refresh behavior.

## Risks
- Portal refresh scans all rooms, so hot-reload work is slightly heavier.
- Any in-memory ad-hoc portal changes are normalized back to JSON-defined linkage during hot-reload (intended safety behavior).

## Status: COMPLETED (2026-05-06)
