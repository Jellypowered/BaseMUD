✅ PLAN COMPLETE

## Plan: PD Loot Enhancement — Full Implementation

Brings the loot system up to the original spec by adding spell_pool/stat_pool/boss_drop_vnum
to the theme struct, completing pd_loot.c with tier bonuses and naming prefixes, adding all
5 seed entries to the JSON, and expanding the MUDEditor LootTab fields.

---

### A — structs.h additions to PD_LOOT_THEME_T
- `#define PD_MAX_SPELL_POOL 16`
- `#define PD_MAX_STAT_POOL  8`
- Add fields: `int spell_pool[PD_MAX_SPELL_POOL]`, `int spell_pool_count`,
  `int stat_pool[PD_MAX_STAT_POOL]`, `int stat_pool_count`, `int boss_drop_vnum`

### B — recycle.c pd_loot_theme_init
- Init new fields to 0

### C — json_tblr.c json_tblr_pd_loot_theme expansion
- Add `*spell_pool`, `*stat_pool`, `*boss_drop_vnum` to `json_import_expect`
- READ_PROP_INT for boss_drop_vnum
- Iterate `spell_pool` string array → `skill_lookup_exact()` per entry
- Iterate `stat_pool` string array → `type_lookup_exact(affect_apply_types, ...)` per entry

### D — pd_loot.h additions
- Declare `PD_LOOT_THEME_T *pd_loot_theme_get(const char *seed_name);`

### E — pd_loot.c improvements
1. Rename `pd_loot_find_theme` → non-static `pd_loot_theme_get` (matches plan API)
2. `pd_pick_spell_sn(th)` — accept theme, draw from `th->spell_pool` first, fallback to
   hardcoded pool when theme has no spell_pool
3. Jewelry/stat selection — draw from `th->stat_pool` when available, cycle INT/WIS/CON fallback
4. Weapon tier-3: add APPLY_STR or APPLY_DEX affix (random 50/50)
5. Armor tier-2+: add APPLY_CON (tier 2) or APPLY_STR (tier 3) affix
6. Naming prefix at end of pd_enhance_obj() for non-consumables:
   - tier >= PD_QUALITY_CHEST: `snprintf(buf, ..., "a fine %s", obj->short_descr)` → str_replace_dup
   - tier >= PD_QUALITY_BOSS: `"a rare %s"` (overrides fine)

### F — pocket_dungeon.c boss_drop_vnum in pd_trigger_boss_loot
- After getting seed, also call `pd_loot_theme_get(inst->theme)`
- If theme && theme->boss_drop_vnum > 0: create that vnum at inst->level+5, no pd_enhance_obj
- Existing random drops continue unchanged (still get PD_QUALITY_BOSS)

### G — json/config/pd_loot_themes.json
- Add seed entries for: crypt, cave, ruins, mountain_frost, stronghold_iron
- Each entry includes seed-appropriate spell_pool (strings) and stat_pool (strings) arrays
- boss_drop_vnum set to 0 for all (populated by builders at runtime)

### H — MUDEditor LootTab in PocketDungeonPage.tsx
- Add spell_pool: textarea for comma-separated spell names (maps to string[])
- Add stat_pool: textarea for comma-separated apply type names (maps to string[])
- Add boss_drop_vnum: number input

---

## Files Modified
- `src/structs.h`
- `src/recycle.c`
- `src/json_tblr.c`
- `src/pd_loot.h`
- `src/pd_loot.c`
- `src/pocket_dungeon.c`
- `json/config/pd_loot_themes.json`
- `mudeditor/web/client/src/pages/PocketDungeonPage.tsx`

## Status: COMPLETED — 2025-07-16
