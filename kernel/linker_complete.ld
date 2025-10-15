/*
 * LimitlessOS Complete Kernel Linker Script
 * Links all enterprise subsystems into bootable kernel
 */

ENTRY(_start)

SECTIONS
{
    . = 0x100000;  /* Load kernel at 1MB mark */
    
    .multiboot : {
        *(.multiboot)
    }
    
    .text ALIGN(4K) : {
        *(.text)
    }
    
    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    }
    
    .data ALIGN(4K) : {
        *(.data)
    }
    
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }
    
    /* Enterprise subsystem sections */
    .enterprise_smp ALIGN(4K) : {
        *(.enterprise_smp)
    }
    
    .enterprise_iommu ALIGN(4K) : {
        *(.enterprise_iommu)
    }
    
    .enterprise_hal ALIGN(4K) : {
        *(.enterprise_hal)
    }
    
    .enterprise_storage ALIGN(4K) : {
        *(.enterprise_storage)
    }
    
    .enterprise_network ALIGN(4K) : {
        *(.enterprise_network)
    }
    
    .enterprise_multimedia ALIGN(4K) : {
        *(.enterprise_multimedia)
    }
    
    .enterprise_virt ALIGN(4K) : {
        *(.enterprise_virt)
    }
    
    .enterprise_security ALIGN(4K) : {
        *(.enterprise_security)
    }
    
    /* Desktop GUI sections */
    .gui_compositor ALIGN(4K) : {
        *(.gui_compositor)
    }
    
    .gui_applications ALIGN(4K) : {
        *(.gui_applications)
    }
    
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}