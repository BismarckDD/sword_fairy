/* 1F2A:8F78 item panel; original arithmetic is expressed directly in C. */
#include "event_engine.h"
#include <string.h>
static uint16_t item_panel_slot(uint16_t row,uint16_t column,uint16_t scroll) {
    return (uint16_t)(0x878a+(((row+scroll)*2u+column)*4u));
}
/* 1F2A:8F78. Reserve30-byte frame; number starts at work+8. */
void jx_item_menu_render(JxEventContext *c,uint16_t column,uint16_t row,uint16_t scroll,JxFarPointer work) {
    /* Restore the original complete backdrop so old text disappears without
     * a solid fill, and the decorative item picture remains intact. */
    for(uint16_t slot=0;slot<4;slot++)
        (void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(79+slot),jx_word(c,0x9332));
    (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a));
    JxFarPointer number={(uint16_t)(work.offset+8),work.segment};
    for(uint16_t r=0;r<10;r++)for(uint16_t col=0;col<2;col++) {
        if(r==row && col==column)(void)JX_CALL(c,0x1f2a,0x06f2,47,0,77);
        uint16_t at=item_panel_slot(r,col,scroll);
        if(jx_word(c,at)!=0) {
            uint16_t label=(uint16_t)(0x8022+jx_word(c,at)*28u);
            (void)JX_CALL(c,0x1f2a,0x0728,(uint16_t)(150+col*80u),(uint16_t)(r*20u),label,c->data_segment);
        }
        (void)JX_CALL(c,0x1000,0x2a8c,number.offset,number.segment,0x05d0,c->data_segment,jx_word(c,(uint16_t)(at+2)));
        if(jx_word(c,at)==0)c->memory[(((uint32_t)number.segment<<4)+number.offset)&0xfffff]=0;
        (void)JX_CALL(c,0x1f2a,0x0728,(uint16_t)(214+col*80u),(uint16_t)(r*20u),number.offset,number.segment);
        (void)JX_CALL(c,0x1f2a,0x06f2,80,0,77);
    }
    uint32_t file=JX_CALL(c,0x1000,0x0738,0x05d7,c->data_segment,0x05d4,c->data_segment);
    int32_t index=jx_signed_word(jx_word(c,item_panel_slot(row,column,scroll)));
    uint32_t seek=(uint32_t)(index*3120);
    (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)file,(uint16_t)(file>>16),(uint16_t)seek,(uint16_t)(seek>>16),0);
    (void)JX_CALL(c,0x1000,0x0754,jx_word(c,0x2eb0),jx_word(c,0x2eb2),1,3120,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)file,(uint16_t)(file>>16));
    (void)JX_CALL(c,0x1832,0x1884,40,29,60,52,255);
    (void)JX_CALL(c,0x1832,0x1a28,40,29,60,52,jx_word(c,0x2eb0),jx_word(c,0x2eb2),255);
    /* Real item descriptions use up to four lines, below the entire picture.
     * Retain room for extended data without clipping text at the screen edge. */
    JxItemDescription description;jx_item_description(c,(uint16_t)index,&description);
    uint16_t text_y=description.count<=4?132:(uint16_t)(198-16*description.count);
    (void)JX_CALL(c,0x1f2a,0x06f2,80,0,77);
    for(unsigned line=0;line<description.count;line++) {
        size_t length=strlen(description.lines[line]);
        for(unsigned k=0;k<=length;k++)jx_far_set_byte(c,work,(uint16_t)k,(uint8_t)description.lines[line][k]);
        (void)JX_CALL(c,0x1f2a,0x0728,8,(uint16_t)(text_y+16*line),work.offset,work.segment);
    }
}
