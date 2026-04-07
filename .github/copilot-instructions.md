# BaseMUD Project Guidelines

## Snippet Integration

When a user pastes or describes code from another MUD codebase (ROM, MERC, Circle, Diku, or custom), **always delegate to the `snippet-integrator` agent** before taking any action. Do not analyze, plan, or edit files yourself for snippet work. The snippet agent manages the full workflow: investigation, clarification, implementation, documentation (help entries + `doc/Json_Documentation.md`), cheatsheet update, credits, and commit.

## Build

Always build using the workspace build task (`run_task`). Never run `make` via `run_in_terminal`.

## Search

Use `rg` for text searches in the terminal. Do not use `grep`.
