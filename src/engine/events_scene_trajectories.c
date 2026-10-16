#include "event_engine.h"
static int trajectory_direction(uint8_t command) {
    switch(command) {
    case 'U':case 'u':return 0;
    case 'D':case 'd':return 1;
    case 'L':case 'l':return 2;
    case 'R':case 'r':return 3;
    default:return -1;
    }
}
static uint8_t trajectory_command(JxEventContext *c,uint16_t slot,uint16_t step) {
    return jx_far_byte(c,(JxFarPointer){(uint16_t)(jx_word(c,(uint16_t)(slot+4))+step),jx_word(c,(uint16_t)(slot+6))},0);
}
/* 2F28:95AA. Five live script slots: party at511E, then four NPCs.
 * Uppercase directions move; lowercase directions only face/animate.
 * There is no path length or iteration limit: an active script without
 * a zero terminator retains the original looping/wrapping behavior. */
void jx_scene_run_trajectories(JxEventContext *c) {
    uint16_t step=0;
    for(;;) {
        if(jx_word(c,0x5120)) {
            uint8_t command=trajectory_command(c,0x511e,step);
            int direction=trajectory_direction(command);
            if(!command) {
                jx_set_word(c,0x8784,2);jx_set_word(c,0x8786,2);jx_set_word(c,0x8788,2);
                (void)JX_CALL0(c,0x2f28,0x5580);jx_set_word(c,0x5120,0);
            } else if(direction>=0) {
                if(command>='A' && command<='Z')
                    (void)JX_CALL(c,0x2f28,0x8dd6,(uint16_t)direction,(uint16_t)direction);
                else {
                    jx_set_word(c,0x8764,(uint16_t)direction);
                    (void)JX_CALL0(c,0x2f28,0x8c6e);
                    (void)JX_CALL0(c,0x2f28,0x5166);
                }
            }
        }
        for(uint16_t index=1;index<5;index++) {
            uint16_t slot=(uint16_t)(0x511e + index*8u);
            if(!jx_word(c,(uint16_t)(slot+2))) continue;
            uint8_t command=trajectory_command(c,slot,step);
            uint16_t npc=jx_word(c,slot),record=(uint16_t)(0x889c+npc*86u);
            int direction=trajectory_direction(command);
            if(!command) {
                uint16_t shape=(uint16_t)(0x31ce + (npc+3u)*18u);
                jx_set_word(c,(uint16_t)(record+62),jx_word(c,(uint16_t)(shape+4))==3?0:2);
                (void)JX_CALL(c,0x1f2a,0xca66,npc);
                jx_set_word(c,(uint16_t)(slot+2),0);
            } else if(direction>=0) {
                if(command>='A' && command<='Z') {
                    uint16_t at=(uint16_t)(record+(direction<2?66:64));
                    uint16_t old=jx_word(c,at);
                    jx_set_word(c,at,(uint16_t)(old+(direction==0 || direction==2?-1:1)));
                }
                jx_set_word(c,(uint16_t)(record+68),(uint16_t)direction);
                jx_set_word(c,(uint16_t)(record+62),(uint16_t)(jx_word(c,(uint16_t)(record+62))+1));
                (void)JX_CALL(c,0x1f2a,0xca66,npc);
            }
        }
        unsigned active=0;
        for(uint16_t index=0;index<5;index++)
            if(jx_word(c,(uint16_t)(0x5120+index*8u))) active=1;
        if(!active) return;
        step++;
        (void)JX_CALL(c,0x1f2a,0x2dbc,2);
    }
}
