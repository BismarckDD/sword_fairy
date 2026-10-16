#ifndef JX_LEGACY_VGA_MODEL_H
#define JX_LEGACY_VGA_MODEL_H
#include "video_model.h"
/* Host device model, separate from recovered game code and the established
 * v2 ABI. Unchained A000 aperture, VGA write modes0..3/read modes0..1.
 * Address chaining and odd/even addressing are not implemented. */
typedef struct {
    JxVideoModel *planar;
    uint8_t sequencer[5],graphics[9],crtc[25];
} JxLegacyVgaModel;
void jx_legacy_vga_model_init(JxLegacyVgaModel *,JxVideoModel *);
void jx_legacy_vga_model_out(void *,uint16_t,uint16_t,unsigned);
uint8_t jx_legacy_vga_model_in(void *,uint16_t);
uint8_t jx_legacy_vga_model_read(void *,uint32_t);
void jx_legacy_vga_model_write(void *,uint32_t,uint8_t);
JxVideoBus jx_legacy_vga_model_bus(JxLegacyVgaModel *);
uint8_t jx_legacy_vga_model_pixel(const JxLegacyVgaModel *,uint16_t,uint16_t);
#endif
