/* syscall.h - minimal userland syscall veneer (temporary stubs) */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "uapi/syscalls.h"

/* Minimal structs expected by some utilities */
typedef struct { uint64_t size; uint32_t mode; uint32_t reserved; } stat_t;
typedef struct { uint32_t version; uint32_t flags; } update_meta_t;

/* Architecture-specific syscall implementation */
#ifdef __x86_64__
static inline long __syscall6(uint64_t n, uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
    register uint64_t rax asm("rax") = n;
    register uint64_t rdi asm("rdi") = a0;
    register uint64_t rsi asm("rsi") = a1;
    register uint64_t rdx asm("rdx") = a2;
    register uint64_t r10 asm("r10") = a3;
    register uint64_t r8 asm("r8") = a4;
    register uint64_t r9 asm("r9") = a5;
    
    asm volatile(
        "syscall"
        : "+r" (rax)
        : "r" (rdi), "r" (rsi), "r" (rdx), "r" (r10), "r" (r8), "r" (r9)
        : "rcx", "r11", "memory"
    );
    
    return (long)rax;
}
#else
/* Fallback for other architectures */
extern long syscall_entry(uint64_t n, uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5);
static inline long __syscall6(uint64_t n, uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
    return syscall_entry(n, a0, a1, a2, a3, a4, a5);
}
#endif

/* System call wrappers that bridge to kernel */
static inline long sys_open(const char* pathname, int flags, int mode) {
    return __syscall6(SYS_open, (uint64_t)pathname, (uint64_t)flags, (uint64_t)mode, 0, 0, 0);
}

static inline long sys_close(int fd) {
    return __syscall6(SYS_close, (uint64_t)fd, 0, 0, 0, 0, 0);
}

static inline long sys_read(int fd, void* buf, size_t count) {
    return __syscall6(SYS_read, (uint64_t)fd, (uint64_t)buf, (uint64_t)count, 0, 0, 0);
}

static inline long sys_write(int fd, const void* buf, size_t len) {
    return __syscall6(SYS_write, (uint64_t)fd, (uint64_t)buf, (uint64_t)len, 0, 0, 0);
}

static inline long sys_pwrite(int fd, const void* buf, size_t len, uint64_t offset) {
    return __syscall6(SYS_pwrite64, (uint64_t)fd, (uint64_t)buf, (uint64_t)len, offset, 0, 0);
}

static inline long sys_stat(const char* pathname, void* statbuf) {
    return __syscall6(SYS_stat, (uint64_t)pathname, (uint64_t)statbuf, 0, 0, 0, 0);
}

static inline long sys_mkdir(const char* pathname, int mode) {
    return __syscall6(SYS_mkdir, (uint64_t)pathname, (uint64_t)mode, 0, 0, 0, 0);
}

static inline long sys_sleep_ms(uint64_t ms) {
    return __syscall6(SYS_sleep_ms, ms, 0, 0, 0, 0, 0);
}

static inline long sys_getpid(void) {
    return __syscall6(SYS_getpid, 0, 0, 0, 0, 0, 0);
}

static inline long sys_fork(void) {
    return __syscall6(SYS_fork, 0, 0, 0, 0, 0, 0);
}

static inline long sys_execve(const char* filename, const char* const argv[], const char* const envp[]) {
    return __syscall6(SYS_execve, (uint64_t)filename, (uint64_t)argv, (uint64_t)envp, 0, 0, 0);
}

static inline long sys_exit(int status) {
    return __syscall6(SYS_exit, (uint64_t)status, 0, 0, 0, 0, 0);
}

static inline long sys_wait4(int pid, int* status, int options, void* rusage) {
    return __syscall6(SYS_waitpid, (uint64_t)pid, (uint64_t)status, (uint64_t)options, (uint64_t)rusage, 0, 0);
}

static inline void* sys_mmap(void* addr, size_t length, int prot, int flags, int fd, uint64_t offset) {
    return (void*)__syscall6(SYS_mmap, (uint64_t)addr, (uint64_t)length, (uint64_t)prot, (uint64_t)flags, (uint64_t)fd, offset);
}

static inline long sys_munmap(void* addr, size_t length) {
    return __syscall6(SYS_munmap, (uint64_t)addr, (uint64_t)length, 0, 0, 0, 0);
}

/* Network syscalls */
static inline long sys_socket(int domain, int type, int protocol) {
    return __syscall6(SYS_socket, (uint64_t)domain, (uint64_t)type, (uint64_t)protocol, 0, 0, 0);
}

static inline long sys_bind(int sockfd, const void* addr, size_t addrlen) {
    return __syscall6(SYS_bind, (uint64_t)sockfd, (uint64_t)addr, (uint64_t)addrlen, 0, 0, 0);
}

static inline long sys_sendto(int sockfd, const void* buf, size_t len, int flags, const void* dest_addr, size_t addrlen) {
    return __syscall6(SYS_sendto, (uint64_t)sockfd, (uint64_t)buf, (uint64_t)len, (uint64_t)flags, (uint64_t)dest_addr, (uint64_t)addrlen);
}

/* System administration syscalls */
static inline long sys_klog_read(char* buf, size_t len) {
    return __syscall6(SYS_klog_read, (uint64_t)buf, (uint64_t)len, 0, 0, 0, 0);
}

static inline long sys_auth_login(const char* username, const char* password) {
    return __syscall6(SYS_auth_login, (uint64_t)username, (uint64_t)password, 0, 0, 0, 0);
}

static inline long sys_update_check(void* metadata) {
    return __syscall6(SYS_update_get_meta, (uint64_t)metadata, 0, 0, 0, 0, 0);
}

static inline long sys_update_apply(void* metadata) {
    return __syscall6(SYS_update_apply, (uint64_t)metadata, 0, 0, 0, 0, 0);
}

static inline long sys_unlink(const char* pathname) {
    return __syscall6(SYS_unlink, (uint64_t)pathname, 0, 0, 0, 0, 0);
}

static inline long sys_kill(int pid, int sig) {
    return __syscall6(SYS_kill, (uint64_t)pid, (uint64_t)sig, 0, 0, 0, 0);
}