#include "event_engine.h"
static JxFarPointer portrait_buffer(JxEventContext *c) {
    uint32_t offset=(uint32_t)jx_word(c,0x2eb0)+0xe998;
    return (JxFarPointer){(uint16_t)offset,(uint16_t)(jx_word(c,0x2eb2)+(offset>>16)*0x1000)};
}
/* 1F2A:752E. File pointer is local, image pointer is reread after callbacks.
 * Original ignores seek/read/close errors and draws even after a short read. */
static JxWriteFlow portrait_draw(JxEventContext *c,uint16_t x,uint16_t y,int16_t portrait,const JxStdioServices *services) {
    if(portrait<0) return JX_WRITE_RETURNED;
    uint32_t file=services?jx_fopen_dos(c,(JxFarPointer){0x0543,c->data_segment},(JxFarPointer){0x0540,c->data_segment},services->open):
        JX_CALL(c,0x1000,0x0738,0x0543,c->data_segment,0x0540,c->data_segment);
    if(file==0) return JX_WRITE_RETURNED;
    uint32_t offset=(uint32_t)portrait*4200u;
    if(services) {
        JxFileWriteResult seek=jx_fseek_dos(c,(uint16_t)file,offset,0,services->close->output);
        if(seek.flow!=JX_WRITE_RETURNED) return seek.flow;
    } else (void)JX_CALL(c,0x1000,0x27fa,(uint16_t)file,(uint16_t)(file>>16),(uint16_t)offset,(uint16_t)(offset>>16),0);
    JxFarPointer buffer=portrait_buffer(c);
    if(services) {
        (void)jx_fread_dos(c,buffer,1,4200,(uint16_t)file,services->read);
        JxFileWriteResult closed=jx_fclose_dos(c,(uint16_t)file,services->close_workspace,services->close);
        if(closed.flow!=JX_WRITE_RETURNED) return closed.flow;
    } else {
        (void)JX_CALL(c,0x1000,0x0754,buffer.offset,buffer.segment,1,4200,(uint16_t)file,(uint16_t)(file>>16));
        (void)JX_CALL(c,0x1000,0x0636,(uint16_t)file,(uint16_t)(file>>16));
    }
    buffer=portrait_buffer(c);
    (void)JX_CALL(c,0x1832,0x1a28,x,y,60,70,buffer.offset,buffer.segment,255);
    return JX_WRITE_RETURNED;
}
void jx_portrait_draw(JxEventContext *c,uint16_t x,uint16_t y,int16_t portrait) {(void)portrait_draw(c,x,y,portrait,NULL);}
JxWriteFlow jx_portrait_draw_dos(JxEventContext *c,uint16_t x,uint16_t y,int16_t portrait,const JxStdioServices *services) {return portrait_draw(c,x,y,portrait,services);}
