✅ PLAN COMPLETE

# Plan: Agent OS Awareness + Snippet File Move Workflow

## Scope
Update BaseMUD instruction and agent files so they:
1) Move integrated snippet files from /Snippets/Pending to /Snippets/Completed before commit.
2) Gate command examples/availability by detected OS (Windows vs Linux) across agent instructions.

## Files To Modify
- .github/copilot-instructions.md
  - Add cross-platform command policy section with Linux and Windows command guidance.
  - Add requirement to determine OS before choosing commands.
- .github/agents/snippet-integrator.agent.md
  - Add explicit workflow step to move processed snippet file from /Snippets/Pending to /Snippets/Completed before commit/push.
  - Add OS-aware command guidance for build/commit command execution examples.
- .github/agents/mudeditor-sync.agent.md
  - Add OS-aware command guidance for verification/build command execution examples.
- .github/agents/pocket-dungeon-author.agent.md
  - Add OS-aware command guidance so command/tool usage follows detected host OS.

## Risks
- Wording could conflict with existing hard-coded command examples; mitigated by clarifying that examples must be adapted per detected OS.
- Overly broad wording could alter unrelated behavior; mitigated by keeping edits limited to workflow and command sections only.

## Notes
- No runtime code changes; documentation/instruction updates only.

## Status: COMPLETED (2026-04-14)
