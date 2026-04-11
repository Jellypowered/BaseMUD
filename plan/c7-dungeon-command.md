# Plan: C7 Dungeon Setup Command (do_dungeon)

## Overview
Implement `dungeon` command for players to enter pocket dungeons with theme/difficulty selection + gold cost.

## Files to Modify

### src/interp.c
- Add command entry: `{"dungeon", do_dungeon, POS_STANDING}`
- Make it available to all players

### src/act_move.c (or create act_dungeon.c)
- Implement `do_dungeon()` function with:
  - List available themes
  - Parse theme selection
  - Calculate gold cost
  - Check player gold/inventory space
  - Validate setup mode setting
  - Generate instance with pd_generate_instance()
  - Teleport player to entry room

## Menu Flow
```
> dungeon
Available themes:
  [1] Catacombs (dangerous underground)
  [2] Dragon's Lair (fire treasure)
  [3] Ancient Ruins (magical secrets)
  ...

Type: dungeon <theme_name>
```

## Implementation Details

### Gold Cost Calculation
```c
int get_dungeon_cost(int difficulty_level) {
    /* Base cost: 100 gold per level */
    /* testing_mode bypass: if pd_config.testing_mode, return 0 */
    if (pd_config.testing_mode)
        return 0;
    return difficulty_level * 100;
}
```

### Command Validation
- Check if player in combat
- Check if player has sufficient gold
- Check if player inventory not full
- Check if setup mode enabled (pd_config.setup_mode_enabled)

### Teleport to Instance
```c
po_generate_instance(members, count, theme);
char_from_room(ch);
char_to_room(ch, room_get_index(inst->entry_vnum));
send_to_char("You enter the pocket dungeon...\n\r", ch);
```

## Risks
- Single-player vs group: allow forming group first, or solo entry?
- Instance already running: check if player in existing instance
- No return portal placed yet: need to ensure entry room has portal back

## Status: NOT STARTED
