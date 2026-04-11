# MUDEditor Sync Audit — Phase 2 Changes

## Overview
BaseMUD has received ~400 lines of new features (Phase 2) that need to be reflected in MUDEditor for complete functionality and UI support.

## Out-of-Sync Issues

### 1. **PocketDungeonConfig Type** ❌ OUT OF SYNC
**File:** `web/shared/types/index.ts`

**Missing fields in MUDEditor:**
```typescript
// Current definition (6 fields)
export interface PocketDungeonConfig {
  autopurge: boolean;
  empty_timeout_mins: number;
  max_instances: number;
  vnum_base: number;
  vnum_size: number;
  max_members: number;
  scaling_formula: number;
  // ❌ MISSING: testing_mode and gold_cost_per_level
}

// Should include:
  testing_mode: boolean;              // NEW in BaseMUD
  gold_cost_per_level: number;        // NEW in BaseMUD
```

**Impact:** Config UI will not display or allow editing of these fields.

---

### 2. **PocketDungeonInstance Type** ❌ OUT OF SYNC
**File:** `web/shared/types/index.ts`

**Missing fields in MUDEditor:**
```typescript
// Current definition
export interface PocketDungeonInstance {
  slot: number;
  id: number;
  theme: string;
  level: number;
  created_at: number;
  last_empty_at: number;
  entry_vnum: number;
  area_name: string;
  members: string[];
  rooms: PocketDungeonInstanceRoom[];
  // ❌ MISSING: affix and power tracking fields
}

// Should include:
  affixes?: number[];          // Array of affix enum values (C1)
  affix_count?: number;        // Count of active affixes (C1)
  rooms_cleared?: number;      // Difficulty progression counter (C2)
  boss_killed?: boolean;       // Boss loot trigger guard (C3)
  boss_powers?: number[];      // Array of power enum values (C4)
  boss_power_count?: number;   // Count of boss powers (C4)
```

**Impact:** Instance details page won't display affix/power information; snapshot data will be incomplete.

---

### 3. **MOB_CURSED Flag** ❌ NOT REGISTERED
**File:** `web/client/src/components/FlagsField.tsx`

**Missing:**
```typescript
// Current MOB_FLAGS array (21 flags)
export const MOB_FLAGS = [
  'sentinel', 'scavenger', 'aggressive', 'stay_area', 'wimpy', 'pet',
  'train', 'practice', 'noquest', 'undead', 'cleric', 'mage', 'thief', 'warrior',
  'noalign', 'nopurge', 'outdoors', 'indoors', 'healer', 'gain',
  'update_always', 'changer',
  // ❌ MISSING: 'cursed'
] as const;

// Should include:
  'cursed',  // NEW in BaseMUD (flag 30) — pocket dungeon cursed affix marker
```

**Missing in FLAG_TIPS:**
```typescript
  cursed: 'Pocket dungeon cursed affix — healing effects are reversed for this mob',
```

**Impact:** Editors cannot see/edit MOB_CURSED flag on mobs; pocket dungeon cursed-flagged mobs won't display properly.

---

### 4. **Affix & Power Enums** ❌ NOT DEFINED
**Files:** `web/shared/types/index.ts` (new)

**Missing enum definitions:**
```typescript
// NEW: Pocket dungeon affix types (C1)
export enum PocketDungeonAffixType {
  NONE = 0,
  STONY = 1,        // +20% mob HP
  CURSED = 2,       // Healing reversed (MOB_CURSED flag)
  SWIFT = 3,        // +25% hitroll
  ANCIENT = 4,      // +50% density
  LUMINOUS = 5,     // -20% AC
}

// NEW: Boss power types (C4)
export enum PocketDungeonPowerType {
  NONE = 0,
  STRIKE = 1,       // Power Strike: stun 50% every 8 rounds
  AURA = 2,         // Healing Aura: +5 HP/round
  SUMMON = 3,       // Summon Guardian at 50% HP
  DODGE = 4,        // Dodge Stance: +30% dodge for 3 rounds
  DRAIN = 5,        // Life Drain: heal 20% damage
}
```

**Impact:** Cannot display affix/power names in UI; developers must use magic numbers instead of readable enum values.

---

### 5. **ConfigTab UI** ❌ INCOMPLETE
**File:** `web/client/src/pages/PocketDungeonPage.tsx` (ConfigTab function)

**Missing control rows:**
```typescript
// After the existing fields (lines 101-122), add:
['testing_mode', 'Testing mode', 'When enabled, dungeon entry is free (no gold cost)'],
['gold_cost_per_level', 'Gold cost per level', 'Gold required to enter dungeons (e.g., 100 means level 10 = 1000 gold)'],
```

**Impact:** Admins cannot toggle testing mode or adjust gold costs without manual JSON editing.

---

## Sync Impact Summary

| Component | Type | Severity | Blocking |
|-----------|------|----------|----------|
| PocketDungeonConfig type | Type def | HIGH | No (uses defaults) |
| PocketDungeonInstance type | Type def | HIGH | No (optional fields) |
| MOB_CURSED flag | Flag reg | MEDIUM | No (UI read-only) |
| Affix/Power enums | Enum | MEDIUM | No (not rendered yet) |
| ConfigTab UI | UI | MEDIUM | No (admin only) |

## Recommended Action Plan

### Phase 2a: Essential Sync (1 hour)
1. Add `testing_mode` and `gold_cost_per_level` to PocketDungeonConfig type
2. Add new fields to PocketDungeonInstance type (optional)
3. Add `cursed` to MOB_FLAGS and FLAG_TIPS
4. Add config fields to ConfigTab UI

### Phase 2b: Optional Polish (2 hours)
1. Create Affix/Power enums in shared types
2. Create `InstanceDetailsModal` with affix/power display
3. Add cheatsheet documentation for new fields

### Phase 2c: Future (backend integration needed)
1. Create AffixView component to display instance affixes
2. Create PowerView component to display boss powers
3. Add affix/power filtering to instances list

---

## Files to Modify

| File | Changes | Priority |
|------|---------|----------|
| `web/shared/types/index.ts` | Add config + instance fields, enum defs | NOW |
| `web/client/src/components/FlagsField.tsx` | Add MOB_CURSED to MOB_FLAGS + tips | NOW |
| `web/client/src/pages/PocketDungeonPage.tsx` | Add config form fields | NOW |
| `web/client/src/pages/PocketDungeonPage.tsx` | Add instance details display | LATER |

---

**Audit Date:** April 11, 2026
**BaseMUD Commit:** a956519 (C7 gold cost)
**Severity:** LOW-MEDIUM (all optional/backward-compatible)
**Recommendation:** Sync PocketDungeonConfig type + ConfigTab immediately; affixes/powers can be polished later
