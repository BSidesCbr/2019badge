#ifndef _H_TINY_MENU_C_H_
#define _H_TINY_MENU_C_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TMNU_API
typedef void(TMNU_API *TmnuDrawClearFn)(void *);

#define TMNU_TRUE 1
#define TMNU_FALSE 0
typedef uint8_t TMNU_BOOL;

typedef struct _TMNU_DATA {
    void *draw_clear_ctx;
    TmnuDrawClearFn draw_clear;
} TMNU_DATA;

#define TMNU_CALC_DATA_SIZE() (sizeof(TMNU_DATA))

TMNU_BOOL TMNU_API tmnu_init(void *data, uint32_t size);
TMNU_BOOL TMNU_API tmnu_fini(void *data);

#ifdef __cplusplus
}
#endif

#endif
