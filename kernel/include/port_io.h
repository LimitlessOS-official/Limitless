/*
 * Port I/O Helper Functions
 * 
 * Simple inline port I/O for x86/x86_64 architecture
 * Used by device drivers for hardware communication
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#ifndef PORT_IO_H
#define PORT_IO_H

#include <stdint.h>

// Output byte to port
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Input byte from port
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Output word to port
static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

// Input word from port
static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Output dword to port
static inline void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

// Input dword from port
static inline uint32_t inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// I/O wait (for timing)
static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif // PORT_IO_H
