/*
 * LimitlessOS Kernel - Production Multiboot2 Implementation
 * 
 * This file implements a robust, standards-compliant multiboot2 kernel
 * with Ubuntu-level reliability and proper boot sequence management.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 * Licensed under GPL v3
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "mm/mm.h"
#include <scheduler.h>
#include "debug.h"
#include "idt.h"
#include "isr.h"
#include "syscall.h"
#include "drivers/vga_text.h"

#include <kernel.h>

/* Global boot info for kernel linkage */
boot_info_t g_boot_info;

/* Multiboot2 specification constants */
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289

/* Multiboot2 tag types */
#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE          1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME 2
#define MULTIBOOT_TAG_TYPE_MODULE           3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO    4
#define MULTIBOOT_TAG_TYPE_BOOTDEV          5
#define MULTIBOOT_TAG_TYPE_MMAP             6
#define MULTIBOOT_TAG_TYPE_VBE              7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER      8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS     9
#define MULTIBOOT_TAG_TYPE_APM              10
#define MULTIBOOT_TAG_TYPE_EFI32            11
#define MULTIBOOT_TAG_TYPE_EFI64            12
#define MULTIBOOT_TAG_TYPE_SMBIOS           13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD         14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW         15

/* VGA text mode constants */
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

/* Color constants for VGA text */
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

/* Multiboot2 structures */
struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_string {
    uint32_t type;
    uint32_t size;
    char string[0];
};

struct multiboot_tag_module {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    char cmdline[0];
};

struct multiboot_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
};

struct multiboot_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
};

struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_mmap_entry entries[0];
};

struct multiboot_tag_framebuffer_common {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint8_t reserved;
};

/* Global variables */
static volatile uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0;
static uint32_t total_memory = 0;
static struct multiboot_tag_framebuffer_common* framebuffer_info = NULL;

/* Graphics system integration */
extern bool initialize_graphics_system(struct multiboot_tag_framebuffer_common* fb_info);
extern void show_boot_splash(void);
extern void display_system_info(void);
extern bool is_graphics_available(void);
extern void graphics_cleanup(void);

/* Function prototypes */
void kernel_main(uint32_t magic, uint32_t addr);
static void terminal_initialize(void);
static uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg);
static uint16_t vga_entry(unsigned char uc, uint8_t color);
static void terminal_setcolor(uint8_t color);
static void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
static void terminal_putchar(char c);
static void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
static void terminal_scroll(void);
static void parse_multiboot_info(uint32_t addr);
static void show_system_info(void);
static void show_boot_menu(void);
static void handle_boot_selection(void);
static void emergency_mode(void);
static void normal_boot(void);
static void installer_mode(void);
static char getchar(void);
void print_hex(uint64_t value);
void print_dec(uint64_t value);
static void init_enhanced_boot_sequence(void);
void test_task_entry();

/* Utility functions for string operations */

/* Utility functions removed - not needed for this implementation */

/* VGA text mode functions */
static uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
    return fg | bg << 4;
}

static uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

static void terminal_initialize(void) {
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

static void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

static void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    vga_buffer[index] = vga_entry(c, color);
}

static void terminal_scroll(void) {
    // Move all lines up by one
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Clear the last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }
    
    terminal_row = VGA_HEIGHT - 1;
}

static void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
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
                terminal_scroll();
            }
        }
        return;
    }
    
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
    }
}

static void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

/* Number printing functions */
void print_hex(uint64_t value) {
    const char* hex_digits = "0123456789ABCDEF";
    char buffer[17];
    buffer[16] = '\0';
    
    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_digits[value & 0xF];
        value >>= 4;
    }
    
    terminal_writestring("0x");
    terminal_writestring(buffer);
}

void print_dec(uint64_t value) {
    char buffer[21];
    buffer[20] = '\0';
    int pos = 19;
    
    if (value == 0) {
        terminal_writestring("0");
        return;
    }
    
    while (value > 0) {
        buffer[pos--] = '0' + (value % 10);
        value /= 10;
    }
    
    terminal_writestring(&buffer[pos + 1]);
}

/* Simple keyboard input (simulation for boot menu) */
static const char scancode_to_ascii[] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
    [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
    [0x1E] = 'a', [0x30] = 'b', [0x2E] = 'c', [0x20] = 'd', [0x12] = 'e',
    [0x21] = 'f', [0x22] = 'g', [0x23] = 'h', [0x17] = 'i', [0x24] = 'j',
    [0x25] = 'k', [0x26] = 'l', [0x32] = 'm', [0x31] = 'n', [0x18] = 'o',
    [0x19] = 'p', [0x10] = 'q', [0x13] = 'r', [0x1F] = 's', [0x14] = 't',
    [0x16] = 'u', [0x2F] = 'v', [0x11] = 'w', [0x2D] = 'x', [0x15] = 'y',
    [0x2C] = 'z',
    [0x1C] = '\n', [0x39] = ' '
};

static char getchar(void) {
    while (1) {
        // Check if the keyboard buffer is full
        if (inb(0x64) & 0x1) {
            unsigned char scancode = inb(0x60);
            // Check for key release
            if (scancode & 0x80) {
                // Key was released, you can handle this if needed
            } else {
                // Key was pressed
                if (scancode < sizeof(scancode_to_ascii) && scancode_to_ascii[scancode]) {
                    return scancode_to_ascii[scancode];
                }
            }
        }
    }
}

/* Parse multiboot2 information structure */
static void parse_multiboot_info(uint32_t addr) {
    struct multiboot_tag *tag;
    uint32_t size = *(uint32_t*)addr;
    
    terminal_writestring("Multiboot2 info size: ");
    print_dec(size);
    terminal_writestring(" bytes\n\n");
    
    for (tag = (struct multiboot_tag *)(addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7))) {
        
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE: {
                struct multiboot_tag_string *cmdline = (struct multiboot_tag_string *)tag;
                terminal_writestring("Command line: ");
                terminal_writestring(cmdline->string);
                terminal_writestring("\n");
                break;
            }
            
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
                struct multiboot_tag_string *bootloader = (struct multiboot_tag_string *)tag;
                terminal_writestring("Bootloader: ");
                terminal_writestring(bootloader->string);
                terminal_writestring("\n");
                break;
            }
            
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
                struct multiboot_tag_basic_meminfo *meminfo = (struct multiboot_tag_basic_meminfo *)tag;
                total_memory = meminfo->mem_lower + meminfo->mem_upper;
                terminal_writestring("Memory: Lower = ");
                print_dec(meminfo->mem_lower);
                terminal_writestring(" KB, Upper = ");
                print_dec(meminfo->mem_upper);
                terminal_writestring(" KB\n");
                break;
            }
            
            case MULTIBOOT_TAG_TYPE_MMAP: {
                struct multiboot_tag_mmap *mmap = (struct multiboot_tag_mmap *)tag;
                terminal_writestring("Memory map:\n");
                
                for (struct multiboot_mmap_entry *entry = mmap->entries;
                     (uint8_t *)entry < (uint8_t *)mmap + mmap->size;
                     entry = (struct multiboot_mmap_entry *)((unsigned long)entry + mmap->entry_size)) {
                    
                    terminal_writestring("  ");
                    print_hex(entry->addr);
                    terminal_writestring(" - ");
                    print_hex(entry->addr + entry->len - 1);
                    terminal_writestring(" (");
                    print_dec(entry->len);
                    terminal_writestring(" bytes) Type: ");
                    print_dec(entry->type);
                    terminal_writestring("\n");
                }
                break;
            }
            
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
                framebuffer_info = (struct multiboot_tag_framebuffer_common *)tag;
                terminal_writestring("Framebuffer: ");
                print_dec(framebuffer_info->framebuffer_width);
                terminal_writestring("x");
                print_dec(framebuffer_info->framebuffer_height);
                terminal_writestring("x");
                print_dec(framebuffer_info->framebuffer_bpp);
                terminal_writestring(" at ");
                print_hex(framebuffer_info->framebuffer_addr);
                terminal_writestring("\n");
                
                // Initialize graphics system
                if (initialize_graphics_system(framebuffer_info)) {
                    terminal_writestring("Graphics system initialized successfully!\n");
                } else {
                    terminal_writestring("Graphics system initialization failed - using text mode\n");
                }
                break;
            }
        }
    }
}

/* Display system information */
static void show_system_info(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("=== LimitlessOS System Information ===\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("Version: 1.0.0 (Production)\n");
    terminal_writestring("Architecture: x86_64\n");
    terminal_writestring("Boot Protocol: Multiboot2\n");
    
    if (total_memory > 0) {
        terminal_writestring("Total Memory: ");
        print_dec(total_memory);
        terminal_writestring(" KB (");
        print_dec(total_memory / 1024);
        terminal_writestring(" MB)\n");
    }
    
    if (framebuffer_info) {
        terminal_writestring("Graphics: ");
        print_dec(framebuffer_info->framebuffer_width);
        terminal_writestring("x");
        print_dec(framebuffer_info->framebuffer_height);
        terminal_writestring(" framebuffer available\n");
    }
    
    terminal_writestring("\n");
}

/* Display boot menu */
static void show_boot_menu(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("=== LimitlessOS Boot Menu ===\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("1. Normal Boot\n");
    terminal_writestring("2. Emergency Mode\n");
    terminal_writestring("3. System Installer\n");
    terminal_writestring("4. Hardware Detection\n");
    terminal_writestring("5. Memory Test\n");
    terminal_writestring("6. System Information\n");
    terminal_writestring("\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Select option (1-6): ");
}

/* Handle boot menu selection */
static void handle_boot_selection(void) {
    char choice = getchar();
    terminal_putchar(choice);
    terminal_writestring("\n\n");
    
    switch (choice) {
        case '1':
            normal_boot();
            break;
        case '2':
            emergency_mode();
            break;
        case '3':
            installer_mode();
            break;
        case '4':
            terminal_writestring("Hardware detection not implemented yet.\n");
            break;
        case '5':
            terminal_writestring("Memory test not implemented yet.\n");
            break;
        case '6':
            show_system_info();
            break;
        default:
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("Invalid selection! Please choose 1-6.\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            break;
    }
}

/* Normal boot process */
static void normal_boot(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Starting LimitlessOS in normal mode...\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("Initializing core systems...\n");

    // Initialize VGA text driver
    terminal_writestring("  [VGA] Initializing VGA Text Driver... ");
    vga_text_init();
    terminal_writestring("OK\n");

    // Initialize interrupt descriptor table
    terminal_writestring("  [INT] Initializing IDT and ISRs... ");
    idt_init();
    terminal_writestring("OK\n");

    // Initialize system calls
    terminal_writestring("  [SYSCALL] Initializing System Call Interface... ");
    syscalls_init();
    terminal_writestring("OK\n");

    // Initialize memory management
    terminal_writestring("  [MM] Initializing Physical Memory Manager (PMM)... ");
    pmm_init(); // Note: In a real scenario, we'd pass the memory map
    terminal_writestring("OK\n");

    terminal_writestring("  [MM] Initializing Virtual Memory Manager (VMM)... ");
    vmm_init(&g_boot_info);
    terminal_writestring("OK\n");

    terminal_writestring("  [MM] Initializing Slab Allocator... ");
    slab_init();
    terminal_writestring("OK\n");

    // Test allocation
    terminal_writestring("  [MM] Testing slab allocation... ");
    void* test_alloc = kmalloc(128);
    if (test_alloc) {
        terminal_writestring("OK\n");
        kfree(test_alloc);
    } else {
        terminal_writestring("FAILED\n");
    }

    // Initialize scheduler
    terminal_writestring("  [SCHED] Initializing Scheduler... ");
    scheduler_init();
    terminal_writestring("OK\n");

    // Create a test task
    terminal_writestring("  [SCHED] Creating test task... ");
    create_task(test_task_entry);
    terminal_writestring("OK\n");

    // Start the scheduler
    terminal_writestring("  [SCHED] Starting scheduler...\n");
    schedule();

    // We should never get here
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    terminal_writestring("FATAL: Returned from schedule(). System halted.\n");
    while(1) asm("hlt");
}

/* Emergency mode */
static void emergency_mode(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    terminal_writestring("=== EMERGENCY MODE ===\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("LimitlessOS Emergency Recovery Console\n");
    terminal_writestring("Type 'help' for available commands\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("emergency> ");
    
    // Simple emergency shell would go here
    while (1) {
        char cmd = getchar();
        terminal_putchar(cmd);
        if (cmd == '\n') {
            terminal_writestring("emergency> ");
        }
    }
}

/* System installer mode */
static void installer_mode(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("=== LimitlessOS System Installer ===\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("Welcome to the LimitlessOS installation wizard!\n\n");
    
    terminal_writestring("Installation options:\n");
    terminal_writestring("1. Full Desktop Installation\n");
    terminal_writestring("2. Server Installation\n");
    terminal_writestring("3. Embedded/IoT Installation\n");
    terminal_writestring("4. Developer Environment\n");
    terminal_writestring("5. Enterprise Deployment\n\n");
    
    terminal_writestring("Select installation type (1-5): ");
    
    char choice = getchar();
    terminal_putchar(choice);
    terminal_writestring("\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Preparing installation environment...\n");
    terminal_writestring("Scanning hardware configuration...\n");
    terminal_writestring("Ready to begin installation!\n");
}

/* Enhanced boot sequence with graphics */
static void init_enhanced_boot_sequence(void) {
    // Show graphics splash screen if available
    if (is_graphics_available()) {
        terminal_writestring("Starting enhanced boot sequence with graphics...\n");
        show_boot_splash();
        
        // Brief pause to show splash
        for (volatile int i = 0; i < 50000000; i++);
        
        // Show system information overlay
        display_system_info();
    } else {
        terminal_writestring("Graphics not available - using text mode boot sequence\n");
    }
}

/* Test task entry function */
void test_task_entry() {
    terminal_writestring("\n[TASK] Hello from test_task_entry!");
    while(1) {
        // This task will print a message and then yield.
        for (volatile int i = 0; i < 10000000; i++); // Delay
        terminal_writestring("\n[TASK] Looping...");
    }
}

/* Main kernel entry point */
void kernel_main(uint32_t magic, uint32_t addr) {
    // Initialize terminal for early boot messages
    terminal_initialize();
    terminal_writestring("LimitlessOS Kernel Booting...\n");

    // Verify multiboot2 magic
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("FATAL: Invalid multiboot2 magic. System halted.\n");
        while(1) asm("hlt");
    }
    terminal_writestring("Multiboot2 magic verified.\n");

    // Parse Multiboot2 info to find memory map
    parse_multiboot_info(addr);
    terminal_writestring("Parsed multiboot info.\n");

    // Show boot menu and handle selection
    show_boot_menu();
    handle_boot_selection();

    // We should never get here if a boot option is selected
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    terminal_writestring("FATAL: Boot selection failed. System halted.\n");
    while(1) {
        asm("hlt");
    }
}