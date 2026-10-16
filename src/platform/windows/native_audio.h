#ifndef JX_NATIVE_AUDIO_H
#define JX_NATIVE_AUDIO_H
#include <stdint.h>
typedef struct {void *state;unsigned plays;} JxNativeAudio;
int jx_native_audio_play(JxNativeAudio *,const uint8_t *,uint32_t,uint32_t,uint16_t,uint16_t);
void jx_native_audio_close(JxNativeAudio *);
int jx_native_audio_take_completion(JxNativeAudio *);
int jx_native_audio_pause(JxNativeAudio *);
int jx_native_audio_resume(JxNativeAudio *);
#endif
