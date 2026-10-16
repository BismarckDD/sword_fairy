#include "event_engine.h"

static void map_four_pages(JxEventContext *c,uint16_t first) {
    for(uint16_t page=0;page<4;page++)
        (void)JX_CALL(c,0x1503,0x007d,page,(uint16_t)(first+page),jx_word(c,0x9332));
}
static void capture_transition(JxEventContext *c) {
    map_four_pages(c,87);
    (void)JX_CALL(c,0x1832,0x1e88,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a));
    (void)JX_CALL(c,0x17d2,0x01e0,jx_word(c,0x5858),jx_word(c,0x585a),jx_word(c,0x2eb0),jx_word(c,0x2eb2),4,10);
}
static void load_scene(JxEventContext *c) {
    (void)JX_CALL(c,0x1f2a,0x6700,jx_word(c,0x9334),0);
}
static void chapter(JxEventContext *c,uint16_t value) {
    jx_set_word(c,0x8894,value); (void)JX_CALL0(c,0x1f2a,0x55f2);
}
static void retrospective_music(JxEventContext *c,uint16_t track) {
    if(jx_signed_word(jx_word(c,0x050c))>0)
        (void)JX_CALL(c,0x1832,0x224c,(uint16_t)((jx_ds(c)[0x932c]+track)&255),2,
            jx_far_word(c,jx_word(c,0x23a0),(uint16_t)(0x510 + track*2)));
}

/* No original arguments: the extra NPC parameter is ignored by this adapter. */
void jx_event_194_0000(JxEventContext *c,int16_t npc) {
    (void)npc;
    uint32_t file=JX_CALL(c,0x1000,0x0738,0x0a45,c->data_segment,0x0a42,c->data_segment);
    if(!file) return;
    uint16_t lo=(uint16_t)file,hi=(uint16_t)(file>>16);
    map_four_pages(c,117);
    (void)JX_CALL(c,0x1000,0x27fa,lo,hi,16,0,0);
    (void)JX_CALL(c,0x1000,0x0754,jx_word(c,0x5858),jx_word(c,0x585a),1,64000,lo,hi);
    (void)JX_CALL(c,0x1000,0x0636,lo,hi);
    capture_transition(c); jx_event_transition_begin(c);
    jx_set_word(c,0x08c8,1); jx_set_word(c,0x889a,0xffff);
    retrospective_music(c,1);
    jx_set_word(c,0x9334,14); chapter(c,1);
    for(unsigned i=0;i<6;i++) jx_event_position(c,(uint16_t)(0x8760 + 6*i),10,10,2);
    load_scene(c);
    jx_set_word(c,0x89de,(uint16_t)(jx_word(c,0x8760)+1));
    jx_set_word(c,0x89e0,jx_word(c,0x8762)); jx_ds(c)[0x6801]=1;
    jx_event_refresh(c);
    (void)JX_CALL(c,0x1f2a,0xcd1c,3,0x0a50,c->data_segment,2);
    jx_event_text(c,3,0x5eae,1);
    (void)JX_CALL(c,0x2f28,0x944c,0x0a52,c->data_segment,2);
    jx_event_text(c,3,0x6082,1);
    capture_transition(c);
    jx_event_position(c,0x64e4,16,0,428); jx_set_word(c,0x64ea,3);
    (void)JX_CALL(c,0x2f28,0x7062,0x0a54,c->data_segment,5,1);
    jx_set_word(c,0x9334,21); jx_event_position(c,0x8760,24,92,1);
    jx_ds(c)[0x68d0]=1; load_scene(c); jx_event_text(c,0,0x608a,1);
    capture_transition(c); jx_event_transition_begin(c);
    jx_set_word(c,0x9334,22); jx_set_word(c,0x875c,1);
    jx_event_position(c,0x8760,8,7,3); jx_event_position(c,0x876c,9,7,2);
    load_scene(c); jx_event_text(c,0,0x608e,1);
    capture_transition(c); jx_event_transition_begin(c);
    jx_set_word(c,0x9334,31); chapter(c,2);
    jx_set_word(c,0x875a,1); jx_set_word(c,0x875c,0);
    jx_event_position(c,0x8760,34,73,0); jx_event_position(c,0x876c,34,71,1);
    load_scene(c); jx_event_text(c,0,0x5e3a,1);
    capture_transition(c); jx_event_transition_begin(c);
    jx_set_word(c,0x9334,124); chapter(c,3);
    jx_set_word(c,0x875a,0); jx_set_word(c,0x875c,1);
    jx_event_position(c,0x8760,84,42,1); jx_event_position(c,0x876c,86,42,1);
    jx_ds(c)[0x74e8]=1; load_scene(c); jx_event_text(c,0,0x5d06,1);
    capture_transition(c); jx_event_transition_begin(c);
    jx_set_word(c,0x875a,1); jx_set_word(c,0x875c,0xffff);
    jx_event_position(c,0x8760,2,9,3); jx_set_word(c,0x9334,108);
    jx_ds(c)[0x7303]=1; jx_ds(c)[0x7304]=1; load_scene(c);
    jx_event_text(c,0,0x5d0a,1); retrospective_music(c,17); jx_event_text(c,0,0x5d0e,1);
    capture_transition(c); jx_event_transition_begin(c);
    chapter(c,5); jx_set_word(c,0x9334,161);
    jx_set_word(c,0x875a,0); jx_set_word(c,0x875c,4); jx_ds(c)[0x7942]=1;
    jx_event_position(c,0x8760,46,41,0); jx_event_position(c,0x876c,46,43,0);
    load_scene(c); jx_event_text(c,10,0x5c16,1);
    (void)JX_CALL(c,0x1f2a,0xcd1c,10,0x0a60,c->data_segment,3);
    jx_set_word(c,0x08c8,0); jx_event_transition_begin(c);
    jx_set_word(c,0x889a,3);
    if(jx_signed_word(jx_word(c,0x31b4))>0) retrospective_music(c,3);
    chapter(c,5); jx_set_word(c,0x9334,189);
    jx_event_position(c,0x8760,20,10,0); jx_ds(c)[0x7c85]=1; load_scene(c);
    jx_event_position(c,0x8a8a,21,11,2); jx_event_refresh(c);
}

static int battle_failed(JxEventContext *c,uint16_t battle,uint16_t mode) {
    if((uint16_t)JX_CALL(c,0x1f2a,0x4934,battle,0,mode)!=1) return 0;
    while((uint16_t)JX_CALL0(c,0x1f2a,0xa2f0)!=0) {}
    return 1;
}

void jx_event_194_088c(JxEventContext *c,int16_t npc) {
    jx_set_word(c,0x89e2,jx_word(c,0x898c)); jx_set_word(c,0x8a38,jx_word(c,0x898c));
    jx_event_refresh(c); jx_event_text(c,npc,0x5b42,1);
    (void)JX_CALL(c,0x2f28,0x7a1c,2); (void)JX_CALL(c,0x2f28,0x7a1c,3);
    (void)JX_CALL(c,0x1f2a,0xcc24,(uint16_t)(jx_word(c,0x8a34)+1),(uint16_t)(jx_word(c,0x8a36)+1));
    jx_event_text(c,npc,0x5b46,1); jx_event_text(c,4,0x5b4a,1); jx_event_text(c,4,0x5b4e,1);
    jx_event_transition_begin(c);
    jx_set_word(c,0xd05c,164); jx_set_word(c,0xd0a0,127);
    (void)JX_CALL0(c,0x1f2a,0xc6a8); jx_event_refresh(c); jx_event_transition_end(c);
    jx_event_text(c,4,0x5b52,1); jx_event_text(c,4,0x5b56,1);
    jx_event_position(c,0x8760,20,10,1); jx_event_position(c,0x876c,22,11,2);
    jx_event_position(c,0x8a34,19,11,3);
    if(battle_failed(c,25,1)) return;
    jx_event_position(c,0x64e4,0,1,56);
    (void)JX_CALL(c,0x2f28,0x7062,0x1277,c->data_segment,7,0);
    jx_event_text(c,4,0x5b6a,1); jx_set_word(c,0x875c,4);
    (void)JX_CALL0(c,0x1f2a,0xc6a8); jx_event_refresh(c);
    jx_event_text(c,4,0x5b6e,1); jx_event_text(c,4,0x5b72,1);
    (void)JX_CALL(c,0x1f2a,0x747c,4);
    jx_set_word(c,0x8764,0); jx_set_word(c,0x889a,0xffff);
    if(battle_failed(c,26,4)) return;
    jx_event_grant(c,11,1); jx_event_transition_begin(c);
    jx_ds(c)[0x7c84]=0; jx_event_refresh(c); jx_event_transition_end(c);
    jx_set_word(c,0x875c,0xffff); jx_event_text(c,0,0x5b5e,1);
    jx_event_194_0000(c,0);
    jx_event_text(c,5,0x5b92,1); jx_set_word(c,0x875c,4);
    jx_event_position(c,0x8760,12,92,0); jx_event_position(c,0x876c,12,90,1);
    jx_event_music(c,5,2); jx_set_word(c,0x9334,190); chapter(c,6); load_scene(c);
    jx_event_text(c,0,0x59da,1); jx_set_word(c,0x875c,0xffff); jx_event_refresh(c);
}
