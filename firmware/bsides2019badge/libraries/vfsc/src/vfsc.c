#include "vfsc.h"

uint8_t vfsc_addr_read(void *data, vfsc_addr_t addr) {
    VFSC_DATA *vfs = (VFSC_DATA *)data;
    return vfs->read_byte(vfs->read_byte_ctx, addr);
}

size_t vfsc_addr_offset(void *data, vfsc_addr_t addr) {
    data = data;
    return (size_t)addr;
}

size_t vfsc_key_offset(void *data, size_t offset) {
    data = data;
    return offset % VFSC_KEY_SIZE;
}

vfsc_addr_t vfsc_addr_ptr(void *data, size_t offset) {
    data = data;
    return (vfsc_addr_t)offset;
}

uint8_t vfsc_read_byte(void *data, vfsc_addr_t addr) {
    uint8_t encb = vfsc_addr_read(data, addr);
    size_t offset = vfsc_addr_offset(data, addr);
    size_t keyoffset = vfsc_key_offset(data, offset);
    vfsc_addr_t keyaddr = vfsc_addr_ptr(data, keyoffset);
    uint8_t keyb = vfsc_addr_read(data, keyaddr);
    keyb = keyb ^ (0xff - (uint8_t)(offset & 0xff));
    keyb = keyb ^ 0xa5;
    uint8_t valb = encb ^ keyb;
    return valb;
}

static vfsc_size_t vfsc_read_value(void *data, vfsc_addr_t addr) {
    vfsc_size_t value = 0;
    value |= ((vfsc_size_t)(vfsc_read_byte(data, addr + 0)) << 0 ) & 0xff;
    if (sizeof(vfsc_size_t) >= 2) {
        value |= ((vfsc_size_t)(vfsc_read_byte(data, addr + 1)) << 8 ) & 0xff00;
    }
    if (sizeof(vfsc_size_t) >= 4) {
        value |= ((vfsc_size_t)(vfsc_read_byte(data, addr + 2)) << 16) & 0x00ff0000;
        value |= ((vfsc_size_t)(vfsc_read_byte(data, addr + 3)) << 24) & 0xff000000;
    }
    return value;
}
#define vfsc_read_reserved(data,addr)       vfsc_read_value(data,addr)
#define vfsc_read_file_count(data,addr)     vfsc_read_value(data,addr)
#define vfsc_read_file_hash(data,addr)      ((vfsc_hash_t)(vfsc_read_value(data,addr)))
#define vfsc_read_file_size(data,addr)      vfsc_read_value(data,addr)

int vfsc_init(void *data, size_t size, size_t vfs_data_size, VfscReadByteFn read_byte, void *read_byte_ctx) {
    ssize_t fd = 0;
    if (!data) {
        return -1;
    }
    if (size < sizeof(VFSC_DATA)) {
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
    if (0 != vfsc_read_reserved(data, VFSC_VF_OFFSET_RESERVED)) {
        // reserved not zero
        return -1;
    }
    ((VFSC_DATA*)data)->vfs_data_size = vfs_data_size;
    ((VFSC_DATA*)data)->vfs_file_count = vfsc_read_file_count(data, VFSC_VF_OFFSET_FILE_COUNT);
    ((VFSC_DATA*)data)->vfs_max_handles = ((size - sizeof(VFSC_DATA) - sizeof(VFSC_HANDLE_DATA)) / sizeof(VFSC_HANDLE_DATA));
    for (fd = 0; fd < ((VFSC_DATA*)data)->vfs_max_handles; fd++) {
        ((VFSC_DATA*)data)->vf_file_handle[fd].vf_index = VFSC_VF_INDEX_INVALID;
        ((VFSC_DATA*)data)->vf_file_handle[fd].vf_offset = 0;
    }
    return 0;
}

static ssize_t vfsc_vf_index(VFSC_DATA *vfs, vfsc_hash_t hash) {
    ssize_t vf_index = 0;
    vfsc_addr_t addr = VFSC_VF_OFFSET_HASH_TABLE;
    for (vf_index = 0; vf_index < vfs->vfs_file_count; vf_index++) {
        if (hash == vfsc_read_file_hash(vfs, addr)) {
            return vf_index;
        }
        addr += sizeof(vfsc_hash_t);
    }
    return VFSC_VF_INDEX_INVALID;
}

static ssize_t vfsc_vf_file(VFSC_DATA *vfs, ssize_t vf_index, void **data, size_t *size) {
    size_t file_size = 0;
    vfsc_addr_t addr = 0;

    // move to hash table
    addr += VFSC_VF_OFFSET_HASH_TABLE;

    // move to data table
    addr += vfs->vfs_file_count * sizeof(vfsc_hash_t);

    // find the file
    while (vf_index > 0) {
        // jump over this file
        file_size = (size_t)vfsc_read_file_size(vfs, addr);
        addr += sizeof(vfsc_size_t) + file_size;
        vf_index--;
    }

    // read the file size
    if (size) {
        *size = (size_t)vfsc_read_file_size(vfs, addr);
    }

    // move past the data size to the data
    addr += sizeof(vfsc_size_t);
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

int vfsc_open_hash(void *data, vfsc_hash_t hash) {
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
    vfsc_addr_t file_data = 0;
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
        *(((uint8_t*)buf) + byte_index) = vfsc_read_byte(data, file_data + handle->vf_offset + byte_index);
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
    size_t size = sizeof(VFSC_DATA);
    if (NULL == data) {
        return;
    }
    memset(data, 0, size);
    return;
}
