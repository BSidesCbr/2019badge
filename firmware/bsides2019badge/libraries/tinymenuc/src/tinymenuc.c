#include "tinymenuc.h"

TMNU_BOOL TMNU_API tmnu_init(void *data, uint32_t size)
{
    if (size < sizeof(TMNU_DATA))
    {
        return TMNU_FALSE;
    }
    memset(data, 0, size);
    return TMNU_TRUE;
}

TMNU_BOOL TMNU_API tmnu_fini(void *data)
{
    uint32_t size = sizeof(TMNU_DATA);
    if (NULL == data) {
        return TMNU_FALSE;
    }
    memset(data, 0, size);
    return TMNU_TRUE;
}
