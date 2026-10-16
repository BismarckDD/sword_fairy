#include "event_engine.h"

/* 1F2A:CF6A. Private path reserves 256 bytes. The original greedy choice
 * can stop before reaching the target; this is not a general pathfinder.
 * Generated strings must fit the original buffer, as in the executable. */
void jx_npc_walk_to(JxEventContext *c,uint16_t npc,uint16_t target_x,uint16_t target_y,JxFarPointer path) {
    jx_far_set_byte(c,path,0,0);
    uint16_t actor=(uint16_t)(0x889c +86u*npc);
    uint16_t x=jx_word(c,(uint16_t)(actor+0x40)),y=jx_word(c,(uint16_t)(actor+0x42));
    for (;;) {
        uint8_t direction=(uint8_t)JX_CALL(c,0x1f2a,0xcb08,x,y,target_x,target_y);
        uint16_t text;
        switch (direction) {
        case 0:
            (void)JX_CALL(c,0x1f2a,0xcd1c,npc,path.offset,path.segment,2);return;
        case 'U':y--;text=0x8bf;break;
        case 'D':y++;text=0x8c1;break;
        case 'L':x--;text=0x8c3;break;
        case 'R':x++;text=0x8c5;break;
        default:continue;
        }
        (void)JX_CALL(c,0x1000,0x23f4,path.offset,path.segment,text,c->data_segment);
    }
}
