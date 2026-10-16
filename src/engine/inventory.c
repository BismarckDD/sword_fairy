/* Manual reconstruction. Valid actor pointers are caller's responsibility.
 * Preserve the original's signed comparisons and 16-bit wrapping arithmetic.
 * These routines deliberately do NOT introduce modern inventory behavior.
 */
#include "save_format.h"
#include <string.h>

/* 1f2a:8bd6 (linear 27e76), called after item count changes. */
void jx_compact_inventory(JxSave *s) {
    JxInventorySlot compact[64]={{0,0}};
    unsigned n=0;
    for (unsigned i=0; i<64; ++i) {
        JxInventorySlot v=s->globals.inventory[i];
        if (v.item_id>0 && v.item_id<0x8000 && v.count>0 && v.count<0x8000)
            compact[n++]=v;
    }
    memcpy(s->globals.inventory,compact,sizeof compact);
}

/* 1f2a:8ca8 (27f48): original tests ID only, NOT quantity. */
int jx_has_item(const JxSave *s, int16_t item_id) {
    for (unsigned i=0; i<64; ++i)
        if (s->globals.inventory[i].item_id==(uint16_t)item_id) return 1;
    return 0;
}

/* 1f2a:8cea (27f8a); text popup 27fa5..28063 omitted.
 * A new ID overwrites slot 63, then compacts. It does not search for a free slot.
 * Existing IDs simply increment the WORD count, including overflow.
 */
void jx_grant_item_core(JxSave *s, int16_t item_id, int16_t count) {
    if (item_id<0 || count<=0) return;
    for (unsigned i=0; i<64; ++i) {
        if (s->globals.inventory[i].item_id==(uint16_t)item_id) {
            s->globals.inventory[i].count=(uint16_t)(s->globals.inventory[i].count+(uint16_t)count);
            return;
        }
    }
    s->globals.inventory[63].item_id=(uint16_t)item_id;
    s->globals.inventory[63].count=(uint16_t)count;
    jx_compact_inventory(s);
}

/* 1f2a:8f28 (281c8): first matching ID only, then compact. */
void jx_remove_item(JxSave *s, int16_t item_id, int16_t count) {
    for (unsigned i=0; i<64; ++i) {
        if (s->globals.inventory[i].item_id==(uint16_t)item_id) {
            s->globals.inventory[i].count=(uint16_t)(s->globals.inventory[i].count-(uint16_t)count);
            jx_compact_inventory(s);
            return;
        }
    }
}

/* 1f2a:8e1c (280bc): CBW/sign test retains only IDs 0..127. */
void jx_compact_skills(JxActor *actor) {
    uint8_t compact[16]; unsigned n=0;
    memset(compact,0xff,sizeof compact);
    for (unsigned i=0; i<16; ++i)
        if (actor->skill_ids[i]<0x80) compact[n++]=actor->skill_ids[i];
    memcpy(actor->skill_ids,compact,sizeof compact);
}

/* 1f2a:8ec4 (28164): compare a signed char ID, remove first match. */
void jx_remove_skill(JxActor *actor, int16_t skill_id) {
    for (unsigned i=0; i<16; ++i) {
        int id=actor->skill_ids[i];
        if (id>=128) id-=256;
        if (id==skill_id) {
            actor->skill_ids[i]=0xff;
            jx_compact_skills(actor);
            return;
        }
    }
}
