#include "event_engine.h"
/* 1000:4A7A. caller_sp is SP after popping this far call's return address. */
uint16_t jx_stack_available(JxEventContext *c,uint16_t caller_sp) {
    uint16_t limit=jx_word(c,0x1352);
    return limit>=caller_sp?0:(uint16_t)(caller_sp-limit);
}
static uint16_t write_error(JxEventContext *c,uint16_t code) {
    (void)jx_dos_error(c,code);return 65535;
}
/* 1000:4A40 shared direct-write tail, not the complete 491A entry. */
uint16_t jx_file_write_binary(JxEventContext *c,uint16_t handle,JxFarPointer source,
    uint16_t count,JxDosWriteService service,void *opaque) {
    if(!count) return 0;
    JxDosWriteResult result={0,0};service(opaque,handle,source,count,&result);
    if(result.carry) return write_error(c,(uint16_t)(0x0900|(result.ax&255)));
    if(result.ax) return result.ax;
    if((jx_ds(c)[(uint16_t)(0x131e + handle)]&64) && jx_far_byte(c,source,0)==0x1a) return 0;
    return write_error(c,0x1c00);
}

static uint16_t text_write_finish(JxEventContext *c,uint16_t total,uint16_t inserted) {
    uint16_t result=(uint16_t)(total-inserted);
    return total<inserted?write_error(c,result):result;
}
/* Flush returns zero to continue, one to return the supplied value. */
static int text_write_flush(JxEventContext *c,uint16_t handle,JxFarPointer source,
    JxFarPointer scratch,uint16_t used,uint16_t *total,uint16_t inserted,
    JxDosWriteService service,void *opaque,uint16_t *value) {
    if(!used) return 0;
    JxDosWriteResult result={0,0};service(opaque,handle,scratch,used,&result);
    if(!result.carry) {
        *total=(uint16_t)(*total+result.ax);
        if(result.ax>=used) return 0;
    }
    if(*total) *value=text_write_finish(c,*total,inserted);
    else if(result.carry) *value=write_error(c,(uint16_t)(0x900|(result.ax&255)));
    else if((jx_ds(c)[(uint16_t)(0x131e + handle)]&64) && jx_far_byte(c,source,0)==26) {
        /* LAHF from CMP used,0 survives the control-Z success branch. */
        unsigned bits=0;for(unsigned v=used&255;v;v>>=1) bits+=v&1;
        *value=(uint16_t)((2|((bits&1)?0:4))<<8);
    } else *value=write_error(c,0x1c00);
    return 1;
}
/* 1000:491A including text translation and 49DA's non-local error exit. */
JxFileWriteResult jx_file_write(JxEventContext *c,uint16_t handle,JxFarPointer source,
    uint16_t count,JxFarPointer scratch,uint16_t available_stack,
    JxDosWriteService write,JxDosSeekService seek,JxWriteHook hook,void *opaque) {
    uint16_t value=0;
    if(handle>=jx_word(c,0x131c)) return (JxFileWriteResult){0,write_error(c,0x900)};
    if(jx_word(c,0x18ea)==0xd6d6) hook(opaque,jx_word(c,0x18ec),handle);
    if(jx_ds(c)[(uint16_t)(0x131e + handle)]&32) {
        JxDosSeekResult result={0,0,0};seek(opaque,handle,2,0,&result);
        if(result.carry) return (JxFileWriteResult){0,write_error(c,result.ax)};
    }
    if(jx_ds(c)[(uint16_t)(0x131e + handle)]&128) {
        if(!count) return (JxFileWriteResult){0,0};
        int newline=0;
        for(unsigned i=0;i<count;i++) if(jx_far_byte(c,source,(uint16_t)i)==10) {newline=1;break;}
        if(newline) {
            if(available_stack<=0xa8) return (JxFileWriteResult){JX_WRITE_STACK_FAILURE,0};
            uint16_t capacity=available_stack<0x228?128:512,total=0,inserted=0,used=0;
            for(unsigned i=0;i<count;i++) {
                uint8_t ch=jx_far_byte(c,source,(uint16_t)i);
                if(ch==10) {
                    if(used==capacity) {
                        if(text_write_flush(c,handle,source,scratch,used,&total,inserted,write,opaque,&value)) return (JxFileWriteResult){0,value};
                        used=0;
                    }
                    jx_far_set_byte(c,scratch,used++,13);inserted++;
                }
                if(used==capacity) {
                    if(text_write_flush(c,handle,source,scratch,used,&total,inserted,write,opaque,&value)) return (JxFileWriteResult){0,value};
                    used=0;
                }
                jx_far_set_byte(c,scratch,used++,ch);
            }
            if(text_write_flush(c,handle,source,scratch,used,&total,inserted,write,opaque,&value)) return (JxFileWriteResult){0,value};
            return (JxFileWriteResult){0,text_write_finish(c,total,inserted)};
        }
    }
    return (JxFileWriteResult){0,jx_file_write_binary(c,handle,source,count,write,opaque)};
}
