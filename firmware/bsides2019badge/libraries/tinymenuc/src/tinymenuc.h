#ifndef _H_TINY_MENU_C_H_
#define _H_TINY_MENU_C_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TMNU_TRUE 1
#define TMNU_FALSE 0
typedef uint8_t TMNU_BOOL;

#define TMNU_API
typedef void(TMNU_API *TmnuMenuItemStringFn)(void *,size_t,char *, size_t);
typedef void(TMNU_API *TmnuMenuItemOnSelectFn)(void *,size_t);
typedef void(TMNU_API *TmnuCalcStringViewFn)(void *, const char *, size_t *, size_t *);
typedef void(TMNU_API *TmnuDrawStringFn)(void *, size_t, size_t, const char *, TMNU_BOOL);

typedef struct _TMNU_DATA {
    void *menu_item_string_ctx;
    TmnuMenuItemStringFn menu_item_string;
    void *calc_string_view_ctx;
    TmnuCalcStringViewFn calc_string_view;
    void *draw_string_ctx;
    TmnuDrawStringFn draw_string;
    void *on_menu_select_ctx;
    TmnuMenuItemOnSelectFn on_menu_select;
    size_t view_width;
    size_t view_height;
    size_t item;
    size_t item_count;
    size_t items_per_view;
    size_t buffer_size;
    char buffer[1];
} TMNU_DATA;

#define TMNU_CALC_DATA_SIZE(buffer_size) ((sizeof(TMNU_DATA)-1)+buffer_size)

TMNU_BOOL tmnu_init(void *data, size_t size);
TMNU_BOOL tmnu_set_view(void *data, size_t width, size_t height);
TMNU_BOOL tmnu_set_menu_item_string(void *data, size_t item_count, TmnuMenuItemStringFn func, void *ctx);
TMNU_BOOL tmnu_set_calc_string_view(void *data, TmnuCalcStringViewFn func, void *ctx);
TMNU_BOOL tmnu_set_draw_string(void *data, TmnuDrawStringFn func, void *ctx);
TMNU_BOOL tmnu_set_on_select(void *data, TmnuMenuItemOnSelectFn func, void *ctx);
TMNU_BOOL tmnu_get_item(void *data, size_t *item);
TMNU_BOOL tmnu_set_item(void *data, size_t item);
TMNU_BOOL tmnu_get_item_count(void *data, size_t *item_count);
TMNU_BOOL tmnu_get_items_per_view(void *data, size_t *item_count);
TMNU_BOOL tmnu_key_up(void *data);
TMNU_BOOL tmnu_key_down(void *data);
TMNU_BOOL tmnu_key_enter(void *data);
TMNU_BOOL tmnu_draw_view(void *data);
TMNU_BOOL tmnu_fini(void *data);

#ifdef __cplusplus
}
#endif

#endif
