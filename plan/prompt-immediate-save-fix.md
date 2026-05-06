✅ PLAN COMPLETE
# Prompt Immediate Save Fix Plan

## Goal
Ensure `prompt all`, `prompt bot`, and custom `prompt <text>` changes persist immediately by saving the character at the time of prompt update.

## Files To Modify
- `src/act_conf.c`
  - Add `save.h` include if needed.
  - In `do_prompt`, after assigning `ch->prompt` for `bot`, `all`, or custom text, call `save_char_obj(ch)` for non-NPC characters.
- `.github/agents/cheatsheet.md`
  - Add a verified note that prompt updates are now persisted immediately via `do_prompt` save.

## Validation
- Run BaseMUD workspace `build` task.

## Risks
- Slightly more frequent disk writes when players spam prompt changes.
- No functional risk expected because `save_char_obj` is already used by many player commands.

## Status: COMPLETED (2026-05-06)
