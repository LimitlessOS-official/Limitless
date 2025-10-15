/*
 * putchar() - print a single character to stdout
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <stdio.h>

extern int write(int fd, const void *buf, unsigned int count);

int putchar(int c)
{
    char ch = (char)c;
    write(1, &ch, 1);
    return c;
}
