#ifndef _H_VIRTUAL_GRAPHICS_C_H_
#define _H_VIRTUAL_GRAPHICS_C_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VGFX_API
typedef void(VGFX_API *VgfxDrawClearFn)(void *, uint32_t);
typedef void(VGFX_API *VgfxDrawPixelFn)(void *, uint32_t, uint32_t, uint32_t);
typedef void(VGFX_API *VgfxDrawCharFn)(void *, uint32_t, uint32_t, char, uint32_t, uint32_t);

#define VGFX_TRUE 1
#define VGFX_FALSE 0
typedef uint8_t VGFX_BOOL;

#define VGFX_ABS(x) (x < 0 ? x * -1 : x)

typedef struct _VGFX_ZOOM_DATA {
    void *draw_clear_ctx;
    VgfxDrawClearFn draw_clear;
    void *draw_pixel_ctx;
    VgfxDrawPixelFn draw_pixel;
    uint32_t factor;
} VGFX_ZOOM_DATA;

VGFX_BOOL VGFX_API vgzm_init(void *data, uint32_t size);
VGFX_BOOL VGFX_API vgzm_set_zoom(void *data, uint32_t factor);
VGFX_BOOL VGFX_API vgzm_set_draw_clear(void *data, VgfxDrawClearFn func, void *ctx);
VGFX_BOOL VGFX_API vgzm_set_draw_pixel(void *data, VgfxDrawPixelFn func, void *ctx);
VGFX_BOOL VGFX_API vgzm_draw_clear(void *data, uint32_t color);
VGFX_BOOL VGFX_API vgzm_draw_pixel(void *data, uint32_t x, uint32_t y, uint32_t color);
VGFX_BOOL VGFX_API vgzm_fini(void *data);

typedef struct _VGFX_CANVAS_2D_DATA {
    void *draw_clear_ctx;
    VgfxDrawClearFn draw_clear;
    void *draw_pixel_ctx;
    VgfxDrawPixelFn draw_pixel;
    void *draw_char_ctx;
    VgfxDrawCharFn draw_char;
    uint32_t char_width;
    uint32_t char_height;
} VGFX_CANVAS_2D_DATA;

VGFX_BOOL VGFX_API vg2d_init(void *data, uint32_t size);
VGFX_BOOL VGFX_API vg2d_set_draw_clear(void *data, VgfxDrawClearFn func, void *ctx);
VGFX_BOOL VGFX_API vg2d_set_draw_pixel(void *data, VgfxDrawPixelFn func, void *ctx);
VGFX_BOOL VGFX_API vg2d_set_draw_char(void *data, VgfxDrawCharFn func, void *ctx, uint32_t width, uint32_t height);
VGFX_BOOL VGFX_API vg2d_draw_clear(void *data, uint32_t color);
VGFX_BOOL VGFX_API vg2d_draw_pixel(void *data, uint32_t x, uint32_t y, uint32_t color);
VGFX_BOOL VGFX_API vg2d_draw_line(void *data, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color);
VGFX_BOOL VGFX_API vg2d_draw_fast_hline(void *data, uint32_t x0, uint32_t y0, uint32_t length, uint32_t color);
VGFX_BOOL VGFX_API vg2d_draw_fast_vline(void *data, uint32_t x0, uint32_t y0, uint32_t length, uint32_t color);
VGFX_BOOL VGFX_API vg2d_draw_rect(void *data, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t color);
VGFX_BOOL VGFX_API vg2d_fill_rect(void *data, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t color);
VGFX_BOOL VGFX_API vg2d_draw_char(void *data, uint32_t x, uint32_t y, char c, uint32_t color, uint32_t bg);
VGFX_BOOL VGFX_API vg2d_draw_string(void *data, uint32_t x, uint32_t y, const char *s, uint32_t color, uint32_t bg);
VGFX_BOOL VGFX_API vg2d_fini(void *data);

#ifdef __cplusplus
}
#endif

#endif
