# Speedwalk (swalk) Command Implementation

## Overview
Implement a speedwalk command that allows players to move through multiple rooms using shorthand syntax:
- `swalk 2s3eu` expands to: go south twice, then east three times, then up once
- Validates input and queues movement commands for sequential execution

## Architecture Notes
BaseMUD differs from the original Envy codebase:
- No `flusher` buffer in descriptor (uses traditional input buffering)
- Has `repeat` field in descriptor for command repetition
- Movement commands are isolated functions in `src/act_move.c`
- Speedwalk will directly call movement functions or inject commands into input stream

## Files to Modify

### 1. src/act_move.h
- **Change**: Add declaration for `do_swalk` function
- **Details**: Add `DECLARE_DO_FUN(do_swalk);` with other movement command declarations

### 2. src/act_move.c
- **Change**: Implement `do_swalk` function
- **Details**:
  - Input validation using `check_speedwalk()` helper function
  - Digit/direction parsing to build movement sequence
  - Execute movement commands sequentially or queue them
  - Error handling for invalid syntax

### 3. src/interp.c
- **Change**: Register `swalk` command in cmd_table
- **Location**: Add entry to cmd_table with:
  - Command name: `"swalk"`
  - Function: `do_swalk`
  - Position: `POS_STANDING` (must be standing to speedwalk)
  - Level: `0` (player command)
  - Log type: `LOG_NORMAL`
  - Show flag: `1` (visible in command list)

## Implementation Approach

### Validation Function
- `bool check_speedwalk(const char *arg)` - similar to original but with BaseMUD string handling
- Validates that argument contains only:
  - Direction letters: n, s, e, w, u, d (case-insensitive)
  - Digits for repetition counts
  - Whitespace
  - At least one letter must be present

### Speedwalk Execution
Two options considered:
1. **Sequential Execution**: Call do_north/do_south/etc. directly
   - Pros: Simple, predictable
   - Cons: All moves execute in one update cycle
2. **Command Injection**: Queue commands into descriptor's input buffer
   - Pros: Moves process like normal player input
   - Cons: Needs careful buffer management

**Decision**: Use sequential execution with direct function calls (simpler, more predictable)

### Help Entry
- Create `json/help/swalk.json` with:
  - Command syntax
  - Example usage
  - Restrictions and notes

## Risks & Considerations

1. **No existing repeat/queue mechanism**: Must call movement functions directly or modify descriptor input buffer
2. **Combat state**: Should probably fail if character is in combat
3. **Movement restrictions**: Individual moves will validate (locked doors, etc.)
4. **Performance**: Executing many moves at once might need throttling
5. **Zone data**: All specified moves must be valid (exits must exist)

## Files to Create

1. `json/help/swalk.json` - Help entry for the command

## Testing Strategy

- Test basic speedwalk: `swalk 2s3eu` 
- Test with uppercase: `swalk 2S3EU`
- Test with spaces: `swalk 2s 3e u`
- Test invalid input: `swalk abc`, `swalk 2q3e`
- Test movement failures: Try to move into invalid exits
- Test combat: Attempt speedwalk while fighting

## Status: COMPLETED

## Implementation Summary

### Files Modified
1. **src/act_move.h** - Added `DECLARE_DO_FUN(do_swalk)` declaration
2. **src/act_move.c** - Implemented do_swalk command with helper functions:
   - `check_speedwalk()` - Validates syntax (directions + digits + whitespace only)
   - `direction_from_char()` - Maps char to direction constant
   - `do_swalk()` - Main command handler with movement loop
3. **src/interp.c** - Registered swalk in cmd_table (POS_STANDING, player-accessible)
4. **json/help/swalk.json** - Created help entry with syntax and examples

### Implementation Details
- Movement is executed sequentially, one room at a time
- Detects move success by comparing ch->in_room before/after char_move()
- Stops immediately on first failure and reports error
- Supports uppercase/lowercase directions (n, s, e, w, u, d)
- Allows repetition counts (e.g., 2s = south twice)
- Maximum 100 consecutive moves per command
- Disabled while in combat (ch->fighting != NULL)
- Each successful move is tracked and reported at end

### Testing Verification
- Build: Clean rebuild with -Werror completed successfully
- Binary: ELF 64-bit executable built and linked correctly
- Command registration complete and available for player use

## Status: COMPLETED - April 16, 2026
