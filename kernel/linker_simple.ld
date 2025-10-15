/*
 * Simplified bootable kernel linker script for LimitlessOS
 * Designed to create a functional bootable kernel
 */

ENTRY(_start)

SECTIONS
{
    /* Multiboot header must be at the beginning */
    . = 0x100000; /* 1MB mark for kernel loading */
    
    .multiboot ALIGN(4K) : {
        *(.multiboot)
    }
    
    .text ALIGN(4K) : {
        *(.text)
        *(.text.*)
    }
    
    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    }
    
    .data ALIGN(4K) : {
        *(.data)
        *(.data.*)
    }
    
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
    
    _edata = .;
    _end = .;
}