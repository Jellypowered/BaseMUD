# BaseMUD Integration Cheatsheet

Verified findings from actual integrations. Update this file as new patterns are confirmed.

---

## Bleeding System and Bandage Skill

### Condition System Extensions
- New condition: `COND_BLEEDING` (index 4). Max value is `COND_HOURS_MAX (48)`.
- Conditions stored in `ch->pcdata->cond_hours[]` array; length auto-expands with `COND_MAX`.
- Condition predicates follow pattern: `DEFINE_COND_FUN(char_is_condition)` in `src/chars.c`, paired with declaration in `src/chars.h`.
- Condition table entry in `src/tables.c` cond_table: `{COND_ID, "name", good_fun, bad_fun, msg_good, msg_bad, msg_better, msg_worse}`.

### Skill Mapping for Bandage
- `SKILL_MAP_BANDAGE` = slot 54. SKILL_MAP_MAX extended to 55.
- Skill_map_table entry: `{SKILL_MAP_BANDAGE, "bandage"}` maps to JSON skill entry.
- Global default skill: initialized to 50% in `player_set_default_skills()` (players.c) and `save.c` for new characters.

### Bleeding Mechanics
- **Trigger 1 (melee):** `one_hit()` in fight.c: 10% chance on successful damage_visible() call when `dt >= ATTACK_FIGHTING`.
- **Trigger 2 (HP threshold):** Also in one_hit(): automatic trigger when `victim->hp < max_hp/4`.
- **Damage:** Applied in `char_update()` in chars.c; damage = `max_hp / 20` (5%) per tick; messages scale by level (>30 heavy, >20 medium, >10 light, else minor).
- **Decay:** 50% chance per tick (`number_percent() < 50`) to decrease by 1 via `player_change_condition()`.

### Bandage Skill Implementation
- Command handler: `DEFINE_DO_FUN(do_bandage)` in `src/act_skills.c` (mirrors spit_acid pattern).
- Skill check: `skill >= number_percent()` for success; calls `player_try_skill_improve()` for both outcomes.
- Restrictions: Fails if `ch->fighting != NULL` (cannot bandage in combat); fails if `COND_BLEEDING <= 0` (no wounds).
- Action: Success reduces `COND_BLEEDING` by 1 via `player_change_condition()`.
- Registered in `src/interp.c` command table: `{"bandage", do_bandage, POS_SITTING, ...}` — allows bandage while sitting or standing.

### Help and Documentation
- Help entry: `json/help/bandage.json` with dual keywords (BANDAGE + BLEEDING).
- Player-visible in help system; bleeding stage descriptions match char_update messages.



## Build Notes

- `make` will fail with `-Werror` if a new helper is left unused in a build target; the pocket dungeon mobprog module initially hit `pd_build_random_idle_behavior` until the dead helper was removed.
- The workspace build task is the right verification path for BaseMUD changes; it catches warnings that the lightweight editor diagnostics can miss.

## Player Command Notes

- Quit text is emitted in `do_quit` inside `src/act_player.c`; replacing the single `send_to_char(...)` call with a `number_range(...)` switch is the minimal pattern for randomized logoff lines.
- Colorized quit lines should use `{X` tokens and end with `{x` reset before `\n\r`.
- `do_remove` in `src/act_obj.c` supports `remove all` and `remove all.<keyword>`; parse with `do_obj_parse_arg(...)` and iterate `ch->content_first` with `obj_next` captured before `char_remove_obj(...)`.
- `MOB_RESTRINGER` is bit 25 (replacing `MOB_UNUSED_FLAG_8`). Used for `do_restring` service NPC check.
- `fix_resets()` in `src/db.c` scans all 'E' resets and corrects invalid wear locations to valid ones for each object using `obj_index_can_wear_flag()` and `wear_loc_get_flag()`. Logs warnings and marks areas as `AREA_CHANGED`.

## Idle Command

- Source: Ferric of MelmothMUD, enhanced by Dennis Reichel (Snippets/Pending/idle.c).
- Implementation: `DEFINE_DO_FUN(do_idle)` in `src/act_info.c` (line ~1227).
- Uses **modern BaseMUD patterns**:
  - Descriptor iteration: `for (d = descriptor_first; d != NULL; d = d->global_next)` with `CH(d)` macro.
  - Visibility check: `char_can_see_anywhere(ch, vch)` (NOT old `can_see()`).
  - Position string: `char_get_position_str(ch, vch->position, NULL, FALSE)`.
  - OLC editor name: `olc_ed_name(vch)` returns `const char*` (NOT int).
- **Include required**: `#include "olc.h"` in act_info.c for olc_ed_name().
- **Empty string handling**: Use `status[0] = '\0'` directly instead of `snprintf(status, sizeof(status), "")` to avoid format warnings.
- Command displays: Name, Idle ticks, Hours played, Hours/level ratio, Position, Status (OLC editor or "Quest"), Host.
- Includes 1% easter egg ("You have become better at idleness!").

## File Operation Notes

- When moving files with `run_in_terminal`, use absolute paths for `Move-Item` to avoid dependence on terminal cwd. In one run, a command chain that included `Set-Location` was simplified by the tool output and made verification confusing.
- For large move batches, verify completion with `list_dir` immediately after the move before retrying; retries can fail with `PathNotFound` simply because files were already moved.

## Pocket Dungeon Notes

- Hidden caches only spawn when a seed populates `hide_keywords`, `hide_look_texts`, and `hide_hint_phrases`; `hide_hint_count` is derived from the `hide_keywords` array length.
- `container_vnum`, `hidden_container_vnum`, `search_scroll_vnum`, and `search_wand_vnum` can be reassigned to new object slots as long as the seed JSON and object JSON stay in sync.

### Loot enhancement system (`src/pd_loot.c`)

- `pd_enhance_obj(obj, inst, tier)` is called after every item creation in pocket_dungeon.c; tiers: `PD_QUALITY_FLOOR=1`, `PD_QUALITY_CHEST=2`, `PD_QUALITY_BOSS=3`.
- Loot themes live in `json/config/pd_loot_themes.json`, loaded via `json_tblr_pd_loot_theme()` into a global linked list at boot. No hot-reload; requires restart.
- Theme lookup: `pd_loot_theme_get(inst->theme)` — falls back to `"default"` entry; returns NULL if neither found.
- `spell_pool` strings are resolved at load time via `skill_lookup_exact()`. Unknown spell names are silently skipped.
- `pd_loot_themes_reload_spells()` is hooked into `skill_reload_mapping`, which fires for BOTH `skills.json` AND `weapons.json` post-load. The function guards against double-invocation by skipping themes whose `spell_names[]` are all NULL (already resolved and freed on the first pass).
- `stat_pool` strings are resolved at load time via `type_lookup_exact(affect_apply_types, name)`. Valid names match `affect_apply_types[]` in `src/types.c` (e.g. `"hit roll"`, `"dam roll"`, `"armor class"`).
- Affects are **stacked on top** of template affects, never cleared.
- New affects use: `affect_new()` + `affect_init(af, AFF_TO_OBJECT, ...)` + `affect_to_obj_back()` + `affect_modify_obj()`.
- `str_replace_dup()` is in `memory.h` — include it in any file that calls it.
- Consumables return early from `pd_enhance_obj` (no naming prefix), equipment falls through to a prefix check at the bottom.

### ITEM_UNIDENTIFIED (BIT_28)

- Set on all procedurally generated consumables at creation in `pd_enhance_obj`.
- `do_sip` in `act_obj.c`: taste-tests only `ITEM_POTION`; chance = `20 + INT*2 + WIS*2 + lore/4`, capped at 95. On success: clears flag + prints thematic hint. On fail: no change.
- `do_lore` / `spell_identify_perform_seeded`: clears flag when player knowledge threshold ≥60%.
- `act_info.c` look suppression: checks `IS_SET(obj->extra_flags, ITEM_UNIDENTIFIED)` before printing `obj->description`; shows generic label by item type instead.
- MUDEditor: `"unidentified"` added to `extra_flags[]` in `useFlagsConfig.ts` (BIT_28, after `"reward"`/`"corroded"`).

## MUDEditor Notes

- The MUDEditor `tsc` tasks in the workspace currently point at `node_modules\.bin\tsc` under `web/shared` and `web/client`, which fails with “The system cannot find the path specified.” Use editor diagnostics or a corrected path from `web/` when validating those packages.
## Diagonal Exits (NE/NW/SE/SW)

DIR_NE=6, DIR_NW=7, DIR_SE=8, DIR_SW=9; DIR_MAX changed from 6 to 10.

- `door_table[]` in `tables.c` extended with 4 new sentinel-terminated entries. Reverse pairs: NE↔SW, NW↔SE.
- `doors.json` in `json/config/` extended with `dir` 6–9 entries.
- Player commands registered in `interp.c`: ne/nw/se/sw (abbreviation) + northeast/northwest/southeast/southwest (full word).
- OLC REDIT: `redit_table` in `olc.c` and handlers/declarations in `olc_redit.c`/`olc_redit.h` extended.
- `pd_dir_names[DIR_MAX]` in `pocket_dungeon.c` extended with 4 new entries (required to avoid NULL crash).
- All `for (i = 0; i < DIR_MAX; i++)` and `door_lookup()`/`door_get()` calls auto-handle new directions once DIR_MAX and door_table are updated — no manual changes needed in `act_info.c`, `chars.c`, `hunt.c`, `fight.c`, `rooms.c`, `scan`, etc.
- `number_door()` in `utils.c` was hardcoded to return 0–5 via bitmask; fixed to `number_range(0, DIR_MAX-1)`. Callers: `do_flee`, `do_mpwalk`, `do_hunt` decoy.
- `do_swalk` rewritten with a space-separated token grammar: each token is `[count]<dir>` (e.g. `3ne`, `sw`, `2n`). `direction_from_token()` does exact string match against all 10 abbreviations. Old char-peek helpers removed.
- MUDEditor already had all 10 directions in shared types, parsers, map layout, and RoomEditor before this change.

## Git Rebase Notes

- When rebasing local pocket dungeon changes onto `origin/Dungeon`, stash the work first if an incoming config-only commit lands on the same file. A `git stash push -u` before `git rebase origin/Dungeon` preserved the changes cleanly, and the only restore conflict was `json/config/pocket_dungeon_config.json`.
- The resolved pocket dungeon config keeps upstream `testing_mode: true` plus the local `show_room_vnums: false` toggle. That merge pattern is the correct outcome when the remote commit only updates the temp defaults file.
- In PowerShell, `git stash drop stash@{0}` can be parsed incorrectly; quote the stash ref as `git stash drop "stash@{0}"` when dropping a specific entry.

---

## Banking System Notes

- PC_DATA balances are added fields: `long balance` (gold) and `long sbalance` (silver). Existing player files do not have these fields initially; the loader must zero-init them on first read.
- New MOB flags: `MOB_BANKER` (12) and `MOB_ATM` (13), replacing UNUSED_FLAG_4/5. Registered in `ext_flags.c` with human-readable names.
- Banking config is loaded from `json/config/banking_config.json` at boot; no hot-reload. JSON structure: object with `banking_config` key containing 8 int fields (hours, limits, feature toggles).
- All banking operations (deposit, withdraw, transfer, convert silver) call `char_save(ch)` immediately after state change to persist player data.
- ATM daily limits are simple per-transaction checks (no session-based daily reset tracking yet); future enhancement could use a persistent counter reset at midnight.
- Business hours check: `time_info.hour >= bank_open_hour && time_info.hour < bank_close_hour`. ATMs with `atm_allow_bypass=1` skip this check.
- Help entry: `json/help/bank.json` defines BANK/BANKING/NOBANK keywords for player documentation.

---

## Object Value Fields

BaseMUD objects use a union `v` for item values instead of `value[0..4]`:

### Armor
```c
obj->v.armor.vs_pierce  // AC vs pierce
obj->v.armor.vs_bash    // AC vs bash
obj->v.armor.vs_slash   // AC vs slash
obj->v.armor.vs_magic   // AC vs magic
```

### Weapon
```c
obj->v.weapon.weapon_type  // weapon class (sword, dagger, etc.)
obj->v.weapon.attack_type  // attack verb index into attack_table[]
obj->v.weapon.dice_num     // number of dice
obj->v.weapon.dice_size    // faces per die
obj->v.weapon.flags        // weapon flags (WEAPON_TWO_HANDS, etc.)
obj->obj_index->new_format // TRUE = dice roll formula; FALSE = legacy flat values

// Average damage:
// new_format: dice_num * (dice_size + 1) / 2.0
// old_format: (dice_num + dice_size) / 2.0
```

### Attack/Weapon name lookups
```c
weapon_get_name(obj->v.weapon.weapon_type)      // "sword", "dagger", etc.  (lookup.h)
attack_table[obj->v.weapon.attack_type].name     // attack verb string (tables.h)
str_if_null(weapon_get_name(...), "unknown")     // safe null fallback  (utils.h)
```

---

## Affect Iteration on Objects

```c
// Prototype affects (skip if obj->enchanted == TRUE):
for (paf = obj->obj_index->affect_first; paf; paf = paf->on_next)
    // paf->apply, paf->modifier, paf->bits, paf->bit_type

// Instance affects (enchantments etc.):
for (paf = obj->affect_first; paf; paf = paf->on_next)
    // same fields

// Affect location name:
affect_apply_name(paf->apply)   // "strength", "armor", etc.  (lookup.h)
// APPLY_NONE == 0; APPLY_MAX == 26 (types.h)
```

---

```c
// Food
obj->v.food.hunger    // value[0]
obj->v.food.fullness  // value[1]
obj->v.food.poisoned  // value[3]

// See spell_create.c for examples
mushroom->v.food.hunger   = level / 2;
mushroom->v.food.fullness = level;
```

Do **not** use `obj->value[n]` directly â€” it may still compile but bypasses the union type.

---

## Linked List Macros

`LIST2_ADD` does **not exist** in BaseMUD. Use `LIST2_BACK` to append to the tail of a doubly-linked list:

```c
LIST2_BACK(node, prev_field, next_field, list_head, list_tail);
```

Use `LIST2_FRONT` to prepend. Use `LIST2_REMOVE` to unlink.

---

## File Handle for Reserved File

BaseMUD uses `reserve_file` (not `fpReserve` as in ROM/ROT). Declared in `globals.h`, defined in `globals.c`.

```c
fclose(reserve_file);
/* ... write ... */
reserve_file = fopen(NULL_FILE, "r");
```

---

## fread_word Signature

BaseMUD's `fread_word` takes a buffer: `fread_word(fp, buf, size)`. For in-place string replacement use:

```c
fread_word_replace(fp, &ptr->name); // frees old string, dups new
fread_word_dup(fp);                 // returns a str_dup'd copy
fread_word_static(fp);              // returns static buffer (no alloc)
```

---

## Type Renames (ROM â†’ BaseMUD)

| ROM / MERC          | BaseMUD         |
| ------------------- | --------------- |
| `CHAR_DATA *`       | `CHAR_T *`      |
| `OBJ_DATA *`        | `OBJ_T *`       |
| `ROOM_INDEX_DATA *` | `ROOM_INDEX_T *`|
| `AFFECT_DATA`       | `AFFECT_T`      |

---

## Spell / Skill Functions

### Signature

```c
// ROM
void spell_foo(int sn, int level, CHAR_DATA *ch, void *vo, int target)

// BaseMUD â€” note extra const char *target_name argument
DEFINE_SPELL_FUN(spell_foo) {
    // sn, level, ch, vo, target, target_name all in scope
}
```

### Declaration (in matching .h)

```c
DECLARE_SPELL_FUN(spell_foo);
```

### Registration â€” always add to spell_dispatch.c

```c
// spell_dispatch.c, in the correct /* spell_off.h */ block
{"spell_foo", spell_foo},
```

---

## act() / Messaging

BaseMUD does **not** use ROM's `TO_ROOM`/`TO_CHAR` pattern for combined ch+victim messages.
Use `act3()` instead:

```c
// ROM:
act("$n blasts $N!", ch, NULL, victim, TO_ROOM);
act("You blast $N!",  ch, NULL, victim, TO_CHAR);

// BaseMUD:
act3("You blast $N!",          // to ch
     "$n blasts you!",         // to victim
     "$n blasts $N!",          // to everyone else
     ch, NULL, victim, 0, POS_RESTING);
```

For ch-only or room-only messages, `act2()` covers ch+room (no victim):

```c
act2("You do the thing.",
     "$n does the thing.",
     ch, NULL, NULL, 0, POS_RESTING);
```

The `act()` macro still works for simple single-target messages:

```c
act("$n trips!", ch, NULL, NULL, TO_NOTCHAR);
```

`TO_NOTCHAR` = everyone except ch. `TO_CHAR`, `TO_VICT`, `TO_ALL` still exist in `flags.h`.

---

## Damage

```c
// ROM
damage(ch, victim, dam, sn, DAM_ACID, TRUE);

// BaseMUD
damage_visible(ch, victim, dam, sn, DAM_ACID, NULL);
// NULL = no custom damage adjective string
```

`damage_visible()` returns `bool` â€” `TRUE` if the victim died (or was already dead).
**Always check the return value in loops** to avoid use-after-free:

```c
for (i = 8; i > 0; i--) {
    damage_visible(ch, victim, dam, sn, DAM_ACID, NULL);
    if (victim->position == POS_DEAD)
        return;
}

---

## Donation Command Notes

- `do_donate` supports `donate <item>` and `donate all <pattern>`. `donate all` without a pattern is rejected with a dedicated message.
- Mass donation scans only carried inventory (`wear_loc == WEAR_LOC_NONE`) and filters by `str_in_namelist(pattern, obj->name)`; it does not donate unmatched inventory items.
- Donation reward logic keeps the same good/evil/neutral gating but uses `obj->cost / 10` (minimum 1 silver) instead of half-cost.
```

---

## Color Codes

ROM-style inline color codes work unchanged in BaseMUD:

| Code | Meaning          | Notes |
| ---- | ---------------- | ----- |
| `{R` | Bright red       | hard-coded |
| `{r` | Dark red         | hard-coded |
| `{G` | Bright green     | hard-coded |
| `{g` | Dark green       | hard-coded |
| `{C` | Bright cyan      | hard-coded |
| `{c` | Dark cyan        | hard-coded |
| `{Y` | Bright yellow    | hard-coded |
| `{y` | Dark yellow      | hard-coded |
| `{M` | Bright magenta   | hard-coded |
| `{m` | Dark magenta     | hard-coded |
| `{W` | Bright white     | hard-coded |
| `{w` | White            | hard-coded |
| `{D` | Dark grey        | hard-coded |
| `{0` | Black            | hard-coded |
| `{x` | Reset            | hard-coded |
| `{!` | Blink            | hard-coded (`\x1b[5m`) |
| `{*` | Beep             | hard-coded |
| `{/` | Newline          | hard-coded |
| `{T` | Help title       | **configurable** — maps to `help_title` colour_setting (index 35) |

**Channel-mapped codes** — these letters fall through to `colour_setting_get_by_char()` and render the player's configured channel colour. They are **not** hard-coded ANSI:

| Code | Channel setting routed to |
| ---- | ------------------------- |
| `{B` | `wiznet` (index 24, default hi-blue) |
| `{5` | `fight_skill` (index 34, default magenta) |
| `{F` | `answer_text` (index 10, default hi-white) |

Do **not** use `{B`/`{5`/`{F` as general-purpose colour codes — they render as the player's personal channel setting, not a fixed colour. Use explicit codes like `{C`/`{M`/`{W` instead.

All other letters map to their channel colour_setting_table entry via `colour_setting_get_by_char()`. See `src/tables.c` `colour_setting_table[]` for the full mapping.

---

## Skills / Spells in JSON (`json/config/skills.json`)

See `doc/Json_Documentation.md` for full schema.

### Unassigned (dormant) spell entry â€” default for new spells

```json
{
	"skill": {
		"name": "my spell",
		"classes": {},
		"target": "char_offensive",
		"min_position": "fighting",
		"slot": 524,
		"min_mana": 20,
		"usage_beats": 12,
		"damage_noun": "my spell",
		"off_msg_char": "!My Spell!",
		"spell_fun": "spell_my_spell"
	}
}
```

- **`"classes": {}`** â€” all-zero `level`/`effort` after `calloc`. `effort == 0` blocks training and practice entirely. Assign levels/efforts later via the web editor.
- **`slot`** — must be unique across all entries. Last confirmed used: 555 (wish). Next available is 556.
- `damage_noun` and `off_msg_char` are optional but recommended for offensive spells.
- File is a top-level JSON array of `{ "skill": { ... } }` wrapper objects.

### Valid `target` values (from source)

- `ignore`, `char_offensive`, `char_defensive`, `char_self`, `obj_inventory`, `obj_character`

### Valid `min_position` values

- `dead`, `mortal`, `incap`, `stunned`, `sleeping`, `resting`, `sitting`, `fighting`, `standing`

---

## Spell Source Files (spell_off.c etc.)

New spells go in the appropriate thematic file:

| File             | Contents                              |
| ---------------- | ------------------------------------- |
| `spell_off.c`    | Offensive damage spells               |
| `spell_aff.c`    | Buff/debuff spells                    |
| `spell_cure.c`   | Healing / cure spells                 |
| `spell_misc.c`   | Misc / utility spells                 |
| `spell_move.c`   | Movement / teleport spells            |
| `spell_create.c` | Create item spells                    |
| `spell_info.c`   | Information / detection spells        |
| `spell_npc.c`    | NPC-only spells (breath weapons etc.) |

Each has a matching `.h`. Add `DECLARE_SPELL_FUN` to the header, `DEFINE_SPELL_FUN` to the `.c`, and register in `spell_dispatch.c`.

---

## Numeric Prefix (`mult_argument`) Pattern

`mult_argument` is available in `interp.c`/`interp.h`. It uses `*` as separator:
- `"3*sword"` → returns 3, arg = `"sword"`
- `"sword"` → returns 1, arg = `"sword"` (no prefix)
- **Always use separate buffers** for in/out: `mult_argument(argument, arg)` where `argument` is the raw input and `arg` is the output buffer. In-place calls (`mult_argument(arg, arg)`) work in practice for short strings but are technically UB.
- Already used in `do_buy_item` (act_shop.c). Also added to `do_get`, `do_put`, `do_drop`, `do_give`, `do_sell`.
- For OBJ_SINGLE items, loop N times calling the single-item helper. OBJ_ALL/OBJ_ALL_OF paths are unchanged.

---


## MUDEditor: flags.json and Mob Flags

`flags.json` does **not** exist in `json/config/` — it is not generated by BaseMUD.
The editor's `useFlagsConfig.ts` falls back to hardcoded arrays in `FlagsField.tsx`
when the file is missing (which is always).

**Consequence:** whenever a new ext_flag is added to BaseMUD's `ext_flags.c`,
you must also add it to **both**:
- `export const MOB_FLAGS` (or the relevant array) in `FlagsField.tsx`
- the matching entry in `FALLBACK` in `useFlagsConfig.ts`
- a tooltip entry in `FLAG_TIPS` in `FlagsField.tsx` (optional but expected)

Only flags with `TRUE` as their third argument in `ext_flags.c` are user-settable
and should appear in the editor.

Example: `MOB_NOQUEST` was added in the quest system and required a manual
editor update to appear as a toggleable pill instead of a raw string.

---

- All JSON files are **top-level arrays** of single-key wrapper objects: `[{ "skill": {...} }, ...]`
- Area data lives in `json/areas/<name>/` â€” `area.json`, `rooms.json`, `mobiles.json`, `objects.json`
- Config tables live in `json/config/` â€” see `doc/Json_Documentation.md` Â§"Config Tables"
- Help pages live in `json/help/`
- Color codes are stored literally in string fields
- Multi-flag fields are **space-separated strings**: `"room_flags": "no_mob indoors"`
- Omit optional fields entirely rather than setting them to `""` or `0`

---

## Slot Numbers

Slot numbers must be globally unique across `skills.json`.

Keep this table updated:

| Range   | Notes                       |
| ------- | --------------------------- |
| 1â€“83    | Stock ROM skills/spells     |
| 200â€“204 | Extended stock entries      |
| 401â€“402 | Additional stock entries    |
| 500â€“523 | Extended/custom entries     |
| **524** | acid rain (first custom)    |
| **525** | butcher                     |
| **526** | deter                       |
| **527** | fear                        |
| **528** | hunt                        |
| **529** | silence                     |
| **530** | cure mute                   |
| **531** | critical strike             |
| **532** | quench                      |
| **533** | sate                        |
| **534** | resurrect                   |
| **535** | spit acid                   |
| 536+    | Available for new additions |

---

## Descriptor Iteration / WHO Patterns

Iterating over all connected players:

```c
DESCRIPTOR_T *d;
for (d = descriptor_first; d != NULL; d = d->global_next)
{
    CHAR_T *wch = CH(d);
    if (d->connected != CON_PLAYING)
        continue;
    // d->character (raw), CH(d) = wch (follows d->original if set)
}
```

- **Head**: `descriptor_first` (in `globals.h`)
- **Next pointer**: `d->global_next`
- **Character**: `CH(d)` macro (follows `d->original`); or `d->character` for raw char
- `max_on` in `do_count` / `do_who` is a **static local** inside `act_info.c` — not a global
- `char_get_who_string(ch, wch, buf, sizeof(buf))` formats one WHO line including flags/clan/title
- **Inventory display**: `obj_list_show_to_char(ch->content_first, ch, TRUE, TRUE)`

---

## Pocket Dungeon System (Phase 2)

### JSON Configuration

Pocket dungeon configuration lives in `json/config/pocket_dungeon_config.json` with these key fields:

| Field                 | Type    | Default | Purpose                                                                           |
| --------------------- | ------- | ------- | ----------------------------------------------------------------------------------- |
| `testing_mode`        | bool    | false   | Free dungeon entry for testing; ignores `gold_cost_per_level`                      |
| `gold_cost_per_level` | int     | 100     | Gold per character level to enter dungeon (e.g., level 10 @ 100 = 1000 gold)       |
| `autopurge`           | bool    | true    | Auto-destroy empty instances after timeout                                         |
| `empty_timeout_mins`  | int     | 120     | Minutes before empty instance is purged if `autopurge` is on                       |
| `max_instances`       | int     | 50      | Hard cap on simultaneous dungeon instances                                        |
| `vnum_base`           | int     | 20000   | First vnum in reserved range (must not overlap static areas)                       |
| `vnum_size`           | int     | 100     | Vnums allocated per instance (must exceed largest seed room count)                 |
| `max_members`         | int     | 10      | Max players per group in one dungeon instance                                      |
| `scaling_formula`     | int     | 0       | Mob scaling mode: 0 = off, 1 = by group size                                     |

**Hot-reload:** `jreload pocket_dungeon_config` applies new settings to next-generated instances only.

### Phase 2 Features in C

**C1: Affixes** — Random dungeon modifiers (1-2 per instance):
- `PD_AFFIX_STONY` (1): +20% mob HP
- `PD_AFFIX_CURSED` (2): Healing reversed; mobs flagged `MOB_CURSED` (heal ticks reversed for 10 ticks)
- `PD_AFFIX_SWIFT` (3): +25% mob hitroll
- `PD_AFFIX_ANCIENT` (4): +50% mob density
- `PD_AFFIX_LUMINOUS` (5): -20% mob AC (easier to hit), +10% item drop

**C2: Progressive Difficulty** — Difficulty bonus scales based on `rooms_cleared`:
- `rooms_cleared` increments as mobs are eliminated from rooms
- Bonus damage/HP calculated from `rooms_cleared / 5` (one step per 5 rooms cleared)

**C3: Boss Loot** — Hooked in `fight.c` after `char_die()` for NPC deaths:
- Calls `pd_trigger_boss_loot(inst, victim)` when boss dies in dungeon
- Boss loot only drops once per instance (`boss_killed` flag guard)

**C4: Boss Powers** — Pre-spawn assignment (2-3 powers):
- `PD_POWER_STRIKE` (1): Stun 50% every 8 rounds
- `PD_POWER_AURA` (2): Healing aura +5 HP/round
- `PD_POWER_SUMMON` (3): Spawn sentinel at 50% HP
- `PD_POWER_DODGE` (4): Dodge stance +30% for 3 rounds
- `PD_POWER_DRAIN` (5): Heal 20% of damage dealt

### MUDEditor Sync

TypeScript interfaces updated in `web/shared/types/index.ts`:
- `PocketDungeonConfig`: Added `testing_mode`, `gold_cost_per_level`
- `PocketDungeonInstance`: Added `affixes[]`, `affix_count`, `rooms_cleared`, `boss_killed`, `boss_powers[]`, `boss_power_count`
- New enums: `PocketDungeonAffixType`, `PocketDungeonPowerType`

MOB_FLAGS updated: `MOB_CURSED` flag added for afflicted mobs (pocket dungeon cursed affix marker).

UI updates in `web/client/src/pages/PocketDungeonPage.tsx`:
- `ConfigTab`: Added checkbox for `testing_mode`, number input for `gold_cost_per_level`
- CheatSheet descriptions updated

---



| Table macro in `tables.c`       | Authority                          | Notes                                         |
| ------------------------------- | ---------------------------------- | --------------------------------------------- |
| `TFLAGS` / `TXFLAGS` / `TTYPES` | **C** (`flags.h`, `flags.c`, etc.) | No reader; `json/meta/` is a generated export |
| `TTABLE` / `TTABLE_DYNAMIC`     | **JSON** (`json/config/`)          | Has a `jread` function; edit JSON not C       |

- `room_flags[]` is C-authoritative — to add a flag, edit `flags.h` and `flags.c`
- `BIT_08` / `ROOM_UNUSED_FLAG_5` was `/* old: no_magic */` — repurposed as `ROOM_NOMAGIC`

### Adding a `TTABLE_DYNAMIC` config table

Required files for each new dynamic table (e.g. `foo`):

1. `defs.h` — increment `TABLE_MAX`; current value after quest_token: **80**
2. `typedefs.h` — `typedef struct foo_type FOO_T;`
3. `structs.h` — `struct foo_type { ... };`
4. `tables.h` — `extern FOO_T *foo_table; extern int foo_count, foo_cap; DECLARE_DISPOSE_FUN(foo_dispose);`
5. `tables.c` — `TTABLE_DYNAMIC(...)` entry + global variable definitions + `DEFINE_DISPOSE_FUN(foo_dispose)`
6. `json_tblr.h/.c` — `DECLARE/DEFINE_JSON_READ_FUN(json_tblr_foo)`
7. `json_tblw.h/.c` — `DECLARE/DEFINE_JSON_WRITE_FUN(json_tblw_foo)`

**Critical stem rule**: The second argument to `JSON_TBLR_START_DYNAMIC(TYPE, stem)` must match the *prefix* of the three globals. If your globals are `quest_token_count / quest_token_cap / quest_token_table`, the stem must be `quest_token` — the macro expands `stem_count`, `stem_cap`, `stem_table`. A mismatch causes a linker error pointing at the wrong symbol.

**No-heap dispose**: If the struct contains no heap-allocated strings or pointers, `DEFINE_DISPOSE_FUN` is a no-op: `DEFINE_DISPOSE_FUN(foo_dispose) { (void)obj; }`. No `free_string` calls needed.

**Writer skip condition**: `JSON_TBLW_START(TYPE, alias, skip_condition)` — set the skip condition to filter out sentinel/default entries (e.g. `qt->vnum <= 0`).

---

## Area `hidden` Field

Areas support a `"hidden": true` boolean in their `area.json`. When set, the area is excluded from `do_areas` / `do_alist` output and is invisible to players. Useful for internal/system areas (e.g. `json/areas/quest/`). Not present in stock ROM/MERC — BaseMUD addition.

---

## XP Bonus System (Fallen Angels Integration)

Mobs with special attributes grant bonus XP when defeated, making stronger or more dangerous mobs reward additional experience. Integration in `fight_compute_kill_exp()` (src/fight.c).

### Mob Attribute Bonuses

Bonuses are **cumulative** — a mob with multiple modifiers stacks them (e.g., sanctuary + haste + spec = 1.3 × 1.2 × 1.25 = 1.95×).

**Affect-based bonuses** (applied via `affect_is_char_affected()`):
- `sanctuary` affect: **+30%** (base_exp × 130 / 100)
- `haste` affect: **+20%** (base_exp × 120 / 100)

**Offensive flag bonuses** (checked via `IS_SET(victim->off_flags, ...)`):
- `OFF_AREA_ATTACK`: **+20%**
- `OFF_BACKSTAB`: **+20%**
- `OFF_FAST`: **+20%**
- `OFF_DODGE`: **+10%**
- `OFF_PARRY`: **+10%**

**Special function bonuses** (function pointer comparison):
- Breath functions (spec_breath_any/acid/fire/frost/gas/lightning): **+25%**
- Cast functions (spec_cast_cleric/mage/undead): **+20%**
- Poison function (spec_poison): **+10%**

### Implementation Details
- Bonuses applied after base_exp calculation, before alignment section
- Only applied to NPCs (`IS_NPC(victim)` check)
- Playtime scaling re-enabled: XP reduced for high-playtime characters via `time_per_level` (quarter-hours per level)
- No changes to alignment XP multiplier system

### Location
- Function: `fight_compute_kill_exp()` in `src/fight.c` (lines ~1676-1726, mob bonuses section)

---

## Furniture Object Values (item_type: furniture)

BaseMUD furniture uses named keys in the `"values"` object — **not** ROM's `rest_bonus`/`sit_bonus`/`sleep_bonus`:

```json
"values": { "heal_rate": 200, "mana_rate": 200 }
```

| Key         | Meaning                              |
| ----------- | ------------------------------------ |
| `heal_rate` | HP regeneration rate (100 = normal)  |
| `mana_rate` | Mana regeneration rate (100 = normal)|

Omit both for a decorative piece with no bonus.

---

## Building BaseMUD (MSYS2 on Windows)

`make` is not in the default PowerShell PATH. Use the MSYS2 task:

```
C:\msys64\usr\bin\env.exe PATH=/mingw64/bin:/usr/bin make
```

In VS Code, use `create_and_run_task` with that command and `group: "build"`. The Makefile auto-discovers all `src/*.c` files via `wildcard`, so new source files are picked up without editing Makefile.

---

## Pocket Dungeon System (Phase implementation notes)

- **Vnum layout**: template area 19900–19999; instances 20000–24999 (50 slots × 100 vnums)
- **`door_table[dir].reverse`** — use this instead of `REV_DIR(dir)` macro; `door_get()` is undeclared in most TUs
- **Portal gate flags**: permanent portals use `SET_BIT(obj->v.portal.gate_flags, GATE_PERMANENT)` — *not* `extra_flags`
- **GATE_PERMANENT** guard in `objs.c obj_update()`: skip timer decrement when portal has this flag
- **`area_dispose`** cascades: frees rooms via `room_index_free` which calls `room_index_dispose` → calls `room_index_from_hash` and `room_to_area(room, NULL)` automatically. Do not manually remove rooms from hash before calling `area_free()`.
- **`room_index_dispose`** calls `char_free` on any characters still in the room — always evacuate PCs (and extract mobs) before `area_free()`
- **`pd_update_all()`** is called from `update_handler()` in `update.c` alongside `quest_update()`
- **`AREA_INSTANCE (BIT_05)`** and **`AREA_HIDDEN`** flags: `area_init` sets `AREA_ADDED` — overwrite after `area_new()`
- **`room_create_exit(room, dir)`** creates an exit and attaches it to the room; then `exit_to_room_index_to(ex, dest)` links the destination

---



- File: `src/special.c` / `src/special.h`
- Declaration: `DECLARE_SPEC_FUN(spec_foo);` in `special.h`
- Registration: add `{"spec_foo", spec_foo}` to `spec_table[]` in `tables.c`
- `SPEC_MAX` (in `defs.h`) must be incremented when a new entry is added (currently 24 after `spec_assassin`)
- Room iteration pattern:
  ```c
  for (victim = ch->in_room->people_first; victim != NULL; victim = v_next) {
      v_next = victim->room_next;
      // ... filter / continue / break
  }
  ```
- Immortal check: `victim->level >= LEVEL_IMMORTAL` (no `IS_IMM` macro)
- Class check: `victim->class == class_lookup_exact("thief")` â€” class is by JSON name, returns int index
- Backstab skill: `SN(BACKSTAB)` (not `gsn_backstab`)
- For `do_say` in spec functions use `do_function(ch, &do_say, buf)` (consistent with other spec usage)

---

## Snippet Porting: Portability Notes (MinGW64 / Windows)

When adapting C snippets for BaseMUD on MinGW64:

- **`bcopy` / `bzero`** — not available; replace with `memmove` / `memset`.
- **`void*` ↔ `int` casts** — pointers are 64-bit on MinGW64 but `int` is 32-bit. Use `intptr_t` from `<stdint.h>` for any hash map or data structure that stores pointer values as integers.
- **`dir_name[]`** — does not exist in BaseMUD; use `door_table[i].name` from `tables.h`.
- **`get_room_index`** → `room_get_index`; **`ROOM_INDEX_DATA*`** → `ROOM_INDEX_T*`.
- **`exit->u1.to_room`** → `exit->to_room`; **`exit_info`** → `exit_flags`.
- **`get_obj_carry(ch, arg, ch)`** → `find_obj_own_inventory(ch, arg)` (`find.h`).
- **`get_obj_list(ch, name, room->contents)`** → `find_obj_room(ch, room, name)` (`find.h`); room's content list is `room->content_first`.
- **`obj_from_char(obj)`** → `obj_take_from_char(obj)`; **`obj_to_obj`** → `obj_give_to_obj`; **`obj_to_room`** → `obj_give_to_room` (all in `objs.h`).
- **`can_drop_obj(ch, obj)`** → `char_can_drop_obj(ch, obj)` (`chars.h`).
- **`AFF_DETER` aggro bypass** — add `|| IS_AFFECTED(wch, AFF_DETER)` to the long condition chain in `update.c` where aggressive mobs pick targets.
- **`TO_ROOM` does not exist** — use `TO_NOTCHAR` for act messages sent to everyone except ch; `TO_CHAR`, `TO_VICT`, `TO_ALL` also exist.
- **`can_see(ch, victim)`** → `char_can_see_anywhere(ch, victim)` (`chars.h`); room-scoped: `char_can_see_in_room`.
- **`is_name(arg, victim->name)`** → `str_in_namelist(arg, victim->name)` (`utils.h`).
- **`PERS(victim, ch)`** → `PERS_AW(victim, ch)` in BaseMUD (defined as `char_get_look_short_descr_anywhere`). Plain `PERS(ch)` exists but takes only one arg and returns the short_descr without visibility check.

---

## JSON Config Loading — Common Pitfalls

### `JSON_TBLR_START` macro requires `<stdbool.h>`

The macro uses `bool`/`true`/`false` (C99). Any `.c` file that includes a header
expanding this macro (directly or indirectly) must include `<stdbool.h>`. Symptom:

```
error: 'false' undeclared (first use in this function); did you mean 'fclose'?
```

Fix: add `#include <stdbool.h>` before any system headers in that file.

### `JSON_TBLR_START` lazy-clear pattern

Static config tables (those backed by C array initializers in `tables.c`) are cleared
the **first time** their JSON reader function is called — not upfront. This is
intentional: alphabetical scan order means one table's JSON can load before another
table it cross-references has had its own JSON run.

- **Do not add upfront `memset` loops** to `json_import_all()` — they break
  cross-table lookups by wiping C static data before it's needed.
- The lazy-clear `static bool` lives inside each `json_tblr_*` function body (expanded
  by the `JSON_TBLR_START` macro).
- Hot-reload via `json_reload_table()` is unaffected: it `memset`s externally before
  calling the reader, so the already-set bool is bypassed with the table already zero.

### JSON load ordering and cross-table lookups

`json_import_all()` scans `json/config/` alphabetically. Files earlier in ASCII order
load before files that depend on them. Known ordering dependencies:

| Dependent file     | Requires    | Why `p` < `s` is safe (lazy-clear) |
| ------------------ | ----------- | ----------------------------------- |
| `pc_races.json`    | `sizes.json`| `size_table` retains C data until `sizes.json` triggers its own clear |

If you add a new config table that another config file cross-references, verify the
alphabetical order and ensure the referenced table has C static data that can survive
until its own JSON file clears it.

### Post-load hooks (`spec_reload_mapping`, `skill_reload_mapping`)

Some tables require a mapping pass after all JSON is loaded (e.g., wiring name-strings
to C function pointers). This is handled by the **post-load hooks loop** in
`json_import_all()` — it iterates `master_table` and calls each entry's
`post_load_fun()` and `invalidate_max_fun()` after the directory scan completes.

If specs or skills show "Unknown special function" at boot (not hot-reload), the
post-load hooks loop is likely missing or the function is not registered in
`master_table`.

### Mob JSON field notes

- **Dice fields** (`hit_dice`, `mana_dice`, `damage_dice`) must be **strings**: `"1d8+0"`.
  Object format `{"number":1,"size":8}` is **not** handled by `json_value_as_dice()`.
- **AC key for magic/exotic**: `ac_types` in `types.c` uses `"magic"` as the key for
  `AC_EXOTIC` — **not** `"exotic"`. Using `"exotic"` silently produces zero AC.
- **`"sex"` valid values**: `"male"`, `"female"`, `"neutral"`. `"neuter"` is not valid.
- **`"race"` must be a valid race name** from `races.json`; `"undead"` is not a valid
  race (use `"mob_flags": "undead"` for undead status, with `"race"` set to e.g.
  `"human"`).
- **`"damage_noun"`** (not `"noun_damage"`) is the correct key in `skills.json` and mob
  JSON. The reversed form is silently ignored by the reader.

---

## MUDEditor Sync Status — April 12, 2026

### Banking System (Recent Addition)
- **New struct**: `banking_config_type` with 8 int fields (open/close hours, ATM bypass, daily limits, features)
- **Loaded from**: `json/config/banking_config.json` (no hot-reload)
- **New MOB flags**: `MOB_BANKER` (BIT_12) and `MOB_ATM` (BIT_13) in `ext_flags.c`
- **MUDEditor sync**: ✅ **DONE** — `BankingConfig` interface added to TypeScript; `banker` and `atm` flags added to `MOB_FLAGS[]` in `FlagsField.tsx`
- **Related**: Player data gained `long balance` and `long sbalance` fields (zero-init on load in player files pre-dating this change)

### Pocket Dungeon Loot Themes (Configuration System)
- **Config table**: `json/config/pd_loot_themes.json` — loaded into global linked list at boot; no hot-reload
- **Struct**: `pd_loot_theme` with 10 fields including `spell_pool[]` and `stat_pool[]` (resolved at load time)
- **JSON format**: spell names and stat apply-type names as strings (e.g., `"cure light"`, `"strength"`)
- **MUDEditor sync**: ✅ **DONE** — `PocketDungeonLootTheme` interface added to TypeScript shared types
- **Note**: spell_names and stat_pool strings are resolved once at boot; double-resolution guard in `pd_loot_themes_reload_spells()`

### Item Unidentified Flag (ITEM_UNIDENTIFIED / BIT_28)
- **Registered in**: `src/flags.c` `extra_flags[]` at position 27 (after `"reward"`/`"corroded"`)
- **Used on**: all procedurally generated consumables in pocket dungeon
- **MUDEditor sync**: ✅ **DONE** — `"unidentified"` added to `EXTRA_FLAGS[]` at index 27 in `FlagsField.tsx`; tooltip added

### TypeScript Interfaces Added
1. `BankingConfig` — 8-field banking configuration struct
2. `PocketDungeonLootTheme` — loot theme configuration (spells, stats, wand/potion levels, boss drop vnum)

### Flag Arrays Updated (FlagsField.tsx)
1. **MOB_FLAGS**: added `'banker'` and `'atm'` at correct indices (BIT_12, BIT_13)
2. **EXTRA_FLAGS**: added `'unidentified'` at index 27 (BIT_28)

### All Verification Passed
- ✅ tsc: shared (no errors)
- ✅ tsc: server (no errors)
- ✅ tsc: client (no errors)

---

## Class Implementation Pattern

**Workflow Skill**: `.github/skills/basemud-class-implementation/`

### Data-Driven Implementation
- All classes defined in `json/config/classes.json` — no C code modification needed
- Classes fully specified at boot by JSON configuration
- Build verification: `make clean && make -j4` (binary ~4.0MB)

### Class JSON Structure
- **Required fields**: `name`, `who_name` (3-char), `primary_stat`, `weapon`, `thac0_00`, `thac0_32`, `hp_gain_min`, `hp_gain_max`, `gains_mana`, `base_group`, `default_group`, `guild`, `titles`
- **Titles**: 68 pairs (male/female), one per level or major milestone (1, 5, 10, 15, etc. to 32+)
- **Primary stat training**: 3 points per attribute at creation based on `primary_stat` field
- **Guild vnums**: 2 required — player guild (zone 30s vnum) and immortal area guild (zone 96 vnum)

### Skill Infrastructure
- Extract all available skills: `grep -o '"name":\s*"[^"]*"' json/config/skills.json | cut -d'"' -f4 | sort | uniq`
- BaseMUD has 154+ skills with per-class level/effort requirements
- Skills.json cannot be edited per-spec — use existing skills only; create skill groups instead

### Skill Groups Pattern
- **Basics group** (free at creation): 2-4 foundational skills (weapon + utility)
- **Default group** (purchasable offer): 12-20 skills for major skill categories
- Cost structure: `{"classname": {"cost": 0}}` (free) or `{"classname": {"cost": 40}}` (purchasable)
- Cost 0 = auto-granted; cost 40-60 = point purchase

### Help Documentation
- Help entries: `json/help/help.json` with keywords and single piped-format entry
- Keywords: uppercase classname(s) with plurals (e.g., `"RANGER RANGERS"`)
- Content: class philosophy, primary stat, core abilities, playstyle guidance

### Legacy Skills Compatibility Audit
- When porting from ROM/Circle/Diku: Extract all `Skill 'name' level effort` entries from source
- Cross-reference against 154+ BaseMUD skills
- Document missing abilities by category: stances, specialized attacks, forms, rare utility
- Typical coverage: 50-70% of legacy skills available; rest are ROM enhancements
- **Not a blocker** — missing abilities documented in plan; implementation proceeds with available skills

### Planning Phase
- Create `/plan/[classname]-class-implementation.md` before coding
- Include: design decisions, stat focus, mana capability, legacy skills analysis (if porting)
- List all files to modify and risks/dependencies
- Document guild vnums used

### Review Checklist Before Commit
1. All 68 titles present (no gaps in level progression)
2. Skill group names match class name exactly (case-sensitive)
3. All skills in groups exist in skills.json
4. Help entry keywords unique and meaningful
5. Build clean: `make clean && make -j4`
6. Plan marked complete with date
7. No unintended file modifications
8. Commit references plan file

### Known Patterns
- **Stat progression**: 
  - STR = warrior/paladin (melee, carry)
  - DEX = ranger/thief (accuracy, dodge, ranged)
  - INT = mage/cleric (spell power, mana)
  - WIS = cleric/druid (healing, mana, perception)
  - CON = all (HP, poison resist)
  - CHA = bard (charm, animal companion)
- **Mana**: `true` for casters/hybrids (wisdom-based mana), `false` for pure warriors
- **Guild creation**: User manually creates .are rooms if using new vnums; existing vnum reuse requires no .are modification
- **Weapon selection**: Match class archetype (spear=ranger, sword=warrior, staff=mage, etc.)

### Example: Ranger Class (Real Implementation)
- **Primary stat**: DEX (archery/tracking focus)
- **Mana**: true (detection/utility spells)
- **Weapon**: 3717 (spear)
- **Skill groups**: "ranger basics" (spear, track) + "ranger default" (17 combat/detection)
- **Guild rooms**: Created at vnums 3025 (grove.are), 9640 (newthalos.are)
- **Skills coverage**: 42+ legacy Ranger skills available; 26 ROM-specific enhancements not in modern system
- **Build result**: Binary 4.0MB, no errors; full playable class
- ✅ BaseMUD rebuild (full clean + make, no warnings or errors)