#include "vintc.h"

VINTC_BOOL vintc_init(void *data, uint32_t size)
{
    if (size < sizeof(VINTC_DATA))
    {
        return VINTC_FALSE;
    }
    memset(data, 0, size);
    size -= sizeof(VINTC_DATA);
    ((VINTC_DATA*)data)->table_count = (size / sizeof(VINTC_ENTRY)) + 1;
    return VINTC_TRUE;
}

VINTC_BOOL vintc_set_get_tick_count(void *data, VIntCGetTickCountFn func, void *ctx)
{
    if (NULL == data) {
        return VINTC_FALSE;
    }
    ((VINTC_DATA*)data)->ctx = ctx;
    ((VINTC_DATA*)data)->get_tick_count = func;
    return VINTC_TRUE;
}

VINTC_BOOL vintc_set_interrupt(void *data, uint32_t period, VIntCTockFn func, void *ctx, VINTC_HANDLE *handle)
{
    uint32_t i = 0;
    VINTC_ENTRY *entry = NULL;
    if (NULL == data) {
        return VINTC_FALSE;
    }
    if (NULL == ((VINTC_DATA*)data)->get_tick_count) {
        return VINTC_FALSE;
    }
    for (i = 0; i < ((VINTC_DATA*)data)->table_count; i++) {
        entry = &(((VINTC_DATA*)data)->table[i]);
        if (NULL == entry->tock) {
            entry->tick = ((VINTC_DATA*)data)->get_tick_count(((VINTC_DATA*)data)->ctx);
            entry->period = period;
            entry->ctx = ctx;
            entry->tock = func;
            if (NULL != handle) {
                *handle = (VINTC_HANDLE)i;
            }
            return VINTC_TRUE;
        }
    }
    return VINTC_FALSE;
}

VINTC_BOOL vintc_remove(void *data, VINTC_HANDLE handle)
{
    uint32_t i = 0;
    if (NULL == data) {
        return VINTC_FALSE;
    }
    i = (uint32_t)handle;
    if (i >= ((VINTC_DATA*)data)->table_count) {
        return VINTC_FALSE;
    }
    memset(&(((VINTC_DATA*)data)->table[i]), 0, sizeof(VINTC_ENTRY));
    return VINTC_TRUE;
}

VINTC_BOOL vintc_run_loop(void *data)
{
    uint32_t i = 0;
    uint32_t tick = 0;
    VINTC_ENTRY *entry = NULL;
    if (NULL == data) {
        return VINTC_FALSE;
    }
    if (NULL == ((VINTC_DATA*)data)->get_tick_count) {
        return VINTC_FALSE;
    }
    tick = ((VINTC_DATA*)data)->get_tick_count(((VINTC_DATA*)data)->ctx);
    for (i = 0; i < ((VINTC_DATA*)data)->table_count; i++) {
        entry = &(((VINTC_DATA*)data)->table[i]);
        if (NULL != entry->tock) {
            if ((tick - entry->tick) >= entry->period) {
                entry->tick = tick;
                entry->tock(entry->ctx);
            }
        }
    }
    return VINTC_TRUE;
}

VINTC_BOOL vintc_fini(void *data)
{
    uint32_t size = sizeof(VINTC_DATA);
    if (NULL == data) {
        return VINTC_FALSE;
    }
    size -= sizeof(VINTC_ENTRY);
    size += (((VINTC_DATA*)data)->table_count * sizeof(VINTC_ENTRY));
    memset(data, 0, size);
    return VINTC_TRUE;
}
