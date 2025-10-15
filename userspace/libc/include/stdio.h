#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

// File descriptor constants
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// Standard I/O functions
int putchar(int c);
int puts(const char *s);
int printf(const char *format, ...);
int vprintf(const char *format, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int snprintf(char* buf, size_t sz, const char* fmt, ...);
int vsnprintf(char* buf, size_t sz, const char* fmt, va_list ap);

// File I/O (basic stubs for now)
int write(int fd, const void *buf, unsigned int count);

#endif // _STDIO_H
