/*
 * Interrupt Service Routine (ISR) Assembly Stubs
 * 
 * These assembly macros create the low-level interrupt handlers that save
 * the processor state and call the high-level C handlers.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

.section .text

# Function to load the IDT
.global idt_flush
idt_flush:
    mov 4(%esp), %eax
    lidt (%eax)
    ret

# This macro creates a stub for an ISR that does NOT push an error code
.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    cli
    push $0
    push $\num
    jmp isr_common_stub
.endm

# This macro creates a stub for an ISR that DOES push an error code
.macro ISR_ERRCODE num
.global isr\num
isr\num:
    cli
    push $\num
    jmp isr_common_stub
.endm

# This macro creates a stub for an IRQ
.macro IRQ num, irq_num
.global irq\num
irq\num:
    cli
    push $0
    push $\irq_num
    jmp irq_common_stub
.endm

# CPU Exception ISRs (0-31)
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

# IRQ handlers (32-47)
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

# System call handler (interrupt 128 = 0x80)
.global isr128
isr128:
    cli
    push $0
    push $128
    jmp isr_common_stub

# Common ISR stub - saves all registers and calls C handler
.global isr_common_stub
isr_common_stub:
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    push %esp
    call isr_handler
    add $4, %esp
    
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    
    add $8, %esp
    iret

# Common IRQ stub
.global irq_common_stub
irq_common_stub:
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    push %esp
    call irq_handler
    add $4, %esp
    
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    
    add $8, %esp
    iret
