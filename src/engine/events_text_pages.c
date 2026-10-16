#include "event_engine.h"
static JxFarPointer tp_buffer(JxEventContext *c){return (JxFarPointer){jx_word(c,0x2eb0),jx_word(c,0x2eb2)};}
static JxFarPointer tp_add(JxFarPointer p,uint16_t amount){uint32_t off=(uint32_t)p.offset+amount;return (JxFarPointer){(uint16_t)off,(uint16_t)(p.segment+((off>>16)<<12))};}
static void tp_clear(JxEventContext *c,JxFarPointer work){for(uint16_t i=0;i<128;i++)jx_far_set_byte(c,work,i,0);}
static void tp_show(JxEventContext *c,uint16_t x,uint16_t y,uint16_t rows,uint16_t drawn,uint16_t timed,JxFarPointer work){
    uint16_t height=(uint16_t)(rows*20);JxFarPointer source=tp_buffer(c);
    (void)JX_CALL(c,0x1832,0x1e88,x,y,260,height,source.offset,source.segment);
    source=tp_buffer(c);JxFarPointer shaded=tp_add(source,32000);
    (void)JX_CALL(c,0x2f28,0x6c7e,source.offset,source.segment,shaded.offset,shaded.segment,(uint16_t)(height*260),0xb584,c->data_segment);
    shaded=tp_add(tp_buffer(c),32000);(void)JX_CALL(c,0x1832,0x1c6c,x,y,260,height,shaded.offset,shaded.segment);
    uint16_t right=(uint16_t)(x+259),bottom=(uint16_t)(y+height-1);
    (void)JX_CALL(c,0x2f28,0xa616,x,y,right,y,80);(void)JX_CALL(c,0x2f28,0xa616,x,bottom,right,bottom,80);
    (void)JX_CALL(c,0x2f28,0xa616,x,y,x,bottom,80);(void)JX_CALL(c,0x2f28,0xa616,right,y,right,bottom,80);
    for(uint16_t row=0;row<drawn;row++){
        JxFarPointer text={(uint16_t)(work.offset+32*row),work.segment};
        if(timed)(void)JX_CALL(c,0x1f2a,0x0e04,(uint16_t)(x+10),(uint16_t)(y+20*row+2),text.offset,text.segment,4);
        else (void)JX_CALL(c,0x1f2a,0x0728,(uint16_t)(x+10),(uint16_t)(y+20*row+2),text.offset,text.segment);
    }
    (void)JX_CALL(c,0x1f2a,0x72b2,90);
    source=tp_buffer(c);(void)JX_CALL(c,0x1832,0x1c6c,x,y,260,height,source.offset,source.segment);
}
/* 2F28:6572. Four 32-byte row buffers, 30-byte lines, shaded/outlined
 * message pages and optional double-byte character timing. Caller work128
 * is private and disjoint from resources and live game state. */
int jx_text_pages(JxEventContext *c,uint16_t x,uint16_t y,uint16_t rows,JxFarPointer text,uint16_t timed,JxFarPointer work){
    for(uint16_t slot=0;slot<4;slot++)if((uint16_t)JX_CALL(c,0x1503,0x7d,slot,(uint16_t)(175+slot),jx_word(c,0x9332)))return 0;
    uint16_t length=(uint16_t)JX_CALL(c,0x1000,0x24ae,text.offset,text.segment),width=(uint16_t)(rows*30);
    if(!width)return 1;
    int len=jx_signed_word(length),divisor=jx_signed_word(width);if(len==-32768 && divisor==-1)return 1;
    uint16_t pages=(uint16_t)(len/divisor),remainder=(uint16_t)(length-(uint16_t)(width*pages));
    uint16_t tail_rows=(uint16_t)(jx_signed_word(remainder)/30),tail=(uint16_t)(remainder-tail_rows*30);
    tp_clear(c,work);
    for(uint16_t page=0;jx_signed_word(page)<jx_signed_word(pages);page++){
        for(uint16_t row=0;jx_signed_word(row)<jx_signed_word(rows);row++)for(uint16_t ch=0;ch<30;ch++){
            jx_far_set_byte(c,work,(uint16_t)(row*32+ch),jx_far_byte(c,text,(uint16_t)(width*page+row*30+ch)));
            jx_far_set_byte(c,work,(uint16_t)(row*32+31),0);
        }
        tp_show(c,x,y,rows,rows,timed,work);tp_clear(c,work);
    }
    if(!tail_rows && !tail)return 0;
    for(uint16_t row=0;jx_signed_word(row)<jx_signed_word(tail_rows);row++)for(uint16_t ch=0;ch<30;ch++){
        jx_far_set_byte(c,work,(uint16_t)(row*32+ch),jx_far_byte(c,text,(uint16_t)(width*pages+row*30+ch)));
        jx_far_set_byte(c,work,(uint16_t)(row*32+31),0);
    }
    for(uint16_t ch=0;jx_signed_word(ch)<jx_signed_word(tail);ch++)jx_far_set_byte(c,work,(uint16_t)(tail_rows*32+ch),jx_far_byte(c,text,(uint16_t)(width*pages+tail_rows*30+ch)));
    jx_far_set_byte(c,work,(uint16_t)(tail_rows*32+tail),0);
    tp_show(c,x,y,rows,(uint16_t)(tail_rows+1),timed,work);return 0;
}
