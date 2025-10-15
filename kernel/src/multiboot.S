/*
 * LimitlessOS Multiboot2 Header
 * Enables booting with GRUB2 and UEFI
 */

#define MULTIBOOT2_MAGIC 0x36d76289
#define MULTIBOOT_ARCHITECTURE_I386 0

.section .multiboot
.align 8

multiboot_header_start:
    .long MULTIBOOT2_MAGIC
    .long MULTIBOOT_ARCHITECTURE_I386
    .long multiboot_header_end - multiboot_header_start
    .long -(MULTIBOOT2_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header_start))

    /* Information request tag */
    .align 8
    .short 1    /* Type: information request */
    .short 0    /* Flags */
    .long 20    /* Size */
    .long 4     /* Memory map */
    .long 6     /* Memory info */
    .long 8     /* Framebuffer info */

    /* Address tag for kernel loading */
    .align 8
    .short 2    /* Type: address */
    .short 0    /* Flags */
    .long 24    /* Size */
    .long multiboot_header_start    /* Header address */
    .long _start                    /* Load address */
    .long _edata                    /* Load end address */
    .long _end                      /* BSS end address */

    /* Entry point tag */
    .align 8
    .short 3    /* Type: entry address */
    .short 0    /* Flags */
    .long 12    /* Size */
    .long _start                    /* Entry address */

    /* Framebuffer tag for GUI support */
    .align 8
    .short 5    /* Type: framebuffer */
    .short 0    /* Flags */
    .long 20    /* Size */
    .long 1920  /* Width */
    .long 1080  /* Height */
    .long 32    /* Depth (bits per pixel) */

    /* Module alignment tag */
    .align 8
    .short 6    /* Type: module alignment */
    .short 0    /* Flags */
    .long 8     /* Size */

    /* EFI boot services tag */
    .align 8
    .short 7    /* Type: EFI boot services */
    .short 0    /* Flags */
    .long 8     /* Size */

    /* End tag */
    .align 8
    .short 0    /* Type: end */
    .short 0    /* Flags */
    .long 8     /* Size */

multiboot_header_end: