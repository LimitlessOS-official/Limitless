/**
 * SMP-Aware Scheduler for LimitlessOS
 * 
 * Multi-core scheduler implementation with load balancing, CPU affinity,
 * and NUMA awareness. Based on CFS (Completely Fair Scheduler) principles.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "smp.h"
#include "mm/mm.h"

/* Scheduling classes */
#define SCHED_CLASS_NORMAL      0   /* Default CFS scheduler */
#define SCHED_CLASS_RT          1   /* Real-time scheduler */
#define SCHED_CLASS_IDLE        2   /* Idle tasks */
#define SCHED_CLASS_STOP        3   /* Stop machine tasks */

/* Task states */
typedef enum {
    TASK_RUNNING = 0,      /* Currently executing */
    TASK_READY,            /* Ready to run */
    TASK_INTERRUPTIBLE,    /* Sleeping, can be woken by signals */
    TASK_UNINTERRUPTIBLE,  /* Sleeping, cannot be interrupted */
    TASK_STOPPED,          /* Stopped by signal */
    TASK_TRACED,           /* Being traced by debugger */
    TASK_DEAD,             /* Dead, waiting for cleanup */
    TASK_ZOMBIE            /* Dead, parent hasn't reaped yet */
} task_state_t;

/* Priority levels */
#define MAX_RT_PRIO     100
#define MAX_PRIO        140
#define DEFAULT_PRIO    120
#define NICE_TO_PRIO(nice)  (DEFAULT_PRIO + (nice))

/* CPU scheduling entity */
typedef struct sched_entity {
    uint64_t vruntime;          /* Virtual runtime */
    uint64_t exec_start;        /* Last execution start time */
    uint64_t sum_exec_runtime;  /* Total execution time */
    uint64_t prev_sum_exec;     /* Previous sum for delta calculation */
    
    /* Load and priority */
    unsigned long load_weight;  /* Load weight for this entity */
    int prio;                   /* Static priority */
    int nice;                   /* Nice value (-20 to +19) */
    
    /* Scheduling statistics */
    uint64_t wait_start;        /* When we started waiting */
    uint64_t wait_max;          /* Max wait time */
    uint64_t wait_count;        /* Number of waits */
    uint64_t wait_sum;          /* Sum of wait times */
} sched_entity_t;

/* Real-time scheduling entity */
typedef struct rt_sched_entity {
    struct list_head run_list;  /* RT run queue list */
    uint64_t timeout;           /* RT timeout */
    uint64_t watchdog_stamp;    /* Watchdog timestamp */
    unsigned int time_slice;    /* RT time slice */
    int nr_cpus_allowed;        /* CPUs this task can run on */
} rt_sched_entity_t;

/* Task structure (Process Control Block) */
typedef struct task {
    /* Basic task information */
    pid_t pid;                  /* Process ID */
    pid_t tgid;                 /* Thread group ID */
    pid_t ppid;                 /* Parent process ID */
    
    /* Task state */
    volatile task_state_t state;
    unsigned int flags;         /* Task flags */
    int exit_code;              /* Exit code when task dies */
    
    /* CPU context */
    struct {
        uint32_t esp;           /* Stack pointer */
        uint32_t ebp;           /* Base pointer */
        uint32_t eip;           /* Instruction pointer */
        uint32_t eflags;        /* CPU flags */
        uint32_t cr3;           /* Page directory */
    } context;
    
    /* Scheduling information */
    int sched_class;            /* Scheduling class */
    sched_entity_t se;          /* CFS scheduling entity */
    rt_sched_entity_t rt;       /* RT scheduling entity */
    
    /* CPU affinity and NUMA */
    cpu_mask_t cpu_affinity;    /* CPUs this task can run on */
    int preferred_cpu;          /* Preferred CPU */
    int numa_node;              /* Preferred NUMA node */
    int last_cpu;               /* Last CPU this task ran on */
    
    /* Memory management */
    void *stack;                /* Kernel stack */
    size_t stack_size;          /* Stack size */
    void *page_dir;             /* Page directory */
    
    /* Timing and statistics */
    uint64_t start_time;        /* Task creation time */
    uint64_t total_time;        /* Total CPU time used */
    uint64_t sleep_time;        /* Total sleep time */
    uint64_t last_ran;          /* Last time task ran */
    
    /* Process relationships */
    struct task *parent;        /* Parent task */
    struct list_head children;  /* List of child tasks */
    struct list_head sibling;   /* Sibling list entry */
    
    /* Synchronization */
    void *blocked_on;           /* What we're waiting for */
    struct list_head wait_list; /* Wait queue entry */
    
    /* Security context */
    uint32_t security_context;  /* Security context ID */
    
    /* Scheduling lists */
    struct list_head run_list;  /* Run queue entry */
    struct list_head task_list; /* Global task list entry */
    
    /* Reference counting */
    atomic_t usage;             /* Reference count */
    
    /* Performance monitoring */
    uint64_t nvcsw;             /* Voluntary context switches */
    uint64_t nivcsw;            /* Involuntary context switches */
    
    char comm[16];              /* Task name */
} task_t;

/* Per-CPU run queue structure */
typedef struct cpu_runqueue {
    spinlock_t lock;            /* Runqueue lock */
    
    /* CFS (Completely Fair Scheduler) */
    struct {
        struct rb_root_cached root;  /* RB-tree for CFS tasks */
        struct rb_node *leftmost;    /* Leftmost node in tree */
        uint64_t min_vruntime;       /* Minimum vruntime */
        uint32_t nr_running;         /* Number of running tasks */
        uint64_t load_weight;        /* Total load weight */
    } cfs;
    
    /* Real-time scheduler */
    struct {
        struct list_head queue[MAX_RT_PRIO]; /* RT priority queues */
        uint32_t nr_running;         /* Number of RT tasks */
        unsigned long rt_nr_migratory; /* Migratory RT tasks */
        uint64_t rt_throttled;       /* RT throttling time */
    } rt;
    
    /* Current task */
    task_t *curr;               /* Currently running task */
    task_t *idle;               /* Idle task for this CPU */
    task_t *stop;               /* Stop task for this CPU */
    
    /* Load balancing */
    uint64_t next_balance;      /* Next load balance time */
    uint32_t balance_interval;  /* Load balance interval */
    uint64_t avg_load_per_task; /* Average load per task */
    
    /* CPU utilization */
    uint64_t cpu_load[5];       /* CPU load averages */
    uint64_t last_update;       /* Last update time */
    
    /* Statistics */
    uint64_t sched_count;       /* Number of times scheduled */
    uint64_t sched_goidle;      /* Times went idle */
    uint64_t ttwu_count;        /* Times tried to wake up */
    uint64_t ttwu_local;        /* Times woke up local task */
    
    /* Clock and timing */
    uint64_t clock;             /* Runqueue clock */
    uint64_t clock_task;        /* Task clock */
    
} cpu_runqueue_t;

/* Scheduling domains for load balancing */
typedef struct sched_domain {
    struct sched_domain *parent;    /* Parent domain */
    struct sched_domain *child;     /* Child domain */
    cpu_mask_t span;                /* CPUs covered by this domain */
    uint32_t level;                 /* Domain level */
    uint32_t flags;                 /* Domain flags */
    
    /* Load balancing parameters */
    uint32_t balance_interval;      /* Balance interval in ms */
    uint32_t busy_factor;           /* Busy threshold multiplier */
    uint32_t imbalance_pct;         /* Imbalance percentage threshold */
    uint32_t cache_nice_tries;      /* Cache-hot migration tries */
    
    char name[32];                  /* Domain name for debugging */
} sched_domain_t;

/* Load balancing groups */
typedef struct sched_group {
    struct sched_group *next;       /* Next group in domain */
    cpu_mask_t cpumask;             /* CPUs in this group */
    uint32_t group_weight;          /* Weight of this group */
    uint32_t group_capacity;        /* Capacity of this group */
} sched_group_t;

/* Wake-up flags */
#define WF_SYNC     0x01    /* Synchronous wakeup */
#define WF_FORK     0x02    /* Wakeup from fork */
#define WF_MIGRATED 0x04    /* Task was migrated */

/* Migration types */
#define MIGRATION_CALL      1
#define MIGRATION_CPU_STOP  2
#define MIGRATION_HOTPLUG   3

/* Global scheduler state */
extern cpu_runqueue_t cpu_runqueues[MAX_CPUS];
extern sched_domain_t *sched_domains[MAX_CPUS];
extern task_t *init_task;

/* Scheduler initialization */
int sched_init(void);
void sched_init_cpu(uint32_t cpu);
void sched_build_domains(void);

/* Task management */
task_t *sched_create_task(void (*entry_point)(void), const char *name);
void sched_destroy_task(task_t *task);
pid_t sched_get_next_pid(void);

/* Core scheduling functions */
void schedule(void);
void schedule_tail(void);
void preempt_schedule(void);
void preempt_schedule_irq(void);
task_t *pick_next_task(cpu_runqueue_t *rq);
void put_prev_task(cpu_runqueue_t *rq, task_t *prev);

/* Task state management */
void set_task_state(task_t *task, task_state_t state);
void wake_up_process(task_t *task);
int wake_up_state(task_t *task, task_state_t state);
void scheduler_tick(void);

/* Load balancing */
void load_balance(uint32_t this_cpu, cpu_runqueue_t *this_rq);
void rebalance_domains(uint32_t cpu);
int can_migrate_task(task_t *task, uint32_t dest_cpu);
void migrate_task(task_t *task, uint32_t dest_cpu);

/* CPU affinity */
int sched_set_affinity(pid_t pid, const cpu_mask_t *mask);
int sched_get_affinity(pid_t pid, cpu_mask_t *mask);
void set_cpus_allowed(task_t *task, const cpu_mask_t *mask);

/* Priority and nice values */
void set_user_nice(task_t *task, long nice);
long get_user_nice(task_t *task);
void set_normal_prio(task_t *task);
void set_effective_prio(task_t *task);

/* Real-time scheduling */
int sched_set_rt_priority(pid_t pid, int priority);
int sched_get_rt_priority(pid_t pid);
void sched_set_fifo(task_t *task, int priority);
void sched_set_rr(task_t *task, int priority);

/* Idle tasks */
task_t *sched_create_idle_task(uint32_t cpu);
void cpu_idle_loop(void);
bool idle_cpu(uint32_t cpu);

/* Context switching */
void context_switch(cpu_runqueue_t *rq, task_t *prev, task_t *next);
void switch_mm(task_t *prev, task_t *next);

/* Time management */
uint64_t sched_clock(void);
uint64_t sched_clock_cpu(uint32_t cpu);
void update_rq_clock(cpu_runqueue_t *rq);

/* Statistics and monitoring */
void sched_show_task(task_t *task);
void show_state_filter(unsigned long state_filter);
void sched_show_stats(void);
uint32_t nr_running(void);
uint32_t nr_iowait(void);

/* Process groups and sessions */
int sched_setpgid(pid_t pid, pid_t pgid);
pid_t sched_getpgid(pid_t pid);
int sched_setsid(void);
pid_t sched_getsid(pid_t pid);

/* Yield and sleep */
void sched_yield(void);
void msleep(unsigned int msecs);
void msleep_interruptible(unsigned int msecs);

/* Task lookup */
task_t *find_task_by_pid(pid_t pid);
task_t *find_task_by_tgid(pid_t tgid);

/* Fork support */
task_t *copy_process(unsigned long clone_flags, void (*fn)(void *), void *arg);
void copy_thread(task_t *p, unsigned long clone_flags);

/* Exit and wait */
void do_exit(int exit_code);
pid_t do_wait(int *status, int options);

/* Signal handling integration */
void signal_wake_up_state(task_t *task, task_state_t state);
void recalc_sigpending(void);

/* Debugging and profiling */
void sched_debug_show_runqueues(void);
void sched_debug_show_domains(void);
void sched_profile_tick(void);

/* Macros for common operations */
#define current         (this_cpu(current_task))
#define current_cpu()   smp_processor_id()
#define task_cpu(p)     ((p)->last_cpu)
#define cpu_rq(cpu)     (&cpu_runqueues[cpu])
#define this_rq()       cpu_rq(current_cpu())
#define task_rq(p)      cpu_rq(task_cpu(p))

/* Priority conversion macros */
#define NICE_TO_PRIO(nice)      (MAX_PRIO - 20 + (nice))
#define PRIO_TO_NICE(prio)      ((prio) - MAX_PRIO + 20)
#define TASK_NICE(p)            PRIO_TO_NICE((p)->se.prio)

/* Load weight calculation */
#define NICE_0_LOAD         1024
#define NICE_0_SHIFT        10

/* Time slice calculations */
#define DEF_TIMESLICE       (100 * 1000000ULL)  /* 100ms in ns */
#define MIN_TIMESLICE       (5 * 1000000ULL)    /* 5ms in ns */
#define MAX_TIMESLICE       (800 * 1000000ULL)  /* 800ms in ns */

/* Red-Black tree support for CFS */
struct rb_node {
    unsigned long rb_parent_color;
    struct rb_node *rb_right;
    struct rb_node *rb_left;
};

struct rb_root {
    struct rb_node *rb_node;
};

struct rb_root_cached {
    struct rb_root rb_root;
    struct rb_node *rb_leftmost;
};

/* List support */
struct list_head {
    struct list_head *next, *prev;
};

/* Atomic operations */
typedef struct {
    int counter;
} atomic_t;

#define ATOMIC_INIT(i) { (i) }

void atomic_set(atomic_t *v, int i);
int atomic_read(atomic_t *v);
void atomic_inc(atomic_t *v);
void atomic_dec(atomic_t *v);
int atomic_inc_return(atomic_t *v);
int atomic_dec_return(atomic_t *v);

/* Per-CPU current task pointer */
DEFINE_PER_CPU(task_t *, current_task);

/* Scheduler class operations */
struct sched_class {
    void (*enqueue_task)(cpu_runqueue_t *rq, task_t *p, int flags);
    void (*dequeue_task)(cpu_runqueue_t *rq, task_t *p, int flags);
    void (*yield_task)(cpu_runqueue_t *rq);
    task_t *(*pick_next_task)(cpu_runqueue_t *rq);
    void (*put_prev_task)(cpu_runqueue_t *rq, task_t *p);
    void (*set_curr_task)(cpu_runqueue_t *rq);
    void (*task_tick)(cpu_runqueue_t *rq, task_t *p, int queued);
    void (*switched_from)(cpu_runqueue_t *rq, task_t *p);
    void (*switched_to)(cpu_runqueue_t *rq, task_t *p);
    void (*prio_changed)(cpu_runqueue_t *rq, task_t *p, int old_prio);
};

extern struct sched_class fair_sched_class;
extern struct sched_class rt_sched_class;
extern struct sched_class idle_sched_class;
extern struct sched_class stop_sched_class;