#include "event_engine.h"
/* These callbacks use fixed resident segment 1832 for driver state. */
void jx_graphics_release_ems(JxEventContext *c,const JxRegisteredCleanupDevice *device) {
    device->release_ems(device->opaque,jx_far_word(c,0x1832,0x21));
}
void jx_graphics_release_xms(JxEventContext *c,const JxRegisteredCleanupDevice *device) {
    uint16_t handle=jx_far_word(c,0x1832,0x23);
    JxFarPointer driver={jx_far_word(c,0x1832,0x31),jx_far_word(c,0x1832,0x33)};
    device->xms(device->opaque,driver,0x0a,handle);
}
/* 1832:02B7, registered CRT callback. Its immediate 39E3 is retained. */
void jx_graphics_exit_vectors(JxEventContext *c,const JxRegisteredCleanupDevice *device) {
    JxFarPointer global={0,0x39e3};
    if(jx_far_byte(c,global,0x1343)==2) {
        uint8_t vector=jx_far_byte(c,global,0x1344);
        JxFarPointer handler={jx_far_word(c,global.segment,0x1345),jx_far_word(c,global.segment,0x1347)};
        device->set_vector(device->opaque,vector,handler);
    }
    if((jx_far_word(c,0x1832,0x1d)&2)!=0)jx_graphics_release_ems(c,device);
    if((jx_far_word(c,0x1832,0x1d)&4)!=0)jx_graphics_release_xms(c,device);
}
/* 1832:309E. The saved byte is ORed whole, not masked to bit zero. */
void jx_graphics_restore_bios_flag(JxEventContext *c) {
    if(jx_ds(c)[0x1318]!=0 || (jx_ds(c)[0x2cc6]&12)==0)return;
    c->memory[0x487]=(uint8_t)((c->memory[0x487]&0xfe)|jx_ds(c)[0x2cb8]);
}
