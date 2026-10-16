#ifndef JX_NATIVE_CD_H
#define JX_NATIVE_CD_H
#include <stdio.h>
#include "native_audio.h"
typedef struct {
    FILE *image;
    uint32_t tracks[100],sectors,start,length,elapsed;
    unsigned first,last,state,requests,plays;
    uint64_t started;
    uint64_t (*clock_milliseconds)(void *);
    void *clock_opaque;
    JxNativeAudio audio;
} JxNativeCd;
int jx_native_cd_open(JxNativeCd *,const char *);
void jx_native_cd_close(JxNativeCd *);
int jx_native_cd_read(JxNativeCd *,uint32_t,uint32_t,uint8_t *);
int jx_native_cd_play(JxNativeCd *,uint32_t,uint32_t,int);
int jx_native_cd_stop(JxNativeCd *);
int jx_native_cd_resume(JxNativeCd *);
uint32_t jx_native_cd_position(JxNativeCd *);
#endif
