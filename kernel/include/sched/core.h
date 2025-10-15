/*
 * LimitlessOS Advanced Scheduler Core
 * Production-quality Completely Fair Scheduler with AI integration
 * SMP-aware, preemptible, real-time capable
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../kernel.h"

/* CPU topology and NUMA information */
#define MAX_CPUS 256
#define MAX_NUMA_NODES 16

typedef struct cpu_topology {
    uint32_t cpu_id;
    uint32_t core_id;
    uint32_t package_id;
    uint32_t numa_node;
    uint32_t l1_cache_id;
    uint32_t l2_cache_id;
    uint32_t l3_cache_id;
    uint64_t cpu_frequency;
    bool smt_enabled;
    uint32_t smt_siblings[8];
    uint32_t smt_sibling_count;
} cpu_topology_t;

/* Power states and frequency scaling */
typedef enum {
    CPU_POWER_C0 = 0,      /* Active */
    CPU_POWER_C1,          /* Halt */
    CPU_POWER_C2,          /* Stop clock */
    CPU_POWER_C3,          /* Sleep */
    CPU_POWER_C6,          /* Deep sleep */
    CPU_POWER_C7,          /* Deeper sleep */
    CPU_POWER_C8,          /* Deepest sleep */
    CPU_POWER_MAX
} cpu_power_state_t;

typedef struct cpu_power_info {
    cpu_power_state_t current_state;
    uint64_t frequency_mhz;
    uint64_t voltage_mv;
    uint32_t p_state;      /* Performance state */
    uint32_t c_state;      /* C-state */
    bool turbo_enabled;
    uint64_t idle_time_ns;
    uint64_t active_time_ns;
} cpu_power_info_t;

/* Scheduling classes and policies */
typedef enum {
    SCHED_CLASS_CFS = 0,    /* Completely Fair Scheduler */
    SCHED_CLASS_RT,         /* Real-time */
    SCHED_CLASS_DL,         /* Deadline */
    SCHED_CLASS_IDLE,       /* Idle tasks */
    SCHED_CLASS_STOP,       /* Stop tasks */
    SCHED_CLASS_MAX
} sched_class_t;

typedef enum {
    SCHED_POLICY_NORMAL = 0, /* CFS normal task */
    SCHED_POLICY_BATCH,      /* CFS batch task */
    SCHED_POLICY_IDLE,       /* CFS idle task */
    SCHED_POLICY_FIFO,       /* RT FIFO */
    SCHED_POLICY_RR,         /* RT Round Robin */
    SCHED_POLICY_DEADLINE,   /* Deadline scheduler */
    SCHED_POLICY_MAX
} sched_policy_t;

/* Task priority and nice values */
#define MAX_NICE 19
#define MIN_NICE -20
#define NICE_WIDTH (MAX_NICE - MIN_NICE + 1)
#define MAX_RT_PRIO 100
#define MAX_USER_RT_PRIO 100
#define MAX_PRIO (MAX_RT_PRIO + NICE_WIDTH)
#define DEFAULT_PRIO (MAX_RT_PRIO + NICE_WIDTH / 2)

/* CFS scheduling parameters */
#define SCHED_LATENCY_NS    6000000ULL   /* 6ms target latency */
#define SCHED_MIN_GRANULARITY_NS 750000ULL /* 0.75ms minimum granularity */
#define SCHED_WAKEUP_GRANULARITY_NS 1000000ULL /* 1ms wakeup granularity */

/* AI prediction system */
typedef struct ai_workload_prediction {
    float cpu_bound_probability;
    float io_bound_probability;
    float interactive_probability;
    float batch_probability;
    uint64_t predicted_runtime_ns;
    uint64_t predicted_sleep_time_ns;
    float confidence_level;
    uint64_t last_update_time;
} ai_workload_prediction_t;

typedef struct ai_scheduler_engine {
    bool enabled;
    uint64_t total_predictions;
    uint64_t correct_predictions;
    float accuracy_threshold;
    
    /* Neural network weights for workload classification */
    struct {
        float input_weights[64][32];   /* 64 inputs, 32 hidden */
        float hidden_weights[32][16];  /* 32 hidden, 16 hidden2 */
        float output_weights[16][4];   /* 16 hidden2, 4 outputs */
        float input_bias[32];
        float hidden_bias[16];
        float output_bias[4];
    } neural_network;
    
    /* Feature extraction for AI */
    struct {
        uint64_t syscall_count;
        uint64_t page_faults;
        uint64_t context_switches;
        uint64_t voluntary_switches;
        uint64_t involuntary_switches;
        uint64_t cpu_cycles;
        uint64_t instructions;
        uint64_t cache_misses;
        uint64_t branch_misses;
        uint64_t io_operations;
        uint64_t sleep_time_total;
        uint64_t run_time_total;
    } features;
} ai_scheduler_engine_t;

/* Real-time scheduling parameters */
typedef struct rt_sched_params {
    uint32_t priority;              /* RT priority (0-99) */
    uint64_t runtime_ns;            /* Runtime budget */
    uint64_t deadline_ns;           /* Relative deadline */
    uint64_t period_ns;             /* Period for periodic tasks */
    uint64_t absolute_deadline_ns;  /* Absolute deadline */
    bool sporadic;                  /* Sporadic or periodic */
} rt_sched_params_t;

/* Load balancing and migration */
typedef struct load_balance_info {
    uint64_t load_weight;
    uint64_t runnable_weight;
    uint32_t nr_running;
    uint32_t nr_numa_running;
    uint32_t nr_preferred_running;
    uint64_t cpu_capacity;
    uint64_t cpu_capacity_orig;
    uint32_t group_type;
    uint32_t group_misfit_task_load;
} load_balance_info_t;

/* Per-CPU runqueue structure */
typedef struct cpu_runqueue {
    /* Basic runqueue info */
    uint32_t cpu_id;
    uint32_t nr_running;
    uint32_t nr_switches;
    uint64_t load_weight;
    
    /* CFS runqueue */
    struct {
        struct rb_root_cached tasks_timeline;
        struct task_struct *curr, *next, *last, *skip;
        uint64_t min_vruntime;
        uint32_t nr_running;
        uint64_t load_weight;
        uint64_t runnable_weight;
        uint64_t exec_clock;
        uint64_t clock;
        uint64_t clock_task;
        int on_list;
        
        /* Load balancing */
        uint64_t avg_load_per_task;
        uint64_t h_load;
        uint64_t last_h_load_update;
    } cfs;
    
    /* Real-time runqueue */
    struct {
        struct rt_prio_array active;
        uint32_t nr_running;
        uint32_t rr_nr_running;
        uint64_t rt_time;
        uint64_t rt_runtime;
        struct hrtimer rt_bandwidth_timer;
        bool rt_throttled;
        uint64_t rt_nr_boosted;
        struct task_struct *push_task;
    } rt;
    
    /* Deadline runqueue */
    struct {
        struct rb_root_cached tasks_timeline;
        struct task_struct *curr;
        uint32_t nr_running;
        uint64_t this_bw;
        uint64_t extra_bw;
        uint64_t bw_ratio;
        struct dl_bw dl_bw;
        uint64_t running_bw;
    } dl;
    
    /* Idle and stop tasks */
    struct task_struct *idle;
    struct task_struct *stop;
    
    /* CPU frequency scaling */
    cpu_power_info_t power_info;
    
    /* Load balancing */
    load_balance_info_t lb_info;
    uint64_t last_load_update_tick;
    uint64_t load_avg_period;
    
    /* AI predictions per CPU */
    ai_workload_prediction_t ai_prediction;
    
    /* Statistics */
    struct {
        uint64_t context_switches;
        uint64_t voluntary_switches;
        uint64_t involuntary_switches;
        uint64_t migrations_in;
        uint64_t migrations_out;
        uint64_t load_balance_count;
        uint64_t idle_time_ns;
        uint64_t iowait_time_ns;
        uint64_t steal_time_ns;
    } stats;
    
    /* CPU topology */
    cpu_topology_t topology;
    
    /* Locking */
    spinlock_t lock;
    uint64_t clock_update_flags;
} cpu_runqueue_t;

/* Task scheduling entity (embedded in task_struct) */
typedef struct sched_entity {
    uint64_t load_weight;
    uint64_t runnable_weight;
    bool on_rq;
    
    uint64_t exec_start;
    uint64_t sum_exec_runtime;
    uint64_t vruntime;
    uint64_t prev_sum_exec_runtime;
    
    uint64_t nr_migrations;
    
    /* Fair scheduling */
    struct {
        uint64_t load_avg;
        uint64_t util_avg;
        uint64_t runnable_load_avg;
        uint64_t runnable_sum;
        uint32_t util_sum;
        uint32_t period_contrib;
        uint64_t load_sum;
        uint64_t last_update_time;
    } avg;
    
    struct rb_node run_node;
    struct list_head group_node;
    bool on_list;
    
    /* CFS bandwidth control */
    struct {
        uint64_t runtime_remaining;
        uint64_t consumed;
        struct hrtimer slack_timer;
    } cfs_bandwidth;
} sched_entity_t;

/* Real-time scheduling entity */
typedef struct rt_sched_entity {
    struct list_head run_list;
    uint64_t timeout;
    uint64_t watchdog_stamp;
    uint32_t time_slice;
    uint16_t nr_cpus_allowed;
    uint16_t rr_priority;
    
    struct rb_node pushable_tasks;
    
    rt_sched_params_t rt_params;
} rt_sched_entity_t;

/* Deadline scheduling entity */
typedef struct dl_sched_entity {
    struct rb_node rb_node;
    uint64_t dl_runtime;
    uint64_t dl_deadline;
    uint64_t dl_period;
    uint64_t dl_bw;
    
    uint64_t runtime;
    uint64_t deadline;
    
    uint16_t nr_cpus_allowed;
    uint8_t dl_throttled      : 1;
    uint8_t dl_boosted        : 1;
    uint8_t dl_yielded        : 1;
    uint8_t dl_non_contending : 1;
    uint8_t dl_overrun        : 1;
    
    struct hrtimer dl_timer;
    struct hrtimer inactive_timer;
} dl_sched_entity_t;

/* Enhanced task structure for scheduling */
typedef struct task_struct {
    /* Basic task info */
    pid_t pid;
    pid_t tgid;
    char comm[16];
    
    /* Task state */
    long state;
    int exit_state;
    unsigned int flags;
    
    /* Scheduling */
    int prio, static_prio, normal_prio;
    const struct sched_class *sched_class;
    sched_policy_t policy;
    
    /* Scheduling entities */
    sched_entity_t se;
    rt_sched_entity_t rt;
    dl_sched_entity_t dl;
    
    /* CPU affinity and NUMA */
    uint32_t cpu;
    uint32_t wakee_cpu;
    cpumask_t cpus_mask;
    uint32_t numa_preferred_nid;
    uint64_t numa_migrate_retry;
    
    /* Preemption */
    int preempt_count;
    bool need_resched;
    
    /* Time accounting */
    uint64_t utime, stime;
    uint64_t gtime;
    uint64_t start_time;
    uint64_t start_boottime;
    
    /* CPU time */
    uint64_t min_flt, maj_flt;
    uint64_t nvcsw, nivcsw;  /* voluntary/involuntary context switches */
    
    /* AI workload prediction */
    ai_workload_prediction_t ai_prediction;
    
    /* Memory management */
    struct mm_struct *mm, *active_mm;
    
    /* Process credentials */
    const struct cred *real_cred;
    const struct cred *cred;
    
    /* Process tree */
    struct task_struct *real_parent;
    struct task_struct *parent;
    struct list_head children;
    struct list_head sibling;
    
    /* Thread group */
    struct task_struct *group_leader;
    
    /* PID namespace */
    struct pid_link pids[PIDTYPE_MAX];
    struct nsproxy *nsproxy;
    
    /* File system */
    struct fs_struct *fs;
    struct files_struct *files;
    
    /* Signal handling */
    struct signal_struct *signal;
    struct sighand_struct *sighand;
    sigset_t blocked, real_blocked;
    sigset_t saved_sigmask;
    struct sigpending pending;
    
    /* Memory management */
    unsigned long sas_ss_sp;
    size_t sas_ss_size;
    unsigned int sas_ss_flags;
    
    /* Namespaces */
    struct nsproxy *nsproxy_cache;
    
    /* Capabilities */
    const struct kernel_cap_struct cap_inheritable, cap_permitted;
    const struct kernel_cap_struct cap_effective, cap_bset;
    const struct kernel_cap_struct cap_ambient;
    
    /* Security */
    struct task_security_struct *security;
    
    /* Performance events */
    struct perf_event_context *perf_event_ctxp;
    struct mutex perf_event_mutex;
    struct list_head perf_event_list;
    
    /* CPU controller */
    struct task_group *sched_task_group;
    
    /* Stack information */
    unsigned long stack_canary;
    void *stack;
    
    /* Architecture specific */
    struct thread_struct thread;
    
    /* Lists */
    struct list_head tasks;
    struct list_head pushable_tasks;
    struct list_head pushable_dl_tasks;
    
    /* RCU */
    struct rcu_head rcu;
    
    /* Flags */
    atomic_t usage;
    unsigned int in_execve:1;
    unsigned int in_iowait:1;
    unsigned int restore_sigmask:1;
    unsigned int in_user_fault:1;
    unsigned int brk_randomized:1;
    unsigned int atomic_flags;
    
    /* Exit handling */
    int exit_code, exit_signal;
    int pdeath_signal;
    unsigned long jobctl;
    
    /* Tracing */
    unsigned int trace;
    unsigned int trace_recursion;
    
} task_struct_t;

/* Scheduling class operations */
struct sched_class {
    const struct sched_class *next;
    
    void (*enqueue_task) (struct rq *rq, struct task_struct *p, int flags);
    void (*dequeue_task) (struct rq *rq, struct task_struct *p, int flags);
    void (*yield_task) (struct rq *rq);
    bool (*yield_to_task) (struct rq *rq, struct task_struct *p, bool preempt);
    
    void (*check_preempt_curr) (struct rq *rq, struct task_struct *p, int flags);
    
    struct task_struct * (*pick_next_task) (struct rq *rq,
                                            struct task_struct *prev,
                                            struct rq_flags *rf);
    void (*put_prev_task) (struct rq *rq, struct task_struct *p);
    void (*set_next_task) (struct rq *rq, struct task_struct *p, bool first);
    
    int  (*select_task_rq)(struct task_struct *p, int task_cpu, int sd_flag, int flags);
    void (*migrate_task_rq)(struct task_struct *p, int new_cpu);
    
    void (*task_woken) (struct rq *this_rq, struct task_struct *task);
    
    void (*set_cpus_allowed)(struct task_struct *p, const struct cpumask *newmask);
    
    void (*rq_online)(struct rq *rq);
    void (*rq_offline)(struct rq *rq);
    
    void (*task_tick) (struct rq *rq, struct task_struct *p, int queued);
    void (*task_fork) (struct task_struct *p);
    void (*task_dead) (struct task_struct *p);
    
    void (*switched_from) (struct rq *this_rq, struct task_struct *task);
    void (*switched_to) (struct rq *this_rq, struct task_struct *task);
    void (*prio_changed) (struct rq *this_rq, struct task_struct *task,
                         int oldprio);
    
    unsigned int (*get_rr_interval) (struct rq *rq,
                                    struct task_struct *task);
    
    void (*update_curr) (struct rq *rq);
};

/* Global scheduler state */
typedef struct scheduler_info {
    bool initialized;
    bool smp_enabled;
    bool ai_enabled;
    uint32_t nr_cpus;
    uint32_t nr_numa_nodes;
    
    /* Per-CPU runqueues */
    cpu_runqueue_t *runqueues[MAX_CPUS];
    
    /* Global AI engine */
    ai_scheduler_engine_t ai_engine;
    
    /* Load balancing */
    struct {
        uint64_t last_balance_time;
        uint32_t balance_interval_ms;
        bool enabled;
        uint32_t migration_cost;
        uint32_t cache_nice_tries;
    } load_balancer;
    
    /* CPU topology */
    cpu_topology_t cpu_topology[MAX_CPUS];
    uint32_t numa_distance[MAX_NUMA_NODES][MAX_NUMA_NODES];
    
    /* Power management */
    struct {
        bool enabled;
        uint32_t policy;  /* 0=performance, 1=balanced, 2=powersave */
        bool frequency_scaling;
        bool idle_optimization;
    } power_mgmt;
    
    /* Statistics */
    struct {
        uint64_t total_context_switches;
        uint64_t total_migrations;
        uint64_t total_load_balances;
        uint64_t total_preemptions;
        uint64_t start_time;
    } stats;
    
    /* Debugging */
    uint32_t debug_level;
    bool tracing_enabled;
    
} scheduler_info_t;

/* External scheduler state */
extern scheduler_info_t scheduler;

/* Core scheduler functions */
void sched_init(void);
void sched_init_smp(void);
void sched_tick(void);
void schedule(void);
void yield(void);
void preempt_schedule(void);
void preempt_schedule_irq(void);

/* Task management */
struct task_struct *get_current_task(void);
void set_current_task(struct task_struct *task);
pid_t kernel_thread(int (*fn)(void *), void *arg, unsigned long flags);
void wake_up_process(struct task_struct *task);
void try_to_wake_up(struct task_struct *task, unsigned int state, int sync);

/* SMP and load balancing */
void sched_exec(void);
void sched_fork(struct task_struct *p);
void sched_exit(struct task_struct *p);
void load_balance_tick(void);
int set_cpus_allowed_ptr(struct task_struct *p, const cpumask_t *new_mask);

/* Priority and policy management */
int sched_setscheduler(struct task_struct *p, int policy, const struct sched_param *param);
int sched_setscheduler_nocheck(struct task_struct *p, int policy, const struct sched_param *param);
int sched_getscheduler(struct task_struct *p);
int sched_setparam(struct task_struct *p, const struct sched_param *param);
int sched_getparam(struct task_struct *p, struct sched_param *param);
int sched_setattr(struct task_struct *p, const struct sched_attr *attr);
int sched_getattr(struct task_struct *p, struct sched_attr *attr, unsigned int size);
long sched_setaffinity(pid_t pid, const cpumask_t *in_mask);
long sched_getaffinity(pid_t pid, cpumask_t *mask);

/* Real-time and deadline scheduling */
int sched_rr_get_interval(struct task_struct *p, struct timespec64 *t);
void init_rt_bandwidth(struct rt_bandwidth *rt_b, u64 period, u64 runtime);
void init_dl_bandwidth(struct dl_bandwidth *dl_b, u64 period, u64 runtime);

/* CPU frequency and power management */
void cpufreq_update_policy(unsigned int cpu);
int sched_setscheduler_cpufreq(unsigned int cpu, unsigned int policy);
void sched_update_cpu_capacity(unsigned int cpu);

/* AI workload prediction */
void ai_predict_workload(struct task_struct *task);
void ai_update_prediction(struct task_struct *task, uint64_t runtime_ns);
float ai_get_cpu_assignment_score(struct task_struct *task, unsigned int cpu);
void ai_train_scheduler_model(void);

/* NUMA awareness */
void task_numa_fault(int last_cpupid, int mem_node, int pages, int flags);
void task_numa_placement(void);
int migrate_task_to(struct task_struct *p, int target_cpu);

/* Debugging and statistics */
void sched_show_task(struct task_struct *p);
void show_state_filter(unsigned long state_filter);
void sysrq_sched_debug_show(void);
void print_scheduler_statistics(void);

/* Preemption control */
#define preempt_disable() \
    do { \
        inc_preempt_count(); \
        barrier(); \
    } while (0)

#define preempt_enable() \
    do { \
        barrier(); \
        if (unlikely(dec_preempt_count_and_test())) \
            preempt_schedule(); \
    } while (0)

#define preempt_enable_no_resched() \
    do { \
        barrier(); \
        dec_preempt_count(); \
    } while (0)

/* Scheduling class declarations */
extern const struct sched_class stop_sched_class;
extern const struct sched_class dl_sched_class;
extern const struct sched_class rt_sched_class;
extern const struct sched_class fair_sched_class;
extern const struct sched_class idle_sched_class;

/* Utility macros */
#define SCHED_FEAT_NORMALIZED_SLEEPER   (1U << 0)
#define SCHED_FEAT_FAIR_SLEEPERS        (1U << 1)
#define SCHED_FEAT_GENTLE_FAIR_SLEEPERS (1U << 2)
#define SCHED_FEAT_START_DEBIT          (1U << 3)
#define SCHED_FEAT_NEXT_BUDDY           (1U << 4)
#define SCHED_FEAT_LAST_BUDDY           (1U << 5)
#define SCHED_FEAT_CACHE_HOT_BUDDY      (1U << 6)
#define SCHED_FEAT_WAKEUP_PREEMPTION    (1U << 7)
#define SCHED_FEAT_HRTICK               (1U << 8)
#define SCHED_FEAT_DOUBLE_TICK          (1U << 9)
#define SCHED_FEAT_LB_BIAS              (1U << 10)

extern unsigned int sysctl_sched_features;

#define sched_feat(x) !!(sysctl_sched_features & (x))

/* Per-CPU accessors */
DECLARE_PER_CPU_SHARED_ALIGNED(struct rq, runqueues);

#define cpu_rq(cpu)             (&per_cpu(runqueues, (cpu)))
#define this_rq()              this_cpu_ptr(&runqueues)
#define task_rq(p)             cpu_rq(task_cpu(p))
#define cpu_curr(cpu)          (cpu_rq(cpu)->curr)
#define raw_rq()               raw_cpu_ptr(&runqueues)

static inline int task_current(struct rq *rq, struct task_struct *p)
{
    return rq->curr == p;
}

static inline int task_running(struct rq *rq, struct task_struct *p)
{
    return task_current(rq, p) && p->on_rq;
}

static inline u64 rq_clock(struct rq *rq)
{
    return rq->clock;
}

static inline u64 rq_clock_task(struct rq *rq)
{
    return rq->clock_task;
}