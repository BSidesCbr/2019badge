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

int vfsc_init(void *data, size_t size, void *vfs_data, size_t vfs_data_size, VfscReadByteFn read_byte, void *read_byte_ctx) {
    ssize_t fd = 0;
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

static ssize_t vfsc_vf_index(VFSC_DATA *vfs, uint32_t hash) {
    ssize_t vf_index = 0;
    void *addr = VFSC_ADDR_ADD(vfs->vfs_data, VFSC_VF_OFFSET_HASH_TABLE);
    for (vf_index = 0; vf_index < vfs->vfs_file_count; vf_index++) {
        if (hash == vfsc_read_uint32_le(vfs, addr)) {
            return vf_index;
        }
        addr = VFSC_ADDR_ADD(addr, sizeof(uint32_t));
    }
    return VFSC_VF_INDEX_INVALID;
}

static ssize_t vfsc_vf_file(VFSC_DATA *vfs, ssize_t vf_index, void **data, size_t *size) {
    size_t file_size = 0;
    void *addr = vfs->vfs_data;

    // move to hash table
    addr = VFSC_ADDR_ADD(addr, VFSC_VF_OFFSET_HASH_TABLE);

    // move to data table
    addr = VFSC_ADDR_ADD(addr, vfs->vfs_file_count * sizeof(uint32_t));

    // find the file
    while (vf_index > 0) {
        // jump over this file
        file_size = (size_t)vfsc_read_uint32_le(vfs, addr);
        addr = VFSC_ADDR_ADD(addr, sizeof(uint32_t) + file_size);
        vf_index--;
    }

    // read the file size
    if (size) {
        *size = (size_t)vfsc_read_uint32_le(vfs, addr);
    }

    // move past the data size to the data
    addr = VFSC_ADDR_ADD(addr, sizeof(uint32_t));
    if (data) {
        *data = addr;
    }

    // done
    return 0;
}

static ssize_t vfsc_new_handle(VFSC_DATA *vfs, ssize_t vf_index) {
    ssize_t fd = -1;
    size_t file_size = 0;
    void *file_data = NULL;
    if (VFSC_VF_INDEX_INVALID == vf_index) {
        return -1;
    }
    if (0 != vfsc_vf_file(vfs, vf_index, &file_data, &file_size)) {
        return -1;
    }
    for (fd = 0; fd < vfs->vfs_max_handles; fd++) {
        if (VFSC_VF_INDEX_INVALID == (vfs->vf_file_handle[fd].vf_index)) {
            vfs->vf_file_handle[fd].vf_index = vf_index;
            vfs->vf_file_handle[fd].vf_offset = 0;
            vfs->vf_file_handle[fd].data = file_data;
            vfs->vf_file_handle[fd].size = file_size;
            return fd;
        }
    }
    return -1;
}

static ssize_t vfsc_del_handle(VFSC_DATA *vfs, ssize_t fd) {
    if (fd >= vfs->vfs_max_handles) {
        return -1;
    }
    if (VFSC_VF_INDEX_INVALID != (vfs->vf_file_handle[fd].vf_index)) {
        vfs->vf_file_handle[fd].vf_index = VFSC_VF_INDEX_INVALID;
        vfs->vf_file_handle[fd].vf_offset = 0;
        vfs->vf_file_handle[fd].data = NULL;
        vfs->vf_file_handle[fd].size = 0;
        return 0;
    }
    return -1;
}

static ssize_t vfsc_get_handle(VFSC_DATA *vfs, ssize_t fd, VFSC_HANDLE_DATA **handle) {
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
    ssize_t vf_index = VFSC_VF_INDEX_INVALID;
    VFSC_DATA *vfs = (VFSC_DATA *)data;
    if (NULL == vfs) {
        return -1;
    }
    vf_index = vfsc_vf_index(vfs, hash);
    if (VFSC_VF_INDEX_INVALID == vf_index) {
        return -1;
    }
    return (int)vfsc_new_handle(vfs, vf_index);
}

ssize_t vfsc_read(void *data, int fd, void *buf, size_t count) {
    VFSC_HANDLE_DATA *handle = NULL;
    size_t file_size = 0;
    void *file_data = NULL;
    size_t file_offset = 0;
    size_t byte_index = 0;
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
    file_data = handle->data;
    file_size = handle->size;
    if (handle->vf_offset >= file_size) {
        return 0;
    }
    if ((((size_t)handle->vf_offset) + count) > file_size) {
        count = file_size - (size_t)handle->vf_offset;
    }
    for (byte_index = 0; byte_index < count; byte_index++) {
        *(((uint8_t*)buf) + byte_index) = vfs->read_byte(vfs->read_byte_ctx, VFSC_ADDR_ADD(file_data, handle->vf_offset + byte_index));
    }
    handle->vf_offset += (ssize_t)count;
    return (ssize_t)count;
}

off_t vfsc_lseek(void *data, int fd, off_t offset, int whence) {
    VFSC_HANDLE_DATA *handle = NULL;
    size_t file_size = 0;
    void *unused = NULL;
    VFSC_DATA *vfs = (VFSC_DATA *)data;
    if (NULL == vfs) {
        return -1;
    }
    if (0 != vfsc_get_handle(vfs, fd, &handle)) {
        return -1;
    }
    file_size = handle->size;
    if (VFSC_SEEK_SET == whence) {
        handle->vf_offset = (ssize_t)offset;
    } else if (VFSC_SEEK_CUR == whence) {
        handle->vf_offset += (ssize_t)offset;
    } else if (VFSC_SEEK_END == whence) {
        handle->vf_offset = (ssize_t)file_size + (ssize_t)offset;
    }
    if (handle->vf_offset > file_size) {
        handle->vf_offset = (ssize_t)file_size;
    }
    return (off_t)handle->vf_offset;
}

int vfsc_close(void *data, int fd) {
    VFSC_DATA *vfs = (VFSC_DATA *)data;
    if (NULL == vfs) {
        return -1;
    }
    return (int)vfsc_del_handle(vfs, fd);
}

void vfsc_fini(void *data) {
    uint32_t size = sizeof(VFSC_DATA);
    if (NULL == data) {
        return -1;
    }
    memset(data, 0, size);
    return 0;
}
