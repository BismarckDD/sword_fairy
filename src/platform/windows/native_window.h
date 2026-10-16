#ifndef JX_NATIVE_WINDOW_H
#define JX_NATIVE_WINDOW_H
#include "video_model.h"
#include "legacy_vga_model.h"
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
    uint8_t pressed[128];
} JxNativeWindow;
int jx_native_window_open(JxNativeWindow *);
int jx_native_window_pump(JxNativeWindow *);
void jx_native_window_close(JxNativeWindow *);
int jx_native_window_post_scan(JxNativeWindow *,uint8_t);
int jx_native_window_post_focus_loss(JxNativeWindow *);
int jx_native_window_post_close(JxNativeWindow *);
#endif
