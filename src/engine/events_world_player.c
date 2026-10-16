#include "event_engine.h"
/* 2F28:8FD4. Each collision call takes freshly read player coordinates.
 * Direction is captured once; the frame period DIV precedes input querying. */
static const int player_dx[4]={0,0,-1,1};
static const int player_dy[4]={-1,1,0,0};
static uint16_t player_query(JxEventContext *c,int dx,int dy) {
    return (uint16_t)JX_CALL(c,0x2f28,0x87d6,
        (uint16_t)(jx_word(c,0x8760)+dx),
        (uint16_t)(jx_word(c,0x8762)+dy));
}
JxWorldUpdateFlow jx_world_player_move(JxEventContext *c) {
    uint16_t period=jx_word(c,0x5860);
    if(period==0)return JX_WORLD_PLAYER_DIVIDE_ERROR;
    if(jx_word(c,0x585e)%period!=0)return JX_WORLD_UPDATE_RETURNED;
    uint16_t direction=(uint16_t)JX_CALL0(c,0x1f2a,0x6e62);
    if(direction>3)return JX_WORLD_UPDATE_RETURNED;
    int dx=player_dx[direction],dy=player_dy[direction];
    if(player_query(c,dx,dy)==0) {
        (void)JX_CALL(c,0x2f28,0x8dd6,direction,direction);
        return JX_WORLD_UPDATE_RETURNED;
    }
    static const uint16_t sides[4][2]={{2,3},{3,2},{1,0},{0,1}};
    for(unsigned i=0;i<2;i++) {
        uint16_t side=sides[direction][i];
        int sx=player_dx[side],sy=player_dy[side];
        if(player_query(c,dx,dy)==1 && player_query(c,sx,sy)==0 &&
            player_query(c,dx+sx,dy+sy)==0) {
            (void)JX_CALL(c,0x2f28,0x8dd6,direction,side);
            return JX_WORLD_UPDATE_RETURNED;
        }
    }
    jx_set_word(c,0x8764,direction);
    (void)JX_CALL0(c,0x2f28,0x8c6e);
    (void)JX_CALL0(c,0x2f28,0x5166);
    (void)JX_CALL0(c,0x2f28,0x88de);
    return JX_WORLD_UPDATE_RETURNED;
}
static JxWorldUpdateFlow player_query_collision(JxEventContext *c,
        JxFarPointer scratch,int dx,int dy,uint16_t *result) {
    return jx_player_cell_status_collision(c,
        (uint16_t)(jx_word(c,0x8760)+dx),
        (uint16_t)(jx_word(c,0x8762)+dy),scratch,result);
}
/* Checked8FD4 executes the actual collision chain. Each query may halt
 * inside a NPC cell DIV; no subsequent query/movement/redraw then executes. */
JxWorldUpdateFlow jx_world_player_move_collision(JxEventContext *c,JxFarPointer scratch) {
    uint16_t period=jx_word(c,0x5860);
    if(period==0)return JX_WORLD_PLAYER_DIVIDE_ERROR;
    if(jx_word(c,0x585e)%period!=0)return JX_WORLD_UPDATE_RETURNED;
    uint16_t direction=(uint16_t)JX_CALL0(c,0x1f2a,0x6e62);
    if(direction>3)return JX_WORLD_UPDATE_RETURNED;
    int dx=player_dx[direction],dy=player_dy[direction];uint16_t status;
    JxWorldUpdateFlow flow=player_query_collision(c,scratch,dx,dy,&status);
    if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
    if(status==0) {
        return jx_party_move_cycle_world(c,direction,direction,scratch);
    }
    static const uint16_t sides[4][2]={{2,3},{3,2},{1,0},{0,1}};
    for(unsigned i=0;i<2;i++) {
        uint16_t side=sides[direction][i];int sx=player_dx[side],sy=player_dy[side];
        flow=player_query_collision(c,scratch,dx,dy,&status);
        if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
        if(status!=1)continue;
        flow=player_query_collision(c,scratch,sx,sy,&status);
        if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
        if(status!=0)continue;
        flow=player_query_collision(c,scratch,dx+sx,dy+sy,&status);
        if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
        if(status!=0)continue;
        return jx_party_move_cycle_world(c,direction,side,scratch);
    }
    jx_set_word(c,0x8764,direction);
    jx_party_advance_frames(c);
    flow=jx_party_redraw_trail_world(c,scratch);
    if(flow!=JX_WORLD_UPDATE_RETURNED)return flow;
    return jx_party_check_triggers_world(c,scratch);
}
