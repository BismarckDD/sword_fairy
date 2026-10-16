#include "event_engine.h"

/* 2F28:0B22. Nine-actor trap animation followed by mandatory reload. */
void jx_callback_2f28_0b22(JxEventContext *c) {
    (void)JX_CALL(c,0x2f28,0x648e,0xffff,0x0944,c->data_segment,0);
    for(unsigned i=0;i<9;++i)jx_ds(c)[0x7488+i]=1;
    for(uint16_t frame=0;frame<3;++frame){
        for(unsigned npc=0;npc<9;++npc)jx_set_word(c,(uint16_t)(0x889c+86*npc+60),frame);
        (void)JX_CALL0(c,0x2f28,0x5580);
    }
    (void)JX_CALL(c,0x1f2a,0x5fd8,36,6);
    (void)JX_CALL(c,0x1f2a,0x155c,6);
    (void)JX_CALL(c,0x2f28,0x648e,0xffff,0x094d,c->data_segment,0);
    (void)JX_CALL(c,0x1f2a,0x2dbc,20);
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
    while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0){}
}

/* 2F28:1FC4. The current map is read after battle returns. */
void jx_callback_2f28_1fc4(JxEventContext *c,uint16_t npc) {
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,27,0,4)==1){
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0){}
        return;
    }
    jx_ds(c)[(uint16_t)(0x665a+30*jx_word(c,0x9334)+npc)]=0;
    (void)JX_CALL0(c,0x2f28,0x5580);
}

/* 2F28:32D8. Dynamic map213 movement callback, including defeat reload. */
void jx_callback_2f28_32d8(JxEventContext *c,uint16_t npc) {
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,34,0,1)==1){
        while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0){}
        return;
    }
    jx_ds(c)[(uint16_t)(0x7f50+npc)]=0;
    (void)JX_CALL0(c,0x2f28,0x5580);
}
