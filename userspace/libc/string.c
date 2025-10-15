#include <stddef.h>

void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n-- > 0) {
        *p++ = (unsigned char)c;
    }
    return s;
}
