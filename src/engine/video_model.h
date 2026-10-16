#ifndef JX_VIDEO_MODEL_H
#define JX_VIDEO_MODEL_H
#include "event_engine.h"
/* Host-side model of the observed byte transfers, NOT recovered game code.
 * Preconditions: unchained planar byte access, write modes 0/1, read mode 0,
 * no rotate/logical operation/set-reset, full bit mask. No VGA setup is
 * simulated. Unsupported transactions set a sticky error; discard the
 * result and stop the host operation on error. No further writes occur.
 */
#define JX_VIDEO_MODEL_VERSION 2
typedef enum {
    JX_VIDEO_MODEL_OK=0,
    JX_VIDEO_MODEL_BAD_PORT=1,
    JX_VIDEO_MODEL_BAD_REGISTER=2,
    JX_VIDEO_MODEL_BAD_ADDRESS=3,
    JX_VIDEO_MODEL_BAD_MODE=4
} JxVideoModelError;
typedef struct {
    uint8_t planes[4][65536];
    uint8_t map_mask,read_map,sequencer_index,graphics_index,crtc_index,start_high;
    JxVideoModelError error;
    uint8_t mode,latches[4];
} JxVideoModel;
void jx_video_model_init(JxVideoModel *);

void jx_video_model_out(void *,uint16_t,uint16_t,unsigned);
uint8_t jx_video_model_in(void *,uint16_t);
JxVideoBus jx_video_model_bus(JxVideoModel *);
void jx_video_model_write(void *,uint32_t,uint8_t);
uint8_t jx_video_model_read(void *,uint32_t);
JxVideoDevice jx_video_model_writer(JxVideoModel *);
JxVideoReadDevice jx_video_model_reader(JxVideoModel *);
/* Host adapter for the specific BIOS13h -> recovered unchained setup.
 * Does not implement chained-mode rendering or arbitrary VGA registers.
 * The embedded v2 model and its established ABI remain unchanged. */
typedef struct {
    JxVideoModel planar;
    uint8_t sequencer[5],graphics[9],crtc[25];
} JxVideoStartupModel;
void jx_video_startup_model_init(JxVideoStartupModel *);
void jx_video_startup_model_bios(void *,uint16_t);
JxVideoBus jx_video_startup_model_bus(JxVideoStartupModel *);
#endif
