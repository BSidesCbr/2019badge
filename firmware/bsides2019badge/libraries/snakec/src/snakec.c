#include "snakec.h"

static SNKC_BOOL snkc_trail_clear(void *data) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    snake->trail_begin = 0;
    snake->trail_end = 0;
    return SNKC_TRUE;
}

static void snkc_trail_set(void *data, uint16_t index, uint16_t value) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    uint16_t byte = 0;
    uint16_t shift = 0;
    if (NULL == snake) {
        return;
    }
    // where is the data?
    byte = index >> 2;  // divide by 4 (>> 2)
    shift = (index & 0x3) << 1;  // value is 2 bits (0x3) and shift x 2 (<< 1)
    // clear value
    snake->trail[byte] &=~ (uint8_t)(0x3 << shift);
    // set value
    snake->trail[byte] |= (uint8_t)((value & 0x3) << shift);
}

static uint16_t snkc_trail_get(void *data, uint16_t index) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    uint16_t byte = 0;
    uint16_t shift = 0;
    if (NULL == snake) {
        return 0;
    }
    // where is the data?
    byte = index >> 2;  // divide by 4 (>> 2)
    shift = (index & 0x3) << 1;  // value is 2 bits (0x3) and shift x 2 (<< 1)
    // get value
    return (uint16_t)(((snake->trail[byte]) >> shift) & 0x3);
}

static uint16_t snkc_trail_inc(void *data, uint16_t index) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    uint16_t size = 0;
    if (NULL == snake) {
        return 0;
    }
    size = snake->trail_array_size * 4;  // 4 values per byte
    index++;
    if (index >= size) {
        index = 0;
    }
    return index;
}

static uint16_t snkc_trail_dec(void *data, uint16_t index) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    uint16_t size = 0;
    if (NULL == snake) {
        return 0;
    }
    size = snake->trail_array_size * 4;  // 4 values per byte
    if (index > 0) {
        index--;
    } else {
        index = size - 1;
    }
    return index;
}

static uint16_t snkc_trail_encode_delta(int16_t xv, int16_t yv) {
    if ((xv < 0) && (yv == 0))
    {
        // moving left
        return 0;
    }
    if ((xv == 0) && (yv < 0))
    {
        // moving up
        return 1;
    }
    if ((xv > 0) && (yv == 0))
    {
        // moving right
        return 2;
    }
    if ((xv == 0) && (yv > 0))
    {
        // moving down
        return 3;
    }
    // stopped (but I have no code for this)
    // so moving left
    return 0;
}

static void snkc_trail_appy_delta(void *data, uint16_t delta, int16_t *x, int16_t *y) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (0 == delta)
    {
        // moving left
        (*x)--;
        if ((*x) < 0) {
            *x = snake->w - 1;
        }
    }
    if (1 == delta)
    {
        // moving up
        (*y)--;
        if ((*y) < 0) {
            *y = snake->h - 1;
        }
    }
    if (2 == delta)
    {
        // moving right
        (*x)++;
        if ((*x) >= snake->w) {
            *x = 0;
        }
    }
    if (3 == delta)
    {
        // moving down
        (*y)++;
        if ((*y) >= snake->h) {
            *y = 0;
        }
    }
}

static uint16_t snkc_trail_size(void *data) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    uint16_t max_size = snake->trail_array_size * 4;
    if (NULL == snake) {
        return 0;
    }
    if (snake->trail_end >= snake->trail_begin) {
        // begin and end are in a straight line
        return snake->trail_end - snake->trail_begin;
    }
    // begin goes to the end of the buffer, wraps around, and then goes to the end
    return (max_size - snake->trail_begin) + snake->trail_end;
}

typedef void(SNKC_API *SnkCBodyFn)(void *,int16_t,int16_t);

static SNKC_BOOL snkc_trail_iterate(void *data, SnkCBodyFn callback, void *ctx) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    uint16_t size = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t pos = 0;
    uint16_t delta = 0;
    if ((NULL == snake) || (NULL == callback)) {
        return SNKC_FALSE;
    }
    // work with a copy (we are no modifying)
    size = snkc_trail_size(snake);
    x = snake->px;
    y = snake->py;
    pos = snake->trail_end;
    // use the delta(s) to trace back all the trail positions
    for (uint16_t i = 0; i < size; i++) {
        if (0 == i) {
            // first position is the previous one
            // the current position doesn't get pushed until after the iteration
            // so the first delta will be based on the current velocity
            delta = snkc_trail_encode_delta(snake->xv, snake->yv);
        } else {
            // use every delta after the first to walk backwards throught the trail
            // get the delta
            delta = snkc_trail_get(snake, pos);
            // move the position in the trail backwards
            pos = snkc_trail_dec(snake, pos);
        }
        // we are going backwards through the trail
        // so the delta(s) are back-to-front
        // point the delta in the opposite direction
        delta = (delta + 2) & 0x3;
        // shift the x, y by the delta
        snkc_trail_appy_delta(snake, delta, &x, &y);
        // callback
        callback(ctx, x, y);
    }
    return SNKC_TRUE;
}

static SNKC_BOOL snkc_trail_push_back(void *data, int16_t x, int16_t y) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    uint16_t max_count = 0;
    uint16_t delta = 0;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    // storing deltas, use the velocity instead
    if ((snake->xv == 0) && (snake->yv == 0)) {
        // not moving, don't push back
        return SNKC_TRUE;
    }
    // don't push back if full
    max_count = snake->trail_array_size * 4;  // 4 values per byte
    if ((snkc_trail_size(snake) + 1) >= max_count) {
        // full
        return SNKC_FALSE;
    }
    // calculate the delta
    // we will cheat and use the velocities of the snake
    // rather than tracking the previous x and y (which would end up the same)
    delta = snkc_trail_encode_delta(snake->xv, snake->yv);
    // move to next free position
    snake->trail_end = snkc_trail_inc(snake, snake->trail_end);
    // store the delta
    snkc_trail_set(snake, snake->trail_end, delta);
    // done
    return SNKC_TRUE;
}

static SNKC_BOOL snkc_trail_pop_front(void *data, int16_t *x, int16_t *y) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    // check if empty
    if (snake->trail_begin == snake->trail_end) {
        // empty
        return SNKC_FALSE;
    }
    // pop is just moving to next position
    snake->trail_begin = snkc_trail_inc(snake, snake->trail_begin);
    if (NULL != x) {
        *x = 0;
    }
    if (NULL != y) {
        *y = 0;
    }
    return SNKC_TRUE;
}

static void SNKC_API snkc_draw_trail_and_check(void *ctx, int16_t x, int16_t y) {
    SNKC_DATA *snake = (SNKC_DATA *)ctx;
    if (NULL == snake) {
        return;
    }
    // draw the body of the snake
    if (NULL != snake->draw_snake) {
        snake->draw_snake(snake->draw_snake_ctx, x, y);
    }
    // check if the snake has colided with itself
    if ((x == snake->px) && (y == snake->py)) {
        snake->tail = 5;
        // score will be reset by tick function
    }
}

static SNKC_BOOL snkc_place_apple(void *data) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    if (NULL != snake->random) {
        snake->ax = snake->random(snake->random_ctx, 0, snake->w - 1);
        snake->ay = snake->random(snake->random_ctx, 0, snake->h - 1);
    } else {
        return SNKC_FALSE;
    }
    return SNKC_TRUE;
}

SNKC_BOOL snkc_init(void *data, size_t size) {
    if (size < sizeof(SNKC_DATA))
    {
        return SNKC_FALSE;
    }
    memset(data, 0, size);
    size -= sizeof(SNKC_DATA);
    ((SNKC_DATA*)data)->trail_array_size = size + 1;
    ((SNKC_DATA*)data)->tail = 5;
    ((SNKC_DATA*)data)->score = 0;
    return SNKC_TRUE;
}

SNKC_BOOL snkc_set_grid(void *data, int16_t width, int16_t height) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    snake->w = width;
    snake->h = height;
    return SNKC_TRUE;
}

SNKC_BOOL snkc_set_draw_clear(void *data, SnkCDrawClearFn func, void *ctx) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    snake->draw_clear = func;
    snake->draw_clear_ctx = ctx;
    return SNKC_TRUE;
}

SNKC_BOOL snkc_set_draw_empty(void *data, SnkCDrawSquareFn func, void *ctx) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    snake->draw_empty = func;
    snake->draw_empty_ctx = ctx;
    return SNKC_TRUE;
}

SNKC_BOOL snkc_set_draw_snake(void *data, SnkCDrawSquareFn func, void *ctx) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    snake->draw_snake = func;
    snake->draw_snake_ctx = ctx;
    return SNKC_TRUE;
}

SNKC_BOOL snkc_set_draw_apple(void *data, SnkCDrawSquareFn func, void *ctx) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    snake->draw_apple = func;
    snake->draw_apple_ctx = ctx;
    return SNKC_TRUE;
}

SNKC_BOOL snkc_set_random(void *data, SnkCRandomFn func, void *ctx) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    snake->random = func;
    snake->random_ctx = ctx;
    return SNKC_TRUE;
}

SNKC_BOOL snkc_set_game_over(void *data, SnkCGameOverFn func, void *ctx) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    snake->game_over = func;
    snake->game_over_ctx = ctx;
    return SNKC_TRUE;
}

SNKC_BOOL snkc_reset(void *data) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    SNKC_BOOL status = SNKC_TRUE;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    snake->xv = 0;
    snake->yv = 0;
    snake->px = snake->w / 2;
    snake->py = snake->h / 2;
    snake->tail = 5;
    snake->score = 0;
    if (SNKC_TRUE == status) {
        status = snkc_trail_clear(data);
    }
    if (SNKC_TRUE == status) {
        status = snkc_place_apple(data);
    }
    return SNKC_TRUE;
}

SNKC_BOOL snkc_tick(void *data) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    SNKC_BOOL status = SNKC_TRUE;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t score = 0;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    snake->px += snake->xv;
    snake->py += snake->yv;
    if (snake->px < 0) {
        snake->px = snake->w - 1;
    }
    if (snake->px > snake->w - 1) {
        snake->px = 0;
    }
    if (snake->py < 0) {
        snake->py = snake->h - 1;
    }
    if (snake->py > snake->h - 1) {
        snake->py = 0;
    }
    if (SNKC_TRUE == status) {
        if (NULL != snake->draw_clear) {
            snake->draw_clear(snake->draw_clear_ctx);
        }
    }
    if (SNKC_TRUE == status) {
        status = snkc_trail_iterate(data, snkc_draw_trail_and_check, data);
    }
    if (SNKC_TRUE == status) {
        status = snkc_trail_push_back(data, snake->px, snake->py);
    }
    while ((SNKC_TRUE == status) && (snkc_trail_size(data) > snake->tail)) {
        status = snkc_trail_pop_front(data, &x, &y);
        if ((SNKC_TRUE == status) && (NULL != snake->draw_empty)) {
            snake->draw_empty(snake->draw_empty_ctx, x, y);
        }
    }
    if (SNKC_TRUE == status) {
        if ((snake->ax == snake->px) && (snake->ay == snake->py)) {
            snake->tail++;
            snake->score++;
            status = snkc_place_apple(data);
        }
    }
    if (SNKC_TRUE == status) {
        if (NULL != snake->draw_apple) {
            snake->draw_apple(snake->draw_apple_ctx, snake->ax, snake->ay);
        } else {
            status = SNKC_FALSE;
        }
    }
    if ((SNKC_TRUE == status) && (snake->tail <= 5) && (snake->score > 0)) {
        score = snake->score;
        snake->score = 0;
        if (snake->game_over) {
            snake->game_over(snake->game_over_ctx, score);
        }
    }
    return status;
}

SNKC_BOOL snkc_key_left(void *data) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    if (snake->xv != 1) {
        snake->xv = -1; snake->yv = 0;
    }
    return SNKC_TRUE;
}

SNKC_BOOL snkc_key_up(void *data) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    if (snake->yv != 1) {
        snake->xv = 0; snake->yv = -1;
    }
    return SNKC_TRUE;
}

SNKC_BOOL snkc_key_right(void *data) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    if (snake->xv != -1) {
        snake->xv = 1; snake->yv = 0;
    }
    return SNKC_TRUE;
}

SNKC_BOOL snkc_key_down(void *data) {
    SNKC_DATA *snake = (SNKC_DATA *)data;
    if (NULL == snake) {
        return SNKC_FALSE;
    }
    if (snake->yv != -1) {
        snake->xv = 0; snake->yv = 1;
    }
    return SNKC_TRUE;
}

SNKC_BOOL snkc_fini(void *data) {
    uint16_t size = sizeof(SNKC_DATA);
    if (NULL == data) {
        return SNKC_FALSE;
    }
    size -= 1;
    size += ((SNKC_DATA*)data)->trail_array_size;
    memset(data, 0, size);
    return SNKC_TRUE;
}
