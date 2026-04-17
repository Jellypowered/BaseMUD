# Explore Tracking System Integration

## Files to Modify

1. **src/structs.h** (PC_T structure)
   - Add `char *explored` field to track visited rooms (4096 bytes for 32768 room vnums using bitpacking)

2. **src/recycle.c**
   - Initialize `explored` in `pcdata_init` (allocate 4096 bytes)
   - Free `explored` in `pcdata_dispose`

3. **src/save.c**
   - Add RLE encoding/decoding functions for explored buffer
   - Load in `load_char_obj` (initialize to all zeros for legacy chars)
   - Save to pfile in `fwrite_char` using RLE
   - Load from pfile in `fread_char` using RLE (case 'E')

4. **src/handler.c**
   - Add `char_to_room` hook to track room visits (set bit when PC enters)

5. **src/act_info.c**
   - Add integration into `do_score` to display exploration stats

6. **src/act_info.c** (new command)
   - Implement `do_explored` command as standalone stats display

7. **src/act_info.h**
   - Declare `do_explored`

8. **src/interp.c**
   - Add command table entry for "explored"

9. **json/help/help.json**
   - Add help entry for EXPLORED command

## New Files to Create

1. **src/explore.c** (adaptation of snippet)
   - Helper functions: `explore_bitcount()`, `explore_roomcount()`,`explore_getbit()`, `explore_setbit()`
   - RLE functions: `explore_fwrite_rle()`, `explore_fread_rle()`

2. **src/explore.h**
   - Declare helper functions

## Implementation Notes

- **Bitpacking**: 8 bits per byte, dynamic buffer based on highest room vnum
- **RLE Compression**: Simple run-length encoding for save file space (stores size prefix)
- **Hook point**: `char_to_room()` in handler.c sets exploration bit when PC enters room
- **Auto-track**: Automatic on room entry, PCs only (not NPCs)
- **Display**: Pretty format matching do_score style with colors and alignment
- **Colors**: Use `{Y` for numbers, `{x` for reset

## Status: IN PROGRESS
