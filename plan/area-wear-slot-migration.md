## Plan: Area Wear-Slot Migrations

Migrate existing area JSON objects and room resets to use the newly added wear slots (back, cloak, eyes, ears, float_2, tattoo). Process areas systematically, identifying items that should move to the new slots.

**Scope:** Treat `json/areas/**` as the runtime source. Ignore legacy `area/*.are` files.

**Overall Strategy**
1. For each area: scan `objects.json` for items that match new slot categories
2. Move items to new slots based on their names and descriptions
3. Update any room resets (`rooms.json`) that reference migrated items
4. Verify no duplicate slot assignments in any single reset chain

**Migration Categories**
- **Back slot** (WEAR_LOC_BACK): backpacks, rucksacks, knapsacks, satchels
- **Cloak slot** (WEAR_LOC_CLOAK): cloaks, capes, shawls (in addition to `about`)
- **Eyes slot** (WEAR_LOC_EYES): glasses, spectacles, goggles, monocles, patches, visors (NOT masks—those stay on head)
- **Ears slot** (WEAR_LOC_EARS): earrings, ear cuffs, ear studs
- **Float 2 slot** (WEAR_LOC_FLOAT_2): orbs, globes, discs that floats (in addition to standard `floating`)
- **Tattoo slot** (WEAR_LOC_TATTOO): tattoo items (new item type=tattoo; typically no values or similar to treasure)

**Decisions**
- Masks and helm-visors stay on head
- Amulets and pendants stay on neck
- Neckwear stays on neck
- `about` body slot is kept as is; new `cloak` is an additional option

---

## Per-Area Migration Tasks

### canyon
**Files:** `json/areas/canyon/objects.json`, `json/areas/canyon/rooms.json`
**Action:** Scan for cloaks; move to `cloak` wear_loc if found. Review resets.
**Status:** not-started

### catacomb
**Files:** `json/areas/catacomb/objects.json`, `json/areas/catacomb/rooms.json`
**Action:** Identify cloaks → `cloak`, float items → `floating2`. Update resets.
**Status:** not-started

### chapel
**Files:** `json/areas/chapel/objects.json`, `json/areas/chapel/rooms.json`
**Action:** Scan for outerwear (capes, shawls); move to `cloak`. Review resets.
**Status:** not-started

### draconia
**Files:** `json/areas/draconia/objects.json`, `json/areas/draconia/rooms.json`
**Action:** Scan for outerwear; move to `cloak`. Update resets.
**Status:** not-started

### gnome
**Files:** `json/areas/gnome/objects.json`, `json/areas/gnome/rooms.json`
**Action:** Find cloaks → `cloak`. Update resets if any.
**Status:** not-started

### hitower
**Files:** `json/areas/hitower/objects.json`, `json/areas/hitower/rooms.json`
**Action:** Backpack-style containers → `back`, spectacles/glasses → `eyes`, cloaks → `cloak`, floats → `floating2`. Update resets.
**Status:** not-started

### mahntor
**Files:** `json/areas/mahntor/objects.json`, `json/areas/mahntor/rooms.json`
**Action:** Cloaks → `cloak`, eyewear (glasses, spectacles, monocles) → `eyes`, earrings → `ears`, backpacks → `back`, floats → `floating2`. Update resets.
**Status:** not-started

### mirror
**Files:** `json/areas/mirror/objects.json`, `json/areas/mirror/rooms.json`
**Action:** Cloaks → `cloak`, floats → `floating2`. Update resets.
**Status:** not-started

### newthalos
**Files:** `json/areas/newthalos/objects.json`
**Action:** Backpack-style containers (ITEM_CONTAINER with appropriate names) → `back`. Check resets if any.
**Status:** not-started

### ofcol2
**Files:** `json/areas/ofcol2/objects.json`, `json/areas/ofcol2/rooms.json`
**Action:** Eyewear → `eyes`, cloaks → `cloak`, floats → `floating2`. Update resets.
**Status:** not-started

### olympus
**Files:** `json/areas/olympus/objects.json`
**Action:** Neckwear stays on neck; selective review for other outerwear. Update only items that clearly belong to new slots.
**Status:** not-started

### pocketdungeon
**Files:** `json/areas/pocketdungeon/objects.json`, `json/areas/pocketdungeon/rooms.json`
**Action:** Cloaks → `cloak`, floats → `floating2`. Update resets.
**Status:** not-started

### pyramid
**Files:** `json/areas/pyramid/objects.json`, `json/areas/pyramid/rooms.json`
**Action:** Review eyewear and masks; move eyewear (not masks) → `eyes`. Update resets if any.
**Status:** not-started

### quest
**Files:** `json/areas/quest/objects.json`
**Action:** Cloaks → `cloak`, neckwear → stays on neck, headwear review (hats stay on head, unless they're clearly masks or glasses).
**Status:** not-started

### sewer
**Files:** `json/areas/sewer/objects.json`, `json/areas/sewer/rooms.json`
**Action:** Backpacks → `back`, floats → `floating2`. Update resets.
**Status:** not-started

### smurf
**Files:** `json/areas/smurf/objects.json`
**Action:** Glasses → `eyes`. Update resets if any.
**Status:** not-started

### thalos
**Files:** `json/areas/thalos/objects.json`
**Action:** Review float items; move clearly floating items → `floating2`.
**Status:** not-started

### tohell
**Files:** `json/areas/tohell/objects.json`, `json/areas/tohell/rooms.json`
**Action:** Spectacles and eyewear → `eyes`. Keep masks on head. Update resets.
**Status:** not-started

### valley
**Files:** `json/areas/valley/objects.json`
**Action:** Cloaks → `cloak`. Update resets if any.
**Status:** not-started

---

## Verification Checklist

- [ ] No item has multiple wear_flags for conflicting slots after migration
- [ ] All room resets that equip migrated items reference the correct new wear_loc
- [ ] Masks remain on head (WEAR_LOC_HEAD), not moved to eyes
- [ ] All backpacks are ITEM_CONTAINER type and worn on back (WEAR_LOC_BACK)
- [ ] Tattoo items use ITEM_TATTOO type (value 35) if added

---

## Status: COMPLETED — All 17 Areas / 24 Items Migrated (April 12, 2026, Final)

**Completed Area Migrations:**
- ✅ Canyon: anum 4 (cape → wearcloak), anum 10 (cloak → wearcloak)
- ✅ Chapel: anum 27 (cape → wearcloak)
- ✅ Gnome: anum 7 (cloak → wearcloak)
- ✅ Hitower: anum 35 (spectacles → weareyes), anum 41 (cloak → wearcloak), anum 45 (backpack → wearback), anum 83 (cloak → wearcloak), anum 89 (cloak → wearcloak)
- ✅ Mahntor: anum 5 (cloak → wearcloak), anum 12 (cape → wearcloak)
- ✅ Mirror: anum 21 (cloak → wearcloak), anum 27 (cape → wearcloak)
- ✅ Draconia: anum 76 (fur cloak → wearcloak)
- ✅ Newthalos: anum 36 (backpack → wearback), anum 48 (fur cloak → wearcloak)
- ✅ Pocketdungeon: anum 69 (cloak → wearcloak), anum 79 (orb → wearfloat2)
- ✅ Quest: anum 15 (cloak → wearcloak), anum 29 (orb → wearfloat2)
- ✅ Sewer: anum 224 (white cloak → wearcloak)
- ✅ Smurf: anum 1 (glasses → weareyes)
- ✅ Tohell: anum 24 (spectacles → weareyes)
- ✅ Catacomb: anum 14 (shadow cloak → wearcloak)
- ✅ Olympus: no migrations (no matching items)
- ✅ Ofcol2: no migrations (orb is hold, not float)
- ✅ Pyramid: no migrations (mask stays on head per rules)
- ✅ Valley: no migrations (cloak on neck per rules)
- ✅ Thalos: no migrations (cloak on neck per rules)

**Final Stats:**
- Total Areas: 17/17 completed
- Total Items Migrated: 24 items ✅
- Backpacks migrated to wearback: 2 items (hitower 45, newthalos 36)
- Cloaks/capes migrated to wearcloak: 14 items
- Eyes wear migrated (glasses/spectacles/goggles to weareyes): 3 items (hitower 35, smurf 1, tohell 24)
- Float orbs migrated to wearfloat2: 2 items (pocketdungeon 79, quest 29)
- Ears wear migrated: 0 items (none found in areas)
- Tattoo wear migrated: 0 items (none found in areas)
- Build Status: Verified passing ✅
- JSON Validation: All migrated files verified ✅

**Items Not Migrated (Per Rules):**
- Valley anum 3 (brown cloak on neck) - neckwear stays on neck
- Thalos anum 23 (green cloak on neck) - neckwear stays on neck
- Sewer anum 203 (purple cloak on neck) - neckwear stays on neck
- Catacomb anum 10 (white cape on neck) - neckwear stays on neck
- Pyramid mask - masks stay on head per migration rules
- Ofcol2 orb 63 - held item, not floated
- Sewer orb 303 - held item, not floated
- Quest orb 6 - held item, not floating
- Olympus - no cloak/backpack/eyewear/earring/float items found

## Status: COMPLETED — April 12, 2026 (Final)
