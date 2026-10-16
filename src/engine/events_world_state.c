#include "event_engine.h"
/* 2F28:99B6. Stop at the original DIV fault after querying direction.
 * Later scene redraw can mutate any game state; no values are cached across it. */
static JxWorldUpdateFlow world_idle_redraw(JxEventContext *c,const JxFarPointer *scratch) {
    if(jx_word(c,0x9364)!=0)return JX_WORLD_UPDATE_RETURNED;
    if(jx_signed_word((uint16_t)JX_CALL0(c,0x1f2a,0x6e62))>=0)
        return JX_WORLD_UPDATE_RETURNED;
    uint16_t period=jx_word(c,0x6658);
    if(period==0)return JX_WORLD_IDLE_DIVIDE_ERROR;
    if(jx_word(c,0x585e)%period!=0)return JX_WORLD_UPDATE_RETURNED;
    if(jx_word(c,0x8784)==2 && jx_word(c,0x8786)==2 && jx_word(c,0x8788)==2)
        return JX_WORLD_UPDATE_RETURNED;
    jx_set_word(c,0x8784,2);jx_set_word(c,0x8786,2);jx_set_word(c,0x8788,2);
    if(scratch)return jx_scene_redraw_world(c,*scratch);
    (void)JX_CALL0(c,0x2f28,0x5580);
    return JX_WORLD_UPDATE_RETURNED;
}
/* 2F28:9B54. Counter wraps as a word; EMS page selection is signed%16,
 * separately re-read after redraw and after each mapping callback. */
static JxWorldUpdateFlow world_background_cycle(JxEventContext *c,const JxFarPointer *scratch) {
    uint16_t mode=jx_word(c,0x9364);
    if(mode==0 || mode==5)return JX_WORLD_UPDATE_RETURNED;
    if(jx_signed_word((uint16_t)JX_CALL0(c,0x1f2a,0x6e62))>=0)
        return JX_WORLD_UPDATE_RETURNED;
    uint16_t period=jx_word(c,0x0e0e);
    if(period==0)return JX_WORLD_BACKGROUND_DIVIDE_ERROR;
    if(jx_word(c,0x585e)%period!=0)return JX_WORLD_UPDATE_RETURNED;
    jx_set_word(c,0x0e1a,(uint16_t)(jx_word(c,0x0e1a)+1));
    if(scratch) {
        JxWorldUpdateFlow flow=jx_scene_redraw_world(c,*scratch);
        if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
    } else (void)JX_CALL0(c,0x2f28,0x5580);
    for(uint16_t slot=0;slot<4;slot++) {
        int remainder=jx_signed_word(jx_word(c,0x0e1a))%16;
        (void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(87+slot+4*remainder),
            jx_word(c,0x9332));
    }
    (void)JX_CALL(c,0x1832,0x1a28,0,0,320,192,jx_word(c,0x5858),
        jx_word(c,0x585a),255);
    return JX_WORLD_UPDATE_RETURNED;
}
JxWorldUpdateFlow jx_world_idle_redraw(JxEventContext *c) {return world_idle_redraw(c,NULL);}
JxWorldUpdateFlow jx_world_idle_redraw_cells(JxEventContext *c,JxFarPointer scratch) {return world_idle_redraw(c,&scratch);}
JxWorldUpdateFlow jx_world_background_cycle(JxEventContext *c) {return world_background_cycle(c,NULL);}
JxWorldUpdateFlow jx_world_background_cycle_cells(JxEventContext *c,JxFarPointer scratch) {return world_background_cycle(c,&scratch);}
