#
# LimitlessOS - Multiboot2 Boot Assembly
#
# This file contains the Multiboot2 header and the initial entry point
# for the kernel. It sets up a minimal environment and jumps to the
# 64-bit C kernel.
#


.section .multiboot2
.align 8
.global multiboot2_start_export
multiboot2_start_export:

# Multiboot2 Header
multiboot_header:
    .long 0xe85250d6                    # Multiboot2 magic number
    .long 0                             # Architecture (i386)
    .long multiboot_header_end - multiboot_header # Header length
    # Checksum
    .long -(0xe85250d6 + 0 + (multiboot_header_end - multiboot_header))

# Framebuffer tag
framebuffer_tag:
    .short 5                           # Type: Framebuffer
    .short 0                           # Flags: 0 (optional)
    .long framebuffer_tag_end - framebuffer_tag # Size
    .long 1024                          # Width
    .long 768                           # Height
    .long 32                            # Depth
framebuffer_tag_end:

# End tag
end_tag:
    .short 0                           # Type: End
    .short 0                           # Flags
    .long 8                             # Size
multiboot_header_end:

.section .text

.global _start
.global multiboot2_start_export
.extern kernel_main

.code32
_start:
    # We are in 32-bit protected mode.
    # The bootloader has set up a basic GDT.
    # `eax` contains the magic number 0x36d76289.
    # `ebx` contains the physical address of the Multiboot2 info struct.

    # Set up the stack
    movl $stack_top, %esp

    # Push the Multiboot2 info pointer and magic number to the stack
    # so the C kernel can access them.
    pushl %ebx
    pushl %eax

    # Call the C kernel's main function
    call kernel_main

    # If kernel_main returns, hang the system.
    cli
hang:
    hlt
    jmp hang

.section .bss
.align 16
stack_bottom:
    .skip 16384 # 16 KB stack
stack_top: