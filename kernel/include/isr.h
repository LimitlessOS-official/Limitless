#ifndef ISR_H
#define ISR_H

#include <common.h>

// A struct describing a processor state pushed to the stack by an ISR.
// This is what the interrupt handler receives as an argument.
typedef struct registers
{
    uint32_t ds;                  // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;

// Defines a function pointer for our interrupt handlers.
typedef void (*isr_t)(registers_t*);

// Allows us to register a custom interrupt handler for a given interrupt.
void register_interrupt_handler(uint8_t n, isr_t handler);

#endif // ISR_H
