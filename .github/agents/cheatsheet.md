# BaseMUD Integration Cheatsheet

Verified findings from actual integrations. Update this file as new patterns are confirmed.

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
```

---

## Color Codes

ROM-style inline color codes work unchanged in BaseMUD:

| Code | Meaning        |
| ---- | -------------- |
| `{R` | Bright red     |
| `{r` | Dark red       |
| `{G` | Bright green   |
| `{g` | Dark green     |
| `{B` | Bright blue    |
| `{b` | Dark blue      |
| `{x` | Reset          |
| `{5` | Bold/highlight |
| `{k` | Dark grey      |
| `{K` | Grey           |

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
- **`slot`** — must be unique across all entries. Last confirmed used: 535 (spit acid). Increment for each new spell.
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


> Full schema reference: `doc/Json_Documentation.md`

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



| Table macro in `tables.c`       | Authority                          | Notes                                         |
| ------------------------------- | ---------------------------------- | --------------------------------------------- |
| `TFLAGS` / `TXFLAGS` / `TTYPES` | **C** (`flags.h`, `flags.c`, etc.) | No reader; `json/meta/` is a generated export |
| `TTABLE` / `TTABLE_DYNAMIC`     | **JSON** (`json/config/`)          | Has a `jread` function; edit JSON not C       |

- `room_flags[]` is C-authoritative â€” to add a flag, edit `flags.h` and `flags.c`
- `BIT_08` / `ROOM_UNUSED_FLAG_5` was `/* old: no_magic */` â€” repurposed as `ROOM_NOMAGIC`

---

## Spec Functions (`special.c` / `special.h`)

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