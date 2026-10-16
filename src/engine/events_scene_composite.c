#include "event_engine.h"

static uint16_t composite_next_page(JxEventContext *c) {
    return (uint16_t)((int16_t)(uint16_t)(jx_word(c,0x3e54)+1)%2);
}
/* 2F28:35FA. Background pages 95..98, two alternating transparent layers,
 * then display the rendered page. Signed remainders match the 16-bit IDIVs. */
void jx_scene_composite_frame(JxEventContext *c,uint16_t x,uint16_t first_frame,uint16_t second_frame) {
    (void)JX_CALL(c,0x1832,0x1810,composite_next_page(c));
    for (uint16_t slot=0;slot<4;slot++)
        (void)JX_CALL(c,0x1503,0x7d,slot,95+slot,jx_word(c,0x9332));
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a));
    int16_t first=(int16_t)first_frame%2,second=(int16_t)second_frame%2;
    for (uint16_t slot=0;slot<2;slot++)
        (void)JX_CALL(c,0x1503,0x7d,slot,(uint16_t)(91+slot+2*first),jx_word(c,0x9332));
    (void)JX_CALL(c,0x1832,0x1a28,x,50,64,16,jx_word(c,0x5858),jx_word(c,0x585a),255);
    for (uint16_t slot=0;slot<2;slot++)
        (void)JX_CALL(c,0x1503,0x7d,slot,(uint16_t)(87+slot+2*second),jx_word(c,0x9332));
    (void)JX_CALL(c,0x1832,0x1a28,160,56,160,144,jx_word(c,0x5858),jx_word(c,0x585a),255);
    (void)JX_CALL(c,0x1832,0x1836,composite_next_page(c));
    jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
}
