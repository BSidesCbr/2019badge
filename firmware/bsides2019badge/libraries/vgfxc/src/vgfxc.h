#ifndef _H_VIRTUAL_GRAPHICS_C_H_
#define _H_VIRTUAL_GRAPHICS_C_H_

#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// need ssize_t
#if SIZE_MAX == UINT_MAX
typedef int ssize_t;        /* common 32 bit case */
typedef int off_t;
#elif SIZE_MAX == ULONG_MAX
typedef long ssize_t;       /* linux 64 bits */
typedef long off_t;
#elif SIZE_MAX == ULLONG_MAX
typedef long long ssize_t;  /* windows 64 bits */
typedef long long off_t;
#elif SIZE_MAX == USHRT_MAX
typedef short ssize_t;      /* think AVR with 32k of flash */
typedef short off_t;
#else
#error platform has exotic SIZE_MAX
#endif

typedef uint32_t vgfx_color_t;

#define VGFX_API
typedef void(VGFX_API *VgfxDrawClearFn)(void *, vgfx_color_t);
typedef void(VGFX_API *VgfxDrawPixelFn)(void *, size_t, size_t, vgfx_color_t);
typedef void(VGFX_API *VgfxDrawCharFn)(void *, size_t, size_t, char, vgfx_color_t, vgfx_color_t);

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

VGFX_BOOL VGFX_API vgzm_init(void *data, size_t size);
VGFX_BOOL VGFX_API vgzm_set_zoom(void *data, size_t factor);
VGFX_BOOL VGFX_API vgzm_set_draw_clear(void *data, VgfxDrawClearFn func, void *ctx);
VGFX_BOOL VGFX_API vgzm_set_draw_pixel(void *data, VgfxDrawPixelFn func, void *ctx);
VGFX_BOOL VGFX_API vgzm_draw_clear(void *data, vgfx_color_t color);
VGFX_BOOL VGFX_API vgzm_draw_pixel(void *data, size_t x, size_t y, vgfx_color_t color);
VGFX_BOOL VGFX_API vgzm_fini(void *data);

typedef struct _VGFX_CANVAS_2D_DATA {
    void *draw_clear_ctx;
    VgfxDrawClearFn draw_clear;
    void *draw_pixel_ctx;
    VgfxDrawPixelFn draw_pixel;
    void *draw_char_ctx;
    VgfxDrawCharFn draw_char;
    size_t char_width;
    size_t char_height;
} VGFX_CANVAS_2D_DATA;

VGFX_BOOL VGFX_API vg2d_init(void *data, size_t size);
VGFX_BOOL VGFX_API vg2d_set_draw_clear(void *data, VgfxDrawClearFn func, void *ctx);
VGFX_BOOL VGFX_API vg2d_set_draw_pixel(void *data, VgfxDrawPixelFn func, void *ctx);
VGFX_BOOL VGFX_API vg2d_set_draw_char(void *data, VgfxDrawCharFn func, void *ctx, size_t width, size_t height);
VGFX_BOOL VGFX_API vg2d_draw_clear(void *data, vgfx_color_t color);
VGFX_BOOL VGFX_API vg2d_draw_pixel(void *data, size_t x, size_t y, vgfx_color_t color);
VGFX_BOOL VGFX_API vg2d_draw_line(void *data, size_t x0, size_t y0, size_t x1, size_t y1, vgfx_color_t color);
VGFX_BOOL VGFX_API vg2d_draw_fast_hline(void *data, size_t x0, size_t y0, size_t length, vgfx_color_t color);
VGFX_BOOL VGFX_API vg2d_draw_fast_vline(void *data, size_t x0, size_t y0, size_t length, vgfx_color_t color);
VGFX_BOOL VGFX_API vg2d_draw_rect(void *data, size_t x0, size_t y0, size_t w, size_t h, vgfx_color_t color);
VGFX_BOOL VGFX_API vg2d_fill_rect(void *data, size_t x0, size_t y0, size_t w, size_t h, vgfx_color_t color);
VGFX_BOOL VGFX_API vg2d_draw_char(void *data, size_t x, size_t y, char c, vgfx_color_t color, vgfx_color_t bg);
VGFX_BOOL VGFX_API vg2d_draw_string(void *data, size_t x, size_t y, const char *s, vgfx_color_t color, vgfx_color_t bg);
VGFX_BOOL VGFX_API vg2d_fini(void *data);

#ifdef __cplusplus
}
#endif

#endif
