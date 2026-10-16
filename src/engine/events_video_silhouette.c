#include "event_engine.h"
/* 1832:1576, DF=0. Replace every nontransparent source pixel by one
 * color, completing each VGA lane in order. All four lanes run even if
 * their word column count is zero: original LOOP then means 65536.
 * Draw segment is captured before OUT; source and target offsets wrap
 * within their fixed segments. No clipping or huge-pointer carry. */
void jx_video_blit_silhouette(JxEventContext *c,const JxVideoDevice *video,
    uint16_t x,uint16_t y,uint16_t width,uint16_t height,
    JxFarPointer source,uint16_t transparent,uint16_t color) {
    uint16_t page=(uint16_t)(c->memory[0x44e]|((unsigned)c->memory[0x44f]<<8));
    uint16_t segment=(uint16_t)(0xa000+(page>>4));
    JxVideoAddress start=jx_video_address(x,y);
    unsigned rows=height?height:65536u;
    for(unsigned lane=0;lane<4;lane++) {
        unsigned count=width/4+(lane<(width&3));
        unsigned columns=count?count:65536u;
        unsigned plane=(start.plane+lane)&3;
        video->out(video->opaque,0x3c4,(uint16_t)(2u|(1u<<(plane+8))),2);
        uint16_t source_row=(uint16_t)(source.offset+lane);
        uint16_t target_row=(uint16_t)(start.offset+(start.plane+lane)/4);
        for(unsigned row=0;row<rows;row++) {
            for(unsigned column=0;column<columns;column++) {
                JxFarPointer pixel={(uint16_t)(source_row+column*4u),source.segment};
                if(jx_far_byte(c,pixel,0)!=(uint8_t)transparent) {
                    uint16_t offset=(uint16_t)(target_row+column);
                    video->write_byte(video->opaque,((uint32_t)segment*16+offset)&0xfffff,(uint8_t)color);
                }
            }
            source_row=(uint16_t)(source_row+width);target_row=(uint16_t)(target_row+80);
        }
    }
}
