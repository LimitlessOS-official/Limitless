/*
 * Kernel Printf - Simple formatted printing for kernel
 * 
 * Basic implementation of kprintf for kernel logging
 * Uses VGA text mode for output
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "kernel.h"
#include <stdarg.h>

// External VGA functions
extern void vga_text_putchar(char c);
extern void vga_text_writestring(const char* str);

// Simple number to string conversion
static void print_number(u64 num, int base) {
    if (num == 0) {
        vga_text_putchar('0');
        return;
    }
    
    char buf[32];
    int i = 0;
    
    while (num > 0 && i < 31) {
        int digit = num % base;
        buf[i++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
        num /= base;
    }
    
    // Print in reverse
    for (int j = i - 1; j >= 0; j--) {
        vga_text_putchar(buf[j]);
    }
}

// Minimal kprintf implementation
void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    for (const char* p = fmt; *p; p++) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 's': {
                    const char* str = va_arg(args, const char*);
                    if (str) {
                        vga_text_writestring(str);
                    } else {
                        vga_text_writestring("(null)");
                    }
                    break;
                }
                case 'd':
                case 'u': {
                    u32 num = va_arg(args, u32);
                    print_number(num, 10);
                    break;
                }
                case 'x': {
                    u32 num = va_arg(args, u32);
                    vga_text_writestring("0x");
                    print_number(num, 16);
                    break;
                }
                case 'p': {
                    void* ptr = va_arg(args, void*);
                    vga_text_writestring("0x");
                    print_number((u64)ptr, 16);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    vga_text_putchar(c);
                    break;
                }
                case '%':
                    vga_text_putchar('%');
                    break;
                default:
                    vga_text_putchar('%');
                    vga_text_putchar(*p);
                    break;
            }
        } else {
            vga_text_putchar(*p);
        }
    }
    
    va_end(args);
}
