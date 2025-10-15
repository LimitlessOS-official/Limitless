#include <sys/mman.h>
#include <syscall.h>

// Syscall function declarations
extern int syscall6(int num, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
extern int syscall2(int num, int arg1, int arg2);
extern int syscall3(int num, int arg1, int arg2, int arg3);

// mmap - map files or devices into memory
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return (void *)syscall6(SYS_MMAP, (int)addr, (int)length, prot, flags, fd, (int)offset);
}

// munmap - unmap files or devices from memory
int munmap(void *addr, size_t length) {
    return syscall2(SYS_MUNMAP, (int)addr, (int)length);
}

// msync - synchronize a file with a memory map
int msync(void *addr, size_t length, int flags) {
    return syscall3(SYS_MSYNC, (int)addr, (int)length, flags);
}
