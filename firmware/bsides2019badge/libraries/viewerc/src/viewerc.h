#ifndef _H_VIEWER_C_H_
#define _H_VIEWER_C_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VWRC_API
typedef int32_t(VWRC_API *VwrcReadFn)(void *, uint32_t, char *, uint32_t);
typedef void(VWRC_API *VwrcCalcStringViewFn)(void *, const char *, uint32_t *, uint32_t *);
typedef void(VWRC_API *VwrcDrawStringFn)(void *, uint32_t, uint32_t, const char *);

#define VWRC_TRUE 1
#define VWRC_FALSE 0
typedef uint8_t VWRC_BOOL;

typedef struct _VWRC_VIEWER_DATA {
    void *read_data_ctx;
    VwrcReadFn read_data;
    void *calc_string_view_ctx;
    VwrcCalcStringViewFn calc_string_view;
    void *draw_string_ctx;
    VwrcDrawStringFn draw_string;
    uint32_t view_width;
    uint32_t view_height;
    uint32_t total_chars;
    uint32_t row;
    uint32_t row_count;
    uint32_t rows_per_view;
    uint32_t max_chars_per_line;
    char buffer[1];
} VWRC_VIEWER_DATA;

#define VWRC_CALC_DATA_SIZE(max_chars_per_line) (sizeof(VWRC_VIEWER_DATA)+max_chars_per_line)

VWRC_BOOL vwrc_init(void *data, uint32_t size);
VWRC_BOOL vwrc_set_view(void *data, uint32_t width, uint32_t height);
VWRC_BOOL vwrc_set_calc_string_view(void *data, VwrcCalcStringViewFn func, void *ctx);
VWRC_BOOL vwrc_set_draw_string(void *data, VwrcDrawStringFn func, void *ctx);
VWRC_BOOL vwrc_set_text(void *data, uint32_t total_chars, VwrcReadFn func, void *ctx);
VWRC_BOOL vwrc_get_row(void *data, uint32_t *row);
VWRC_BOOL vwrc_get_row_count(void *data, uint32_t *rows);
VWRC_BOOL vwrc_get_rows_per_view(void *data, uint32_t *rows);
VWRC_BOOL vwrc_scroll_up(void *data);
VWRC_BOOL vwrc_scroll_to_row(void *data, uint32_t row);
VWRC_BOOL vwrc_scroll_down(void *data);
VWRC_BOOL vwrc_draw_view(void *data);
VWRC_BOOL vwrc_fini(void *data);

#ifdef __cplusplus
}
#endif

#endif
