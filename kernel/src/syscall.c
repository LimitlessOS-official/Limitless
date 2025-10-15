/*
 * System Call Handler
 * 
 * This module implements the system call dispatcher for LimitlessOS.
 * It provides the bridge between userspace applications and kernel services.
 * 
 * System calls use interrupt 0x80 with parameters passed in registers:
 * - EAX: System call number
 * - EBX, ECX, EDX, ESI, EDI: Arguments 1-5
 * - Return value in EAX
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <syscall.h>
#include <isr.h>
#include <common.h>
#include <drivers/vga_text.h>

// System call numbers (must match userspace/libc/include/syscall.h)
#define SYS_WRITE 1
#define SYS_READ  2
#define SYS_OPEN  3
#define SYS_CLOSE 4
#define SYS_SBRK  5
#define SYS_EXIT  6
#define SYS_FORK  7
#define SYS_EXEC  8
#define SYS_WAIT  9
#define SYS_GETPID 10

// Maximum number of system calls
#define SYSCALL_MAX 256

// Array of system call handlers
typedef int (*syscall_fn_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
static syscall_fn_t syscall_table[SYSCALL_MAX];

/**
 * sys_write - Write data to a file descriptor
 * 
 * @param fd File descriptor (0=stdin, 1=stdout, 2=stderr)
 * @param buf Pointer to the data buffer
 * @param count Number of bytes to write
 * @return Number of bytes written, or -1 on error
 */
static int sys_write(uint32_t fd, uint32_t buf, uint32_t count, uint32_t unused1, uint32_t unused2)
{
    // For now, we only support writing to stdout (fd=1)
    if (fd != 1) {
        return -1;  // Error: invalid file descriptor
    }
    
    const char *str = (const char *)buf;
    for (uint32_t i = 0; i < count; i++) {
        vga_text_putchar(str[i]);
    }
    
    return (int)count;
}

/**
 * sys_read - Read data from a file descriptor
 * 
 * @param fd File descriptor
 * @param buf Buffer to store the data
 * @param count Maximum number of bytes to read
 * @return Number of bytes read, or -1 on error
 */
static int sys_read(uint32_t fd, uint32_t buf, uint32_t count, uint32_t unused1, uint32_t unused2)
{
    // TODO: Implement keyboard input
    return -1;  // Not implemented yet
}

/**
 * sys_sbrk - Change the program break (for malloc)
 * 
 * @param increment Number of bytes to increase the break by
 * @return Previous break value, or -1 on error
 */
static int sys_sbrk(uint32_t increment, uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4)
{
    // TODO: Implement proper heap management
    // For now, return an error
    return -1;
}

/**
 * sys_exit - Terminate the current process
 * 
 * @param status Exit status code
 */
static int sys_exit(uint32_t status, uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4)
{
    // TODO: Implement process termination
    vga_text_writestring("\nProcess exited with status: ");
    // TODO: Print status value
    for(;;) asm("hlt");
    return 0;
}

/**
 * Initialize the system call subsystem
 * 
 * This function sets up the system call table and registers the
 * system call handler with the interrupt subsystem.
 */
void syscalls_init()
{
    // Clear the system call table
    for (int i = 0; i < SYSCALL_MAX; i++) {
        syscall_table[i] = NULL;
    }
    
    // Register system calls
    syscall_table[SYS_WRITE] = sys_write;
    syscall_table[SYS_READ]  = sys_read;
    syscall_table[SYS_SBRK]  = sys_sbrk;
    syscall_table[SYS_EXIT]  = sys_exit;
    
    // Register the syscall handler for interrupt 0x80 (128)
    register_interrupt_handler(128, syscall_handler);
}

/**
 * Main system call handler
 * 
 * This function is called from the ISR when interrupt 0x80 is triggered.
 * It dispatches to the appropriate system call based on the value in EAX.
 */
void syscall_handler(registers_t *regs)
{
    // Get the system call number from EAX
    uint32_t syscall_num = regs->eax;
    
    // Check if the system call number is valid
    if (syscall_num >= SYSCALL_MAX || syscall_table[syscall_num] == NULL) {
        // Invalid system call
        regs->eax = -1;
        return;
    }
    
    // Get the function pointer for this system call
    syscall_fn_t handler = syscall_table[syscall_num];
    
    // Call the system call handler with the arguments from the registers
    // Arguments are passed in: EBX, ECX, EDX, ESI, EDI
    int result = handler(regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
    
    // Store the return value in EAX
    regs->eax = result;
}
