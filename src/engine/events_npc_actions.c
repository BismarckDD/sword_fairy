/* Verified NPC action bodies; action names describe
 * observed state fields, not recovered original source names. */
#include "event_engine.h"
static uint16_t npc_at(uint16_t base,uint16_t field) {return (uint16_t)(base+field);}
static void npc_increment(JxEventContext *c,uint16_t field) {
    jx_set_word(c,field,(uint16_t)(jx_word(c,field)+1));
}
/* Positive means moved, zero blocked, negative is a typed guest fault.
 * A collision fault must precede the coordinate write and path advancement. */
static int npc_step(JxEventContext *c,uint16_t npc,uint16_t direction,uint16_t width,const JxFarPointer *scratch) {
    uint16_t x=jx_word(c,npc_at(npc,0x40)),y=jx_word(c,npc_at(npc,0x42));
    uint16_t target_x=x,target_y=y;
    if(direction==0)target_y=(uint16_t)(y-1);
    if(direction==1)target_y=(uint16_t)(y+1);
    if(direction==2)target_x=(uint16_t)(x-1);
    if(direction==3)target_x=(uint16_t)(x+width);
    uint16_t result;
    if(scratch) {
        JxWorldUpdateFlow flow=jx_movement_classify_cells(c,target_x,target_y,*scratch,&result);
        if(flow!=JX_WORLD_UPDATE_RETURNED)return -(int)flow;
    } else result=(uint16_t)JX_CALL(c,0x2f28,0x86ce,target_x,target_y);
    if(result!=0)return 0;
    uint16_t coordinate=npc_at(npc,direction<2?0x42:0x40);
    int step=(direction==0 || direction==2)?-1:1;
    jx_set_word(c,coordinate,(uint16_t)(jx_word(c,coordinate)+step));return 1;
}
/* 1F2A:DA5A. Sentinel999 suppresses even the signed DIV. Cached descriptor
 * frame counts remain local; all NPC fields after chance callbacks are live. */
JxWorldUpdateFlow jx_npc_active_update(JxEventContext *c,uint16_t slot) {
    uint16_t npc=(uint16_t)(0x889c+86u*slot),descriptor=(uint16_t)(0x31ce + 18u*slot);
    if(jx_word(c,npc_at(npc,0x1c))==999)return JX_WORLD_UPDATE_RETURNED;
    int divisor=jx_signed_word(jx_word(c,npc_at(descriptor,6)));
    int idle_frames=jx_signed_word(jx_word(c,npc_at(descriptor,4)));
    int frame=jx_signed_word(jx_word(c,npc_at(npc,0x3c)));
    if(divisor==0 || (frame==-32768 && divisor==-1))return JX_WORLD_NPC_ACTIVE_FRAME_DIVIDE_ERROR;
    if(frame%divisor==0) {
        if(jx_word(c,npc_at(npc,0x3c))!=0 && idle_frames>0 &&
            (uint16_t)JX_CALL(c,0x1f2a,0x71de,jx_word(c,npc_at(npc,0x24)))==1) {
            jx_set_word(c,npc_at(npc,0x1a),0);jx_set_word(c,npc_at(npc,0x3e),0);
        }
        if(jx_word(c,npc_at(npc,0x3c))!=0 &&
            (uint16_t)JX_CALL(c,0x1f2a,0x71de,jx_word(c,npc_at(npc,0x22)))==1)
            return JX_WORLD_UPDATE_RETURNED;
    }
    npc_increment(c,npc_at(npc,0x3c));return JX_WORLD_UPDATE_RETURNED;
}
/* 1F2A:D076. IDIV occurs before activation chance and the six-mode switch. */
static JxWorldUpdateFlow npc_idle_update(JxEventContext *c,uint16_t slot,const JxFarPointer *scratch) {
    uint16_t npc=(uint16_t)(0x889c+86u*slot),descriptor=(uint16_t)(0x31ce + 18u*slot);
    int active_frames=jx_signed_word(jx_word(c,npc_at(descriptor,6)));
    int divisor=jx_signed_word(jx_word(c,npc_at(descriptor,4)));
    uint16_t width=jx_word(c,descriptor);
    int frame=jx_signed_word(jx_word(c,npc_at(npc,0x3e)));
    if(divisor==0 || (frame==-32768 && divisor==-1))return JX_WORLD_NPC_IDLE_FRAME_DIVIDE_ERROR;
    int remainder=frame%divisor;
    if(jx_word(c,npc_at(npc,0x3e))!=0 && active_frames>0 && remainder==0 &&
        (uint16_t)JX_CALL(c,0x1f2a,0x71de,jx_word(c,npc_at(npc,0x20)))==1) {
        jx_set_word(c,npc_at(npc,0x1a),1);jx_set_word(c,npc_at(npc,0x3c),0);
        return JX_WORLD_UPDATE_RETURNED;
    }
    uint16_t mode=jx_word(c,npc_at(npc,0x10));
    if(mode==1 || mode==4) {
        int direction=jx_signed_word((uint16_t)JX_CALL0(c,0x1000,0x32b2))%5-1;
        if(direction>=0)jx_set_word(c,npc_at(npc,0x44),(uint16_t)direction);
        npc_increment(c,npc_at(npc,0x3e));
        if(direction<0)return JX_WORLD_UPDATE_RETURNED;
        if(mode==4) {
            uint16_t coordinate=npc_at(npc,direction<2?0x42:0x40);
            uint16_t center=npc_at(npc,direction<2?0x16:0x14);
            uint16_t limit=npc_at(npc,direction<2?0x28:0x26);
            uint16_t distance=(direction==0 || direction==2)?
                (uint16_t)(jx_word(c,center)-jx_word(c,coordinate)):
                (uint16_t)(jx_word(c,coordinate)-jx_word(c,center));
            if(jx_signed_word(distance)>=jx_signed_word(jx_word(c,limit)))return JX_WORLD_UPDATE_RETURNED;
        }
        int step=npc_step(c,npc,(uint16_t)direction,width,scratch);
        if(step<0)return (JxWorldUpdateFlow)-step;
    } else if(mode==2 || mode==3) {
        npc_increment(c,npc_at(npc,0x3e));
        uint16_t direction=jx_word(c,npc_at(npc,0x12));
        direction=mode==2?(direction==2?2:3):(direction==0?0:1);
        uint16_t coordinate=npc_at(npc,mode==2?0x40:0x42);
        uint16_t center=npc_at(npc,mode==2?0x14:0x16);
        uint16_t limit=npc_at(npc,mode==2?0x26:0x28);
        uint16_t distance=(direction==0 || direction==2)?
            (uint16_t)(jx_word(c,center)-jx_word(c,coordinate)):
            (uint16_t)(jx_word(c,coordinate)-jx_word(c,center));
        if(jx_signed_word(distance)<jx_signed_word(jx_word(c,limit))) {
            jx_set_word(c,npc_at(npc,0x44),direction);
            int step=npc_step(c,npc,direction,width,scratch);
            if(step<0)return (JxWorldUpdateFlow)-step;
        } else {
            direction=(uint16_t)(direction^1u);
            jx_set_word(c,npc_at(npc,0x12),direction);jx_set_word(c,npc_at(npc,0x44),direction);
        }
    } else if(mode==5) {
        uint16_t length=(uint16_t)JX_CALL(c,0x1000,0x24ae,
            jx_word(c,npc_at(npc,0x46)),jx_word(c,npc_at(npc,0x48)));
        if(jx_signed_word(jx_word(c,npc_at(npc,0x3e)))>=jx_signed_word(length)) {
            jx_set_word(c,npc_at(npc,0x10),0);return JX_WORLD_UPDATE_RETURNED;
        }
        JxFarPointer script={jx_word(c,npc_at(npc,0x46)),jx_word(c,npc_at(npc,0x48))};
        uint8_t command=jx_far_byte(c,script,jx_word(c,npc_at(npc,0x3e)));
        if(command=='0')npc_increment(c,npc_at(npc,0x3e));
        else {
            int direction=-1,move=0;
            switch(command) {
                case 'U':move=1;direction=0;break;case 'u':direction=0;break;
                case 'D':move=1;direction=1;break;case 'd':direction=1;break;
                case 'L':move=1;direction=2;break;case 'l':direction=2;break;
                case 'R':move=1;direction=3;break;case 'r':direction=3;break;
                default:break;
            }
            if(direction>=0) {
                jx_set_word(c,npc_at(npc,0x44),(uint16_t)direction);
                int step=move?npc_step(c,npc,(uint16_t)direction,width,scratch):1;
                if(step<0)return (JxWorldUpdateFlow)-step;
                if(step)npc_increment(c,npc_at(npc,0x3e));
            }
        }
    }
    return JX_WORLD_UPDATE_RETURNED;
}
JxWorldUpdateFlow jx_npc_idle_update(JxEventContext *c,uint16_t slot) {
    return npc_idle_update(c,slot,NULL);
}
JxWorldUpdateFlow jx_npc_idle_update_cells(JxEventContext *c,uint16_t slot,JxFarPointer scratch) {
    return npc_idle_update(c,slot,&scratch);
}
