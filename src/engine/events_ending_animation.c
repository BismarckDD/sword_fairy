#include "event_engine.h"
static void ending_map(JxEventContext *c,uint16_t first) {
    for(uint16_t slot=0;slot<4;slot++)(void)JX_CALL(c,0x1503,0x7d,slot,(uint16_t)(first+slot),jx_word(c,0x9332));
}
static uint16_t ending_next(JxEventContext *c){return (uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x3e54)+1))%2);}
static void ending_draw(JxEventContext *c){(void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a));}
static void ending_clear(JxEventContext *c){
    (void)JX_CALL(c,0x2f28,0x3f66,0x2eb4,c->data_segment,1);
    for(uint16_t page=0;page<2;page++){
        (void)JX_CALL(c,0x1832,0x1810,page);(void)JX_CALL(c,0x1832,0x1884,0,192,320,8,0);
    }
    (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
}
/* 2F28:3B8E. Cache the ANM frames in EMS, then alternate frames 0/1
 * every five timer events. Tick 281 or key-down fades and returns. */
void jx_ending_animation(JxEventContext *c,JxFarPointer filename,JxFarPointer event_work){
    ending_map(c,179);
    if((uint16_t)JX_CALL(c,0x1000,0x4e32,filename.offset,filename.segment,0x3542,c->data_segment)!=0)return;
    (void)JX_CALL(c,0x1000,0x3146,0x2eb4,c->data_segment,0x354c,c->data_segment,768);
    for(uint16_t frame=0;frame<jx_word(c,0x3548);frame++){
        ending_map(c,179);
        (void)JX_CALL(c,0x1000,0x4d50,0x3542,c->data_segment,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
        ending_map(c,(uint16_t)(87+4*frame));
        (void)JX_CALL(c,0x1000,0x3146,jx_word(c,0x5858),jx_word(c,0x585a),jx_word(c,0x2eb0),jx_word(c,0x2eb2),64000);
    }
    (void)JX_CALL(c,0x1000,0x4fe0,0x3542,c->data_segment);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    (void)JX_CALL(c,0x1832,0x1810,ending_next(c));ending_draw(c);
    (void)JX_CALL(c,0x1832,0x1836,ending_next(c));
    (void)JX_CALL(c,0x2f28,0x415e,0x2eb4,c->data_segment,1);
    jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
    (void)JX_CALL0(c,0x1f2a,0xc24e);
    uint16_t ticks=0;
    for(;;){
        while((uint16_t)JX_CALL(c,0x1f2a,0xc1f2,event_work.offset,event_work.segment)!=0){}
        uint16_t kind=jx_far_word(c,event_work.segment,event_work.offset);
        if(kind==0){ending_clear(c);return;}
        if(kind!=8)continue;
        ticks++;
        if(jx_signed_word(ticks)>280){ending_clear(c);return;}
        if(jx_signed_word(ticks)%5==0){
            (void)JX_CALL(c,0x1832,0x1810,ending_next(c));
            ending_map(c,(uint16_t)(87+4*(jx_signed_word(ticks)%2)));ending_draw(c);
            (void)JX_CALL(c,0x1832,0x1836,ending_next(c));
            jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
        }
    }
}
