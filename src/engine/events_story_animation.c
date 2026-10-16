#include "event_engine.h"
#include <string.h>

static void story_map_pages(JxEventContext *c,uint16_t first) {
    for(uint16_t slot=0;slot<4;slot++)
        (void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(first+slot),jx_word(c,0x9332));
}
static void story_palette(JxEventContext *c,uint16_t destination) {
    uint64_t bits=0;
    for(unsigned i=0;i<8;i++)bits|=(uint64_t)jx_ds(c)[0x23a2+i]<<(8*i);
    double original;float weight;uint32_t argument;
    _Static_assert(sizeof(original)==8 && sizeof(weight)==4,"IEEE floating sizes required");
    memcpy(&original,&bits,8);weight=(float)original;memcpy(&argument,&weight,4);
    (void)JX_CALL(c,0x2f28,0xa8d0,destination,c->data_segment,
        jx_word(c,0x61de),jx_word(c,0x61e0),0xb584,c->data_segment,
        (uint16_t)argument,(uint16_t)(argument>>16));
}
static uint16_t story_next_page(JxEventContext *c) {
    return (uint16_t)(jx_signed_word((uint16_t)(jx_word(c,0x3e54)+1))%2);
}
/* 2F28:7062. The third ABI word is not read by the original function.
 * A divide fault occurs after map/open/palette effects and must stop playback.
 * The double-to-float conversion shares the verified finite/nearest limits
 * documented for the other original MS software-floating call sites. */
JxStoryAnimationFlow jx_story_animation_play(JxEventContext *c,JxFarPointer filename,
    uint16_t unused,uint16_t fade) {
    (void)unused;
    uint16_t next_cue=0,handled=0;
    story_map_pages(c,179);
    (void)JX_CALL(c,0x1000,0x4e32,filename.offset,filename.segment,0x3542,c->data_segment);
    story_palette(c,0x354c);
    uint16_t rate=jx_word(c,0x354a);
    if(rate==0)return JX_STORY_ANIMATION_DIVIDE_ERROR;
    uint16_t delay=(uint16_t)(1000/rate);
    if(delay==0)delay=1;
    if(fade!=0)(void)JX_CALL(c,0x2f28,0x3f66,0x61de,c->data_segment,1);
    for(uint16_t frame=0;frame<jx_word(c,0x3548);frame=(uint16_t)(frame+1)) {
        (void)JX_CALL(c,0x1832,0x1810,story_next_page(c));
        (void)JX_CALL(c,0x1000,0x4d50,0x3542,c->data_segment,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
        (void)JX_CALL(c,0x1832,0x1c6c,0,0,320,200,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
        if(jx_word(c,0x08c8)==1) {
            story_map_pages(c,117);
            (void)JX_CALL(c,0x1832,0x1a28,0,0,320,200,jx_word(c,0x5858),jx_word(c,0x585a),255);
            story_map_pages(c,179);
        }
        (void)JX_CALL(c,0x1832,0x1836,story_next_page(c));
        if(jx_signed_word(fade)>0 && frame==0)
            (void)JX_CALL(c,0x2f28,0x415e,0x354c,c->data_segment,1);
        jx_set_word(c,0x3e54,(uint16_t)(jx_word(c,0x3e54)+1));
        (void)JX_CALL(c,0x1f2a,0x720e,delay);
        uint16_t cue=(uint16_t)(0x64e4+8*next_cue);
        if(frame==jx_word(c,cue) && jx_signed_word(next_cue)<20) {
            handled=1;
            story_map_pages(c,87);
            (void)JX_CALL(c,0x1000,0x3146,jx_word(c,0x5858),jx_word(c,0x585a),
                jx_word(c,0x2eb0),jx_word(c,0x2eb2),64000);
            if(jx_word(c,(uint16_t)(cue+2))==0) {
                uint16_t text=(uint16_t)(0x59d6+4*jx_word(c,(uint16_t)(cue+4)));
                (void)JX_CALL(c,0x2f28,0x648e,jx_word(c,(uint16_t)(cue+6)),
                    jx_word(c,text),jx_word(c,(uint16_t)(text+2)),1);
            } else {
                (void)JX_CALL(c,0x1f2a,0x5fd8,jx_word(c,(uint16_t)(cue+4)),6);
                (void)JX_CALL(c,0x1f2a,0x155c,6);
            }
            story_map_pages(c,87);
            (void)JX_CALL(c,0x1000,0x3146,jx_word(c,0x2eb0),jx_word(c,0x2eb2),
                jx_word(c,0x5858),jx_word(c,0x585a),64000);
            story_map_pages(c,179);
            next_cue=(uint16_t)(next_cue+1);
        }
    }
    (void)JX_CALL(c,0x1000,0x4fe0,0x3542,c->data_segment);
    if(fade!=0) {
        (void)JX_CALL(c,0x2f28,0x3f66,0x354c,c->data_segment,1);
        (void)JX_CALL(c,0x1832,0x1810,0);
        (void)JX_CALL(c,0x1832,0x1884,0,192,320,8,0);
        (void)JX_CALL(c,0x1832,0x1810,1);
        (void)JX_CALL(c,0x1832,0x1884,0,192,320,8,0);
        (void)JX_CALL(c,0x1832,0x1810,(uint16_t)(jx_signed_word(jx_word(c,0x3e54))%2));
    }
    if(handled!=0) {
        for(uint16_t i=0;i<20;i++)
            for(uint16_t field=0;field<8;field=(uint16_t)(field+2))
                jx_set_word(c,(uint16_t)(0x64e4+8*i+field),0xffff);
        (void)JX_CALL(c,0x1f2a,0x5ae8);
    }
    story_palette(c,0x61de);
    return JX_STORY_ANIMATION_RETURNED;
}
