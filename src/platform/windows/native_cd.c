#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "native_cd.h"
static uint64_t cd_clock(JxNativeCd *cd) {
    return cd->clock_milliseconds?cd->clock_milliseconds(cd->clock_opaque):GetTickCount64();
}
int jx_native_cd_open(JxNativeCd *cd,const char *cue_name) {
    FILE *cue=fopen(cue_name,"r");if(!cue)return 0;
    char line[512],binary[256]={0},mode[32];unsigned track=0;uint8_t seen[100]={0};
    while(fgets(line,sizeof line,cue)){
        unsigned candidate,m,s,f;
        if(sscanf(line," FILE \"%255[^\"]\" BINARY",binary)==1)continue;
        if(sscanf(line," TRACK %u %31s",&candidate,mode)==2){
            if(strcmp(mode,"AUDIO") || (track && !seen[track])){fclose(cue);return 0;}
            if(!candidate || candidate>=100 || candidate!=cd->last+1){fclose(cue);return 0;}
            track=candidate;if(!cd->first)cd->first=track;cd->last=track;continue;
        }
        if(sscanf(line," INDEX 01 %u:%u:%u",&m,&s,&f)==3){
            if(!track || seen[track] || m>99 || s>=60 || f>=75){fclose(cue);return 0;}
            seen[track]=1;cd->tracks[track]=m*4500+s*75+f;
            if(track>cd->first && cd->tracks[track]<=cd->tracks[track-1]){fclose(cue);return 0;}
        }
    }
    fclose(cue);
    if(!cd->first || !seen[cd->last] || !binary[0] || strchr(binary,'/') || strchr(binary,'\\') || strchr(binary,':'))return 0;
    const char *slash=strrchr(cue_name,'/');size_t prefix=slash?(size_t)(slash-cue_name+1):0;
    char path[512];if(snprintf(path,sizeof path,"%.*s%s",(int)prefix,cue_name,binary)>=(int)sizeof path)return 0;
    cd->image=fopen(path,"rb");if(!cd->image)return 0;
    if(fseek(cd->image,0,SEEK_END)){jx_native_cd_close(cd);return 0;}
    long bytes=ftell(cd->image);
    if(bytes<=0 || bytes%2352){jx_native_cd_close(cd);return 0;}
    cd->sectors=(uint32_t)bytes/2352;
    if(cd->tracks[cd->last]>=cd->sectors){jx_native_cd_close(cd);return 0;}
    return 1;
}
void jx_native_cd_close(JxNativeCd *cd) {
    jx_native_audio_close(&cd->audio);
    if(cd->image)fclose(cd->image);cd->image=NULL;cd->state=0;
}
int jx_native_cd_read(JxNativeCd *cd,uint32_t start,uint32_t count,uint8_t *data) {
    if(!cd->image || start>cd->sectors || count>cd->sectors-start)return 0;
    return fseek(cd->image,(long)(start*2352),SEEK_SET)==0 && fread(data,2352,count,cd->image)==count;
}
uint32_t jx_native_cd_position(JxNativeCd *cd) {
    uint64_t elapsed=cd->elapsed;
    if(cd->state==1)elapsed+=(cd_clock(cd)-cd->started)*75/1000;
    if(elapsed>=cd->length){elapsed=cd->length;if(cd->state==1)cd->state=0;}
    return cd->start+(uint32_t)elapsed;
}
int jx_native_cd_play(JxNativeCd *cd,uint32_t start,uint32_t count,int output) {
    if(!cd->image || start>cd->sectors || count>cd->sectors-start)return 0;
    jx_native_audio_close(&cd->audio);
    if(output && count){
        size_t size=(size_t)count*2352;uint8_t *data=malloc(size);if(!data)return 0;
        int ok=jx_native_cd_read(cd,start,count,data) && jx_native_audio_play(&cd->audio,data,(uint32_t)size,44100,2,16);
        free(data);if(!ok)return 0;
    }
    cd->start=start;cd->length=count;cd->elapsed=0;cd->started=cd_clock(cd);cd->state=count?1:0;cd->plays++;return 1;
}
int jx_native_cd_stop(JxNativeCd *cd) {
    if(cd->state==1){cd->elapsed=jx_native_cd_position(cd)-cd->start;if(cd->state!=1)return 1;if(!jx_native_audio_pause(&cd->audio))return 0;cd->state=2;}
    return cd->image!=NULL;
}
int jx_native_cd_resume(JxNativeCd *cd) {
    if(!cd->image)return 0;
    if(cd->state==2){if(!jx_native_audio_resume(&cd->audio))return 0;cd->started=cd_clock(cd);cd->state=1;}
    return 1;
}
