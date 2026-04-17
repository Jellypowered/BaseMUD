# Info Broadcast System Implementation

## Phase Overview
Implement the info broadcast system based on Voltec's 1998 system, adapted for modern BaseMUD conventions. Players can toggle info categories, and admins broadcast news to subscribed players.

## Files to Modify

### 1. **src/structs.h** (CHAR_T struct)
- Add `flag_t info;` field after `flag_t wiznet;` (around line 550)

### 2. **src/flags.h** (flag definitions)
- Add INFO flag definitions:
  - `#define INFO_ON (BIT_01)`
  - `#define INFO_LEVELS (BIT_02)`
  - `#define INFO_CONSENT (BIT_03)`
  - `#define INFO_DEATHS (BIT_04)`
  - `#define INFO_LOGINS (BIT_05)`
  - `#define INFO_QUESTS (BIT_06)`
- Add extern declaration: `extern const FLAG_T info_flags[];`

### 3. **src/flags.c** (flag table)
- Add `info_flags[]` table with all INFO flags registered with names

### 4. **src/save.c** (character save/load)
- In `fwrite_char()`: Add `Info <flags>` write after Wizn line (around line 180)
- In `fread_char()`: Add `Info` keyword read in case 'I' section with KEY macro

### 5. **src/interp.c** (command table)
- Look for existing `{"info", do_groups, ...}` entry
- Replace with: `{"news", do_news, POS_DEAD, 0, LOG_NORMAL, 1}`
- Add aliases:
  - `{"broadcast", do_news, POS_DEAD, 0, LOG_NORMAL, 1}`
  - `{"system", do_news, POS_DEAD, 0, LOG_NORMAL, 1}`

### 6. **src/nanny.c** (new character defaults)
- In the new character creation section (around line 1036-1050), add:
  ```c
  ch->info = INFO_ON | INFO_LEVELS | INFO_CONSENT | INFO_DEATHS | INFO_LOGINS | INFO_QUESTS;
  ```
  (Set when `ch->level == 0`, after other defaults but before final login setup)

### 7. **src/info.h** (new header file)
- Add struct info_type definition
- Add extern declarations for info_table[]
- Add DECLARE_DO_FUN(do_news)
- Add news() function declaration
- Add info_lookup() function declaration

## New Files to Create

### 1. **src/info.c** (new implementation file)
- info_table[] with all info categories
- do_news() command handler supporting:
  - No args: toggle INFO_ON
  - "on" / "off": explicit on/off
  - "show": list all available info options based on trust
  - "status": display current info settings
  - Individual category toggle (levels, consent, deaths, logins, quests)
- news() broadcast function:
  - Iterates descriptor_first with d->global_next
  - Checks d->connected == CON_PLAYING
  - Checks INFO_ON flag
  - Checks specific category flag
  - Checks min_level requirement
  - Checks admin status (get_trust >= IMMORTAL_LEVEL for restricted categories)
- info_lookup() helper to find flag by name

### 2. **json/help/news.json** (help entry)
- Keywords: NEWS BROADCAST SYSTEM
- Syntax and usage documentation

## Info Categories

```
Name            Flag                Level   Admin-Only
on              INFO_ON             0       no
levels          INFO_LEVELS         4       no
consent         INFO_CONSENT        12      no
deaths          INFO_DEATHS         0       no
logins          INFO_LOGINS         0       no
quests          INFO_QUESTS         0       no
```

## Data Structure
- Field: `flag_t info;` on CHAR_T
- Save format: `Info <flags>` keyword in player file
- Load format: KEY macro reading fread_flag

## Default Behavior
- New players get all flags: `INFO_ON | INFO_LEVELS | INFO_CONSENT | INFO_DEATHS | INFO_LOGINS | INFO_QUESTS`
- Existing players default to 0 (loaded from file or set on first login)
- Players can toggle individual categories with `news <category>`
- Admins can detect player subscription via flag checks before sending

## Admin Gating
- Not a separate info category
- Implemented as admin check in news() function:
  - Admin broadcasts call news() with min_level set appropriately
  - Receptors check `get_trust(d->character) >= min_level` before receiving
  - Non-admins silently don't see admin-targeted broadcasts

## Build Validation
- Build with `make` task
- Verify no compiler errors
- Verify info.c and info.h compile without warnings

## Implementation Order
1. Add flags (flags.h, flags.c)
2. Add struct field (structs.h)
3. Create info.h header
4. Create info.c implementation
5. Modify save.c (read/write)
6. Modify interp.c (register commands)
7. Modify nanny.c (new character defaults)
8. Create help file (json/help/news.json)
9. Build and verify

## Notes
- Use modern BaseMUD descriptor iteration: `descriptor_first` with `d->global_next`
- Use `get_trust()` for level/admin checks
- Use `CH(d)` macro for descriptor->character access (if available, else d->character)
- Send messages with `send_to_char()` and `act()` functions
- All act() calls should use modern patterns (act3, act2, etc.)
- Use fwrite_flags_static() and fread_flag() for flag I/O

## Status: COMPLETED
Date: April 17, 2026

All implementation steps have been successfully completed and integrated into BaseMUD.
The build completed without errors. The info broadcast system is ready for use.
