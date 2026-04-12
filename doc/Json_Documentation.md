# BaseMUD JSON Editor Reference

This document covers the complete JSON schema for BaseMUD area, entity, config, and help data. It is intended for developers building online editors that read and write these files.

## Table of Contents

1. [File Layout](#file-layout)
2. [Common Conventions](#common-conventions)
3. [Entity Schemas](#entity-schemas)
   - [area](#area)
   - [room](#room)
   - [exit (within doors)](#exit-within-doors)
   - [reset (within resets)](#reset-within-resets)
   - [mobile](#mobile)
   - [shop (nested in mobile)](#shop-nested-in-mobile)
   - [object](#object)
   - [Object values by item_type](#object-values-by-item_type)
   - [affect (within affects)](#affect-within-affects)
   - [extra_description](#extra_description)
   - [social](#social)
   - [portal (config)](#portal-config)
   - [help_area](#help_area)
   - [help (within help_area)](#help-within-help_area)
4. [Valid Enum Values](#valid-enum-values)
5. [Flag Strings](#flag-strings)
6. [Business Rules and Constraints](#business-rules-and-constraints)
7. [Computed Fields](#computed-fields-never-write-these)
8. [Config Tables: What You Can Edit Live](#config-tables-what-you-can-edit-live)

---

## File Layout

```
json/
  areas/
    <area_name>/
      area.json        -- One area definition
      rooms.json       -- All rooms in the area
      mobiles.json     -- All mobile prototypes
      objects.json     -- All object prototypes
  config/
    socials.json       -- All emote/social commands (world-global)
    portals.json       -- Inter-area portal link definitions
    attacks.json       -- Attack type names (read-only config)
    items.json         -- Item type names (read-only config)
    races.json         -- Race definitions (read-only config)
    skills.json        -- Skill/spell definitions (read-only config)
    weapons.json       -- Weapon type names (read-only config)
    liquids.json       -- Liquid type names (read-only config)
    materials.json     -- Material names (read-only config)
    greetings.json     -- Login greeting messages (live-editable)
    <others>           -- Additional config tables (see editing guide)
  help/
    <name>.json        -- Help page collections
```

Every JSON file is a **top-level array** of single-key wrapper objects. The key matches the entity type:

```json
[
  { "room": { ... } },
  { "room": { ... } }
]
```

This format applies even to `area.json`, which contains only one entry.

---

## Common Conventions

### Required vs Optional Fields

Fields marked **(opt)** are optional; all others are **required**. The server logs a warning and skips the entity if a required field is absent.

### Anum (Area-Local Numbers)

Rooms, mobiles, and objects are identified within their area by an integer `anum`. The server resolves these to global vnums at boot. **Always use anums within area files � never global vnums** for cross-references within the same area.

Cross-area references (e.g. `key` vnum on a container or portal object) use the global vnum directly.

### Dice Strings

Dice notation uses the format `"XdY+Z"` where X = number of dice, Y = sides per die, Z = fixed modifier. Examples: `"1d4+0"`, `"2d6+4"`, `"1d1+999"` (effectively a constant 1000).

### Flag Strings

Multi-flag fields are space-separated strings:

```json
"room_flags": "no_mob indoors law",
"extra_flags": "hum bless"
```

Omit the field entirely for zero flags (do not include an empty string).

### Color Codes

Description strings may contain ROM-style inline color codes: `{r` (red), `{b` (blue), `{x` (reset), etc. These are stored literally in the JSON.

### Social Substitution Codes

Social message strings use substitution placeholders: `$n` actor name, `$N` target name, `$m`/`$M` him/her (object), `$s`/`$S` his/her (possessive), `$e`/`$E` he/she, `$p` object name, `$P` secondary object name.

---

## Entity Schemas

### area

**File:** `json/areas/<area_name>/area.json` | **Wrapping key:** `"area"`

```json
{
	"area": {
		"name": "midgaard",
		"filename": "midgaard",
		"title": "Midgaard",
		"credits": "Original Midgaard by Merc, enhanced by ROM.",
		"min_vnum": 3000,
		"max_vnum": 3299,
		"builders": "None",
		"security": 9,
		"low_range": 5,
		"high_range": 95
	}
}
```

| Field        | Type    | Req   | Notes                                                              |
| ------------ | ------- | ----- | ------------------------------------------------------------------ |
| `name`       | string  | yes   | Unique identifier. Must match the folder name under `json/areas/`. |
| `filename`   | string  | yes   | Base filename used when saving (no extension).                     |
| `title`      | string  | yes   | Human-readable title shown to players.                             |
| `credits`    | string  | yes   | Credit line displayed in area list.                                |
| `min_vnum`   | integer | yes   | Lowest global vnum allocated to this area.                         |
| `max_vnum`   | integer | yes   | Highest global vnum. Must be greater than `min_vnum`.              |
| `builders`   | string  | yes   | Space-separated builder names, or `"None"`.                        |
| `security`   | integer | yes   | OLC security level required to edit. Range: 0�9.                   |
| `low_range`  | integer | (opt) | Recommended minimum player level. Default: 0.                      |
| `high_range` | integer | (opt) | Recommended maximum player level. Default: 0 (any).                |
| `hidden`     | boolean | (opt) | When `true`, the area is excluded from the `areas` list shown to players. Useful for internal/system areas (e.g. quest token storage). Default: `false`. |

---

### room

**File:** `json/areas/<area_name>/rooms.json` | **Wrapping key:** `"room"`

```json
{
  "room": {
    "area":        "midgaard",
    "anum":        1,
    "name":        "The {bTemple{x Of Mota",
    "description": "You are in the southern end of the temple hall...",
    "sector_type": "inside",
    "room_flags":  "no_mob indoors law",
    "portal":      "midgaard-room-1",
    "heal_rate":   150,
    "mana_rate":   120,
    "owner":       "Admin",
    "clan":        "rom",
    "doors": [ ... ],
    "extra_description": [ ... ],
    "resets": [ ... ]
  }
}
```

| Field               | Type           | Req   | Notes                                                                        |
| ------------------- | -------------- | ----- | ---------------------------------------------------------------------------- |
| `area`              | string         | yes   | Must match the area `name`.                                                  |
| `anum`              | integer        | yes   | Area-local number, unique within the area.                                   |
| `name`              | string         | yes   | Short room name shown in the room title.                                     |
| `description`       | string         | yes   | Long room description.                                                       |
| `sector_type`       | string (enum)  | yes   | See [sector_type](#sector_type).                                             |
| `room_flags`        | string (flags) | (opt) | See [room_flags](#room_flags).                                               |
| `heal_rate`         | integer        | (opt) | HP regen rate multiplier. Default: 100.                                      |
| `mana_rate`         | integer        | (opt) | Mana regen rate multiplier. Default: 100.                                    |
| `owner`             | string         | (opt) | Player name owning this room (personal rooms).                               |
| `clan`              | string (enum)  | (opt) | Clan affiliation. See [clans](#clans).                                       |
| `portal`            | string         | (opt) | Named portal exit point on this room. See [Portal Naming](#portal-naming).   |
| `doors`             | array          | (opt) | Exits. See [exit](#exit-within-doors).                                       |
| `extra_description` | array          | (opt) | Keyword-triggered descriptions. See [extra_description](#extra_description). |
| `resets`            | array          | (opt) | Spawn instructions. See [reset](#reset-within-resets).                       |

---

### exit (within `doors`)

```json
{ "dir": "north", "to": 54, "description": "You see a great hall." }
```

```json
{ "dir": "up", "description": "You see Mud School.", "portal": "midgaard-up-4" }
```

```json
{
	"dir": "east",
	"to": 12,
	"keyword": "iron door",
	"exit_flags": "door closed locked",
	"key": 3050
}
```

| Field         | Type           | Req   | Notes                                                                              |
| ------------- | -------------- | ----- | ---------------------------------------------------------------------------------- |
| `dir`         | string (enum)  | yes   | `"north"`, `"south"`, `"east"`, `"west"`, `"up"`, `"down"`.                        |
| `to`          | integer (anum) | (opt) | Anum of the destination room within the same area. Omit for portal exits.          |
| `description` | string         | (opt) | Text shown when player looks in this direction.                                    |
| `keyword`     | string         | (opt) | Space-separated nouns identifying the door (used with open/close/lock).            |
| `exit_flags`  | string (flags) | (opt) | See [exit_flags](#exit_flags).                                                     |
| `key`         | integer (vnum) | (opt) | Global vnum of the key item. Use `0` for no key.                                   |
| `portal`      | string         | (opt) | Named portal exit point for cross-area links. See [Portal Naming](#portal-naming). |

An exit provides a destination via either `to` (same-area anum) or `portal` (cross-area). Exits with neither are rendered as walls in the description but do not allow passage.

---

### reset (within `resets`)

Resets populate rooms with mobiles and objects each time the area resets. Each reset object has a `command` string and a `values` object:

```json
{ "command": "<type>", "values": { ... } }
```

**Order is significant** � see [Reset Ownership](#reset-ownership-critical-for-editors).

#### `mobile` � spawn a mob in this room

```json
{
	"command": "mobile",
	"values": { "mob": 11, "global_limit": 1, "room_limit": 1 }
}
```

| Field          | Type           | Notes                                       |
| -------------- | -------------- | ------------------------------------------- |
| `mob`          | integer (anum) | Anum of the mobile prototype to spawn.      |
| `global_limit` | integer        | Max instances world-wide. `-1` = unlimited. |
| `room_limit`   | integer        | Max instances in this specific room.        |

#### `object` � place an object in this room

```json
{
	"command": "object",
	"values": { "obj": 32, "global_limit": 1, "room_limit": 0 }
}
```

| Field          | Type           | Notes                                       |
| -------------- | -------------- | ------------------------------------------- |
| `obj`          | integer (anum) | Anum of the object prototype.               |
| `global_limit` | integer        | Max instances world-wide. `-1` = unlimited. |
| `room_limit`   | integer        | Max instances in this room.                 |

#### `give` � put an object in the preceding mobile's inventory

```json
{
	"command": "give",
	"values": { "obj": 7, "global_limit": 1 }
}
```

| Field          | Type           | Notes                                       |
| -------------- | -------------- | ------------------------------------------- |
| `obj`          | integer (anum) | Anum of the object prototype.               |
| `global_limit` | integer        | Max instances world-wide. `-1` = unlimited. |

The item is given to the nearest preceding `mobile` reset in the room's reset list.

#### `equip` � equip an object on the preceding mobile

```json
{
	"command": "equip",
	"values": { "obj": 5, "wear_loc": "wielded", "global_limit": 1 }
}
```

| Field          | Type           | Notes                                         |
| -------------- | -------------- | --------------------------------------------- |
| `obj`          | integer (anum) | Anum of the object prototype.                 |
| `wear_loc`     | string (enum)  | Where to equip it. See [wear_loc](#wear_loc). |
| `global_limit` | integer        | Max instances world-wide. `-1` = unlimited.   |

The item is equipped on the nearest preceding `mobile` reset.

#### `put` � place an object inside a container object

```json
{
	"command": "put",
	"values": { "obj": 15, "into": 10, "global_limit": 5, "put_count": 3 }
}
```

| Field          | Type           | Notes                                       |
| -------------- | -------------- | ------------------------------------------- |
| `obj`          | integer (anum) | Anum of the object to place inside.         |
| `into`         | integer (anum) | Anum of the container object prototype.     |
| `global_limit` | integer        | Max instances world-wide. `-1` = unlimited. |
| `put_count`    | integer        | Number of `obj` copies to place.            |

The container must have been placed in the room by a preceding `object` reset.

#### `randomize` � shuffle exit directions in this room

```json
{
	"command": "randomize",
	"values": { "dir_count": 4 }
}
```

| Field       | Type    | Notes                                               |
| ----------- | ------- | --------------------------------------------------- |
| `dir_count` | integer | Number of exits to randomize (starting from north). |

---

### mobile

**File:** `json/areas/<area_name>/mobiles.json` | **Wrapping key:** `"mobile"`

```json
{
  "mobile": {
    "area":        "midgaard",
    "anum":        0,
    "name":        "wizard",
    "short_descr": "the wizard",
    "long_descr":  "A wizard walks around behind the counter, talking to himself.",
    "description": "The wizard looks old and senile, and yet very powerful.",
    "race":        "human",
    "alignment":   900,
    "level":       23,
    "hitroll":     0,
    "hit_dice":    "1d1+999",
    "mana_dice":   "1d1+999",
    "damage_dice": "1d8+32",
    "attack_type": "magic",
    "ac": { "pierce": -150, "bash": -150, "slash": -150, "magic": -150 },
    "sex":         "male",
    "wealth":      15000,
    "size":        "medium",
    "start_pos":   "stand",
    "default_pos": "stand",
    "material":    "flesh",
    "group":       0,
    "mob_flags":   "sentinel nopurge",
    "affected_by": "detect_invis",
    "offense":     "area_attack dodge",
    "immune":      "summon charm magic weapon",
    "resist":      "fire cold",
    "vulnerable":  "holy",
    "form":        "biped mammal sentient",
    "parts":       "head arms legs heart brains guts hands feet fingers ear eye",
    "spec_fun":    "spec_cast_mage",
    "shop": { ... }
  }
}
```

| Field               | Type           | Req   | Notes                                                                                         |
| ------------------- | -------------- | ----- | --------------------------------------------------------------------------------------------- |
| `area`              | string         | yes   | Must match area `name`.                                                                       |
| `anum`              | integer        | yes   | Area-local number, unique within the area.                                                    |
| `name`              | string         | yes   | Space-separated keyword list used to target the mob.                                          |
| `short_descr`       | string         | yes   | Short description (e.g. in room listings).                                                    |
| `long_descr`        | string         | yes   | Description line shown when mob is standing in room.                                          |
| `description`       | string         | yes   | Full description seen when player looks at mob.                                               |
| `race`              | string (enum)  | yes   | See [races](#races).                                                                          |
| `alignment`         | integer        | yes   | Range: -1000 (evil) to +1000 (good).                                                          |
| `level`             | integer        | yes   | Range: 1�60.                                                                                  |
| `hitroll`           | integer        | yes   | To-hit bonus.                                                                                 |
| `hit_dice`          | string (dice)  | yes   | HP formula, e.g. `"4d8+20"`.                                                                  |
| `mana_dice`         | string (dice)  | yes   | Mana formula.                                                                                 |
| `damage_dice`       | string (dice)  | yes   | Damage formula.                                                                               |
| `attack_type`       | string (enum)  | yes   | Default attack verb. See [attacks](#attacks).                                                 |
| `ac`                | object         | yes   | Armor class. Four integer keys: `pierce`, `bash`, `slash`, `magic`. Negative = harder to hit. |
| `wealth`            | integer        | yes   | Gold distributed at death.                                                                    |
| `size`              | string (enum)  | yes   | See [sizes](#sizes).                                                                          |
| `start_pos`         | string (enum)  | (opt) | Starting position. Default: `"stand"`. See [positions](#positions).                           |
| `default_pos`       | string (enum)  | (opt) | Position mob returns to. Default: `"stand"`.                                                  |
| `sex`               | string (enum)  | (opt) | Default: `"neutral"`. See [sexes](#sexes).                                                    |
| `material`          | string (enum)  | (opt) | Default: `"flesh"`. See [materials](#materials).                                              |
| `group`             | integer        | (opt) | Linked mob group ID. 0 = no group.                                                            |
| `spec_fun`          | string (enum)  | (opt) | Special function. See [spec_funs](#spec_funs).                                                |
| `mob_flags`         | string (flags) | (opt) | See [mob_flags](#mob_flags).                                                                  |
| `mob_flags_minus`   | string (flags) | (opt) | Flags to clear (race inheritance override).                                                   |
| `affected_by`       | string (flags) | (opt) | Active affect bits. See [affect_flags](#affect_flags).                                        |
| `affected_by_minus` | string (flags) | (opt) | Affect bits to clear.                                                                         |
| `offense`           | string (flags) | (opt) | Combat behaviors. See [off_flags](#off_flags).                                                |
| `offense_minus`     | string (flags) | (opt) | Offense flags to clear.                                                                       |
| `immune`            | string (flags) | (opt) | Damage immunities. See [res_flags](#res_flags).                                               |
| `immune_minus`      | string (flags) | (opt) | Immunities to clear.                                                                          |
| `resist`            | string (flags) | (opt) | Damage resistances. Same flag table as `immune`.                                              |
| `resist_minus`      | string (flags) | (opt) | Resistances to clear.                                                                         |
| `vulnerable`        | string (flags) | (opt) | Damage vulnerabilities. Same flag table as `immune`.                                          |
| `vulnerable_minus`  | string (flags) | (opt) | Vulnerabilities to clear.                                                                     |
| `form`              | string (flags) | (opt) | Physical form descriptors. See [form_flags](#form_flags).                                     |
| `form_minus`        | string (flags) | (opt) | Form flags to clear.                                                                          |
| `parts`             | string (flags) | (opt) | Body part flags. See [part_flags](#part_flags).                                               |
| `parts_minus`       | string (flags) | (opt) | Body parts to clear.                                                                          |
| `shop`              | object         | (opt) | Makes mob a shopkeeper. See [shop](#shop-nested-in-mobile).                                   |

---

### shop (nested in mobile)

```json
{
	"trades": ["scroll", "wand", "staff", "potion"],
	"profit_buy": 105,
	"profit_sell": 15,
	"open_hour": 0,
	"close_hour": 23
}
```

| Field         | Type                   | Req | Notes                                                                                                                                                                        |
| ------------- | ---------------------- | --- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `trades`      | array of string (enum) | yes | Item types the shop buys from players. Up to `MAX_TRADE` (currently 16) entries; excess entries are ignored. Empty array = buys nothing. Uses [item_type](#item_type) names. |
| `profit_buy`  | integer                | yes | Buy price as % of list. E.g. `105` = 5% markup.                                                                                                                              |
| `profit_sell` | integer                | yes | Sell (to shop) price as % of list. E.g. `15` = shop pays 15%.                                                                                                                |
| `open_hour`   | integer                | yes | Opening hour 0�23.                                                                                                                                                           |
| `close_hour`  | integer                | yes | Closing hour 0�23.                                                                                                                                                           |

What the shop **sells** is determined by `give` resets on the mob � items placed in the mob's inventory via `give` resets appear as shop stock. `equip` resets on a shopkeeper mob equip items on the mob personally (not for sale).

---

### object

**File:** `json/areas/<area_name>/objects.json` | **Wrapping key:** `"object"`

```json
{
  "object": {
    "area":        "midgaard",
    "anum":        5,
    "name":        "scimitar blade",
    "short_descr": "Hassan's scimitar",
    "description": "Hassan's scimitar lies upon the ground.",
    "material":    "adamantite",
    "item_type":   "weapon",
    "values": {
      "weapon_type": "sword",
      "dice_num":    4,
      "dice_size":   10,
      "attack_type": "cleave",
      "flags":       "vorpal twohands"
    },
    "level":      42,
    "weight":     450,
    "cost":       5600,
    "condition":  90,
    "extra_flags": "hum bless",
    "wear_flags":  "take wield",
    "extra_description": [ ... ],
    "affects": [ ... ]
  }
}
```

| Field               | Type           | Req   | Notes                                                                               |
| ------------------- | -------------- | ----- | ----------------------------------------------------------------------------------- |
| `area`              | string         | yes   | Must match area `name`.                                                             |
| `anum`              | integer        | yes   | Area-local number, unique within area.                                              |
| `name`              | string         | yes   | Space-separated keyword list.                                                       |
| `short_descr`       | string         | yes   | Short description seen in room/inventory.                                           |
| `description`       | string         | yes   | Text shown when object is on the floor.                                             |
| `item_type`         | string (enum)  | yes   | See [item_type](#item_type).                                                        |
| `values`            | object         | yes   | Per-type fields � see section below.                                                |
| `level`             | integer        | yes   | Minimum level to use (0 = no restriction).                                          |
| `weight`            | integer        | yes   | Weight in grams.                                                                    |
| `cost`              | integer        | yes   | Base value in silver coins.                                                         |
| `material`          | string (enum)  | (opt) | Default: `"generic"`. See [materials](#materials).                                  |
| `condition`         | integer        | (opt) | Condition 0�100. Default: 100 (perfect). Omitted when 100.                          |
| `extra_flags`       | string (flags) | (opt) | See [extra_flags](#extra_flags).                                                    |
| `wear_flags`        | string (flags) | (opt) | See [wear_flags](#wear_flags). Omit entirely for non-wearable items like furniture. |
| `extra_description` | array          | (opt) | See [extra_description](#extra_description).                                        |
| `affects`           | array          | (opt) | See [affect](#affect-within-affects).                                               |

---

### Object values by `item_type`

The `values` object fields vary by `item_type`. Fields with defaults may be omitted when the default value applies. Always include all meaningful fields for clarity.

#### `weapon`

```json
{
	"weapon_type": "sword",
	"dice_num": 2,
	"dice_size": 5,
	"attack_type": "slash",
	"flags": "sharp"
}
```

| Field         | Type           | Notes                                                         |
| ------------- | -------------- | ------------------------------------------------------------- |
| `weapon_type` | string (enum)  | See [weapon_types](#weapon_types).                            |
| `dice_num`    | integer        | Number of damage dice.                                        |
| `dice_size`   | integer        | Sides per damage die.                                         |
| `attack_type` | string (enum)  | Attack verb. See [attacks](#attacks).                         |
| `flags`       | string (flags) | Weapon enhancements. See [weapon_flags](#weapon_flags). (opt) |

#### `armor`

```json
{ "vs_pierce": 6, "vs_bash": 5, "vs_slash": 7, "vs_magic": 5 }
```

| Field       | Type    | Notes                                        |
| ----------- | ------- | -------------------------------------------- |
| `vs_pierce` | integer | AC vs piercing (higher = better for wearer). |
| `vs_bash`   | integer | AC vs bashing.                               |
| `vs_slash`  | integer | AC vs slashing.                              |
| `vs_magic`  | integer | AC vs magical.                               |

#### `container`

```json
{
	"capacity": 50,
	"flags": "closeable",
	"key": 3050,
	"max_weight": 100,
	"weight_mult": 75
}
```

| Field         | Type                  | Notes                                                                  |
| ------------- | --------------------- | ---------------------------------------------------------------------- |
| `capacity`    | integer               | Max number of items.                                                   |
| `flags`       | string (flags)        | See [container_flags](#container_flags). (opt)                         |
| `key`         | integer (global vnum) | Key item vnum. `0` = no lock.                                          |
| `max_weight`  | integer               | Max weight of contents. `0` = no limit.                                |
| `weight_mult` | integer               | Percentage of contents weight added to container weight. `100` = full. |

#### `drink` / `fountain`

```json
{ "capacity": 16, "filled": 16, "liquid": "beer", "poisoned": false }
```

| Field      | Type          | Notes                                               |
| ---------- | ------------- | --------------------------------------------------- |
| `capacity` | integer       | Maximum liquid units.                               |
| `filled`   | integer       | Current liquid units (0 = empty).                   |
| `liquid`   | string (enum) | See [liquids](#liquids).                            |
| `poisoned` | boolean       | Whether contents are poisoned. (opt, default false) |

`fountain` uses the same schema but the object cannot be taken and refills each area reset.

#### `food`

```json
{ "hunger": 18, "fullness": 12, "poisoned": false }
```

| Field      | Type    | Notes                                              |
| ---------- | ------- | -------------------------------------------------- |
| `hunger`   | integer | Hunger points restored.                            |
| `fullness` | integer | Fullness points restored.                          |
| `poisoned` | boolean | Whether the food is poisoned. (opt, default false) |

#### `light`

```json
{ "duration": 250 }
```

| Field      | Type    | Notes                                                                       |
| ---------- | ------- | --------------------------------------------------------------------------- |
| `duration` | integer | Hours of illumination. `-1` = permanent. All other value slots are ignored. |

#### `wand` / `staff`

```json
{ "level": 10, "recharge": 5, "charges": 5, "skill": "magic missile" }
```

| Field      | Type          | Notes                                 |
| ---------- | ------------- | ------------------------------------- |
| `level`    | integer       | Effective spell level when cast.      |
| `recharge` | integer       | Max charges after recharging.         |
| `charges`  | integer       | Current charges remaining.            |
| `skill`    | string (enum) | Spell to cast. See [skills](#skills). |

#### `scroll` / `potion` / `pill`

```json
{ "level": 15, "skill1": "cure light", "skill2": "bless", "skill3": "armor" }
```

| Field    | Type          | Notes                                       |
| -------- | ------------- | ------------------------------------------- |
| `level`  | integer       | Effective spell level.                      |
| `skill1` | string (enum) | (opt) Primary spell. See [skills](#skills). |
| `skill2` | string (enum) | (opt) Second spell.                         |
| `skill3` | string (enum) | (opt) Third spell.                          |
| `skill4` | string (enum) | (opt) Fourth spell.                         |

#### `money`

```json
{ "silver": 100, "gold": 5 }
```

#### `furniture`

```json
{
	"max_people": 4,
	"max_weight": 500,
	"flags": "sit_on rest_on sleep_on",
	"heal_rate": 150,
	"mana_rate": 150
}
```

| Field        | Type           | Notes                                                      |
| ------------ | -------------- | ---------------------------------------------------------- |
| `max_people` | integer        | Maximum occupants.                                         |
| `max_weight` | integer        | Maximum combined weight of occupants.                      |
| `flags`      | string (flags) | Allowed postures. See [furniture_flags](#furniture_flags). |
| `heal_rate`  | integer        | HP regen multiplier while using this furniture.            |
| `mana_rate`  | integer        | Mana regen multiplier.                                     |

#### `portal` (portal-type object)

```json
{
	"charges": -1,
	"exit_flags": "door",
	"gate_flags": "normal_exit go_with",
	"to_vnum": 3001,
	"key": 0
}
```

| Field        | Type                  | Notes                             |
| ------------ | --------------------- | --------------------------------- |
| `charges`    | integer               | Uses remaining. `-1` = unlimited. |
| `exit_flags` | string (flags)        | See [exit_flags](#exit_flags).    |
| `gate_flags` | string (flags)        | See [gate_flags](#gate_flags).    |
| `to_vnum`    | integer (global vnum) | Global destination vnum.          |
| `key`        | integer (global vnum) | Key item. `0` = no key needed.    |

#### `map`

```json
{ "persist": true }
```

| Field     | Type    | Notes                               |
| --------- | ------- | ----------------------------------- |
| `persist` | boolean | Whether the map survives after use. |

#### No-value types

These item types carry no meaningful values. Provide `"values": {}`:

`trash`, `gem`, `treasure`, `key`, `jewelry`, `clothing`, `boat`, `npc_corpse`, `pc_corpse`, `jukebox`, `warp_stone`

---

### affect (within `affects`)

Affects modify the wearer while the object is equipped. Two mutually exclusive forms � use one or the other, never both in the same affect entry.

**Stat affect** � adds or subtracts a numeric attribute:

```json
{ "level": 8, "apply": "strength", "modifier": 2 }
```

**Bit affect** � toggles flag bits:

```json
{ "level": 10, "bit_type": "affects", "bits": "flying haste" }
```

| Field      | Type           | Req    | Notes                                                                                                 |
| ---------- | -------------- | ------ | ----------------------------------------------------------------------------------------------------- |
| `level`    | integer        | yes    | Level at which the affect is applied.                                                                 |
| `apply`    | string (enum)  | one of | Attribute name. See [affect_apply](#affect_apply).                                                    |
| `modifier` | integer        | one of | Amount to add (negative = subtract).                                                                  |
| `bit_type` | string (enum)  | one of | Which flag set: `"affects"`, `"object"`, `"immune"`, `"resist"`, `"vuln"`, `"weapon"`.                |
| `bits`     | string (flags) | one of | Flags to set. Flag table depends on `bit_type` � see [bit_type flag mapping](#bit_type-flag-mapping). |

---

### extra_description

Used in both `room.extra_description` and `object.extra_description`:

```json
{
	"keyword": "plaque inscription",
	"description": "The plaque reads:\nBuilt in 1992."
}
```

| Field         | Type   | Req | Notes                                                               |
| ------------- | ------ | --- | ------------------------------------------------------------------- |
| `keyword`     | string | yes | Space-separated keywords. Player uses `look <keyword>` to see this. |
| `description` | string | yes | Text displayed when a matching keyword is looked at.                |

---

### social

**File:** `json/config/socials.json` | **Wrapping key:** `"social"`

```json
{
	"social": {
		"name": "gack",
		"char_no_arg": "Gaaack!",
		"others_no_arg": "$n gacks with dismay!",
		"char_found": "Appalled, you gack at $N.",
		"others_found": "$n gacks at $N.",
		"vict_found": "$n gacks, and looks your way.",
		"char_not_found": "That person is not here.",
		"char_auto": "*GACK!*",
		"others_auto": "Appalled, $n gacks at $mself!",
		"min_pos": "resting"
	}
}
```

| Field            | Type          | Req   | Notes                                                                       |
| ---------------- | ------------- | ----- | --------------------------------------------------------------------------- |
| `name`           | string        | yes   | Command name (lowercase, single word).                                      |
| `char_no_arg`    | string        | (opt) | Shown to actor when used with no target.                                    |
| `others_no_arg`  | string        | (opt) | Shown to room when used with no target.                                     |
| `char_found`     | string        | (opt) | Shown to actor when targeting another player.                               |
| `others_found`   | string        | (opt) | Shown to room when targeting another player.                                |
| `vict_found`     | string        | (opt) | Shown to the target player.                                                 |
| `char_not_found` | string        | (opt) | Shown to actor when target is not found.                                    |
| `char_auto`      | string        | (opt) | Shown to actor when targeting themselves.                                   |
| `others_auto`    | string        | (opt) | Shown to room when targeting themselves.                                    |
| `min_pos`        | string (enum) | (opt) | Minimum position to use. Default: `"resting"`. See [positions](#positions). |

---

### portal (config)

**File:** `json/config/portals.json` | **Wrapping key:** `"portal"`

Defines the link between named portal exit points referenced by rooms and exits.

```json
{ "portal": { "two-way": true,  "from": "midgaard-room-1", "to": "school-room-1" } }
{ "portal": { "two-way": false, "from": "arachnos-north-1", "to": "haon-room-1" } }
```

| Field     | Type    | Req | Notes                                       |
| --------- | ------- | --- | ------------------------------------------- |
| `two-way` | boolean | yes | If true, the link works in both directions. |
| `from`    | string  | yes | Source portal exit name.                    |
| `to`      | string  | yes | Destination portal exit name.               |

Named portal exit points appear in `room.portal` and `exit.portal` fields. A room or exit assigns its named exit point; the portals config wires two named points together.

Recommended naming convention: `"<area>-<direction>-<number>"` or `"<area>-room-<number>"`.

---

### help_area

**File:** `json/help/<name>.json`

```json
{
	"help_area": {
		"area": "help",
		"name": "ROM Help",
		"filename": "help",
		"pages": [
			{
				"help": {
					"keyword": "newbie beginner",
					"text": "Welcome!\n...",
					"level": 0
				}
			}
		]
	}
}
```

| Field      | Type   | Req | Notes                                 |
| ---------- | ------ | --- | ------------------------------------- |
| `area`     | string | yes | Area name this collection belongs to. |
| `name`     | string | yes | Display name of the help collection.  |
| `filename` | string | yes | Base filename (no extension).         |
| `pages`    | array  | yes | Array of `{"help": {...}}` entries.   |

### help (within `help_area` pages)

| Field           | Type    | Req   | Notes                                                                        |
| --------------- | ------- | ----- | ---------------------------------------------------------------------------- |
| `keyword`       | string  | yes   | Space-separated keywords. First keyword is the canonical name.               |
| `text`          | string  | yes   | Help text body.                                                              |
| `level`         | integer | (opt) | Minimum level to view. `-1` = immortal only. Default: 0.                     |
| `hide_keywords` | boolean | (opt) | When true and `level >= 0`, entry is hidden from help lists. Default: false. |

---

## Valid Enum Values

### sector_type

Used in `room.sector_type`:

`inside`, `city`, `field`, `forest`, `hills`, `mountain`, `swim`, `noswim`, `air`, `desert`

### positions

Used in `mobile.start_pos`, `mobile.default_pos`, `social.min_pos`:

`dead`, `mort`, `incap`, `stun`, `sleep`, `rest`, `sit`, `fight`, `stand`

### sizes

Used in `mobile.size`:

`tiny`, `small`, `medium`, `large`, `huge`, `giant`

### sexes

Used in `mobile.sex`:

`neutral`, `male`, `female`, `either`

### wear_loc

Used in `reset.equip.wear_loc`:

`none`, `light`, `lfinger`, `rfinger`, `neck1`, `neck2`, `body`, `head`, `legs`, `feet`, `hands`, `arms`, `shield`, `about`, `waist`, `lwrist`, `rwrist`, `wielded`, `hold`, `floating`, `tail`

### item_type

Used in `object.item_type` and `shop.trades[]`:

`light`, `scroll`, `wand`, `staff`, `weapon`, `treasure`, `armor`, `potion`, `clothing`, `furniture`, `trash`, `container`, `drink`, `key`, `food`, `money`, `boat`, `npc_corpse`, `pc_corpse`, `fountain`, `pill`, `map`, `portal`, `warp_stone`, `gem`, `jewelry`, `jukebox`

> **Note:** The raw config also contains `unused_item_1` through `unused_item_3`, which appear in some legacy shop `trades` arrays. Do not use them in new data.

### weapon_types

Used in `object.values.weapon_type` for `item_type: "weapon"`:

`sword`, `mace`, `dagger`, `axe`, `staff`, `flail`, `whip`, `polearm`

### attacks

Used in `mobile.attack_type` and `object.values.attack_type`:

`none`, `slice`, `stab`, `slash`, `whip`, `claw`, `blast`, `pound`, `crush`, `grep`, `bite`, `pierce`, `suction`, `beating`, `digestion`, `charge`, `slap`, `punch`, `wrath`, `magic`, `divine`, `cleave`, `scratch`, `peck`, `peckb`, `chop`, `sting`, `smash`, `shbite`, `flbite`, `frbite`, `acbite`, `chomp`, `drain`, `thrust`, `slime`, `shock`, `thwack`, `flame`, `chill`

### races

Used in `mobile.race`:

`unique`, `bat`, `bear`, `cat`, `centipede`, `dog`, `doll`, `dragon`, `dwarf`, `elf`, `fido`, `fox`, `giant`, `goblin`, `hobgoblin`, `human`, `kobold`, `lizard`, `modron`, `orc`, `pig`, `pixie`, `rabbit`, `school monster`, `snake`, `song bird`, `troll`, `water fowl`, `wolf`, `wyvern`

### materials

Used in `mobile.material` and `object.material`. Default for mobiles: `"flesh"`. Default for objects: `"generic"`.

`generic`, `adamantite`, `aluminum`, `brass`, `bronze`, `china`, `clay`, `cloth`, `copper`, `crystal`, `diamond`, `energy`, `flesh`, `food`, `fur`, `gem`, `glass`, `gold`, `ice`, `iron`, `ivory`, `lead`, `leather`, `meat`, `mithril`, `obsidian`, `paper`, `parchment`, `pearl`, `platinum`, `rubber`, `shadow`, `silver`, `steel`, `tin`, `vellum`, `water`, `wood`

### liquids

Used in `object.values.liquid` for `drink` and `fountain` items:

`water`, `beer`, `red wine`, `ale`, `dark ale`, `whisky`, `lemonade`, `firebreather`, `local specialty`, `slime mold juice`, `milk`, `tea`, `coffee`, `blood`, `salt water`, `coke`, `root beer`, `elvish wine`, `white wine`, `champagne`, `mead`, `rose wine`, `benedictine wine`, `vodka`, `cranberry juice`, `orange juice`, `absinthe`, `brandy`, `aquavit`, `schnapps`, `icewine`, `amontillado`, `sherry`, `framboise`, `rum`, `cordial`

### skills

Used in `object.values.skill` (wand/staff) and `object.values.skill1`�`skill4` (scroll/potion/pill):

`acid blast`, `armor`, `bless`, `blindness`, `burning hands`, `call lightning`, `calm`, `cancellation`, `cause critical`, `cause light`, `cause serious`, `chain lightning`, `change sex`, `charm person`, `chill touch`, `colour spray`, `continual light`, `control weather`, `create food`, `create rose`, `create spring`, `create water`, `cure blindness`, `cure critical`, `cure disease`, `cure light`, `cure poison`, `cure serious`, `curse`, `demonfire`, `detect evil`, `detect good`, `detect hidden`, `detect invis`, `detect magic`, `detect poison`, `dispel evil`, `dispel good`, `dispel magic`, `earthquake`, `enchant armor`, `enchant weapon`, `energy drain`, `faerie fire`, `faerie fog`, `farsight`, `fireball`, `fireproof`, `flamestrike`, `fly`, `floating disc`, `frenzy`, `gate`, `giant strength`, `harm`, `haste`, `heal`, `heat metal`, `holy word`, `identify`, `infravision`, `invisibility`, `know alignment`, `lightning bolt`, `locate object`, `magic missile`, `mass healing`, `mass invis`, `nexus`, `pass door`, `plague`, `poison`, `portal`, `protection evil`, `protection good`, `ray of truth`, `recharge`, `refresh`, `remove curse`, `restore mana`, `sanctuary`, `shield`, `shocking grasp`, `sleep`, `slow`, `stone skin`, `summon`, `teleport`, `ventriloquate`, `weaken`, `word of recall`, `acid breath`, `fire breath`, `frost breath`, `gas breath`, `lightning breath`, `general purpose`, `high explosive`

### spec_funs

Used in `mobile.spec_fun`:

`spec_breath_any`, `spec_breath_acid`, `spec_breath_fire`, `spec_breath_frost`, `spec_breath_gas`, `spec_breath_lightning`, `spec_cast_adept`, `spec_cast_cleric`, `spec_cast_judge`, `spec_cast_mage`, `spec_cast_undead`, `spec_executioner`, `spec_fido`, `spec_guard`, `spec_janitor`, `spec_mayor`, `spec_poison`, `spec_thief`, `spec_nasty`, `spec_troll_member`, `spec_ogre_member`, `spec_patrolman`

### clans

Used in `room.clan`:

`loner`, `rom`

Omit the `clan` field for clanless rooms.

### affect_apply

Used in stat affects (`affect.apply`):

`none`, `strength`, `dexterity`, `intelligence`, `wisdom`, `constitution`, `sex`, `class`, `level`, `age`, `height`, `weight`, `mana`, `hp`, `moves`, `gold`, `experience`, `armor class`, `hit roll`, `damage roll`, `saves`, `save vs rod`, `save vs petrification`, `save vs breath`, `save vs spell`

---

## Flag Strings

All multi-flag fields accept space-separated flag name strings. Omit the field entirely for zero flags.

### room_flags

`dark`, `no_mob`, `indoors`, `private`, `safe`, `solitary`, `pet_shop`, `no_recall`, `imp_only`, `gods_only`, `heroes_only`, `newbies_only`, `law`, `nowhere`

### mob_flags

`sentinel`, `scavenger`, `aggressive`, `stay_area`, `wimpy`, `pet`, `train`, `practice`, `undead`, `cleric`, `mage`, `thief`, `warrior`, `noalign`, `nopurge`, `outdoors`, `indoors`, `healer`, `gain`, `update_always`, `changer`, `noquest`

> `npc` is set automatically and should not be included in mob_flags.

> **`noquest`** — Prevents a mobile from being assigned as a quest target. Set this on mobs that shouldn't be killable for quests (shopkeepers, guards, quest-givers, etc.).

### extra_flags

`glow`, `hum`, `dark`, `lock`, `evil`, `invis`, `magic`, `nodrop`, `bless`, `antigood`, `antievil`, `antineutral`, `noremove`, `inventory`, `nopurge`, `rotdeath`, `visdeath`, `nonmetal`, `nolocate`, `meltdrop`, `hadtimer`, `sellextract`, `burnproof`, `nouncurse`, `corroded`, `reward`

> **`reward`** — Quest heirloom flag. Marks an object as a scaling heirloom reward.
> When a character levels up, every equipped `reward` item is automatically re-scaled to
> the new level via `obj_reward_scale()`. Stats are proportional to level: ~25% at
> level 1, 100% at level 45, with a small bonus above 45 and a ±1 variance above
> level 10. Combine with `nopurge` on prototypes so heirlooms survive world resets.

### wear_flags

`take`, `finger`, `neck`, `body`, `head`, `legs`, `feet`, `hands`, `arms`, `shield`, `about`, `waist`, `wrist`, `wield`, `hold`, `nosac`, `wearfloat`, `wearlight`, `weartail`

### exit_flags

`door`, `closed`, `locked`, `pickproof`, `nopass`, `easy`, `hard`, `infuriating`, `noclose`, `nolock`

### affect_flags

Used when `bit_type = "affects"`. Also used in `mobile.affected_by`:

`blind`, `invisible`, `detect_evil`, `detect_invis`, `detect_magic`, `detect_hidden`, `detect_good`, `sanctuary`, `faerie_fire`, `infrared`, `curse`, `poison`, `protect_evil`, `protect_good`, `sneak`, `hide`, `sleep`, `charm`, `flying`, `pass_door`, `haste`, `calm`, `plague`, `weaken`, `dark_vision`, `berserk`, `swim`, `regeneration`, `slow`

### off_flags

Used in `mobile.offense`:

`area_attack`, `backstab`, `bash`, `berserk`, `disarm`, `dodge`, `fade`, `fast`, `kick`, `dirt_kick`, `parry`, `rescue`, `tail`, `trip`, `crush`, `assist_all`, `assist_align`, `assist_race`, `assist_players`, `assist_guard`, `assist_vnum`

### res_flags

Used in `mobile.immune`, `mobile.resist`, `mobile.vulnerable`, and when `bit_type` is `"immune"`, `"resist"`, or `"vuln"`:

`summon`, `charm`, `magic`, `weapon`, `bash`, `pierce`, `slash`, `fire`, `cold`, `lightning`, `acid`, `poison`, `negative`, `holy`, `energy`, `mental`, `disease`, `drowning`, `light`, `sound`, `wood`, `silver`, `iron`

### form_flags

Used in `mobile.form`:

`edible`, `poison`, `magical`, `instant_decay`, `other`, `animal`, `sentient`, `undead`, `construct`, `mist`, `intangible`, `biped`, `centaur`, `insect`, `spider`, `crustacean`, `worm`, `blob`, `mammal`, `bird`, `reptile`, `snake`, `dragon`, `amphibian`, `fish`, `cold_blood`

### part_flags

Used in `mobile.parts`:

`head`, `arms`, `legs`, `heart`, `brains`, `guts`, `hands`, `feet`, `fingers`, `ear`, `eye`, `long_tongue`, `eyestalks`, `tentacles`, `fins`, `wings`, `tail`, `claws`, `fangs`, `horns`, `scales`, `tusks`

### weapon_flags

Used in `object.values.flags` for weapons, and when `bit_type = "weapon"`:

`flaming`, `frost`, `vampiric`, `sharp`, `vorpal`, `twohands`, `shocking`, `poison`

### container_flags

Used in `object.values.flags` for containers:

`closeable`, `pickproof`, `closed`, `locked`, `puton`

### gate_flags

Used in `object.values.gate_flags` for portal-type objects:

`normal_exit`, `no_curse`, `go_with`, `buggy`, `random`

### furniture_flags

Used in `object.values.flags` for furniture:

`stand_at`, `stand_on`, `stand_in`, `sit_at`, `sit_on`, `sit_in`, `rest_at`, `rest_on`, `rest_in`, `sleep_at`, `sleep_on`, `sleep_in`, `put_at`, `put_on`, `put_in`, `put_inside`

### bit_type flag mapping

When using a bit affect, the `bits` field uses the flag table corresponding to `bit_type`:

| `bit_type`  | Flag table     |
| ----------- | -------------- |
| `"affects"` | `affect_flags` |
| `"object"`  | `extra_flags`  |
| `"immune"`  | `res_flags`    |
| `"resist"`  | `res_flags`    |
| `"vuln"`    | `res_flags`    |
| `"weapon"`  | `weapon_flags` |

---

## Business Rules and Constraints

### Reset Ownership (Critical for Editors)

Resets in `room.resets` form an ordered flat array. **Order is significant.** The server processes them top-to-bottom each time the area resets:

- **`give`** resets are owned by the **nearest preceding `mobile` reset** � the item is placed in that mob's inventory.
- **`equip`** resets are owned by the **nearest preceding `mobile` reset** � the item is equipped on that mob.
- **`put`** resets are owned by the **nearest preceding `object` reset** that produced the `into` container � the item is placed inside that container.

An editor must enforce this ordering and visually group dependent resets under their owner. Example valid sequence:

```
reset: mobile  (mob=11)                      <- wizard spawns
reset: equip   (obj=5, wear_loc="wielded")   <- equipped on wizard
reset: give    (obj=7)                       <- in wizard's inventory
reset: give    (obj=8)                       <- also in wizard's inventory
reset: object  (obj=10)                      <- donation pit placed in room
reset: put     (obj=15, into=10)             <- item placed inside the pit
reset: mobile  (mob=20)                      <- guard spawns (new owner context)
```

A `give` or `equip` reset with no preceding `mobile` reset, or a `put` reset with no preceding `object` reset, is invalid and will be ignored at load time.

### Portal Naming

Room and exit `portal` fields hold a string key that must exactly match the `from` or `to` field of a portal entry in `json/config/portals.json`. If a named exit point has no matching portal entry, the exit has no destination.

A `two-way: true` portal means traversal works in both directions. A `two-way: false` portal only routes from `"from"` to `"to"`.

### Shopkeeper Stock

A shopkeeper mob `shop.trades` defines what item types the shop **buys from players**. What the shop **sells** is determined entirely by `give` resets: items in the mob's inventory (placed via `give` resets) appear as purchasable stock. There is no separate stock list in the JSON.

### Unlimited Spawning

`global_limit: -1` in a `mobile`, `object`, `give`, or `equip` reset means no world-wide cap on instances.

### Alignment Ranges

- **Evil:** -1000 to -350
- **Neutral:** -349 to 349
- **Good:** 350 to 1000

### AC Values

Mobile armor class uses negative = better (harder to hit). Typical ranges:

- Low-level mobs: 0 to -50
- Mid-level mobs: -50 to -125
- High-level mobs: -125 to -250

### Dice String Format

All dice fields must follow the exact format `"XdY+Z"` where X, Y, Z are integers. A constant value N is represented as `"1d1+<N-1>"` (result always equals N). Examples: `"1d8+0"`, `"2d6+4"`, `"1d1+999"`.

### Object `condition`

Range: 0�100. 100 = perfect condition. The field is omitted from output when the value is 100 (the reader defaults to 100 when absent).

### Room `heal_rate` / `mana_rate`

100 = normal regeneration speed. Common values:

- 50 = half rate (dangerous/hostile room)
- 100 = normal
- 200 = double rate (safe haven)

### Shop `trades` Maximum

A shop's `trades` array may contain at most **`MAX_TRADE` entries** (currently 16). The `buy_type` array is heap-allocated at shop creation with `buy_count = MAX_TRADE` slots; excess JSON entries are silently ignored. To allow more trade types, raise `MAX_TRADE` in `src/defs.h` and recompile.

### Armor `vs_magic` (Fourth Value)

The fourth field in `armor.values` is named `vs_magic`. Some older area files written before the named-values system may show this as `"value3"` � the reader accepts both names.

### Object `values` for No-Value Types

Item types with no meaningful values (`trash`, `gem`, `key`, etc.) must still include a `"values": {}` key with an empty object. The key must be present even if empty.

### Help `hide_keywords` and `level`

When `hide_keywords: true` and `level >= 0`, the entry is hidden from help search results but still accessible by exact keyword. The underlying engine stores this as a negative level internally, but the JSON always records the positive level and the boolean separately.

---

## Computed Fields (Never Write These)

The following fields are computed at server load time from surrounding context. Editors must not expose them as editable fields and must not write them to JSON:

| Field       | Entity                        | Notes                                                        |
| ----------- | ----------------------------- | ------------------------------------------------------------ |
| `room_vnum` | reset `values` (all commands) | Automatically resolved from the containing room's area+anum. |

---

## Config Tables: What You Can Edit Live

All files in `json/config/` are **loaded from JSON at boot**, not baked into the binary. This means most of them are fully editable — you change the file and restart the server (or use `jreload`). However some tables have fixed-size C arrays, so there is an upper cap on how many entries they can hold.

### jreload Reference

`jreload` is an implementor-only (level 60) command for live-reloading JSON data without restarting the server. It is always logged.

```
jreload list                -- list all reloadable config table names
jreload <table_name>        -- reload a config table (e.g. jreload greetings)
jreload areas <name>        -- force-reload all JSON files for one area (e.g. jreload areas midgaard)
jreload help <name>         -- reload a help JSON file (e.g. jreload help quest)
jreload quest_config        -- reload quest system parameters (quest_config.json)
```

| Subcommand               | What it reloads                                               |
| ------------------------ | ------------------------------------------------------------- |
| `list`                   | Prints all table names accepted by `jreload <table_name>`.   |
| `<table_name>`           | Any config table in `json/config/` (see `jreload list`).     |
| `areas <name>`           | All JSON entity files for the named area (rooms, mobs, objects, resets, area meta). The area name must match the folder under `json/areas/`. |
| `help <name>`            | One help bundle from `json/help/<name>.json` (e.g. `quest`, `olc`, `rom`). All pages in the file are re-read; stale pages are removed. |
| `quest_config`           | `json/config/quest_config.json` — quest tuning parameters.   |

> Quests already in progress use parameter values that were active when the quest was assigned. `jreload quest_config` only affects **new** quests started after the reload.



### Fully editable, no cap concerns

These tables use heap-allocated storage and all internal per-entry sub-arrays are also heap-allocated. There is no compile-time limit:

| File                | Currently | Notes                                                                                                                                                                                                                    |
| ------------------- | --------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `classes.json`      | 4         | Classes: name, stat, THAC0, HP gain, mana, guild rooms, default groups. `guild[]` and `titles[]` arrays are heap-allocated. `CLASS_MAX` has been removed from the codebase; classes are fully dynamic.                   |
| `pc_races.json`     | 6         | Player-selectable races: stats, class exp modifiers, bonus skills. `class_mult[]` and `skills[]` are heap-allocated. No compile-time cap; the character-creation listing in `nanny.c` now uses `pc_race_count` directly. |
| `skills.json`       | ~137      | Skills and spells: per-class level/effort, mana cost, beats, target, position. `classes[]` sub-array is heap-allocated (one entry per loaded class).                                                                     |
| `skill_groups.json` | 27        | Skill groups: member skills, per-class purchase cost. `classes[]` and `spells[]` are both heap-allocated.                                                                                                                |
| `liquids.json`      | 36        | Liquid types: name, color, thirst/hunger/intoxication values. Fully dynamic; `LIQ_MAX` has been removed from the codebase.                                                                                               |
| `materials.json`    | 38        | Material names and properties. Fully dynamic; `MATERIAL_MAX` has been removed from the codebase.                                                                                                                         |
| `socials.json`      | many      | Emote commands (world data — fully writable)                                                                                                                                                                             |
| `portals.json`      | many      | Inter-area portal links (world data — fully writable)                                                                                                                                                                    |
| `greetings.json`    | 1–4+      | Login greeting screens shown to connecting players. Up to 4 are chosen randomly at connect time. Supports `{x` color codes. Live-editable via `jreload greetings`. See [greetings](#greetings) schema below.            |
| `races.json`        | 30        | Race definitions: flags, stats, ext flags. Fully dynamic; `RACE_MAX` has been removed from the codebase. `race_count` is used wherever race limits are needed.                                                           |
| `quest_rewards.json` | 6        | Quest rewards purchasable with quest points. Each entry has an id, label, keyword aliases, cost, type, and value. See [quest_reward](#quest_reward) schema below.                                                      |
| `quest_tokens.json`  | 5        | Object vnums used as random quest item targets. A random entry is chosen when a player is assigned an item recovery quest. See [quest_token](#quest_token) schema below.                                               |
| `quest_config.json`  | 1        | Singleton object of 23 integer tuning parameters for the quest system (timers, rewards, type chances). Hot-reloadable via `jreload quest_config`. See [quest_config](#quest_config) schema below.                      |
| `pocket_dungeon_config.json` | 1 | Pocket dungeon system parameters: autopurge, timeouts, vnum allocation, scaling, testing mode, and gold costs. Hot-reloadable via `jreload pocket_dungeon_config`. See [pocket_dungeon_config](#pocket_dungeon_config) schema below. |
| `pd_loot_themes.json` | 1+ | Per-seed loot theme definitions: themed spell/stat pools for procedural item generation, wand charge ranges, potion level ranges, and authored boss heirloom vnum. Loaded at boot via the normal JSON import path. See [pd_loot_theme](#pd_loot_theme) schema below. |

> **Note on remaining fixed-size sub-array limits:** The `shop.trades` array is dynamically allocated and sized to `MAX_TRADE = 16` entries per shop; adding more than 16 trade types requires raising `MAX_TRADE` in `src/defs.h` and recompiling. Songs loaded from `music.txt` are capped at `MAX_SONG_LINES = 100` lyrics per song.

### Read from JSON but not normally edited

These config tables are populated directly from internal C data structures (lookup tables, app tables, etc.). They are loaded by JSON at boot, but their values are tightly coupled to hardcoded C behavior. Editing them without corresponding C changes will have no visible effect or may cause errors:

`attacks.json`, `dam_types.json`, `items.json`, `positions.json`, `sectors.json`, `sexes.json`, `sizes.json`, `weapons.json`, `wear_locs.json`, `doors.json`, `str_app.json`, `dex_app.json`, `int_app.json`, `wis_app.json`, `con_app.json`, `skies.json`, `suns.json`, `days.json`, `months.json`, `hp_conds.json`, `colors.json`, `color_settings.json`, `conds.json`, `songs.json`, `pose.json`, `boards.json`

### greetings

**File:** `json/config/greetings.json`  
**Live-reload:** `jreload greetings`

Contains one or more login greeting screens shown to players when they first connect. The server picks randomly between all defined entries. The displayed text is passed through the colour processor, so `{x` colour codes (e.g. `{R`, `{G`, `{Y`, `{x`) are fully supported.

Up to 4 entries map to the named globals used in the connect/ANSI-prompt flow:

| Array index | Internal global   |
| ----------- | ----------------- |
| 0           | `help_greeting`   |
| 1           | `help_greeting1`  |
| 2           | `help_greeting2`  |
| 3           | `help_greeting3`  |

Entries beyond index 3 are loaded into memory but are not currently reachable by the random picker (`number_range(0, 3)`). To use more than 4, raise the upper bound in `src/descs.c` and `src/nanny.c` and add corresponding globals.

The server will abort at boot if `greeting_count == 0` (i.e. the file is missing or empty).

#### Schema

```json
[
  {
    "greeting": {
      "text": "(required) The full text sent to a connecting player. Supports {x colour codes."
    }
  }
]
```

| Field  | Type   | Required | Notes                                                                                                               |
| ------ | ------ | -------- | ------------------------------------------------------------------------------------------------------------------- |
| `text` | string | yes      | Raw greeting text. Newlines are written using the `\n...\|` pipe-continuation format. Colour codes are processed.  |

#### Example

```json
[
  {
    "greeting": {
      "text": "
|{YBASEMUD{x  --  A ROM 2.4 derivative
|
|By what name do you wish to be known? "
    }
  }
]
```

---

### quest_reward

**File:** `json/config/quest_rewards.json`  
**Wrapping key:** `"quest_reward"`

Defines items and other rewards that players can purchase with accumulated quest points. The list is read into `quest_reward_table[]` at boot and drives both `quest list` and `quest buy` output.

```json
{
  "quest_reward": {
    "id":       "amulet",
    "label":    "Amulet of Moongate",
    "keywords": "amulet",
    "cost":     750,
    "type":     "object",
    "value":    200
  }
}
```

| Field      | Type    | Req | Notes                                                                                                                                      |
| ---------- | ------- | --- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| `id`       | string  | yes | Unique string key. Used as a display identifier in the editor.                                                                             |
| `label`    | string  | yes | Human-readable name shown in `quest list` output.                                                                                          |
| `keywords` | string  | yes | Space-separated namelist matched by `quest buy <keyword>`. Must include at least one alias.                                                |
| `cost`     | integer | yes | Quest points required to purchase.                                                                                                         |
| `type`     | string  | yes | Reward category: `"object"` (gives item by vnum), `"gold"` (awards gold), `"practices"` (adds practice sessions), or `"quest_chance"` (grants extra quest charges). |
| `value`    | integer | yes | Meaning depends on `type`: object vnum / gold amount / practice count / number of charges.                                                 |

---

### quest_token

**File:** `json/config/quest_tokens.json`  
**Wrapping key:** `"quest_token"`

Defines the pool of object vnums used as quest item targets for recovery quests. When a player is assigned an item quest, the server picks a random entry from this list and spawns that object in a random room for the player to retrieve.

All token objects live in the hidden `quest` area (vnums 204–208).

```json
{"quest_token": {"vnum": 204}}
```

| Field  | Type    | Req | Notes                                            |
| ------ | ------- | --- | ------------------------------------------------ |
| `vnum` | integer | yes | Global vnum of the object to use as quest token. |

> **Note:** Object vnums must exist in a loaded area. The quest token objects (204–208, item_type `treasure`, extra_flag `rotdeath`) are defined in `json/areas/quest/objects.json`.

---

### quest_config

**File:** `json/config/quest_config.json`  
**Wrapping key:** `"quest_config"`  
**Live-reload:** `jreload quest_config`

A singleton object containing all 23 integer tuning parameters for the quest system. Loaded at boot into the `quest_config` global struct and hot-reloadable without restart.

```json
[{"quest_config": {
  "quest_timer_min": 10, "quest_timer_max": 30,
  "cooldown_success": 10, "cooldown_none": 2,
  "gold_min": 2500, "gold_max": 45000,
  "qp_min": 25, "qp_max": 75,
  "practice_chance": 15, "practice_min": 1, "practice_max": 6,
  "obj_quest_chance": 40, "xp_chance_divisor": 20,
  "reward_level_divisor": 30,
  "purge_quest_chance": 30, "purge_count_min": 2, "purge_count_max": 5,
  "collect_quest_chance": 25, "collect_count_min": 2, "collect_count_max": 4,
  "xp_reward_min_pct": 1, "xp_reward_max_pct": 2,
  "train_chance": 5
}}]
```

| Field                  | Type    | Default | Notes                                                                                                             |
| ---------------------- | ------- | ------- | ----------------------------------------------------------------------------------------------------------------- |
| `quest_timer_min`      | integer | 10      | Minimum quest duration in minutes.                                                                                |
| `quest_timer_max`      | integer | 30      | Maximum quest duration in minutes.                                                                                |
| `cooldown_success`     | integer | 10      | Minutes before a player can request another quest after completing one.                                           |
| `cooldown_none`        | integer | 2       | Minutes before a player can try again when the questmaster has no quest available.                                |
| `gold_min`             | integer | 2500    | Baseline minimum gold reward before level scaling.                                                                |
| `gold_max`             | integer | 45000   | Baseline maximum gold reward before level scaling.                                                                |
| `qp_min`               | integer | 25      | Baseline minimum quest points before level scaling.                                                               |
| `qp_max`               | integer | 75      | Baseline maximum quest points before level scaling.                                                               |
| `practice_chance`      | integer | 15      | Percent chance to award bonus practice sessions on completion.                                                    |
| `practice_min`         | integer | 1       | Minimum bonus practices when triggered.                                                                           |
| `practice_max`         | integer | 6       | Maximum bonus practices when triggered.                                                                           |
| `obj_quest_chance`     | integer | 40      | Percent chance that a quest becomes an item recovery/collection quest instead of a mob slay.                      |
| `xp_chance_divisor`    | integer | 20      | Threshold divisor: players earn a quest charge every `exp_per_level / xp_chance_divisor` XP from kills.          |
| `reward_level_divisor` | integer | 30      | Gold and QP are multiplied by `player_level / reward_level_divisor`. At this level = baseline reward.             |
| `purge_quest_chance`   | integer | 30      | When a mob quest is generated, percent chance it becomes a multi-kill (purge) quest.                              |
| `purge_count_min`      | integer | 2       | Minimum number of mobs to kill for a purge quest.                                                                 |
| `purge_count_max`      | integer | 5       | Maximum number of mobs to kill for a purge quest.                                                                 |
| `collect_quest_chance` | integer | 25      | When an item quest is generated, percent chance it becomes a multi-item collection quest.                         |
| `collect_count_min`    | integer | 2       | Minimum number of items to collect for a collection quest.                                                        |
| `collect_count_max`    | integer | 4       | Maximum number of items to collect for a collection quest.                                                        |
| `xp_reward_min_pct`    | integer | 1       | Minimum XP awarded as a percentage of `exp_per_level`. Hard-capped at 5.                                          |
| `xp_reward_max_pct`    | integer | 2       | Maximum XP awarded as a percentage of `exp_per_level`. Hard-capped at 5.                                          |
| `train_chance`         | integer | 5       | Percent chance to award 1 training session on completion.                                                         |

**Reward scaling:** Gold, QP, and XP rewards are all affected by the player's level. For gold and QP the formula is `baseline * level / reward_level_divisor`. Multi-target quests (purge/collection) add +25% per extra target beyond 1.

**Hot-reload:** Changes to `quest_config.json` take effect immediately for all new quests after `jreload quest_config` — no restart required. Quests already in progress use the values that were active when the quest was assigned.

---

### pocket_dungeon_config

**File:** `json/config/pocket_dungeon_config.json`  
**Wrapping key:** `"pocket_dungeon_config"`  
**Live-reload:** `jreload pocket_dungeon_config`

A singleton object containing all tuning parameters for the pocket dungeon instanced area system. Includes configuration for autopurge timeouts, vnum allocation, scaling, Phase 2 features (testing mode), and gold-based entry costs.

```json
[{"pocket_dungeon_config": {
  "autopurge": true,
  "empty_timeout_mins": 120,
  "max_instances": 50,
  "vnum_base": 20000,
  "vnum_size": 100,
  "max_members": 10,
  "scaling_formula": 0,
  "testing_mode": false,
  "gold_cost_per_level": 100
}}]
```

| Field                 | Type    | Default | Notes                                                                                                                  |
| --------------------- | ------- | ------- | ---------------------------------------------------------------------------------------------------------------------- |
| `autopurge`           | boolean | true    | When true, empty instances are automatically destroyed after `empty_timeout_mins`.                                     |
| `empty_timeout_mins`  | integer | 120     | Minutes before an empty (all players left) instance is purged if `autopurge` is enabled.                              |
| `max_instances`       | integer | 50      | Hard cap on simultaneously loaded pocket dungeon instances across all player groups.                                   |
| `vnum_base`           | integer | 20000   | First vnum in the reserved range for dynamically generated instance areas. **Must not overlap any static area vnums.** |
| `vnum_size`           | integer | 100     | Vnums allocated per instance slot. Must be large enough for your largest seed (e.g. 100 rooms + mobs per instance).  |
| `max_members`         | integer | 10      | Maximum players allowed per dungeon instance (groups exceeding this are rejected at entry).                            |
| `scaling_formula`     | integer | 0       | Difficulty scaling mode: 0 = no scaling, 1 = by group size, 2 = by max group level (reserved for future use).        |
| `testing_mode`        | boolean | false   | When true, dungeon entry is **free** (ignores `gold_cost_per_level`). Useful for testing and development.             |
| `gold_cost_per_level` | integer | 100     | Gold cost per character level to enter (e.g. 100 = 1000 gold total for five level-10 players). **Bypassed if `testing_mode` is true.** |

**Gold cost calculation:** When a group enters a pocket dungeon, the total cost is calculated as: `cost = sum(each_player_level) * gold_cost_per_level`. Each player must have at least their share of gold, and the gold is deducted from all players at entry time.

**Scaling formula:** 
- `0`: No scaling by group size or level.
- `1`: Mobs and items scale by average group level.
- Higher values reserved for future use.

**Hot-reload:** Changes to `pocket_dungeon_config.json` take effect immediately after `jreload pocket_dungeon_config` — no restart required. Only **new instances** generated after the reload use the updated parameters; existing instances in memory are unaffected.

---

### pd_loot_theme

**File:** `json/config/pd_loot_themes.json`  
**Wrapping key:** `"pd_loot_theme"`  
**Live-reload:** restart required (loaded via normal JSON import path, no dedicated hot-reload helper).

Defines per-seed loot themes for the Pocket Dungeon procedural item enhancement system. Each entry is keyed by `name` and matched against the running instance's seed name at loot generation time. A special `"default"` entry is used as a fallback when no seed-specific theme is found.

Items are enhanced by `pd_enhance_obj()` in `src/pd_loot.c` immediately after creation. Quality tier is location-driven: `floor=1 affix, chest=2, boss=3`.

```json
[{"pd_loot_theme": {
  "name": "crypt",
  "title": "Crypt",
  "spell_pool": ["cure light", "cure serious", "protection evil", "cure poison"],
  "stat_pool":  ["strength", "constitution", "armor class"],
  "item_vnums": [],
  "wand_charges_min": 4,
  "wand_charges_max": 10,
  "potion_level_min": 5,
  "potion_level_max": 30,
  "boss_drop_vnum": 0
}}]
```

| Field | Type | Req | Notes |
| --- | --- | --- | --- |
| `name` | string | yes | Stable id matched against `inst->theme` (the seed name). Use `"default"` for the global fallback. |
| `title` | string | yes | Display label shown in MUDEditor. |
| `spell_pool` | string[] | no | Named spells drawn from for potions, scrolls, and wands. Resolved at load time via `skill_lookup_exact()`. Missing/unknown spell names are silently skipped. Falls back to a hardcoded beneficial list if empty. |
| `stat_pool` | string[] | no | Named `APPLY_*` constants drawn from for jewelry and equipment affixes. Resolved via `type_lookup_exact(affect_apply_types, ...)`. Falls back to STR/DEX/INT/WIS/CON cycle if empty. Valid names match `affect_apply_types[]` in `src/types.c` (e.g. `"strength"`, `"dexterity"`, `"hit roll"`, `"dam roll"`, `"armor class"`). |
| `item_vnums` | integer[] | no | Reserved for future per-theme item pool override. Currently unused by the enhancer. |
| `wand_charges_min` | integer | no | Minimum charges rolled for generated wands. Defaults to tier-based range when 0. |
| `wand_charges_max` | integer | no | Maximum charges rolled for generated wands. Defaults to tier-based range when 0. |
| `potion_level_min` | integer | no | Minimum spell level clamped onto generated potions. Defaults to instance level when 0. |
| `potion_level_max` | integer | no | Maximum spell level clamped onto generated potions. Defaults to instance level when 0. |
| `boss_drop_vnum` | integer | no | Vnum of an authored "heirloom" item created at boss kill. The item is created at `inst->level+5` with no further enhancement. `0` = no authored drop; a random pool item gets highest-tier enhancement instead. |

**Tier bonuses applied by the enhancer:**

| Item type | Floor (tier 1) | Chest (tier 2) | Boss (tier 3) |
| --- | --- | --- | --- |
| Potion / Scroll / Pill | 1 spell, level clamped to range | 2 spells | 2 spells |
| Wand / Staff | charges from theme range or tier roll | same | same |
| Weapon | +hit, +dam | +hit, +dam | +hit, +dam, +STR or +DEX |
| Armor | AC scaled by tier | AC + CON | AC + STR |
| Jewelry / Treasure | 1 stat affix | 2 stat affixes | 3 stat affixes |

Equipment naming prefixes: tier 2 (chest) prepends `"a fine "`, tier 3 (boss) prepends `"a rare "`. Consumables never receive naming prefixes but are marked `ITEM_UNIDENTIFIED` at creation.

**ITEM_UNIDENTIFIED flag:** All procedurally generated consumables (potions, scrolls, pills, wands, staves) are created with `ITEM_UNIDENTIFIED` set. While the flag is set:
- `look <item>` suppresses the authored description and shows a generic label (e.g. "A murky liquid sloshes around inside.")
- `do_sip` gives a thematic taste hint and clears the flag on an INT/WIS/lore-gated chance roll
- `do_lore` and `spell_identify` clear the flag when the player's knowledge threshold is met (≥60%)

**MUDEditor:** The loot themes are fully editable in the PocketDungeonPage LootTab. The server route is `GET/POST/PUT/DELETE /api/config/pocket-dungeon-loot-themes` backed by `pocketDungeonLootThemesRouter()` in `web/server/src/routes/config.ts`.

---

### Adding new spells

The skill data (level requirements, mana, beats, target) lives in `skills.json` and is fully live. However, the `spell_fun` field references a C function that must exist in `src/spell_dispatch.c`. You cannot add _new spell behavior_ without writing and compiling C code — but you can:

- Freely adjust any existing spell's level requirements, mana cost, beats, and class availability
- Create a new skill slot that reuses an existing `spell_fun` (e.g. a renamed version of an existing spell)
- Leave `spell_fun` absent to create a passive or combat skill with no spell effect

---

_Generated from BaseMUD source: `src/json_objr.c`, `src/json_objw.c`, `src/json_tblr.c`, `src/json_tblw.c`, `src/flags.c`, `src/ext_flags.c`, `src/tables.c`, `src/types.c`, `src/defs.h`, `src/spell_dispatch.c`, `src/recycle.c`, `src/lookup.c`._
