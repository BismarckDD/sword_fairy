#include "event_engine.h"
static uint16_t open_error(JxEventContext *c,uint16_t error) {
    (void)jx_dos_error(c,error);return 65535;
}
/* 1000:202A. Reconstruct the original DOS CRT policy, including ignored
 * errors in truncation and text EOF cleanup, rather than host fopen policy. */
uint16_t jx_file_open(JxEventContext *c,JxFarPointer name,uint16_t options,
    uint16_t sharing,uint16_t permissions,const JxOpenServices *s) {
    uint8_t share=jx_ds(c)[0x1315]>=3?(uint8_t)sharing:0;
    uint8_t flags=(!(options&0x8000) && ((options&0x4000)||!(jx_ds(c)[0x15cd]&128)))?128:0;
    uint8_t mode=(uint8_t)((options&3)|share);
    uint16_t attributes=options,handle=0,saved_attributes=0;
    int existed=1,create=0;
    JxDosWriteResult result={0,0};
    s->open(s->opaque,name,mode,&result);
    if(result.carry) {
        if(result.ax!=2 || !(options&0x100)) return open_error(c,result.ax);
        existed=0;create=1;
        /* Near 21E5: only the write-permission bit affects DOS readonly. */
        attributes=jx_file_mask_permissions(c,permissions).attributes;
        saved_attributes=attributes;
        if(share || !(options&2)) attributes&=(uint16_t)~1;
    } else {
        handle=result.ax;
        if((options&0x500)==0x500) {
            s->close(s->opaque,handle,&result);return open_error(c,0x1100);
        }
        JxDosSeekResult info={0,0,0};s->info(s->opaque,handle,&info);
        if(info.dx&128) flags|=64;
        if(!(flags&64)) {
            if(options&0x200) {
                if(options&3) {
                    attributes=0;s->truncate(s->opaque,handle,&result);
                } else {
                    s->close(s->opaque,handle,&result);
                    JxDosAttributeResult attr={0,attributes,0};
                    s->attributes(s->opaque,name,0,attributes,&attr);
                    attributes=attr.attributes;create=1;
                }
            } else if((flags&128) && (options&2)) {
                JxDosSeekResult position={0,0,0};
                s->seek(s->opaque,handle,2,0xffffffffu,&position);
                s->read(s->opaque,handle,s->tail_byte,1,&result);
                if(result.ax && jx_far_byte(c,s->tail_byte,0)==26) {
                    s->seek(s->opaque,handle,2,0xffffffffu,&position);
                    s->truncate(s->opaque,handle,&result);
                }
                s->seek(s->opaque,handle,0,0,&position);attributes=0;
            }
        }
    }
    if(create) {
        s->create(s->opaque,name,attributes,&result);
        if(result.carry) return open_error(c,result.ax);
        handle=result.ax;
        if(share || !(options&2)) {
            s->close(s->opaque,handle,&result);
            s->open(s->opaque,name,mode,&result);
            if(result.carry) return open_error(c,result.ax);
            handle=result.ax;
            if(!existed && (saved_attributes&1)) {
                attributes|=1;
                JxDosAttributeResult attr={0,attributes,0};
                s->attributes(s->opaque,name,1,attributes,&attr);
                if(attr.carry) return open_error(c,attr.ax);
                attributes=attr.attributes;
            }
        }
    }
    uint8_t status=0;
    if(!(flags&64)) {
        JxDosAttributeResult attr={0,attributes,0};
        s->attributes(s->opaque,name,0,attributes,&attr);
        if(attr.attributes&1) status=16;
        if(options&8) status|=32;
    }
    if(handle>=jx_word(c,0x131c)) {
        s->close(s->opaque,handle,&result);return open_error(c,0x1800);
    }
    jx_ds(c)[(uint16_t)(0x131e + handle)]=(uint8_t)(status|flags|1);
    return handle;
}
