#ifndef IDT_H
#define IDT_H

#include <common.h>

// Defines an entry in the Interrupt Descriptor Table.
// This structure is architecture-specific (x86).
struct idt_entry {
    uint16_t base_lo;    // The lower 16 bits of the ISR's address
    uint16_t sel;        // The kernel segment selector
    uint8_t  always0;    // This must always be zero
    uint8_t  flags;      // More flags. See documentation.
    uint16_t base_hi;    // The upper 16 bits of the ISR's address
} __attribute__((packed));

// Defines a pointer to the array of interrupt handlers.
// This format is required by the `lidt` instruction.
struct idt_ptr {
    uint16_t limit;
    uint32_t base;       // The address of the first element in idt_entry_t
} __attribute__((packed));

// Initializes the Interrupt Descriptor Table.
void idt_init();

#endif // IDT_H
