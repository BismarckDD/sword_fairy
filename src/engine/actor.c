#include "save_format.h"

static int16_t word_add(int16_t value, int delta) {
    unsigned bits=((unsigned)(uint16_t)value+(unsigned)delta)&65535u;
    return (int16_t)(bits>=32768u ? (int)bits-65536 : (int)bits);
}

/* 1f2a:747c (2671c). Poison remains set; recovery is halved while poisoned.
 * DIV is unsigned in the executable, hence the uint16_t casts below.
 */
void jx_restore_actor(JxActor *actor) {
    if (actor->condition==2) {
        actor->hp=(int16_t)((uint16_t)actor->max_hp/2u);
        actor->mp=(int16_t)((uint16_t)actor->max_mp/2u);
    } else {
        actor->hp=actor->max_hp;
        actor->mp=actor->max_mp;
    }
}

/* 1f2a:43b2 (23652). The name/level-up notification UI is omitted.
 * Gain tier is computed from the OLD level before incrementing.
 */
void jx_level_up_core(JxSave *save, int actor_index) {
    if (actor_index<0 || actor_index>4) return;
    JxActor *actor=&save->actors[actor_index];
    int tier=actor->level/10+1;
    actor->level=word_add(actor->level,1);
    actor->max_hp=word_add(actor->max_hp,tier*15);
    actor->max_mp=word_add(actor->max_mp,tier*10);
    actor->base_attack=word_add(actor->base_attack,tier*6);
    actor->base_defense=word_add(actor->base_defense,tier*6);
    jx_restore_actor(actor);
}
