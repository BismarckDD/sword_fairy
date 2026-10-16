#include "event_engine.h"
/* 2F28:A332. World frame counter wraps before every update. Read the
 * active scene's far callback after the first two updates, never cache it
 * on entry: either update may change it. No null check exists in the EXE. */
void jx_world_tick(JxEventContext *c) {
    jx_set_word(c,0x585e,(uint16_t)(jx_word(c,0x585e)+1));
    (void)JX_CALL0(c,0x2f28,0x8fd4);
    (void)JX_CALL0(c,0x2f28,0x99b6);
    uint16_t offset=jx_word(c,0x873e),segment=jx_word(c,0x8740);
    (void)JX_CALL0(c,segment,offset);
    (void)JX_CALL0(c,0x2f28,0x9b04);
    (void)JX_CALL0(c,0x2f28,0x9b54);
    (void)JX_CALL0(c,0x1f2a,0xdc10);
    (void)JX_CALL0(c,0x2f28,0xa0fc);
    (void)JX_CALL0(c,0x2f28,0xa1fa);
    (void)JX_CALL0(c,0x2f28,0xa2e6);
}
/* Same A332 guest body with the two recovered state updates called as
 * typed C flows. Virtual INT0 stops all remaining updates. Host scheduling
 * reads the flow rather than interpreting undefined return AX from A332. */
static JxWorldUpdateFlow world_tick_checked(JxEventContext *c,const JxFarPointer *scratch) {
    jx_set_word(c,0x585e,(uint16_t)(jx_word(c,0x585e)+1));
    JxWorldUpdateFlow flow=scratch?jx_world_player_move_collision(c,*scratch):jx_world_player_move(c);
    if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
    flow=scratch?jx_world_idle_redraw_cells(c,*scratch):jx_world_idle_redraw(c);
    if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
    uint16_t offset=jx_word(c,0x873e),segment=jx_word(c,0x8740);
    (void)JX_CALL0(c,segment,offset);
    (void)JX_CALL0(c,0x2f28,0x9b04);
    flow=scratch?jx_world_background_cycle_cells(c,*scratch):jx_world_background_cycle(c);
    if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
    flow=scratch?jx_world_npc_update_cells(c,*scratch):jx_world_npc_update(c);
    if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
    (void)JX_CALL0(c,0x2f28,0xa0fc);
    (void)JX_CALL0(c,0x2f28,0xa1fa);
    (void)JX_CALL0(c,0x2f28,0xa2e6);
    return JX_WORLD_UPDATE_RETURNED;
}
JxWorldUpdateFlow jx_world_tick_state(JxEventContext *c) {
    return world_tick_checked(c,NULL);
}
JxWorldUpdateFlow jx_world_tick_collision(JxEventContext *c,JxFarPointer scratch) {
    return world_tick_checked(c,&scratch);
}
