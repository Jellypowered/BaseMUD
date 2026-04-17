# Implementation Plan: Idle Command

## Overview
Implement a player list command showing idle time, playtime, and current position/status for all visible players.

## Files to Modify

### 1. src/act_info.c
- **Purpose**: Implement the `do_idle()` command handler
- **Changes**:
  - Add `DEFINE_DO_FUN(do_idle)` function
  - List all non-NPC players visible to the executor
  - Display: Name, Idle time (ticks), Playtime (hours), Hours/level ratio, Position (using `char_get_position_str()`), Status (OLC or Quest), Host
  - Use modern BaseMUD patterns:
    - `for (d = descriptor_first; ...)` iteration over descriptors
    - `IS_NPC()` check to filter NPCs
    - `can_see()` for visibility checks
    - `char_get_position_str(ch, vch->position, NULL, FALSE)` for position display
    - OLC status via `olc_ed_name()` if `vch->desc->editor` is set
    - Quest status: show "Quest" if `vch->countdown > 0`
  - Random Easter egg: 1% chance to send "You have become better at idleness!" flavor text

### 2. src/act_info.h
- **Purpose**: Declare the command
- **Changes**:
  - Add `DECLARE_DO_FUN (do_idle);` entry
  - Place it alphabetically near `do_who()` or other info commands

### 3. src/interp.c
- **Purpose**: Register the command in the command table
- **Changes**:
  - Add command table entry: `{"idle", do_idle, POS_DEAD, 0, LOG_NORMAL, 1},`
  - Place it alphabetically in the informational commands section (after `"help"` or near `"who"`)

## New Files to Create

### 4. json/help/idle.json
- **Purpose**: Player-visible help for the idle command
- **Format**: Single help entry with keywords IDLE and NOIDLE
- **Content**: Command description, usage example, explanation of displayed fields

### 5. json/help/credits.json (or update existing)
- **Purpose**: Record contributor attribution
- **Citation**: "Idle command --" credited to Ferric (MelmothMUD) and Dennis Reichel (enhancement)

## Implementation Notes

- **Position formatting**: Use `char_get_position_str(ch, vch->position, NULL, FALSE)` (modern BaseMUD approach, not position_flags table lookup)
- **Command position requirement**: `POS_DEAD` allows full visibility from any state (matches `do_who()`)
- **Status parsing logic**:
  - If `vch->desc && vch->desc->editor`: show `olc_ed_name(vch)` (e.g., "REDIT")
  - Else if `vch->countdown > 0`: show "Quest"
  - Else: show "" (blank status)
- **Playtime calculation**: `(vch->played + (int)(current_time - vch->logon)) / 3600` (hours)
- **Hours per level**: playtime / vch->level (avoid division by zero if level=0)
- **Random easter egg**: 1% chance per idle command execution (use `number_percent() == 1`)

## Known Risks
- None identified. Pattern follows established do_who() structure and uses standard BaseMUD utilities.

## Status: COMPLETED
- Implementation completed on April 17, 2026
- All files modified and created as planned
- Build verification: ✅ PASSED
- Compilation errors: Fixed (added olc.h include, changed can_see to char_can_see_anywhere, fixed empty string handling)
- Ready for use
