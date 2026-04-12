# Area Migration Mini-Plan — Wear Slot Revamp

## Overview
Migrate area JSON objects and room resets to use the new wear slots. Items in each area are reviewed and moved to their appropriate slots based on their descriptions and purpose.

## Migration Rules
- **Cloaks/capes/shawls**: Move from existing wear slots to WEAR_LOC_CLOAK (21)
- **Backpacks/containers**: Move to WEAR_LOC_BACK (20), ensure item_type = ITEM_CONTAINER
- **Glasses/spectacles/monocles/patches/goggles**: Move to WEAR_LOC_EYES (22)
- **Earrings/ear cuffs**: Move to WEAR_LOC_EARS (23)
- **Orbs/globes/discs**: Move from existing wear slots to WEAR_LOC_FLOAT_2 (24)
- **Tattoo items**: Change item_type to ITEM_TATTOO (35), set wear_loc to WEAR_LOC_TATTOO (25)
- **Masks/visors on head**: Stay on WEAR_LOC_HEAD (6)
- **Necklaces/amulets**: Stay on WEAR_LOC_NECK (3 or 4)

## Files to Migrate

### Areas requiring object & room migration:
- [  ] `json/areas/canyon/objects.json` — Move cloak items to CLOAK slot
- [  ] `json/areas/canyon/rooms.json` — Update room resets for cloak items
- [  ] `json/areas/catacomb/objects.json` — Move cloak & float items
- [  ] `json/areas/catacomb/rooms.json` — Update room resets
- [  ] `json/areas/chapel/objects.json` — Move outerwear items
- [  ] `json/areas/chapel/rooms.json` — Update room resets
- [  ] `json/areas/draconia/objects.json` — Move outerwear items
- [  ] `json/areas/draconia/rooms.json` — Update room resets
- [  ] `json/areas/gnome/objects.json` — Move cloak items
- [  ] `json/areas/gnome/rooms.json` — Update room resets
- [  ] `json/areas/hitower/objects.json` — Move spectacles, cloak, backpack, float
- [  ] `json/areas/hitower/rooms.json` — Update room resets
- [  ] `json/areas/mahntor/objects.json` — Move cloak, eyewear, earrings, backpack, float
- [  ] `json/areas/mahntor/rooms.json` — Update room resets
- [  ] `json/areas/mirror/objects.json` — Move cloak & float items
- [  ] `json/areas/mirror/rooms.json` — Update room resets
- [  ] `json/areas/newthalos/objects.json` — Move backpack-style containers
- [  ] `json/areas/newthalos/rooms.json` — Update room resets if any
- [  ] `json/areas/ofcol2/objects.json` — Move eyewear, cloak, float items
- [  ] `json/areas/ofcol2/rooms.json` — Update room resets
- [  ] `json/areas/olympus/objects.json` — Review neckwear (stay put), move other outerwear
- [  ] `json/areas/olympus/rooms.json` — Update room resets
- [  ] `json/areas/pocketdungeon/objects.json` — Move cloak & float items
- [  ] `json/areas/pocketdungeon/rooms.json` — Update room resets
- [  ] `json/areas/pyramid/objects.json` — Review eyewear/masks
- [  ] `json/areas/pyramid/rooms.json` — Update room resets if needed
- [  ] `json/areas/quest/objects.json` — Move cloak, review neck & headwear
- [  ] `json/areas/quest/rooms.json` — Update room resets
- [  ] `json/areas/sewer/objects.json` — Move backpack & float items
- [  ] `json/areas/sewer/rooms.json` — Update room resets
- [  ] `json/areas/smurf/objects.json` — Move glasses to EYES slot
- [  ] `json/areas/smurf/rooms.json` — Update room resets if any
- [  ] `json/areas/thalos/objects.json` — Review float items
- [  ] `json/areas/thalos/rooms.json` — Update room resets if needed
- [  ] `json/areas/tohell/objects.json` — Review spectacles/eyewear
- [  ] `json/areas/tohell/rooms.json` — Update room resets
- [  ] `json/areas/valley/objects.json` — Move cloak items
- [  ] `json/areas/valley/rooms.json` — Update room resets if any

## Migration Procedure
1. For each area's `objects.json`:
   - Search for items matching the migration rules (cloaks, backpacks, eyewear, etc.)
   - Update `wear_flags` if the item has a wear flag  
   - Keep `wear_loc` field ONLY in room reset entries (not in object definitions)
   
2. For each area's `rooms.json`:
   - Search for room reset entries that equip items
   - Update `wear_loc` values if the item moved to a new slot
   - Example: If a cloak was on HEAD (6), change to CLOAK (21)

## Verification Checklist
- [ ] No unwanted items changed slots
- [ ] Masks/visors remain on HEAD
- [ ] Necklaces remain on NECK
- [ ] Containers on BACK use ITEM_CONTAINER type
- [ ] Building the server loads area JSON cleanly with no errors

## Dependencies
- Requires Phase 1 C code (new wear slot constants)
- Requires Phase 2 JSON config migration (wear_locs.json updated)

## Status
**Draft** — Ready to implement after Phase 2 completion and user approval of the area-by-area checklist.
