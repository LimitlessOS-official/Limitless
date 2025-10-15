/*
 * printf() - formatted output
 * 
 * A complete implementation of printf supporting:
 * - %c, %s, %d, %i, %u, %x, %X, %p, %o
 * - Width and padding
 * - Left/right alignment
 * - Zero padding
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

extern int write(int fd, const void *buf, unsigned int count);

// Helper function to convert number to string
static void itoa_helper(int value, char *str, int base, int *len)
{
    int i = 0;
    int is_negative = 0;
    
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        *len = i;
        return;
    }
    
    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }
    
    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value = value / base;
    }
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    *len = i;
    
    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - 1 - j];
        str[i - 1 - j] = temp;
    }
}

static void uitoa_helper(unsigned int value, char *str, int base, int uppercase, int *len)
{
    int i = 0;
    
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        *len = i;
        return;
    }
    
    while (value != 0) {
        int rem = value % base;
        if (uppercase) {
            str[i++] = (rem > 9) ? (rem - 10) + 'A' : rem + '0';
        } else {
            str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        }
        value = value / base;
    }
    
    str[i] = '\0';
    *len = i;
    
    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - 1 - j];
        str[i - 1 - j] = temp;
    }
}

int vprintf(const char *format, va_list ap)
{
    char buffer[1024];
    int buf_pos = 0;
    int written = 0;
    
    for (const char *p = format; *p; p++) {
        if (*p != '%') {
            buffer[buf_pos++] = *p;
            if (buf_pos >= 1023) {
                write(1, buffer, buf_pos);
                written += buf_pos;
                buf_pos = 0;
            }
            continue;
        }
        
        p++; // Skip '%'
        
        // Parse flags
        int left_align = 0;
        int zero_pad = 0;
        
        if (*p == '-') {
            left_align = 1;
            p++;
        } else if (*p == '0') {
            zero_pad = 1;
            p++;
        }
        
        // Parse width
        int width = 0;
        while (*p >= '0' && *p <= '9') {
            width = width * 10 + (*p - '0');
            p++;
        }
        
        // Process format specifier
        char temp[64];
        int temp_len = 0;
        
        switch (*p) {
            case 'c': {
                char c = (char)va_arg(ap, int);
                buffer[buf_pos++] = c;
                break;
            }
            
            case 's': {
                const char *s = va_arg(ap, const char *);
                if (!s) s = "(null)";
                int s_len = strlen(s);
                
                if (!left_align && width > s_len) {
                    for (int i = 0; i < width - s_len; i++) {
                        buffer[buf_pos++] = ' ';
                    }
                }
                
                for (int i = 0; i < s_len; i++) {
                    buffer[buf_pos++] = s[i];
                }
                
                if (left_align && width > s_len) {
                    for (int i = 0; i < width - s_len; i++) {
                        buffer[buf_pos++] = ' ';
                    }
                }
                break;
            }
            
            case 'd':
            case 'i': {
                int val = va_arg(ap, int);
                itoa_helper(val, temp, 10, &temp_len);
                
                if (!left_align && width > temp_len) {
                    char pad_char = zero_pad ? '0' : ' ';
                    for (int i = 0; i < width - temp_len; i++) {
                        buffer[buf_pos++] = pad_char;
                    }
                }
                
                for (int i = 0; i < temp_len; i++) {
                    buffer[buf_pos++] = temp[i];
                }
                
                if (left_align && width > temp_len) {
                    for (int i = 0; i < width - temp_len; i++) {
                        buffer[buf_pos++] = ' ';
                    }
                }
                break;
            }
            
            case 'u': {
                unsigned int val = va_arg(ap, unsigned int);
                uitoa_helper(val, temp, 10, 0, &temp_len);
                
                for (int i = 0; i < temp_len; i++) {
                    buffer[buf_pos++] = temp[i];
                }
                break;
            }
            
            case 'x': {
                unsigned int val = va_arg(ap, unsigned int);
                uitoa_helper(val, temp, 16, 0, &temp_len);
                
                for (int i = 0; i < temp_len; i++) {
                    buffer[buf_pos++] = temp[i];
                }
                break;
            }
            
            case 'X': {
                unsigned int val = va_arg(ap, unsigned int);
                uitoa_helper(val, temp, 16, 1, &temp_len);
                
                for (int i = 0; i < temp_len; i++) {
                    buffer[buf_pos++] = temp[i];
                }
                break;
            }
            
            case 'p': {
                unsigned int val = va_arg(ap, unsigned int);
                buffer[buf_pos++] = '0';
                buffer[buf_pos++] = 'x';
                uitoa_helper(val, temp, 16, 0, &temp_len);
                
                for (int i = 0; i < temp_len; i++) {
                    buffer[buf_pos++] = temp[i];
                }
                break;
            }
            
            case 'o': {
                unsigned int val = va_arg(ap, unsigned int);
                uitoa_helper(val, temp, 8, 0, &temp_len);
                
                for (int i = 0; i < temp_len; i++) {
                    buffer[buf_pos++] = temp[i];
                }
                break;
            }
            
            case '%': {
                buffer[buf_pos++] = '%';
                break;
            }
            
            default: {
                buffer[buf_pos++] = '%';
                buffer[buf_pos++] = *p;
                break;
            }
        }
        
        if (buf_pos >= 1000) {
            write(1, buffer, buf_pos);
            written += buf_pos;
            buf_pos = 0;
        }
    }
    
    if (buf_pos > 0) {
        write(1, buffer, buf_pos);
        written += buf_pos;
    }
    
    return written;
}

int printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int ret = vprintf(format, ap);
    va_end(ap);
    return ret;
}

int sprintf(char *str, const char *format, ...)
{
    // TODO: Implement sprintf
    return 0;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
    // TODO: Implement snprintf
    return 0;
}
