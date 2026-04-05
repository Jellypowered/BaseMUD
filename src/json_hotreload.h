/***************************************************************************
 *  BaseMUD JSON hot-reload system.                                        *
 *  Monitors json/areas/<name>/ for mtime changes and reloads areas        *
 *  in-process without disconnecting players.                              *
 ***************************************************************************/

#ifndef __ROM_JSON_HOTRELOAD_H
#define __ROM_JSON_HOTRELOAD_H

#include "merc.h"

#ifdef BASEMUD_JSON_HOTRELOAD

/* Number of pulses between filesystem scans (20 pulses = 5 seconds). */
#define HOTRELOAD_SCAN_PULSES 20
/* Seconds a file must be stable before a reload is triggered (5 seconds). */
#define HOTRELOAD_DEBOUNCE_SECS 10
/* Pulses of combat immunity granted after a reload (60 = 15 seconds). */
#define HOTRELOAD_IMMUNITY_PULSES 60
/* Maximum number of areas tracked simultaneously. */
#define HOTRELOAD_MAX_AREAS 256

/* Per-area tracking entry. */
typedef struct hotreload_entry
{
    AREA_T *area;
    char area_name[MAX_INPUT_LENGTH]; /* copy of area->name at init time */
    char dir_path[MAX_INPUT_LENGTH];  /* json/areas/<name>/ */
    time_t last_mtime;                /* highest mtime seen so far */
    time_t mtime_first_changed;       /* when we first noticed a change */
    bool reload_pending;              /* debounce timer running */
} HOTRELOAD_ENTRY_T;

/* Public API. */
void hotreload_init(void);
void hotreload_scan(void);
void hotreload_force_reload_area(const char *name, CHAR_T *ch);

#endif /* BASEMUD_JSON_HOTRELOAD */
#endif /* __ROM_JSON_HOTRELOAD_H */
