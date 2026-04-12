Q: Which item categories should be procedurally generated? Templates currently have partial values for consumables (potions, scrolls) and fixed stats on equipment (armor/weapons/jewelry).
A: Both — all item types
Q: Some templates already have values (e.g., anum 50 is a potion with spell1='cure serious'). Should generation override those, fill only the empty fields, or scale existing values upward?
A: 1 and randomize number and level of spells/skills for potions/scrolls
Q: Should there be quality tiers (like Diablo magic/rare) controlling affix count and power per item, or just level-based scaling?
A: Location-based: floor=1 affix, chest=2, boss=3
Q: When an item is procedurally enhanced (e.g., a sword gets +2 dam), should its short description change to hint at quality/stats?
A: Append quality tier to equipment names only
Q: Should each seed theme have its own spell/stat pool (e.g., crypt favors protection evil, cure poison; frost favors armor, stone skin), or one global beneficial pool for all seeds?
A: Theme Clusters but a seperate json so it can be dynamically edited at any time in the mudeditor
Q: For equipment affixes, should stat bonuses be thematic by item type (weapons get combat stats; caster items get int/wis/mana), or fully random?
A: Item-type-aware: weapons=hit/dam/str, armor=AC/con, jewelry=any
Q: Should boss-dropped items (from pd_trigger_boss_loot) also go through the enhancer as highest-quality items, or stay fixed?
A: What do you recommend?

Boss drop recommendation: Each seed should define a boss_drop_vnum in the new loot themes JSON. The authored boss item is always created using that vnum (if defined) at pd_trigger_boss_loot time — this gives each theme a "signature" drop. If no boss_drop_vnum is set, a random item from the pool gets the highest enhancement tier instead. This is clean, works well with the mudeditor, and gives each seed identity.

Q: For armor/weapons: should BASE stats (weapon dice e.g. 1d8, AC values e.g. vs_pierce=2) also scale with instance level, or only the AFFECT_T bonuses (hit, dam, str, ac) get generated?
A: Scale base stats (dice/AC) with level + add affixes
Q: Potions and scrolls: can they get multiple spells (e.g. cure critical + haste)? If so, how many max per tier?
A: 1-2: floor=1 spell, chest/boss=2 spells
Q: For boss loot's guaranteed authored item: should each seed define its own special boss_drop_vnum in config, or pick randomly from the heirloom pool?
A: What would you recommend?
Q: The theme cluster spell/stat pools need a JSON config. How much mudeditor UI do you want now?
A: Full editor page in mudeditor now

Further Considerations

1: Wand charges at generation time: Wands have recharge + charges values. Should charges be set to the template value, or randomized in a range (e.g., 3–15 based on tier)? Template currently has 15/15 for the detect hidden wand.
2: Affect clearing on equipment: The enhancer will affect_copy_to_obj new affixes, but template items already have authored affects (e.g., anum 62 sword has hit+1, dam+1 from the template). Should those be cleared before adding procedural affixes, or stacked on top?
3: MUD client discovery of new spells on potions: Do players use identify or sip to learn what's in a potion? If so, no extra code is needed. But if there's a /skills.json display in the editor showing owned potions, that may need a small update.

1: randomized in a range.
2: stacked
3: No sip exists, i suggest looking for identify and see what exists. Maybe a "do sip" needs to be added? Explain why. 

Plan: PD Procedural Item Enhancement (Final)
TL;DR: A new pd_enhance_obj(obj, inst, tier) function runs after every obj_create() call during instance generation, filling in complete item values based on location tier (floor=1, chest=2, boss=3). A pd_loot_themes.json config defines thematic spell/stat pools per seed, editable in a new mudeditor page. A do_sip command is added as a stretch goal to let players taste-test mystery potions.

Phase A — Data Model (parallel)
Define PD_LOOT_THEME_T struct in src/structs.h — seed_name, spell_pool[] (stored as int SNs + string names), stat_pool[] (APPLY_ constants), boss_drop_vnum, linked list pointers
Add recycler pd_loot_theme_new() / pd_loot_theme_free() in src/recycle.h + src/recycle.c
Add pd_loot_theme_first / pd_loot_theme_last globals in src/globals.c + src/globals.h
Phase B — JSON Loading (depends on A)
Create json/config/pd_loot_themes.json with entries for all existing seeds (crypt, cave, ruins, mountain_frost, stronghold_iron)
Add json_tblr_pd_loot_theme() in src/json_tblr.h + src/json_tblr.c — reads seed_name, spell_pool strings → skill_lookup_exact(), stat_pool strings → APPLY_ lookup, boss_drop_vnum
Add dispatch if (json->name == "pd_loot_theme") in src/json_import.c
Phase C — Enhancement Engine, new files (depends on A, B)
Create src/pd_loot.h — tier constants PD_QUALITY_FLOOR/CHEST/BOSS, pd_loot_theme_get(), pd_enhance_obj() declarations
Create src/pd_loot.c:
pd_loot_theme_get(seed_name) — linked list lookup, NULL = hardcoded global defaults
pd_enhance_consumable() — set v.value[0] = scaled level; roll 1 spell (floor) or 1–2 (chest/boss) from theme pool into v.value[1..4]; wands get randomized charges: tier 1→3–8, tier 2→6–12, tier 3→10–18 (both recharge and charges set to same roll)
pd_enhance_weapon() — scale dice_num and dice_size with instance level; stack APPLY_HITROLL + APPLY_DAMROLL affects; tier 3 adds STR or DEX bonus
pd_enhance_armor() — scale pierce/bash/slash/magic AC values; stack APPLY_AC affect; tier 2+ stacks additional stat bonus (CON or STR)
pd_enhance_jewelry() — stack tier number of stats from theme's stat_pool as affects
Naming: tier ≥ 2 equipment gets "a fine " prefix; tier 3 gets "a rare " prefix via str_replace_dup()
Affects are STACKED on top of template affects, not cleared
Main dispatch pd_enhance_obj() switches on obj->item_type
Phase D — Integration in src/pocket_dungeon.c (depends on C)
#include "pd_loot.h" in pocket_dungeon.c
Floor loot loop (Tier 3 drops): pd_enhance_obj(obj, inst, PD_QUALITY_FLOOR) after each obj_create()
Chest loot (Tier 1 chest) + hidden cache (Tier 2): pd_enhance_obj(obj, inst, PD_QUALITY_CHEST)
Boss loot — modify pd_trigger_boss_loot(): if theme->boss_drop_vnum > 0, create that vnum as authored (no enhancement); all other boss drops get pd_enhance_obj(..., PD_QUALITY_BOSS)
Phase E — Mudeditor Server (parallel with C/D)
Create web/server/src/routes/pdLootThemes.ts — GET /api/pd-loot-themes (public), PUT /api/pd-loot-themes (requireRole builder+)
Register route in web/server/src/index.ts
Phase F — Mudeditor Client (depends on E)
Create web/client/src/pages/PdLootThemesPage.tsx:
Left: theme list, add/delete theme
Right: spell_pool multiselect (dropdown built from known beneficial spell list), stat_pool multiselect (APPLY_ names), boss_drop_vnum number input
Save → PUT /api/pd-loot-themes
Uses SortSelect, CheatSheet from web/client/src/components/SortSelect.tsx; useQuery error aliased as loadError
Add route /pd-loot-themes to App.tsx inside ProtectedRoutes
Add nav link in AppShell.tsx
Phase G — do_sip (Stretch, independent)
Add sip <potion> command in act_obj.c / interp.c / interp.h — no consumption, returns vague thematic category hint based on which spells are stored; optionally skill-gated by lore
Relevant files

New: src/pd_loot.c, src/pd_loot.h, json/config/pd_loot_themes.json, web/server/src/routes/pdLootThemes.ts, web/client/src/pages/PdLootThemesPage.tsx
Modified: src/pocket_dungeon.c, src/structs.h, src/recycle.h, src/recycle.c, src/globals.c, src/globals.h, src/json_tblr.c, src/json_tblr.h, src/json_import.c
Verification

make build task — zero warnings on all new C files
Boot log shows pd_loot_themes: loaded N themes
pd create crypt → stat obj on floor items confirms non-zero values + 1 affix
Chest/boss items confirm 2–3 affixes and tier naming prefixes
Boss kill → authored heirloom + 2–3 enhanced drops confirmed
tsc: server + tsc: client tasks — zero errors
Browser /pd-loot-themes — save round-trips cleanly, server reloads confirm edits
Does this plan look good to approve? Any changes before handoff to implementation?

Plan looks good, but regarding the potions, is there a flag set on them that hides the spells until sip/identify/lore is used? if not, that should go with the independent stretch 

Good news and a design opportunity. Here's the full picture:

Good news: Spells are already naturally hidden. examine, look, inventory, and equipment never print spell contents — only lore (skill-gated) and identify (always works) do via spell_identify_perform(). So procedurally generated potions are already mysterious to players by default. No extra work is needed for basic mystery.

But there's a deeper design question raised by do_sip: Right now, if a player successfully uses lore on a potion today, there's no persistent state change — next time they lore the same potion it rolls again. There's also no visual indicator that an item is "unknown." ITEM_HIDDEN at BIT_24 is defined but completely unused and available for repurposing.

An ITEM_UNIDENTIFIED system (using ITEM_HIDDEN or a renamed version of it) could mean:

Procedurally generated consumables spawn with the flag set
do_sip gives a category hint; on full success, clears the flag
lore / identify: on success, clears the flag permanently
While flag is set: look shows "a mysterious vial" instead of the flavor description
Without the flag, the natural hiding works fine — but players who use lore twice get two random rolls on the same item, and there's no visual "I haven't identified this yet" state. The flag makes identification feel like a meaningful, persistent action.

I'd suggest adding this to the stretch phase alongside do_sip, since they form a natural pair:

G1 — Set ITEM_UNIDENTIFIED on all procedurally generated consumables at creation time. When set: look/examine show flavor description replaced with generic label (e.g. "a murky vial", "a worn scroll"). Gate lore and identify clearing the flag upon success.
G2 — do_sip gives thematic hint; on a high roll (based on lore skill check), also clears the flag.
G3 — In item_examine and item_look_at, add a case for potions/scrolls/wands: if ITEM_UNIDENTIFIED is set, suppress the authored description and print a generic label instead.
This is still independent of Phases A–F and doesn't block core delivery.