#include "hal_core.h"
/*
 * Interrupt Descriptor Table (IDT) Implementation
 * 
 * This module implements the Interrupt Descriptor Table for x86 architecture.
 * The IDT is a critical data structure that the CPU uses to determine which
 * function to call when an interrupt or exception occurs.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 * Licensed under GPL v3
 */

#include <idt.h>
#include <common.h>
#include <isr.h>
#include <string.h>

#include "hal/hal_kernel.h"

// Declare the external ISR stub functions from assembly
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

// IRQs (hardware interrupts)
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

// System call interrupt
extern void isr128();

// The IDT itself - 256 entries
struct idt_entry idt_entries[256];
struct idt_ptr   idt_ptr;

// External assembly function to load the IDT
extern void idt_flush(uint32_t);

/**
 * Set an entry in the IDT
 * 
 * @param num The interrupt number (0-255)
 * @param base The address of the ISR
 * @param sel The kernel code segment selector (usually 0x08)
 * @param flags Access flags (present, DPL, type)
 */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    
    // Uncomment the OR below when you get to using user-mode.
    // It sets the interrupt gate's privilege level to 3.
    idt_entries[num].flags   = flags /* | 0x60 */;
}

/**
 * Initialize the Interrupt Descriptor Table
 * 
 * This function sets up all the IDT entries and loads the IDT into the CPU.
 * It maps CPU exceptions (0-31), hardware interrupts (32-47), and the
 * system call interrupt (0x80 = 128).
 */
void idt_init()
{
    idt_ptr.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;
    
    // Clear the IDT
    memset(&idt_entries, 0, sizeof(struct idt_entry) * 256);
    
    // Remap the IRQ table to use interrupts 32-47
    // By default, IRQs 0-7 are mapped to interrupts 8-15,
    // which conflicts with CPU exceptions. We remap them.
    outb(0x20, 0x11);  // Initialize PIC1
    outb(0xA0, 0x11);  // Initialize PIC2
    outb(0x21, 0x20);  // PIC1 offset (32)
    outb(0xA1, 0x28);  // PIC2 offset (40)
    outb(0x21, 0x04);  // Tell PIC1 there's a slave PIC at IRQ2
    outb(0xA1, 0x02);  // Tell PIC2 its cascade identity
    outb(0x21, 0x01);  // 8086 mode for PIC1
    outb(0xA1, 0x01);  // 8086 mode for PIC2
    outb(0x21, 0x0);   // Unmask all IRQs on PIC1
    outb(0xA1, 0x0);   // Unmask all IRQs on PIC2
    
    // Set up CPU exception handlers (ISRs 0-31)
    // Flags: 0x8E = Present, Ring 0, 32-bit Interrupt Gate
    idt_set_gate(0,  (uint32_t)isr0,  0x08, 0x8E);
    idt_set_gate(1,  (uint32_t)isr1,  0x08, 0x8E);
    idt_set_gate(2,  (uint32_t)isr2,  0x08, 0x8E);
    idt_set_gate(3,  (uint32_t)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (uint32_t)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (uint32_t)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (uint32_t)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (uint32_t)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (uint32_t)isr8,  0x08, 0x8E);
    idt_set_gate(9,  (uint32_t)isr9,  0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);
    
    // Set up hardware interrupt handlers (IRQs 0-15, mapped to ISRs 32-47)
    idt_set_gate(32, (uint32_t)irq0,  0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1,  0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2,  0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3,  0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4,  0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5,  0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6,  0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7,  0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8,  0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9,  0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
    
    // Set up system call handler (int 0x80 = 128)
    // Flags: 0xEE = Present, Ring 3 (user mode can call), 32-bit Interrupt Gate
    idt_set_gate(128, (uint32_t)isr128, 0x08, 0xEE);
    
    // Load the IDT
    idt_flush((uint32_t)&idt_ptr);
}
