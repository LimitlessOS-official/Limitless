#pragma once
#include <stddef.h>

void* memset(void* dst, int c, size_t n);
void* memcpy(void* dst, const void* src, size_t n);
size_t strlen(const char* s);
int strcmp(const char* a, const char* b);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
