## Plan: PD Loot Bug Fixes

Four bugs found during audit of the pd-loot-identify implementation before commit.

---

### Bug 1 — New `ITEM_UNIDENTIFIED` flag instead of `ITEM_HIDDEN`

`pd_enhance_obj()` was setting `ITEM_HIDDEN` on generated potions/wands. In BaseMUD,
`ITEM_HIDDEN` makes items invisible to anyone without `detect_hidden` (chars.c:506),
so dungeon floor potions would be completely invisible to players. The intended mechanic
is "mystery potion" (identity unknown, but item is visible). A new `ITEM_UNIDENTIFIED`
flag separate from the visibility system is the correct approach.

Files to modify:
- `src/flags.h` — add `#define ITEM_UNIDENTIFIED (BIT_28)` after ITEM_CORRODED
- `src/flags.c` — add `{"unidentified", ITEM_UNIDENTIFIED, TRUE}` to `extra_flags[]`
- `src/pd_loot.c` — replace both `ITEM_HIDDEN` references with `ITEM_UNIDENTIFIED`
- `src/spell_info.c` — replace `ITEM_HIDDEN` reference with `ITEM_UNIDENTIFIED`

---

### Bug 2 — `do_sip` doesn't work on potions; rewrite as pure taste-test

`item_is_drinkable()` rejects `ITEM_POTION` (only accepts ITEM_DRINK_CON/ITEM_FOUNTAIN),
so potions returned "You can't sip from that." Also `item_quaff_effect()` extracts
(destroys) the object, which is wrong for a taste-test. Sip is redefined as: accept only
`ITEM_POTION`, reveal `ITEM_UNIDENTIFIED` flag (remove it, print identification message),
no consumption, no spell effects.

Files to modify:
- `src/act_obj.c` — rewrite `do_sip()` to target ITEM_POTION, remove ITEM_UNIDENTIFIED,
  no extract/quaff

---

### Bug 3 — `pd_loot_themes.json` wrong format (zero themes load at boot)

The file was written as a flat array `[{"name":"default",...}]` but both:
- The C loader (`json_import.c`) dispatches on `json->name == "pd_loot_theme"`, which only
  matches the object key in the wrapped format
- The TS server `readConfig()` does `parsed.map(item => item[wrapKey])`, expecting
  `[{"pd_loot_theme": {...}}]`

Result: zero themes load at boot; editor reads `[undefined]`.

Fix: rewrite the file to the standard wrapped format used by every other config file.

Files to modify:
- `json/config/pd_loot_themes.json` — rewrite to wrapped format

---

### Bug 4 — Stray `package-lock.json` in BaseMUD repo root

An empty `package-lock.json` (lockfileVersion 3, no packages) was created in
`f:\Source\BaseMUD\` by the previous agent. Should not be tracked in the C MUD repo.

Action: delete the file.

---

## Files Modified / Created

- `src/flags.h` — add ITEM_UNIDENTIFIED (BIT_28)
- `src/flags.c` — register "unidentified" in extra_flags[]
- `src/pd_loot.c` — use ITEM_UNIDENTIFIED instead of ITEM_HIDDEN
- `src/spell_info.c` — use ITEM_UNIDENTIFIED instead of ITEM_HIDDEN
- `src/act_obj.c` — rewrite do_sip() as ITEM_POTION taste-test
- `json/config/pd_loot_themes.json` — fix to wrapped format
- deleted: `package-lock.json` (BaseMUD root)

## Status: COMPLETED — 2026-04-12
