/*
 * LimitlessOS Production Multiboot2 Linker Script
 *
 * This linker script ensures proper multiboot2 header placement,
 * memory layout, and section alignment for reliable boot loading.
 * Compatible with GRUB2 and other multiboot2-compliant bootloaders.
 */

ENTRY(_start)

/* Memory layout constants */
KERNEL_BASE_VIRTUAL  = 0xFFFFFFFF80000000;  /* Higher half kernel */
KERNEL_BASE_PHYSICAL = 0x100000;            /* 1MB - standard multiboot load address */

SECTIONS
{
    /* Ensure we start at the physical load address */
    . = KERNEL_BASE_PHYSICAL;

    /* Multiboot2 header MUST be in the first 32KB of the file */
    .multiboot2 ALIGN(8) : AT(ADDR(.multiboot2))
    {
    KEEP(*(.multiboot2))
    KEEP(*(multiboot2_start_export))
    PROVIDE(multiboot2_start = ADDR(.multiboot2));
        . = ALIGN(8);
    }

    /* Boot code that runs in 32-bit mode */
    .boot ALIGN(4K) : AT(ADDR(.boot))
    {
        boot_start = .;
        *(.text.boot)
        . = ALIGN(4K);
        boot_end = .;
    }

    /* Switch to higher half for the main kernel */
    . += KERNEL_BASE_VIRTUAL;

    /* Main kernel text section */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_BASE_VIRTUAL)
    {
        text_start = .;
        *(.text*)
        . = ALIGN(4K);
        text_end = .;
    }

    /* Read-only data */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_BASE_VIRTUAL)
    {
        rodata_start = .;
        *(.rodata*)
        . = ALIGN(4K);
        rodata_end = .;
    }

    /* Initialized data */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_BASE_VIRTUAL)
    {
        data_start = .;
        *(.data*)
        . = ALIGN(4K);
        data_end = .;
    }

    /* Uninitialized data */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_BASE_VIRTUAL)
    {
        bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4K);
        bss_end = .;
    }

    /* Stack section */
    .stack ALIGN(4K) : AT(ADDR(.stack) - KERNEL_BASE_VIRTUAL)
    {
        stack_bottom = .;
        . += 16K;  /* 16KB stack */
        . = ALIGN(4K);
        stack_top = .;
    }

    /* Kernel end marker */
    kernel_end = .;
    kernel_size = kernel_end - KERNEL_BASE_VIRTUAL - KERNEL_BASE_PHYSICAL;

    /* Debug sections (not loaded into memory) */
    .debug_info     0 : { *(.debug_info) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_str      0 : { *(.debug_str) }
    .debug_ranges   0 : { *(.debug_ranges) }
    .debug_loc      0 : { *(.debug_loc) }

    /* Discard unwanted sections */
    /DISCARD/ :
    {
        *(.note*)
        *(.comment*)
        *(.eh_frame*)
        *(.gcc_except_table*)
    }
}

/* Provide physical addresses for boot code */
boot_start_phys = boot_start - KERNEL_BASE_VIRTUAL;
text_start_phys = text_start - KERNEL_BASE_VIRTUAL;
data_start_phys = data_start - KERNEL_BASE_VIRTUAL;
bss_start_phys = bss_start - KERNEL_BASE_VIRTUAL;

/* Memory layout validation */
ASSERT(SIZEOF(.multiboot2) < 32K, "Multiboot2 header too large!")
ASSERT(ADDR(.multiboot2) >= KERNEL_BASE_PHYSICAL, "Multiboot2 header below load address!")
ASSERT(ADDR(.multiboot2) < KERNEL_BASE_PHYSICAL + 32K, "Multiboot2 header not in first 32KB!")
ASSERT(kernel_size < 16M, "Kernel too large!")
ASSERT((bss_end - bss_start) % 4K == 0, "BSS section not page-aligned!")

/*
 * Memory layout summary:
 * 
 * Physical Address    Virtual Address         Section
 * ================    ===============         =======
 * 0x100000           0x100000                .multiboot2
 * 0x101000           0x101000                .boot
 * 0x102000           0xFFFFFFFF80102000      .text
 * ...                ...                     .rodata, .data, .bss
 * 
 * The bootloader loads everything at physical addresses starting at 1MB.
 * The boot code sets up paging and maps the kernel to higher half virtual addresses.
 */