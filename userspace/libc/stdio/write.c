/*
 * write() system call wrapper
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <syscall.h>

extern int syscall3(int num, int arg1, int arg2, int arg3);

int write(int fd, const void *buf, unsigned int count)
{
    return syscall3(SYS_WRITE, fd, (int)buf, count);
}
