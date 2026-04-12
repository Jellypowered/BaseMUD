/***************************************************************************
 *  BaseMUD - Original Code                                                 *
 *                                                                         *
 *  Summoning spells: Find Familiar, Mount, Animate Dead.                  *
 *  Implemented based on the John Lin spell list.                          *
 ***************************************************************************/

#include "spell_summon.h"

#include "act_comm.h"
#include "affects.h"
#include "chars.h"
#include "comm.h"
#include "groups.h"
#include "interp.h"
#include "items.h"
#include "magic.h"
#include "mobiles.h"
#include "objs.h"
#include "utils.h"

/* Find Familiar — summon a random magical companion.
 * The familiar grants AC -5 to the caster via a temporary affect.
 * If the familiar is slain, the caster suffers a -1 CON penalty for 200 ticks
 * (penalty applied in fight.c using MOB_VNUM_FAMILIAR_MIN/MAX). */
DEFINE_SPELL_FUN (spell_find_familiar) {
    MOB_INDEX_T *mob_index;
    CHAR_T *mob;
    int fam_type;
    AFFECT_T af;
    CHAR_T *fch;

    /* Only one familiar at a time. */
    for (fch = ch->in_room->people_first; fch != NULL; fch = fch->room_next) {
        if (IS_NPC (fch) && fch->master == ch &&
            fch->mob_index != NULL &&
            fch->mob_index->vnum >= MOB_VNUM_FAMILIAR_MIN &&
            fch->mob_index->vnum <= MOB_VNUM_FAMILIAR_MAX)
        {
            send_to_char ("Your familiar is already here with you.\n\r", ch);
            return;
        }
    }

    fam_type = number_range (0, FAMILIAR_TYPE_COUNT - 1);
    mob_index = mobile_get_index (MOB_VNUM_FAMILIAR_MIN + fam_type);
    BAIL_IF (mob_index == NULL,
        "The magical bond fails to form. (missing familiar mob)\n\r", ch);

    mob = mobile_create (mob_index);
    char_to_room (mob, ch->in_room);
    add_follower (mob, ch);
    mob->leader = ch;

    /* AC bonus to caster for the duration. */
    affect_init (&af, AFF_TO_AFFECTS, sn, level, level + 20, APPLY_AC, -5, 0);
    affect_copy_to_char (&af, ch);

    act ("A $N materialises before you, bonding its spirit to yours.", ch, NULL, mob, TO_CHAR);
    act ("A $N appears and answers $n's summons.", ch, NULL, mob, TO_NOTCHAR);
    do_function (mob, &do_say, "I am yours, master.");
}

/* Mount — summon a rideable animal based on caster level.
 * No actual mounted-combat mechanics; mob follows and assists. */
DEFINE_SPELL_FUN (spell_mount) {
    MOB_INDEX_T *mob_index;
    CHAR_T *mob;
    int tier;

    /* Choose mount tier by caster level (5 tiers: 0-4). */
    tier = UMIN (4, level / 10);
    mob_index = mobile_get_index (MOB_VNUM_MOUNT_MIN + tier);
    BAIL_IF (mob_index == NULL,
        "The summoning fails. (missing mount mob)\n\r", ch);

    mob = mobile_create (mob_index);
    char_to_room (mob, ch->in_room);
    add_follower (mob, ch);
    mob->leader = ch;

    act ("$N appears in a flash of light, ready to carry you!", ch, NULL, mob, TO_CHAR);
    act ("$N appears in a flash of light as $n's mount!", ch, NULL, mob, TO_NOTCHAR);
}

/* Animate Dead — raise an NPC corpse in the room as a zombie follower. */
DEFINE_SPELL_FUN (spell_animate_dead) {
    OBJ_T *corpse = NULL, *obj;
    CHAR_T *zombie;
    int i;

    /* Find an NPC corpse in the room. */
    for (obj = ch->in_room->content_first; obj != NULL;
         obj = obj->content_next)
    {
        if (obj->item_type == ITEM_CORPSE_NPC) {
            corpse = obj;
            break;
        }
    }

    BAIL_IF (corpse == NULL,
        "There is no suitable corpse here to animate.\n\r", ch);
    BAIL_IF (corpse->level > ch->level + 5,
        "That creature's spirit is too powerful for you to command.\n\r", ch);
    BAIL_IF (ch->pet != NULL,
        "You already have a pet.\n\r", ch);

    zombie = mobile_create (mobile_get_index (MOB_VNUM_ZOMBIE));
    zombie->level = corpse->level;
    zombie->max_hit = zombie->level * 8 +
        number_range (zombie->level * zombie->level / 4,
                      zombie->level * zombie->level);
    zombie->max_hit = zombie->max_hit * 9 / 10;
    zombie->hit = zombie->max_hit;
    zombie->max_mana = 100 + dice (zombie->level, 10);
    zombie->mana = zombie->max_mana;
    for (i = 0; i < 3; i++)
        zombie->armor[i] = int_interpolate (zombie->level, 100, -100);
    zombie->armor[3] = int_interpolate (zombie->level, 100, 0);
    for (i = 0; i < STAT_MAX; i++)
        zombie->perm_stat[i] = 11 + zombie->level / 4;

    char_to_room (zombie, ch->in_room);
    act ("You speak dark words and $p stirs to life as a zombie!", ch, corpse, NULL, TO_CHAR);
    act ("$n speaks dark words and $p rises as a zombie!", ch, corpse, NULL, TO_NOTCHAR);
    obj_extract (corpse);
    SET_BIT (zombie->affected_by, AFF_CHARM);
    EXT_SET (zombie->ext_mob, MOB_PET);
    zombie->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;
    add_follower (zombie, ch);
    zombie->leader = ch;
    ch->pet = zombie;
    do_function (zombie, &do_say, "How may I serve you, master?");
}
