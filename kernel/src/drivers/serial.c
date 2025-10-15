/*
 * Serial Port Driver (COM1-COM4)
 * 
 * Implements basic serial port communication:
 * - Character device interface
 * - 8250/16550 UART support
 * - Configurable baud rates
 * - Interrupt-driven I/O
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "kernel.h"
#include "device.h"
#include <string.h>

// Serial port I/O ports
#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

// UART registers (offset from base)
#define UART_DATA       0   // Data register (R/W)
#define UART_IER        1   // Interrupt Enable Register
#define UART_IIR        2   // Interrupt ID Register
#define UART_LCR        3   // Line Control Register
#define UART_MCR        4   // Modem Control Register
#define UART_LSR        5   // Line Status Register
#define UART_MSR        6   // Modem Status Register
#define UART_SCRATCH    7   // Scratch Register

// Line Status Register bits
#define LSR_DATA_READY  0x01
#define LSR_OVERRUN     0x02
#define LSR_PARITY_ERR  0x04
#define LSR_FRAMING_ERR 0x08
#define LSR_BREAK       0x10
#define LSR_THR_EMPTY   0x20
#define LSR_TRANSMITTER_EMPTY 0x40

// Baud rate divisors
#define BAUD_115200     1
#define BAUD_57600      2
#define BAUD_38400      3
#define BAUD_19200      6
#define BAUD_9600       12

// Forward declarations
extern void* kmalloc(size_t size);

// Serial port private data
typedef struct {
    u16 port;       // I/O port base
    u32 baud;       // Baud rate
    u8 irq;         // IRQ number
} serial_port_t;

// Initialize serial port hardware
static int serial_init_port(u16 port) {
    // Disable interrupts
    outb(port + UART_IER, 0x00);
    
    // Enable DLAB (set baud rate divisor)
    outb(port + UART_LCR, 0x80);
    
    // Set baud rate to 38400 (divisor = 3)
    outb(port + UART_DATA, 0x03);
    outb(port + UART_IER, 0x00);
    
    // 8 bits, no parity, one stop bit
    outb(port + UART_LCR, 0x03);
    
    // Enable FIFO, clear them, 14-byte threshold
    outb(port + UART_IIR, 0xC7);
    
    // Enable IRQs, set RTS/DSR
    outb(port + UART_MCR, 0x0B);
    
    // Enable interrupts
    outb(port + UART_IER, 0x01);
    
    return 0;
}

// Check if transmitter is ready
static int serial_transmit_ready(u16 port) {
    return inb(port + UART_LSR) & LSR_THR_EMPTY;
}

// Check if data is available
static int serial_data_available(u16 port) {
    return inb(port + UART_LSR) & LSR_DATA_READY;
}

// Write a byte to serial port
static void serial_write_byte(u16 port, u8 data) {
    while (!serial_transmit_ready(port));
    outb(port + UART_DATA, data);
}

// Read a byte from serial port
static u8 serial_read_byte(u16 port) {
    while (!serial_data_available(port));
    return inb(port + UART_DATA);
}

// Device operations
static int serial_open(device_t* dev) {
    serial_port_t* port = (serial_port_t*)dev->private_data;
    if (!port) return -1;
    
    serial_init_port(port->port);
    dev->flags |= DEV_FLAG_READY;
    
    return 0;
}

static int serial_close(device_t* dev) {
    dev->flags &= ~DEV_FLAG_READY;
    return 0;
}

static long serial_read(device_t* dev, u64 offset, void* buf, size_t len) {
    (void)offset; // Serial ports don't use offsets
    
    serial_port_t* port = (serial_port_t*)dev->private_data;
    if (!port) return -1;
    
    u8* buffer = (u8*)buf;
    size_t i;
    
    for (i = 0; i < len; i++) {
        if (!serial_data_available(port->port)) {
            break; // No more data available
        }
        buffer[i] = serial_read_byte(port->port);
    }
    
    return i;
}

static long serial_write(device_t* dev, u64 offset, const void* buf, size_t len) {
    (void)offset; // Serial ports don't use offsets
    
    serial_port_t* port = (serial_port_t*)dev->private_data;
    if (!port) return -1;
    
    const u8* buffer = (const u8*)buf;
    
    for (size_t i = 0; i < len; i++) {
        serial_write_byte(port->port, buffer[i]);
    }
    
    return len;
}

static int serial_ioctl(device_t* dev, u32 cmd, void* arg) {
    (void)dev; (void)cmd; (void)arg;
    // TODO: Implement ioctl for baud rate, parity, etc.
    return -1;
}

// Device operations table
static device_ops_t serial_ops = {
    .open = serial_open,
    .close = serial_close,
    .read = serial_read,
    .write = serial_write,
    .ioctl = serial_ioctl,
};

// Driver probe function
static int serial_probe(device_t* dev) {
    // Simple probe - just accept the device
    dev->ops = &serial_ops;
    return 0;
}

// Driver operations
static driver_ops_t serial_driver_ops = {
    .probe = serial_probe,
};

// Driver structure
static driver_t serial_driver = {
    .name = "serial",
    .type = DEV_TYPE_CHAR,
    .ops = &serial_driver_ops,
};

// Initialize serial driver and create devices
void serial_driver_init(void) {
    // Register driver
    driver_register(&serial_driver);
    
    // Create COM1 device
    device_t* com1 = char_device_create("ttyS0", 4, 64);
    if (com1) {
        serial_port_t* port = kmalloc(sizeof(serial_port_t));
        if (port) {
            port->port = COM1;
            port->baud = 38400;
            port->irq = 4;
            com1->private_data = port;
            device_register(com1);
        }
    }
    
    // Create COM2 device
    device_t* com2 = char_device_create("ttyS1", 4, 65);
    if (com2) {
        serial_port_t* port = kmalloc(sizeof(serial_port_t));
        if (port) {
            port->port = COM2;
            port->baud = 38400;
            port->irq = 3;
            com2->private_data = port;
            device_register(com2);
        }
    }
    
    kprintf("[SERIAL] Serial port driver initialized\n");
}
