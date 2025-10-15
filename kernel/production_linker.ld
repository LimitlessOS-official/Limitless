/*
 * LimitlessOS Production Linker Script
 * 
 * Enterprise-grade linker script with optimized memory layout,
 * proper section ordering, and comprehensive memory management.
 * Designed for maximum compatibility and robustness.
 */

ENTRY(_start)

/* Memory layout constants for optimal performance */
KERNEL_LOAD_ADDRESS = 0x100000;        /* 1MB - Standard multiboot load address */
KERNEL_VIRTUAL_BASE = 0xC0000000;      /* 3GB - Higher half kernel base */
PAGE_SIZE = 4K;                        /* 4KB pages for x86 */
SECTION_ALIGN = PAGE_SIZE;             /* Align all sections to page boundaries */

/* Memory regions definition */
MEMORY
{
    /* Low memory region for multiboot header and boot code */
    LOW_MEM (rwx) : ORIGIN = KERNEL_LOAD_ADDRESS, LENGTH = 1M
    
    /* Main kernel memory region */
    KERNEL_MEM (rwx) : ORIGIN = KERNEL_LOAD_ADDRESS + 1M, LENGTH = 15M
}

SECTIONS
{
    /* Ensure we start at the correct load address */
    . = KERNEL_LOAD_ADDRESS;
    
    /* ========================================================================
     * MULTIBOOT2 SECTION - CRITICAL: Must be in first 32KB of file
     * ======================================================================== */
    .multiboot2 ALIGN(8) : AT(ADDR(.multiboot2))
    {
        __multiboot2_start = .;
        KEEP(*(.multiboot2))
        __multiboot2_end = .;
        
        /* Ensure multiboot2 header is within first 32KB */
        ASSERT(__multiboot2_end - __multiboot2_start <= 32K, 
               "ERROR: Multiboot2 header exceeds 32KB limit!")
        ASSERT(ADDR(.multiboot2) == KERNEL_LOAD_ADDRESS,
               "ERROR: Multiboot2 header not at load address!")
    } > LOW_MEM
    
    /* Padding to ensure proper alignment for next section */
    . = ALIGN(SECTION_ALIGN);
    
    /* ========================================================================
     * BOOT CODE SECTION - 32-bit entry point and initialization
     * ======================================================================== */
    .boot ALIGN(SECTION_ALIGN) : AT(ADDR(.boot))
    {
        __boot_start = .;
        *(.text.boot)           /* Boot assembly code */
        . = ALIGN(16);
        __boot_end = .;
    } > LOW_MEM
    
    /* ========================================================================
     * TEXT SECTION - Main kernel code
     * ======================================================================== */
    .text ALIGN(SECTION_ALIGN) : AT(ADDR(.text))
    {
        __text_start = .;
        
        /* Entry point functions first for optimal loading */
        *(.text._start)
        *(.text.kernel_main)
        
        /* Core kernel functions */
        *(.text.multiboot*)
        *(.text.terminal*)
        *(.text.boot*)
        
        /* All other text */
        *(.text*)
        *(.text.*)
        
        /* GNU linkonce sections */
        *(.gnu.linkonce.t.*)
        
        . = ALIGN(16);
        __text_end = .;
    } > LOW_MEM
    
    /* ========================================================================
     * READ-ONLY DATA SECTIONS
     * ======================================================================== */
    
    /* Exception handling data */
    .eh_frame_hdr ALIGN(8) : AT(ADDR(.eh_frame_hdr))
    {
        __eh_frame_hdr_start = .;
        *(.eh_frame_hdr)
        __eh_frame_hdr_end = .;
    } > LOW_MEM
    
    .eh_frame ALIGN(8) : AT(ADDR(.eh_frame))
    {
        __eh_frame_start = .;
        *(.eh_frame)
        __eh_frame_end = .;
    } > LOW_MEM
    
    /* Read-only data */
    .rodata ALIGN(SECTION_ALIGN) : AT(ADDR(.rodata))
    {
        __rodata_start = .;
        
        /* String literals and constants */
        *(.rodata*)
        *(.rodata.*)
        
        /* GNU linkonce sections */
        *(.gnu.linkonce.r.*)
        
        . = ALIGN(16);
        __rodata_end = .;
    } > LOW_MEM
    
    /* ========================================================================
     * INITIALIZED DATA SECTIONS
     * ======================================================================== */
    .data ALIGN(SECTION_ALIGN) : AT(ADDR(.data))
    {
        __data_start = .;
        
        /* Global and static variables */
        *(.data*)
        *(.data.*)
        
        /* GNU linkonce sections */
        *(.gnu.linkonce.d.*)
        
        /* Constructor and destructor lists (if needed) */
        . = ALIGN(8);
        __ctors_start = .;
        *(.ctors*)
        __ctors_end = .;
        
        . = ALIGN(8);
        __dtors_start = .;
        *(.dtors*)
        __dtors_end = .;
        
        . = ALIGN(16);
        __data_end = .;
    } > LOW_MEM
    
    /* ========================================================================
     * UNINITIALIZED DATA SECTIONS
     * ======================================================================== */
    .bss ALIGN(SECTION_ALIGN) : AT(ADDR(.bss))
    {
        __bss_start = .;
        
        /* Uninitialized global and static variables */
        *(.bss*)
        *(.bss.*)
        *(COMMON)
        
        /* GNU linkonce sections */
        *(.gnu.linkonce.b.*)
        
        . = ALIGN(16);
        __bss_end = .;
    } > LOW_MEM
    
    /* ========================================================================
     * KERNEL STACK SECTION
     * ======================================================================== */
    .stack ALIGN(SECTION_ALIGN) : AT(ADDR(.stack))
    {
        __stack_bottom = .;
        . += 64K;                /* 64KB kernel stack */
        . = ALIGN(SECTION_ALIGN);
        __stack_top = .;
    } > LOW_MEM
    
    /* ========================================================================
     * KERNEL HEAP SECTION (for future memory management)
     * ======================================================================== */
    .heap ALIGN(SECTION_ALIGN) : AT(ADDR(.heap))
    {
        __heap_start = .;
        . += 1M;                 /* 1MB initial heap */
        . = ALIGN(SECTION_ALIGN);
        __heap_end = .;
    } > LOW_MEM
    
    /* ========================================================================
     * KERNEL END MARKER
     * ======================================================================== */
    . = ALIGN(SECTION_ALIGN);
    __kernel_end = .;
    
    /* Calculate kernel size for memory management */
    __kernel_size = __kernel_end - KERNEL_LOAD_ADDRESS;
    
    /* ========================================================================
     * DEBUG SECTIONS (not loaded into memory)
     * ======================================================================== */
    
    /* DWARF Debug Information */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
    .debug_pubtypes  0 : { *(.debug_pubtypes) }
    .debug_ranges    0 : { *(.debug_ranges) }
    .debug_macro     0 : { *(.debug_macro) }
    
    /* GNU Build ID */
    .note.gnu.build-id 0 : { *(.note.gnu.build-id) }
    
    /* ========================================================================
     * DISCARDED SECTIONS - Remove unwanted sections
     * ======================================================================== */
    /DISCARD/ :
    {
        /* Remove sections that cause issues with freestanding kernels */
        *(.note*)
        *(.comment*)
        *(.eh_frame*)
        *(.eh_frame_hdr*)
        *(.gcc_except_table*)
        *(.gnu.version*)
        *(.gnu.hash)
        *(.gnu.gnu_debuglink*)
        *(.gnu.gnu_debugaltlink*)
        
        /* Remove dynamic linking sections */
        *(.dynamic)
        *(.dynstr)
        *(.dynsym)
        *(.interp)
        *(.hash)
        *(.plt*)
        *(.got*)
        
        /* Remove exception handling sections for kernel */
        *(.gcc_except_table)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
}

/* ============================================================================
 * EXPORTED SYMBOLS - Available to kernel code
 * ============================================================================ */

/* Memory layout information */
PROVIDE(__kernel_start = KERNEL_LOAD_ADDRESS);
PROVIDE(__kernel_physical_start = KERNEL_LOAD_ADDRESS);
PROVIDE(__kernel_virtual_start = KERNEL_LOAD_ADDRESS);

/* Section boundaries */
PROVIDE(__text_size = __text_end - __text_start);
PROVIDE(__rodata_size = __rodata_end - __rodata_start);
PROVIDE(__data_size = __data_end - __data_start);
PROVIDE(__bss_size = __bss_end - __bss_start);

/* Stack information */
PROVIDE(__initial_stack_top = __stack_top);
PROVIDE(__stack_size = __stack_top - __stack_bottom);

/* Heap information */
PROVIDE(__heap_size = __heap_end - __heap_start);

/* Total kernel size */
PROVIDE(__kernel_total_size = __kernel_size);

/* ============================================================================
 * MEMORY LAYOUT ASSERTIONS - Ensure correctness
 * ============================================================================ */

/* Verify multiboot2 header placement */
ASSERT(__multiboot2_start == KERNEL_LOAD_ADDRESS, 
       "Multiboot2 header must be at load address")
ASSERT(__multiboot2_end - __multiboot2_start <= 32K,
       "Multiboot2 header must be within first 32KB")

/* Verify section alignment */
ASSERT(__text_start % SECTION_ALIGN == 0, "Text section not page-aligned")
ASSERT(__rodata_start % SECTION_ALIGN == 0, "Rodata section not page-aligned")
ASSERT(__data_start % SECTION_ALIGN == 0, "Data section not page-aligned")
ASSERT(__bss_start % SECTION_ALIGN == 0, "BSS section not page-aligned")

/* Verify reasonable kernel size (less than 16MB total) */
ASSERT(__kernel_size < 16M, "Kernel size exceeds 16MB limit")
ASSERT(__kernel_size > 4K, "Kernel size too small (less than 4KB)")

/* Verify stack size */
ASSERT(__stack_size >= 16K, "Stack too small (minimum 16KB required)")
ASSERT(__stack_size <= 1M, "Stack too large (maximum 1MB recommended)")

/* Verify no section overlaps */
ASSERT(__text_end <= __rodata_start || __rodata_end <= __text_start,
       "Text and rodata sections overlap")
ASSERT(__rodata_end <= __data_start || __data_end <= __rodata_start,
       "Rodata and data sections overlap")
ASSERT(__data_end <= __bss_start || __bss_end <= __data_start,
       "Data and BSS sections overlap")

/* ============================================================================
 * MEMORY LAYOUT DOCUMENTATION
 * ============================================================================ */

/*
 * Final Memory Layout:
 * 
 * Physical Address    Virtual Address     Section           Size        Purpose
 * ================    ===============     =======           ====        =======
 * 0x100000           0x100000            .multiboot2       ~100B       Multiboot2 header
 * 0x101000           0x101000            .boot             varies      Boot assembly code
 * 0x102000           0x102000            .text             varies      Kernel executable code
 * 0x103000+          0x103000+           .rodata           varies      Read-only data
 * 0x104000+          0x104000+           .data             varies      Initialized data
 * 0x105000+          0x105000+           .bss              varies      Uninitialized data
 * 0x106000+          0x106000+           .stack            64KB        Kernel stack
 * 0x116000+          0x116000+           .heap             1MB         Initial heap
 * 
 * Key Features:
 * - Multiboot2 header at exact load address (1MB)
 * - All sections page-aligned (4KB boundaries)
 * - Comprehensive memory layout validation
 * - Support for debug information (not loaded)
 * - Clean separation of code, data, and stack
 * - Future-proof heap allocation
 * - Robust error checking and assertions
 */