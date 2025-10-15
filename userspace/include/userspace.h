/**
 * Userspace Environment Header for LimitlessOS
 * 
 * Complete userspace infrastructure including system libraries, shell environment,
 * IPC mechanisms, POSIX compliance, and compatibility layers for running
 * standard applications and enterprise software.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#ifndef __USERSPACE_H__
#define __USERSPACE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "security/security.h"

/* System Library Management */
typedef struct shared_library {
    char *name;                     /* Library name */
    char *path;                     /* Full path */
    uintptr_t base_addr;            /* Base load address */
    size_t size;                    /* Library size */
    
    /* ELF information */
    void *elf_header;               /* ELF header */
    void *program_headers;          /* Program headers */
    void *section_headers;          /* Section headers */
    uint32_t num_sections;          /* Number of sections */
    
    /* Symbol table */
    void *symtab;                   /* Symbol table */
    void *strtab;                   /* String table */
    uint32_t num_symbols;           /* Number of symbols */
    
    /* Dynamic linking */
    void *dynamic;                  /* Dynamic section */
    char **dependencies;            /* Library dependencies */
    uint32_t num_deps;              /* Number of dependencies */
    
    /* Reference counting */
    atomic_t refcount;              /* Reference count */
    
    struct shared_library *next;    /* Next in global list */
} shared_library_t;

/* Process Environment */
typedef struct process_env {
    /* Command line and environment */
    int argc;                       /* Argument count */
    char **argv;                    /* Argument vector */
    char **envp;                    /* Environment variables */
    uint32_t env_count;             /* Environment variable count */
    
    /* Working directory */
    char *cwd;                      /* Current working directory */
    char *root;                     /* Root directory (for chroot) */
    
    /* Signal handling */
    struct sigaction *sig_handlers[64]; /* Signal handlers */
    sigset_t sig_mask;              /* Signal mask */
    sigset_t sig_pending;           /* Pending signals */
    
    /* File descriptors */
    struct file **fd_table;         /* File descriptor table */
    uint32_t fd_max;                /* Maximum FD number */
    uint32_t fd_count;              /* Open file count */
    
    /* Memory layout */
    uintptr_t stack_start;          /* Stack start address */
    uintptr_t stack_end;            /* Stack end address */
    uintptr_t heap_start;           /* Heap start address */
    uintptr_t heap_end;             /* Heap end address */
    uintptr_t mmap_base;            /* mmap base address */
    
    /* Loaded libraries */
    shared_library_t *libraries;    /* Linked library list */
    
    /* Process limits */
    struct rlimit *limits;          /* Resource limits */
    
    spinlock_t lock;                /* Environment lock */
} process_env_t;

/* IPC Mechanisms */
typedef enum ipc_type {
    IPC_PIPE,
    IPC_FIFO,
    IPC_SOCKET,
    IPC_SYSV_MSG,
    IPC_SYSV_SEM,
    IPC_SYSV_SHM,
    IPC_POSIX_MSG,
    IPC_POSIX_SEM,
    IPC_POSIX_SHM
} ipc_type_t;

/* Pipe IPC */
typedef struct pipe_ipc {
    int read_fd;                    /* Read file descriptor */
    int write_fd;                   /* Write file descriptor */
    
    uint8_t *buffer;                /* Pipe buffer */
    size_t buffer_size;             /* Buffer size */
    size_t read_pos;                /* Read position */
    size_t write_pos;               /* Write position */
    size_t data_len;                /* Data length */
    
    spinlock_t lock;                /* Pipe lock */
    wait_queue_head_t read_wait;    /* Read wait queue */
    wait_queue_head_t write_wait;   /* Write wait queue */
} pipe_ipc_t;

/* System V Message Queue */
typedef struct sysv_msg_queue {
    key_t key;                      /* IPC key */
    int msgqid;                     /* Message queue ID */
    
    struct msg_msg *messages;       /* Message list */
    size_t msg_count;               /* Message count */
    size_t msg_bytes;               /* Total bytes */
    
    /* Permissions */
    uid_t uid;                      /* Owner UID */
    gid_t gid;                      /* Owner GID */
    mode_t mode;                    /* Permissions */
    
    /* Statistics */
    time_t msg_stime;               /* Last send time */
    time_t msg_rtime;               /* Last receive time */
    time_t msg_ctime;               /* Last change time */
    pid_t msg_lspid;                /* Last send PID */
    pid_t msg_lrpid;                /* Last receive PID */
    
    spinlock_t lock;                /* Queue lock */
    wait_queue_head_t send_wait;    /* Send wait queue */
    wait_queue_head_t recv_wait;    /* Receive wait queue */
} sysv_msg_queue_t;

/* POSIX Message Queue */
typedef struct posix_msg_queue {
    char *name;                     /* Queue name */
    mode_t mode;                    /* Permissions */
    
    struct mq_attr attr;            /* Queue attributes */
    struct posix_msg *messages;     /* Message list */
    
    uid_t uid;                      /* Owner UID */
    gid_t gid;                      /* Owner GID */
    
    atomic_t refcount;              /* Reference count */
    spinlock_t lock;                /* Queue lock */
    wait_queue_head_t send_wait;    /* Send wait queue */
    wait_queue_head_t recv_wait;    /* Receive wait queue */
} posix_msg_queue_t;

/* Shared Memory */
typedef struct shared_memory {
    key_t key;                      /* IPC key (System V) */
    char *name;                     /* Name (POSIX) */
    
    void *addr;                     /* Memory address */
    size_t size;                    /* Memory size */
    
    /* Permissions */
    uid_t uid;                      /* Owner UID */
    gid_t gid;                      /* Owner GID */
    mode_t mode;                    /* Permissions */
    
    /* Statistics */
    time_t shm_atime;               /* Last attach time */
    time_t shm_dtime;               /* Last detach time */
    time_t shm_ctime;               /* Last change time */
    pid_t shm_cpid;                 /* Creator PID */
    pid_t shm_lpid;                 /* Last operation PID */
    
    uint32_t shm_nattch;            /* Number of attachments */
    atomic_t refcount;              /* Reference count */
    
    spinlock_t lock;                /* Shared memory lock */
} shared_memory_t;

/* Semaphore */
typedef struct semaphore_ipc {
    key_t key;                      /* IPC key (System V) */
    char *name;                     /* Name (POSIX) */
    
    int value;                      /* Semaphore value */
    int max_value;                  /* Maximum value */
    
    /* Permissions */
    uid_t uid;                      /* Owner UID */
    gid_t gid;                      /* Owner GID */
    mode_t mode;                    /* Permissions */
    
    /* Statistics */
    time_t sem_otime;               /* Last operation time */
    time_t sem_ctime;               /* Last change time */
    
    atomic_t refcount;              /* Reference count */
    spinlock_t lock;                /* Semaphore lock */
    wait_queue_head_t wait_queue;   /* Wait queue */
} semaphore_ipc_t;

/* Signal Information */
typedef struct signal_info {
    int si_signo;                   /* Signal number */
    int si_errno;                   /* Error number */
    int si_code;                    /* Signal code */
    
    union {
        /* SIGCHLD */
        struct {
            pid_t si_pid;           /* Child PID */
            uid_t si_uid;           /* Child UID */
            int si_status;          /* Exit status */
        } sigchld;
        
        /* SIGILL, SIGFPE, SIGSEGV, SIGBUS */
        struct {
            void *si_addr;          /* Faulting address */
        } sigfault;
        
        /* SIGPOLL */
        struct {
            int si_band;            /* Band event */
            int si_fd;              /* File descriptor */
        } sigpoll;
        
        /* Generic */
        struct {
            pid_t si_pid;           /* Sending PID */
            uid_t si_uid;           /* Sending UID */
            union sigval si_value;  /* Signal value */
        } generic;
    } info;
} signal_info_t;

/* System Library Interface */
typedef struct libc_interface {
    /* Memory management */
    void *(*malloc)(size_t size);
    void (*free)(void *ptr);
    void *(*calloc)(size_t nmemb, size_t size);
    void *(*realloc)(void *ptr, size_t size);
    
    /* String functions */
    size_t (*strlen)(const char *s);
    char *(*strcpy)(char *dest, const char *src);
    char *(*strncpy)(char *dest, const char *src, size_t n);
    int (*strcmp)(const char *s1, const char *s2);
    int (*strncmp)(const char *s1, const char *s2, size_t n);
    
    /* I/O functions */
    int (*printf)(const char *format, ...);
    int (*fprintf)(FILE *stream, const char *format, ...);
    int (*sprintf)(char *str, const char *format, ...);
    int (*snprintf)(char *str, size_t size, const char *format, ...);
    
    /* File operations */
    FILE *(*fopen)(const char *pathname, const char *mode);
    int (*fclose)(FILE *stream);
    size_t (*fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
    size_t (*fwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream);
    
    /* System calls */
    long (*syscall)(long number, ...);
} libc_interface_t;

/* Shell Environment */
typedef struct shell_env {
    char *shell_path;               /* Shell executable path */
    char *prompt;                   /* Shell prompt */
    
    /* History */
    char **history;                 /* Command history */
    uint32_t history_size;          /* History size */
    uint32_t history_count;         /* History count */
    
    /* Variables */
    char **variables;               /* Shell variables */
    uint32_t var_count;             /* Variable count */
    
    /* Aliases */
    struct shell_alias {
        char *name;                 /* Alias name */
        char *value;                /* Alias value */
        struct shell_alias *next;   /* Next alias */
    } *aliases;
    
    /* Functions */
    struct shell_function {
        char *name;                 /* Function name */
        char *body;                 /* Function body */
        struct shell_function *next; /* Next function */
    } *functions;
    
    /* Job control */
    struct job {
        int job_id;                 /* Job ID */
        pid_t pgid;                 /* Process group ID */
        char *command;              /* Command line */
        bool background;            /* Background job */
        bool stopped;               /* Job stopped */
        struct job *next;           /* Next job */
    } *jobs;
    
    uint32_t next_job_id;           /* Next job ID */
    bool job_control;               /* Job control enabled */
    
    spinlock_t lock;                /* Shell lock */
} shell_env_t;

/* POSIX Compatibility */
typedef struct posix_compat {
    /* Thread support */
    bool pthread_support;           /* POSIX threads */
    
    /* Real-time extensions */
    bool realtime_support;          /* POSIX.1b real-time */
    
    /* Advanced I/O */
    bool aio_support;               /* Asynchronous I/O */
    
    /* Memory management */
    bool mlock_support;             /* Memory locking */
    bool mmap_support;              /* Memory mapping */
    
    /* Timers */
    bool timer_support;             /* POSIX timers */
    
    /* Message queues */
    bool mqueue_support;            /* POSIX message queues */
    
    /* Semaphores */
    bool semaphore_support;         /* POSIX semaphores */
    
    /* Shared memory */
    bool shm_support;               /* POSIX shared memory */
    
    /* Signals */
    bool sigqueue_support;          /* Signal queuing */
    bool sigwait_support;           /* Signal waiting */
    
    /* Process management */
    bool spawn_support;             /* Process spawning */
    
    /* File system */
    bool statvfs_support;           /* File system information */
    
    /* Feature test macros */
    int posix_version;              /* POSIX version */
    int xopen_version;              /* X/Open version */
} posix_compat_t;

/* Application Binary Interface */
typedef struct abi_compat {
    /* Linux compatibility */
    bool linux_syscalls;            /* Linux system calls */
    bool linux_procfs;              /* /proc filesystem */
    bool linux_sysfs;               /* /sys filesystem */
    
    /* Windows compatibility */
    bool win32_api;                 /* Win32 API emulation */
    bool wine_support;              /* Wine compatibility */
    
    /* BSD compatibility */
    bool bsd_sockets;               /* BSD socket API */
    bool bsd_kqueue;                /* kqueue/kevent */
    
    /* Emulation layers */
    void *linux_emulation;          /* Linux emulation state */
    void *win32_emulation;          /* Win32 emulation state */
    
    /* ABI version tracking */
    uint32_t abi_version;           /* ABI version */
    uint32_t compat_flags;          /* Compatibility flags */
} abi_compat_t;

/* Dynamic Loader State */
typedef struct dynamic_loader {
    /* Symbol resolution */
    void *global_symtab;            /* Global symbol table */
    uint32_t global_symcount;       /* Global symbol count */
    
    /* Library search paths */
    char **library_paths;           /* Library search paths */
    uint32_t path_count;            /* Path count */
    
    /* Loaded libraries */
    shared_library_t *loaded_libs;  /* Loaded library list */
    uint32_t lib_count;             /* Library count */
    
    /* Runtime linking */
    bool lazy_binding;              /* Lazy symbol binding */
    bool debug_mode;                /* Debug mode enabled */
    
    spinlock_t loader_lock;         /* Loader lock */
} dynamic_loader_t;

/* Global Userspace State */
typedef struct userspace_state {
    /* Process management */
    process_env_t **processes;      /* Process environment table */
    uint32_t max_processes;         /* Maximum processes */
    uint32_t process_count;         /* Active process count */
    spinlock_t process_lock;        /* Process table lock */
    
    /* IPC management */
    pipe_ipc_t **pipes;             /* Pipe table */
    sysv_msg_queue_t **msg_queues;  /* Message queue table */
    posix_msg_queue_t **posix_mqs;  /* POSIX message queues */
    shared_memory_t **shared_mem;   /* Shared memory table */
    semaphore_ipc_t **semaphores;   /* Semaphore table */
    
    uint32_t max_ipc_objects;       /* Maximum IPC objects */
    spinlock_t ipc_lock;            /* IPC lock */
    
    /* Dynamic loader */
    dynamic_loader_t loader;        /* Dynamic loader state */
    
    /* Shell environment */
    shell_env_t default_shell;      /* Default shell environment */
    
    /* System library interface */
    libc_interface_t libc;          /* libc interface */
    
    /* Compatibility layers */
    posix_compat_t posix;           /* POSIX compatibility */
    abi_compat_t abi;               /* ABI compatibility */
    
    /* Statistics */
    atomic_long_t syscall_count;    /* Total system calls */
    atomic_long_t ipc_operations;   /* IPC operations */
    atomic_long_t lib_loads;        /* Library loads */
    atomic_long_t process_spawns;   /* Process spawns */
} userspace_state_t;

/* Function Prototypes */

/* Core Userspace */
int userspace_init(void);
void userspace_shutdown(void);

/* Process Environment */
process_env_t *create_process_env(void);
void destroy_process_env(process_env_t *env);
int setup_process_environment(process_env_t *env, const char *program, 
                             char *const argv[], char *const envp[]);
int set_working_directory(process_env_t *env, const char *path);
int add_environment_variable(process_env_t *env, const char *name, const char *value);

/* Dynamic Linking */
shared_library_t *load_shared_library(const char *path);
void unload_shared_library(shared_library_t *lib);
void *resolve_symbol(const char *name);
int relocate_library(shared_library_t *lib);
int bind_library_symbols(shared_library_t *lib);

/* IPC System */
int create_pipe(int pipefd[2]);
int create_sysv_msgqueue(key_t key, int msgflg);
int create_posix_msgqueue(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
int create_shared_memory(key_t key, size_t size, int shmflg);
int create_semaphore(key_t key, int nsems, int semflg);

/* Signal Handling */
int install_signal_handler(process_env_t *env, int sig, struct sigaction *act);
int send_signal(pid_t pid, int sig, signal_info_t *info);
int wait_for_signal(process_env_t *env, sigset_t *set, signal_info_t *info);

/* File Descriptor Management */
int allocate_fd(process_env_t *env);
void deallocate_fd(process_env_t *env, int fd);
struct file *get_file_from_fd(process_env_t *env, int fd);
int associate_file_fd(process_env_t *env, int fd, struct file *file);

/* Shell Interface */
int init_shell_environment(shell_env_t *shell, const char *shell_path);
int execute_shell_command(shell_env_t *shell, const char *command);
int add_shell_alias(shell_env_t *shell, const char *name, const char *value);
int add_shell_function(shell_env_t *shell, const char *name, const char *body);

/* POSIX Compliance */
int init_posix_support(void);
bool check_posix_feature(int feature);
int posix_spawn(pid_t *pid, const char *path, const posix_spawn_file_actions_t *file_actions,
               const posix_spawnattr_t *attrp, char *const argv[], char *const envp[]);

/* System Calls */
long sys_fork(void);
long sys_execve(const char *pathname, char *const argv[], char *const envp[]);
long sys_wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage);
long sys_pipe(int pipefd[2]);
long sys_dup2(int oldfd, int newfd);
long sys_chdir(const char *path);
long sys_getcwd(char *buf, size_t size);

/* Compatibility Layers */
int init_linux_compat(void);
int init_win32_compat(void);
long linux_syscall_emulate(long number, ...);
long win32_api_emulate(const char *function, ...);

/* Library Management */
void *libc_malloc(size_t size);
void libc_free(void *ptr);
int libc_printf(const char *format, ...);
FILE *libc_fopen(const char *pathname, const char *mode);

/* Utility Functions */
char *userspace_strdup(const char *s);
void userspace_show_stats(void);
int validate_user_pointer(const void *ptr, size_t size);

/* ELF Loading */
int load_elf_executable(const char *path, uintptr_t *entry_point);
int load_elf_library(const char *path, shared_library_t **lib);
int parse_elf_headers(void *elf_data, shared_library_t *lib);

#endif /* __USERSPACE_H__ */