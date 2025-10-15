#pragma once
/* POSIX-compatible syscall numbers for LimitlessOS */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Standard Linux syscall numbers (x86_64) for compatibility */
#define SYS_read           0
#define SYS_write          1
#define SYS_open           2
#define SYS_close          3
#define SYS_stat           4
#define SYS_fstat          5
#define SYS_lstat          6
#define SYS_poll           7
#define SYS_lseek          8
#define SYS_mmap           9
#define SYS_mprotect      10
#define SYS_munmap        11
#define SYS_brk           12
#define SYS_rt_sigaction  13
#define SYS_rt_sigprocmask 14
#define SYS_rt_sigreturn  15
#define SYS_ioctl         16
#define SYS_pread64       17
#define SYS_pwrite64      18
#define SYS_readv         19
#define SYS_writev        20
#define SYS_access        21
#define SYS_pipe          22
#define SYS_select        23
#define SYS_sched_yield   24
#define SYS_mremap        25
#define SYS_msync         26
#define SYS_mincore       27
#define SYS_madvise       28
#define SYS_shmget        29
#define SYS_shmat         30
#define SYS_shmctl        31
#define SYS_dup           32
#define SYS_dup2          33
#define SYS_pause         34
#define SYS_nanosleep     35
#define SYS_getitimer     36
#define SYS_alarm         37
#define SYS_setitimer     38
#define SYS_getpid        39
#define SYS_sendfile      40
#define SYS_socket        41
#define SYS_connect       42
#define SYS_accept        43
#define SYS_sendto        44
#define SYS_recvfrom      45
#define SYS_sendmsg       46
#define SYS_recvmsg       47
#define SYS_shutdown      48
#define SYS_bind          49
#define SYS_listen        50
#define SYS_getsockname   51
#define SYS_getpeername   52
#define SYS_socketpair    53
#define SYS_setsockopt    54
#define SYS_getsockopt    55
#define SYS_clone         56
#define SYS_fork          57
#define SYS_vfork         58
#define SYS_execve        59
#define SYS_exit          60
#define SYS_wait4         61
#define SYS_waitpid       61  /* Alias for wait4 */
#define SYS_kill          62
#define SYS_uname         63

/* File system syscalls */
#define SYS_getcwd        79
#define SYS_chdir         80
#define SYS_mkdir         83
#define SYS_rmdir         84
#define SYS_unlink        87
#define SYS_symlink       88
#define SYS_readlink      89
#define SYS_chmod         90
#define SYS_chown         92
#define SYS_umask         95
#define SYS_getuid       102
#define SYS_syslog       103
#define SYS_getgid       104
#define SYS_setuid       105
#define SYS_setgid       106
#define SYS_geteuid      107
#define SYS_getegid      108

/* LimitlessOS-specific syscalls */
#define SYS_sleep_ms     150
#define SYS_netping      151
#define SYS_netconnect   152
#define SYS_ipc_create   160
#define SYS_ipc_close    161
#define SYS_ipc_send     162
#define SYS_ipc_recv     163
#define SYS_auth_login   200
#define SYS_update_get_meta 201
#define SYS_update_begin    202
#define SYS_update_apply    203
#define SYS_update_finalize 204
#define SYS_update_mark_success 205
#define SYS_update_rollback 206
#define SYS_klog_read    210

/* Graphics syscalls */
#define SYS_graphics_init           300
#define SYS_graphics_get_screen_info 301
#define SYS_graphics_create_window  302
#define SYS_graphics_create_buffer  303
#define SYS_graphics_fill_rect      304
#define SYS_graphics_blit           305
#define SYS_graphics_present        306
#define SYS_graphics_destroy_window 307
#define SYS_graphics_destroy_buffer 308

/* Network extension syscalls */
#define SYS_net_init         350
#define SYS_net_icmp_ping    351
#define SYS_send             352
#define SYS_recv             353
#define SYS_close_socket     354

#ifdef __cplusplus
}
#endif
