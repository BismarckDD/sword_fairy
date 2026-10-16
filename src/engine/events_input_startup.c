#include "event_engine.h"

/* 1F2A:6D46. Initialization is skipped completely when already installed. */
void jx_keyboard_install_vector(JxEventContext *c) {
    if(jx_word(c,0x04f2)!=0)return;
    uint32_t vector=JX_CALL(c,0x1000,0x3302,9);
    jx_set_word(c,0x294e,(uint16_t)vector);jx_set_word(c,0x2950,(uint16_t)(vector>>16));
    (void)JX_CALL(c,0x1000,0x3314,9,0x7038,0x1f2a);
    jx_set_word(c,0x04f2,1);
    for(unsigned i=0;i<4;i++) {
        jx_set_word(c,(uint16_t)(0x293a+2*i),0);
        jx_set_word(c,(uint16_t)(0x2942+2*i),0xffff);
    }
}
/* 2F28:A36E, install the software timer vector without host IRQ access. */
void jx_timer_install_vector(JxEventContext *c) {
    if(jx_word(c,0x0e5a)!=0)return;
    uint32_t vector=JX_CALL(c,0x1000,0x3302,0x1c);
    jx_set_word(c,0x2b7a,(uint16_t)vector);jx_set_word(c,0x2b7c,(uint16_t)(vector>>16));
    (void)JX_CALL(c,0x1000,0x3314,0x1c,0xa4bc,0x2f28);
    jx_set_word(c,0x0e5a,1);
}
/* 2F28:A3D0. The five slots contain period, remaining, active, enabled.
 * Any nonzero active byte occupies a slot. Preserve wrapping live count. */
uint16_t jx_timer_add_callback(JxEventContext *c,uint16_t period) {
    unsigned index=0;
    while(index<5 && jx_ds(c)[0x2b5c+6*index+4]!=0)index++;
    if(index==5)return 0xffff;
    uint16_t at=(uint16_t)(0x2b5c+6*index);
    jx_set_word(c,at,period);jx_set_word(c,(uint16_t)(at+2),period);
    jx_ds(c)[at+4]=1;jx_ds(c)[at+5]=1;
    jx_set_word(c,0x0e58,(uint16_t)(jx_word(c,0x0e58)+1));
    if(jx_word(c,0x0e5a)==0)(void)JX_CALL(c,0x2f28,0xa36e);
    return (uint16_t)index;
}
/* 1F2A:5F60, BIOS typematic settings precede vector installation. */
void jx_keyboard_initialize(JxEventContext *c,JxKeyboardBiosService bios,
    void *opaque) {
    bios(opaque,0x0305,0);
    (void)JX_CALL(c,0x1f2a,0x6d46);
}
/* 1F2A:5F78, saved callback index also preserves the FFFF no-slot result. */
void jx_timer_initialize(JxEventContext *c) {
    (void)JX_CALL(c,0x2f28,0xa36e);
    jx_set_word(c,0x6656,(uint16_t)JX_CALL(c,0x2f28,0xa3d0,1));
}
