# ✅ PLAN COMPLETE — MUDEditor UI Cohesion Audit

**Status:** COMPLETED

## Goal
Bring the MUDEditor client UI into a single visual system across all `web/client/src/**/*.tsx` files, using `ClassesPage` as the reference pattern for page chrome, control density, and save-state presentation.

## Findings
- `ClassesPage` already represents the cleanest config-page baseline: consistent left-list/right-editor layout, compact headers, shared input styling, and predictable action placement.
- `PocketDungeonPage` is the largest outlier: it mixes `btn` / `checkbox` variants with the local `input` style, uses hard-coded success colors, and has different spacing and header treatment between tabs.
- `AppShell` and several editor components still use hard-coded colors and bespoke toggle / status styling that does not fully inherit the theme tokens.
- A number of config pages still rely on page-local select / checkbox / status patterns instead of shared bubble pickers, checkbox styling, and the existing picker / flag components.
- Theme support is mostly present through tokens, but several TSX files still render hard-coded green / amber / black surfaces that need to be normalized for light and dark themes.

## Proposed Changes
1. Standardize page chrome across config pages on the `ClassesPage` layout model: compact list rail, consistent editor header, compact save/delete action cluster, and a shared empty-state presentation.
2. Normalize controls to shared primitives where possible:
- use bubble / pill pickers for finite option sets
- use checkbox styling consistently for toggles
- keep text / number / textarea fields on the same input surface and spacing scale
3. Replace hard-coded success / warning / preview colors with theme-safe semantic tokens so saved states, warnings, and helper previews remain readable in both light and dark mode.
4. Tighten shared components first, then update each outlier page so page-specific fixes inherit the new baseline instead of duplicating styling.

## Files Expected to Change
- `web/client/src/index.css`
- `web/client/src/components/AppShell.tsx`
- `web/client/src/components/FlagsField.tsx`
- `web/client/src/components/SinglePickerField.tsx`
- `web/client/src/components/SortSelect.tsx`
- `web/client/src/components/RawJsonModal.tsx`
- `web/client/src/components/Tooltip.tsx`
- `web/client/src/components/AreaMiniMap.tsx`
- `web/client/src/components/editors/AreaMetaEditor.tsx`
- `web/client/src/components/editors/AreImportModal.tsx`
- `web/client/src/components/editors/MobileEditor.tsx`
- `web/client/src/components/editors/MobileEquipmentModal.tsx`
- `web/client/src/components/editors/ObjectEditor.tsx`
- `web/client/src/components/editors/RoomEditor.tsx`
- `web/client/src/components/wizards/NewAreaWizard.tsx`
- `web/client/src/components/wizards/NewMobileWizard.tsx`
- `web/client/src/components/wizards/NewObjectWizard.tsx`
- `web/client/src/components/wizards/NewRoomWizard.tsx`
- `web/client/src/pages/AreaEditorPage.tsx`
- `web/client/src/pages/AreaListPage.tsx`
- `web/client/src/pages/ClassesPage.tsx`
- `web/client/src/pages/ClansPage.tsx`
- `web/client/src/pages/GreetingsPage.tsx`
- `web/client/src/pages/HelpPage.tsx`
- `web/client/src/pages/LandingPage.tsx`
- `web/client/src/pages/LiquidsPage.tsx`
- `web/client/src/pages/MaterialsPage.tsx`
- `web/client/src/pages/PCRacesPage.tsx`
- `web/client/src/pages/PocketDungeonPage.tsx`
- `web/client/src/pages/PortalsPage.tsx`
- `web/client/src/pages/QuestPage.tsx`
- `web/client/src/pages/RacesPage.tsx`
- `web/client/src/pages/SkillGroupsPage.tsx`
- `web/client/src/pages/SkillsPage.tsx`
- `web/client/src/pages/SocialsPage.tsx`

## Risks
- This touches many TSX files, so the biggest risk is introducing regressions in saved-state flows, picker behavior, or read-only role gating.
- Replacing local select / checkbox implementations with shared primitives may require small prop or markup adjustments in a few editor components.
- Theme changes should stay token-based; if any component still needs a hard-coded literal, it should be justified by contrast or preview behavior.

## Verification
- Run the MUDEditor TypeScript checks for shared, client, and server where relevant.
- Spot-check `ClassesPage`, `PocketDungeonPage`, `AppShell`, `MobileEditor`, and `ObjectEditor` in both light and dark theme modes.
- Confirm the config pages still preserve existing save / delete / dirty-state behavior after the visual cleanup.

## Status: COMPLETED
