#include "event_engine.h"

/* Resident 1512:000C: four consecutive final battles. Every losing branch
 * restores the transition before waiting for the original death menu. */
static void final_death(JxEventContext *c){
    while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0){}
}
static void final_text(JxEventContext *c,uint16_t npc,uint16_t ptr,uint16_t mode){
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,ptr),jx_word(c,(uint16_t)(ptr+2)),mode);
}
static void final_move(JxEventContext *c,uint16_t npc,uint16_t ptr){
    (void)JX_CALL(c,0x1f2a,0xcd1c,npc,ptr,c->data_segment,3);
}
static void final_black(JxEventContext *c){
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
    (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);
}
void jx_callback_1512_000c(JxEventContext *c){
    (void)JX_CALL(c,0x2f28,0x56ec,85,4);
    (void)JX_CALL0(c,0x2f28,0x56cc);
    final_text(c,3,0x5b7e,1);
    jx_ds(c)[0xcfb6]=1;
    (void)JX_CALL(c,0x1f2a,0xcc24,55,44);
    (void)JX_CALL(c,0x2f28,0x944c,0x129b,c->data_segment,3);
    final_move(c,3,0x129d);
    final_text(c,3,0x5b82,1);
    const uint16_t flags[]={0x7f6f,0x7f6e,0x7f70,0x7f71};
    for(unsigned i=0;i<4;i++){
        if((uint16_t)JX_CALL(c,0x1f2a,0x4934,i==3?36:38,0,1)==1){
            (void)JX_CALL0(c,0x2f28,0x56dc);final_death(c);return;
        }
        if(i==3)(void)JX_CALL0(c,0x2f28,0x56dc);
        jx_ds(c)[flags[i]]=0;
        (void)JX_CALL0(c,0x2f28,0x5580);
    }
}
/* Resident 1512:016A: CE8B chooses the survivor confrontation or reunion
 * ending. Dialogue pointers and music availability are read at use time. */
void jx_callback_1512_016a(JxEventContext *c){
    if(jx_ds(c)[0xce8b]){
        jx_ds(c)[0x7f79]=1;
        (void)JX_CALL(c,0x1f2a,0x5fd8,77,6);
        (void)JX_CALL(c,0x1f2a,0x155c,6);
        (void)JX_CALL(c,0x1f2a,0xdb46,11);
        jx_ds(c)[0x7f78]=1;jx_ds(c)[0x7f79]=0;
        (void)JX_CALL0(c,0x2f28,0x5580);
        (void)JX_CALL(c,0x2f28,0x944c,0x129f,c->data_segment,3);
        final_text(c,10,0x5bba,1);
        if((uint16_t)JX_CALL(c,0x1f2a,0x4934,37,0,1)==1){final_death(c);return;}
        final_text(c,10,0x5bbe,1);
        (void)JX_CALL(c,0x2f28,0x7a1c,10);
        jx_event_music(c,2,2);
        final_text(c,10,0x5bc2,1);
        (void)JX_CALL(c,0x2f28,0x3b8e,0x12a1,c->data_segment);
        final_black(c);
        final_text(c,0xffff,0x5bc6,2);
    }else{
        final_text(c,4,0x5b8e,1);
        jx_ds(c)[0x7f75]=1;
        (void)JX_CALL(c,0x1f2a,0xcf6a,7,55,43);
        final_move(c,7,0x12ad);
        jx_ds(c)[0x7f76]=1;
        (void)JX_CALL(c,0x1f2a,0xcf6a,8,54,43);
        final_move(c,8,0x12af);
        (void)JX_CALL(c,0x2f28,0x944c,0x12b1,c->data_segment,3);
        final_text(c,7,0x5b9a,1);
        jx_ds(c)[0x7f77]=1;
        (void)JX_CALL(c,0x1f2a,0xcf6a,9,55,44);
        final_move(c,9,0x12b3);final_move(c,7,0x12b5);final_move(c,8,0x12b7);
        final_text(c,9,0x5b9e,1);
        (void)JX_CALL(c,0x2f28,0x7a1c,4);
        (void)JX_CALL(c,0x2f28,0x7a1c,5);
        (void)JX_CALL(c,0x2f28,0x7a1c,6);
        (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
        jx_event_music(c,2,2);
        final_black(c);
        final_text(c,9,0x5ba2,1);final_text(c,9,0x5ba6,1);
        (void)JX_CALL(c,0x2f28,0x7062,0x12b9,c->data_segment,7,1);
        final_black(c);
        (void)JX_CALL(c,0x2f28,0x6d18,0x12c5,c->data_segment);
        (void)JX_CALL0(c,0x1f2a,0x7272);
        (void)JX_CALL(c,0x2f28,0x3f66,0x2eb4,c->data_segment,1);
        final_black(c);
        final_text(c,9,0x5baa,2);
    }
    (void)JX_CALL0(c,0x2f28,0x7ec8);
    (void)JX_CALL0(c,0x1f2a,0xb196);
}
