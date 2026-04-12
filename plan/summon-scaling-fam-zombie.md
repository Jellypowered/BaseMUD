# Implementation Plan: Summon Scaling (Familiar + Zombie)

## Status: COMPLETED

## Goal
Make summoned familiars and animate-dead zombies scale to be similar in power to the summoner at summon time.

## Files to Modify
- `src/spell_summon.c`
  - Add helper to scale summoned mob combat stats from caster.
  - Apply scaling in `spell_find_familiar` after creation.
  - Apply scaling in `spell_animate_dead` after creation.

## Verification
- Run workspace build task (`build`) and resolve any compile issues.

## Risks
- Overpowering summons if copied too literally; keep scaling "similar" by applying percentages.

Completion date: 2026-04-12
