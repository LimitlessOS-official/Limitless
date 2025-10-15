/**
 * SMP Trampoline Code for LimitlessOS
 * 
 * This assembly code is executed by Application Processors (APs) during
 * the SMP boot process. It sets up the basic environment and jumps to
 * the C initialization code.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

.code16
.section .text

.global smp_trampoline_start
.global smp_trampoline_end
.global smp_trampoline_target

/* Real mode entry point for APs */
smp_trampoline_start:
    cli                     /* Disable interrupts */
    
    /* Set up segments */
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    /* Load GDT */
    lgdt gdt_descriptor - smp_trampoline_start
    
    /* Enable A20 line */
    in $0x92, %al
    or $2, %al
    out %al, $0x92
    
    /* Enable protected mode */
    mov %cr0, %eax
    or $1, %eax
    mov %eax, %cr0
    
    /* Jump to 32-bit code */
    ljmp $0x08, $(protected_mode_start - smp_trampoline_start)

.code32
protected_mode_start:
    /* Set up 32-bit segments */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    /* Set up temporary stack */
    mov $(stack_top - smp_trampoline_start), %esp
    
    /* Enable paging */
    mov kernel_page_dir - smp_trampoline_start, %eax
    mov %eax, %cr3
    
    mov %cr0, %eax
    or $0x80000000, %eax    /* Enable paging bit */
    mov %eax, %cr0
    
    /* Jump to high memory */
    mov smp_trampoline_target - smp_trampoline_start, %eax
    jmp *%eax

/* GDT for trampoline */
.align 8
gdt_start:
    /* Null descriptor */
    .quad 0x0000000000000000
    
    /* Code segment (32-bit) */
    .quad 0x00CF9A000000FFFF
    
    /* Data segment (32-bit) */
    .quad 0x00CF92000000FFFF

gdt_end:

gdt_descriptor:
    .word gdt_end - gdt_start - 1
    .long gdt_start - smp_trampoline_start

/* Variables filled in by boot CPU */
.align 4
smp_trampoline_target:
    .long 0

kernel_page_dir:
    .long 0

/* Temporary stack for APs */
.align 16
stack_bottom:
    .space 1024
stack_top:

smp_trampoline_end: