#include "event_engine.h"
/* 2F28:88DE. Priority: first eligible exit, first location event, then first
 * eligible nearby NPC. Dynamic event targets remain real required calls. */
static JxWorldUpdateFlow party_check_triggers(JxEventContext *c,const JxFarPointer *scratch) {
    for(uint16_t i=0;i<50;i++) {
        uint16_t exit=(uint16_t)(0x3e56+16*i);
        if(jx_word(c,0x8760)!=jx_word(c,exit) || jx_word(c,0x8762)!=jx_word(c,(uint16_t)(exit+2)) ||
           jx_word(c,0x8764)!=jx_word(c,(uint16_t)(exit+4))) continue;
        if(!jx_ds(c)[(uint16_t)(0x3466+jx_word(c,(uint16_t)(exit+6)))]) continue;
        if(!jx_word(c,(uint16_t)(exit+8))) (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
        for(uint16_t sample=0;sample<6;sample++) for(uint16_t field=0;field<6;field+=2)
            jx_set_word(c,(uint16_t)(0x8760+6*sample+field),jx_word(c,(uint16_t)(exit+10+field)));
        jx_set_word(c,0x9334,jx_word(c,(uint16_t)(exit+6)));
        (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),jx_word(c,(uint16_t)(exit+8)));return JX_WORLD_UPDATE_RETURNED;
    }
    for(uint16_t i=0;i<10;i++) {
        uint16_t event=(uint16_t)(0x6584+8*i);
        if(jx_word(c,0x8760)!=jx_word(c,event) || jx_word(c,0x8762)!=jx_word(c,(uint16_t)(event+2))) continue;
        (void)JX_CALL(c,jx_word(c,(uint16_t)(event+6)),jx_word(c,(uint16_t)(event+4)));
        (void)JX_CALL(c,0x1f2a,0xc24e);return JX_WORLD_UPDATE_RETURNED;
    }
    uint16_t facing=jx_word(c,0x8764),opposite=facing==0?1:facing==1?0:facing==2?3:2;
    for(uint16_t i=0;i<30;i++) {
        uint16_t actor=(uint16_t)(0x889c+86*i),descriptor=(uint16_t)(0x31ce + 18*i);
        uint16_t left,top,width,height,column,row;
        if(!jx_word(c,(uint16_t)(actor+0x1a))) {
            left=(uint16_t)(jx_word(c,(uint16_t)(actor+0x40))-2);top=(uint16_t)(jx_word(c,(uint16_t)(actor+0x42))-2);width=height=5;
        } else {
            width=jx_word(c,descriptor);height=jx_word(c,(uint16_t)(descriptor+2));
            left=(uint16_t)(jx_word(c,(uint16_t)(actor+0x40))-1);top=(uint16_t)(jx_word(c,(uint16_t)(actor+0x42))-height);
            width+=2;height+=2;
        }
        if(jx_ds(c)[(uint16_t)(0x665a+30*jx_word(c,0x9334)+i)]!=1) continue;
        if(jx_rectangle_local_point(&column,&row,jx_word(c,0x8760),jx_word(c,0x8762),left,top,
            (int16_t)jx_signed_word(width),(int16_t)jx_signed_word(height))) continue;
        if(jx_word(c,(uint16_t)(actor+0x52))==0x86c4 && jx_word(c,(uint16_t)(actor+0x54))==0x2f28) continue;
        jx_set_word(c,(uint16_t)(actor+0x3e),jx_word(c,(uint16_t)(descriptor+4))==5?2:0);
        jx_set_word(c,(uint16_t)(actor+0x44),opposite);
        if(scratch) {
            JxWorldUpdateFlow flow=jx_npc_redraw_background_world(c,(int16_t)i,*scratch);
            if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
        } else jx_npc_redraw_background(c,(int16_t)i);
        (void)JX_CALL(c,jx_word(c,(uint16_t)(actor+0x54)),jx_word(c,(uint16_t)(actor+0x52)),i);
        (void)JX_CALL(c,0x1f2a,0xc24e);return JX_WORLD_UPDATE_RETURNED;
    }
    return JX_WORLD_UPDATE_RETURNED;
}
void jx_party_check_triggers(JxEventContext *c) {(void)party_check_triggers(c,NULL);}
JxWorldUpdateFlow jx_party_check_triggers_world(JxEventContext *c,JxFarPointer scratch) {
    return party_check_triggers(c,&scratch);
}
/* 1F2A:C24E. Clear the two-word event state, high word first. */
uint16_t jx_event_clear_state(JxEventContext *c) {
    jx_set_word(c,0x0866,0);jx_set_word(c,0x0864,0);return 0;
}
/* 2F28:86C4 is genuinely an empty original event, not an unresolved stub. */
void jx_event_empty(JxEventContext *c) {(void)c;}
