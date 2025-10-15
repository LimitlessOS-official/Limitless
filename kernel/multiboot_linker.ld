/* LimitlessOS Multiboot2 Kernel Linker Script */

ENTRY(_start)

SECTIONS
{
    /* Start at 1MB (standard for multiboot kernels) */
    . = 1M;

    /* Multiboot header must be within first 32KB of file */
    .multiboot ALIGN(8) : {
        *(.multiboot)
    }

    /* Text section (code) */
    .text ALIGN(4K) : {
        *(.text)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata*)
    }

    /* Initialized data */
    .data ALIGN(4K) : {
        *(.data)
    }

    /* Uninitialized data */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }

    /* Ensure sections don't overlap */
    /DISCARD/ : {
        *(.note*)
        *(.eh_frame*)
        *(.comment)
    }
}