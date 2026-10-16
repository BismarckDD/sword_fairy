#include "event_engine.h"
static uint32_t database_open(JxEventContext *c,uint16_t filename,uint16_t mode) {
    return JX_CALL(c,0x1000,0x0738,filename,c->data_segment,mode,c->data_segment);
}
static void database_read(JxEventContext *c,uint32_t file,uint16_t destination,uint16_t size,uint16_t count) {
    (void)JX_CALL(c,0x1000,0x0754,destination,c->data_segment,size,count,(uint16_t)file,(uint16_t)(file>>16));
}
static void database_close(JxEventContext *c,uint32_t file) {
    (void)JX_CALL(c,0x1000,0x0636,(uint16_t)file,(uint16_t)(file>>16));
}
/* 1F2A:60AC. An open failure ends the entire database sequence; short
 * reads/close errors still proceed. Sound loads precede trajectory reset. */
void jx_game_database_load(JxEventContext *c) {
    static const uint16_t names[]={0x040c,0x0417,0x0424,0x0431};
    static const uint16_t modes[]={0x0409,0x0414,0x0421,0x042e};
    static const uint16_t destinations[]={0xd04e,0x499e,0x5154,0x8022};
    static const uint16_t sizes[]={68,30,28,28},counts[]={16,64,64,64};
    for(unsigned i=0;i<4;i++) {
        uint32_t file=database_open(c,names[i],modes[i]);if(!file)return;
        database_read(c,file,destinations[i],sizes[i],counts[i]);database_close(c,file);
    }
    (void)JX_CALL(c,0x1f2a,0x5fd8,35,0);
    (void)JX_CALL(c,0x1f2a,0x5fd8,34,1);
    (void)JX_CALL(c,0x1f2a,0x5fd8,38,2);
    for(unsigned i=0;i<80;i++)jx_set_word(c,(uint16_t)(0x64e4+2*i),0xffff);
    (void)JX_CALL(c,0x2f28,0x6dfe,0x043b,c->data_segment);
}
/* 1F2A:CA04. The 6600-byte scene-presence table ends exactly at 8022. */
void jx_scene_presence_load(JxEventContext *c) {
    uint32_t file=database_open(c,0x08ac,0x08a9);if(!file)return;
    database_read(c,file,0x665a,1,6600);database_close(c,file);
}
/* 1F2A:629E. Chapter/NPC file loaders may mutate live state. Keep their
 * exact positions in initialization and preserve partial initialization
 * when the final optional file cannot be opened. */
void jx_new_game_initialize(JxEventContext *c) {
    jx_set_word(c,0x8756,100);jx_set_word(c,0x8758,0);jx_set_word(c,0x8752,20);
    for(unsigned i=0;i<128;i++)jx_set_word(c,(uint16_t)(0x878a+2*i),0);
    jx_set_word(c,0x8894,1);jx_set_word(c,0x889a,0xffff);
    (void)JX_CALL(c,0x1f2a,0x55f2);
    jx_set_word(c,0x9334,0);(void)JX_CALL(c,0x1f2a,0xca04);
    jx_set_word(c,0x875a,0);jx_set_word(c,0x875c,0xffff);jx_set_word(c,0x875e,0xffff);
    for(unsigned i=0;i<6;i++) {
        uint16_t at=(uint16_t)(0x8760+6*i);
        jx_set_word(c,at,2);jx_set_word(c,(uint16_t)(at+2),2);jx_set_word(c,(uint16_t)(at+4),1);
    }
    uint32_t file=database_open(c,0x0448,0x0445);if(!file)return;
    database_read(c,file,0x3466,1,220);database_close(c,file);
    for(unsigned i=0;i<6600;i++)jx_ds(c)[0xb686+i]=0;
    static const uint16_t levels[]={200,201,202,202,203};
    for(unsigned i=0;i<5;i++)jx_set_word(c,(uint16_t)(0xd06e + 68*i),levels[i]);
}
