#include "event_engine.h"

/* 1832:17AE. The BIOS callback establishes mode 13h; the observed register
 * transfers then disable chain-4 and clear the complete VGA aperture.
 * The original OR DX,4 leaves port 03C5 unchanged: do not change it to OR AL. */
void jx_video_enter_mode_x(const JxVideoBus *bus,
    void (*bios_mode)(void *,uint16_t),void *bios_opaque) {
    bios_mode(bios_opaque,0x13);
    bus->out(bus->opaque,0x3c4,4,1);
    uint8_t value=bus->in_byte(bus->opaque,0x3c5);
    bus->out(bus->opaque,0x3c5,value&0xf7,1);
    bus->out(bus->opaque,0x3ce,5,1);
    value=bus->in_byte(bus->opaque,0x3cf);
    bus->out(bus->opaque,0x3cf,value&0xef,1);
    bus->out(bus->opaque,0x3ce,6,1);
    value=bus->in_byte(bus->opaque,0x3cf);
    bus->out(bus->opaque,0x3cf,value&0xfd,1);
    for(uint32_t offset=0;offset<65536;offset++)
        bus->write_byte(bus->opaque,0xa0000+offset,0);
    bus->out(bus->opaque,0x3d4,0x14,1);
    value=bus->in_byte(bus->opaque,0x3d5);
    bus->out(bus->opaque,0x3d5,value&0xbf,1);
    bus->out(bus->opaque,0x3d4,0x17,1);
    value=bus->in_byte(bus->opaque,0x3d5);
    bus->out(bus->opaque,0x3d5,value|0x40,1);
}

/* 1502:000A / 150E:000A, sequencer clocking-mode screen-disable bit. */
void jx_video_blank_screen(const JxVideoBus *bus) {
    bus->out(bus->opaque,0x3c4,1,1);
    uint8_t value=bus->in_byte(bus->opaque,0x3c5);
    bus->out(bus->opaque,0x3c5,value|0x20,1);
}
void jx_video_unblank_screen(const JxVideoBus *bus) {
    bus->out(bus->opaque,0x3c4,1,1);
    uint8_t value=bus->in_byte(bus->opaque,0x3c5);
    bus->out(bus->opaque,0x3c5,value&0xdf,1);
}

