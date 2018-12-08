#ifndef _H_VIRTUAL_INTERRUPT_C_H_
#define _H_VIRTUAL_INTERRUPT_C_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VINTC_API
typedef uint32_t(VINTC_API *VIntCGetTickCountFn)(void *);
typedef void(VINTC_API *VIntCTockFn)(void *);

typedef struct _VINTC_ENTRY {
	uint32_t period;
	uint32_t tick;
	void *ctx;
	VIntCTockFn tock;
} VINTC_ENTRY;

typedef struct _VINTC_DATA {
	void *ctx;
	VIntCGetTickCountFn get_tick_count;
	size_t table_count;
	VINTC_ENTRY table[1];
} VINTC_DATA;

#define VINTC_CALC_DATA_SIZE(slots) (sizeof(VINTC_DATA) - sizeof(VINTC_ENTRY) + (slots * sizeof(VINTC_ENTRY)))

typedef size_t VINTC_HANDLE;
#define VINTC_INVALID_HANDLE (~((VINTC_HANDLE)0))

#define VINTC_TRUE 1
#define VINTC_FALSE 0
typedef uint8_t VINTC_BOOL;

VINTC_BOOL vintc_init(void *data, size_t size);
VINTC_BOOL vintc_set_get_tick_count(void *data, VIntCGetTickCountFn func, void *ctx);
VINTC_BOOL vintc_set_interrupt(void *data, uint32_t period, VIntCTockFn func, void *ctx, VINTC_HANDLE *handle);
VINTC_BOOL vintc_remove(void *data, VINTC_HANDLE handle);
VINTC_BOOL vintc_run_loop(void *data);
VINTC_BOOL vintc_fini(void *data);

#ifdef __cplusplus
}
#endif

#endif
