#include "event_engine.h"

static void scene_text(JxEventContext *c,uint16_t npc,uint16_t text,uint16_t mode) {
    (void)JX_CALL(c,0x2f28,0x648e,npc,jx_word(c,text),jx_word(c,(uint16_t)(text+2)),mode);
}
static void scene_music(JxEventContext *c,uint16_t track) {
    jx_set_word(c,0x889a,track);
    if ((int16_t)jx_word(c,0x31b4)>0 && (int16_t)jx_word(c,0x050c)>0) {
        uint16_t seconds=jx_far_word(c,jx_word(c,0x23a0),(uint16_t)(0x510+2*track));
        (void)JX_CALL(c,0x1832,0x224c,(uint8_t)(jx_ds(c)[0x932c]+track),2,seconds);
    }
}
static void scene_black_screen(JxEventContext *c) {
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,0);
    (void)JX_CALL(c,0x2f28,0x415e,0x61de,c->data_segment,1);
}
/* 2F28:2B58. Branch after the music service, using the current D04D flag. */
void jx_callback_2f28_2b58(JxEventContext *c,uint16_t npc) {
    scene_music(c,10);
    if (jx_ds(c)[0xd04d]!=0) {
        scene_text(c,npc,0x5aaa,1);scene_text(c,npc,0x5aca,1);
        jx_set_word(c,0x875c,1);jx_set_word(c,0x875e,4);
        (void)JX_CALL0(c,0x1f2a,0xc6a8);
        jx_ds(c)[0x7cf9]=0;jx_ds(c)[0x7cfa]=0;
        jx_ds(c)[0x7cbc]=1;jx_ds(c)[0x7cbd]=1;
        (void)JX_CALL0(c,0x2f28,0x5580);
        return;
    }
    scene_text(c,npc,0x5aaa,2);scene_text(c,npc,0x5aae,2);
    (void)JX_CALL(c,0x2f28,0x6d18,0x9f8,c->data_segment);
    (void)JX_CALL0(c,0x1f2a,0x7272);
    (void)JX_CALL(c,0x2f28,0x3f66,0x2eb4,c->data_segment,1);
    scene_black_screen(c);
    (void)JX_CALL(c,0x2f28,0x6d18,0xa01,c->data_segment);
    (void)JX_CALL0(c,0x1f2a,0x7272);
    (void)JX_CALL(c,0x2f28,0x3f66,0x2eb4,c->data_segment,1);
    (void)JX_CALL0(c,0x2f28,0x7ec8);
    (void)JX_CALL0(c,0x1f2a,0xb196);
}
/* 2F28:2592. Item, dialogue, departure and transfer to place 200. */
void jx_callback_2f28_2592(JxEventContext *c,uint16_t npc) {
    if (jx_ds(c)[0x7cbc]==0) return;
    scene_text(c,npc,0x5ace,1);scene_text(c,npc,0x5ad2,1);
    (void)JX_CALL(c,0x1f2a,0x8cea,0x22,1);
    scene_text(c,0,0x5ac6,1);scene_text(c,1,0x5ac2,1);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    scene_black_screen(c);
    scene_text(c,0xffff,0x5ad6,1);
    jx_set_word(c,0x8760,43);jx_set_word(c,0x8762,85);jx_set_word(c,0x8764,3);
    jx_set_word(c,0x9334,200);jx_set_word(c,0x875c,0xffff);jx_set_word(c,0x875e,0xffff);
    scene_music(c,16);
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
    scene_text(c,6,0x5ada,1);
}
/* 2F28:2946 and2A46 are alternate endings reached through2AEE.
 * Every final shutdown call remains real; returning shutdown fixtures only
 * exist in differential tests, not in the native game host. */
static void ending_walk_intro(JxEventContext *c,uint16_t moves){
    (void)JX_CALL(c,0x1f2a,0xcc24,22,11);(void)JX_CALL(c,0x1f2a,0xcc24,18,6);
    (void)JX_CALL(c,0x2f28,0x944c,moves,c->data_segment,3);
    scene_text(c,0xffff,0x5a7a,1);(void)JX_CALL0(c,0x2f28,0x276e);
    (void)JX_CALL(c,0x1f2a,0x72b2,100);
}
void jx_callback_2f28_2946(JxEventContext *c){
    ending_walk_intro(c,0x09dc);
    (void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);scene_black_screen(c);
    scene_text(c,0xffff,0x5a7e,1);jx_set_word(c,0xd05a,0xffff);
    jx_set_word(c,0x64e4,33);jx_set_word(c,0x64e6,0);jx_set_word(c,0x64e8,43);jx_set_word(c,0x64ea,0);
    (void)JX_CALL(c,0x2f28,0x7062,0x09de,c->data_segment,7,1);
    (void)JX_CALL(c,0x1832,0x1884,0,0,320,200,48);
    (void)JX_CALL(c,0x2f28,0x415e,0x2eb4,c->data_segment,1);
    (void)JX_CALL0(c,0x2f28,0x7ec8);(void)JX_CALL0(c,0x1f2a,0xb196);
}
void jx_callback_2f28_2a46(JxEventContext *c){
    ending_walk_intro(c,0x09ea);(void)JX_CALL(c,0x2f28,0x3b8e,0x09ec,c->data_segment);
    scene_black_screen(c);scene_text(c,0xffff,0x5a96,1);
    (void)JX_CALL0(c,0x2f28,0x7ec8);(void)JX_CALL0(c,0x1f2a,0xb196);
}
void jx_callback_2f28_2aee(JxEventContext *c){
    if(!jx_ds(c)[0xcd23])return;
    if(jx_ds(c)[0xc5fe]==5)(void)JX_CALL0(c,0x2f28,0x2946);
    if(jx_ds(c)[0xcd40])(void)JX_CALL0(c,0x2f28,0x2a46);
}
