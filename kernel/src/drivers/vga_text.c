/*
 * VGA Text Mode Driver
 * 
 * This module provides a clean interface for VGA text mode output.
 * It handles screen scrolling, cursor positioning, and color management.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <drivers/vga_text.h>
#include <common.h>

// VGA text mode constants
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// Color constants
typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
} vga_color_t;

// Global state
static volatile uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0;

// Helper functions
static inline uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

/**
 * Scroll the screen up by one line
 */
static void vga_text_scroll(void) {
    // Move all lines up by one
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Clear the last line
    uint16_t blank = vga_entry(' ', terminal_color);
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank;
    }
    
    terminal_row = VGA_HEIGHT - 1;
}

/**
 * Initialize the VGA text mode driver
 */
void vga_text_init(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    vga_text_clear();
}

/**
 * Clear the entire screen
 */
void vga_text_clear(void) {
    uint16_t blank = vga_entry(' ', terminal_color);
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = blank;
        }
    }
    
    terminal_row = 0;
    terminal_column = 0;
}

/**
 * Put a single character on the screen
 * 
 * @param c The character to print
 */
void vga_text_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            vga_text_scroll();
        }
        return;
    }
    
    if (c == '\r') {
        terminal_column = 0;
        return;
    }
    
    if (c == '\t') {
        terminal_column = (terminal_column + 8) & ~(8 - 1);
        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                vga_text_scroll();
            }
        }
        return;
    }
    
    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    vga_buffer[index] = vga_entry(c, terminal_color);
    
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            vga_text_scroll();
        }
    }
}

/**
 * Write a null-terminated string to the screen
 * 
 * @param data The string to print
 */
void vga_text_writestring(const char* data) {
    size_t datalen = strlen(data);
    for (size_t i = 0; i < datalen; i++)
        vga_text_putchar(data[i]);
}
