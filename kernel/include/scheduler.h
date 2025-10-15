/*
 * LimitlessOS Advanced Scheduler Header
 * Production-grade scheduler definitions and interfaces
 */

#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "mm/mm.h"

// Define pid_t for kernel use
typedef int pid_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;

// Forward declarations for types used in this header
struct rq;
struct rb_node { struct rb_node *left, *right, *parent; int color; };

// Constants (move NR_CPUS and related macros up)
#define NR_CPUS 64
#define MAX_CPUS NR_CPUS
#define BITS_PER_LONG 64
#define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, BITS_PER_LONG)
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define TASK_COMM_LEN 16
#define WMULT_SHIFT 32

// Scheduling classes
#define SCHED_NORMAL    0   // CFS normal tasks
#define SCHED_FIFO      1   // Real-time FIFO
#define SCHED_RR        2   // Real-time round-robin
#define SCHED_BATCH     3   // Batch processing
#define SCHED_IDLE      5   // Very low priority
#define SCHED_DEADLINE  6   // Deadline scheduling
#define SCHED_MAX       6

// Task states
#define TASK_RUNNING            0
#define TASK_INTERRUPTIBLE      1
#define TASK_UNINTERRUPTIBLE    2
#define TASK_ZOMBIE             4
#define TASK_STOPPED            8
#define TASK_TRACED            16

// Priority ranges
#define MAX_USER_RT_PRIO    100
#define MAX_RT_PRIO         MAX_USER_RT_PRIO
#define MAX_PRIO           (MAX_RT_PRIO + NICE_WIDTH)
#define DEFAULT_PRIO       (MAX_RT_PRIO + NICE_WIDTH / 2)
#define NICE_WIDTH          40
#define MAX_NICE            19
#define MIN_NICE           -20

// Number of priority levels for scheduler queues
#define NUM_PRIORITY_LEVELS 32

// CPU power states
#define CPU_POWER_LOW       0
#define CPU_POWER_NORMAL    1
#define CPU_POWER_HIGH      2

// Load balancing flags
#define SD_LOAD_BALANCE     0x0001
#define SD_BALANCE_NEWIDLE  0x0002
#define SD_BALANCE_EXEC     0x0004
#define SD_BALANCE_FORK     0x0008
#define SD_BALANCE_WAKE     0x0010
#define SD_WAKE_AFFINE      0x0020
#define SD_NUMA             0x0040
#define SD_ASYM_CPUCAPACITY 0x0080

// Wake flags
#define WF_SYNC         0x01
#define WF_FORK         0x02
#define WF_MIGRATED     0x04

// Enqueue flags
#define ENQUEUE_WAKEUP      0x01
#define ENQUEUE_HEAD        0x02
#define ENQUEUE_WAKING      0x04
#define DEQUEUE_SLEEP       0x01

// Forward declarations
struct task_struct;
struct sched_domain;
struct cpumask;

// Load weight structure for CFS
struct load_weight {
    unsigned long weight;
    uint32_t inv_weight;
};

// List head for linked lists
struct list_head {
    struct list_head *next, *prev;
};

// High-resolution timer
struct hrtimer {
    struct rb_node node;
    uint64_t expires;
    void (*function)(struct hrtimer *);
};

// CPU mask operations
typedef struct cpumask {
    unsigned long bits[BITS_TO_LONGS(NR_CPUS)];
} cpumask_t;

// Scheduling parameters
struct sched_param {
    int sched_priority;
};

// Deadline scheduling parameters
struct sched_attr {
    uint32_t size;
    uint32_t sched_policy;
    uint64_t sched_flags;
    
    // SCHED_NORMAL, SCHED_BATCH
    int32_t sched_nice;
    
    // SCHED_FIFO, SCHED_RR
    uint32_t sched_priority;
    
    // SCHED_DEADLINE
    uint64_t sched_runtime;
    uint64_t sched_deadline;
    uint64_t sched_period;
};

// Per-entity load tracking
struct sched_avg {
    uint64_t last_update_time;
    uint64_t load_sum;
    uint32_t util_sum;
    uint32_t load_avg;
    uint32_t util_avg;
    uint32_t period_contrib;
};

// CPU statistics
struct cpu_stats {
    uint32_t nr_running;
    uint32_t nr_switches;
    uint32_t cpu_capacity;
    uint64_t avg_idle;
    bool is_performance_core;
    uint32_t numa_node;
    bool parked;
};

// System scheduler statistics
struct sched_stats {
    uint32_t nr_cpus;
    uint32_t total_forks;
    uint32_t nr_running;
    uint32_t nr_uninterruptible;
    struct cpu_stats per_cpu[64];
};

// Scheduling class operations
struct sched_class {
    const struct sched_class *next;
    
    void (*enqueue_task)(struct rq *rq, struct task_struct *p, int flags);
    void (*dequeue_task)(struct rq *rq, struct task_struct *p, int flags);
    void (*yield_task)(struct rq *rq);
    bool (*yield_to_task)(struct rq *rq, struct task_struct *p, bool preempt);
    
    void (*check_preempt_curr)(struct rq *rq, struct task_struct *p, int flags);
    
    struct task_struct * (*pick_next_task)(struct rq *rq);
    void (*put_prev_task)(struct rq *rq, struct task_struct *p);
    
    int  (*select_task_rq)(struct task_struct *p, int task_cpu, int sd_flag, int flags);
    void (*migrate_task_rq)(struct task_struct *p);
    
    void (*task_woken)(struct rq *this_rq, struct task_struct *task);
    
    void (*set_cpus_allowed)(struct task_struct *p, const struct cpumask *newmask);
    
    void (*rq_online)(struct rq *rq);
    void (*rq_offline)(struct rq *rq);
    
    void (*task_tick)(struct rq *rq, struct task_struct *p, int queued);
    void (*task_fork)(struct task_struct *p);
    void (*task_dead)(struct task_struct *p);
    
    void (*switched_from)(struct rq *this_rq, struct task_struct *task);
    void (*switched_to)(struct rq *this_rq, struct task_struct *task);
    void (*prio_changed)(struct rq *this_rq, struct task_struct *task, int oldprio);
    
    unsigned int (*get_rr_interval)(struct rq *rq, struct task_struct *task);
    
    void (*update_curr)(struct rq *rq);
};

// External API functions
extern int scheduler_init(void);
extern void scheduler_shutdown(void);
extern void __schedule(bool preempt);
extern void schedule(void);
extern void schedule_timeout(signed long timeout);
extern void yield(void);
extern void yield_to(struct task_struct *p, bool preempt);

extern int try_to_wake_up(struct task_struct *p, unsigned int state, int wake_flags);
extern int wake_up_process(struct task_struct *p);
extern int wake_up_state(struct task_struct *p, unsigned int state);

extern void set_task_cpu(struct task_struct *p, unsigned int cpu);
extern int task_cpu(const struct task_struct *p);
extern unsigned int task_rq(struct task_struct *p);

extern void activate_task(struct rq *rq, struct task_struct *p, int flags);
extern void deactivate_task(struct rq *rq, struct task_struct *p, int flags);

extern void check_preempt_curr(struct rq *rq, struct task_struct *p, int flags);
extern void resched_curr(struct rq *rq);
extern void resched_cpu(int cpu);

// System calls
extern int sys_sched_setscheduler(pid_t pid, int policy, 
                                 const struct sched_param *param);
extern int sys_sched_getscheduler(pid_t pid);
extern int sys_sched_setparam(pid_t pid, const struct sched_param *param);
extern int sys_sched_getparam(pid_t pid, struct sched_param *param);
extern int sys_sched_setattr(pid_t pid, const struct sched_attr *attr, 
                            unsigned int flags);
extern int sys_sched_getattr(pid_t pid, struct sched_attr *attr, 
                            unsigned int size, unsigned int flags);
extern int sys_sched_yield(void);
extern int sys_sched_get_priority_max(int policy);
extern int sys_sched_get_priority_min(int policy);
extern int sys_sched_rr_get_interval(pid_t pid, struct timespec *interval);

// CPU affinity
extern int sys_sched_setaffinity(pid_t pid, unsigned int len, 
                                unsigned long *user_mask_ptr);
extern int sys_sched_getaffinity(pid_t pid, unsigned int len, 
                                unsigned long *user_mask_ptr);

// Load balancing
extern void load_balance_init(void);
extern void update_rq_clock(struct rq *rq);
extern void trigger_load_balance(struct rq *rq);
extern int idle_balance(int this_cpu, struct rq *this_rq);

// CPU capacity and topology
extern void update_cpu_capacity(struct rq *rq);
extern unsigned long cpu_capacity(int cpu);
extern unsigned long cpu_capacity_orig(int cpu);
extern bool is_performance_core(int cpu);
extern int cpu_to_numa_node(int cpu);

// NUMA balancing
extern void task_numa_fault(int last_cpupid, int mem_node, int pages, int flags);
extern void task_numa_free(struct task_struct *p);
extern void task_numa_work(struct callback_head *work);

// Power management integration
extern void scheduler_update_cpu_capacity(int cpu, unsigned long capacity);
extern void scheduler_cpu_online(int cpu);
extern void scheduler_cpu_offline(int cpu);
extern bool scheduler_can_stop_cpu(int cpu);

// Statistics and monitoring
extern void scheduler_get_stats(struct sched_stats *stats);
extern void update_rq_runnable_avg(struct rq *rq, int runnable);

// Scheduling class instances
extern const struct sched_class stop_sched_class;
extern const struct sched_class dl_sched_class;
extern const struct sched_class rt_sched_class;
extern const struct sched_class fair_sched_class;
extern const struct sched_class idle_sched_class;

// Legacy compatibility
typedef enum {
    TASK_READY = TASK_RUNNING,
    TASK_SLEEPING = TASK_INTERRUPTIBLE,
    TASK_DEAD = TASK_ZOMBIE
} task_state_t;

typedef struct {
    uint32_t esp;
} cpu_state_t;

typedef struct task {
    int id;
    int state;
    cpu_state_t context;
    int priority;
    uint64_t quantum;
    void* stack;
    struct task* next;
} task_t;

// Legacy API for compatibility
extern task_t* create_task(void (*entry)(void));
extern void switch_to_task(task_t* task);

// Constants
#define NR_CPUS 64
#define MAX_CPUS NR_CPUS
#define BITS_PER_LONG 64
#define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, BITS_PER_LONG)
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define TASK_COMM_LEN 16
#define WMULT_SHIFT 32

#endif // KERNEL_SCHEDULER_H