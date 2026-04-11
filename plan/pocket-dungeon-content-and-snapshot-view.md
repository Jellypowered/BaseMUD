# ✅ PLAN COMPLETE — Pocket Dungeon Content and Snapshot View

## Findings

- The `[Room 20040]` suffix is a server-side visibility feature from `src/act_info.c:do_look_room()`. If a player-level character is seeing it, that is a real bug and it needs an explicit pocket-dungeon debug toggle instead of relying on implicit builder/immortal checks.
- `room_descs` are not optional content for the shipped pocket dungeon pack. The generator should use authored room descriptions as the normal path, with only a legacy fallback for old or malformed seed data.
- The current pocket dungeon generator is still bland because room names and descriptions are mostly static: `src/pocket_dungeon.c` always writes the same room description and does not yet use the richer seed description pool as the primary source.
- Snapshot JSON already carries live rooms, mobs, and objects, but it does not yet expose room descriptions or mobprog trigger summaries for the webapp.

## Plan

- `json/config/pocket_dungeon_seeds.json` — author `room_descs` for every seed/theme so each dungeon has distinct flavor text instead of repeating the same generic sentence.
- `src/json_tblr.c` — parse `room_descs` entries from pocket dungeon seed JSON and keep the loader tolerant of older files that may not have them yet.
- `src/recycle.c` — free `room_descs` strings when a seed is disposed so the authored content path stays memory-safe.
- `MUDEditor/web/client/src/pages/PocketDungeonPage.tsx` — add seed-editor controls for room descriptions so the flavored content can be maintained in the webapp instead of hand-editing JSON only.
- `src/pocket_dungeon.c` — capitalize generated room names, use authored room descriptions as the primary source, and export room description plus per-mob mobprog summaries nested under each room's live mob list in pocket dungeon snapshot JSON.
- `MUDEditor/web/shared/types/index.ts` — extend pocket dungeon seed and snapshot types with room description and per-mob mobprog metadata fields.
- `MUDEditor/web/client/src/pages/PocketDungeonPage.tsx` — render room descriptions and per-mob mobprog metadata inline in the Instances viewer so live snapshot data is visible in the webapp.
- `src/structs.h`, `src/globals.c`, `src/json_tblr.c`, `json/config/pocket_dungeon_config.json`, `MUDEditor/web/shared/types/index.ts`, `MUDEditor/web/client/src/pages/PocketDungeonPage.tsx`, `src/act_info.c` — add a pocket-dungeon debug visibility toggle for the room-vnum suffix and wire it into the config UI so the shipped system can turn the suffix off cleanly.

## Risks

- Capitalizing generated room names changes existing output text slightly, but only for presentation.
- The room-vnum toggle must default to off in shipped configs, or the visibility bug remains masked rather than fixed.
- `room_descs` loading must stay backward compatible with old seed files while still making the authored content path the standard for new data.
- The seed editor needs to stay schema-safe so room description edits cannot drift away from the JSON contract.

## Status: COMPLETED

- Date: April 11, 2026
