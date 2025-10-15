#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <syscall.h>

// Syscall function declarations
extern int syscall3(int num, int arg1, int arg2, int arg3);
extern int syscall2(int num, int arg1, int arg2);
extern int syscall1(int num, int arg1);

// read - read from file descriptor
ssize_t read(int fd, void *buf, size_t count) {
    return syscall3(SYS_READ, fd, (int)buf, (int)count);
}

// write - write to file descriptor
ssize_t write(int fd, const void *buf, size_t count) {
    return syscall3(SYS_WRITE, fd, (int)buf, (int)count);
}

// open - open file
int open(const char *pathname, int flags, ...) {
    // TODO: Handle variable args for mode
    return syscall3(SYS_OPEN, (int)pathname, flags, 0644);
}

// close - close file descriptor
int close(int fd) {
    return syscall1(SYS_CLOSE, fd);
}

// lseek - reposition file offset
off_t lseek(int fd, off_t offset, int whence) {
    return syscall3(SYS_LSEEK, fd, (int)offset, whence);
}

// fstat - get file status
int fstat(int fd, struct stat *buf) {
    return syscall2(SYS_FSTAT, fd, (int)buf);
}

// dup - duplicate file descriptor
int dup(int oldfd) {
    return syscall1(SYS_DUP, oldfd);
}

// dup2 - duplicate to specific fd
int dup2(int oldfd, int newfd) {
    return syscall2(SYS_DUP2, oldfd, newfd);
}
