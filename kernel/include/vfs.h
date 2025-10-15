#pragma once
#include "kernel.h"
#include "block.h"

/*
 VFS core (Phase 6 minimal, read-only)
 - Single-root mount with mount table abstraction
 - vnode with ops for file and directory
 - Path lookup, open, read, readdir
 - Filesystem registration and mount by type

 This VFS is intentionally minimal to bootstrap userspace on read-only ext4/FAT32.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VNODE_FILE = 1,
    VNODE_DIR  = 2,
    VNODE_SYMLINK = 3
} vnode_type_t;

struct vnode;
struct vfs_mount;
struct vfs_super;

/* Directory read callback */
typedef int (*vfs_dirent_cb)(const char* name, size_t namelen, int is_dir, void* ctx);

typedef struct vnode_ops {
    /* Read up to 'len' bytes into 'buf' starting at 'off'. Return bytes read or <0 on error. */
    long (*read)(struct vnode* vn, u64 off, void* buf, size_t len);

    /* Optional write callback: overwrite/extend file starting at off. Returns bytes written or <0. */
    long (*write)(struct vnode* vn, u64 off, const void* buf, size_t len);

    /* Iterate directory entries; call cb for each. Return 0 or error. */
    int  (*readdir)(struct vnode* vn, vfs_dirent_cb cb, void* ctx);

    /* Lookup child name under directory; returns new referenced vnode or NULL on error. */
    struct vnode* (*lookup)(struct vnode* dir, const char* name, size_t namelen);

    /* Drop reference (optional no-op for static vnodes) */
    void (*release)(struct vnode* vn);
} vnode_ops_t;

typedef struct vnode {
    struct vfs_mount* mnt;
    vnode_type_t type;
    u64 size;
    u64 ino;
    void* fs_priv;       /* fs-specific inode pointer or data */
    const vnode_ops_t* ops;
    /* Basic ownership & permissions (POSIX-like subset) */
    u32 mode;            /* permission bits (e.g., 0755) */
    u32 uid;             /* owner user id */
    u32 gid;             /* owner group id */
} vnode_t;

typedef struct vfs_super_ops {
    /* Root directory vnode of this superblock */
    vnode_t* (*get_root)(struct vfs_super* sb);
    /* Optional: release superblock */
    void     (*put_super)(struct vfs_super* sb);
} vfs_super_ops_t;

typedef struct vfs_super {
    struct vfs_mount* mnt;
    block_dev_t* bdev;
    u32 block_size; /* fs block size */
    void* fs_priv;  /* fs-specific super data */
    const vfs_super_ops_t* ops;
} vfs_super_t;

typedef struct vfs_mount {
    char mountpoint[128]; /* e.g., "/" */
    char fstype[16];      /* "ext4" or "fat32" */
    vfs_super_t* sb;
} vfs_mount_t;

typedef struct fs_type {
    const char* name;
    /* Probe and mount: given block device, create superblock and return it */
    int (*mount)(block_dev_t* bdev, vfs_super_t** out_sb);
} fs_type_t;

/* Registration */
int vfs_register_fs(const fs_type_t* type);

/* Mounting */
int vfs_mount_root(const char* fsname, block_dev_t* bdev);
int vfs_mount_at(const char* fsname, block_dev_t* bdev, const char* path);

/* Lookup and open */
int vfs_lookup(const char* path, vnode_t** out);
long vfs_read_path(const char* path, u64 off, void* buf, size_t len);

/* Simple file descriptor style API (Phase consolidation shim) */
typedef struct file file_t;
struct file {
    vnode_t* vn;
    u64 offset;
    int flags;
};

/* ---------------- VFS Event Hook Framework (Phase 1 Observability) ---------------- */
typedef struct vfs_open_event {
    const char* path;
    int flags; /* VFS_O_* */
    int rc;    /* result after open (0 success or -errno) */
} vfs_open_event_t;

typedef struct vfs_create_event {
    const char* path;
    u32 mode;
    int rc;
} vfs_create_event_t;

typedef struct vfs_unlink_event {
    const char* path;
    int rc;
} vfs_unlink_event_t;

typedef void (*vfs_open_hook_t)(const vfs_open_event_t* ev);
typedef void (*vfs_create_hook_t)(const vfs_create_event_t* ev);
typedef void (*vfs_unlink_hook_t)(const vfs_unlink_event_t* ev);

int vfs_register_open_hook(vfs_open_hook_t fn);
int vfs_register_create_hook(vfs_create_hook_t fn);
int vfs_register_unlink_hook(vfs_unlink_hook_t fn);

/* Security preflight hooks (can deny operations) */
typedef int (*security_exec_hook_t)(const char* path, const char* const argv[]);
typedef int (*security_open_hook_t)(const char* path, int flags);
typedef int (*security_socket_hook_t)(int domain, int type, int protocol);

int security_register_exec_hook(security_exec_hook_t fn);
int security_register_open_hook(security_open_hook_t fn);
int security_register_socket_hook(security_socket_hook_t fn);

/* Aggregated per-category hook & security stats */
typedef struct vfs_sec_hook_stats {
    /* Event emission counts */
    u64 open_events;
    u64 create_events;
    u64 unlink_events;
    /* Registration counts (snapshotted on read) */
    u32 open_hooks;
    u32 create_hooks;
    u32 unlink_hooks;
    /* Security checks */
    u64 exec_checks;
    u64 exec_denied;
    u64 open_checks;
    u64 open_denied;
    u64 socket_checks;
    u64 socket_denied;
    /* Security hook registration counts */
    u32 exec_hooks;
    u32 sec_open_hooks;
    u32 socket_sec_hooks;
} vfs_sec_hook_stats_t;

const vfs_sec_hook_stats_t* vfs_sec_get_hook_stats(void);
void vfs_sec_reset_hook_stats(void);

/* Open flags */
#define VFS_O_RDONLY  0x0001
#define VFS_O_WRONLY  0x0002
#define VFS_O_RDWR    0x0004
#define VFS_O_CREAT   0x0100

/* Seek whence */
#define VFS_SEEK_SET 0
#define VFS_SEEK_CUR 1
#define VFS_SEEK_END 2

int  vfs_open(const char* path, int flags, file_t** out);
int  vfs_close(file_t* f);
int  vfs_read(file_t* f, void* buf, u64 len, u64* out_rd);
int  vfs_write(file_t* f, const void* buf, u64 len, u64* out_wr);
int  vfs_seek(file_t* f, s64 off, int whence);
u64  vfs_tell(file_t* f);

/* Creation / mutation helpers (tmpfs only in current phase) */
int  vfs_create_file(const char* path, u32 mode, file_t** opt_file);
int  vfs_mkdir_path(const char* path, u32 mode);
int  vfs_unlink_path(const char* path);
int  vfs_symlink_path(const char* target, const char* linkpath, u32 mode);
int  vfs_readlink_path(const char* path, char* buf, size_t buflen, u64* out_len);

/* Directory listing (calls cb for each entry); use path to a directory */
int vfs_listdir(const char* path, vfs_dirent_cb cb, void* ctx);

/* Helpers */
vnode_t* vfs_ref(vnode_t* vn);
void     vfs_put(vnode_t* vn);
vfs_mount_t* vfs_get_root_mount(void);

/* Phase 6 bootstrap: mounts root on vda and /tmp tmpfs (if available), then execs /sbin/init */
int vfs_phase6_bootstrap_mount_and_exec(void);

/* Exported mount API for sys_mount handler to call later by device name */
int vfs_mount_device_by_name(const char* fsname, const char* devname, const char* path);

/* FS registry access (for internal use) */
const fs_type_t* vfs_find_fs(const char* name);

/* Low-level block IO helper */
int bdev_read_bytes(block_dev_t* bdev, u64 off_bytes, void* buf, size_t len);
int bdev_write_bytes(block_dev_t* bdev, u64 off_bytes, const void* buf, size_t len);

/* VFS initialization helpers */
void vfs_init_dirs(void);

#ifdef __cplusplus
}
#endif
