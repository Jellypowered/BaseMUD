/***************************************************************************
 * Pocket Dungeon mobprog generation helpers.
 ***************************************************************************/
#ifndef POCKET_DUNGEON_MOBPROG_H
#define POCKET_DUNGEON_MOBPROG_H

#include "merc.h"

typedef enum pd_mobprog_personality {
    PD_MOBPROG_BRUTE = 0,
    PD_MOBPROG_TACTICIAN,
    PD_MOBPROG_GUARDIAN,
    PD_MOBPROG_COWARD,
    PD_MOBPROG_SUMMONER,
    PD_MOBPROG_BERSERKER,
    PD_MOBPROG_ASSASSIN,
    PD_MOBPROG_PERSONALITY_MAX
} PD_MOBPROG_PERSONALITY_T;

char *pd_generate_mobprog_script(CHAR_T *mob, int personality, int difficulty);
char *pd_generate_combat_ai(CHAR_T *mob, int personality, int difficulty);
char *pd_generate_passive_behavior(CHAR_T *mob, int difficulty);
char *pd_generate_personality_behavior(CHAR_T *mob, int personality);
const char *pd_describe_spell_for_personality(int personality, int level);
void pd_generate_and_attach_mobprog(CHAR_T *mob, PD_INSTANCE_T *instance);
int pd_select_personality(int mob_vnum);

#endif /* POCKET_DUNGEON_MOBPROG_H */