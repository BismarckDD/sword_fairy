/* Original 1832:3331 near palette selector. The 3367 CMP instruction skips
 * an overlapping MOV AL,FD at3368: preserve both original error entries.
 * BIOS AH0B and the descriptor palette callback remain device boundaries. */
#include "event_engine.h"
uint16_t jx_legacy_video_select_cga_palette(JxEventContext *c,uint16_t value,
    const JxLegacyVideoDevice *device,JxLegacyPaletteService palette,void *opaque){
    uint8_t kind=jx_ds(c)[0x187c];
    if(kind==4){
        if(value>15){jx_ds(c)[0x2cb6]=0xfc;return 0xffff;}
        uint16_t old=c->memory[0x466]&15;
        JxLegacyBiosRegisters regs={(uint16_t)(0x0b00|old),value,0,0,0,0,0,0};
        device->bios(device->opaque,&regs);return old;
    }
    if(kind!=1){jx_ds(c)[0x2cb6]=0xfd;return 0xffff;}
    if(value>3){jx_ds(c)[0x2cb6]=0xfc;return 0xffff;}
    if(jx_ds(c)[0x2cc6]&0x1c){
        (void)palette(opaque,jx_word(c,0x18a2),
            (JxFarPointer){(uint16_t)(0x30c0+16*value),0x1832});
        uint16_t old=jx_ds(c)[0x2cba];jx_ds(c)[0x2cba]=(uint8_t)value;return old;
    }
    uint8_t old=c->memory[0x466];
    JxLegacyBiosRegisters regs={(uint16_t)(0x0b00|value),(uint16_t)(0x0100|(value&1)),0,0,0,0,0,0};
    device->bios(device->opaque,&regs);
    regs.ax=(uint16_t)(0x0b00|((value&2)<<3));
    regs.bx=(uint16_t)(((value&2)<<3)|(old&15));
    device->bios(device->opaque,&regs);
    return (uint16_t)(((old>>4)&1)*2+((old>>5)&1));
}
