#include <string.h>

void* memset(void* dst, int c, size_t n){ unsigned char* p=(unsigned char*)dst; while(n--) *p++=(unsigned char)c; return dst; }
void* memcpy(void* dst, const void* src, size_t n){ unsigned char* d=(unsigned char*)dst; const unsigned char* s=(const unsigned char*)src; while(n--) *d++=*s++; return dst; }
size_t strlen(const char* s){ size_t n=0; while(s && *s++) ++n; return n; }
int strcmp(const char* a, const char* b){ while(*a && *b && *a==*b){ a++; b++; } return (unsigned char)*a - (unsigned char)*b; }
