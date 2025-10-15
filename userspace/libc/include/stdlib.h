#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

// Program termination
void exit(int status);
void abort(void);

// Memory allocation
void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

// String to number conversions
int atoi(const char *str);
long atol(const char *str);
long long atoll(const char *str);

// Absolute value functions
int abs(int n);
long labs(long n);
long long llabs(long long n);

// Common macros
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#endif // _STDLIB_H
