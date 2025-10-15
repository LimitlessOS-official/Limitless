#ifndef VGA_TEXT_H
#define VGA_TEXT_H

#include <common.h>

// Initializes the VGA text mode driver.
void vga_text_init(void);

// Clears the screen.
void vga_text_clear(void);

// Puts a single character on the screen.
void vga_text_putchar(char c);

// Writes a null-terminated string to the screen.
void vga_text_writestring(const char *data);

#endif // VGA_TEXT_H
