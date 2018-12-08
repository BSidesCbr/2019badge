#ifndef _H_CSV_C_H_
#define _H_CSV_C_H_

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

#define CSVC_TRUE   1
#define CSVC_FALSE  0
typedef uint8_t CSVC_BOOL;

#define CSVC_API
typedef ssize_t(CSVC_API *CsvcReadFn)(void *, size_t, char *, size_t);
typedef CSVC_BOOL(CSVC_API *CsvcCellFn)(void *, size_t row, size_t column, const char *text);

CSVC_BOOL csvc_dimensions(size_t size, CsvcReadFn read_fn, void *read_ctx, size_t *rows, size_t *columns);
CSVC_BOOL csvc_for_each_cell(size_t size, CsvcReadFn read_fn, void *read_ctx, CsvcCellFn cell_fn, void *cell_ctx, char *buffer, size_t buffer_size);
CSVC_BOOL csvc_read_cell(size_t size, CsvcReadFn read_fn, void *read_ctx, size_t row, size_t column, char *buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
