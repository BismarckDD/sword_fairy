#include "event_engine.h"
/* 2F28:87D6. Signed map bounds precede the scene query. The scene
 * query writes the shared nine-layer table; its return AX is ignored. */
uint16_t jx_player_cell_status(JxEventContext *c,uint16_t x,uint16_t y) {
    if(jx_signed_word(x)>=jx_signed_word(jx_word(c,0x663a)) ||
        jx_signed_word(y)>=jx_signed_word(jx_word(c,0x663c)) ||
        jx_signed_word(x)<0 || jx_signed_word(y)<2)return 2;
    (void)JX_CALL(c,0x2f28,0x4d12,
        (uint16_t)(x-jx_word(c,0x8896)),
        (uint16_t)(y-jx_word(c,0x8898)));
    if(jx_signed_word(jx_word(c,0x9388))>=0)return 2;
    if(jx_word(c,0x050a)!=0)return 0;
    for(uint16_t page=0;page<4;page++)
        (void)JX_CALL(c,0x1503,0x007d,page,(uint16_t)(63+page),jx_word(c,0x9332));
    uint16_t offset=(uint16_t)(jx_word(c,0x5858)+2u*x+2u*y*jx_word(c,0x663a));
    uint16_t tile=jx_far_word(c,jx_word(c,0x585a),offset);
    return (uint16_t)(jx_signed_word(tile)>=0);
}
/* Same87D6 with actual4D12 and typed DIV propagation. On a fault the
 * result pointer is untouched: the guest never reached its normal return. */
JxWorldUpdateFlow jx_player_cell_status_collision(JxEventContext *c,
        uint16_t x,uint16_t y,JxFarPointer scratch,uint16_t *result) {
    if(jx_signed_word(x)>=jx_signed_word(jx_word(c,0x663a)) ||
        jx_signed_word(y)>=jx_signed_word(jx_word(c,0x663c)) ||
        jx_signed_word(x)<0 || jx_signed_word(y)<2) {
        *result=2;return JX_WORLD_UPDATE_RETURNED;
    }
    JxWorldUpdateFlow flow=jx_scene_collision_layers(c,
        (uint16_t)(x-jx_word(c,0x8896)),
        (uint16_t)(y-jx_word(c,0x8898)),scratch);
    if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
    if(jx_signed_word(jx_word(c,0x9388))>=0) {
        *result=2;return JX_WORLD_UPDATE_RETURNED;
    }
    if(jx_word(c,0x050a)!=0) {
        *result=0;return JX_WORLD_UPDATE_RETURNED;
    }
    for(uint16_t page=0;page<4;page++)
        (void)JX_CALL(c,0x1503,0x007d,page,(uint16_t)(63+page),jx_word(c,0x9332));
    uint16_t offset=(uint16_t)(jx_word(c,0x5858)+2u*x+2u*y*jx_word(c,0x663a));
    *result=(uint16_t)(jx_signed_word(jx_far_word(c,jx_word(c,0x585a),offset))>=0);
    return JX_WORLD_UPDATE_RETURNED;
}
