# BaseMUD Source Limitations & Discovered Constraints

This file documents hard-coded limits and missing features discovered while building the MUD Editor
and auditing the BaseMUD codebase. Each entry explains the constraint, its source location, and
what would be required to lift it.

---

## Guild Rooms per Class

**Limit:** Maximum **2** guild rooms per class.

**Source:** `src/defs.h:215` — `#define MAX_GUILD 2`

**Details:**
- The class struct uses a heap-allocated `sh_int *guild` array alongside `int guild_count`.
- The JSON parser (`src/json_tblr.c`) hard-stops at `guild_count < MAX_GUILD`, silently
  discarding any additional entries.
- Runtime access (`src/players.c`) iterates up to `guild_count`, so the limit is enforced
  at load time, not at use time.

**To increase:** Raise `MAX_GUILD` in `src/defs.h` and recompile. No schema changes needed
(the JSON array is already dynamic). The MUD Editor enforces this cap and disables the
"Add Guild" button once 2 entries are present.

---

<!-- Add further discoveries below as they are found during Phase H audit -->
