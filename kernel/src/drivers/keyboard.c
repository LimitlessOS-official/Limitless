/*
 * PS/2 Keyboard Driver
 * 
 * Implements PS/2 keyboard support:
 * - Scancode translation
 * - Key event buffering
 * - Character device interface
 * - Interrupt-driven I/O
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "kernel.h"
#include "device.h"
#include <string.h>

// Keyboard I/O ports
#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64
#define KBD_CMD_PORT    0x64

// Status register bits
#define KBD_STATUS_OUT_FULL  0x01
#define KBD_STATUS_IN_FULL   0x02

// Key buffer
#define KEY_BUFFER_SIZE 256

typedef struct {
    u8 buffer[KEY_BUFFER_SIZE];
    u32 read_pos;
    u32 write_pos;
    u32 count;
} key_buffer_t;

static key_buffer_t key_buffer = {0};

// Simple scancode to ASCII table (US keyboard layout)
static const char scancode_to_ascii[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

// Shifted characters
static const char scancode_to_ascii_shift[] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

static int shift_pressed = 0;

// Add key to buffer
static void kbd_buffer_add(u8 key) {
    if (key_buffer.count < KEY_BUFFER_SIZE) {
        key_buffer.buffer[key_buffer.write_pos] = key;
        key_buffer.write_pos = (key_buffer.write_pos + 1) % KEY_BUFFER_SIZE;
        key_buffer.count++;
    }
}

// Read key from buffer
static int kbd_buffer_read(u8* key) {
    if (key_buffer.count == 0) {
        return 0; // Buffer empty
    }
    
    *key = key_buffer.buffer[key_buffer.read_pos];
    key_buffer.read_pos = (key_buffer.read_pos + 1) % KEY_BUFFER_SIZE;
    key_buffer.count--;
    return 1;
}

// Keyboard interrupt handler
void keyboard_irq_handler(void) {
    u8 scancode = inb(KBD_DATA_PORT);
    
    // Handle special keys
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }
    
    // Ignore key release events (scancode & 0x80)
    if (scancode & 0x80) {
        return;
    }
    
    // Translate scancode to ASCII
    char ascii = 0;
    if (scancode < sizeof(scancode_to_ascii)) {
        if (shift_pressed) {
            ascii = scancode_to_ascii_shift[scancode];
        } else {
            ascii = scancode_to_ascii[scancode];
        }
    }
    
    if (ascii) {
        kbd_buffer_add(ascii);
    }
}

// Device operations
static int kbd_open(device_t* dev) {
    dev->flags |= DEV_FLAG_READY;
    return 0;
}

static int kbd_close(device_t* dev) {
    dev->flags &= ~DEV_FLAG_READY;
    return 0;
}

static long kbd_read(device_t* dev, u64 offset, void* buf, size_t len) {
    (void)dev;
    (void)offset;
    
    u8* buffer = (u8*)buf;
    size_t i;
    
    for (i = 0; i < len; i++) {
        if (!kbd_buffer_read(&buffer[i])) {
            break; // No more keys available
        }
    }
    
    return i;
}

static long kbd_write(device_t* dev, u64 offset, const void* buf, size_t len) {
    (void)dev; (void)offset; (void)buf; (void)len;
    return -1; // Keyboard is read-only
}

static int kbd_poll(device_t* dev, u32 events) {
    (void)dev; (void)events;
    return key_buffer.count > 0 ? 1 : 0;
}

// Device operations table
static device_ops_t kbd_ops = {
    .open = kbd_open,
    .close = kbd_close,
    .read = kbd_read,
    .write = kbd_write,
    .poll = kbd_poll,
};

// Driver probe function
static int kbd_probe(device_t* dev) {
    dev->ops = &kbd_ops;
    return 0;
}

// Driver operations
static driver_ops_t kbd_driver_ops = {
    .probe = kbd_probe,
};

// Driver structure
static driver_t kbd_driver = {
    .name = "keyboard",
    .type = DEV_TYPE_CHAR,
    .ops = &kbd_driver_ops,
};

// Initialize keyboard driver
void keyboard_driver_init(void) {
    // Register driver
    driver_register(&kbd_driver);
    
    // Create keyboard device
    device_t* kbd = char_device_create("kbd", 10, 1);
    if (kbd) {
        device_register(kbd);
    }
    
    // TODO: Register IRQ handler (IRQ 1)
    // For now, just log initialization
    kprintf("[KEYBOARD] PS/2 keyboard driver initialized\n");
}
