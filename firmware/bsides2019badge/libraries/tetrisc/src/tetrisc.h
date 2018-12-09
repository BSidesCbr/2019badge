#ifndef _H_TETRIS_C_H_
#define _H_TETRIS_C_H_

// Based on tutorial
// http://javilop.com/gamedev/tetris-tutorial-in-c-platform-independent-focused-in-game-logic-for-beginners/

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Bool
typedef uint8_t TTRS_BOOL;
#define TTRS_TRUE 1
#define TTRS_FALSE 0

// Piece types
typedef uint8_t TTRS_PIECE_TYPE;
#define TTRS_PIECE_TYPE_SQUARE      0
#define TTRS_PIECE_TYPE_I           1
#define TTRS_PIECE_TYPE_L           2
#define TTRS_PIECE_TYPE_L_MIRRORED  3
#define TTRS_PIECE_TYPE_N           4
#define TTRS_PIECE_TYPE_N_MIRRORED  5
#define TTRS_PIECE_TYPE_T           6
#define TTRS_PIECE_TYPE_UNKNOWN     7

// Block type
typedef uint8_t TTRS_BLOCK_TYPE;
#define TTRS_BLOCK_TYPE_NO_BLOCK        0
#define TTRS_BLOCK_TYPE_NORMAL_BLOCK    1
#define TTRS_BLOCK_TYPE_PIVOT_BLOCK     2

// Pieces definition
#define TTRS_PIECE_KIND_COUNT           7
#define TTRS_ROTATION_COUNT             4
#define TTRS_PIECE_ROW_BLOCK_COUNT      5
#define TTRS_PIECE_COLUMN_BLOCK_COUNT   5
#define TTRS_VERTICAL_BLOCK_COUNT       TTRS_PIECE_ROW_BLOCK_COUNT
#define TTRS_HORIZONTAL_BLOCK_COUNT     TTRS_PIECE_COLUMN_BLOCK_COUNT

// Default (recommended) board dimensions
#define TTRS_BOARD_WIDTH    10  // Board width in blocks 
#define TTRS_BOARD_HEIGHT   20  // Board height in blocks

// Board blocks
typedef uint8_t TTRS_BOARD_BLOCK_TYPE;
#define TTRS_BOARD_BLOCK_FREE       0
#define TTRS_BOARD_BLOCK_FILLED     1

#define TTRS_API
typedef void(TTRS_API *TTrsDrawClearFn)(void *);
typedef void(TTRS_API *TTrsDrawPieceFn)(void *,int16_t,int16_t,TTRS_PIECE_TYPE);
typedef void(TTRS_API *TTrsDrawBlockFn)(void *,int16_t,int16_t);
typedef int16_t(TTRS_API *TTrsRandomFn)(void *,int16_t,int16_t);
typedef void(TTRS_API *TTrsGameOverFn)(void *, uint16_t);

typedef struct _TTRS_DATA {
    void *draw_clear_ctx;
    TTrsDrawClearFn draw_clear;
    void *draw_piece_ctx;
    TTrsDrawPieceFn draw_piece;
    void *draw_next_piece_ctx;
    TTrsDrawPieceFn draw_next_piece;
    void *draw_block_ctx;
    TTrsDrawBlockFn draw_block;
    void *random_ctx;
    TTrsRandomFn random;
    void *game_over_ctx;
    TTrsGameOverFn game_over;
    uint16_t score;
    int16_t pos_x, pos_y;           // Position of the piece that is falling down
    TTRS_PIECE_TYPE piece;          // Kind of the piece that is falling down
    uint8_t rotation;               // Rotation of the piece that is falling down
    int16_t next_pos_x, next_pos_y; // Position of the next piece
    TTRS_PIECE_TYPE next_piece;     // Kind of the next piece
    uint8_t next_rotation;          // Rotation of the next piece
    int16_t w;
    int16_t h;
    uint16_t board_size;
    uint8_t board[1];
} TTRS_DATA;

#define TTRS_CALC_DATA_SIZE(width, height) (sizeof(TTRS_DATA) + ((width * height) >> 3))

TTRS_BOOL ttrs_init(void *data, size_t size);
TTRS_BOOL ttrs_set_grid(void *data, int16_t width, int16_t height);
TTRS_BOOL ttrs_set_draw_clear(void *data, TTrsDrawClearFn func, void *ctx);
TTRS_BOOL ttrs_set_draw_piece(void *data, TTrsDrawPieceFn func, void *ctx);
TTRS_BOOL ttrs_set_draw_next_piece(void *data, TTrsDrawPieceFn func, void *ctx);
TTRS_BOOL ttrs_set_draw_block(void *data, TTrsDrawBlockFn func, void *ctx);
TTRS_BOOL ttrs_set_random(void *data, TTrsRandomFn func, void *ctx);
TTRS_BOOL ttrs_set_game_over(void *data, TTrsGameOverFn func, void *ctx);
TTRS_BOOL ttrs_reset(void *data);
TTRS_BOOL ttrs_key_left(void *data);
TTRS_BOOL ttrs_key_right(void *data);
TTRS_BOOL ttrs_key_down(void *data);
TTRS_BOOL ttrs_key_drop(void *data);
TTRS_BOOL ttrs_key_rotate(void *data);
TTRS_BOOL ttrs_tick(void *data);
TTRS_BOOL ttrs_fini(void *data);

#ifdef __cplusplus
}
#endif

#endif
