/*
 * puts() - print a string to stdout
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <stdio.h>
#include <string.h>

extern int write(int fd, const void *buf, unsigned int count);

int puts(const char *s)
{
    if (!s) return -1;
    
    size_t len = strlen(s);
    write(1, s, len);
    write(1, "\n", 1);
    
    return 0;
}
