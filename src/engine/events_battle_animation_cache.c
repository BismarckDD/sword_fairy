#include "event_engine.h"
static void cache_map(JxEventContext *c,uint16_t first,uint16_t slots) {
    for(uint16_t slot=0;slot<slots;slot++)
        (void)JX_CALL(c,0x1503,0x007d,slot,(uint16_t)(first+slot),jx_word(c,0x9332));
}
static JxFarPointer cache_source_add(JxFarPointer source,uint16_t offset) {
    uint32_t sum=(uint32_t)source.offset+offset;
    source.offset=(uint16_t)sum;
    source.segment=(uint16_t)(source.segment+((sum>>16)<<12));return source;
}
/* 1F2A:288A. Coordinates and dimensions are checked separately, without
 * checking x+width or y+height. Preserve truncated row multiplication and
 * fixed-segment destination advancement, including their edge behavior. */
uint16_t jx_battle_cache_animation(JxEventContext *c,JxFarPointer filename,
                                 uint16_t first_page,uint16_t x,uint16_t y,
                                 uint16_t width,uint16_t height) {
    if(x>319 || y>199 || width<1 || width>320 || height<1 || height>200)return 0xffff;
    cache_map(c,179,4);
    if((uint16_t)JX_CALL(c,0x1000,0x4e32,filename.offset,filename.segment,0x3542,c->data_segment)!=0)
        return 0xffff;
    for(uint16_t frame=0;frame<jx_word(c,0x3548);frame++) {
        cache_map(c,179,4);
        (void)JX_CALL(c,0x1000,0x4d50,0x3542,c->data_segment,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
        JxFarPointer destination={jx_word(c,0x5858),jx_word(c,0x585a)};
        cache_map(c,(uint16_t)(first_page+frame*2),2);
        for(uint16_t row=0;row<height;row++) {
            JxFarPointer source={jx_word(c,0x2eb0),jx_word(c,0x2eb2)};
            source=cache_source_add(source,(uint16_t)((uint16_t)(y+row)*320));
            source=cache_source_add(source,x);
            (void)JX_CALL(c,0x1000,0x3146,destination.offset,destination.segment,
                          source.offset,source.segment,width);
            destination.offset=(uint16_t)(destination.offset+width);
        }
    }
    (void)JX_CALL(c,0x1000,0x4fe0,0x3542,c->data_segment);
    return 0;
}
