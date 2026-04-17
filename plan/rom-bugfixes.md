✅ PLAN COMPLETE

## Plan Overview

Five confirmed bugs fixed:

### 1. Charm Breaking via NOFOLLOW
**Problem**: Players could break free from charm by using `nofollow` command.
**Solution**: Added `AFF_CHARM` check in `do_nofollow()` to block the command for charmed players.
**File**: `src/act_conf.c`

### 2. Linkdead Pet Extraction
**Problem**: Pets were orphaned in `char_list` with `in_room==NULL` when master went linkdead.
**Solution**: Extract pet to LIMBO in `close_socket()` before player goes linkdead.
**File**: `src/descs.c`

### 3. Group Experience Bypass
**Problem**: Disabled level-range checking allowed low-level characters to gain full XP from high-level kills.
**Solution**: Re-enabled the level check and changed logic to use highest group member level instead of group leader level.
**File**: `src/fight.c`

### 4. Corpse Silver Deposit
**Problem**: NPC corpses only deposited gold; silver was lost.
**Solution**: Changed condition from `ch->gold > 0` to `ch->gold > 0 || ch->silver > 0`.
**File**: `src/fight.c`

### 5. Linkdead Immortal Timer
**Problem**: Linkdead immortals had timer reset every pulse, preventing eventual quit.
**Solution**: Changed immortal check from `IS_IMMORTAL(ch)` to `IS_IMMORTAL(ch) && (ch->desc != NULL)`.
**File**: `src/players.c`

## Implementation Details

- All fixes applied cleanly without conflicts
- Build verified with `-Werror` flag (all warnings as errors)
- Commit: `b7f2877` on `Dungeon` branch
- Pushed to origin successfully

## Status: COMPLETED
Date: Session commit successful
