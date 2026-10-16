#include "event_engine.h"
/* 1F2A:DC10. Both tests are independent: animation callbacks may change
 * the live scene, occupancy byte, state and frame before the second test.
 * Table offsets use the original 16-bit wrapping address arithmetic. */
static int world_npc_present(JxEventContext *c,uint16_t slot) {
    uint16_t offset=(uint16_t)(0x665a+30u*jx_word(c,0x9334)+slot);
    return jx_ds(c)[offset]==1;
}
static JxWorldUpdateFlow world_npc_checked(JxEventContext *c,int actions,const JxFarPointer *scratch) {
    for(uint16_t slot=0;slot<30;slot++) {
        uint16_t npc=(uint16_t)(0x889c+86u*slot);
        if(world_npc_present(c,slot) && jx_word(c,(uint16_t)(npc+0x1a))==0 &&
            jx_signed_word(jx_word(c,(uint16_t)(npc+0x0e)))>=0) {
            uint16_t period=jx_word(c,(uint16_t)(npc+0x1e));
            if(period==0)return JX_WORLD_NPC_IDLE_DIVIDE_ERROR;
            if(jx_word(c,0x585e)%period==0) {
                if(actions) {
                    JxWorldUpdateFlow flow=scratch?jx_npc_idle_update_cells(c,slot,*scratch):jx_npc_idle_update(c,slot);
                    if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
                } else (void)JX_CALL(c,0x1f2a,0xd076,slot);
                if(scratch) {
                    JxWorldUpdateFlow flow=jx_npc_redraw_background_world(c,(int16_t)slot,*scratch);
                    if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
                } else (void)JX_CALL(c,0x1f2a,0xca66,slot);
            }
        }
        if(world_npc_present(c,slot) && jx_word(c,(uint16_t)(npc+0x1a))==1 &&
            jx_signed_word(jx_word(c,(uint16_t)(npc+0x0e)))>=0) {
            uint16_t period=jx_word(c,(uint16_t)(npc+0x1c));
            if(period==0)return JX_WORLD_NPC_ACTIVE_DIVIDE_ERROR;
            if(jx_word(c,0x585e)%period==0) {
                if(actions) {
                    JxWorldUpdateFlow flow=jx_npc_active_update(c,slot);
                    if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
                } else (void)JX_CALL(c,0x1f2a,0xda5a,slot);
                if(scratch) {
                    JxWorldUpdateFlow flow=jx_npc_redraw_background_world(c,(int16_t)slot,*scratch);
                    if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
                } else (void)JX_CALL(c,0x1f2a,0xca66,slot);
            }
        }
    }
    return JX_WORLD_UPDATE_RETURNED;
}
JxWorldUpdateFlow jx_world_npc_update(JxEventContext *c) {
    return world_npc_checked(c,0,NULL);
}
JxWorldUpdateFlow jx_world_npc_update_actions(JxEventContext *c) {
    return world_npc_checked(c,1,NULL);
}
JxWorldUpdateFlow jx_world_npc_update_cells(JxEventContext *c,JxFarPointer scratch) {
    return world_npc_checked(c,1,&scratch);
}
