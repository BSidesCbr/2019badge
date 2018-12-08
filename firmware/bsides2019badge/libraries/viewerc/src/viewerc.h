#ifndef _H_VIEWER_C_H_
#define _H_VIEWER_C_H_

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

#define VWRC_API
typedef ssize_t(VWRC_API *VwrcReadFn)(void *, size_t, char *, size_t);
typedef void(VWRC_API *VwrcCalcStringViewFn)(void *, const char *, size_t *, size_t *);
typedef void(VWRC_API *VwrcDrawStringFn)(void *, size_t, size_t, const char *);

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
    size_t view_width;
    size_t view_height;
    size_t total_chars;
    size_t row;
    size_t row_count;
    size_t rows_per_view;
    size_t max_chars_per_line;
    char buffer[1];
} VWRC_VIEWER_DATA;

#define VWRC_CALC_DATA_SIZE(max_chars_per_line) (sizeof(VWRC_VIEWER_DATA)+max_chars_per_line)

VWRC_BOOL vwrc_init(void *data, size_t size);
VWRC_BOOL vwrc_set_view(void *data, size_t width, size_t height);
VWRC_BOOL vwrc_set_calc_string_view(void *data, VwrcCalcStringViewFn func, void *ctx);
VWRC_BOOL vwrc_set_draw_string(void *data, VwrcDrawStringFn func, void *ctx);
VWRC_BOOL vwrc_set_text(void *data, size_t total_chars, VwrcReadFn func, void *ctx);
VWRC_BOOL vwrc_get_row(void *data, size_t *row);
VWRC_BOOL vwrc_get_row_count(void *data, size_t *rows);
VWRC_BOOL vwrc_get_rows_per_view(void *data, size_t *rows);
VWRC_BOOL vwrc_scroll_up(void *data);
VWRC_BOOL vwrc_scroll_to_row(void *data, size_t row);
VWRC_BOOL vwrc_scroll_down(void *data);
VWRC_BOOL vwrc_draw_view(void *data);
VWRC_BOOL vwrc_fini(void *data);

#ifdef __cplusplus
}
#endif

#endif
