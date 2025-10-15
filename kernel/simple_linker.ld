/*
 * LimitlessOS Simple Multiboot2 Linker Script
 *
 * Simple, reliable linker script for multiboot2 compliance
 */

ENTRY(_start)

/* Load at 1MB physical address */
. = 0x100000;

SECTIONS
{
    /* Multiboot2 header - must be in first 32KB */
    .multiboot2 ALIGN(8) : {
        KEEP(*(.multiboot2))
        *(.multiboot2_end)
    }

    /* Boot text section */
    .text ALIGN(4096) : {
        *(.text)
    }

    /* Read-only data */
    .rodata ALIGN(4096) : {
        *(.rodata*)
    }

    /* Initialized data */
    .data ALIGN(4096) : {
        *(.data)
    }

    /* Uninitialized data */
    .bss ALIGN(4096) : {
        *(COMMON)
        *(.bss)
    }

    /* Discard debug info */
    /DISCARD/ : {
        *(.note*)
        *(.comment*)
        *(.eh_frame*)
    }
}