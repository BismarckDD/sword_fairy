#include "event_engine.h"

/* 1000:32DA. DOS returns the allocation segment in AX on success and
 * largest available paragraph count in BX on failure. The output word is
 * written before the shared 05E6/0606 error mapping; failure returns AL. */
uint16_t jx_dos_allocate_paragraphs(JxEventContext *c,uint16_t paragraphs,
    JxFarPointer output,JxDosParagraphService service,void *opaque) {
    JxDosParagraphResult result={0,0,0};service(opaque,paragraphs,&result);
    uint16_t value=result.carry?result.bx:result.ax;
    uint32_t at=((uint32_t)output.segment*16+output.offset)&0xfffff;
    c->memory[at]=(uint8_t)value;c->memory[(at+1)&0xfffff]=(uint8_t)(value>>8);
    if(!result.carry)return 0;
    (void)jx_dos_error(c,result.ax);return (uint8_t)result.ax;
}

/* 1000:21E5, near register helper: AX is the unmasked permission word,
 * CX becomes DOS readonly attributes (0 when bit7 is enabled, else1). */
JxPermissionMaskResult jx_file_mask_permissions(JxEventContext *c,uint16_t permissions) {
    uint16_t masked=(uint16_t)(permissions&(uint16_t)~jx_word(c,0x130e));
    return (JxPermissionMaskResult){masked,(masked&0x80)?0:1};
}
