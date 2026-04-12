## Plan: Wear Slot Revamp

Treat `json/areas/**` as the runtime source for item and reset changes, not the legacy `area/*.are` files. Append new wear slots and wear flags without renumbering the existing ones, add the tattoo item type, then sync the MUDEditor slot pickers, item-type dropdowns, flag catalogs, and help text. The back slot will be a real backpack mechanic: the container itself still weighs something, but items inside a back-worn container will be exempt from carry number and carry weight while it is worn.

**Steps**
1. Lock the server wear-slot model first. Append new wear locations for back, cloak, eyes, ears, second float, and tattoo instead of reshuffling existing IDs, then update the wear-slot table and the wear-flag table so the new names are recognized everywhere the table is read. As part of that pass, preserve the existing carry-cap baseline by decoupling `char_get_max_carry_count()` from `WEAR_LOC_MAX` so adding slots does not silently increase every character's inventory cap.
2. Add the back-container exemption logic. Teach equip, unequip, and object-in-container accounting to ignore the contents of a container only while that container is worn on the back. Keep the backpack object's own weight/counting behavior intact, and restrict the back slot to container-style items so the slot stays immersive.
3. Migrate the runtime JSON data. Update `json/config/wear_locs.json` and `json_std/config_unsupported/wear_locs.json` with the new slots, and update `json/meta/flags/wear_flags.json` plus `json_std/meta/flags/wear_flags.json` with the matching wear flags. Also add the tattoo item type in `json/meta/types/item_types.json` and `json_std/meta/types/item_types.json`. Then migrate the affected `json/areas/**/objects.json` and `json/areas/**/rooms.json` entries so cloaks/capes/shawls move to cloak, backpack-style containers move to back, glasses/spectacles/monocles/patches/goggles move to eyes, earrings move to ears, obvious orb/globe/disc wearables move to the second float slot, and tattoo items use the new tattoo item type and slot. Leave masks and helm-visors on head, and keep neckwear on neck.
4. Sync MUDEditor so builders see the same model. Update the wear-location arrays and slot grid in `MUDEditor/web/client/src/components/editors/MobileEquipmentModal.tsx` and `MUDEditor/web/client/src/components/editors/RoomEditor.tsx`, and refresh the wear-flag fallback catalogs in `MUDEditor/web/client/src/components/FlagsField.tsx` and `MUDEditor/web/client/src/hooks/useFlagsConfig.ts`. Keep the slot labels and reset dropdown values aligned with the server JSON names.
5. Refresh player-facing help. Update `json/help/help.json` to describe the expanded equipment layout and the backpack behavior that matters to players. Update `json/help/olc.json` if the reset help should call out the new wear-location names or clarify that `? WEAR-LOC` reflects the expanded table.
6. Verify the whole chain. Build the BaseMUD side with the workspace build task, typecheck the MUDEditor packages, and sanity-check that the equipment display, room reset wear-location dropdown, and wear-flag picker all expose the new names while existing saved wear_loc numbers still load unchanged.

**Relevant files**
- `src/types.h` — append the new wear-location constants and raise `WEAR_LOC_MAX`.
- `src/types.c` — add the new item type entry for tattoo.
- `src/flags.h` — add the new `ITEM_WEAR_*` bits.
- `src/flags.c` — add the new wear flag names to the parser table.
- `src/tables.c` — extend `wear_loc_table` with the new slot definitions and messages.
- `src/chars.c` — update carry-count logic, slot iteration, equipment display, and the back-container exemption hooks.
- `src/items.c` — enforce container-only behavior for the back slot if needed and add tattoo to the no-values item map.
- `src/act_info.c` — redraw the equipment list so the new slots are visible and readable.
- `json/config/wear_locs.json` — runtime wear-location JSON updated with the new slot entries.
- `json_std/config_unsupported/wear_locs.json` — reference copy kept in sync with the runtime wear locations.
- `json/meta/flags/wear_flags.json` — runtime wear-flag metadata updated with the new slot flags.
- `json_std/meta/flags/wear_flags.json` — reference copy kept in sync with the runtime wear flags.
- `json/meta/types/item_types.json` — runtime item-type metadata updated with tattoo.
- `json_std/meta/types/item_types.json` — reference copy kept in sync with the runtime item types.
- `json/help/help.json` — update equipment and wear help text.
- `json/help/olc.json` — update reset/help wording if needed for the new location names.
- `json/areas/canyon/objects.json`, `json/areas/canyon/rooms.json` — cloak and any reset migrations from the inventory sweep.
- `json/areas/catacomb/objects.json`, `json/areas/catacomb/rooms.json` — cloak and float/reset migrations from the inventory sweep.
- `json/areas/chapel/objects.json`, `json/areas/chapel/rooms.json` — outerwear and any reset migrations from the inventory sweep.
- `json/areas/draconia/objects.json`, `json/areas/draconia/rooms.json` — outerwear and any reset migrations from the inventory sweep.
- `json/areas/gnome/objects.json`, `json/areas/gnome/rooms.json` — cloak and any reset migrations from the inventory sweep.
- `json/areas/hitower/objects.json`, `json/areas/hitower/rooms.json` — backpack, spectacles, cloak, and float migrations from the inventory sweep.
- `json/areas/mahntor/objects.json`, `json/areas/mahntor/rooms.json` — cloak, eyewear, earrings, backpack, and float migrations from the inventory sweep.
- `json/areas/mirror/objects.json`, `json/areas/mirror/rooms.json` — cloak and float migrations from the inventory sweep.
- `json/areas/newthalos/objects.json` — backpack-style container migrations from the inventory sweep.
- `json/areas/ofcol2/objects.json`, `json/areas/ofcol2/rooms.json` — eyewear, cloak, and float migrations from the inventory sweep.
- `json/areas/olympus/objects.json` — neckwear stays put; only move items that clearly belong to a new slot.
- `json/areas/pocketdungeon/objects.json`, `json/areas/pocketdungeon/rooms.json` — cloak and float migrations from the inventory sweep.
- `json/areas/pyramid/objects.json` — eyewear or mask review from the inventory sweep.
- `json/areas/quest/objects.json` — cloak, neck, and headwear review from the inventory sweep.
- `json/areas/sewer/objects.json`, `json/areas/sewer/rooms.json` — backpack and float migrations from the inventory sweep.
- `json/areas/smurf/objects.json` — glasses to eyes from the inventory sweep.
- `json/areas/thalos/objects.json` — float migration review from the inventory sweep.
- `json/areas/tohell/objects.json`, `json/areas/tohell/rooms.json` — spectacles/eyewear review from the inventory sweep.
- `json/areas/valley/objects.json` — cloak migration from the inventory sweep.
- `MUDEditor/web/client/src/components/editors/MobileEquipmentModal.tsx` — wearable slot UI grid and labels.
- `MUDEditor/web/client/src/components/editors/RoomEditor.tsx` — reset wear-location dropdown.
- `MUDEditor/web/client/src/components/FlagsField.tsx` — wear-flag fallback list.
- `MUDEditor/web/client/src/hooks/useFlagsConfig.ts` — wear-flag fallback config.
- `MUDEditor/web/client/src/components/editors/ObjectEditor.tsx` — item type dropdown and field handling.
- `MUDEditor/web/client/src/components/wizards/NewObjectWizard.tsx` — new-object item type selection.
- `MUDEditor/web/client/src/components/editors/MobileEditor.tsx` — shopkeeper item type list.

**Verification**
1. Run the BaseMUD build task and confirm the wear-slot JSON loads cleanly with the new slots and flags.
2. Typecheck MUDEditor from the `web/` root and confirm the new slot and item-type names flow through the equipment modal, room reset editor, object editor, and new-object wizard.
3. Spot-check a few migrated JSON area files to verify cloaks, backpacks, eyewear, earrings, float items, and tattoos landed in the intended slots, and that no masks or neck items were accidentally moved.
4. Confirm the carry-count display still matches the pre-revamp baseline except for the intentional back-container exemption.

**Decisions**
- Do not edit the legacy `.are` files; the runtime path for this revamp is the JSON data.
- Preserve existing wear_loc numeric IDs and append the new ones so saved equipment and resets stay compatible.
- Preserve the current carry/weight baseline for non-back equipment; only contents of items worn on the back should be exempt from carry limits.
- Use a single back slot for container-style backpacks; only the contents become exempt while worn.
- Add an eyes slot and an ears slot; keep masks on head and amulets/necklaces on neck.
- Treat tattoo as a new item type that uses the tattoo wear slot and carries stat-boosting behavior.
- Add a second float slot for orb/globe/disc-style items that are clearly meant to hover rather than be held.

**Status**: DRAFT
