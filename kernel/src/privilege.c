/**
 * Privilege Separation Implementation for LimitlessOS
 * 
 * Implements Ring 0/3 separation, syscall interface, and user process management.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "privilege.h"
#include "mm/mm.h"
#include "smp.h"
#include "kernel.h"
#include <string.h>

/* Global Descriptor Table */
gdt_entry_t gdt[6];
gdt_ptr_t gdt_ptr;

/* Task State Segment */
tss_entry_t tss;

/* System call table */
syscall_handler_t syscall_table[MAX_SYSCALLS];

/* Current process pointer */
user_process_t *current_process = NULL;

/* Process list */
static user_process_t *process_list = NULL;
static spinlock_t process_list_lock = SPINLOCK_INIT;
static pid_t next_pid = 100;  /* Start user PIDs at 100 */

/**
 * Initialize privilege separation system
 */
int privilege_init(void) {
    kprintf("[PRIV] Initializing privilege separation...\n");
    
    /* Set up GDT with kernel and user segments */
    setup_gdt();
    load_gdt();
    
    /* Set up TSS for privilege switching */
    setup_tss();
    load_tss();
    
    /* Initialize system call table */
    syscall_init();
    
    kprintf("[PRIV] Privilege separation initialized\n");
    kprintf("[PRIV] Kernel segments: CS=0x%02X DS=0x%02X\n", KERNEL_CS, KERNEL_DS);
    kprintf("[PRIV] User segments: CS=0x%02X DS=0x%02X\n", USER_CS, USER_DS);
    
    return 0;
}

/**
 * Set up Global Descriptor Table
 */
void setup_gdt(void) {
    /* Null descriptor */
    memset(&gdt[0], 0, sizeof(gdt_entry_t));
    
    /* Kernel code segment (0x08) - Ring 0 */
    gdt[1].limit_low = 0xFFFF;
    gdt[1].base_low = 0;
    gdt[1].base_middle = 0;
    gdt[1].access = 0x9A;       /* Present, Ring 0, Code, Executable, Readable */
    gdt[1].granularity = 0xCF;  /* 4KB granularity, 32-bit */
    gdt[1].base_high = 0;
    
    /* Kernel data segment (0x10) - Ring 0 */
    gdt[2].limit_low = 0xFFFF;
    gdt[2].base_low = 0;
    gdt[2].base_middle = 0;
    gdt[2].access = 0x92;       /* Present, Ring 0, Data, Writable */
    gdt[2].granularity = 0xCF;  /* 4KB granularity, 32-bit */
    gdt[2].base_high = 0;
    
    /* User code segment (0x18) - Ring 3 */
    gdt[3].limit_low = 0xFFFF;
    gdt[3].base_low = 0;
    gdt[3].base_middle = 0;
    gdt[3].access = 0xFA;       /* Present, Ring 3, Code, Executable, Readable */
    gdt[3].granularity = 0xCF;  /* 4KB granularity, 32-bit */
    gdt[3].base_high = 0;
    
    /* User data segment (0x20) - Ring 3 */
    gdt[4].limit_low = 0xFFFF;
    gdt[4].base_low = 0;
    gdt[4].base_middle = 0;
    gdt[4].access = 0xF2;       /* Present, Ring 3, Data, Writable */
    gdt[4].granularity = 0xCF;  /* 4KB granularity, 32-bit */
    gdt[4].base_high = 0;
    
    /* TSS segment (0x28) */
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss_entry_t) - 1;
    
    gdt[5].limit_low = limit & 0xFFFF;
    gdt[5].base_low = base & 0xFFFF;
    gdt[5].base_middle = (base >> 16) & 0xFF;
    gdt[5].access = 0x89;       /* Present, Ring 0, TSS */
    gdt[5].granularity = 0x40 | ((limit >> 16) & 0x0F); /* Byte granularity */
    gdt[5].base_high = (base >> 24) & 0xFF;
    
    /* Set up GDT pointer */
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint32_t)gdt;
}

/**
 * Load Global Descriptor Table
 */
void load_gdt(void) {
    asm volatile("lgdt %0" : : "m"(gdt_ptr));
    
    /* Reload segment registers */
    asm volatile(
        "ljmp $0x08, $1f\n"
        "1:\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss"
        :
        :
        : "eax"
    );
}

/**
 * Set up Task State Segment
 */
void setup_tss(void) {
    memset(&tss, 0, sizeof(tss_entry_t));
    
    /* Set up kernel stack segment */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = 0;  /* Will be set when switching to user mode */
    
    /* Set I/O map base to end of TSS (no I/O bitmap) */
    tss.iomap_base = sizeof(tss_entry_t);
}

/**
 * Load Task State Segment
 */
void load_tss(void) {
    asm volatile("ltr %%ax" : : "a"(TSS_SELECTOR));
}

/**
 * Set kernel stack pointer in TSS
 */
void set_kernel_stack(void *stack_ptr) {
    tss.esp0 = (uint32_t)stack_ptr;
}

/**
 * Initialize system call table
 */
void syscall_init(void) {
    /* Clear system call table */
    memset(syscall_table, 0, sizeof(syscall_table));
    
    /* Register basic system calls */
    register_syscall(SYS_EXIT, (syscall_handler_t)sys_exit);
    register_syscall(SYS_FORK, (syscall_handler_t)sys_fork);
    register_syscall(SYS_READ, (syscall_handler_t)sys_read);
    register_syscall(SYS_WRITE, (syscall_handler_t)sys_write);
    register_syscall(SYS_OPEN, (syscall_handler_t)sys_open);
    register_syscall(SYS_CLOSE, (syscall_handler_t)sys_close);
    register_syscall(SYS_EXECVE, (syscall_handler_t)sys_execve);
    register_syscall(SYS_GETPID, (syscall_handler_t)sys_getpid);
    register_syscall(SYS_GETPPID, (syscall_handler_t)sys_getppid);
    register_syscall(SYS_GETUID, (syscall_handler_t)sys_getuid);
    register_syscall(SYS_GETGID, (syscall_handler_t)sys_getgid);
    register_syscall(SYS_SETUID, (syscall_handler_t)sys_setuid);
    register_syscall(SYS_SETGID, (syscall_handler_t)sys_setgid);
    register_syscall(SYS_BRK, (syscall_handler_t)sys_brk);
    register_syscall(SYS_MMAP, (syscall_handler_t)sys_mmap);
    register_syscall(SYS_MUNMAP, (syscall_handler_t)sys_munmap);
    register_syscall(SYS_KILL, (syscall_handler_t)sys_kill);
    register_syscall(SYS_SIGNAL, (syscall_handler_t)sys_signal);
    
    kprintf("[PRIV] System call table initialized\n");
}

/**
 * Register system call handler
 */
void register_syscall(int syscall_num, syscall_handler_t handler) {
    if (syscall_num >= 0 && syscall_num < MAX_SYSCALLS) {
        syscall_table[syscall_num] = handler;
    }
}

/**
 * System call handler (called from interrupt)
 */
int syscall_handler(interrupt_frame_t *frame) {
    /* Get system call number from EAX */
    uint32_t syscall_num;
    asm volatile("movl %%eax, %0" : "=m"(syscall_num));
    
    /* Get arguments from registers */
    uint32_t arg1, arg2, arg3, arg4, arg5, arg6;
    asm volatile(
        "movl %%ebx, %0\n"
        "movl %%ecx, %1\n"
        "movl %%edx, %2\n"
        "movl %%esi, %3\n"
        "movl %%edi, %4\n"
        "movl %%ebp, %5"
        : "=m"(arg1), "=m"(arg2), "=m"(arg3), "=m"(arg4), "=m"(arg5), "=m"(arg6)
    );
    
    /* Validate system call number */
    if (syscall_num >= MAX_SYSCALLS || !syscall_table[syscall_num]) {
        kprintf("[PRIV] Invalid system call: %u\n", syscall_num);
        return -ENOSYS;
    }
    
    /* Check permissions */
    if (!check_syscall_permission(syscall_num, current_process)) {
        kprintf("[PRIV] Permission denied for syscall %u\n", syscall_num);
        return -EPERM;
    }
    
    /* Mark process as in system call */
    if (current_process) {
        current_process->in_syscall = true;
    }
    
    /* Call system call handler */
    int result = syscall_table[syscall_num](arg1, arg2, arg3, arg4, arg5, arg6);
    
    /* Clear in_syscall flag */
    if (current_process) {
        current_process->in_syscall = false;
    }
    
    /* Return result in EAX */
    asm volatile("movl %0, %%eax" : : "m"(result));
    
    return result;
}

/**
 * Create user process
 */
user_process_t *create_user_process(void) {
    user_process_t *proc = (user_process_t*)kmalloc(sizeof(user_process_t));
    if (!proc) {
        return NULL;
    }
    
    /* Clear process structure */
    memset(proc, 0, sizeof(user_process_t));
    
    /* Allocate PID */
    spin_lock(&process_list_lock);
    proc->pid = next_pid++;
    spin_unlock(&process_list_lock);
    
    /* Set default IDs */
    proc->ppid = (current_process) ? current_process->pid : 1;
    proc->uid = 1000;   /* Default user ID */
    proc->gid = 1000;   /* Default group ID */
    proc->euid = proc->uid;
    proc->egid = proc->gid;
    
    /* Set up default file descriptors */
    proc->files[STDIN_FILENO] = NULL;   /* TODO: Connect to terminal */
    proc->files[STDOUT_FILENO] = NULL;  /* TODO: Connect to terminal */
    proc->files[STDERR_FILENO] = NULL;  /* TODO: Connect to terminal */
    proc->next_fd = 3;
    
    /* Set default working directory */
    proc->cwd = kmalloc(2);
    if (proc->cwd) {
        strcpy(proc->cwd, "/");
        proc->cwd_len = 1;
    }
    
    /* Initialize resource limits */
    for (int i = 0; i < 16; i++) {
        proc->rlimits[i].rlim_cur = 0xFFFFFFFFUL;  /* No limit by default */
        proc->rlimits[i].rlim_max = 0xFFFFFFFFUL;
    }
    
    /* Set creation time */
    proc->start_time = get_ticks();
    
    /* Add to process list */
    spin_lock(&process_list_lock);
    proc->next = process_list;
    process_list = proc;
    spin_unlock(&process_list_lock);
    
    kprintf("[PRIV] Created user process PID %d\n", proc->pid);
    
    return proc;
}

/**
 * Set up user memory layout
 */
int setup_user_memory(user_process_t *proc, size_t code_size, size_t data_size, size_t stack_size) {
    if (!proc) return -EINVAL;
    
    /* Create user page directory */
    proc->page_directory = pmm_alloc_page();
    if (!proc->page_directory) {
        return -ENOMEM;
    }
    
    /* Clear page directory */
    memset(proc->page_directory, 0, PAGE_SIZE);
    
    /* Set up memory layout */
    proc->code_start = (void*)USER_SPACE_START;
    proc->code_end = (void*)((uintptr_t)proc->code_start + code_size);
    proc->data_start = proc->code_end;
    proc->data_end = (void*)((uintptr_t)proc->data_start + data_size);
    proc->heap_start = proc->data_end;
    proc->heap_end = proc->heap_start;
    proc->stack_end = (void*)USER_STACK_TOP;
    proc->stack_start = (void*)((uintptr_t)proc->stack_end - stack_size);
    
    /* Allocate and map code pages */
    size_t code_pages = (code_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t i = 0; i < code_pages; i++) {
        void *phys_page = pmm_alloc_page();
        if (!phys_page) {
            return -ENOMEM;
        }
        
        uintptr_t virt_addr = (uintptr_t)proc->code_start + i * PAGE_SIZE;
        /* TODO: Map page with PROT_READ | PROT_EXEC permissions */
        proc->resident_pages++;
    }
    
    /* Allocate and map data pages */
    size_t data_pages = (data_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t i = 0; i < data_pages; i++) {
        void *phys_page = pmm_alloc_page();
        if (!phys_page) {
            return -ENOMEM;
        }
        
        uintptr_t virt_addr = (uintptr_t)proc->data_start + i * PAGE_SIZE;
        /* TODO: Map page with PROT_READ | PROT_WRITE permissions */
        proc->resident_pages++;
    }
    
    /* Allocate and map stack pages */
    size_t stack_pages = (stack_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t i = 0; i < stack_pages; i++) {
        void *phys_page = pmm_alloc_page();
        if (!phys_page) {
            return -ENOMEM;
        }
        
        uintptr_t virt_addr = (uintptr_t)proc->stack_start + i * PAGE_SIZE;
        /* TODO: Map page with PROT_READ | PROT_WRITE permissions */
        proc->resident_pages++;
    }
    
    proc->virtual_size = code_size + data_size + stack_size;
    
    kprintf("[PRIV] Set up user memory for PID %d: %zu KB\n", 
            proc->pid, proc->virtual_size / 1024);
    
    return 0;
}

/**
 * Load user program from ELF
 */
int load_user_program(user_process_t *proc, const void *program, size_t size) {
    if (!proc || !program || size < sizeof(elf_header_t)) {
        return -EINVAL;
    }
    
    const elf_header_t *elf = (const elf_header_t*)program;
    
    /* Validate ELF header */
    if (!validate_elf_header(elf)) {
        return -ENOEXEC;
    }
    
    /* Load program segments */
    const elf_program_header_t *phdr = (const elf_program_header_t*)
        ((uintptr_t)program + elf->e_phoff);
    
    size_t max_addr = 0;
    size_t min_addr = 0xFFFFFFFF;
    
    for (int i = 0; i < elf->e_phnum; i++) {
        if (phdr[i].p_type == 1) {  /* PT_LOAD */
            if (phdr[i].p_vaddr < min_addr) {
                min_addr = phdr[i].p_vaddr;
            }
            if (phdr[i].p_vaddr + phdr[i].p_memsz > max_addr) {
                max_addr = phdr[i].p_vaddr + phdr[i].p_memsz;
            }
        }
    }
    
    /* Set up memory layout */
    size_t program_size = max_addr - min_addr;
    int result = setup_user_memory(proc, program_size, 4096, USER_STACK_SIZE);
    if (result != 0) {
        return result;
    }
    
    /* Load segments */
    for (int i = 0; i < elf->e_phnum; i++) {
        if (phdr[i].p_type == 1) {  /* PT_LOAD */
            const void *src = (const void*)((uintptr_t)program + phdr[i].p_offset);
            void *dst = (void*)phdr[i].p_vaddr;
            
            /* Copy file data */
            memcpy(dst, src, phdr[i].p_filesz);
            
            /* Zero BSS section */
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                memset((void*)((uintptr_t)dst + phdr[i].p_filesz), 0, 
                      phdr[i].p_memsz - phdr[i].p_filesz);
            }
        }
    }
    
    /* Set entry point */
    proc->context.eip = elf->e_entry;
    proc->context.esp = (uint32_t)proc->stack_end - 16;  /* Leave some space */
    proc->context.cs = USER_CS;
    proc->context.user_ss = USER_DS;
    proc->context.eflags = 0x202;  /* Interrupts enabled */
    
    kprintf("[PRIV] Loaded ELF program for PID %d, entry: 0x%08X\n", 
            proc->pid, proc->context.eip);
    
    return 0;
}

/**
 * Validate ELF header
 */
bool validate_elf_header(const elf_header_t *header) {
    /* Check ELF magic */
    if (header->e_ident[0] != 0x7F || 
        header->e_ident[1] != 'E' || 
        header->e_ident[2] != 'L' || 
        header->e_ident[3] != 'F') {
        return false;
    }
    
    /* Check for 32-bit ELF */
    if (header->e_ident[4] != 1) {  /* EI_CLASS = ELFCLASS32 */
        return false;
    }
    
    /* Check for little-endian */
    if (header->e_ident[5] != 1) {  /* EI_DATA = ELFDATA2LSB */
        return false;
    }
    
    /* Check machine type (i386) */
    if (header->e_machine != 3) {
        return false;
    }
    
    /* Check type (executable) */
    if (header->e_type != 2) {
        return false;
    }
    
    return true;
}

/**
 * Enter user mode
 */
void enter_user_mode(user_process_t *proc) {
    if (!proc) return;
    
    current_process = proc;
    
    /* Set up kernel stack */
    void *kernel_stack = kmalloc(PAGE_SIZE);
    if (kernel_stack) {
        set_kernel_stack((void*)((uintptr_t)kernel_stack + PAGE_SIZE - 16));
    }
    
    /* Switch to user page directory */
    if (proc->page_directory) {
        asm volatile("mov %0, %%cr3" : : "r"(proc->page_directory));
    }
    
    /* Switch to user mode */
    switch_to_user_mode(proc->context.eip, proc->context.esp);
}

/**
 * Switch to user mode (assembly implementation needed)
 */
void switch_to_user_mode(uint32_t entry_point, uint32_t user_stack) {
    /* This needs to be implemented in assembly to properly set up
     * the interrupt frame for returning to user mode */
    
    asm volatile(
        "cli\n"                     /* Disable interrupts */
        "mov $0x23, %%ax\n"         /* User data segment with RPL 3 */
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        
        "pushl $0x23\n"             /* User data segment (SS) */
        "pushl %1\n"                /* User stack pointer (ESP) */
        "pushfl\n"                  /* EFLAGS */
        "pushl $0x1B\n"             /* User code segment with RPL 3 (CS) */
        "pushl %0\n"                /* Entry point (EIP) */
        "iret"                      /* Return to user mode */
        :
        : "r"(entry_point), "r"(user_stack)
        : "eax"
    );
}

/**
 * Validate user pointer
 */
bool validate_user_pointer(const void *ptr, size_t size) {
    uintptr_t addr = (uintptr_t)ptr;
    
    /* Check if pointer is in user space */
    if (addr < USER_SPACE_START || addr >= USER_SPACE_END) {
        return false;
    }
    
    /* Check if range extends beyond user space */
    if (addr + size > USER_SPACE_END) {
        return false;
    }
    
    /* TODO: Check if pages are mapped and accessible */
    
    return true;
}

/**
 * Copy from user space
 */
int copy_from_user(void *dst, const void *src, size_t size) {
    if (!validate_user_pointer(src, size)) {
        return -EFAULT;
    }
    
    /* TODO: Use safe copy with page fault handling */
    memcpy(dst, src, size);
    return 0;
}

/**
 * Copy to user space  
 */
int copy_to_user(void *dst, const void *src, size_t size) {
    if (!validate_user_pointer(dst, size)) {
        return -EFAULT;
    }
    
    /* TODO: Use safe copy with page fault handling */
    memcpy(dst, src, size);
    return 0;
}

/**
 * Check current privilege level
 */
bool is_kernel_mode(void) {
    uint16_t cs;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    return (cs & 3) == 0;
}

bool is_user_mode(void) {
    return !is_kernel_mode();
}

int get_current_privilege_level(void) {
    uint16_t cs;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    return cs & 3;
}

/**
 * Check system call permission
 */
bool check_syscall_permission(int syscall_num, user_process_t *proc) {
    /* TODO: Implement proper permission checking based on:
     * - Process capabilities
     * - Security context
     * - Resource limits
     * - System call restrictions
     */
    
    /* For now, allow all system calls */
    return true;
}

/* Basic system call implementations */

int sys_exit(int exit_code) {
    if (current_process) {
        kprintf("[PRIV] Process %d exiting with code %d\n", 
                current_process->pid, exit_code);
        current_process->exit_code = exit_code;
        /* TODO: Implement proper process cleanup */
    }
    return 0;
}

int sys_getpid(void) {
    return current_process ? current_process->pid : 0;
}

int sys_getppid(void) {
    return current_process ? current_process->ppid : 0;
}

int sys_getuid(void) {
    return current_process ? current_process->uid : 0;
}

int sys_getgid(void) {
    return current_process ? current_process->gid : 0;
}

int sys_setuid(uid_t uid) {
    if (!current_process) return -ESRCH;
    
    /* TODO: Check permissions */
    current_process->uid = uid;
    current_process->euid = uid;
    return 0;
}

int sys_setgid(gid_t gid) {
    if (!current_process) return -ESRCH;
    
    /* TODO: Check permissions */
    current_process->gid = gid;
    current_process->egid = gid;
    return 0;
}

int sys_read(int fd, void *buf, size_t count) {
    if (!current_process || fd < 0 || fd >= 256) return -EBADF;
    
    /* Validate user buffer */
    if (!validate_user_pointer(buf, count)) return -EFAULT;
    
    /* TODO: Implement file I/O */
    return -ENOSYS;
}

int sys_write(int fd, const void *buf, size_t count) {
    if (!current_process || fd < 0 || fd >= 256) return -EBADF;
    
    /* Validate user buffer */
    if (!validate_user_pointer(buf, count)) return -EFAULT;
    
    /* Simple console output for stdout/stderr */
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        for (size_t i = 0; i < count; i++) {
            terminal_putchar(((const char*)buf)[i]);
        }
        return count;
    }
    
    /* TODO: Implement file I/O */
    return -ENOSYS;
}

int sys_open(const char *pathname, int flags, mode_t mode) {
    if (!current_process) return -ESRCH;
    
    /* Validate pathname */
    if (!validate_user_pointer(pathname, 1)) return -EFAULT;
    
    /* TODO: Implement file opening */
    return -ENOSYS;
}

int sys_close(int fd) {
    if (!current_process || fd < 0 || fd >= 256) return -EBADF;
    
    /* TODO: Implement file closing */
    return -ENOSYS;
}

int sys_fork(void) {
    if (!current_process) return -ESRCH;
    
    /* TODO: Implement process forking */
    return -ENOSYS;
}

int sys_execve(const char *filename, char *const argv[], char *const envp[]) {
    if (!current_process) return -ESRCH;
    
    /* Validate parameters */
    if (!validate_user_pointer(filename, 1)) return -EFAULT;
    
    /* TODO: Implement program execution */
    return -ENOSYS;
}

int sys_brk(void *addr) {
    if (!current_process) return -ESRCH;
    
    /* TODO: Implement heap management */
    return (int)current_process->heap_end;
}

int sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (!current_process) return -ESRCH;
    
    /* TODO: Implement memory mapping */
    return -ENOSYS;
}

int sys_munmap(void *addr, size_t length) {
    if (!current_process) return -ESRCH;
    
    /* TODO: Implement memory unmapping */
    return -ENOSYS;
}

int sys_kill(pid_t pid, int sig) {
    if (!current_process) return -ESRCH;
    
    /* TODO: Implement signal sending */
    return -ENOSYS;
}

int sys_signal(int signum, void *handler) {
    if (!current_process) return -ESRCH;
    
    /* TODO: Implement signal handler registration */
    return -ENOSYS;
}

/**
 * Debug functions
 */
void dump_user_context(const user_context_t *ctx) {
    kprintf("[PRIV] User Context:\n");
    kprintf("  EIP: 0x%08X  ESP: 0x%08X  EBP: 0x%08X\n", 
            ctx->eip, ctx->esp, ctx->ebp);
    kprintf("  EAX: 0x%08X  EBX: 0x%08X  ECX: 0x%08X  EDX: 0x%08X\n",
            ctx->eax, ctx->ebx, ctx->ecx, ctx->edx);
    kprintf("  ESI: 0x%08X  EDI: 0x%08X  EFLAGS: 0x%08X\n",
            ctx->esi, ctx->edi, ctx->eflags);
    kprintf("  CS: 0x%04X  DS: 0x%04X  SS: 0x%04X\n",
            ctx->cs, ctx->ds, ctx->user_ss);
}

void show_privilege_state(void) {
    kprintf("[PRIV] Current privilege level: Ring %d\n", get_current_privilege_level());
    kprintf("[PRIV] Current process: %s\n", 
            current_process ? "User process" : "Kernel");
    if (current_process) {
        kprintf("[PRIV] PID: %d, UID: %d, GID: %d\n",
                current_process->pid, current_process->uid, current_process->gid);
    }
}