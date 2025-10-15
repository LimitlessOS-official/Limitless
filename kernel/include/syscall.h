#ifndef _KERNEL_SYSCALL_H
#define _KERNEL_SYSCALL_H

#include <stdint.h>
#include "isr.h"

// System call numbers
#define SYS_WRITE    1
#define SYS_READ     2
#define SYS_OPEN     3
#define SYS_CLOSE    4
#define SYS_SBRK     5
#define SYS_EXIT     6
#define SYS_FORK     7
#define SYS_EXEC     8
#define SYS_WAIT     9
#define SYS_GETPID   10
#define SYS_GETPPID  11
#define SYS_KILL     12
#define SYS_SIGNAL   13
#define SYS_ALARM    14
#define SYS_PAUSE    15
#define SYS_PIPE     16
#define SYS_SHMGET   17
#define SYS_SHMAT    18
#define SYS_SHMDT    19
#define SYS_SEMGET   20
#define SYS_SEMOP    21
#define SYS_MSGGET   22
#define SYS_MSGSND   23
#define SYS_MSGRCV   24
#define SYS_WAITPID  25
#define SYS_EXECVE   26
#define SYS_SIGACTION 27
#define SYS_SIGPROCMASK 28
#define SYS_SIGPENDING 29
#define SYS_MMAP     30
#define SYS_MUNMAP   31
#define SYS_MSYNC    32
#define SYS_LSEEK    33
#define SYS_FSTAT    34
#define SYS_DUP      35
#define SYS_DUP2     36

#define SYSCALL_MAX  37

// Initialize the syscall subsystem
void syscalls_init(void);

// System call handler (called from ISR)
void syscall_handler(registers_t *regs);

#endif // _KERNEL_SYSCALL_H