#include "event_engine.h"

/* 1F2A:6E8E / 2F28:A3AA. Restore the captured interrupt vectors. */
void jx_keyboard_restore_vector(JxEventContext *c) {
    if(jx_word(c,0x04f2)!=0)
        (void)JX_CALL(c,0x1000,0x3314,9,jx_word(c,0x294e),jx_word(c,0x2950));
}
void jx_timer_restore_vector(JxEventContext *c) {
    if(jx_word(c,0x0e5a)!=0)
        (void)JX_CALL(c,0x1000,0x3314,0x1c,jx_word(c,0x2b7a),jx_word(c,0x2b7c));
}
/* 2F28:A468. Five six-byte timer slots; no count repair or saturation. */
void jx_timer_remove_callback(JxEventContext *c,uint16_t slot) {
    if(jx_signed_word(slot)<0 || jx_signed_word(slot)>=5)return;
    uint16_t active=(uint16_t)(0x2b5c+6*slot+4);
    if(jx_ds(c)[active]==0)return;
    jx_ds(c)[active]=0;
    uint16_t remaining=(uint16_t)(jx_word(c,0x0e58)-1);
    jx_set_word(c,0x0e58,remaining);
    if(remaining==0)(void)JX_CALL(c,0x2f28,0xa3aa);
}
/* 2F28:A562. Pass the complete AX to INT 10, including non-mode functions. */
void jx_bios_video_call(const JxCleanupDevice *device,uint16_t ax) {
    device->bios_video(device->opaque,ax);
}
/* 1F2A:2602. IRQ is signed and re-read after the secondary PIC write.
 * CLI/STI belong to the emulated device, never to the host process. */
void jx_sound_restore_irq(JxEventContext *c,JxFarPointer descriptor,
    const JxCleanupDevice *device) {
    device->interrupt_enable(device->opaque,0);
    (void)JX_CALL(c,0x1000,0x2b5e,0x21,jx_word(c,0x5862));
    uint16_t vector;
    if(jx_signed_word(jx_far_word(c,descriptor.segment,(uint16_t)(descriptor.offset+8)))>7) {
        (void)JX_CALL(c,0x1000,0x2b5e,0xa1,jx_word(c,0x5864));
        vector=(uint16_t)(jx_far_word(c,descriptor.segment,(uint16_t)(descriptor.offset+8))+0x68);
    } else vector=(uint16_t)(jx_far_word(c,descriptor.segment,(uint16_t)(descriptor.offset+8))+8);
    (void)JX_CALL(c,0x1000,0x3314,vector,jx_word(c,0x5146),jx_word(c,0x5148));
    device->interrupt_enable(device->opaque,1);
}
