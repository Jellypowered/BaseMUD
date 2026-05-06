# ✅ PLAN COMPLETE
# Prompt Bot Hidden Command Plan

## Scope
Add a hidden `prompt bot` player command in BaseMUD that behaves like `prompt all` and appends a room identifier suffix that shows both anum and vnum to non-immortals.

## Files To Modify
1. `src/act_conf.c`
- Extend `do_prompt` with a `bot` subcommand (`prompt bot`).
- Reuse existing `prompt all` behavior by setting prompt to `DEFAULT_PROMPT` plus a suffix token.
- Keep behavior hidden (no help/documentation changes).

2. `src/comm.c`
- Add a new prompt token that clones immortal room-vnum display behavior but is player-accessible and prints `anum/vnum`.
- Do not alter existing immortal-only `%R` behavior.

## Risks
- Prompt token collisions: ensure a unique token is selected that is currently unused.
- Buffer formatting: keep snprintf/sprintf usage consistent with existing style and bounds.
- Hidden feature requirement: avoid touching any help/json/docs.

## Status: COMPLETED (2026-05-05)
