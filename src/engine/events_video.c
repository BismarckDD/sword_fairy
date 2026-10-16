#include "event_engine.h"

/* 1832:1551. Original register ABI: DX=x, CX=y; DI=offset,
 * CL=plane mask, CH=plane index. The C result names those fields. */
JxVideoAddress jx_video_address(uint16_t x,uint16_t y) {
    uint8_t plane=(uint8_t)(x&3);
    return (JxVideoAddress){(uint16_t)(y*80u+(x>>2)),(uint8_t)(1u<<plane),plane};
}

/* 1832:14CE. No clipping in the original. The BIOS data-area draw page
 * is read after the port transaction, preserving externally visible order. */
void jx_video_plot(JxEventContext *c,const JxVideoDevice *video,
    uint16_t x,uint16_t y,uint16_t color) {
    JxVideoAddress a=jx_video_address(x,y);
    video->out(video->opaque,0x3c4,(uint16_t)(2u|((unsigned)a.plane_mask<<8)),2);
    uint16_t page=(uint16_t)(c->memory[0x44e]|((unsigned)c->memory[0x44f]<<8));
    uint16_t segment=(uint16_t)(0xa000+(page>>4));
    video->write_byte(video->opaque,((uint32_t)segment*16+a.offset)&0xfffff,(uint8_t)color);
}
/* 1832:1884. Fill each VGA lane before selecting the next; draw page is
 * captured before OUT and remains fixed if the device callback changes BDA. */
void jx_video_fill_rectangle(JxEventContext *c,const JxVideoDevice *video,
    uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint16_t color) {
    uint16_t page=(uint16_t)(c->memory[0x44e]|((unsigned)c->memory[0x44f]<<8));
    uint16_t segment=(uint16_t)(0xa000+(page>>4));
    JxVideoAddress start=jx_video_address(x,y);
    unsigned rows=height?height:65536u;
    for(unsigned lane=0;lane<4;lane++) {
        unsigned columns=width/4+(lane<(width&3));
        if(!columns) continue;
        unsigned plane=(start.plane+lane)&3;
        video->out(video->opaque,0x3c4,(uint16_t)(2u|(1u<<(plane+8))),2);
        uint16_t row_start=(uint16_t)(start.offset+(start.plane+lane)/4);
        for(unsigned row=0;row<rows;row++) {
            for(unsigned column=0;column<columns;column++) {
                uint16_t offset=(uint16_t)(row_start+column);
                video->write_byte(video->opaque,((uint32_t)segment*16+offset)&0xfffff,(uint8_t)color);
            }
            row_start=(uint16_t)(row_start+80);
        }
    }
}

/* 1832:1810. Four 16-KiB pages in each VGA plane. */
void jx_video_draw_page(JxEventContext *c,uint16_t page) {
    uint16_t offset=(uint16_t)((page&3)*0x4000u);
    c->memory[0x44e]=(uint8_t)offset;c->memory[0x44f]=(uint8_t)(offset>>8);
}

/* 1832:1836. Only CRTC start-address high is written by this routine. */
void jx_video_show_page(const JxVideoDevice *video,uint16_t page) {
    uint16_t high=(uint16_t)((page&3)*0x40u);
    video->out(video->opaque,0x3d4,0x0c,1);
    video->out(video->opaque,0x3d5,high,1);
}

/* 1832:1C6C / 1A28, DF=0. Transfer each lane completely before selecting
 * the next VGA plane. BDA draw page is captured before the first OUT,
 * unlike video_plot. There is no clipping or huge-pointer segment carry.
 * Original height LOOP is entered unchecked: zero means 65536 rows.
 * Width zero has no lanes and generates no device transactions. */
static void blit(JxEventContext *c,const JxVideoDevice *video,uint16_t x,uint16_t y,
    uint16_t width,uint16_t height,JxFarPointer source,int transparent) {
    uint16_t page=(uint16_t)(c->memory[0x44e]|((unsigned)c->memory[0x44f]<<8));
    uint16_t segment=(uint16_t)(0xa000+(page>>4));
    JxVideoAddress start=jx_video_address(x,y);
    unsigned rows=height?height:65536u;
    for(unsigned lane=0;lane<4;lane++) {
        unsigned columns=width/4+(lane<(width&3));
        if(columns==0) continue;
        unsigned plane=(start.plane+lane)&3;
        video->out(video->opaque,0x3c4,(uint16_t)(2u|(1u<<(plane+8))),2);
        uint16_t source_row=(uint16_t)(source.offset+lane);
        uint16_t target_row=(uint16_t)(start.offset+(start.plane+lane)/4);
        for(unsigned row=0;row<rows;row++) {
            for(unsigned column=0;column<columns;column++) {
                JxFarPointer pixel={(uint16_t)(source_row+column*4),source.segment};
                uint8_t value=jx_far_byte(c,pixel,0);
                if(transparent<0 || value!=(uint8_t)transparent) {
                    uint16_t offset=(uint16_t)(target_row+column);
                    video->write_byte(video->opaque,((uint32_t)segment*16+offset)&0xfffff,value);
                }
            }
            source_row=(uint16_t)(source_row+width);
            target_row=(uint16_t)(target_row+80);
        }
    }
}
void jx_video_blit(JxEventContext *c,const JxVideoDevice *video,uint16_t x,uint16_t y,
    uint16_t width,uint16_t height,JxFarPointer source) {
    blit(c,video,x,y,width,height,source,-1);
}
void jx_video_blit_transparent(JxEventContext *c,const JxVideoDevice *video,uint16_t x,uint16_t y,
    uint16_t width,uint16_t height,JxFarPointer source,uint16_t transparent) {
    blit(c,video,x,y,width,height,source,(uint8_t)transparent);
}

/* 1832:1E88, DF=0. Read-map selection and byte reads are device operations;
 * destination is RAM. Like blit, capture has unchecked zero-height LOOP,
 * fixed segments, wrapping offsets, and no clipping. */
void jx_video_capture(JxEventContext *c,const JxVideoReadDevice *video,
    uint16_t x,uint16_t y,uint16_t width,uint16_t height,JxFarPointer destination) {
    uint16_t page=(uint16_t)(c->memory[0x44e]|((unsigned)c->memory[0x44f]<<8));
    uint16_t segment=(uint16_t)(0xa000+(page>>4));
    JxVideoAddress start=jx_video_address(x,y);
    unsigned rows=height?height:65536u;
    for(unsigned lane=0;lane<4;lane++) {
        unsigned columns=width/4+(lane<(width&3));
        if(columns==0) continue;
        unsigned plane=(start.plane+lane)&3;
        video->out(video->opaque,0x3ce,(uint16_t)(4u|(plane<<8)),2);
        uint16_t source_row=(uint16_t)(start.offset+(start.plane+lane)/4);
        uint16_t target_row=(uint16_t)(destination.offset+lane);
        for(unsigned row=0;row<rows;row++) {
            for(unsigned column=0;column<columns;column++) {
                uint16_t offset=(uint16_t)(source_row+column);
                uint8_t value=video->read_byte(video->opaque,((uint32_t)segment*16+offset)&0xfffff);
                JxFarPointer pixel={(uint16_t)(target_row+column*4),destination.segment};
                jx_far_set_byte(c,pixel,0,value);
            }
            source_row=(uint16_t)(source_row+80);
            target_row=(uint16_t)(target_row+width);
        }
    }
}

/* 1832:135E, DF=0. Page numbers are not masked to 0..3. The device owns
 * latches and write mode; every read must precede its paired write.
 * Exit clears bit 0 of the CURRENT mode, not a saved entry mode. */
void jx_video_copy_page(const JxVideoBus *video,uint16_t destination,uint16_t source) {
    uint16_t source_segment=(uint16_t)(0xa000u+source*0x400u);
    uint16_t destination_segment=(uint16_t)(0xa000u+destination*0x400u);
    video->out(video->opaque,0x3c4,0x0f02,2);
    video->out(video->opaque,0x3ce,5,1);
    uint8_t mode=video->in_byte(video->opaque,0x3cf);
    video->out(video->opaque,0x3cf,(uint8_t)((mode|1u)&0xfdu),1);
    for(unsigned offset=0;offset<16000;offset++) {
        uint8_t value=video->read_byte(video->opaque,((uint32_t)source_segment*16+offset)&0xfffff);
        video->write_byte(video->opaque,((uint32_t)destination_segment*16+offset)&0xfffff,value);
    }
    video->out(video->opaque,0x3ce,5,1);
    mode=video->in_byte(video->opaque,0x3cf);
    video->out(video->opaque,0x3cf,(uint8_t)(mode&0xfeu),1);
}
/* 1832:11A8, DF=0. VGA latch copy of complete four-pixel groups. Low two
 * coordinate/width bits are discarded. Forward overlapping copies remain
 * forward; zero height enters the original 65536-row LOOP. */
void jx_video_copy_rectangle(const JxVideoBus *video,uint16_t x,uint16_t y,uint16_t width,uint16_t height,
    uint16_t target_x,uint16_t target_y,uint16_t destination,uint16_t source) {
    uint16_t source_segment=(uint16_t)(0xa000u+source*0x400u);
    uint16_t destination_segment=(uint16_t)(0xa000u+destination*0x400u);
    uint16_t source_row=jx_video_address(x,y).offset,target_row=jx_video_address(target_x,target_y).offset;
    unsigned columns=width>>2,rows=height?height:65536u;
    video->out(video->opaque,0x3c4,0x0f02,2);
    video->out(video->opaque,0x3ce,5,1);
    uint8_t mode=video->in_byte(video->opaque,0x3cf);
    video->out(video->opaque,0x3cf,(uint8_t)((mode|1u)&0xfdu),1);
    for(unsigned row=0;row<rows;row++) {
        for(unsigned column=0;column<columns;column++) {
            uint8_t value=video->read_byte(video->opaque,((uint32_t)source_segment*16+(uint16_t)(source_row+column))&0xfffff);
            video->write_byte(video->opaque,((uint32_t)destination_segment*16+(uint16_t)(target_row+column))&0xfffff,value);
        }
        source_row=(uint16_t)(source_row+80);target_row=(uint16_t)(target_row+80);
    }
    video->out(video->opaque,0x3ce,5,1);
    mode=video->in_byte(video->opaque,0x3cf);
    video->out(video->opaque,0x3cf,(uint8_t)(mode&0xfeu),1);
}
/* 1832:1250. Plane-major packed image, unlike interleaved 1E88 capture.
 * Each lane stores whole WORDs: width is rounded down to a multiple of 8.
 * A WORD crosses FFFF physically; the next WORD starts at wrapped offset. */
void jx_video_capture_planes(JxEventContext *c,const JxVideoReadDevice *video,uint16_t x,uint16_t y,
    uint16_t width,uint16_t height,JxFarPointer destination) {
    uint16_t page=(uint16_t)(c->memory[0x44e]|((uint16_t)c->memory[0x44f]<<8));
    uint16_t segment=(uint16_t)(0xa000+(page>>4));JxVideoAddress start=jx_video_address(x,y);
    unsigned words=width>>3,rows=height?height:65536u;
    for(unsigned lane=0;lane<4;lane++) {
        unsigned plane=(start.plane+lane)&3;
        video->out(video->opaque,0x3ce,(uint16_t)(4u|(plane<<8)),2);
        uint16_t row_start=(uint16_t)(start.offset+(start.plane+lane)/4);
        for(unsigned row=0;row<rows;row++) {
            for(unsigned column=0;column<words;column++) {
                uint32_t address=((uint32_t)segment*16+(uint16_t)(row_start+2*column))&0xfffff;
                uint8_t lo=video->read_byte(video->opaque,address),hi=video->read_byte(video->opaque,(address+1)&0xfffff);
                uint32_t target=((uint32_t)destination.segment*16+destination.offset)&0xfffff;
                c->memory[target]=lo;c->memory[(target+1)&0xfffff]=hi;destination.offset+=2;
            }
            row_start=(uint16_t)(row_start+80);
        }
    }
}
/* 1832:13B0. Inverse plane-major WORD transfer, DF=0 and fixed segments. */
void jx_video_blit_planes(JxEventContext *c,const JxVideoDevice *video,uint16_t x,uint16_t y,
    uint16_t width,uint16_t height,JxFarPointer source) {
    uint16_t page=(uint16_t)(c->memory[0x44e]|((uint16_t)c->memory[0x44f]<<8));
    uint16_t segment=(uint16_t)(0xa000+(page>>4));JxVideoAddress start=jx_video_address(x,y);
    unsigned words=width>>3,rows=height?height:65536u;
    for(unsigned lane=0;lane<4;lane++) {
        unsigned plane=(start.plane+lane)&3;
        video->out(video->opaque,0x3c4,(uint16_t)(2u|(1u<<(plane+8))),2);
        uint16_t row_start=(uint16_t)(start.offset+(start.plane+lane)/4);
        for(unsigned row=0;row<rows;row++) {
            for(unsigned column=0;column<words;column++) {
                uint16_t value=jx_far_word(c,source.segment,source.offset);source.offset+=2;
                uint32_t address=((uint32_t)segment*16+(uint16_t)(row_start+2*column))&0xfffff;
                video->write_byte(video->opaque,address,(uint8_t)value);
                video->write_byte(video->opaque,(address+1)&0xfffff,(uint8_t)(value>>8));
            }
            row_start=(uint16_t)(row_start+80);
        }
    }
}
