# Plan: Lunge Skill Integration

## Source
ROM/custom snippet. Skill for **paladin** and **vampire** classes.
A powerful impaling attack requiring a sword, spear, or polearm.

## Investigation Findings

| ROM symbol | BaseMUD equivalent | Notes |
|---|---|---|
| `CHAR_DATA *` | `CHAR_T *` | Type rename |
| `OBJ_DATA *` | `OBJ_T *` | Type rename |
| `drain_level` | **removed** | Does not exist in BaseMUD |
| `get_skill(ch, gsn)` | `char_get_skill(ch, SN(X))` | Different API |
| `check_improve(ch, gsn, success, mult)` | `player_try_skill_improve(ch, SN(X), success, mult)` | |
| `gsn_enhanced_damage` | `SN(ENHANCED_DAMAGE)` | Via SKILL_MAP |
| `GET_DAMROLL(ch)` | `GET_DAMROLL(ch)` | Macro exists in chars.h |
| `get_eq_char(ch, WEAR_WIELD)` | `char_get_eq_by_wear_loc(ch, WEAR_LOC_WIELD)` | |
| `wield->pIndexData->new_format` | `wield->obj_index->new_format` | |
| `wield->value[0]` | `wield->v.weapon.weapon_type` | Union accessor |
| `wield->value[1]`, `wield->value[2]` | `wield->v.weapon.dice_num`, `wield->v.weapon.dice_size` | |
| `is_safe(ch, victim)` | `do_filter_can_attack(ch, victim)` (returns TRUE if blocked) | |
| `get_char_room(ch, arg)` | `find_char_same_room(ch, arg)` | |
| `damage(ch, victim, ...)` | `damage_visible(ch, victim, ..., NULL)` | |
| `act(..., TO_NOTVICT)` + `TO_CHAR` + `TO_VICT` | `act3(to_ch, to_victim, to_others, ...)` | |
| `get_curr_stat(ch, STAT_X)` | `char_get_curr_stat(ch, STAT_X)` | |
| `ch->pcdata->learned[gsn]` | `ch->pcdata->learned[SN(LUNGE)]` | Same field, SN() macro |
| `skill_table[gsn].beats` | `skill_table[SN(LUNGE)].beats` | |
| `skill_table[gsn].classes[ch->class].level` | same pattern | |

### `drain_level` removal
`drain_level` is a custom vampire stat that does not exist in BaseMUD. References are removed:
- `chance += ch->drain_level + ch->level - victim->level - victim->drain_level` → `chance += ch->level - victim->level`
- multiplier using `drain_level` → removed entirely; the multiplier collapses to 1 for mortal-level chars anyway

## Files to Modify or Create

| File | Change |
|---|---|
| `plan/lunge-skill.md` | This file (create) |
| `src/defs.h` | Add `SKILL_MAP_LUNGE = 56`, update `SKILL_MAP_MAX = 57` |
| `src/tables.c` | Add `{SKILL_MAP_LUNGE, "lunge"}` to `skill_map_table[]` |
| `src/act_skills.h` | Add `DECLARE_DO_FUN(do_lunge)` |
| `src/act_skills.c` | Add `DEFINE_DO_FUN(do_lunge)` implementation |
| `src/interp.c` | Register `{"lunge", do_lunge, POS_FIGHTING, ...}` in command table |
| `json/config/skills.json` | Add lunge skill entry (slot 556, paladin+vampire classes) |
| `json/help/lunge.json` | Help entry for lunge skill |
| `json/help/credits.json` | Append credit entry |
| `.github/agents/cheatsheet.md` | Update slot table (556 = lunge) |

## Risks / Dependencies
- Slot 556 must be unique — confirmed next available after 555 (wish).
- `SKILL_MAP_MAX` must be incremented every time a new map entry is added.
- `ch->pcdata->learned[SN(LUNGE)]` is only safe for non-NPC characters; NPC path guarded by early return.

## Status: COMPLETED
