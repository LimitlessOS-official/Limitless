#ifndef _SYSCALL_H
#define _SYSCALL_H

// System call numbers (must match kernel/src/syscall.c)
#define SYS_WRITE  1
#define SYS_READ   2
#define SYS_OPEN   3
#define SYS_CLOSE  4
#define SYS_SBRK   5
#define SYS_EXIT   6
#define SYS_FORK   7
#define SYS_EXEC   8
#define SYS_WAIT   9

#define SYS_GETPPID 11
#define SYS_KILL 12
#define SYS_SIGNAL 13
#define SYS_SIGPROCMASK 14
#define SYS_SIGPENDING 15
#define SYS_WAITPID 16
#define SYS_EXECVE 17
#define SYS_ALARM 18

// System call wrapper functions
int syscall0(int num);
int syscall1(int num, int arg1);
int syscall2(int num, int arg1, int arg2);
int syscall3(int num, int arg1, int arg2, int arg3);
int syscall4(int num, int arg1, int arg2, int arg3, int arg4);
int syscall5(int num, int arg1, int arg2, int arg3, int arg4, int arg5);

// Convenience wrappers
int write(int fd, const void *buf, unsigned int count);

#endif // _SYSCALL_H
