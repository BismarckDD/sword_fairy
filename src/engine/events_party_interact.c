#include "event_engine.h"

static uint16_t npc_at(uint16_t npc,uint16_t field) {return (uint16_t)(0x889c+86*npc+field);}
static uint16_t definition_at(uint16_t npc,uint16_t field) {return (uint16_t)(0x31ce + 18*npc+field);}
static uint16_t state_at(JxEventContext *c,uint16_t npc) {return (uint16_t)(0xb686+30*jx_word(c,0x9334)+npc);}

/* 1F2A:799A. Present an item to the first matching NPC in front.
 * Unlike7606 this tests +26 alone for the expanded shape, and retains
 * the new facing after calling the item-specific callback at +78. */
void jx_party_present_item(JxEventContext *c,uint16_t item) {
    uint16_t x=jx_word(c,0x8760),y=jx_word(c,0x8762),facing;
    switch(jx_word(c,0x8764)){
    case 0:y=(uint16_t)(y-1);facing=1;break;
    case 1:y=(uint16_t)(y+1);facing=0;break;
    case 2:x=(uint16_t)(x-1);facing=3;break;
    default:x=(uint16_t)(x+1);facing=2;break;
    }
    for(uint16_t i=0;i<30;i++){
        uint16_t left,top,width,height;
        if(jx_word(c,npc_at(i,26))==0){
            left=(uint16_t)(jx_word(c,npc_at(i,64))-1);top=(uint16_t)(jx_word(c,npc_at(i,66))-1);width=3;height=3;
        }else{
            width=jx_word(c,definition_at(i,0));height=jx_word(c,definition_at(i,2));
            left=jx_word(c,npc_at(i,64));top=(uint16_t)(jx_word(c,npc_at(i,66))-height+1);height=(uint16_t)(height+1);
        }
        if(jx_ds(c)[(uint16_t)(0x665a+30*jx_word(c,0x9334)+i)]!=1)continue;
        uint16_t col,row;
        if(jx_rectangle_local_point(&col,&row,x,y,left,top,jx_signed_word(width),jx_signed_word(height))!=0)continue;
        if((jx_word(c,npc_at(i,78))==0x86c4 && jx_word(c,npc_at(i,80))==0x2f28) ||
           jx_word(c,npc_at(i,42))!=item)continue;
        jx_set_word(c,npc_at(i,62),jx_word(c,definition_at(i,4))==5?2:0);
        jx_set_word(c,npc_at(i,68),facing);(void)JX_CALL(c,0x1f2a,0xca66,i);
        (void)JX_CALL(c,jx_word(c,npc_at(i,80)),jx_word(c,npc_at(i,78)),i);
        (void)JX_CALL(c,0x1f2a,0xc24e);return;
    }
}

/* 1F2A:7606. Space interacts with the cell in front of the leader.
 * Object pickup and NPC interaction are both attempted. Callback changes
 * remain live; captured facing is restored only after the NPC callback. */
void jx_party_interact(JxEventContext *c) {
    uint16_t x=jx_word(c,0x8760),y=jx_word(c,0x8762),facing;
    switch(jx_word(c,0x8764)) {
    case 0:y=(uint16_t)(y-1);facing=1;break;
    case 1:y=(uint16_t)(y+1);facing=0;break;
    case 2:x=(uint16_t)(x-1);facing=3;break;
    default:x=(uint16_t)(x+1);facing=2;break;
    }
    for(uint16_t i=0;i<10;i++) {
        uint16_t object=(uint16_t)(0x92b8+10*i);
        if(jx_word(c,object)!=x || jx_word(c,(uint16_t)(object+2))!=y ||
           jx_ds(c)[(uint16_t)(0xacec+10*jx_word(c,0x9334)+i)]!=0)continue;
        (void)JX_CALL(c,0x1f2a,0x5fd8,75,6);(void)JX_CALL(c,0x1f2a,0x155c,6);
        jx_ds(c)[(uint16_t)(0xacec+10*jx_word(c,0x9334)+i)]=1;
        (void)JX_CALL(c,0x2f28,0x4fdc,(uint16_t)(x-jx_word(c,0x8896)),(uint16_t)(y-jx_word(c,0x8898)));
        uint16_t item=jx_word(c,(uint16_t)(object+6));
        if(item)(void)JX_CALL(c,0x1f2a,0x8cea,item,jx_word(c,(uint16_t)(object+8)));
        break;
    }
    for(uint16_t i=0;i<30;i++) {
        uint16_t left,top,width,height;
        if(jx_word(c,npc_at(i,26))==0 && jx_signed_word(jx_word(c,npc_at(i,16)))>0) {
            left=(uint16_t)(jx_word(c,npc_at(i,64))-1);top=(uint16_t)(jx_word(c,npc_at(i,66))-1);width=3;height=3;
        } else {
            width=jx_word(c,definition_at(i,0));height=jx_word(c,definition_at(i,2));
            left=jx_word(c,npc_at(i,64));top=(uint16_t)(jx_word(c,npc_at(i,66))-height+1);height=(uint16_t)(height+1);
        }
        if(jx_ds(c)[(uint16_t)(0x665a+30*jx_word(c,0x9334)+i)]!=1)continue;
        uint16_t column,row;
        if(jx_rectangle_local_point(&column,&row,x,y,left,top,(int16_t)jx_signed_word(width),(int16_t)jx_signed_word(height))!=0)continue;
        uint16_t old_facing=jx_word(c,npc_at(i,68));
        jx_set_word(c,npc_at(i,62),jx_word(c,definition_at(i,4))==5?2:0);
        if(jx_word(c,npc_at(i,28))!=999)jx_set_word(c,npc_at(i,60),0);
        jx_set_word(c,npc_at(i,68),facing);(void)JX_CALL(c,0x1f2a,0xca66,i);
        int state=jx_signed_byte(jx_ds(c)[state_at(c,i)]);
        uint16_t text=jx_word(c,(uint16_t)(npc_at(i,44)+2*state));
        if(jx_signed_word(text)>=0) {
            uint16_t pointer=(uint16_t)(0x59d6+4*text);
            (void)JX_CALL(c,0x2f28,0x648e,i,jx_word(c,pointer),jx_word(c,(uint16_t)(pointer+2)),0);
        }
        uint16_t callback=npc_at(i,74);
        (void)JX_CALL(c,jx_word(c,(uint16_t)(callback+2)),jx_word(c,callback),i);
        state=jx_signed_byte(jx_ds(c)[state_at(c,i)]);
        if(state<4) {uint16_t at=state_at(c,i);jx_ds(c)[at]=(uint8_t)(jx_ds(c)[at]+1);}
        jx_set_word(c,npc_at(i,68),old_facing);(void)JX_CALL(c,0x1f2a,0xca66,i);
        (void)JX_CALL0(c,0x1f2a,0xc24e);return;
    }
}
