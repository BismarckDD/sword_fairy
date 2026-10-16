#include "event_engine.h"
/* 1832:2790. Shared packet storage makes the surrounding API non-reentrant. */
uint16_t jx_cd_send_request(const JxCdDevice *device,uint16_t drive,JxFarPointer packet) {
    return device->request(device->opaque,drive,packet);
}
static uint16_t cd_status(JxEventContext *c,uint16_t packet) {
    return (jx_ds(c)[(uint16_t)(packet+4)]&128)?(uint16_t)(jx_ds(c)[(uint16_t)(packet+3)]+16):0;
}
static void cd_header(JxEventContext *c,uint16_t packet,uint8_t length,uint8_t unit,uint8_t command) {
    jx_ds(c)[packet]=length;jx_ds(c)[(uint16_t)(packet+1)]=unit;
    jx_ds(c)[(uint16_t)(packet+2)]=command;jx_set_word(c,(uint16_t)(packet+3),0);
}
static uint16_t cd_simple(JxEventContext *c,const JxCdDevice *device,uint8_t unit,uint8_t command) {
    cd_header(c,0x2bd6,13,unit,command);
    (void)jx_cd_send_request(device,jx_word(c,0x2bd4),(JxFarPointer){0x2bd6,c->data_segment});
    return cd_status(c,0x2bd6);
}
/* 1832:2A84 / 2ACE. Preserve the original status low-byte + 16 mapping. */
uint16_t jx_cd_stop(JxEventContext *c,const JxCdDevice *device,uint8_t unit) {return cd_simple(c,device,unit,0x85);}
uint16_t jx_cd_resume(JxEventContext *c,const JxCdDevice *device,uint8_t unit) {return cd_simple(c,device,unit,0x88);}
/* 1832:2A1A. Start is MSF or sector addressing according to mode; length
 * remains a frame count. Reserved packet bytes retain their old values. */
uint16_t jx_cd_play(JxEventContext *c,const JxCdDevice *device,uint8_t unit,uint32_t start,uint32_t length,uint8_t mode) {
    cd_header(c,0x2bd6,22,unit,0x84);jx_ds(c)[0x2be3]=mode;
    jx_set_word(c,0x2be4,(uint16_t)start);jx_set_word(c,0x2be6,(uint16_t)(start>>16));
    jx_set_word(c,0x2be8,(uint16_t)length);jx_set_word(c,0x2bea,(uint16_t)(length>>16));
    (void)jx_cd_send_request(device,jx_word(c,0x2bd4),(JxFarPointer){0x2bd6,c->data_segment});
    return cd_status(c,0x2bd6);
}
/* 1832:2B18, command 03 IOCTL input. Order matters when data aliases packet. */
uint16_t jx_cd_ioctl_input(JxEventContext *c,const JxCdDevice *device,uint8_t unit,JxFarPointer data,uint8_t code,uint8_t length) {
    cd_header(c,0x2bec,26,unit,3);jx_ds(c)[0x2bf9]=0;
    jx_set_word(c,0x2bfa,data.offset);jx_set_word(c,0x2bfc,data.segment);
    jx_far_set_byte(c,data,0,code);
    jx_set_word(c,0x2bfe,length);jx_set_word(c,0x2c00,0);
    jx_set_word(c,0x2c04,0);jx_set_word(c,0x2c02,0);
    (void)jx_cd_send_request(device,jx_word(c,0x2bd4),(JxFarPointer){0x2bec,c->data_segment});
    return cd_status(c,0x2bec);
}
/* 1832:2D10. */
uint16_t jx_cd_audio_position(JxEventContext *c,const JxCdDevice *device,uint8_t unit,JxFarPointer data) {
    return jx_cd_ioctl_input(c,device,unit,data,12,11);
}
/* 1832:296E defines AL only, including non-decimal BCD nibbles. */
uint8_t jx_bcd_to_binary(uint8_t value) {return (uint8_t)((value&15)+(value>>4)*10);}
/* 1832:292E / 298C. No implicit 150-frame origin adjustment. */
uint32_t jx_cd_msf_to_frames(uint32_t value) {
    return ((value>>16)&255)*4500+((value>>8)&255)*75+(value&255);
}
uint32_t jx_cd_frames_to_msf(uint32_t value) {
    return ((uint32_t)(uint16_t)(value/4500)<<16)|((value%4500/75)<<8)|(value%75);
}
uint16_t jx_cd_disc_info(JxEventContext *c,const JxCdDevice *device,uint8_t unit,JxFarPointer data) {
    return jx_cd_ioctl_input(c,device,unit,data,10,7);
}
static void cd_far_set_word(JxEventContext *c,JxFarPointer p,uint16_t value) {
    uint32_t at=(((uint32_t)p.segment<<4)+p.offset)&0xfffffu;
    c->memory[at]=(uint8_t)value;c->memory[(at+1)&0xfffffu]=(uint8_t)(value>>8);
}
static void cd_far_set_long(JxEventContext *c,JxFarPointer p,uint32_t value) {
    cd_far_set_word(c,p,(uint16_t)value);p.offset+=2;cd_far_set_word(c,p,(uint16_t)(value>>16));
}
static uint32_t cd_far_long(JxEventContext *c,JxFarPointer p) {
    return jx_far_word(c,p.segment,p.offset)|((uint32_t)jx_far_word(c,p.segment,(uint16_t)(p.offset+2))<<16);
}
/* 1832:2C10. The duplicate disc-info query and unusual first-status return
 * on second-query failure are intentional original behavior. */
uint16_t jx_cd_track_table(JxEventContext *c,const JxCdDevice *device,uint8_t unit,JxFarPointer table,JxFarPointer scratch) {
    JxFarPointer info={0x2bbe,c->data_segment};
    uint16_t first=jx_cd_ioctl_input(c,device,unit,info,10,7);
    if(jx_cd_disc_info(c,device,unit,info)) return first;
    uint16_t last=jx_ds(c)[0x2bc0];
    JxFarPointer end={(uint16_t)(table.offset+last*4),table.segment};
    cd_far_set_long(c,end,cd_far_long(c,(JxFarPointer){0x2bc1,c->data_segment}));
    for(uint16_t track=1;track<=jx_ds(c)[0x2bc0];track++) {
        jx_far_set_byte(c,scratch,1,(uint8_t)track);
        uint16_t status=jx_cd_ioctl_input(c,device,unit,scratch,11,7);
        if(status&0x8000) return (uint8_t)status;
        cd_far_set_long(c,table,cd_far_long(c,(JxFarPointer){(uint16_t)(scratch.offset+2),scratch.segment}));
        table.offset+=4;
    }
    return 0;
}
/* 1832:2D80. Requested -1 means all remaining frames, otherwise unsigned min.
 * Track-table errors are ignored and leave private table bytes live. */
uint16_t jx_cd_play_track(JxEventContext *c,const JxCdDevice *device,uint8_t unit,uint8_t track,
    uint32_t offset,uint32_t requested,JxFarPointer table,JxFarPointer scratch) {
    (void)jx_cd_track_table(c,device,unit,table,scratch);
    JxFarPointer entry={(uint16_t)(table.offset+(uint16_t)(track*4-4)),table.segment};
    uint32_t start=jx_cd_msf_to_frames(cd_far_long(c,entry))+offset;
    uint32_t remaining=jx_cd_msf_to_frames(cd_far_long(c,(JxFarPointer){0x2bc1,c->data_segment}))-start;
    uint32_t count=requested==0xffffffffu || requested>=remaining?remaining:requested;
    return jx_cd_play(c,device,unit,jx_cd_frames_to_msf(start),count,1);
}
/* 1832:224C, frequently called by events. Seconds are unsigned words; only
 * FFFF duration means all remaining frames. The BCD conversion is discarded. */
uint16_t jx_cd_play_seconds(JxEventContext *c,const JxCdDevice *device,uint8_t track,
    uint16_t start_seconds,uint16_t duration_seconds,JxFarPointer table,JxFarPointer scratch) {
    (void)jx_cd_stop(c,device,jx_ds(c)[0x1823]);
    uint16_t status=jx_cd_audio_position(c,device,jx_ds(c)[0x1823],(JxFarPointer){0x2bb2,c->data_segment});
    if(status&0x8000) return status;
    return jx_cd_play_track(c,device,jx_ds(c)[0x1823],track,(uint32_t)start_seconds*75,
        duration_seconds==65535?0xffffffffu:(uint32_t)duration_seconds*75,table,scratch);
}
/* 1832:2D48, audio-status IOCTL. */
uint16_t jx_cd_audio_status(JxEventContext *c,const JxCdDevice *device,uint8_t unit,JxFarPointer data) {
    return jx_cd_ioctl_input(c,device,unit,data,15,11);
}
/* 1832:2180 always copies the status word, including on request error. */
uint16_t jx_cd_current_status(JxEventContext *c,const JxCdDevice *device,JxFarPointer output) {
    uint16_t status=jx_cd_audio_status(c,device,jx_ds(c)[0x1823],(JxFarPointer){0x2ba0,c->data_segment});
    cd_far_set_word(c,output,jx_word(c,0x2ba1));return status;
}
/* 1832:2212 only copies the nine position bytes after successful query. */
uint16_t jx_cd_current_position(JxEventContext *c,const JxCdDevice *device,JxFarPointer output) {
    uint16_t status=jx_cd_audio_position(c,device,jx_ds(c)[0x1823],(JxFarPointer){0x2bb2,c->data_segment});
    if(!status) for(uint16_t i=0;i<9;i++) jx_far_set_byte(c,output,i,jx_ds(c)[(uint16_t)(0x2bb4+i)]);
    return status;
}
/* 1832:22CE intentionally sends stop twice when the first high bit is clear. */
uint16_t jx_cd_stop_current(JxEventContext *c,const JxCdDevice *device) {
    uint16_t status=jx_cd_stop(c,device,jx_ds(c)[0x1823]);
    if(!(status&0x8000)) status=jx_cd_stop(c,device,jx_ds(c)[0x1823]);
    return status;
}
/* 1832:22F2. Private two-byte status; query error itself is ignored. */
uint16_t jx_cd_stop_if_active(JxEventContext *c,const JxCdDevice *device,JxFarPointer scratch) {
    (void)jx_cd_current_status(c,device,scratch);
    return (jx_far_byte(c,scratch,0)&1)?35:jx_cd_stop(c,device,jx_ds(c)[0x1823]);
}
/* 1832:2324, inverse status gate with a distinct no-op return value. */
uint16_t jx_cd_resume_if_paused(JxEventContext *c,const JxCdDevice *device,JxFarPointer scratch) {
    (void)jx_cd_current_status(c,device,scratch);
    return (jx_far_byte(c,scratch,0)&1)?jx_cd_resume(c,device,jx_ds(c)[0x1823]):36;
}
/* 1832:21D8. Copy forward, preserving overlaps, only after success. */
uint16_t jx_cd_current_disc(JxEventContext *c,const JxCdDevice *device,JxFarPointer output) {
    uint16_t status=jx_cd_disc_info(c,device,jx_ds(c)[0x1823],(JxFarPointer){0x2bbe,c->data_segment});
    if(!status) for(uint16_t i=0;i<6;i++) jx_far_set_byte(c,output,i,jx_ds(c)[(uint16_t)(0x2bbf+i)]);
    return status;
}
/* 1832:27A4 / 27BC each perform a fresh INT2F AX=1500, BX=0 query. */
uint16_t jx_cd_first_drive(const JxCdDiscovery *service) {
    JxCdInstallation result={0,0};service->query(service->opaque,&result);return result.first_drive;
}
uint16_t jx_cd_drive_count(const JxCdDiscovery *service) {
    JxCdInstallation result={0,0};service->query(service->opaque,&result);return result.count;
}
/* 1832:27D4: INT2F AX=1501 supplies the list and carry, AX is ignored. */
uint16_t jx_cd_device_list(const JxCdDiscovery *service,JxFarPointer list) {
    return service->list(service->opaque,list)?1:0;
}
/* 1832:2E30. The count written to the output is reloaded after enumeration. */
uint16_t jx_cd_initialize(JxEventContext *c,const JxCdDiscovery *service,JxFarPointer list,JxFarPointer count) {
    uint16_t installed=jx_cd_drive_count(service);jx_set_word(c,0x2bd2,installed);
    if(!installed) return 1;
    jx_set_word(c,0x2bd4,jx_cd_first_drive(service));
    if(jx_cd_device_list(service,list)) return 1;
    cd_far_set_word(c,count,jx_word(c,0x2bd2));return 0;
}
/* 1832:2098 wraps discovery with the game-level error value 35. */
uint16_t jx_cd_initialize_current(JxEventContext *c,const JxCdDiscovery *service,JxFarPointer count) {
    return jx_cd_initialize(c,service,(JxFarPointer){0x2c2e,c->data_segment},count)?35:0;
}
