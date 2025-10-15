/*
 * Interrupt Service Routine (ISR) Handlers
 * 
 * This module provides high-level C handlers for CPU exceptions and
 * hardware interrupts. It includes detailed error messages for debugging.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <isr.h>
#include <common.h>
#include <drivers/vga_text.h>

// Array of function pointers for custom interrupt handlers
isr_t interrupt_handlers[256];

// Exception messages for CPU exceptions
static const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

/**
 * Register a custom handler for a specific interrupt
 * 
 * @param n The interrupt number
 * @param handler The function to call when this interrupt occurs
 */
void register_interrupt_handler(uint8_t n, isr_t handler)
{
    interrupt_handlers[n] = handler;
}

/**
 * Main ISR handler - called from assembly stub
 * 
 * This function is called whenever a CPU exception occurs. It prints
 * detailed information about the exception and either calls a custom
 * handler if one is registered, or halts the system.
 */
void isr_handler(registers_t *regs)
{
    // If a custom handler has been registered for this interrupt, call it
    if (interrupt_handlers[regs->int_no] != 0)
    {
        isr_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    }
    else
    {
        // No custom handler - this is an unhandled exception
        vga_text_writestring("\n!!! KERNEL PANIC !!!\n");
        vga_text_writestring("Unhandled interrupt: ");
        
        if (regs->int_no < 32)
        {
            vga_text_writestring(exception_messages[regs->int_no]);
        }
        else
        {
            vga_text_writestring("Unknown");
        }
        
        vga_text_writestring("\n");
        
        // Print register dump for debugging
        vga_text_writestring("Register dump:\n");
        // TODO: Implement hex printing and display all register values
        
        // Halt the system
        for(;;) {
            asm volatile("hlt");
        }
    }
}

/**
 * IRQ handler - called from assembly stub
 * 
 * This function handles hardware interrupts. It sends an EOI (End of Interrupt)
 * signal to the PIC and calls any registered custom handler.
 */
void irq_handler(registers_t *regs)
{
    // Send an EOI (End of Interrupt) signal to the PICs.
    // If the interrupt came from the slave PIC (IRQ 8-15), we need to
    // send an EOI to both the master and slave PICs.
    if (regs->int_no >= 40)
    {
        // Send EOI to slave PIC
        outb(0xA0, 0x20);
    }
    // Send EOI to master PIC
    outb(0x20, 0x20);
    
    // Call the custom handler if one is registered
    if (interrupt_handlers[regs->int_no] != 0)
    {
        isr_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    }
}
