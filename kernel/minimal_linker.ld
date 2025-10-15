/* LimitlessOS Minimal Kernel Linker Script with Multiboot2 Support */

ENTRY(kernel_main)

SECTIONS {
    . = 1M;
    
    /* Multiboot2 header must be within first 32KB */
    .multiboot2 ALIGN(8) : {
        *(.multiboot2)
    }
    
    .text ALIGN(4K) : {
        *(.text)
    }
    
    .data ALIGN(4K) : {
        *(.data)
        *(.rodata)
    }
    
    .bss ALIGN(4K) : {
        *(.bss)
    }
}