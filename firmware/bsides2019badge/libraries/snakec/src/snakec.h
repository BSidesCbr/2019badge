#ifndef _H_SNAKE_C_H_
#define _H_SNAKE_C_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SNKC_API
typedef void(SNKC_API *SnkCDrawClearFn)(void *);
typedef void(SNKC_API *SnkCDrawSquareFn)(void *,int16_t,int16_t);
typedef int16_t(SNKC_API *SnkCRandomFn)(void *,int16_t,int16_t);
typedef void(SNKC_API *SnkCGameOverFn)(void *,uint16_t);

typedef struct _SNKC_DATA {
    void *draw_clear_ctx;
    SnkCDrawClearFn draw_clear;
    void *draw_empty_ctx;
    SnkCDrawSquareFn draw_empty;
    void *draw_snake_ctx;
    SnkCDrawSquareFn draw_snake;
    void *draw_apple_ctx;
    SnkCDrawSquareFn draw_apple;
    void *random_ctx;
    SnkCRandomFn random;
    void *game_over_ctx;
    SnkCGameOverFn game_over;
    int16_t w;
    int16_t h;
    int16_t xv;
    int16_t yv;
    int16_t px;
    int16_t py;
    int16_t ax;
    int16_t ay;
    uint16_t score;
    uint16_t tail;
    uint16_t trail_begin;
    uint16_t trail_end;
    uint16_t trail_array_size;
    uint8_t trail[1];
} SNKC_DATA;

#define SNKC_CALC_DATA_SIZE(width, height) (sizeof(SNKC_DATA) + ((width * height) >> 2))

#define SNKC_TRUE 1
#define SNKC_FALSE 0
typedef uint8_t SNKC_BOOL;

SNKC_BOOL snkc_init(void *data, size_t size);
SNKC_BOOL snkc_set_grid(void *data, int16_t width, int16_t height);
SNKC_BOOL snkc_set_draw_clear(void *data, SnkCDrawClearFn func, void *ctx);
SNKC_BOOL snkc_set_draw_empty(void *data, SnkCDrawSquareFn func, void *ctx);
SNKC_BOOL snkc_set_draw_snake(void *data, SnkCDrawSquareFn func, void *ctx);
SNKC_BOOL snkc_set_draw_apple(void *data, SnkCDrawSquareFn func, void *ctx);
SNKC_BOOL snkc_set_random(void *data, SnkCRandomFn func, void *ctx);
SNKC_BOOL snkc_set_game_over(void *data, SnkCGameOverFn func, void *ctx);
SNKC_BOOL snkc_reset(void *data);
SNKC_BOOL snkc_key_left(void *data);
SNKC_BOOL snkc_key_up(void *data);
SNKC_BOOL snkc_key_right(void *data);
SNKC_BOOL snkc_key_down(void *data);
#define  snkc_get_score(data)       (((SNKC_DATA*)data)->score)
SNKC_BOOL snkc_tick(void *data);
SNKC_BOOL snkc_fini(void *data);

#ifdef __cplusplus
}
#endif

#endif
