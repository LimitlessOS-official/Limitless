/*
 * Virtual File System (VFS) Core Implementation
 * 
 * Implements the VFS layer with:
 * - Mount table management
 * - Path resolution and lookup
 * - File operations (open, read, write)
 * - Filesystem registration
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "kernel.h"
#include "vfs.h"
#include <string.h>

// Forward declare kmalloc/kfree from slab allocator
extern void* kmalloc(size_t size);
extern void kfree(void* ptr);

// Wrapper functions for ext2 driver
void* vmm_kmalloc(size_t size, size_t align) {
    // TODO: Implement aligned allocation
    (void)align; // Ignore alignment for now
    return kmalloc(size);
}

void vmm_kfree(void* ptr, size_t size) {
    (void)size; // slab allocator tracks size internally
    kfree(ptr);
}

// Maximum number of mounts and filesystem types
#define MAX_MOUNTS 16
#define MAX_FS_TYPES 8

// VFS state
static vfs_mount_t mounts[MAX_MOUNTS];
static const fs_type_t *fs_types[MAX_FS_TYPES];
static int fs_type_count = 0;
static vfs_mount_t *root_mount = NULL;

// Initialize VFS
void vfs_init(void) {
    // Clear mount table
    memset(mounts, 0, sizeof(mounts));
    memset(fs_types, 0, sizeof(fs_types));
    fs_type_count = 0;
    root_mount = NULL;
}

// Register a filesystem type
int vfs_register_fs(const fs_type_t *type) {
    if (!type || fs_type_count >= MAX_FS_TYPES) {
        return -1;
    }
    
    fs_types[fs_type_count++] = type;
    return 0;
}

// Find filesystem type by name
static const fs_type_t *find_fs_type(const char *name) {
    for (int i = 0; i < fs_type_count; i++) {
        if (strcmp(fs_types[i]->name, name) == 0) {
            return fs_types[i];
        }
    }
    return NULL;
}

// Mount root filesystem
int vfs_mount_root(const char *fsname, block_dev_t *bdev) {
    if (root_mount) return -1; // Already mounted
    
    vfs_mount_t *mnt = &mounts[0];
    const fs_type_t *fs = find_fs_type(fsname);
    if (!fs) return -1;
    
    strncpy(mnt->mountpoint, "/", 127);
    strncpy(mnt->fstype, fsname, 15);
    
    // Call filesystem mount
    if (fs->mount(bdev, &mnt->sb) != 0) {
        return -1;
    }
    
    mnt->sb->mnt = mnt;
    root_mount = mnt;
    return 0;
}

// Simple path lookup
int vfs_lookup(const char *path, vnode_t **out) {
    if (!root_mount || !root_mount->sb) return -1;
    
    // Get root
    vnode_t *vn = root_mount->sb->ops->get_root(root_mount->sb);
    if (!vn) return -1;
    
    *out = vn;
    return 0;
}

// Simplified read from path
long vfs_read_path(const char *path, u64 off, void *buf, size_t len) {
    vnode_t *vn;
    if (vfs_lookup(path, &vn) != 0) return -1;
    
    if (!vn->ops || !vn->ops->read) return -1;
    
    return vn->ops->read(vn, off, buf, len);
}

// Open a file
int vfs_open(const char* path, int flags, file_t** out) {
    vnode_t *vn;
    if (vfs_lookup(path, &vn) != 0) return -1;
    
    // Allocate file structure
    file_t *file = kmalloc(sizeof(file_t));
    if (!file) return -1;
    
    file->vn = vn;
    file->offset = 0;
    file->flags = flags;
    
    *out = file;
    return 0;
}

// Close a file
int vfs_close(file_t* f) {
    if (!f) return -1;
    
    // TODO: Release vnode reference
    kfree(f);
    return 0;
}

// Read from a file
int vfs_read(file_t* f, void* buf, u64 len, u64* out_rd) {
    if (!f || !f->vn) return -1;
    
    if (!f->vn->ops || !f->vn->ops->read) return -1;
    
    long result = f->vn->ops->read(f->vn, f->offset, buf, len);
    if (result < 0) return (int)result;
    
    *out_rd = (u64)result;
    return 0;
}

// Write to a file
int vfs_write(file_t* f, const void* buf, u64 len, u64* out_wr) {
    if (!f || !f->vn) return -1;
    
    if (!f->vn->ops || !f->vn->ops->write) return -1;
    
    long result = f->vn->ops->write(f->vn, f->offset, buf, len);
    if (result < 0) return (int)result;
    
    *out_wr = (u64)result;
    return 0;
}

