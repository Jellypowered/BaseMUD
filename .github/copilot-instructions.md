# BaseMUD Project Guidelines

## Agent Workflow

All Copilot agents in this workspace should follow a phase-based workflow for planning, editing, debugging, and refactoring.

## Phase 1 — Investigation

Gather relevant context from files, history, and workspace instructions. Use built-in tools first (`grep_search`, `file_search`, `semantic_search`, `read_file`) and verify assumptions against the actual source.

## Phase 2 — Planning

Summarize the proposed fix or change, then confirm the exact code edits before applying them. The plan should list files to modify, new files to create, and any risks or dependencies.

## Phase 3 — Clarifying Questions

Ask the user any questions needed to define scope and intent in one batch before implementation. Focus on integration intent, conflicts with existing features, required JSON support, and any boundaries for files or subsystems.

## Phase 4 — Implementation

Implement the approved plan using the appropriate workspace tools. Prefer `replace_string_in_file`/`multi_replace_string_in_file` for precise edits, and use `create_file` only for new files.

## Phase 5 — Debugging

Verify the fix with build/tests/search as appropriate. Use the workspace build command or available compiler tools, and resolve any errors before moving on.

## Phase 6 — Refactoring

Clean up related code or comments after the fix is implemented. Keep the final code readable and consistent with BaseMUD conventions.

## Phase 7 — Documentation

If the change adds or alters player-visible behavior, update help entries and any relevant JSON documentation. For new commands, spells, skills, systems, or features, add or update the appropriate `json/help/` entries and ensure any JSON-visible data changes are reflected in documentation.

## Phase 8 — Credits / Cheatsheet Update

Update `json/help/credits.json` if the work includes a notable contribution. Also update `.github/agents/cheatsheet.md` with any verified findings that will help future integrations.

## Phase 9 — Review

Inspect the diff and confirm the final output matches the plan. Ensure the change is scoped, correct, and follows the workspace guidance.

## Phase 10 — Build, Verify, and Commit

Always build using the workspace build task (`run_task`). Do not commit or push a broken build.

When a user provides external or legacy code (including ROM, MERC, Circle, Diku, or other custom MUD code), delegate to the `snippet-integrator` agent before taking action. That agent handles investigation, clarification, implementation, documentation, cheatsheet update, credits, and commit for external-code integration work.

## Build

Always build using the workspace build task (`run_task`). Never run `make` via `run_in_terminal`.

## Search

**Always prefer built-in tools first:** `grep_search` (text/regex), `file_search` (glob), `semantic_search` (NL), `read_file` (known path). These have no shell escaping issues and are faster.

Use `rg` in the terminal only as a last resort when built-in tools are genuinely insufficient. `rg` via PowerShell fails often due to quoting rules and unsupported flags (e.g. `--include` is not a ripgrep flag). Do not use `grep` — it aliases to `Select-String` in PowerShell and is unreliable.

## Agent Contract — PowerShell / Windows

- **Python**: use `python`, not `python3`.
- **Unix tools** (`sed`, `awk`, `find`, `cat`): do NOT assume they are available in PowerShell. Only use them if a task explicitly injects MSYS2 or WSL into PATH.
- **PowerShell version**: assume 5.1. Cmdlets like `Join-String` require PS 6+; avoid them.
- **One-shot execution**: if unsure a command exists in the current environment, ask first. Do not probe by trying multiple variants.
- **Workflow**: propose 1–3 steps, then execute only the next step after confirmation when actions are destructive or time-consuming.
- **Paths**: use `${workspaceFolder}` rather than hardcoded paths.
