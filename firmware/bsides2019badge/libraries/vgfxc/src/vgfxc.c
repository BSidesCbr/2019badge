#include "vgfxc.h"

VGFX_BOOL VGFX_API vgzm_init(void *data, uint32_t size)
{
    if (size < sizeof(VGFX_ZOOM_DATA))
    {
        return VGFX_FALSE;
    }
    memset(data, 0, size);
    ((VGFX_ZOOM_DATA*)data)->factor = 1; // no zoom
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vgzm_set_zoom(void *data, uint32_t factor)
{
    VGFX_ZOOM_DATA *zoom = (VGFX_ZOOM_DATA *)data;
    if (NULL == zoom) {
        return VGFX_FALSE;
    }
    zoom->factor = factor;
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vgzm_set_draw_clear(void *data, VgfxDrawClearFn func, void *ctx)
{
    VGFX_ZOOM_DATA *zoom = (VGFX_ZOOM_DATA *)data;
    if (NULL == zoom) {
        return VGFX_FALSE;
    }
    zoom->draw_clear = func;
    zoom->draw_clear_ctx = ctx;
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vgzm_set_draw_pixel(void *data, VgfxDrawPixelFn func, void *ctx)
{
    VGFX_ZOOM_DATA *zoom = (VGFX_ZOOM_DATA *)data;
    if (NULL == zoom) {
        return VGFX_FALSE;
    }
    zoom->draw_pixel = func;
    zoom->draw_pixel_ctx = ctx;
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vgzm_draw_clear(void *data, uint32_t color)
{
    VGFX_ZOOM_DATA *zoom = (VGFX_ZOOM_DATA *)data;
    if (NULL == zoom) {
        return VGFX_FALSE;
    }
    if (NULL == zoom->draw_clear) {
        return VGFX_FALSE;
    }
    zoom->draw_clear(zoom->draw_clear_ctx, color);
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vgzm_draw_pixel(void *data, uint32_t x, uint32_t y, uint32_t color)
{
    VGFX_ZOOM_DATA *zoom = (VGFX_ZOOM_DATA *)data;
    uint32_t dx = 0;
    uint32_t dy = 0;
    if (NULL == zoom) {
        return VGFX_FALSE;
    }
    if (NULL == zoom->draw_pixel) {
        return VGFX_FALSE;
    }
    x *= zoom->factor;
    y *= zoom->factor;
    for (dx = 0; dx < zoom->factor; dx++)
    {
        for (dy = 0; dy < zoom->factor; dy++)
        {
            zoom->draw_pixel(zoom->draw_pixel_ctx, x + dx, y + dy, color);
        }
    }
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vgzm_fini(void *data)
{
    uint32_t size = sizeof(VGFX_ZOOM_DATA);
    if (NULL == data) {
        return VGFX_FALSE;
    }
    memset(data, 0, size);
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vg2d_init(void *data, uint32_t size)
{
    if (size < sizeof(VGFX_CANVAS_2D_DATA))
    {
        return VGFX_FALSE;
    }
    memset(data, 0, size);
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vg2d_set_draw_clear(void *data, VgfxDrawClearFn func, void *ctx)
{
    VGFX_CANVAS_2D_DATA *canvas = (VGFX_CANVAS_2D_DATA *)data;
    if (NULL == canvas) {
        return VGFX_FALSE;
    }
    canvas->draw_clear = func;
    canvas->draw_clear_ctx = ctx;
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vg2d_set_draw_pixel(void *data, VgfxDrawPixelFn func, void *ctx)
{
    VGFX_CANVAS_2D_DATA *canvas = (VGFX_CANVAS_2D_DATA *)data;
    if (NULL == canvas) {
        return VGFX_FALSE;
    }
    canvas->draw_pixel = func;
    canvas->draw_pixel_ctx = ctx;
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vg2d_set_draw_char(void *data, VgfxDrawCharFn func, void *ctx, uint32_t width, uint32_t height)
{
    VGFX_CANVAS_2D_DATA *canvas = (VGFX_CANVAS_2D_DATA *)data;
    if (NULL == canvas) {
        return VGFX_FALSE;
    }
    canvas->draw_char = func;
    canvas->draw_char_ctx = ctx;
    canvas->char_width = width;
    canvas->char_height = height;
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vg2d_draw_clear(void *data, uint32_t color)
{
    VGFX_CANVAS_2D_DATA *canvas = (VGFX_CANVAS_2D_DATA *)data;
    if (NULL == canvas) {
        return VGFX_FALSE;
    }
    if (NULL == canvas->draw_clear) {
        return VGFX_FALSE;
    }
    canvas->draw_clear(canvas->draw_clear_ctx, color);
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vg2d_draw_pixel(void *data, uint32_t x, uint32_t y, uint32_t color)
{
    VGFX_CANVAS_2D_DATA *canvas = (VGFX_CANVAS_2D_DATA *)data;
    if (NULL == canvas) {
        return VGFX_FALSE;
    }
    if (NULL == canvas->draw_pixel) {
        return VGFX_FALSE;
    }
    canvas->draw_pixel(canvas->draw_pixel_ctx, x, y, color);
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vg2d_draw_line(void *data, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color)
{
    // http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
    int dx = VGFX_ABS((int32_t)x1 - (int32_t)x0), sx = (int32_t)x0 < x1 ? 1 : -1;
    int dy = VGFX_ABS((int32_t)y1 - (int32_t)y0), sy = (int32_t)y0 < (int32_t)y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;
    VGFX_CANVAS_2D_DATA *canvas = (VGFX_CANVAS_2D_DATA *)data;
    if (NULL == canvas) {
        return VGFX_FALSE;
    }
    if (NULL == canvas->draw_pixel) {
        return VGFX_FALSE;
    }
    for(;;) {
        canvas->draw_pixel(canvas->draw_pixel_ctx, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vg2d_draw_fast_hline(void *data, uint32_t x0, uint32_t y0, uint32_t length, uint32_t color)
{
    if (0 == length) {
        return VGFX_TRUE;
    }
    return vg2d_draw_line(data, x0, y0, x0 + length - 1, y0, color);
}

VGFX_BOOL VGFX_API vg2d_draw_fast_vline(void *data, uint32_t x0, uint32_t y0, uint32_t length, uint32_t color)
{
    if (0 == length) {
        return VGFX_TRUE;
    }
    return vg2d_draw_line(data, x0, y0, x0, y0 + length - 1, color);
}

VGFX_BOOL VGFX_API vg2d_draw_rect(void *data, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t color)
{
    VGFX_BOOL status = VGFX_TRUE;
    if (VGFX_TRUE == status) {
        status = vg2d_draw_fast_hline(data, x0, y0, w, color);
    }
    if (VGFX_TRUE == status) {
        status = vg2d_draw_fast_hline(data, x0, y0 + h - 1, w, color);
    }
    if (VGFX_TRUE == status) {
        status = vg2d_draw_fast_vline(data, x0, y0, h, color);
    }
    if (VGFX_TRUE == status) {
        status = vg2d_draw_fast_vline(data, x0 + w - 1, y0, h, color);
    }
    return status;
}

VGFX_BOOL VGFX_API vg2d_fill_rect(void *data, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t color)
{
    VGFX_BOOL status = VGFX_TRUE;
    uint32_t i = 0;
    for (i = 0; i < h; i++)
    {
        status = vg2d_draw_fast_hline(data, x0, y0 + i, w, color);
        if (VGFX_TRUE != status) {
            return status;
        }
    }
    return status;
}

VGFX_BOOL VGFX_API vg2d_draw_char(void *data, uint32_t x, uint32_t y, char c, uint32_t color, uint32_t bg)
{
    VGFX_CANVAS_2D_DATA *canvas = (VGFX_CANVAS_2D_DATA *)data;
    if (NULL == canvas) {
        return VGFX_FALSE;
    }
    if (NULL == canvas->draw_char) {
        return VGFX_FALSE;
    }
    canvas->draw_char(canvas->draw_char_ctx, x, y, c, color, bg);
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vg2d_draw_string(void *data, uint32_t x, uint32_t y, const char *s, uint32_t color, uint32_t bg)
{
    VGFX_CANVAS_2D_DATA *canvas = (VGFX_CANVAS_2D_DATA *)data;
    uint32_t dx = 0;
    if (NULL == canvas) {
        return VGFX_FALSE;
    }
    if (NULL == canvas->draw_char) {
        return VGFX_FALSE;
    }
    while (*s) {
        if (*s != '\n') {
            canvas->draw_char(canvas->draw_char_ctx, x + dx, y, *s, color, bg);
            dx += canvas->char_width;
        } else {
            dx = 0;
            y += canvas->char_height;
        }
        s++;
    }
    return VGFX_TRUE;
}

VGFX_BOOL VGFX_API vg2d_fini(void *data)
{
    uint32_t size = sizeof(VGFX_ZOOM_DATA);
    if (NULL == data) {
        return VGFX_FALSE;
    }
    memset(data, 0, size);
    return VGFX_TRUE;
}
