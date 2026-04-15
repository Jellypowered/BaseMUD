# Restringer Integration Plan (2026-04-15)

## Scope
Integrate the `restring` command snippet that allows players to rename their objects by paying questpoints at a Restringer NPC. Support `name`, `short`, and `long` field updates on carried/worn items.

## Files to modify
- `src/ext_flags.h`
  - Define `MOB_RESTRINGER` using `MOB_UNUSED_FLAG_8` (bit 25).
- `src/ext_flags.c`
  - Register `"restringer"` in the `mob_flags` table.
- `src/chars.h`
  - Declare `char_get_restringer_room(const CHAR_T *ch)`.
- `src/chars.c`
  - Implement `char_get_restringer_room` using the `MOB_RESTRINGER` flag.
- `src/act_obj.h`
  - Declare `DECLARE_DO_FUN(do_restring)`.
- `src/act_obj.c`
  - Implement `do_restring` with questpoint pricing (100/150/200) and service NPC check.
  - Use `str_smash_tilde` for input sanitation.
  - Use `str_replace_dup` for safe string swap (requires `#include "memory.h"`).
- `src/interp.c`
  - Register `restring` command as a normal player command.
- `doc/Json_Documentation.md`
  - Update `mob_flags` section to include `restringer`.
- `json/help/restring.json`
  - Create help file for RESTRING command.
- `json/help/credits.json`
  - Add TAKA credit for the restring snippet.
- `.github/agents/cheatsheet.md`
  - Record the new `MOB_RESTRINGER` bit (25).

## Order of changes
1. Mob flag and Restringer lookup helper (ext_flags/chars).
2. Command implementation and registration (act_obj/interp).
3. Documentation and help updates.
4. Clean up snippet move.

## Risks / Notes
- `questpoints` is already in `ch->questpoints`. 
- Ensure `memory.h` is included for `str_replace_dup` in `act_obj.c`.
- Restricting to items carried/worn requires using `find_obj_here` which checks room + inventory + worn.
- Pricing follows the requested "restring <item> <field> <text>" syntax.
