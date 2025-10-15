/*
 * DevFS - Device Filesystem
 * 
 * Virtual filesystem that exposes devices as files in /dev
 * Provides device nodes for:
 * - Character devices (keyboard, serial, tty)
 * - Block devices (disks, partitions)
 * - Special devices (null, zero, random)
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "kernel.h"
#include "vfs.h"
#include "device.h"
#include <string.h>

// Forward declarations
extern void* kmalloc(size_t size);
extern void kfree(void* ptr);

// DevFS vnode private data
typedef struct {
    device_t* device;
    u32 major;
    u32 minor;
} devfs_node_t;

// DevFS operations
static long devfs_read(vnode_t* vn, u64 off, void* buf, size_t len) {
    devfs_node_t* node = (devfs_node_t*)vn->fs_priv;
    if (!node || !node->device) return -1;
    
    return device_read(node->device, off, buf, len);
}

static long devfs_write(vnode_t* vn, u64 off, const void* buf, size_t len) {
    devfs_node_t* node = (devfs_node_t*)vn->fs_priv;
    if (!node || !node->device) return -1;
    
    return device_write(node->device, off, buf, len);
}

static vnode_ops_t devfs_vnode_ops = {
    .read = devfs_read,
    .write = devfs_write,
};

// Create a devfs vnode for a device
vnode_t* devfs_create_device_node(device_t* dev) {
    vnode_t* vn = kmalloc(sizeof(vnode_t));
    if (!vn) return NULL;
    
    memset(vn, 0, sizeof(vnode_t));
    
    devfs_node_t* node = kmalloc(sizeof(devfs_node_t));
    if (!node) {
        kfree(vn);
        return NULL;
    }
    
    node->device = dev;
    node->major = dev->major;
    node->minor = dev->minor;
    
    vn->ino = (dev->major << 16) | dev->minor;
    vn->size = 0;
    vn->mode = 0600; // rw-------
    if (dev->type == DEV_TYPE_CHAR) {
        vn->mode |= 0x2000; // S_IFCHR
    } else if (dev->type == DEV_TYPE_BLOCK) {
        vn->mode |= 0x6000; // S_IFBLK
    }
    
    vn->ops = &devfs_vnode_ops;
    vn->fs_priv = node;
    
    return vn;
}

// Special device: /dev/null
static long devnull_read(vnode_t* vn, u64 off, void* buf, size_t len) {
    (void)vn; (void)off; (void)buf; (void)len;
    return 0; // Always return EOF
}

static long devnull_write(vnode_t* vn, u64 off, const void* buf, size_t len) {
    (void)vn; (void)off; (void)buf;
    return len; // Pretend to write everything
}

static vnode_ops_t devnull_ops = {
    .read = devnull_read,
    .write = devnull_write,
};

// Special device: /dev/zero
static long devzero_read(vnode_t* vn, u64 off, void* buf, size_t len) {
    (void)vn; (void)off;
    memset(buf, 0, len);
    return len;
}

static long devzero_write(vnode_t* vn, u64 off, const void* buf, size_t len) {
    (void)vn; (void)off; (void)buf;
    return len; // Discard everything
}

static vnode_ops_t devzero_ops = {
    .read = devzero_read,
    .write = devzero_write,
};

// Initialize devfs special devices
void devfs_init(void) {
    // Create /dev/null device
    device_t* null_dev = char_device_create("null", 1, 3);
    if (null_dev) {
        device_ops_t* ops = kmalloc(sizeof(device_ops_t));
        if (ops) {
            memset(ops, 0, sizeof(device_ops_t));
            null_dev->ops = ops;
            device_register(null_dev);
        }
    }
    
    // Create /dev/zero device
    device_t* zero_dev = char_device_create("zero", 1, 5);
    if (zero_dev) {
        device_ops_t* ops = kmalloc(sizeof(device_ops_t));
        if (ops) {
            memset(ops, 0, sizeof(device_ops_t));
            zero_dev->ops = ops;
            device_register(zero_dev);
        }
    }
    
    kprintf("[DEVFS] Device filesystem initialized\n");
}
