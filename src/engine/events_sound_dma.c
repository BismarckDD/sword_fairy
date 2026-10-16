#include "event_engine.h"

static uint16_t dma_format(JxEventContext *c,JxFarPointer descriptor) {
    return jx_far_word(c,descriptor.segment,(uint16_t)(descriptor.offset+2));
}
static void dma_out(JxEventContext *c,uint16_t port,uint16_t value) {
    (void)JX_CALL(c,0x1000,0x2b5e,port,value);
}
/* 1F2A:16B0. Unsupported DMA channels leave three original local words
 * uninitialized. Their explicit incoming stack values preserve that behavior
 * without reading indeterminate C variables or inventing a default channel. */
void jx_sound_dma_setup(JxEventContext *c,uint32_t physical,JxFarPointer descriptor,
    uint16_t length,JxSoundDmaPorts incoming_stack) {
    JxSoundDmaPorts ports=incoming_stack;
    uint16_t page=(uint16_t)(physical>>16),address=(uint16_t)physical;
    uint16_t count;
    if (dma_format(c,descriptor)<4) {
        switch (jx_word(c,0x31ba)) {
        case 0:ports=(JxSoundDmaPorts){0,1,0x87};break;
        case 1:ports=(JxSoundDmaPorts){2,3,0x83};break;
        case 3:ports=(JxSoundDmaPorts){6,7,0x82};break;
        }
        dma_out(c,10,(uint16_t)(jx_word(c,0x31ba)|4));
        dma_out(c,12,0);
        dma_out(c,11,(uint16_t)(jx_word(c,0x31ba)|0x58));
        count=(uint16_t)(length-1);
    } else {
        switch (jx_word(c,0x31bc)) {
        case 5:ports=(JxSoundDmaPorts){0xc4,0xc6,0x8b};break;
        case 6:ports=(JxSoundDmaPorts){0xc8,0xca,0x89};break;
        case 7:ports=(JxSoundDmaPorts){0xcc,0xce,0x8a};break;
        }
        address=(uint16_t)((address>>1)|((page&1)<<15));
        dma_out(c,0xd4,(uint16_t)((uint16_t)(jx_word(c,0x31bc)-4)|4));
        dma_out(c,0xd8,0);
        dma_out(c,0xd6,(uint16_t)((uint16_t)(jx_word(c,0x31bc)-4)|0x58));
        count=(uint16_t)(length/2-1);
    }
    dma_out(c,ports.count,(uint16_t)(count&255));
    dma_out(c,ports.count,(uint16_t)(count>>8));
    dma_out(c,ports.page,page);
    dma_out(c,ports.address,(uint16_t)(address&255));
    /* The original SAR passes a sign-extended high byte to the port helper. */
    dma_out(c,ports.address,(uint16_t)((address>>8)|((address&0x8000)?0xff00:0)));
    if (dma_format(c,descriptor)<4) dma_out(c,10,jx_word(c,0x31ba));
    else dma_out(c,0xd4,(uint16_t)(jx_word(c,0x31bc)-4));
}
