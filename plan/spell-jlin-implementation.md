# ✅ PLAN COMPLETE
# Implementation Plan: 15 New John Lin Spells

## Status: Completed

## Overview

Implement 15 truly new spells from the John Lin list. Five spells (demonfire/547,
mass_invis/548, prot_evil/549, prot_good/550, stone_skin/551) already exist; skip them.

## Slot Assignments (15 new spells)

| Slot | Spell Name       | Function             | File          |
|------|-----------------|----------------------|---------------|
| 536  | detect undead   | spell_detect_undead  | spell_info.c  |
| 537  | find familiar   | spell_find_familiar  | spell_summon.c|
| 538  | light           | spell_light          | spell_aff.c   |
| 539  | mount           | spell_mount          | spell_summon.c|
| 540  | deafness        | spell_deafness       | spell_aff.c   |
| 541  | glitterdust     | spell_glitterdust    | spell_aff.c   |
| 542  | locate person   | spell_locate_person  | spell_info.c  |
| 543  | magic mouth     | spell_magic_mouth    | spell_misc.c  |
| 544  | acid arrow      | spell_acid_arrow     | spell_off.c   |
| 545  | explosive runes | spell_explosive_runes| spell_misc.c  |
| 546  | flame arrow     | spell_flame_arrow    | spell_off.c   |
| 550  | vampiric touch  | spell_vampiric_touch | spell_misc.c  |
| 552  | animate dead    | spell_animate_dead   | spell_summon.c|
| 553  | legend lore     | spell_legend_lore    | spell_info.c  |
| 555  | wish            | spell_wish           | spell_misc.c  |

## Key Technical Decisions

- **AFF flags**: No new AFF bits (all 31 used). Deafness uses affect presence check via sn.
- **ROOM_LIGHT**: Not using room flags. spell_light creates OBJ_VNUM_LIGHT_BALL with timer=level.
- **Summons area**: New `json/areas/summons/` with vnums 30000-30099 (hidden=true).
- **Mounts** (30001-30005): 5 tiers by level/10.
- **Familiars** (30010-30017): 8 types by random roll.
- **Zombie**: Reuse MOB_VNUM_ZOMBIE (vnum 1, from limbo area).
- **Magic mouth / Explosive runes**: Cosmetic objects (30025/30026) with timers; no trigger engine.
- **Familiar death penalty**: In fight.c before char_die().

## Files to Create
- `json/areas/summons/area.json`
- `json/areas/summons/mobiles.json`
- `json/areas/summons/objects.json`
- `src/spell_summon.h`
- `src/spell_summon.c`

## Files to Modify
- `src/spell_off.c` — add spell_acid_arrow, spell_flame_arrow
- `src/spell_off.h` — add 2 declarations
- `src/spell_aff.c` — add spell_light, spell_deafness, spell_glitterdust
- `src/spell_aff.h` — add 3 declarations
- `src/act_comm.c` — add deafness check in do_say, do_shout, do_yell
- `src/spell_info.c` — add spell_detect_undead, spell_locate_person, spell_legend_lore
- `src/spell_info.h` — add 3 declarations
- `src/spell_misc.c` — add spell_vampiric_touch, spell_wish, spell_magic_mouth, spell_explosive_runes
- `src/spell_misc.h` — add 4 declarations
- `src/fight.c` — add familiar death penalty + lookup.h include
- `src/spell_dispatch.c` — add #include "spell_summon.h" + 15 new entries
- `json/config/skills.json` — add 15 dormant skill entries
- `.github/agents/cheatsheet.md` — update slot table
- `src/resets.c` — include `lookup.h` for wear-loc helper prototypes used in equip warning path

## Risks
- fight.c familiar death: captures victim->master before char_die() invalidates victim
- spell_wish: uses number_range(0, 7), all paths tested
- Summons area: vnums 30000-30099 verified free
- Build dependency notes: `wear_loc_get_flag` helpers are provided by `lookup.c`; avoid redefining in table modules

## Status: COMPLETED
Date: 2026-04-12
