; LimitlessOS Simple Boot Entry
; Simple x86_64 boot entry for multiboot2

[BITS 32]

section .text

; Export symbols
global _start
extern kernel_main

; Boot entry point - called by GRUB2
_start:
    ; Disable interrupts
    cli
    
    ; Set up stack
    mov esp, stack_top
    
    ; Save multiboot2 magic and info address
    ; EAX = magic, EBX = info address
    push ebx    ; multiboot2 info address
    push eax    ; multiboot2 magic
    
    ; Verify multiboot2 magic
    cmp eax, 0x36d76289
    jne .halt
    
    ; Call C kernel (parameters already on stack)
    call kernel_main
    
.halt:
    ; If we return or invalid magic, halt
    cli
    hlt
    jmp .halt

; Kernel stack (16KB)
section .bss
align 16
stack_bottom:
    resb 16384
stack_top: