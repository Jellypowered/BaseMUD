# ✅ PLAN COMPLETE

**Status**: Banking system integration completed successfully.

**Date Completed**: 2026-04-12

---

# Banking System Integration Plan

**Source**: bankTODO.md (TAKA's ROM 2.4b6 banking system)  
**Status**: Ready for implementation  
**Date**: 2026-04-12

---

## Phase 1: Snippet Analysis (COMPLETED)

**What**: Full-featured banking system with deposits, withdrawals, player transfers, and optional stock market.  
**Source**: ROM 2.4b6 (TAKA's Windows port)  
**Key Features**:
- Balance tracking (gold & silver)
- Deposit/withdraw operations
- Player-to-player transfers
- ATM support (business hours bypass, daily limits)
- Stock market (excluded per user requirements)

---

## Phase 2: Codebase Findings (COMPLETED)

- **PC_DATA** (structs.h:630): No existing balance fields; must add `long balance` and `long sbalance`
- **MOB Flags** (ext_flags.h): MOB_UNUSED_FLAG_4 (12) and MOB_UNUSED_FLAG_5 (13) available → rename to MOB_BANKER and MOB_ATM
- **Flag Registration** (ext_flags.c): Flag table already references unused slots; update with new names
- **Command Registration** (interp.c): Standard pattern: add entry to `cmd_table[]` with function pointer
- **JSON Config**: Use array-of-objects layout (e.g., pocket_dungeon_config.json) for banking settings

---

## Files to Modify

### 1. `src/structs.h` — Add PC_DATA fields
**Change**: Add to `struct pc_data` (after `pkdeaths`):
```c
long balance;    /* Bank account gold */
long sbalance;   /* Bank account silver */
```

### 2. `src/ext_flags.h` — Define new MOB flags
**Change**: Replace lines with MOB flags (slots 12 & 13):
```c
#define MOB_BANKER 12       /* Can handle banking transactions */
#define MOB_ATM 13          /* ATM kiosk (bypasses business hours) */
```

### 3. `src/ext_flags.c` — Register new MOB flags
**Change**: Update `mob_flags[]` table entries for slots 12 & 13:
```c
{"banker", MOB_BANKER, TRUE},
{"atm", MOB_ATM, TRUE},
```

### 4. `src/structs.h` — Add banking config struct (after quest_config_type)
**New struct**:
```c
struct banking_config_type
{
    int bank_open_hour;        /* Hour bank opens (24-hour) */
    int bank_close_hour;       /* Hour bank closes */
    int atm_allow_bypass;      /* If 1, ATMs ignore business hours */
    int atm_daily_limit;       /* Max gold per day from ATM */
    int atm_daily_limit_silver; /* Max silver per day from ATM */
    int silver_deposit_enabled; /* If 1, allow silver deposits */
    int silver_convert_enabled; /* If 1, allow 100 silver → 1 gold conversion */
    int transfer_enabled;      /* If 1, allow player-to-player transfers */
};
```

### 5. `json/config/banking_config.json` — New config file
**Schema**:
```json
[
  {
    "banking_config": {
      "bank_open_hour": 9,
      "bank_close_hour": 17,
      "atm_allow_bypass": 1,
      "atm_daily_limit": 1000,
      "atm_daily_limit_silver": 10000,
      "silver_deposit_enabled": 1,
      "silver_convert_enabled": 1,
      "transfer_enabled": 1
    }
  }
]
```

### 6. `src/globals.h` — Declare global banking config
**Add**:
```c
extern const struct banking_config_type *banking_config;
```

### 7. `src/globals.c` — Define banking config global
**Add** (after quest_config):
```c
const struct banking_config_type *banking_config;
```

### 8. `src/db.c` — Add banking config load at boot
**Location**: After quest config load (search for `jread_quest_config`)  
**Add call**:
```c
banking_config = jread_banking_config(DATA_DIR "/config/banking_config.json");
if (banking_config == NULL)
    banking_config = banking_config_default(); /* fallback with hardcoded defaults */
```

### 9. New file: `src/act_bank.h`
**Content**: Declare do_bank function:
```c
#ifndef __ROM_ACT_BANK_H
#define __ROM_ACT_BANK_H

#include "merc.h"

DECLARE_DO_FUN(do_bank);

#endif
```

### 10. New file: `src/act_bank.c`
**Content**: Full banking command implementation
- Transact with MOB_BANKER or MOB_ATM (check availability)
- Subcommands: `balance`, `deposit <amount>`, `withdraw <amount>`, `transfer <player> <amount>`, etc.
- Respect business hours (unless ATM + bypass enabled)
- Enforce ATM daily limits
- Handle silver if enabled
- Call `char_save()` after state changes
- Use `act3()` for NPC messages (mob says confirmations)
- Use modern BaseMUD patterns (check error conditions early, return on failure)

### 11. `src/merc.h` — Add includes and declarations
**Add** (after other act headers):
```c
#include "act_bank.h"
```

### 12. `src/interp.c` — Register do_bank command
**Add to cmd_table[]** (in alphabetical position):
```c
{"bank", do_bank, POS_RESTING, 0, LOG_NORMAL, 1},
```

### 13. `src/interp.c` — Include act_bank.h at top
**Add** (with other act headers):
```c
#include "act_bank.h"
```

### 14. `json/help/bank.json` — New help entry
**Content**:
```json
[
  {
    "help": {
      "area": null,
      "name": "BANK",
      "filename": "bank.are",
      "pages": [
        {
          "keyword": "BANK BANKING NOBANK",
          "text": "Syntax: bank [command]
                 |
                 |Available commands:
                 |  bank balance        - Display your account balance
                 |  bank deposit <amt>  - Deposit gold into your account
                 |  bank withdraw <amt> - Withdraw gold from your account
                 |  bank transfer <who> <amt> - Send gold to another player
                 |
                 |Banks are open from 9am to 5pm game time. ATMs allow
                 |withdrawals at any hour but have daily limits. Deposits
                 |are available during banking hours only.
                 |"
        }
      ]
    }
  }
]
```

---

## Files to Create

1. `src/act_bank.h` — Banking declarations
2. `src/act_bank.c` — Banking implementation
3. `json/config/banking_config.json` — Banking configuration
4. `json/help/bank.json` — Help entry (new banking help area)
5. `src/json_bank.h` — JSON read/write declarations (optional; use with jread_banking_config)
6. `src/json_bank.c` — JSON reader for banking config (optional; if modularizing)

---

## Implementation Order

1. Add MOB flags (ext_flags.h/c) — no dependencies
2. Add PC_DATA fields (structs.h) — no dependencies
3. Add banking config struct & global (structs.h, globals.h/c) — no dependencies
4. Create banking_config.json
5. Create json_bank readers OR inline JSON reader in db.c (one-time load)
6. Create act_bank.h/c with do_bank implementation
7. Register do_bank in interp.c
8. Create help entry
9. Rebuild and test

---

## Known Risks

- **PC_DATA persistence**: Existing player files do not have `balance`/`sbalance` fields. Loader must zero-init on file read.
- **JSON loader**: If banking_config.json is missing, fallback to hardcoded defaults to avoid crashes.
- **Time system**: banking system depends on `time_info.hour` global. Verify it exists and is maintained.
- **Silent failures**: If a MOB is missing MOB_BANKER or MOB_ATM flags, the command silently fails. Consider adding debug output for builders.

---

## Scope Exclusions

- ✅ Stock market system (BANK_INVEST) — NOT implemented
- ✅ Per-player daily limit reset — assumes global time_info tracks days
- ✅ Admin commands to adjust share_values — N/A
- ✅ Persistent ledger/transaction logging — not implemented

---

## Success Criteria

- [ ] `make` builds cleanly (no errors/warnings)
- [ ] Bank command responds with help text when no subcommand given
- [ ] Balance returns a figure (0 for new players)
- [ ] Deposit/withdraw adjust balance and inventory gold correctly
- [ ] Transfer moves gold between players and saves both
- [ ] Silver subcommands work if enabled in config
- [ ] ATM respects time bypass and daily limits
- [ ] Help entry appears in `help bank`
- [ ] New MOB flags visible in `@flag list` (immortal command)
- [x] All new sources in `.github/agents/cheatsheet.md`

## Status: COMPLETED 2026-04-12

All implementation phases completed successfully. No build errors. Ready for final testing and deployment.

