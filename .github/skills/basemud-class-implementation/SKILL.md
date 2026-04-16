---
name: basemud-class-implementation
description: "Use when: implementing a new playable class (warrior, caster, hybrid, etc.) into BaseMUD. Handles full workflow from legacy source analysis through JSON implementation, skill group creation, help documentation, and verification. Validates skill compatibility and documents missing abilities vs. legacy system."
---

# BaseMUD Class Implementation

Complete workflow for adding a new playable class to BaseMUD using JSON configuration files.

## Phase 1: Investigation

### Gathered Context
Before implementing, explore the existing class infrastructure:

1. **Review existing classes** in `json/config/classes.json`
   - Note all fields: `name`, `who_name`, `primary_stat`, `weapon`, `thac0` progression, `hp_gain`, `mana_gain`, `guild` array
   - Check for similar class archetypes (warrior-like, caster-like, hybrid)
   - Document stat progressions and skill group patterns

2. **Examine skill infrastructure** in `json/config/skills.json`
   - Extract complete skill list: `grep -o '"name":\s*"[^"]*"' json/config/skills.json | cut -d'"' -f4 | sort | uniq`
   - Note which skills apply per-class (each skill has per-class level/effort/spellcasting requirements)
   - Identify trainable vs. auto-granted abilities

3. **Check skill groups** in `json/config/skill_groups.json`
   - Understand grouping patterns (e.g., "warrior basics" free, category groups for point cost)
   - Note cost structure: `{"classname": {"cost": 0}}` for free, higher numbers for purchasable

4. **Document legacy source** (if porting from ROM/Circle/MERC/Diku)
   - Extract all `Skill 'name' level effort` entries
   - Note stat modifiers (Base/Max for each attribute)
   - Record class-specific thac0 progression, HP/mana formulae
   - List companion guilds/trainers (note vnums if different from player guilds)

### Tools for Investigation
- `grep_search` or `run_in_terminal` to extract skill names
- `semantic_search` for class patterns and skill functions
- `read_file` to review existing class definitions and skill infrastructure

---

## Phase 2: Planning

### Create Plan Document
Write `/plan/[classname]-class-implementation.md` before any code changes.

**Template Sections:**
```markdown
# [ClassName] Class Implementation

## Status
- [ ] Investigation Complete
- [ ] Plan Approved  
- [ ] Implementation Complete
- [ ] Build Verified
- [ ] Documentation Complete

## Overview
[Class philosophy, archetype, intended playstyle]

## Design Decisions
- **Primary Stat**: [DEX/STR/INT/WIS/CON/CHA] — reasoning
- **Combat Bonus**: [thac0 progression from legacy or comparable class]
- **Mana**: [true/false — can cast detection/tracking/enhancement spells?]
- **HP Progression**: [min-max per level]
- **Guild Vnums**: [two vnums for player guild + immortal area guild]
- **Signature Weapon**: [vnum from weapons]

## Files to Modify
- `json/config/classes.json` — Add class entry (68 title pairs + all stats)
- `json/config/skill_groups.json` — Add basic + default skill groups
- `json/help/help.json` — Add keyword help entry
- [Optional] `.are` area files — Create guild trainer rooms if using new vnums

## Legacy Skills Analysis
[If porting from legacy source]

### Existing BaseMUD Skills (✅ Available)
[List 1-50+ skills found in both legacy and modern system]

### Missing Legacy Abilities (❌ Not in BaseMUD)
[Categorized by type]
- **Stances/Forms** [if applicable]: ...
- **Specialized Attacks**: ...
- **ROM-Only Enhancements**: ...

## Risks & Dependencies
- Guild rooms: Two vnums required; if reusing existing, adjust class entry
- Skill coverage: [X% of legacy abilities available; Y% are ROM-specific enhancements]
- Build: All changes data-driven; no C code modification needed

## Next Steps for User
1. Create/assign guild room vnums
2. [Optional] Expand skill groups if desired
3. Build: `make clean && make -j4`
```

---

## Phase 3: Clarifying Questions

Ask the user (before implementation) to define scope:

1. **Stat Focus**: Which primary stat? (affects skill training bonus, playstyle)
2. **Mana**: Should the class cast spells, or physical combat only?
3. **Guild Location**: Use existing vnums or create new dedicated guild rooms?
4. **Skill Coverage**: Any custom skill groups beyond basics + defaults? (e.g., archery focus, stealth focus)
5. **Legacy Gaps**: OK with ROM-specific abilities not being implemented?

---

## Phase 4: Implementation

### Step 1: Add Class to `json/config/classes.json`

**Required Fields:**
```json
{
  "name": "classname",
  "who_name": "Abr",              // 3-char abbreviation in who list
  "primary_stat": "dex",           // Stat trained at creation
  "weapon": 3717,                  // Default weapon vnum
  "thac0_00": 18,                  // At level 1
  "thac0_32": 6,                   // At level 32+
  "hp_gain_min": 11,               // Per level
  "hp_gain_max": 15,
  "gains_mana": true,              // false = warrior-like, true = caster-like
  "base_group": "classname basics",  // Free at creation
  "default_group": "classname default", // Major skill offer (40-60 pts)
  "guild": [3025, 9640],           // Player guild vnum, Immortal area vnum
  "titles": [
    { "level": 1,  "male": "Title 1 M", "female": "Title 1 F" },
    { "level": 5,  "male": "Title 2 M", "female": "Title 2 F" },
    // ... 68 title pairs total (every level, or major level milestones)
  ]
}
```

**Stat Progression Reference:**
- **STR**: Warrior, Paladin (melee damage, carry limit)
- **DEX**: Ranger, Thief (combat accuracy, dodge, ranged weapons)
- **INT**: Mage, Cleric (spell power, mana pool, language comprehension)
- **WIS**: Cleric, Druid (healing, mana pool, perception)
- **CON**: All classes (HP gain, poison resistance)
- **CHA**: Bard (charm, persuade, animal companion)

### Step 2: Create Skill Groups in `json/config/skill_groups.json`

**Pattern 1: Basics (Free at Creation)**
```json
{
  "name": "classname basics",
  "skills": ["skill1", "skill2", ...],
  "classes": {"classname": {"cost": 0}}
}
```

**Pattern 2: Default (Major Offer)**
- Cost 40-60 points (normal cost)
- 12-20 skills covering core combat + utility
- Example: dodge, parry, rescue, disarm, second attack + detection/tracking

**Tips:**
- Always include `dodge` and `parry` (fundamental defense)
- Add `rescue` for team play
- Include 3-4 detection/utility skills (track, detect_hidden, etc.)
- Primary weapon skill must be in basics

### Step 3: Add Help Entry to `json/help/help.json`

**Format:**
```json
{
  "keywords": "CLASSNAME CLASSNAMES",
  "entry": "~[CLASSNAME] — Class philosophy paragraph.\n\nPrimary abilities: list key combat/utility skills.\n\nBest for players who: describe playstyle.\n\nRecommended attributes: [DEX/STR/INT/WIS/CON] for optimal performance."
}
```

---

## Phase 5: Debugging & Verification

### Build & Verify
```bash
make clean && make -j4  # Clean rebuild
# Check: No errors, binary ~4.0MB
```

### Syntax Check
- `json/config/classes.json`: All required fields present, no trailing commas
- `json/config/skill_groups.json`: Class names match exactly, skill names exist in skills.json
- `json/help/help.json`: Keywords unique, no control characters

### In-Game Verification (optional)
1. Create character with new class
2. Check `help classname` works
3. Verify skill training costs and availability
4. Test guild room access (if created)

---

## Phase 6: Documentation & Cheatsheet

### Update Plan File
Mark complete at top:
```markdown
# [ClassName] Class Implementation

✅ STATUS: IMPLEMENTATION COMPLETE
```

Append to bottom:
```markdown
## Status: COMPLETED
Date: YYYY-MM-DD
```

### Update `.github/agents/cheatsheet.md`
Add verified findings about class implementation workflow:
```markdown
## Class Implementation Pattern
- All classes data-driven; no C code changes
- Titles: 68 pairs (male/female per level or major milestone)
- Skill groups: basics (free) + default (purchasable)
- Guild: 2 vnums required (player + immortal area)
- Skills: Extract full list with `grep -o '"name":\s*"[^"]*"' json/config/skills.json | ...`
- Build: `make clean && make -j4` (verify ~4.0MB binary)
```

### Add to Credits (if porting legacy code)
Edit `json/help/credits.json` if implementing a non-original class snippet:
```json
{
  "name": "Class Name",
  "snippet_source": "ROM/Circle/Diku/etc.",
  "porter": "Your Name",
  "date": "YYYY-MM-DD"
}
```

---

## Phase 7: Review Checklist

Before marking complete, verify:

- [ ] Plan file created with full spec
- [ ] Class entry in classes.json (all 68 titles, correct stats, guild vnums)
- [ ] Skill groups created and linked to class
- [ ] Help entry added and accessible
- [ ] Build successful (no errors, binary present)
- [ ] Skills compatibility analysis documented (what exists vs. missing)
- [ ] Plan marked complete with date
- [ ] Cheatsheet updated (if new patterns learned)
- [ ] Credits updated (if ported from legacy)
- [ ] No unintended files modified
- [ ] Commit message clear and references plan

---

## Troubleshooting

### "Class not appearing in who list"
- Verify `name` field matches exactly (case-sensitive)
- Check `who_name` is 3 characters
- Rebuild: `make clean && make -j4`

### "Skill not trainable"
- Verify skill exists in skills.json (use extraction command above)
- Check skill group lists correct skill name (no typos)
- Confirm class name in skill group matches class.json exactly
- Rebuild

### "Guild access denied"
- Guild vnums must be created as actual rooms in .are files first
- Each room needs MOB_PRACTICE trainer NPC
- Verify vnums in class match .are room definitions
- Rebuild from clean

### "Title stuck at level 1"
- Verify titles array is complete (1-32+ levels or at major milestones)
- Check male/female titles both populated
- No gaps in level progression
- Rebuild

---

## Example: Ranger Class

Real implementation reference showing all steps:

- **Plan**: `/plan/ranger-class-implementation.md`
- **Classes Entry**: `name: "ranger"`, `primary_stat: "dex"`, `gains_mana: true`, `guild: [3025, 9640]`
- **Skill Groups**: "ranger basics" (spear, track) + "ranger default" (17 combat/detection)
- **Help**: `keywords: "RANGER RANGERS"` with DEX focus, archery/tracking description
- **Skills Analysis**: 42+ legacy Ranger skills found in BaseMUD, 26 legacy abilities not available (stances, forms, ROM enhancements)
- **Guild Rooms**: Pending user creation at vnums 3025 (grove.are), 9640 (newthalos.are)

---

## Key Takeaways

✅ **Data-driven**: All changes in JSON; no C code modification  
✅ **Phased approach**: Plan before code, verify after  
✅ **Complete audit**: Document legacy gaps explicitly  
✅ **Guild required**: Vnums must be created manually in .are files  
✅ **Build verification**: Always clean rebuild before committing  
✅ **Skill infrastructure**: Leverage existing 154+ skills; document what's missing  
