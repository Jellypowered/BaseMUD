# Plan: Sharpen Skill Integration

## Source
SMAUG-style snippet by Froboz (froboz@cyberdude.com).
Warrior-only skill that uses a sharpening stone from inventory to add a damroll boost to a carried weapon, marking it with ITEM_SHARP.

## Investigation Findings

| SMAUG symbol | BaseMUD equivalent |
|---|---|
| `CHAR_DATA *` | `CHAR_T *` |
| `OBJ_DATA *` | `OBJ_T *` |
| `AFFECT_DATA *` | `AFFECT_T *` |
| `get_obj_carry(ch, arg)` | `find_obj_own_inventory(ch, arg)` |
| `ch->first_carrying` / `next_content` | `ch->content_first` / `content_next` |
| `CREATE(paf,AFFECT_DATA,1) + LINK(...)` | `affect_new() + affect_init() + affect_to_obj_back() + affect_modify_obj()` |
| `learn_from_success/failure` | `player_try_skill_improve(ch, SN(SHARPEN), bool, difficulty)` |
| `damage(ch, ch, ...)` | `damage_visible(ch, ch, ..., DAM_SLASH, NULL)` |
| `ms_find_obj(ch)` | removed — no equivalent |
| `separate_obj(obj)` | removed — no equivalent |
| `AT_RED/AT_GREEN color args` | removed — BaseMUD uses plain act/send_to_char |
| `paf->duration = 0` | `affect_init(..., -1, ...)` — duration -1 = permanent on object |
| `format_obj_to_char` | `obj_format_to_char` in `src/objs.c` |
| `percent` uninitialized | assigned `percent = number_percent()` |
| `IS_OBJ_STAT(obj, ITEM_SHARP)` | `IS_OBJ_STAT(obj, ITEM_SHARP)` — same macro |

### New ITEM_SHARP flag
- `ITEM_UNIDENTIFIED` is the last flag at BIT_28.
- `ITEM_SHARP` = BIT_29 (next available bit).
- Must be added to both `flags.h` (C constant) and `flags.c` (name registration).

### Sharpening stone object
- Original vnum = 38; limbo area has min_vnum=0, max_vnum=99. Anum 38 is unused.
- Add `OBJ_VNUM_STONE 38` to `defs.h`.
- Add anum 38 object to `json/areas/limbo/objects.json`.

### Skill slot
- Next available slot: **557** (after lunge = 556).
- SKILL_MAP_SHARPEN = 57, SKILL_MAP_MAX = 58.

## Files to Modify or Create

| File | Change |
|---|---|
| `plan/sharpen-skill.md` | This file (create) |
| `src/defs.h` | Add `OBJ_VNUM_STONE 38`, `SKILL_MAP_SHARPEN 57`, update `SKILL_MAP_MAX 58` |
| `src/flags.h` | Add `#define ITEM_SHARP (BIT_29)` |
| `src/flags.c` | Add `{"sharp", ITEM_SHARP, TRUE}` to `extra_flags[]` |
| `src/tables.c` | Add `{SKILL_MAP_SHARPEN, "sharpen"}` to `skill_map_table[]` |
| `src/act_skills.h` | Add `DECLARE_DO_FUN(do_sharpen)` |
| `src/act_skills.c` | Add `DEFINE_DO_FUN(do_sharpen)` implementation |
| `src/interp.c` | Register `{"sharpen", do_sharpen, POS_STANDING, 0, LOG_NORMAL, 1}` |
| `src/objs.c` | Add `(Sharp)` display for `ITEM_SHARP` in `obj_format_to_char` |
| `json/areas/limbo/objects.json` | Add sharpening stone at anum 38 |
| `json/config/skills.json` | Add sharpen skill (slot 557, warrior class lvl 12) |
| `json/help/sharpen.json` | New help file |
| `json/help/credits.json` | Append credit for Froboz |
| `.github/agents/cheatsheet.md` | Add slot 557 = sharpen |

## Risks / Dependencies
- SKILL_MAP_MAX must be bumped when adding SKILL_MAP_SHARPEN.
- BIT_29 must be confirmed free (it is — flags.c ends at BIT_28).
- Affect duration -1 = permanent on object; timer on object controls decay.
- Object carries `enchanted = FALSE` so the prototype affects flow through; only instance affect applied.

## Status: COMPLETED
