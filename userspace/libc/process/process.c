#include <syscall.h>
#include <stddef.h>

// Process management
int getpid(void) {
    return syscall0(SYS_GETPID);
}

int getppid(void) {
    return syscall0(SYS_GETPPID);
}

int fork(void) {
    return syscall0(SYS_FORK);
}

int execve(const char *path, char *const argv[], char *const envp[]) {
    return syscall3(SYS_EXECVE, (int)path, (int)argv, (int)envp);
}

void exit(int status) {
    syscall1(SYS_EXIT, status);
    while (1);  // Should never reach here
}

int waitpid(int pid, int *status, int options) {
    return syscall3(SYS_WAITPID, pid, (int)status, options);
}

int wait(int *status) {
    return waitpid(-1, status, 0);
}
