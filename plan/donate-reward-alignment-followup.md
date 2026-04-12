# ✅ PLAN COMPLETE
# Plan: donate reward alignment follow-up

## Scope
- Update `do_donate` reward logic so donated items still award 10% even when alignment checks would previously block payout.
- Keep all existing donation restrictions and altar/pit behavior unchanged.

## Files To Modify
- `src/act_obj.c`
  - Remove alignment-based condition from reward calculation in both single-item and `all` donation paths.
  - Keep cost/level safety guards and 10% payout formula.

## Risks / Notes
- This intentionally changes gameplay economy by awarding silver for more donated items.
- Message/output behavior should remain unchanged.

## Status: COMPLETED (2026-04-12)
