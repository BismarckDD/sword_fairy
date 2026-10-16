#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <string.h>
#include "native_audio.h"
typedef struct {HWAVEOUT handle;WAVEHDR header;uint8_t *samples;int completed;} AudioState;
int jx_native_audio_take_completion(JxNativeAudio *audio) {
    AudioState *s=audio->state;
    if(!s || s->completed || !(s->header.dwFlags&WHDR_DONE))return 0;
    s->completed=1;return 1;
}
int jx_native_audio_pause(JxNativeAudio *audio) {
    AudioState *s=audio->state;return !s || waveOutPause(s->handle)==MMSYSERR_NOERROR;
}
int jx_native_audio_resume(JxNativeAudio *audio) {
    AudioState *s=audio->state;return !s || waveOutRestart(s->handle)==MMSYSERR_NOERROR;
}
void jx_native_audio_close(JxNativeAudio *audio) {
    AudioState *s=audio->state;if(!s)return;
    waveOutReset(s->handle);waveOutUnprepareHeader(s->handle,&s->header,sizeof s->header);
    waveOutClose(s->handle);free(s->samples);free(s);audio->state=NULL;
}
int jx_native_audio_play(JxNativeAudio *audio,const uint8_t *samples,uint32_t size,
    uint32_t rate,uint16_t channels,uint16_t bits) {
    jx_native_audio_close(audio);
    if(!size || !rate || (channels!=1 && channels!=2) || (bits!=8 && bits!=16))return 0;
    AudioState *s=calloc(1,sizeof *s);if(!s)return 0;
    WAVEFORMATEX format={0};format.wFormatTag=WAVE_FORMAT_PCM;format.nChannels=channels;
    format.nSamplesPerSec=rate;format.wBitsPerSample=bits;format.nBlockAlign=(WORD)(channels*bits/8);
    format.nAvgBytesPerSec=rate*format.nBlockAlign;
    if(waveOutOpen(&s->handle,WAVE_MAPPER,&format,0,0,CALLBACK_NULL)!=MMSYSERR_NOERROR){free(s);return 0;}
    s->samples=malloc(size);if(!s->samples){waveOutClose(s->handle);free(s);return 0;}
    memcpy(s->samples,samples,size);s->header.lpData=(LPSTR)s->samples;s->header.dwBufferLength=size;
    if(waveOutPrepareHeader(s->handle,&s->header,sizeof s->header)!=MMSYSERR_NOERROR){free(s->samples);waveOutClose(s->handle);free(s);return 0;}
    audio->state=s;
    if(waveOutWrite(s->handle,&s->header,sizeof s->header)!=MMSYSERR_NOERROR){jx_native_audio_close(audio);return 0;}
    audio->plays++;return 1;
}
