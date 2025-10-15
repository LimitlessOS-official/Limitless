/**
 * Kernel/Userspace Separation for LimitlessOS
 * 
 * Implements Ring 0/3 privilege separation with syscall interface,
 * user mode processes, memory protection, and security controls.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/* CPU privilege levels */
#define KERNEL_MODE     0   /* Ring 0 - Kernel mode */
#define USER_MODE       3   /* Ring 3 - User mode */

/* Segment descriptors */
#define KERNEL_CS       0x08    /* Kernel code segment */
#define KERNEL_DS       0x10    /* Kernel data segment */
#define USER_CS         0x18    /* User code segment */
#define USER_DS         0x20    /* User data segment */
#define TSS_SELECTOR    0x28    /* Task State Segment */

/* GDT entry structure */
typedef struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

/* GDT pointer structure */
typedef struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

/* Task State Segment (TSS) */
typedef struct tss_entry {
    uint32_t prev_tss;      /* Previous TSS */
    uint32_t esp0;          /* Kernel stack pointer */
    uint32_t ss0;           /* Kernel stack segment */
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;           /* Page directory */
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

/* Interrupt frame for syscalls */
typedef struct interrupt_frame {
    /* Pushed by CPU during interrupt */
    uint32_t error_code;    /* Error code (if applicable) */
    uint32_t eip;           /* Instruction pointer */
    uint32_t cs;            /* Code segment */
    uint32_t eflags;        /* CPU flags */
    uint32_t esp;           /* Stack pointer (if privilege change) */
    uint32_t ss;            /* Stack segment (if privilege change) */
} __attribute__((packed)) interrupt_frame_t;

/* CPU context for user processes */
typedef struct user_context {
    /* General purpose registers */
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
    
    /* Segment registers */
    uint32_t ds, es, fs, gs;
    
    /* Instruction pointer and flags */
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    
    /* User stack (only saved on privilege change) */
    uint32_t user_esp;
    uint32_t user_ss;
} __attribute__((packed)) user_context_t;

/* System call numbers */
#define SYS_EXIT        1
#define SYS_FORK        2
#define SYS_READ        3
#define SYS_WRITE       4
#define SYS_OPEN        5
#define SYS_CLOSE       6
#define SYS_WAITPID     7
#define SYS_CREAT       8
#define SYS_LINK        9
#define SYS_UNLINK      10
#define SYS_EXECVE      11
#define SYS_CHDIR       12
#define SYS_TIME        13
#define SYS_MKNOD       14
#define SYS_CHMOD       15
#define SYS_LCHOWN      16
#define SYS_BREAK       17
#define SYS_OLDSTAT     18
#define SYS_LSEEK       19
#define SYS_GETPID      20
#define SYS_MOUNT       21
#define SYS_UMOUNT      22
#define SYS_SETUID      23
#define SYS_GETUID      24
#define SYS_STIME       25
#define SYS_PTRACE      26
#define SYS_ALARM       27
#define SYS_OLDFSTAT    28
#define SYS_PAUSE       29
#define SYS_UTIME       30
#define SYS_STTY        31
#define SYS_GTTY        32
#define SYS_ACCESS      33
#define SYS_NICE        34
#define SYS_FTIME       35
#define SYS_SYNC        36
#define SYS_KILL        37
#define SYS_RENAME      38
#define SYS_MKDIR       39
#define SYS_RMDIR       40
#define SYS_DUP         41
#define SYS_PIPE        42
#define SYS_TIMES       43
#define SYS_PROF        44
#define SYS_BRK         45
#define SYS_SETGID      46
#define SYS_GETGID      47
#define SYS_SIGNAL      48
#define SYS_GETEUID     49
#define SYS_GETEGID     50
#define SYS_ACCT        51
#define SYS_UMOUNT2     52
#define SYS_LOCK        53
#define SYS_IOCTL       54
#define SYS_FCNTL       55
#define SYS_MPX         56
#define SYS_SETPGID     57
#define SYS_ULIMIT      58
#define SYS_OLDOLDUNAME 59
#define SYS_UMASK       60
#define SYS_CHROOT      61
#define SYS_USTAT       62
#define SYS_DUP2        63
#define SYS_GETPPID     64
#define SYS_GETPGRP     65
#define SYS_SETSID      66
#define SYS_SIGACTION   67
#define SYS_SGETMASK    68
#define SYS_SSETMASK    69
#define SYS_SETREUID    70
#define SYS_SETREGID    71
#define SYS_SIGSUSPEND  72
#define SYS_SIGPENDING  73
#define SYS_SETHOSTNAME 74
#define SYS_SETRLIMIT   75
#define SYS_GETRLIMIT   76
#define SYS_GETRUSAGE   77
#define SYS_GETTIMEOFDAY 78
#define SYS_SETTIMEOFDAY 79
#define SYS_GETGROUPS   80
#define SYS_SETGROUPS   81
#define SYS_SELECT      82
#define SYS_SYMLINK     83
#define SYS_OLDLSTAT    84
#define SYS_READLINK    85
#define SYS_USELIB      86
#define SYS_SWAPON      87
#define SYS_REBOOT      88
#define SYS_READDIR     89
#define SYS_MMAP        90
#define SYS_MUNMAP      91
#define SYS_TRUNCATE    92
#define SYS_FTRUNCATE   93
#define SYS_FCHMOD      94
#define SYS_FCHOWN      95
#define SYS_GETPRIORITY 96
#define SYS_SETPRIORITY 97
#define SYS_PROFIL      98
#define SYS_STATFS      99
#define SYS_FSTATFS     100

/* Maximum number of system calls */
#define MAX_SYSCALLS    400

/* User process structure */
typedef struct user_process {
    /* Process identification */
    pid_t pid;
    pid_t ppid;
    uid_t uid;
    gid_t gid;
    uid_t euid;
    gid_t egid;
    
    /* Memory layout */
    void *code_start;       /* Start of code segment */
    void *code_end;         /* End of code segment */
    void *data_start;       /* Start of data segment */
    void *data_end;         /* End of data segment */
    void *heap_start;       /* Start of heap */
    void *heap_end;         /* Current heap end */
    void *stack_start;      /* Start of user stack */
    void *stack_end;        /* End of user stack */
    
    /* Memory management */
    void *page_directory;   /* User page directory */
    size_t resident_pages;  /* Number of resident pages */
    size_t virtual_size;    /* Virtual memory size */
    
    /* File descriptors */
    struct file *files[256]; /* File descriptor table */
    int next_fd;            /* Next available file descriptor */
    
    /* Working directory */
    char *cwd;              /* Current working directory */
    size_t cwd_len;         /* Length of CWD path */
    
    /* Signal handling */
    void *signal_handlers[64]; /* Signal handler table */
    uint64_t signal_mask;   /* Signal mask */
    uint64_t pending_signals; /* Pending signals */
    
    /* Security context */
    uint32_t security_context; /* Security context ID */
    uint32_t capabilities;  /* Process capabilities */
    
    /* Resource limits */
    struct rlimit {
        uint64_t rlim_cur;  /* Current limit */
        uint64_t rlim_max;  /* Maximum limit */
    } rlimits[16];
    
    /* Statistics */
    uint64_t start_time;    /* Process start time */
    uint64_t utime;         /* User CPU time */
    uint64_t stime;         /* System CPU time */
    uint64_t cutime;        /* Children user time */
    uint64_t cstime;        /* Children system time */
    
    /* Execution state */
    user_context_t context; /* Saved user context */
    int exit_code;          /* Exit code */
    bool in_syscall;        /* Currently in system call */
    
} user_process_t;

/* System call handler function pointer */
typedef int (*syscall_handler_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3, 
                                uint32_t arg4, uint32_t arg5, uint32_t arg6);

/* Global state */
extern gdt_entry_t gdt[6];
extern gdt_ptr_t gdt_ptr;
extern tss_entry_t tss;
extern syscall_handler_t syscall_table[MAX_SYSCALLS];

/* GDT and privilege initialization */
int privilege_init(void);
void setup_gdt(void);
void setup_tss(void);
void load_gdt(void);
void load_tss(void);

/* User process management */
user_process_t *create_user_process(void);
void destroy_user_process(user_process_t *proc);
int load_user_program(user_process_t *proc, const void *program, size_t size);
void enter_user_mode(user_process_t *proc);

/* System call interface */
void syscall_init(void);
void register_syscall(int syscall_num, syscall_handler_t handler);
int syscall_handler(interrupt_frame_t *frame);

/* Memory protection */
int setup_user_memory(user_process_t *proc, size_t code_size, size_t data_size, 
                     size_t stack_size);
bool validate_user_pointer(const void *ptr, size_t size);
bool validate_user_string(const char *str, size_t max_len);
int copy_from_user(void *dst, const void *src, size_t size);
int copy_to_user(void *dst, const void *src, size_t size);

/* Context switching */
void save_user_context(user_context_t *ctx, interrupt_frame_t *frame);
void restore_user_context(const user_context_t *ctx, interrupt_frame_t *frame);
void switch_to_user_mode(uint32_t entry_point, uint32_t user_stack);
void switch_to_kernel_mode(void);

/* Privilege level management */
bool is_kernel_mode(void);
bool is_user_mode(void);
int get_current_privilege_level(void);
void set_kernel_stack(void *stack_ptr);

/* Security and validation */
bool check_user_access(const void *addr, size_t size, int prot);
bool check_syscall_permission(int syscall_num, user_process_t *proc);
int validate_syscall_args(int syscall_num, uint32_t *args);

/* ELF loader support */
typedef struct elf_header {
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) elf_header_t;

typedef struct elf_program_header {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} __attribute__((packed)) elf_program_header_t;

int load_elf_program(user_process_t *proc, const void *elf_data, size_t size);
bool validate_elf_header(const elf_header_t *header);

/* File descriptor management */
int allocate_fd(user_process_t *proc);
void free_fd(user_process_t *proc, int fd);
struct file *get_file(user_process_t *proc, int fd);
void set_file(user_process_t *proc, int fd, struct file *file);

/* Signal handling */
int send_signal(pid_t pid, int signal);
void handle_signals(user_process_t *proc);
void setup_signal_handler(user_process_t *proc, int signal, void *handler);

/* Resource management */
int set_rlimit(user_process_t *proc, int resource, const struct rlimit *rlim);
int get_rlimit(user_process_t *proc, int resource, struct rlimit *rlim);
bool check_resource_limit(user_process_t *proc, int resource, uint64_t usage);

/* System call implementations */
int sys_exit(int exit_code);
int sys_fork(void);
int sys_execve(const char *filename, char *const argv[], char *const envp[]);
int sys_getpid(void);
int sys_getppid(void);
int sys_getuid(void);
int sys_getgid(void);
int sys_setuid(uid_t uid);
int sys_setgid(gid_t gid);
int sys_read(int fd, void *buf, size_t count);
int sys_write(int fd, const void *buf, size_t count);
int sys_open(const char *pathname, int flags, mode_t mode);
int sys_close(int fd);
int sys_lseek(int fd, off_t offset, int whence);
int sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int sys_munmap(void *addr, size_t length);
int sys_brk(void *addr);
int sys_kill(pid_t pid, int sig);
int sys_signal(int signum, void *handler);

/* Process creation and execution */
user_process_t *fork_process(user_process_t *parent);
int exec_process(user_process_t *proc, const char *filename);
void exit_process(user_process_t *proc, int exit_code);

/* Debug and monitoring */
void dump_user_context(const user_context_t *ctx);
void dump_interrupt_frame(const interrupt_frame_t *frame);
void show_user_memory_map(user_process_t *proc);
void show_privilege_state(void);

/* Utility macros */
#define USER_SPACE_START    0x08048000  /* Standard user space start */
#define USER_SPACE_END      0xC0000000  /* End of user space */
#define USER_STACK_TOP      0xC0000000  /* Top of user stack */
#define USER_STACK_SIZE     0x00800000  /* 8MB default stack */

#define KERNEL_SPACE_START  0xC0000000  /* Kernel space start */
#define KERNEL_SPACE_END    0xFFFFFFFF  /* End of kernel space */

/* Protection flags */
#define PROT_NONE   0x0     /* No access */
#define PROT_READ   0x1     /* Read access */
#define PROT_WRITE  0x2     /* Write access */
#define PROT_EXEC   0x4     /* Execute access */

/* Memory mapping flags */
#define MAP_SHARED      0x01
#define MAP_PRIVATE     0x02
#define MAP_FIXED       0x10
#define MAP_ANONYMOUS   0x20

/* File descriptor flags */
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_CREAT     0x0100
#define O_EXCL      0x0200
#define O_TRUNC     0x1000
#define O_APPEND    0x2000

/* Standard file descriptors */
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

/* Error codes */
#define EPERM       1   /* Operation not permitted */
#define ENOENT      2   /* No such file or directory */
#define ESRCH       3   /* No such process */
#define EINTR       4   /* Interrupted system call */
#define EIO         5   /* I/O error */
#define ENXIO       6   /* No such device or address */
#define E2BIG       7   /* Argument list too long */
#define ENOEXEC     8   /* Exec format error */
#define EBADF       9   /* Bad file number */
#define ECHILD      10  /* No child processes */
#define EAGAIN      11  /* Try again */
#define ENOMEM      12  /* Out of memory */
#define EACCES      13  /* Permission denied */
#define EFAULT      14  /* Bad address */
#define EBUSY       16  /* Device or resource busy */
#define EEXIST      17  /* File exists */
#define ENODEV      19  /* No such device */
#define ENOTDIR     20  /* Not a directory */
#define EISDIR      21  /* Is a directory */
#define EINVAL      22  /* Invalid argument */
#define EMFILE      24  /* Too many open files */
#define ENOSPC      28  /* No space left on device */
#define EROFS       30  /* Read-only file system */

/* Current process pointer */
extern user_process_t *current_process;