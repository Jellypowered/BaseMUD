# Counter Skill Implementation

## Overview
Implement a defensive fighting technique that allows skilled fighters to reverse opponent attacks and counter with half damage.

## Features
- Automatic trigger: victim's counter activates when being hit
- Success based on: skill %, level diff, dexterity, weapon skill
- Returns half damage to attacker
- Improves with use upon successful counter

## Files to Modify

### 1. json/config/skills.json
- **Add**: Counter skill entry after other combat skills (near backstab, bash)
- **Location**: Find appropriate alphabetical position or add at end
- **Format**: Match existing skills.json structure

### 2. src/fight.c
- **Add function**: `check_counter()` - handles counter mechanics
- **Hook location**: In `one_hit()`, before `damage_visible()` call (line ~430)
- **Integration**: If counter succeeds, apply counter damage and return

### 3. json/help/counter.json (optional)
- **Add**: Help entry explaining counter skill
- **Format**: Match other help entries (see bandage.json)

## Counter Success Calculation

```
chance = skill / 6
chance += (victim_level - attacker_level) / 2
chance += 2 * (victim_DEX - attacker_DEX)
chance += victim_weapon_skill - attacker_weapon_skill
chance += (victim_STR - attacker_STR)

If number_percent() < chance: success
```

## Counter Mechanics
1. Check prerequisites:
   - Victim has weapon equipped
   - Victim is awake and can see attacker
   - Victim has skill > 0
   - Must be in combat (ch->fighting != NULL)
   
2. Calculate success chance (see formula above)

3. On success:
   - Apply counter attack (dam/2) back to attacker
   - Improve victim's counter skill
   - Send messages
   - Return TRUE (skip normal damage)

4. On failure:
   - Return FALSE (continue normal damage)

## JSON Entry Structure

```json
{
    "skill": {
        "name": "counter",
        "classes": {
            "mage": { "level": 53, "effort": 0 },
            "cleric": { "level": 15, "effort": 4 },
            "thief": { "level": 9, "effort": 5 },
            "warrior": { "level": 5, "effort": 5 }
        },
        "target": "ignore",
        "min_position": "fighting",
        "usage_beats": 0
    }
}
```

## Implementation Notes
- Counter is automatic (no do_fun needed) - checked every time character is hit
- Only triggers if defender can see/reach attacker
- Half damage to prevent game imbalance (strong defensive option)
- Uses existing infrastructure: gsn, learned[], player_try_skill_improve()
- BaseMUD automatically handles JSON skill loading

## Status: COMPLETED - April 16, 2026

## Implementation Summary

### Files Modified
1. **json/config/skills.json** - Added counter skill entry between "continual light" and "control weather"
2. **src/fight.c** - Implemented check_counter() function and hooked into one_hit()
3. **json/help/counter.json** - Created help documentation

### Counter Skill Details
- **Availability**: Warriors (level 5), Thieves (level 9), Clerics (level 15), Mages (unavailable)
- **Automatic Trigger**: Activates when character is hit in combat
- **Success Formula**:
  - Base: skill ÷ 6
  - Level diff: (victim_level - attacker_level) ÷ 2
  - Dexterity: 2 × (victim_DEX - attacker_DEX)
  - Weapon skills: victim_weapon_skill - attacker_weapon_skill
  - Strength: victim_STR - attacker_STR

### Mechanics
- **Prerequisites**: Victim must be awake, can see attacker, and have weapon equipped
- **On Success**: Applies half damage back to attacker, improves victim's skill
- **Messages**: Three-part message system (to victim, to attacker, to others)
- **Skill Improvement**: Called via player_try_skill_improve() on successful counter

### Implementation Approach
- Used skill_lookup("counter") for dynamic skill resolution
- Integrated with BaseMUD's existing damage system
- Returns early from one_hit() on successful counter to prevent normal damage
- Follows BaseMUD conventions for damage calculations and messaging

### Build Verification
- ✅ Clean build with -Werror (no warnings treated as errors)
- ✅ Binary: 4.0M ELF executable (built successfully)
- ✅ No compilation errors or warnings

### Files Moved
- Snippets/Pending/counter.c → Snippets/Completed/counter.c

## Status: COMPLETED - April 16, 2026
