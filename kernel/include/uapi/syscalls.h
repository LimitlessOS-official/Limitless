#pragma once
#include <stdint.h>

/*
 * UAPI syscalls (Phase 5 additions)
 * Keep in sync with userspace/include/syscall.h
 */

enum {
    SYS_write = 1,
    SYS_exit  = 2,
    SYS_yield = 3,
    SYS_sleep_ms = 4,
    SYS_getpid   = 5,


    SYS_fork = 6,
    SYS_waitpid = 7,
    SYS_execve = 10,

    SYS_ipc_create = 20,
    SYS_ipc_close  = 21,
    SYS_ipc_send   = 22,
    SYS_ipc_recv   = 23,

    /* VFS */
    SYS_pipe   = 40,
    SYS_mount  = 41,
    SYS_open   = 42,
    SYS_read   = 43,
    SYS_pwrite = 44,
    SYS_close  = 45,

    /* Directory and metadata */
    SYS_stat    = 46,
    SYS_readdir = 47,
    SYS_mkdir   = 48,
    SYS_unlink  = 49,

    /* Sockets (full TCP/IP stack) */
    SYS_socket     = 60,
    SYS_bind       = 61,
    SYS_listen     = 62,
    SYS_accept     = 63,
    SYS_connect    = 64,
    SYS_send       = 65,
    SYS_recv       = 66,
    SYS_sendto     = 67,
    SYS_recvfrom   = 68,
    SYS_close_socket = 69,

    /* Network helpers */
    SYS_netping    = 70,
    SYS_netconnect = 71,
    SYS_net_init   = 72,
    SYS_net_icmp_ping = 73,

    /* Graphics and windowing */
    SYS_graphics_init = 80,
    SYS_graphics_get_screen_info = 81,
    SYS_graphics_create_window = 82,
    SYS_graphics_create_buffer = 83,
    SYS_graphics_fill_rect = 84,
    SYS_graphics_blit = 85,
    SYS_graphics_present = 86,
    SYS_graphics_destroy_window = 87,
    SYS_graphics_destroy_buffer = 88,

    /* Auth and identity */
    SYS_auth_login = 80,
    SYS_getuid     = 81,
    SYS_setuid     = 82,
    SYS_getgid     = 83,
    SYS_setgid     = 84,

    /* OTA Update (A/B) */
    SYS_update_get_meta = 90,
    SYS_update_begin = 91,
    SYS_update_apply = 92,
    SYS_update_finalize = 93,
    SYS_update_mark_success = 94,
    SYS_update_rollback = 95,

    /* Observability */
    SYS_klog_read = 100,

    /* Firewall control */
    SYS_fw_add_rule     = 110,
    SYS_fw_del_rule     = 111,
    SYS_fw_list_rules   = 112,
    SYS_fw_set_default  = 113,
    SYS_fw_get_default  = 114,

    /* File permission management */
    SYS_umask           = 120,
    SYS_chmod           = 121,
    SYS_chown           = 122,

    /* Symlink operations */
    SYS_symlink         = 130,
    SYS_readlink        = 131,

    /* Signal handling */
    SYS_kill            = 62,
    SYS_rt_sigaction    = 13,
    SYS_rt_sigprocmask  = 14,
    SYS_rt_sigreturn    = 15,
};

/* UAPI structs for basic file metadata and directory entries */
typedef struct {
    uint64_t size;     /* file size in bytes */
    uint32_t type;     /* maps to vnode_type_t: file=1, dir=2, symlink=3 */
    uint32_t mode;     /* permission bits */
    uint64_t ino;      /* inode number if available */
    uint32_t uid;      /* owner user id */
    uint32_t gid;      /* owner group id */
} u_stat_t;

typedef struct {
    char     name[256];
    uint32_t is_dir;   /* non-zero if entry is a directory */
    uint32_t _pad;     /* reserved */
} u_dirent_t;

/* UAPI structs for firewall rules */
typedef struct {
    uint8_t  direction;     /* 0=input, 1=output */
    uint8_t  proto;         /* 1=ICMP, 6=TCP, 17=UDP; 0=any */
    uint16_t _pad0;
    uint32_t src_ip_be;     /* 0 to wildcard */
    uint32_t src_mask_be;   /* 0 to wildcard */
    uint32_t dst_ip_be;     /* 0 to wildcard */
    uint32_t dst_mask_be;   /* 0 to wildcard */
    uint16_t src_port_be;   /* 0 to wildcard; ICMP uses type/code in ports if needed */
    uint16_t dst_port_be;   /* 0 to wildcard */
    uint8_t  action;        /* 0=DROP, 1=ACCEPT */
    uint8_t  _pad1[7];
} u_fw_rule_t;