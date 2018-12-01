#include "vfsc.h"

#define VFSC_ADDR_ADD(addr,offset)  ((void*)(((uint8_t*)(addr)) + (offset)))

static uint32_t vfsc_read_uint32_le(void *data, void *addr) {
    VFSC_DATA *vfs = (VFSC_DATA *)data;
    uint32_t value = 0;
    value |= ((uint32_t)(vfs->read_byte(vfs->read_byte_ctx, VFSC_ADDR_ADD(addr, 0))) << 0 ) & 0x000000ff;
    value |= ((uint32_t)(vfs->read_byte(vfs->read_byte_ctx, VFSC_ADDR_ADD(addr, 1))) << 8 ) & 0x0000ff00;
    value |= ((uint32_t)(vfs->read_byte(vfs->read_byte_ctx, VFSC_ADDR_ADD(addr, 2))) << 16) & 0x00ff0000;
    value |= ((uint32_t)(vfs->read_byte(vfs->read_byte_ctx, VFSC_ADDR_ADD(addr, 3))) << 24) & 0xff000000;
    return value;
}

int vfsc_init(void *data, uint32_t size, void *vfs_data, uint32_t vfs_data_size, VfscReadByteFn read_byte, void *read_byte_ctx) {
    int fd = 0;
    if (!data) {
        return -1;
    }
    if (size < sizeof(VFSC_DATA)) {
        return -1;
    }
    if (!vfs_data) {
        return -1;
    }
    if (vfs_data_size < 8) {
        return -1;
    }
    if (!read_byte) {
        return -1;
    }
    memset(data, 0, size);
    ((VFSC_DATA*)data)->read_byte = read_byte;
    ((VFSC_DATA*)data)->read_byte_ctx = read_byte_ctx;
    if (0 != vfsc_read_uint32_le(data, VFSC_ADDR_ADD(vfs_data, VFSC_VF_OFFSET_RESERVED))) {
        // reserved not zero
        return -1;
    }
    ((VFSC_DATA*)data)->vfs_data = vfs_data;
    ((VFSC_DATA*)data)->vfs_data_size = vfs_data_size;
    ((VFSC_DATA*)data)->vfs_file_count = vfsc_read_uint32_le(data, VFSC_ADDR_ADD(vfs_data, VFSC_VF_OFFSET_FILE_COUNT));
    ((VFSC_DATA*)data)->vfs_max_handles = ((size - sizeof(VFSC_DATA) - sizeof(VFSC_HANDLE_DATA)) / sizeof(VFSC_HANDLE_DATA));
    for (fd = 0; fd < ((VFSC_DATA*)data)->vfs_max_handles; fd++) {
        ((VFSC_DATA*)data)->vf_file_handle[fd].vf_index = VFSC_VF_INDEX_INVALID;
        ((VFSC_DATA*)data)->vf_file_handle[fd].vf_offset = 0;
    }
    return 0;
}

static uint32_t vfsc_vf_index(VFSC_DATA *vfs, uint32_t hash) {
    uint32_t vf_index = 0;
    void *addr = VFSC_ADDR_ADD(vfs->vfs_data, VFSC_VF_OFFSET_HASH_TABLE);
    for (vf_index = 0; vf_index < vfs->vfs_file_count; vf_index++) {
        if (hash == vfsc_read_uint32_le(vfs, addr)) {
            return vf_index;
        }
        addr = VFSC_ADDR_ADD(addr, sizeof(uint32_t));
    }
    return VFSC_VF_INDEX_INVALID;
}

static int vfsc_vf_file(VFSC_DATA *vfs, uint32_t vf_index, void **data, uint32_t *size) {
    uint32_t file_size = 0;
    void *addr = vfs->vfs_data;

    // move to hash table
    addr = VFSC_ADDR_ADD(addr, VFSC_VF_OFFSET_HASH_TABLE);

    // move to data table
    addr = VFSC_ADDR_ADD(addr, vfs->vfs_file_count * sizeof(uint32_t));

    // find the file
    while (vf_index > 0) {
        // jump over this file
        file_size = vfsc_read_uint32_le(vfs, addr);
        addr = VFSC_ADDR_ADD(addr, sizeof(uint32_t) + file_size);
        vf_index--;
    }

    // read the file size
    if (size) {
        *size = vfsc_read_uint32_le(vfs, addr);
    }

    // move past the data size to the data
    addr = VFSC_ADDR_ADD(addr, sizeof(uint32_t));
    if (data) {
        *data = addr;
    }

    // done
    return 0;
}

static void *vfsc_vf_addr(VFSC_DATA *vfs, uint32_t vf_index) {
    return NULL;
}

static int vfsc_new_handle(VFSC_DATA *vfs, uint32_t vf_index) {
    int fd = -1;
    if (VFSC_VF_INDEX_INVALID == vf_index) {
        return -1;
    }
    for (fd = 0; fd < vfs->vfs_max_handles; fd++) {
        if (VFSC_VF_INDEX_INVALID == (vfs->vf_file_handle[fd].vf_index)) {
            vfs->vf_file_handle[fd].vf_index = vf_index;
            vfs->vf_file_handle[fd].vf_offset = 0;
            return fd;
        }
    }
    return -1;
}

static int vfsc_del_handle(VFSC_DATA *vfs, int fd) {
    if (fd >= vfs->vfs_max_handles) {
        return -1;
    }
    if (VFSC_VF_INDEX_INVALID != (vfs->vf_file_handle[fd].vf_index)) {
        vfs->vf_file_handle[fd].vf_index = VFSC_VF_INDEX_INVALID;
        vfs->vf_file_handle[fd].vf_offset = 0;
        return 0;
    }
    return -1;
}

static int vfsc_get_handle(VFSC_DATA *vfs, int fd, VFSC_HANDLE_DATA **handle) {
    if (fd >= vfs->vfs_max_handles) {
        return -1;
    }
    if (VFSC_VF_INDEX_INVALID == (vfs->vf_file_handle[fd].vf_index)) {
        return -1;
    }
    if (handle) {
        *handle = &(vfs->vf_file_handle[fd]);
    }
    return 0;
}

int vfsc_open_hash(void *data, uint32_t hash) {
    uint32_t vf_index = VFSC_VF_INDEX_INVALID;
    VFSC_DATA *vfs = (VFSC_DATA *)data;
    if (NULL == vfs) {
        return -1;
    }
    vf_index = vfsc_vf_index(vfs, hash);
    if (VFSC_VF_INDEX_INVALID == vf_index) {
        return -1;
    }
    return vfsc_new_handle(vfs, vf_index);
}

ssize_t vfsc_read(void *data, int fd, void *buf, size_t count) {
    VFSC_HANDLE_DATA *handle = NULL;
    uint32_t file_size = 0;
    void *file_data = NULL;
    uint32_t file_offset = 0;
    uint32_t byte_index = 0;
    VFSC_DATA *vfs = (VFSC_DATA *)data;
    if (NULL == vfs) {
        return -1;
    }
    if (count <= 0) {
        return 0;
    }
    if (NULL == buf) {
        return 0;
    }
    if (0 != vfsc_get_handle(vfs, fd, &handle)) {
        return -1;
    }
    if (0 != vfsc_vf_file(vfs, handle->vf_index, &file_data, &file_size)) {
        return -1;
    }
    if (handle->vf_offset >= file_size) {
        return 0;
    }
    if ((handle->vf_offset + ((uint32_t)count)) > file_size) {
        count = (size_t)(file_size - handle->vf_offset);
    }
    for (byte_index = 0; byte_index < (uint32_t)count; byte_index++) {
        *(((uint8_t*)buf) + byte_index) = vfs->read_byte(vfs->read_byte_ctx, VFSC_ADDR_ADD(file_data, handle->vf_offset + byte_index));
    }
    handle->vf_offset += (uint32_t)count;
    return (ssize_t)count;
}

int vfsc_close(void *data, int fd) {
    VFSC_DATA *vfs = (VFSC_DATA *)data;
    if (NULL == vfs) {
        return -1;
    }
    return vfsc_del_handle(vfs, fd);
}

void vfsc_fini(void *data) {
    uint32_t size = sizeof(VFSC_DATA);
    if (NULL == data) {
        return -1;
    }
    memset(data, 0, size);
    return 0;
}
