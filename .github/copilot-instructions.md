# BaseMUD Project Guidelines

## Snippet Integration

When a user pastes or describes code from another MUD codebase (ROM, MERC, Circle, Diku, or custom), **always delegate to the `snippet-integrator` agent** before taking any action. Do not analyze, plan, or edit files yourself for snippet work. The snippet agent manages the full workflow: investigation, clarification, implementation, documentation (help entries + `doc/Json_Documentation.md`), cheatsheet update, credits, and commit.

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
