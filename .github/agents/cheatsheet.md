# BaseMUD Integration Cheatsheet

Verified findings from actual integrations. Update this file as new patterns are confirmed.

---

## Type Renames (ROM → BaseMUD)

| ROM / MERC          | BaseMUD    |
| ------------------- | ---------- |
| `CHAR_DATA *`       | `CHAR_T *` |
| `OBJ_DATA *`        | `OBJ_T *`  |
| `ROOM_INDEX_DATA *` | `ROOM_T *` |
| `AFFECT_DATA`       | `AFFECT_T` |

---

## Spell / Skill Functions

### Signature

```c
// ROM
void spell_foo(int sn, int level, CHAR_DATA *ch, void *vo, int target)

// BaseMUD — note extra const char *target_name argument
DEFINE_SPELL_FUN(spell_foo) {
    // sn, level, ch, vo, target, target_name all in scope
}
```

### Declaration (in matching .h)

```c
DECLARE_SPELL_FUN(spell_foo);
```

### Registration — always add to spell_dispatch.c

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

`damage_visible()` returns `bool` — `TRUE` if the victim died (or was already dead).
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

### Unassigned (dormant) spell entry — default for new spells

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

- **`"classes": {}`** — all-zero `level`/`effort` after `calloc`. `effort == 0` blocks training and practice entirely. Assign levels/efforts later via the web editor.
- **`slot`** — must be unique across all entries. Last confirmed used: 524 (acid rain). Increment for each new spell.
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

## JSON System Overview

> Full schema reference: `doc/Json_Documentation.md`

- All JSON files are **top-level arrays** of single-key wrapper objects: `[{ "skill": {...} }, ...]`
- Area data lives in `json/areas/<name>/` — `area.json`, `rooms.json`, `mobiles.json`, `objects.json`
- Config tables live in `json/config/` — see `doc/Json_Documentation.md` §"Config Tables"
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
| 1–83    | Stock ROM skills/spells     |
| 200–204 | Extended stock entries      |
| 401–402 | Additional stock entries    |
| 500–523 | Extended/custom entries     |
| **524** | acid rain (first custom)    |
| 525+    | Available for new additions |
