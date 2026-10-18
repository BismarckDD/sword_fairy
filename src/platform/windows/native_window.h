#ifndef JX_NATIVE_WINDOW_H
#define JX_NATIVE_WINDOW_H
#include "video_model.h"
#include "event_engine.h"
#include "legacy_vga_model.h"
#include "native_hd.h"
typedef struct {
    void *handle,*opaque;
    JxVideoModel *video;
    const uint8_t *palette;
    void (*scan)(void *,uint8_t);
    void (*tick)(void *);
    JxLegacyVgaModel *legacy;
    const uint16_t *mode;
    uint32_t pixels[640*480];
    int closed;
    unsigned paints,ticks,planar_paints;
    uint64_t tick_origin,retrace_frame;
    uint64_t retrace_origin,retrace_frequency;
    int timer_resolution;
    uint8_t pressed[128];
    uint32_t save_scene[160*100];
    int save_scene_valid;
    JxHd *hd;
    int hd_disabled,hd_suspended;
    void *back_dc,*back_bitmap,*back_original;
    int back_width,back_height;
    unsigned buffered_paints;
} JxNativeWindow;
int jx_native_window_open(JxNativeWindow *);
int jx_native_window_pump(JxNativeWindow *);
void jx_native_window_sync_ticks(JxNativeWindow *);
int jx_native_window_wait_retrace(JxNativeWindow *);
void jx_native_window_present(JxNativeWindow *);
void jx_native_window_close(JxNativeWindow *);
int jx_native_window_post_scan(JxNativeWindow *,uint8_t);
int jx_native_window_post_focus_loss(JxNativeWindow *);
int jx_native_window_post_close(JxNativeWindow *);
int jx_native_save_browser(JxNativeWindow *,uint8_t *,int);
int jx_native_game_save_browser(JxNativeWindow *,JxEventContext *,int);
#endif
