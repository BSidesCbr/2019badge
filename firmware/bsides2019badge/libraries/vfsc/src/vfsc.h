#ifndef _H_VFS_C_H_
#define _H_VFS_C_H_

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

// credits go to:
// http://lolengine.net/blog/2011/12/20/cpp-constant-string-hash
#define VFSC_H1(s,i,x)   (x*65599u+(uint8_t)s[(i)<(sizeof(s)-1)?(sizeof(s)-1)-1-(i):(sizeof(s)-1)])
#define VFSC_H4(s,i,x)   VFSC_H1(s,i,VFSC_H1(s,i+1,VFSC_H1(s,i+2,VFSC_H1(s,i+3,x))))
#define VFSC_H16(s,i,x)  VFSC_H4(s,i,VFSC_H4(s,i+4,VFSC_H4(s,i+8,VFSC_H4(s,i+12,x))))
#define VFSC_H64(s,i,x)  VFSC_H16(s,i,VFSC_H16(s,i+16,VFSC_H16(s,i+32,VFSC_H16(s,i+48,x))))
#define VFSC_H256(s,i,x) VFSC_H64(s,i,VFSC_H64(s,i+64,VFSC_H64(s,i+128,VFSC_H64(s,i+192,x))))
#define VFSC_HASH(s)     ((uint32_t)(VFSC_H256(s,0,0)^(VFSC_H256(s,0,0)>>16)))

#define VFSC_API
typedef uint8_t(VFSC_API *VfscReadByteFn)(void *,void *);

#define VFSC_VF_INDEX_INVALID       ((ssize_t)(-1))
#define VFSC_VF_OFFSET_RESERVED     0
#define VFSC_VF_OFFSET_FILE_COUNT   4
#define VFSC_VF_OFFSET_HASH_TABLE   8

typedef struct _VFSC_HANDLE_DATA {
    ssize_t vf_index;
    ssize_t vf_offset;
    void* data;
    size_t size;
} VFSC_HANDLE_DATA;

typedef struct _VFSC_DATA {
    VfscReadByteFn read_byte;
    void *read_byte_ctx;
    void *vfs_data;
    size_t vfs_data_size;
    ssize_t vfs_file_count;
    ssize_t vfs_max_handles;
    VFSC_HANDLE_DATA vf_file_handle[1];
} VFSC_DATA;

#define VFSC_VF_CALC_DATA_SIZE(max_handles) (sizeof(VFSC_DATA)+((max_handles-1)*sizeof(VFSC_HANDLE_DATA)))

#define VFSC_SEEK_SET    0
#define VFSC_SEEK_CUR    1
#define VFSC_SEEK_END    2

int vfsc_init(void *data, size_t size, void *vfs_data, size_t vfs_data_size, VfscReadByteFn read_byte, void *read_byte_ctx);
int vfsc_open_hash(void *data, uint32_t hash);
#define vfsc_open(data, pathname)     vfsc_open_hash(data, VFSC_HASH(pathname))
ssize_t vfsc_read(void *data, int fd, void *buf, size_t count);
off_t vfsc_lseek(void *data, int fd, off_t offset, int whence);
int vfsc_close(void *data, int fd);
void vfsc_fini(void *data);

#ifdef __cplusplus
}
#endif

#endif
