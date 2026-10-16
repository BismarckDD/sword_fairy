#include "event_engine.h"

/* 1000:0581. Runtime errors bypass stdio and write directly to handle 2. */
uint16_t jx_crt_write_error(JxEventContext *c,uint16_t code,
    JxDosWriteService write,JxCrtErrorHook hook,void *opaque) {
    uint16_t offset=jx_crt_error_message(c,code);
    if(offset==0)return 0;
    uint16_t cursor=offset,remaining=0xffff;
    do {
        uint8_t value=jx_ds(c)[cursor];cursor=(uint16_t)(cursor+1);
        remaining=(uint16_t)(remaining-1);
        if(value==0)break;
    } while(remaining!=0);
    uint16_t count=(uint16_t)((uint16_t)~remaining-1),handle=2;
    if(jx_word(c,0x18ea)==0xd6d6)
        hook(opaque,jx_word(c,0x18ec),&handle,&offset,&count);
    JxDosWriteResult result={0,0};
    write(opaque,handle,(JxFarPointer){offset,c->data_segment},count,&result);
    return result.ax;
}
/* 1000:0272 near. The overlay hook uses BX/CX rather than stack parameters. */
void jx_crt_restore_system(JxEventContext *c,const JxCrtExitDevice *device) {
    uint16_t segment=jx_word(c,0x18fa);
    if(segment!=0)device->overlay_cleanup(device->opaque,
        (JxFarPointer){jx_word(c,0x18f8),segment},2,segment);
    (void)jx_dos_set_vector(device->set_vector,device->opaque,0,
        (JxFarPointer){jx_word(c,0x12fe),jx_word(c,0x1300)});
}
/* 1000:01EB/01F2/01FA/0204 share this sequence, with mode words
 * 0000/0001/0100/0101. TERMINATED must stop the host's game execution. */
JxCrtExitResult jx_crt_exit(JxEventContext *c,uint16_t status,uint16_t mode,
    const JxCrtExitDevice *device) {
    jx_ds(c)[0x1349]=(uint8_t)(mode>>8);
    if((uint8_t)mode==0) {
        jx_crt_run_exit_callbacks(c,0x2eaa,0x2eaa);
        jx_crt_run_exit_callbacks(c,0x1910,0x1914);
        if(jx_word(c,0x18ea)==0xd6d6)
            (void)JX_CALL0(c,0x1000,jx_word(c,0x18f0));
    }
    jx_crt_run_exit_callbacks(c,0x1914,0x1914);
    jx_crt_run_exit_callbacks(c,0x1914,0x191c);
    uint16_t check=jx_crt_check_data(c);
    if(check!=0 && (uint8_t)(mode>>8)==0 && status==0)status=0xff;
    jx_crt_restore_system(c,device);
    if((uint8_t)(mode>>8)==0) {
        device->terminate(device->opaque,(uint8_t)status);
        return (JxCrtExitResult){JX_CRT_EXIT_TERMINATED,status};
    }
    return (JxCrtExitResult){JX_CRT_EXIT_RETURNED,status};
}
JxCrtExitResult jx_crt_exit_quick(JxEventContext *c,uint16_t status,const JxCrtExitDevice *device) {
    return jx_crt_exit(c,status,1,device);
}
JxCrtExitResult jx_crt_cleanup(JxEventContext *c,const JxCrtExitDevice *device) {
    return jx_crt_exit(c,0,0x100,device);
}
JxCrtExitResult jx_crt_cleanup_quick(JxEventContext *c,const JxCrtExitDevice *device) {
    return jx_crt_exit(c,0,0x101,device);
}
