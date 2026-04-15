# BaseMUD Project Guidelines

## Agent Workflow

All Copilot agents in this workspace should follow a phase-based workflow for planning, editing, debugging, and refactoring. Always read this file in it's entirety before starting work, and refer back to it as needed during the process. The workflow is designed to ensure thorough investigation, clear planning, precise implementation, and careful review for every change made to the codebase.

## Phase 1 — Investigation

Before running any terminal commands, read `.github/agents/cheatsheet.md` to check for known working and failing command patterns for this environment.

Gather relevant context from files, history, and workspace instructions. Use built-in tools first (`grep_search`, `file_search`, `semantic_search`, `read_file`) and verify assumptions against the actual source.

## Phase 2 — Planning

Summarize the proposed fix or change, then confirm the exact code edits before applying them. The plan should list files to modify, new files to create, and any risks or dependencies.

Before beginning implementation, write the plan to a file under `/plan/` in the repo (e.g. `/plan/my-feature.md`). The file must list every file to be modified or created, the intended change for each, and any known risks. This serves as the authoritative spec for the implementation and is checked during review. Do not deviate from it without updating the file first.
### Completed Plans

When a plan is finished, mark it with `✅ PLAN COMPLETE` at the top of the file and `## Status: COMPLETED` at the end (with optional date). Do not delete completed plans—they serve as an audit trail. **When analyzing workspace code, ignore plan files marked as COMPLETE** (they are historical reference only; active work plans do not have this marker).
## Phase 3 — Clarifying Questions

Ask the user any questions needed to define scope and intent in one batch before implementation. Focus on integration intent, conflicts with existing features, required JSON support, and any boundaries for files or subsystems.

## Phase 4 — Implementation

Implement the approved plan using the appropriate workspace tools. Prefer `replace_string_in_file`/`multi_replace_string_in_file` for precise edits, and use `create_file` only for new files. Do not deviate from the plan without asking the user first and updating the plan file accordingly. If you encounter unexpected issues during implementation, pause and ask for guidance rather than making unplanned changes.

## Phase 5 — Debugging

Verify the fix with build/tests/search as appropriate. Use the workspace build command or available compiler tools, and resolve any errors before moving on.

## Phase 6 — Refactoring

Clean up related code or comments after the fix is implemented. Keep the final code readable and consistent with BaseMUD conventions.

## Phase 7 — Documentation

If the change adds or alters player-visible behavior, update help entries and any relevant JSON documentation. For new commands, spells, skills, systems, or features, add or update the appropriate `json/help/` entries and ensure any JSON-visible data changes are reflected in documentation.

## Phase 8 — Credits / Cheatsheet Update

Update `json/help/credits.json` if the work includes a notable contribution. Also update `.github/agents/cheatsheet.md` with any verified findings that will help future integrations. Original work does not require credit. 

## Phase 9 — Review

Inspect the diff and confirm the final output matches the plan. Open the plan file from `/plan/` and cross-check every item: verify each listed file was modified or created as described, no unplanned files were changed, and no planned items were skipped. Note any deviations explicitly before closing. Ensure the change is scoped, correct, and follows the workspace guidance.

When the review passes, mark the plan file as complete by appending `## Status: COMPLETED` (with the date) to the bottom of the file. Do not delete or move the file — completed plans serve as an audit trail.

## Phase 10 — Build, Verify, and Commit, then Push

Always build using the workspace build task (`run_task`). Do not commit or push a broken build.

When a user provides external or legacy code (including ROM, MERC, Circle, Diku, or other custom MUD code), delegate to the `snippet-integrator` agent before taking action. That agent handles investigation, clarification, implementation, documentation, cheatsheet update, credits, and commit for external-code integration work.

## Build

Always build using the workspace build task (`run_task`). Never run `make` via `run_in_terminal`.

## Search

**Always prefer built-in tools first:** `grep_search` (text/regex), `file_search` (glob), `semantic_search` (NL), `read_file` (known path). These have no shell escaping issues and are faster.

Use `rg` in the terminal only as a last resort when built-in tools are genuinely insufficient. `rg` via PowerShell fails often due to quoting rules and unsupported flags (e.g. `--include` is not a ripgrep flag). Do not use `grep` — it aliases to `Select-String` in PowerShell and is unreliable.

## OS Detection And Command Set

Before running terminal commands, determine the active OS for the current environment and only use commands that are valid for that OS.

- **Linux/macOS shells**: use POSIX-compatible commands and tooling.
- **Windows PowerShell/CMD**: follow the Windows contract below; do not assume POSIX utilities exist.
- **Do not mix command families** in one workflow step (for example, `sed`/`awk` patterns in PowerShell, or PowerShell-only cmdlets in POSIX shells).
- **When command examples are documented**, adapt them to the detected OS before execution.

## Agent Contract — PowerShell / Windows

- **Python**: use `python`, not `python3`.
- **Unix tools** (`sed`, `awk`, `find`, `cat`): do NOT assume they are available in PowerShell. Only use them if a task explicitly injects MSYS2 or WSL into PATH.
- **PowerShell version**: assume 5.1. Cmdlets like `Join-String` require PS 6+; avoid them.
- **One-shot execution**: if unsure a command exists in the current environment, ask first. Do not probe by trying multiple variants.
- **Workflow**: propose 1–3 steps, then execute only the next step after confirmation when actions are destructive or time-consuming.
- **Paths**: use `${workspaceFolder}` rather than hardcoded paths.
