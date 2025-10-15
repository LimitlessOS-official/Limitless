#ifndef _STRINGS_H
#define _STRINGS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int bcmp(const void *s1, const void *s2, size_t n);
void bcopy(const void *src, void *dest, size_t n);
void bzero(void *s, size_t n);

#ifdef __cplusplus
}
#endif

#endif // _STRINGS_H
