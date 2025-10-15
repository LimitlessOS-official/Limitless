/*
 * System Call Wrapper - Userspace
 * 
 * This file provides the low-level system call interface for userspace
 * applications. It uses inline assembly to trigger interrupt 0x80.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <syscall.h>

/**
 * Perform a system call with 0 arguments
 */
int syscall0(int num)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(num));
    return ret;
}

/**
 * Perform a system call with 1 argument
 */
int syscall1(int num, int arg1)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1));
    return ret;
}

/**
 * Perform a system call with 2 arguments
 */
int syscall2(int num, int arg1, int arg2)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1), "c"(arg2));
    return ret;
}

/**
 * Perform a system call with 3 arguments
 */
int syscall3(int num, int arg1, int arg2, int arg3)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3));
    return ret;
}

/**
 * Perform a system call with 4 arguments
 */
int syscall4(int num, int arg1, int arg2, int arg3, int arg4)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4));
    return ret;
}

/**
 * Perform a system call with 5 arguments
 */
int syscall5(int num, int arg1, int arg2, int arg3, int arg4, int arg5)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5));
    return ret;
}
