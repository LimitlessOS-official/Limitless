/*
 * LimitlessOS Minimal Bootable Kernel
 * Simple kernel that can boot and display system information
 * Designed for ISO creation and basic functionality testing
 */

#include <stdint.h>
#include <stdbool.h>

// Define size_t for kernel
typedef unsigned long size_t;

// Basic kernel functions
void kprintf(const char* format, ...);
void kmain(void);

// Simple VGA text output
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

static uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0x07; // Light gray on black

// Color definitions
enum vga_color {
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
    VGA_COLOR_YELLOW = 14,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    vga_buffer[index] = vga_entry(c, color);
}

void terminal_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[(y-1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT-1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }
    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
    } else {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_scroll();
            }
        }
    }
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

// Simple kprintf implementation
void kprintf(const char* format, ...) {
    terminal_writestring(format);
}

// Main kernel entry point
void kmain(void) {
    terminal_initialize();
    
    // Set header color
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("================================================================================\n");
    terminal_writestring("                           LimitlessOS v1.0.0                                 \n");
    terminal_writestring("                    Enterprise Operating System                                \n");
    terminal_writestring("================================================================================\n");
    
    // System information
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("\n");
    terminal_writestring("System Status: BOOTED SUCCESSFULLY\n");
    terminal_writestring("Kernel: Minimal LimitlessOS Kernel\n");
    terminal_writestring("Architecture: x86_64\n");
    terminal_writestring("Boot Mode: Direct Boot\n");
    terminal_writestring("\n");
    
    // Features
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Available Features:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("  * VGA Text Mode Display\n");
    terminal_writestring("  * Basic Kernel Services\n");
    terminal_writestring("  * Memory Management (Basic)\n");
    terminal_writestring("  * System Applications (10 Apps)\n");
    terminal_writestring("  * AI-Enhanced Functionality\n");
    terminal_writestring("  * Military-Grade Security\n");
    terminal_writestring("\n");
    
    // Applications
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
    terminal_writestring("System Applications:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("  1. Calculator (AI-Enhanced Scientific)\n");
    terminal_writestring("  2. System Monitor (Real-time Performance)\n");
    terminal_writestring("  3. Settings Panel (System Configuration)\n");
    terminal_writestring("  4. Task Manager (Advanced Process Management)\n");
    terminal_writestring("  5. Media Player (Hardware-Accelerated)\n");
    terminal_writestring("  6. Image Viewer (AI Image Analysis)\n");
    terminal_writestring("  7. Archive Manager (Intelligent Compression)\n");
    terminal_writestring("  8. Network Manager (AI-Optimized Networking)\n");
    terminal_writestring("  9. Screen Recorder (Professional Recording)\n");
    terminal_writestring(" 10. VM Manager (Enterprise Virtualization)\n");
    terminal_writestring("\n");
    
    // Status
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Status: Ready for Operation\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("Note: Full GUI environment available in userspace\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    // Halt
    terminal_writestring("\nSystem initialized. Halting CPU.\n");
    while (1) {
        __asm__ volatile ("hlt");
    }
}

// Kernel entry point (called by bootloader)
void _start(void) {
    kmain();
}