#include "event_engine.h"
#include <assert.h>

/* 150F:0006, DF=0. Start index uses AL; wait until retrace bit is set.
 * Count zero enters the original LOOP for 65536 RGB triples. Source wraps
 * its 16-bit offset and is reread for each OUT (callback mutations matter). */
void jx_palette_write(JxEventContext *c,const JxVideoBus *bus,JxFarPointer source,uint16_t first,uint16_t count) {
    bus->out(bus->opaque,0x3c8,(uint8_t)first,1);
    while(!(bus->in_byte(bus->opaque,0x3da)&8)) {}
    unsigned colors=count?count:65536u;
    for(unsigned i=0;i<colors*3u;i++)
        bus->out(bus->opaque,0x3c9,jx_far_byte(c,source,(uint16_t)i),1);
}
/* 2F28:A58A. BIOS AX=1017, BX=first, CX=count, ES:DX=destination.
 * The host owns BIOS behavior, including its count-zero/error semantics. */
void jx_palette_read(JxFarPointer destination,uint16_t first,uint16_t count,JxPaletteBiosRead read,void *opaque) {
    read(opaque,destination,first,count);
}
/* 2F28:A572. BIOS AX1012 forwards all16-bit arguments, including count0.
 * Unlike150F:0006, this wrapper never performs the raw LOOP/retrace wait. */
void jx_palette_bios_write(JxFarPointer source,uint16_t first,uint16_t count,JxPaletteBiosWrite write,void *opaque) {
    write(opaque,source,first,count);
}
/* Native BIOS1012 model: low DAC index, linear real-mode data pointer,
 * count0 writes no RGB bytes, no raw150F retrace wait. Device owns DAC wrap. */
void jx_palette_bios_bus_write(JxEventContext *c,const JxVideoBus *bus,JxFarPointer source,uint16_t first,uint16_t count) {
    bus->out(bus->opaque,0x3c8,(uint8_t)first,1);
    uint32_t address=((uint32_t)source.segment<<4)+source.offset;
    for(unsigned i=0;i<(unsigned)count*3u;i++)
        bus->out(bus->opaque,0x3c9,c->memory[(address+i)&0xfffffu],1);
}
/* 2F28:7C2E native BIOS101B device policy for BX0/CX256. Formula and clamp
 * follow DOSBox-X int10_pal.cpp/VGABIOS; physical BIOS variants are outside
 * this device model. DAC input channels are already six-bit values. */
void jx_palette_bios_grayscale(uint8_t *rgb) {
    for(unsigned i=0;i<256;i++) {
        unsigned gray=(77u*rgb[i*3]+151u*rgb[i*3+1]+28u*rgb[i*3+2]+128u)>>8;
        if(gray>63)gray=63;
        rgb[i*3]=rgb[i*3+1]=rgb[i*3+2]=(uint8_t)gray;
    }
}
/* 2F28:3F66. Snapshot all source colors before any device operation. */
JxPaletteFlow jx_palette_fade_black(JxEventContext *c,JxFarPointer source,int16_t step,const JxPaletteIO *io) {
    uint8_t palette[768];
    for(unsigned i=0;i<768;i++) palette[i]=jx_far_byte(c,source,(uint16_t)i);
    if(step==0) return JX_PALETTE_DIVIDE_ERROR;
    int frames=64/step;if(frames<1) frames=1;
    for(int frame=0;frame<frames;frame++) {
        for(unsigned i=0;i<768;i++) palette[i]=palette[i]>step?(uint8_t)(palette[i]-step):0;
        io->write(io->opaque,0,256,palette);
    }
    return JX_PALETTE_RETURNED;
}
/* 2F28:415E. BIOS capture occurs BEFORE division (also on divide error).
 * Target remains live across frames, and threshold subtraction wraps to
 * a signed word before comparison, even for unusual negative step values. */
JxPaletteFlow jx_palette_fade_to(JxEventContext *c,JxFarPointer target,int16_t step,const JxPaletteIO *io) {
    uint8_t palette[768];io->read(io->opaque,0,256,palette);
    if(step==0) return JX_PALETTE_DIVIDE_ERROR;
    int frames=64/step;if(frames<1) frames=1;
    for(int frame=0;frame<frames;frame++) {
        for(unsigned i=0;i<768;i++) {
            uint8_t value=jx_far_byte(c,target,(uint16_t)i);
            int threshold=jx_signed_word((uint16_t)(value-step));
            palette[i]=palette[i]<threshold?(uint8_t)(palette[i]+step):value;
        }
        io->write(io->opaque,0,256,palette);
    }
    return JX_PALETTE_RETURNED;
}

/* Native local-buffer adapter, not an additional recovered entry point. */
static void palette_bus_read(void *opaque,uint16_t first,uint16_t count,uint8_t *rgb) {
    JxPaletteBusAdapter *adapter=opaque;
    assert(adapter->read);adapter->read(adapter->read_opaque,first,count,rgb);
}
static void palette_bus_write(void *opaque,uint16_t first,uint16_t count,const uint8_t *rgb) {
    JxPaletteBusAdapter *adapter=opaque;const JxVideoBus *bus=adapter->bus;
    assert(count>0 && count<=256);
    bus->out(bus->opaque,0x3c8,(uint8_t)first,1);
    while(!(bus->in_byte(bus->opaque,0x3da)&8)) {}
    for(unsigned i=0;i<(unsigned)count*3;i++) bus->out(bus->opaque,0x3c9,rgb[i],1);
}
JxPaletteIO jx_palette_bus_io(JxPaletteBusAdapter *adapter) {
    assert(adapter && adapter->bus && adapter->bus->out && adapter->bus->in_byte);
    return (JxPaletteIO){adapter,palette_bus_read,palette_bus_write};
}
