/* LimitlessOS Kernel Linker Script */

ENTRY(multiboot2_start_export)

SECTIONS
{
    /* Kernel loads at 1MB physical */
    . = 1M;

    .boot ALIGN(8):
    {
        KEEP(*(.multiboot2))
    }

    /* Read-only sections */
    .text ALIGN(4K) :
    {
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata))
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* Read-write sections */
    .data ALIGN(4K) : AT(ADDR(.data))
    {
        *(.data)
        *(.data.*)
    }

    /* BSS section */
    .bss ALIGN(4K) : AT(ADDR(.bss))
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Global constructors */
    .init_array ALIGN(8) :
    {
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
    }

    /* Global destructors */
    .fini_array ALIGN(8) :
    {
        __fini_array_start = .;
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        __fini_array_end = .;
    }

    /* Kernel end marker */
    . = ALIGN(4K);
    _kernel_end = .;

    /* Discard unnecessary sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.*)
    }
}
