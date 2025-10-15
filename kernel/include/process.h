#pragma once
#include "kernel.h"

struct vmm_aspace;

/* Process states */
typedef enum {
    PROC_STATE_NEW = 0,
    PROC_STATE_READY,
    PROC_STATE_RUNNING,
    PROC_STATE_BLOCKED,
    PROC_STATE_SLEEPING,
    PROC_STATE_ZOMBIE,
    PROC_STATE_TERMINATED
} process_state_t;

/* Process priority levels */
typedef enum {
    PROC_PRIO_REALTIME = 0,
    PROC_PRIO_HIGH = 1,
    PROC_PRIO_NORMAL = 2,
    PROC_PRIO_LOW = 3,
    PROC_PRIO_IDLE = 4
} process_priority_t;

/* Security context */
typedef struct security_context {
    uid_t uid, euid, suid;  /* User IDs */
    gid_t gid, egid, sgid;  /* Group IDs */
    uint64_t capabilities;   /* Capability bitmask */
    uint32_t security_label; /* SELinux-style label */
    bool is_privileged;
    bool can_exec_setuid;
} security_context_t;

/* Resource limits and usage */
typedef struct resource_usage {
    uint64_t cpu_time_us;       /* CPU time in microseconds */
    uint64_t wall_time_us;      /* Wall clock time */
    size_t memory_usage;        /* Current memory usage */
    size_t memory_peak;         /* Peak memory usage */
    uint32_t open_files;        /* Number of open files */
    uint32_t threads_count;     /* Number of threads */
    uint64_t syscalls_count;    /* System calls made */
    uint64_t context_switches;  /* Context switches */
    uint64_t page_faults;       /* Page faults */
    uint64_t io_bytes_read;     /* I/O bytes read */
    uint64_t io_bytes_written;  /* I/O bytes written */
} resource_usage_t;

/* Resource limits */
typedef struct resource_limits {
    size_t max_memory;          /* Maximum memory */
    uint64_t max_cpu_time;      /* Maximum CPU time */
    uint32_t max_open_files;    /* Maximum open files */
    uint32_t max_threads;       /* Maximum threads */
    uint32_t max_processes;     /* Maximum child processes */
} resource_limits_t;

/* Thread pool for process */
typedef struct thread_pool {
    struct thread** threads;    /* Array of threads */
    uint32_t size;             /* Pool size */
    uint32_t active_count;     /* Active threads */
    uint32_t max_size;         /* Maximum pool size */
    spinlock_t lock;
    struct work_queue* work_queue; /* Work queue */
} thread_pool_t;

/* Enhanced process structure */
typedef struct process {
    pid_t pid;
    pid_t parent_pid;
    pid_t session_id;
    pid_t process_group_id;
    
    char name[64];              /* Process name */
    char cmdline[256];          /* Command line */
    
    process_state_t state;
    process_priority_t priority;
    int nice_value;             /* Nice value (-20 to 19) */
    
    security_context_t security; /* Security context */
    resource_usage_t usage;      /* Resource usage */
    resource_limits_t limits;    /* Resource limits */
    
    uint64_t start_time;        /* Process start time */
    int exit_code;              /* Exit code */
    
    struct vmm_aspace* as;      /* Address space */
    struct fd_table* fdtab;     /* File descriptor table */
    struct signal_state* sigstate; /* Signal state */
    
    thread_pool_t* thread_pool; /* Thread pool */
    
    /* Process tree links */
    struct process* parent;
    struct process* first_child;
    struct process* next_sibling;
    
    /* Scheduling and performance */
    uint32_t cpu_affinity;      /* CPU affinity mask */
    uint64_t last_scheduled;    /* Last scheduled time */
    uint64_t total_runtime;     /* Total runtime */
    
    spinlock_t lock;            /* Process lock */
} process_t;

/* Enhanced Process API */
status_t process_create_advanced(process_t** out, const char* name, const char* cmdline, 
                                security_context_t* security, resource_limits_t* limits);
status_t process_destroy(process_t* p);
process_t* process_lookup(pid_t pid);
process_t* process_get_kernel(void);
process_t* process_current(void);
void process_set_current(process_t* p);
status_t process_fork(process_t* parent, process_t** child_out);
status_t process_exec(process_t* proc, const char* path, char* const argv[], char* const envp[]);
status_t process_clone(process_t* parent, process_t** child_out, uint32_t flags);

/* Process state management */
status_t process_suspend(process_t* proc);
status_t process_resume(process_t* proc);
status_t process_terminate(process_t* proc, int signal);
status_t process_kill(pid_t pid, int signal);
status_t process_yield(void);
void process_exit(process_t* proc, int exit_code);
status_t process_waitpid(pid_t pid, int* status, int options);

/* Process priority and scheduling */
status_t process_set_priority(process_t* proc, process_priority_t priority);
status_t process_set_nice(process_t* proc, int nice_value);
status_t process_set_cpu_affinity(process_t* proc, uint32_t cpu_mask);
status_t process_get_cpu_affinity(process_t* proc, uint32_t* cpu_mask);

/* Resource management */
status_t process_set_resource_limit(process_t* proc, int resource, uint64_t limit);
status_t process_get_resource_usage(process_t* proc, resource_usage_t* usage);
status_t process_update_resource_usage(process_t* proc);

/* Thread pool management */
status_t process_create_thread_pool(process_t* proc, uint32_t initial_size, uint32_t max_size);
status_t process_destroy_thread_pool(process_t* proc);
status_t process_thread_pool_add_work(process_t* proc, void (*work_func)(void*), void* arg);
status_t process_thread_pool_resize(process_t* proc, uint32_t new_size);

/* Process tree navigation */
process_t* process_get_parent(process_t* proc);
process_t* process_get_first_child(process_t* proc);
process_t* process_get_next_sibling(process_t* proc);
status_t process_enum_children(process_t* proc, process_t** children, uint32_t* count);

/* Security context management */
status_t process_set_security_context(process_t* proc, security_context_t* security);
status_t process_get_security_context(process_t* proc, security_context_t* security);
status_t process_check_capability(process_t* proc, uint64_t capability);
status_t process_grant_capability(process_t* proc, uint64_t capability);
status_t process_revoke_capability(process_t* proc, uint64_t capability);

/* Performance profiling */
status_t process_start_profiling(process_t* proc);
status_t process_stop_profiling(process_t* proc);
status_t process_get_profile_data(process_t* proc, void** data, size_t* size);

/* Inter-Process Communication */
status_t process_create_ipc_channel(process_t* proc1, process_t* proc2, uint32_t* channel_id);
status_t process_send_ipc_message(uint32_t channel_id, const void* data, size_t size);
status_t process_receive_ipc_message(uint32_t channel_id, void* buffer, size_t* size);
status_t process_destroy_ipc_channel(uint32_t channel_id);

/* Signal handling integration */
void process_wakeup(process_t* proc);

/* Legacy compatibility */
int process_create(process_t** out, uid_t uid, gid_t gid, const char* name);
process_t* process_create_user(const char* name);
struct thread* thread_create_user(process_t* proc, vaddr_t entry, vaddr_t sp);
pid_t process_get_pid(process_t* proc);
void scheduler_add_thread(struct thread* thread);

/* Thread alloc (kernel threads in Phase 1) */
/* Forward declarations */
/* Forward declarations */
struct thread;
typedef struct thread thread_t;

/* Function declarations with proper types */
thread_t* process_alloc_kernel_thread(void (*entry)(void*), void* arg, void* stack_base, size_t stack_size);
struct thread* process_thread_lookup(tid_t tid);

/* Simple accessors */
/* Moved to avoid duplicate declaration */
