#include "event_engine.h"
static uint16_t skill_panel_label(JxEventContext *c,uint16_t actor,uint16_t row,uint16_t column) {
    uint8_t v=jx_ds(c)[(uint16_t)(0xd04e + actor*68u+0x34+row*2u+column)];
    int skill=v<128?v:(int)v-256;
    return (uint16_t)(0x5154+skill*28);
}
static void skill_panel_text(JxEventContext *c,uint16_t x,uint16_t y,uint16_t offset) {
    (void)JX_CALL(c,0x1f2a,0x0728,x,y,offset,c->data_segment);
}
static void skill_panel_font(JxEventContext *c,uint16_t color) {
    (void)JX_CALL(c,0x1f2a,0x06f2,color,0,77);
}
/* 1F2A:9712. Reserve28 original frame bytes; formatted signed decimal
 * number is at work+6. Even negative skill slots are rendered through
 * their wrapping record address, as in the original. */
void jx_skill_menu_render(JxEventContext *c,uint16_t column,uint16_t row,uint16_t actor,JxFarPointer work) {
    for(uint16_t slot=0;slot<4;slot++)(void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(183+slot),jx_word(c,0x9332));
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a));
    skill_panel_text(c,20,140,0x05e7);
    JxFarPointer number={(uint16_t)(work.offset+6),work.segment};
    (void)JX_CALL(c,0x1000,0x2a8c,number.offset,number.segment,0x05f2,c->data_segment,jx_word(c,(uint16_t)(0xd04e + actor*68u+0x12)));
    uint16_t cost=jx_word(c,(uint16_t)(skill_panel_label(c,actor,row,column)+0x14));
    uint16_t mp=jx_word(c,(uint16_t)(0xd04e + actor*68u+0x12));
    skill_panel_font(c,jx_signed_word(mp)>=jx_signed_word(cost)?106:47);
    (void)JX_CALL(c,0x1f2a,0x0728,100,140,number.offset,number.segment);
    skill_panel_font(c,80);skill_panel_text(c,20,160,0x05f6);
    cost=jx_word(c,(uint16_t)(skill_panel_label(c,actor,row,column)+0x14));
    (void)JX_CALL(c,0x1000,0x2a8c,number.offset,number.segment,0x0601,c->data_segment,cost);
    (void)JX_CALL(c,0x1f2a,0x0728,100,160,number.offset,number.segment);
    for(uint16_t r=0;r<8;r++)for(uint16_t col=0;col<2;col++)
        skill_panel_text(c,(uint16_t)(136+col*96u),(uint16_t)(r*25u),skill_panel_label(c,actor,r,col));
    skill_panel_font(c,47);
    skill_panel_text(c,(uint16_t)(136+column*96u),(uint16_t)(row*25u),skill_panel_label(c,actor,row,column));
    skill_panel_font(c,80);
}
