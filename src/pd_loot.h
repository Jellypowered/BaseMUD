/* pd_loot.h — Pocket Dungeon procedural item enhancer interface */
#ifndef __PD_LOOT_H
#define __PD_LOOT_H

#include "merc.h"
#include "structs.h"

#define PD_QUALITY_FLOOR 1
#define PD_QUALITY_CHEST 2
#define PD_QUALITY_BOSS  3

PD_LOOT_THEME_T *pd_loot_theme_get(const char *seed_name);
void             pd_enhance_obj(OBJ_T *obj, PD_INSTANCE_T *inst, int tier);

#endif /* __PD_LOOT_H */
