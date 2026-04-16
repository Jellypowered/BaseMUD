# Ranger Class Implementation Plan

## Overview
Implement the Ranger class from the legacy ROM/Envy MUD `.class` file format into BaseMUD's modern JSON-based system. The Ranger is a hybrid class combining martial prowess with wilderness tracking abilities.

## Legacy Format Analysis

### Source: Snippets/Pending/Classes/Ranger.class
- **Class ID**: 6
- **Primary Needs**: Attributes, combat stats, HP/Mana gains, skill listings, titles
- **File Format**: ROM text-based class definition

### Key Legacy Attributes
- **Primary Stat**: Weapon = 10313 (suggests DEX or WIS focus)
- **Combat Stats**: 
  - thac0_00 = 18, thac0_32 = 6
  - HP min/max = 11-15 (warrior-like)
  - Mana gain = 1 (very low, warrior tendency)
- **Skills**: 80+ skills listed with level/effort pairs (e.g., 'climb' 3 90)
- **Titles**: 68 level progression titles (male/female pairs)

---

## Implementation Checklist

### ✅ PHASE 1: Investigation (COMPLETE)
- [x] Examined existing classes.json structure (Mage, Cleric, Thief, Warrior)
- [x] Analyzed skill_groups.json patterns
- [x] Reviewed help system structure (help.json format)
- [x] Identified class JSON schema requirements

### ⏳ PHASE 2: Planning (CURRENT)
- [x] Document legacy vs. modern format differences
- [ ] Determine Ranger's primary stat (likely DEX or WIS for healing/tracking)
- [ ] Identify which guild rooms to assign (or create new ones)
- [ ] Map legacy skills to modern skill names
- [ ] Create skill group structure for Ranger

### ⏳ PHASE 3: Clarifying Questions
**AWAITING USER INPUT:**
1. **Primary Stat**: Should Ranger be DEX-focused (tracking/archery) or WIS-focused (nature affinity)? Legacy weapon=10313 doesn't clearly map.
2. **Guild Rooms**: Do we assign existing guild rooms or need new Ranger guild room vnums?
3. **Mana Capability**: Should Ranger gain mana (for spells) or be mana-less (warrior-like)?
4. **Skills**: Should we include all 80+ legacy skills or curate a Ranger-specific subset?
5. **Thac0 Values**: Use legacy values (18/6) or baseline warrior (20/-10)?

### 📋 PHASE 4: Implementation (PENDING)

#### 4.1 Update classes.json
**File**: `json/config/classes.json`
**Task**: Add Ranger class entry with:
- name: "ranger"
- who_name: "Ran" (or similar 3-letter abbreviation)
- primary_stat: [DEX or WIS - **NEEDS DECISION**]
- weapon: [vnum to determine - **NEEDS DECISION**]
- guild: [two guild room vnums - **NEEDS DECISION**]
- skill_adept: 75 (match warrior/thief pattern)
- thac0_00: 18 (from legacy)
- thac0_32: 6 (from legacy)
- hp_gain_min: 11 (from legacy)
- hp_gain_max: 15 (from legacy)
- gains_mana: [true/false - **NEEDS DECISION**]
- base_group: "ranger basics" (to create in 4.2)
- default_group: "ranger default" (to create in 4.2)
- can_sneak_away: [true/false - **NEEDS DECISION**]
- titles: [68 male/female title pairs from legacy .class file]

#### 4.2 Create Ranger Skill Groups (skill_groups.json)
**New Skill Groups to Create:**
```
- "ranger basics" (free at creation)
  - weapon: [bow/spear/sword]
  - tracking or stealth ability
  
- "ranger default" (40 point purchase on first login)
  - survival skills
  - tracking/detection skills
  - outdoor combat abilities
  - nature/animal interaction if applicable
```

#### 4.3 Add Ranger Help Entry
**File**: `json/help/help.json`
**Add**: New help keyword entry for Ranger class
- Explain class philosophy (nature, hunting, tracking)
- Primary stats and abilities
- Recommended playstyle

#### 4.4 Update Skill/Spell Compatibility
**File**: `json/config/skills.json` (likely already has all skills)
**Task**: Verify Ranger can access skills listed in legacy file
- Check each skill's class availability
- Add Ranger class access as needed for class-specific mechanics

### 📊 PHASE 5: What Exists vs. What Needs Creation

| Component | Status | Notes |
|-----------|--------|-------|
| Class JSON structure | ✅ EXISTS | Mage, Cleric, Thief, Warrior models exist |
| Skill system | ✅ EXISTS | 80+ skills already defined in skills.json |
| Skill groups | ✅ EXISTS | Pattern established, need Ranger-specific groups |
| Guild rooms | ❓ UNKNOWN | May need new vnums for Ranger-only guilds |
| Help system | ✅ EXISTS | help.json format documented |
| Class-specific spells | ✅ EXISTS | Inherited from skill_groups |
| **Ranger class entry** | ❌ MISSING | To be created |
| **Ranger skill groups** | ❌ MISSING | To be created |
| **Ranger help entry** | ❌ MISSING | To be created |

### 🔍 PHASE 6: Debugging & Verification
- [ ] Compile with new Ranger class in classes.json (make clean && make -j4)
- [ ] Verify Ranger appears in class selection on new login
- [ ] Test skill acquisition from Ranger skill groups
- [ ] Verify help entry displays correctly with `help ranger`

### 📝 PHASE 7: Documentation
- [ ] Update Json_Documentation.md if needed (Ranger-specific mechanics)
- [ ] Add Ranger to help credit system if applicable
- [ ] Document any race/class restrictions

### 🏆 PHASE 8: Credits/Cheatsheet
- [ ] Update `.github/agents/cheatsheet.md` with Ranger implementation notes
- [ ] Add to json/help/credits.json if new contributions noted

### 📍 Guild Room Vnums (TO BE CREATED IN .ARE FILES)

**Suggested Ranger Guild Rooms** (create in .are files at your convenience):
- **Ranger Guild Primary**: vnum **3025** in `grove.are` (area 89)
  - Room name: "Ranger's Guild" or "The Hunting Lodge"
  - Trainer NPC with MOB_PRACTICE flag for skill training
  
- **Ranger Guild Secondary**: vnum **9640** in `newthalos.are` (area 95)
  - Room name: "Ranger's Outpost" or "Ranger's Tower"
  - Secondary practice location for Ranger class

**How to Add These Rooms:**
1. Edit `area/grove.are` and add a room with vnum 3025 (check VNUMS section at top for area 89 range)
2. Edit `area/newthalos.are` and add a room with vnum 9640 (check VNUMS section at top for area 95 range)
3. Include MOB_PRACTICE trainer NPC in each room
4. Rebuild (`make clean && make -j4`) — rooms will load from JSON areas automatically
5. Class will reference these vnums: [3025, 9640]

**Alternative**: If these vnums conflict with existing content, adjust in implementation step 4.1 and document new vnums here.

---

### ✅ PHASE 9: Review & Git Commit
**Files to Modify:**
1. `json/config/classes.json` — Add Ranger class (using guild vnums [3025, 9640])
2. `json/config/skill_groups.json` — Add "ranger basics" and "ranger default"
3. `json/help/help.json` — Add Ranger help entry

**Files NOT Modified:**
- Source C files (JSON-driven classes, no code changes needed)
- Existing skills.json (Ranger uses existing skills)
- .are files (you'll add rooms separately)

**Expected Build Output:**
- Binary size ~4.1MB (unchanged from races implementation)
- No compilation warnings/errors
- Test: `make clean && make -j4`

**Post-Implementation Action Items:**
- [ ] Create Ranger guild rooms in grove.are (vnum 3025) and newthalos.are (vnum 9640)
- [ ] Add MOB_PRACTICE trainer NPCs to each guild room
- [ ] Rebuild and test Ranger class selection on new character

---

## Legacy .class File Reference

```
Name: Ranger~
Class: 6
AttrPrime: 1 (Maps to stat enum: 0=STR, 1=INT, 2=WIS, 3=DEX, 4=CON)
Races: 32236 (bitmask of allowed races)
Weapon: 10313 (weapon vnum)
Guild: 3039 (guild bitmask)
Skilladept: 90
Thac0: 18
Thac32: 6
Hpmin: 11
Hpmax: 15
Mana: 1 (gains_mana: false-ish, very little mana)
Expbase: 1125
Affected: 0
Resist: 0
Suscept: 0

Skills: 80+ lines of "Skill 'name' level effort"
Titles: 68 male/female pairs through level progression
```

---

## Decision Log

### RESEARCH FINDINGS:
- **AttrPrime=1 maps to INT** in ROM/Envy enum (0=STR, 1=INT, 2=WIS, 3=DEX, 4=CON)
- **Weapon vnum 10313** doesn't correspond to any school weapon (schools are 3700-3722)
  - Suggests a legacy area vnum, not a standard selection
- **Existing class primary stats**: Warrior=STR, Mage=INT, Cleric=WIS, Thief=DEX
- **Guild room patterns**: Cleric [3003,9619], Warrior [3022,9633] — no Rangers yet

### PENDING DECISIONS FOR USER:
1. **Primary Stat for Ranger**: 
   - Legacy specifies INT (AttrPrime=1), but Ranger archetype often uses DEX or WIS
   - **Options**: Keep INT for legacy compatibility, or change to DEX (hunter/tracking) or WIS (nature affinity)?
   - **RECOMMENDATION**: DEX (archery/tracking focus) or WIS (nature magic hybrid)

2. **Guild Room Vnums**:
   - Current assigned guild rooms: Mage [3018, 9618], Cleric [3003, 9619], Thief [?, ?], Warrior [3022, 9633]
   - **Should we**: Reuse adjacent existing rooms, or create new Ranger guild locations?
   - **RECOMMENDATION**: Research guild room availability before deciding

3. **Mana Capability** (gains_mana flag):
   - Legacy shows Mana=1 (very little)
   - **Should Ranger be**: Pure warrior (no spells, gains_mana=false)? Hybrid (gains some spells)?
   - **RECOMMENDATION**: gains_mana=false (warrior-like, matches HP/Mana gains of 11-15/-)

4. **can_sneak_away flag**:
   - Used by all existing classes: false (players can't sneak away from combat death)
   - **RECOMMENDATION**: false (matches all existing classes)

5. **Skill Adept Value**:
   - All existing classes use: 75 (perception threshold for learning skills)
   - **RECOMMENDATION**: 75 (standard, matches Warrior/Cleric)

---

## Status: BLOCKED - AWAITING USER INPUT

Cannot proceed to Phase 4 (Implementation) without clarification on design decisions above.
