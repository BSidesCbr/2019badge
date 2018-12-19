#include "tetrisc.h"

// Based on tutorial
// http://javilop.com/gamedev/tetris-tutorial-in-c-platform-independent-focused-in-game-logic-for-beginners/

#include <avr/pgmspace.h>

// so I can read a 32-bit value from program memory (AVR)
#define TTRS_ADDR_ADD(addr,offset)  ((void*)(((uint8_t*)(addr)) + (offset)))
static uint32_t ttrs_pgm_read_uint32_le(const void *addr) {
    uint32_t value = 0;
    value |= ((uint32_t)(pgm_read_byte(TTRS_ADDR_ADD(addr, 0))) << 0 ) & 0x000000ff;
    value |= ((uint32_t)(pgm_read_byte(TTRS_ADDR_ADD(addr, 1))) << 8 ) & 0x0000ff00;
    value |= ((uint32_t)(pgm_read_byte(TTRS_ADDR_ADD(addr, 2))) << 16) & 0x00ff0000;
    value |= ((uint32_t)(pgm_read_byte(TTRS_ADDR_ADD(addr, 3))) << 24) & 0xff000000;
    return value;
}

// encode the blocks
// originally 7 x 4 x 5 x 5 = 700 bytes
// encoded    7 x 4 x 4 = 112 bytes
// much (MUCH) smaller table (removed 84% of bytes)
#define TTRS_PIECE_ENCODE_ROW(x0,x1,x2,x3,x4)   ((((uint32_t)x4 << 6) & 0xc0) | (((uint32_t)x3 << 4) & 0x30) | (((uint32_t)x2 << 2) & 0x0c) | (((uint32_t)x1 << 0) & 0x03))
#define TTRS_PIECE_ENCODE_ROW_0(x0,x1,x2,x3,x4) ((TTRS_PIECE_ENCODE_ROW(x0,x1,x2,x3,x4) << 0)  & (uint32_t)0x000000ff)
#define TTRS_PIECE_ENCODE_ROW_1(x0,x1,x2,x3,x4) ((TTRS_PIECE_ENCODE_ROW(x0,x1,x2,x3,x4) << 8)  & (uint32_t)0x0000ff00)
#define TTRS_PIECE_ENCODE_ROW_2(x0,x1,x2,x3,x4) ((TTRS_PIECE_ENCODE_ROW(x0,x1,x2,x3,x4) << 16) & (uint32_t)0x00ff0000)
#define TTRS_PIECE_ENCODE_ROW_3(x0,x1,x2,x3,x4) ((TTRS_PIECE_ENCODE_ROW(x0,x1,x2,x3,x4) << 24) & (uint32_t)0xff000000)
#define TTRS_PIECE_ENCODE_ROW_4(x0,x1,x2,x3,x4) 0  // as we handle this in the function
#define TTRS_PIECE_INDEX(piece,rotation)        (((piece) * TTRS_ROTATION_COUNT) + (rotation))
#define TTRS_PIECE_DECODE(value,x,y)            ((TTRS_BLOCK_TYPE)(((value) >> ((8 * y) + ((x - 1) * 2))) & 0x3))
const uint32_t TTRS_PIECE_BITS[TTRS_PIECE_KIND_COUNT * TTRS_ROTATION_COUNT] PROGMEM = {
/*
    const static BlockType kPieces[kKindCount][kRotationCount][kHorizontalBlockCount][kVerticalBlockCount] =
    {
        // Square
        {
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            }
        },
        // I
        {
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 1, 2, 1, 1) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 1, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(1, 1, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            }
        },
        // L
        {
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 1, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 1, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 1, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 1, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            }
        },
        // L mirrored
        {
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 1, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 1, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 1, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 1, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 1, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 0, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            }
        },
        // N
        {
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 1, 2, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 1, 2, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 1, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 1, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            }
        },
        // N mirrored
        {
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 0, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 1, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 1, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 1, 2, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 1, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 1, 2, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            }
        },
        // T
        {
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 0, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 1, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 1, 2, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            },
            {
                */
                TTRS_PIECE_ENCODE_ROW_0(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_1(0, 0, 1, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_2(0, 1, 2, 1, 0) |
                TTRS_PIECE_ENCODE_ROW_3(0, 0, 0, 0, 0) |
                TTRS_PIECE_ENCODE_ROW_4(0, 0, 0, 0, 0),
                /*
            }
        }
    };
*/
};

static TTRS_BLOCK_TYPE ttrs_get_block_type(TTRS_PIECE_TYPE piece, uint8_t rotation, int16_t x, int16_t y)
{
    // there is only 1 column that has a "1" in the middle
    // the rest are "0"
    if (0 == x)
    {
        // block "I", rotation "2", pos = (0, 2) is the only one with a "1", rest are "0"
        if ((TTRS_PIECE_TYPE_I == piece) && (2 == rotation) && (2 == y))
        {
            return 1;
        }
        return 0;
    }

    // there is only 1 row that has a "1" in the middle column
    // the rest are "0"
    if (4 == y)
    {
        // block "I", rotation "1", pos = (2, 4) is the only one with a "1", rest are "0"
        if ((TTRS_PIECE_TYPE_I == piece) && (1 == rotation) && (2 == x))
        {
            return 1;
        }
        return 0;
    }

    // we have eliminated a row and column
    // our 5x5 box is now 4x4 (with values 0-2, 2 bits)
    // 4 * 4 * 2 = 32 bits
    // we can have an array of 32-bit numbers now :)
    // much smaller
    return TTRS_PIECE_DECODE(ttrs_pgm_read_uint32_le(&TTRS_PIECE_BITS[TTRS_PIECE_INDEX(piece, rotation)]), x, y);
}

// Displacement of the piece to the position where it is first drawn in the board when it is created
// encode the displacement
// originally 7 x 8 x 2 = 112 bytes
// encoded    7         = 7 bytes
// much (MUCH) smaller table (removed 93% of bytes)
// I can do this becuase there is only 2 displacement values used, -2 and -3
// so -2 is 0 and -3 is 1
#define TTRS_DISPLACEMENT_ENCODE_ROW(x0,x1)     (((((uint8_t)((int16_t)x0 == -2 ? 0 : 1)) << 0) & 0x1) | ((((uint8_t)((int16_t)x1 == -2 ? 0 : 1)) << 1) & 0x2))
#define TTRS_DISPLACEMENT_ENCODE_ROW_0(x0,x1)   ((TTRS_DISPLACEMENT_ENCODE_ROW(x0,x1) << 0)  & 0x03)
#define TTRS_DISPLACEMENT_ENCODE_ROW_1(x0,x1)   ((TTRS_DISPLACEMENT_ENCODE_ROW(x0,x1) << 2)  & 0x0c)
#define TTRS_DISPLACEMENT_ENCODE_ROW_2(x0,x1)   ((TTRS_DISPLACEMENT_ENCODE_ROW(x0,x1) << 4)  & 0x30)
#define TTRS_DISPLACEMENT_ENCODE_ROW_3(x0,x1)   ((TTRS_DISPLACEMENT_ENCODE_ROW(x0,x1) << 6)  & 0xc0)
#define TTRS_DISPLACEMENT_INDEX(piece)                  (piece)
#define TTRS_DISPLACEMENT_DECODE(value,rotation,pos)    ((0 == ((value >> ((rotation << 1) | (0 == pos? 0x0 : 0x1))) & 0x1)) ? ((int16_t)-2) : ((int16_t)-3))
const uint8_t TTRS_DISPLACEMENT_BITS[TTRS_PIECE_KIND_COUNT] PROGMEM = {
/*
    int16_t kPiecesInitialPosition[kKindCount][kRotationCount][kPositionCount] =
    {
        // Square
        {*/
            TTRS_DISPLACEMENT_ENCODE_ROW_0(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_1(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_2(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_3(-2, -3),
        /*},
        // I
        {*/
            TTRS_DISPLACEMENT_ENCODE_ROW_0(-2, -2) |
            TTRS_DISPLACEMENT_ENCODE_ROW_1(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_2(-2, -2) |
            TTRS_DISPLACEMENT_ENCODE_ROW_3(-2, -3),
        /*},
        // L
        {*/
            TTRS_DISPLACEMENT_ENCODE_ROW_0(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_1(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_2(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_3(-2, -2),
        /*},
        // L mirrored
        {*/
            TTRS_DISPLACEMENT_ENCODE_ROW_0(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_1(-2, -2) |
            TTRS_DISPLACEMENT_ENCODE_ROW_2(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_3(-2, -3),
        /*},
        // N
        {*/
            TTRS_DISPLACEMENT_ENCODE_ROW_0(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_1(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_2(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_3(-2, -2),
        /*},
        // N mirrored
        {*/
            TTRS_DISPLACEMENT_ENCODE_ROW_0(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_1(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_2(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_3(-2, -2),
        /*},
        // T
        {*/
            TTRS_DISPLACEMENT_ENCODE_ROW_0(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_1(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_2(-2, -3) |
            TTRS_DISPLACEMENT_ENCODE_ROW_3(-2, -2),
        /*},
    };
*/
};

static int16_t ttrs_get_x_initial_position(TTRS_PIECE_TYPE piece, uint8_t rotation)
{
    return TTRS_DISPLACEMENT_DECODE(pgm_read_byte(&TTRS_DISPLACEMENT_BITS[TTRS_DISPLACEMENT_INDEX(piece)]), rotation, 0);
}

static int16_t ttrs_get_y_initial_position(TTRS_PIECE_TYPE piece, uint8_t rotation)
{
    return TTRS_DISPLACEMENT_DECODE(pgm_read_byte(&TTRS_DISPLACEMENT_BITS[TTRS_DISPLACEMENT_INDEX(piece)]), rotation, 1);
}

// The board is stored as an array of bytes
// Each bit in each byte represents a single block on the board
#define TTRS_BOARD_WIDTH_GET(data)                  (((TTRS_DATA*)data)->w)
#define TTRS_BOARD_HEIGHT_GET(data)                 (((TTRS_DATA*)data)->h)
#define TTRS_BOARD_BLOCK_INDEX(data,x,y)            ((size_t)(((y * TTRS_BOARD_WIDTH_GET(data)) + x) >> 3))
#define TTRS_BOARD_BLOCK_BIT(data,x,y)              (((y * TTRS_BOARD_WIDTH_GET(data)) + x) & 0x7)
#define TTRS_BOARD_BLOCK_BYTE(data,x,y)             ((((TTRS_DATA*)data)->board)[TTRS_BOARD_BLOCK_INDEX(data,x,y)])
#define TTRS_BOARD_BLOCK_SET_FREE(data,x,y)         (TTRS_BOARD_BLOCK_BYTE(data,x,y) &=~ (1 << TTRS_BOARD_BLOCK_BIT(data,x,y)))
#define TTRS_BOARD_BLOCK_SET_FILLED(data,x,y)       (TTRS_BOARD_BLOCK_BYTE(data,x,y) |= (1 << TTRS_BOARD_BLOCK_BIT(data,x,y)))
#define TTRS_BOARD_BLOCK_GET(data,x,y)              ((TTRS_BOARD_BLOCK_BYTE(data,x,y) >> TTRS_BOARD_BLOCK_BIT(data,x,y)) & 0x1)
#define TTRS_BOARD_BLOCK_OUT_OF_BOUNDS(data,x,y)    (TTRS_BOARD_BLOCK_INDEX(data,x,y) >= ((size_t)((TTRS_DATA*)data)->board_size))
static void ttrs_board_set(void *data, int16_t x, int16_t y, uint8_t block) {
    if (TTRS_BOARD_BLOCK_OUT_OF_BOUNDS(data, x, y)) {
        return;
    }
    if (TTRS_BOARD_BLOCK_FREE == block) {
        TTRS_BOARD_BLOCK_SET_FREE(data, x, y);
    } else {
        TTRS_BOARD_BLOCK_SET_FILLED(data, x, y);
    }
}

static uint8_t ttrs_board_get(void *data, int16_t x, int16_t y) {
    if (TTRS_BOARD_BLOCK_OUT_OF_BOUNDS(data, x, y)) {
        return 0;
    }
    return TTRS_BOARD_BLOCK_GET(data, x, y);
}

static void ttrs_board_reset(void *data) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    memset(tetris->board, 0, tetris->board_size);
}

static void ttrs_board_store_piece(void *data, int16_t x, int16_t y, TTRS_PIECE_TYPE piece, uint8_t rotation)
{
    // Store each block of the piece into the board
    for (int16_t i1 = x, i2 = 0; i1 < x + TTRS_HORIZONTAL_BLOCK_COUNT; i1++, i2++)
    {
        for (int16_t j1 = y, j2 = 0; j1 < y + TTRS_VERTICAL_BLOCK_COUNT; j1++, j2++)
        {   
            // Store only the blocks of the piece that are not holes
            if (ttrs_get_block_type(piece, rotation, i2, j2) != TTRS_BLOCK_TYPE_NO_BLOCK) {
                ttrs_board_set(data, i1, j1, TTRS_BOARD_BLOCK_FILLED);
            }
        }
    }
}

static TTRS_BOOL ttrs_board_is_game_over(void *data)
{
    // If the first line has blocks, then, game over
    for (int16_t i = 0; i < TTRS_BOARD_WIDTH_GET(data); i++)
    {
        if (TTRS_BOARD_BLOCK_FILLED == TTRS_BOARD_BLOCK_GET(data, i, 0)) {
            return TTRS_TRUE;
        }
    }
    return TTRS_FALSE;
}

static void ttrs_board_delete_line(void *data, int16_t y)
{
    // Moves all the upper lines one row down
    for (int16_t j = y; j > 0; j--)
    {
        for (int16_t i = 0; i < TTRS_BOARD_WIDTH_GET(data); i++)
        {
            ttrs_board_set(data, i, j, ttrs_board_get(data, i, j - 1));
        }
    }
}

static uint16_t ttrs_board_delete_possible_lines(void *data)
{
    uint16_t removed_lines = 0;
    for (int16_t j = 0; j < TTRS_BOARD_HEIGHT_GET(data); j++)
    {
        int16_t i = 0;
        while (i < TTRS_BOARD_WIDTH_GET(data))
        {
            if (TTRS_BOARD_BLOCK_FILLED != ttrs_board_get(data, i, j)) {
                break;
            }
            i++;
        }
        if (i == TTRS_BOARD_WIDTH_GET(data)) {
            ttrs_board_delete_line(data, j);
            removed_lines++;
        }
    }
    switch(removed_lines) {
        case 1:
            ((TTRS_DATA *)data)->score++;
            break;
        case 2:
            ((TTRS_DATA *)data)->score += 4;
            break;
        case 3:
            ((TTRS_DATA *)data)->score += 8;
            break;
        case 4:
            ((TTRS_DATA *)data)->score += 16;
            break;
        default:
            break;
    }
}

static TTRS_BOOL ttrs_board_is_free_block(void *data, int16_t x, int16_t y)
{
    if (TTRS_BOARD_BLOCK_FREE == ttrs_board_get(data, x, y)) {
        return TTRS_TRUE;
    } else { 
        return TTRS_FALSE;
    }
}

static TTRS_BOOL ttrs_board_is_possible_movement(void *data, int16_t x, int16_t y, TTRS_PIECE_TYPE piece, int16_t rotation)
{
    // Checks collision with pieces already stored in the board or the board limits
    // This is just to check the 5x5 blocks of a piece with the appropriate area in the board
    for (int16_t i1 = x, i2 = 0; i1 < x + TTRS_HORIZONTAL_BLOCK_COUNT; i1++, i2++)
    {
        for (int16_t j1 = y, j2 = 0; j1 < y + TTRS_VERTICAL_BLOCK_COUNT; j1++, j2++)
        {   
            // Check if the piece is outside the limits of the board
            if (    i1 < 0           || 
                i1 > TTRS_BOARD_WIDTH_GET(data)  - 1    ||
                j1 > TTRS_BOARD_HEIGHT_GET(data) - 1)
            {
                if (TTRS_BLOCK_TYPE_NO_BLOCK != ttrs_get_block_type(piece, rotation, i2, j2)) {
                    return TTRS_FALSE;
                }
            }

            // Check if the piece have collisioned with a block already stored in the map
            if (j1 >= 0) 
            {
                if ((TTRS_BLOCK_TYPE_NO_BLOCK != ttrs_get_block_type(piece, rotation, i2, j2)) &&
                    (TTRS_FALSE == ttrs_board_is_free_block(data, i1, j1)) ) {
                    return TTRS_FALSE;
                }
            }
        }
    }

    // No collision
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_init(void *data, size_t size) {
    if (size < sizeof(TTRS_DATA))
    {
        return TTRS_FALSE;
    }
    memset(data, 0, size);
    size -= sizeof(TTRS_DATA);
    ((TTRS_DATA*)data)->board_size = size + 1;
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_set_grid(void *data, int16_t width, int16_t height) {
    size_t requried_board_size = 0;
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    requried_board_size = ((((size_t)width) * ((size_t)height)) >> 3);
    if (requried_board_size > (((TTRS_DATA*)data)->board_size)) {
        return TTRS_FALSE;
    }
    tetris->w = width;
    tetris->h = height;
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_set_draw_clear(void *data, TTrsDrawClearFn func, void *ctx) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    tetris->draw_clear = func;
    tetris->draw_clear_ctx = ctx;
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_set_draw_piece(void *data, TTrsDrawPieceFn func, void *ctx) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    tetris->draw_piece = func;
    tetris->draw_piece_ctx = ctx;
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_set_draw_next_piece(void *data, TTrsDrawPieceFn func, void *ctx) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    tetris->draw_next_piece = func;
    tetris->draw_next_piece_ctx = ctx;
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_set_draw_block(void *data, TTrsDrawBlockFn func, void *ctx) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    tetris->draw_block = func;
    tetris->draw_block_ctx = ctx;
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_set_random(void *data, TTrsRandomFn func, void *ctx) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    tetris->random = func;
    tetris->random_ctx = ctx;
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_set_game_over(void *data, TTrsGameOverFn func, void *ctx) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    tetris->game_over = func;
    tetris->game_over_ctx = ctx;
    return TTRS_TRUE;
}

static int16_t ttrs_random(void *data, int16_t min, int16_t max) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL != tetris->random) {
        return tetris->random(tetris->random_ctx, min, max);
    }
    return min;
}

TTRS_BOOL ttrs_reset(void *data) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }

    // no score
    tetris->score = 0;

    // First piece
    tetris->piece          = (TTRS_PIECE_TYPE)(ttrs_random(data, 0, TTRS_PIECE_KIND_COUNT - 1));
    tetris->rotation       = (uint8_t)(ttrs_random(data, 0, TTRS_ROTATION_COUNT - 1));
    tetris->pos_x          = ((int16_t)(tetris->w / 2)) + ttrs_get_x_initial_position(tetris->piece, tetris->rotation);
    tetris->pos_y          = ttrs_get_y_initial_position(tetris->piece, tetris->rotation);

    //  Next piece
    tetris->next_piece     = (TTRS_PIECE_TYPE)(ttrs_random(data, 0, TTRS_PIECE_KIND_COUNT - 1));
    tetris->next_rotation  = (uint8_t)(ttrs_random(data, 0, TTRS_ROTATION_COUNT - 1));
    tetris->next_pos_x     = tetris->w + TTRS_HORIZONTAL_BLOCK_COUNT;
    tetris->next_pos_y     = TTRS_VERTICAL_BLOCK_COUNT;

    // Clear board
    ttrs_board_reset(data);

    // Done
    return TTRS_TRUE;
}

static void ttrs_create_new_piece(void *data) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;

    // The new piece
    tetris->piece          = tetris->next_piece;
    tetris->rotation       = tetris->next_rotation;
    tetris->pos_x          = ((int16_t)(tetris->w / 2)) + ttrs_get_x_initial_position(tetris->piece, tetris->rotation);
    tetris->pos_y          = ttrs_get_y_initial_position(tetris->piece, tetris->rotation);

    //  Next piece
    tetris->next_piece     = (TTRS_PIECE_TYPE)(ttrs_random(data, 0, TTRS_PIECE_KIND_COUNT - 1));
    tetris->next_rotation  = (uint8_t)(ttrs_random(data, 0, TTRS_ROTATION_COUNT - 1));
}

static void ttrs_draw_clear(void *data) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (tetris->draw_clear) {
        tetris->draw_clear(tetris->draw_clear_ctx);
    }
}

static void ttrs_draw_piece(void *data, int16_t x, int16_t y, TTRS_PIECE_TYPE piece, uint8_t rotation, TTrsDrawPieceFn draw_piece_cb, void *draw_piece_ctx) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == draw_piece_cb) {
        return;
    }
    for (int16_t i = 0; i < TTRS_VERTICAL_BLOCK_COUNT; i++)
    {
        for (int16_t j = 0; j < TTRS_HORIZONTAL_BLOCK_COUNT; j++)
        {
            if (((x + j) < 0) || ((x + j) > (tetris->w)) || ((y + i) < 0) || ((y + i) > (tetris->h))) {
                // out of bounds
                continue;
            }
            // Get the type of the block and draw it with the correct color
            if (TTRS_BLOCK_TYPE_NO_BLOCK != ttrs_get_block_type(piece, rotation, j, i)) {
                draw_piece_cb(draw_piece_ctx, x + j, y + i, piece);
            }
        }
    }
}

static void ttrs_draw_board(void *data) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris->draw_block) {
        return;
    }
    for (int16_t x = 0; x < (tetris->w); x++) {
        for (int16_t y = 0; y < (tetris->h); y++) {
            if (!(ttrs_board_is_free_block(data, x, y))) {
                tetris->draw_block(tetris->draw_block_ctx, x, y);
            }
        }
    }
}

static void ttrs_draw_scene(void *data) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    ttrs_draw_clear(data);
    ttrs_draw_piece(data, 0, 0, tetris->next_piece, tetris->next_rotation, tetris->draw_next_piece, tetris->draw_next_piece_ctx);
    ttrs_draw_board(data);
    ttrs_draw_piece(data, tetris->pos_x, tetris->pos_y, tetris->piece, tetris->rotation, tetris->draw_piece, tetris->draw_piece_ctx);
}

TTRS_BOOL ttrs_key_left(void *data) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    if (TTRS_TRUE == ttrs_board_is_game_over(data))
    {
        // reset required
        return TTRS_TRUE;
    }
    if (TTRS_TRUE == ttrs_board_is_possible_movement(data, tetris->pos_x - 1, tetris->pos_y, tetris->piece, tetris->rotation)) {
        tetris->pos_x--;
        ttrs_draw_scene(data);
    }
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_key_right(void *data) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    if (TTRS_TRUE == ttrs_board_is_game_over(data))
    {
        // reset required
        return TTRS_TRUE;
    }
    if (TTRS_TRUE == ttrs_board_is_possible_movement(data, tetris->pos_x + 1, tetris->pos_y, tetris->piece, tetris->rotation)) {
        tetris->pos_x++;
        ttrs_draw_scene(data);
    }
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_key_down(void *data) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    if (TTRS_TRUE == ttrs_board_is_game_over(data))
    {
        // reset required
        return TTRS_TRUE;
    }
    if (TTRS_TRUE == ttrs_board_is_possible_movement(data, tetris->pos_x, tetris->pos_y + 1, tetris->piece, tetris->rotation)) {
        tetris->pos_y++;
        ttrs_draw_scene(data);
    }
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_key_drop(void *data) {
    uint16_t score = 0;
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    if (TTRS_TRUE == ttrs_board_is_game_over(data))
    {
        // reset required
        return TTRS_TRUE;
    }
    // Check collision from up to down
    while (TTRS_TRUE == ttrs_board_is_possible_movement(data, tetris->pos_x, tetris->pos_y + 1, tetris->piece, tetris->rotation)) {
        tetris->pos_y++;
    }
    ttrs_board_store_piece(data, tetris->pos_x, tetris->pos_y, tetris->piece, tetris->rotation);
    ttrs_board_delete_possible_lines(data);
    if (TTRS_TRUE == ttrs_board_is_game_over(data))
    {
        ttrs_draw_scene(data);
        score = tetris->score;
        (void)ttrs_reset(data);
        if (tetris->game_over) {
            tetris->game_over(tetris->game_over_ctx, score);
        }
        return TTRS_TRUE;
    }
    ttrs_create_new_piece(data);
    ttrs_draw_scene(data);
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_key_rotate(void *data) {
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    if (TTRS_TRUE == ttrs_board_is_game_over(data))
    {
        // reset required
        return TTRS_TRUE;
    }
    if (TTRS_TRUE == ttrs_board_is_possible_movement(data, tetris->pos_x, tetris->pos_y, tetris->piece, (tetris->rotation + 1) % TTRS_ROTATION_COUNT)) {
        tetris->rotation = (tetris->rotation + 1) % TTRS_ROTATION_COUNT;
        ttrs_draw_scene(data);
    }
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_tick(void *data) {
    uint16_t score = 0;
    TTRS_DATA *tetris = (TTRS_DATA *)data;
    if (NULL == tetris) {
        return TTRS_FALSE;
    }
    if (TTRS_TRUE == ttrs_board_is_game_over(data))
    {
        // reset required
        return TTRS_TRUE;
    }
    if (TTRS_TRUE == ttrs_board_is_possible_movement(data, tetris->pos_x, tetris->pos_y + 1, tetris->piece, tetris->rotation))
    {
        tetris->pos_y++;
        ttrs_draw_scene(data);
    }
    else
    {
        ttrs_board_store_piece(data, tetris->pos_x, tetris->pos_y, tetris->piece, tetris->rotation);
        ttrs_board_delete_possible_lines(data);
        if (TTRS_TRUE == ttrs_board_is_game_over(data))
        {
            ttrs_draw_scene(data);
            score = tetris->score;
            (void)ttrs_reset(data);
            if (tetris->game_over) {
                tetris->game_over(tetris->game_over_ctx, score);
            }
            return TTRS_TRUE;
        }
        ttrs_create_new_piece(data);
        ttrs_draw_scene(data);
    }
    return TTRS_TRUE;
}

TTRS_BOOL ttrs_fini(void *data) {
    size_t size = sizeof(TTRS_DATA);
    if (NULL == data) {
        return TTRS_FALSE;
    }
    size -= 1;
    size += ((TTRS_DATA*)data)->board_size;
    memset(data, 0, size);
    return TTRS_TRUE;
}
