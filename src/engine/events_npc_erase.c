#include "event_engine.h"
/* 2F28:7978. Mark the NPC off-map before redrawing each occupied tile.
 * Captured position/dimensions stay fixed; camera offsets stay live. */
void jx_npc_erase_background(JxEventContext *c,uint16_t npc) {
    uint16_t record=(uint16_t)(0x889c+npc*86u);
    uint16_t shape=(uint16_t)(0x31ce + npc*18u);
    uint16_t x=jx_word(c,(uint16_t)(record+64));
    uint16_t y=jx_word(c,(uint16_t)(record+66));
    uint16_t width=jx_word(c,shape),height=jx_word(c,(uint16_t)(shape+2));
    jx_set_word(c,(uint16_t)(record+64),999);
    for(uint16_t row=0;jx_signed_word(row)<jx_signed_word(height);row++) {
        for(uint16_t column=0;jx_signed_word(column)<jx_signed_word(width);column++) {
            uint16_t tile_x=(uint16_t)(x+column-jx_word(c,0x8896));
            uint16_t tile_y=(uint16_t)(y-height+1+row-jx_word(c,0x8898));
            (void)JX_CALL(c,0x2f28,0x4fdc,tile_x,tile_y);
        }
    }
}
