#include "event_engine.h"
static void output_word(JxEventContext *c,JxFarPointer pointer,uint16_t value) {
    uint32_t address=((uint32_t)pointer.segment*16+pointer.offset)&0xfffff;
    c->memory[address]=(uint8_t)value;c->memory[(address+1)&0xfffff]=(uint8_t)(value>>8);
}
/* 1503:0006 / 001D / 003A / 005D / 007D / 009F.
 * All wrappers zero-extend the returned AH status into AX. Output words
 * are stored even on nonzero status; count outputs retain original order. */
uint16_t jx_ems_status(const JxEmsDevice *ems) {
    uint16_t bx=0,dx=0;return ems->service(ems->opaque,0x40,0,0,0,&bx,&dx);
}
uint16_t jx_ems_frame(JxEventContext *c,const JxEmsDevice *ems,JxFarPointer frame) {
    uint16_t bx=0,dx=0;uint8_t status=ems->service(ems->opaque,0x41,0,0,0,&bx,&dx);
    output_word(c,frame,bx);return status;
}
uint16_t jx_ems_page_counts(JxEventContext *c,const JxEmsDevice *ems,JxFarPointer total,JxFarPointer available) {
    uint16_t bx=0,dx=0;uint8_t status=ems->service(ems->opaque,0x42,0,0,0,&bx,&dx);
    output_word(c,total,dx);output_word(c,available,bx);return status;
}
uint16_t jx_ems_allocate(JxEventContext *c,const JxEmsDevice *ems,uint16_t pages,JxFarPointer handle) {
    uint16_t bx=0,dx=0;uint8_t status=ems->service(ems->opaque,0x43,0,pages,0,&bx,&dx);
    output_word(c,handle,dx);return status;
}
uint16_t jx_ems_map(const JxEmsDevice *ems,uint16_t physical,uint16_t logical,uint16_t handle) {
    uint16_t bx=0,dx=0;return ems->service(ems->opaque,0x44,(uint8_t)physical,logical,handle,&bx,&dx);
}
uint16_t jx_ems_release(const JxEmsDevice *ems,uint16_t handle) {
    uint16_t bx=0,dx=0;return ems->service(ems->opaque,0x45,0,0,handle,&bx,&dx);
}
