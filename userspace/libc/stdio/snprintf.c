#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// Helper function to convert number to string
static int num_to_str(char *buf, int bufsize, int num, int base, int width, int flags) {
    char tmp[32];
    int i = 0;
    int negative = 0;
    unsigned int n;
    
    if (num < 0 && base == 10) {
        negative = 1;
        n = -num;
    } else {
        n = num;
    }
    
    if (n == 0) {
        tmp[i++] = '0';
    } else {
        while (n > 0) {
            int digit = n % base;
            tmp[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            n /= base;
        }
    }
    
    if (negative) {
        tmp[i++] = '-';
    }
    
    // Reverse
    int len = i;
    for (int j = 0; j < len && j < bufsize - 1; j++) {
        buf[j] = tmp[len - 1 - j];
    }
    buf[len < bufsize ? len : bufsize - 1] = '\0';
    
    return len;
}

// snprintf implementation
int snprintf(char *buf, size_t sz, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = vsnprintf(buf, sz, fmt, args);
    va_end(args);
    return result;
}

// vsnprintf implementation
int vsnprintf(char *buf, size_t sz, const char *fmt, va_list args) {
    size_t pos = 0;
    
    while (*fmt && pos < sz - 1) {
        if (*fmt == '%') {
            fmt++;
            
            // Handle width
            int width = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }
            
            // Handle format specifier
            switch (*fmt) {
                case 'c': {
                    char c = (char)va_arg(args, int);
                    if (pos < sz - 1) {
                        buf[pos++] = c;
                    }
                    break;
                }
                case 's': {
                    const char *s = va_arg(args, const char *);
                    if (!s) s = "(null)";
                    while (*s && pos < sz - 1) {
                        buf[pos++] = *s++;
                    }
                    break;
                }
                case 'd':
                case 'i': {
                    int num = va_arg(args, int);
                    char tmp[32];
                    int len = num_to_str(tmp, sizeof(tmp), num, 10, width, 0);
                    for (int i = 0; i < len && pos < sz - 1; i++) {
                        buf[pos++] = tmp[i];
                    }
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    char tmp[32];
                    int len = num_to_str(tmp, sizeof(tmp), (int)num, 10, width, 0);
                    for (int i = 0; i < len && pos < sz - 1; i++) {
                        buf[pos++] = tmp[i];
                    }
                    break;
                }
                case 'x':
                case 'X': {
                    unsigned int num = va_arg(args, unsigned int);
                    char tmp[32];
                    int len = num_to_str(tmp, sizeof(tmp), (int)num, 16, width, 0);
                    for (int i = 0; i < len && pos < sz - 1; i++) {
                        buf[pos++] = tmp[i];
                    }
                    break;
                }
                case 'p': {
                    void *ptr = va_arg(args, void *);
                    if (pos < sz - 2) {
                        buf[pos++] = '0';
                        buf[pos++] = 'x';
                    }
                    char tmp[32];
                    int len = num_to_str(tmp, sizeof(tmp), (int)ptr, 16, width, 0);
                    for (int i = 0; i < len && pos < sz - 1; i++) {
                        buf[pos++] = tmp[i];
                    }
                    break;
                }
                case '%': {
                    if (pos < sz - 1) {
                        buf[pos++] = '%';
                    }
                    break;
                }
            }
            fmt++;
        } else {
            buf[pos++] = *fmt++;
        }
    }
    
    if (pos < sz) {
        buf[pos] = '\0';
    } else if (sz > 0) {
        buf[sz - 1] = '\0';
    }
    
    return pos;
}

// sprintf implementation
int sprintf(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = vsnprintf(buf, 4096, fmt, args); // Assume max 4KB
    va_end(args);
    return result;
}
