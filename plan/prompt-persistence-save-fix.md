✅ PLAN COMPLETE
# Prompt Persistence Save Fix Plan

## Goal
Ensure player prompt selections (`prompt all`, `prompt bot`, and custom prompt strings) persist across save/quit/login.

## Files To Modify
- `src/save.c`
  - Replace the broken `Prom` write condition in `fwrite_char` with logic that writes non-empty prompts.
- `.github/agents/cheatsheet.md`
  - Add a verified note documenting the prompt-save gotcha and the correct persistence behavior.

## Validation
- Run the workspace BaseMUD build task (`build`) to verify no compile/regression issues.

## Risks
- Saving prompts unconditionally (when non-empty) slightly increases player file size.
- If any legacy player file parser assumes missing `Prom`, this change could expose latent parser issues (unlikely since loader already supports `Prom`/`Prompt`).

## Status: COMPLETED (2026-05-05)
