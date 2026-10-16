#include "event_engine.h"

static void crop_decoder_pages(JxEventContext *c) {
    for (uint16_t slot=0;slot<4;slot++)
        (void)JX_CALL(c,0x1503,0x7d,slot,179+slot,jx_word(c,0x9332));
}
/* 1F2A:288A. Bounds apply independently, not to x+width or y+height.
 * Each frame reserves two EMS pages regardless of cropped byte count. */
uint16_t jx_animation_crop_cache(JxEventContext *c,JxFarPointer filename,
    uint16_t first_page,uint16_t x,uint16_t y,uint16_t width,uint16_t height) {
    if ((int16_t)x<0 || x>319 || (int16_t)y<0 || y>199 ||
        (int16_t)width<1 || width>320 || (int16_t)height<1 || height>200) return 0xffff;
    crop_decoder_pages(c);
    if ((uint16_t)JX_CALL(c,0x1000,0x4e32,filename.offset,filename.segment,0x3542,c->data_segment)!=0)
        return 0xffff;
    for (uint16_t frame=0;frame<jx_word(c,0x3548);frame++) {
        crop_decoder_pages(c);
        (void)JX_CALL(c,0x1000,0x4d50,0x3542,c->data_segment,jx_word(c,0x2eb0),jx_word(c,0x2eb2));
        JxFarPointer destination={jx_word(c,0x5858),jx_word(c,0x585a)};
        (void)JX_CALL(c,0x1503,0x7d,0,(uint16_t)(first_page+2*frame),jx_word(c,0x9332));
        (void)JX_CALL(c,0x1503,0x7d,1,(uint16_t)(first_page+2*frame+1),jx_word(c,0x9332));
        for (uint16_t row=0;row<height;row++) {
            /* IMUL's high half is discarded before huge-pointer addition. */
            uint16_t scanline=(uint16_t)(320u*(y+row));
            uint32_t offset=(uint32_t)jx_word(c,0x2eb0)+scanline+x;
            JxFarPointer source={(uint16_t)offset,
                (uint16_t)(jx_word(c,0x2eb2)+((offset>>16)<<12))};
            (void)JX_CALL(c,0x1000,0x3146,destination.offset,destination.segment,source.offset,source.segment,width);
            /* Destination increments wrap the offset without segment carry. */
            destination.offset=(uint16_t)(destination.offset+width);
        }
    }
    (void)JX_CALL(c,0x1000,0x4fe0,0x3542,c->data_segment);
    return 0;
}
