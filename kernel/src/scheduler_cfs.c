/*
 * LimitlessOS Complete Fair Scheduler (CFS) Implementation
 * Production-grade process scheduler with enterprise features
 * 
 * Features:
 * - Completely Fair Scheduling algorithm
 * - Real-time process support (SCHED_FIFO, SCHED_RR)
 * - CPU core parking and power management
 * - Heterogeneous CPU support (P-cores vs E-cores)
 * - NUMA-aware scheduling
 * - Load balancing across CPU cores
 * - Interactive process boosting
 * - Deadline scheduling for critical tasks
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../include/scheduler.h"
#include "../include/process.h"
#include "../include/memory.h"
#include "../include/atomic.h"
#include "../include/cpu.h"
#include "../include/numa.h"

// Red-Black tree for CFS runqueue
typedef struct rb_node {
    struct rb_node *left, *right, *parent;
    int color;
} rb_node_t;

typedef struct rb_tree {
    rb_node_t *root;
    rb_node_t *leftmost;  // For O(1) next task selection
} rb_tree_t;

// Per-CPU runqueue structure
typedef struct cpu_runqueue {
    uint32_t cpu_id;
    uint64_t clock;               // CPU-local clock
    uint64_t prev_clock_raw;      // Previous raw timestamp
    
    // CFS runqueue
    rb_tree_t cfs_tree;
    uint32_t cfs_nr_running;      // Number of CFS tasks
    uint64_t min_vruntime;        // Minimum virtual runtime
    uint64_t cfs_load_weight;     // Total load weight
    
    // Real-time runqueue
    struct rt_runqueue {
        uint32_t rt_nr_running;
        uint32_t rt_throttled;
        uint64_t rt_time;
        uint64_t rt_runtime;
        struct list_head queue[MAX_RT_PRIO];
        unsigned long rt_nr_migratory;
    } rt;
    
    // Deadline runqueue
    struct dl_runqueue {
        rb_tree_t dl_tree;
        uint32_t dl_nr_running;
        uint64_t earliest_dl;
    } dl;
    
    // Load balancing
    uint32_t nr_running;          // Total tasks on this CPU
    uint32_t nr_switches;         // Context switch count
    uint64_t avg_idle;            // Average idle time
    
    // CPU characteristics
    uint32_t cpu_capacity;        // CPU performance capacity (1024 = baseline)
    uint32_t cpu_power;           // Current power state
    bool is_performance_core;     // P-core vs E-core
    uint32_t numa_node;           // NUMA node ID
    
    // Locks and synchronization
    spinlock_t lock;
    struct task_struct *curr;     // Currently running task
    struct task_struct *idle;     // Idle task for this CPU
    struct task_struct *stop;     // Stop machine task
    
    // Power management
    bool online;                  // CPU is online
    bool parked;                  // CPU is parked for power saving
    uint64_t last_decay;          // Last load decay timestamp
} cpu_runqueue_t;

// Global scheduler state
typedef struct scheduler_state {
    cpu_runqueue_t *cpu_rq;       // Per-CPU runqueues
    uint32_t nr_cpus;             // Number of CPUs
    uint32_t nr_numa_nodes;       // Number of NUMA nodes
    
    // Global load balancing
    uint64_t last_balance;        // Last load balance timestamp
    uint32_t balance_interval;    // Load balance interval (jiffies)
    
    // Real-time bandwidth control
    uint64_t rt_bandwidth_ns;     // RT bandwidth per second
    uint64_t rt_period_ns;        // RT period length
    
    // CFS tuning parameters
    uint32_t sched_latency_ns;    // Target preemption latency
    uint32_t sched_min_granularity_ns; // Minimum scheduling granularity
    uint32_t sched_wakeup_granularity_ns; // Wakeup preemption granularity
    
    // CPU topology
    struct cpu_topology {
        uint32_t *performance_cores; // List of P-core IDs
        uint32_t nr_perf_cores;
        uint32_t *efficiency_cores;  // List of E-core IDs  
        uint32_t nr_eff_cores;
        uint32_t *numa_domains;      // NUMA node topology
    } topology;
    
    atomic_t total_forks;         // Total processes created
    atomic_t nr_running;          // Total running processes
    atomic_t nr_uninterruptible;  // Uninterruptible sleep processes
} scheduler_state_t;

// Task scheduling entity for CFS
typedef struct sched_entity {
    rb_node_t run_node;           // Red-black tree node
    uint64_t vruntime;            // Virtual runtime
    uint64_t prev_sum_exec_runtime; // Previous sum of execution time
    uint64_t sum_exec_runtime;    // Total execution time
    uint64_t exec_start;          // Execution start time
    
    uint32_t load_weight;         // Load weight for this task
    uint32_t inv_weight;          // Inverse weight (cached)
    
    struct sched_entity *parent;  // Parent in group scheduling
    struct cfs_runqueue *cfs_rq;  // CFS runqueue this entity is on
    struct cfs_runqueue *my_q;    // Runqueue this entity owns (for groups)
} sched_entity_t;

// Real-time scheduling entity
typedef struct sched_rt_entity {
    struct list_head run_list;
    unsigned long timeout;
    unsigned long watchdog_stamp;
    unsigned int time_slice;
    
    struct sched_rt_entity *back;
    struct sched_rt_entity *parent;
    struct rt_runqueue *rt_rq;
    struct rt_runqueue *my_q;
} sched_rt_entity_t;

// Deadline scheduling entity
typedef struct sched_dl_entity {
    rb_node_t rb_node;
    uint64_t dl_runtime;          // Remaining runtime
    uint64_t dl_deadline;         // Absolute deadline
    uint64_t dl_period;           // Period length
    uint64_t dl_bw;               // Utilization
    
    int dl_throttled;
    int dl_new;
    int dl_boosted;
    int dl_yielded;
    
    struct hrtimer dl_timer;      // Deadline timer
} sched_dl_entity_t;

// Global scheduler instance
static scheduler_state_t scheduler;

// CFS constants and tuning
#define SCHED_LATENCY_NS        (6000000ULL)    // 6ms target latency
#define SCHED_MIN_GRANULARITY   (750000ULL)     // 0.75ms minimum granularity
#define SCHED_WAKEUP_GRANULARITY (1000000ULL)   // 1ms wakeup granularity
#define NICE_0_LOAD             (1024)
#define NICE_0_SHIFT            (10)

// Load weight table based on nice values (-20 to +19)
static const uint32_t prio_to_weight[40] = {
    /* -20 */ 88761, 71755, 56483, 46273, 36291,
    /* -15 */ 29154, 23254, 18705, 14949, 11916,
    /* -10 */ 9548,  7620,  6100,  4904,  3906,
    /*  -5 */ 3121,  2501,  1991,  1586,  1277,
    /*   0 */ 1024,  820,   655,   526,   423,
    /*   5 */ 335,   272,   215,   172,   137,
    /*  10 */ 110,   87,    70,    56,    45,
    /*  15 */ 36,    29,    23,    18,    15,
};

// Inverse weight table for multiplication optimization
static const uint32_t prio_to_wmult[40] = {
    /* -20 */ 48388, 59856, 76040, 92818, 118348,
    /* -15 */ 147320, 184698, 229616, 287308, 360437,
    /* -10 */ 449829, 563644, 704093, 875809, 1099582,
    /*  -5 */ 1376151, 1717300, 2157191, 2708050, 3363326,
    /*   0 */ 4194304, 5237765, 6557202, 8165337, 10153587,
    /*   5 */ 12820798, 15790321, 19976592, 24970740, 31350126,
    /*  10 */ 39045157, 49367440, 61356676, 76695844, 95443717,
    /*  15 */ 119304647, 148102320, 186737708, 238609294, 286331153,
};

// Forward declarations
static void update_curr(cpu_runqueue_t *rq);
static void enqueue_task_cfs(cpu_runqueue_t *rq, struct task_struct *p, int flags);
static void dequeue_task_cfs(cpu_runqueue_t *rq, struct task_struct *p, int flags);
static struct task_struct *pick_next_task_cfs(cpu_runqueue_t *rq);
static void put_prev_task_cfs(cpu_runqueue_t *rq, struct task_struct *p);
static void task_tick_cfs(cpu_runqueue_t *rq, struct task_struct *p, int queued);
static void switched_to_cfs(cpu_runqueue_t *rq, struct task_struct *p);
static void prio_changed_cfs(cpu_runqueue_t *rq, struct task_struct *p, int oldprio);

// Load balancing functions
static void update_cpu_capacity(cpu_runqueue_t *rq);
static void trigger_load_balance(cpu_runqueue_t *rq);
static int find_busiest_group(struct sched_domain *sd, struct cpumask *cpus);
static void active_load_balance_cpu(int busiest_cpu);

// NUMA-aware scheduling
static int numa_preferred_nid(struct task_struct *p);
static void update_numa_stats(struct task_struct *p);
static int task_numa_migrate(struct task_struct *p);

// Power management integration
static void update_cpu_power_state(uint32_t cpu_id, uint32_t power_state);
static bool should_park_cpu(uint32_t cpu_id);
static void park_cpu(uint32_t cpu_id);
static void unpark_cpu(uint32_t cpu_id);

/*
 * Initialize the Complete Fair Scheduler
 */
int scheduler_init(void)
{
    uint32_t cpu;
    
    // Initialize global scheduler state
    scheduler.nr_cpus = get_num_cpus();
    scheduler.nr_numa_nodes = get_num_numa_nodes();
    
    // Allocate per-CPU runqueues
    scheduler.cpu_rq = kmalloc(sizeof(cpu_runqueue_t) * scheduler.nr_cpus, 
                              GFP_KERNEL);
    if (!scheduler.cpu_rq) {
        return -ENOMEM;
    }
    
    // Set CFS tuning parameters
    scheduler.sched_latency_ns = SCHED_LATENCY_NS;
    scheduler.sched_min_granularity_ns = SCHED_MIN_GRANULARITY;
    scheduler.sched_wakeup_granularity_ns = SCHED_WAKEUP_GRANULARITY;
    
    // RT bandwidth control (95% CPU for RT tasks by default)
    scheduler.rt_bandwidth_ns = 950000000ULL; // 950ms per second
    scheduler.rt_period_ns = 1000000000ULL;   // 1 second period
    
    // Initialize per-CPU runqueues
    for (cpu = 0; cpu < scheduler.nr_cpus; cpu++) {
        cpu_runqueue_t *rq = &scheduler.cpu_rq[cpu];
        
        rq->cpu_id = cpu;
        rq->clock = 0;
        rq->prev_clock_raw = 0;
        
        // Initialize CFS runqueue
        rq->cfs_tree.root = NULL;
        rq->cfs_tree.leftmost = NULL;
        rq->cfs_nr_running = 0;
        rq->min_vruntime = 0;
        rq->cfs_load_weight = 0;
        
        // Initialize RT runqueue
        rq->rt.rt_nr_running = 0;
        rq->rt.rt_throttled = 0;
        rq->rt.rt_time = 0;
        rq->rt.rt_runtime = scheduler.rt_bandwidth_ns;
        rq->rt.rt_nr_migratory = 0;
        
        for (int i = 0; i < MAX_RT_PRIO; i++) {
            INIT_LIST_HEAD(&rq->rt.queue[i]);
        }
        
        // Initialize deadline runqueue
        rq->dl.dl_tree.root = NULL;
        rq->dl.dl_tree.leftmost = NULL;
        rq->dl.dl_nr_running = 0;
        rq->dl.earliest_dl = 0;
        
        // Initialize CPU characteristics
        rq->cpu_capacity = 1024; // Default capacity
        rq->cpu_power = CPU_POWER_NORMAL;
        rq->is_performance_core = is_performance_core(cpu);
        rq->numa_node = cpu_to_numa_node(cpu);
        
        // Initialize load balancing
        rq->nr_running = 0;
        rq->nr_switches = 0;
        rq->avg_idle = 0;
        rq->last_decay = 0;
        
        // Initialize locks
        spin_lock_init(&rq->lock);
        
        // Power management
        rq->online = true;
        rq->parked = false;
        
        // Create idle task for this CPU
        rq->idle = create_idle_task(cpu);
        rq->curr = rq->idle;
        rq->stop = NULL;
    }
    
    // Detect CPU topology for heterogeneous scheduling
    detect_cpu_topology(&scheduler.topology);
    
    // Initialize global counters
    atomic_set(&scheduler.total_forks, 0);
    atomic_set(&scheduler.nr_running, 0);
    atomic_set(&scheduler.nr_uninterruptible, 0);
    
    scheduler.last_balance = get_jiffies();
    scheduler.balance_interval = msecs_to_jiffies(10); // 10ms interval
    
    printk(KERN_INFO "LimitlessOS CFS Scheduler initialized: %u CPUs, %u NUMA nodes\n",
           scheduler.nr_cpus, scheduler.nr_numa_nodes);
    
    return 0;
}

/*
 * Update the current task's runtime statistics
 */
static void update_curr(cpu_runqueue_t *rq)
{
    struct task_struct *curr = rq->curr;
    uint64_t now = rq->clock;
    uint64_t delta_exec;
    
    if (unlikely(!curr || curr == rq->idle))
        return;
    
    delta_exec = now - curr->se.exec_start;
    if (unlikely(delta_exec <= 0))
        return;
    
    curr->se.exec_start = now;
    curr->se.sum_exec_runtime += delta_exec;
    
    // Update virtual runtime for CFS tasks
    if (curr->sched_class == &fair_sched_class) {
        curr->se.vruntime += calc_delta_fair(delta_exec, &curr->se);
        
        // Ensure vruntime doesn't fall too far behind
        curr->se.vruntime = max(curr->se.vruntime, rq->min_vruntime);
        
        // Update minimum vruntime
        update_min_vruntime(rq);
    }
    
    // Account for CPU time in various statistics
    account_user_time(curr, delta_exec);
    account_system_time(curr, delta_exec);
    
    // Update CPU capacity based on current frequency
    update_cpu_capacity(rq);
}

/*
 * Calculate fair delta for CFS virtual runtime
 */
static uint64_t calc_delta_fair(uint64_t delta, struct sched_entity *se)
{
    if (unlikely(se->load_weight != NICE_0_LOAD))
        return calc_delta_mine(delta, NICE_0_LOAD, &se->load);
    
    return delta;
}

/*
 * Calculate weighted delta for load balancing
 */
static uint64_t calc_delta_mine(uint64_t delta_exec, unsigned long weight,
                               struct load_weight *lw)
{
    uint64_t fact = scale_load_down(weight);
    int shift = WMULT_SHIFT;
    
    __update_inv_weight(lw);
    
    if (unlikely(fact >> 32)) {
        while (fact >> 32) {
            fact >>= 1;
            shift--;
        }
    }
    
    /* hint to use a 32x32->64 mul */
    fact = (uint64_t)(uint32_t)fact * lw->inv_weight;
    
    while (fact >> 32) {
        fact >>= 1;
        shift--;
    }
    
    return mul_u64_u32_shr(delta_exec, fact, shift);
}

/*
 * Update minimum virtual runtime for the CFS runqueue
 */
static void update_min_vruntime(cpu_runqueue_t *rq)
{
    struct task_struct *curr = rq->curr;
    rb_node_t *leftmost = rq->cfs_tree.leftmost;
    uint64_t vruntime = rq->min_vruntime;
    
    if (curr && curr->sched_class == &fair_sched_class) {
        vruntime = curr->se.vruntime;
    }
    
    if (leftmost) {
        struct sched_entity *se = rb_entry(leftmost, struct sched_entity, run_node);
        
        if (!curr || (curr->sched_class != &fair_sched_class))
            vruntime = se->vruntime;
        else
            vruntime = min(vruntime, se->vruntime);
    }
    
    /* ensure we never gain time by being placed backwards. */
    rq->min_vruntime = max(rq->min_vruntime, vruntime);
}

/*
 * Enqueue a task in the CFS runqueue
 */
static void enqueue_task_cfs(cpu_runqueue_t *rq, struct task_struct *p, int flags)
{
    struct sched_entity *se = &p->se;
    
    // Place the task in the tree
    if (!(flags & ENQUEUE_WAKEUP) || (flags & ENQUEUE_WAKING))
        se->vruntime += rq->min_vruntime;
    
    update_curr(rq);
    
    // Insert into red-black tree
    __enqueue_entity(rq, se);
    rq->cfs_nr_running++;
    rq->nr_running++;
    
    // Update load weight
    rq->cfs_load_weight += se->load_weight;
    
    // NUMA balancing hint
    if (flags & ENQUEUE_WAKEUP)
        update_numa_stats(p);
    
    // Trigger load balancing if needed
    if (rq->nr_running > 1)
        check_preempt_curr(rq, p, flags);
}

/*
 * Dequeue a task from the CFS runqueue
 */
static void dequeue_task_cfs(cpu_runqueue_t *rq, struct task_struct *p, int flags)
{
    struct sched_entity *se = &p->se;
    
    update_curr(rq);
    
    // Remove from red-black tree
    __dequeue_entity(rq, se);
    rq->cfs_nr_running--;
    rq->nr_running--;
    
    // Update load weight
    rq->cfs_load_weight -= se->load_weight;
    
    // Normalize vruntime if task is sleeping
    if (!(flags & DEQUEUE_SLEEP))
        se->vruntime -= rq->min_vruntime;
}

/*
 * Pick the next task to run from CFS runqueue
 */
static struct task_struct *pick_next_task_cfs(cpu_runqueue_t *rq)
{
    struct sched_entity *se;
    struct task_struct *p;
    rb_node_t *left = rq->cfs_tree.leftmost;
    
    if (!left)
        return NULL;
    
    se = rb_entry(left, struct sched_entity, run_node);
    p = task_of(se);
    
    se->exec_start = rq->clock;
    
    return p;
}

/*
 * Put the previous task back into the runqueue
 */
static void put_prev_task_cfs(cpu_runqueue_t *rq, struct task_struct *p)
{
    struct sched_entity *se = &p->se;
    
    if (rq->cfs_nr_running) {
        update_curr(rq);
        
        // Place the task back in the tree
        __enqueue_entity(rq, se);
    }
}

/*
 * Scheduler tick for CFS tasks
 */
static void task_tick_cfs(cpu_runqueue_t *rq, struct task_struct *p, int queued)
{
    struct sched_entity *se = &p->se;
    
    update_curr(rq);
    
    // Check if we need to preempt
    if (rq->cfs_nr_running > 1)
        check_preempt_tick(rq, p);
    
    // Update load average
    update_cfs_load_avg(se);
    
    // Trigger load balancing periodically
    if (time_after(jiffies, rq->next_balance))
        trigger_load_balance(rq);
}

/*
 * Check if current task should be preempted
 */
static void check_preempt_tick(cpu_runqueue_t *rq, struct task_struct *curr)
{
    unsigned long ideal_runtime, delta_exec;
    struct sched_entity *se = &curr->se;
    s64 delta;
    
    ideal_runtime = sched_slice(rq, se);
    delta_exec = se->sum_exec_runtime - se->prev_sum_exec_runtime;
    
    if (delta_exec > ideal_runtime) {
        resched_curr(rq);
        return;
    }
    
    // Check against leftmost task
    if (rq->cfs_tree.leftmost) {
        struct sched_entity *left_se = rb_entry(rq->cfs_tree.leftmost,
                                               struct sched_entity, run_node);
        
        delta = curr->se.vruntime - left_se->vruntime;
        
        if (delta > (s64)scheduler.sched_wakeup_granularity_ns)
            resched_curr(rq);
    }
}

/*
 * Calculate time slice for a task
 */
static uint64_t sched_slice(cpu_runqueue_t *rq, struct sched_entity *se)
{
    uint64_t slice = __sched_period(rq->cfs_nr_running + !se->on_rq);
    
    for_each_sched_entity(se) {
        struct load_weight *load;
        struct load_weight lw;
        
        cfs_rq = cfs_rq_of(se);
        load = &cfs_rq->load;
        
        if (unlikely(!se->on_rq)) {
            lw = cfs_rq->load;
            
            update_load_add(&lw, se->load.weight);
            load = &lw;
        }
        
        slice = __calc_delta(slice, se->load.weight, load);
    }
    
    return slice;
}

/*
 * Main scheduler entry point - schedule next task
 */
void __schedule(bool preempt)
{
    struct task_struct *prev, *next;
    cpu_runqueue_t *rq;
    uint32_t cpu;
    unsigned long flags;
    
    cpu = smp_processor_id();
    rq = &scheduler.cpu_rq[cpu];
    prev = rq->curr;
    
    local_irq_save(flags);
    spin_lock(&rq->lock);
    
    // Update clock
    update_rq_clock(rq);
    
    // Update current task
    update_curr(rq);
    
    // Clear preempt flag
    clear_tsk_need_resched(prev);
    
    if (prev->state && !(preempt && prev->state == TASK_RUNNING)) {
        if (unlikely(signal_pending_state(prev->state, prev))) {
            prev->state = TASK_RUNNING;
        } else {
            deactivate_task(rq, prev, DEQUEUE_SLEEP);
            prev->on_rq = 0;
        }
    }
    
    // Pick next task based on scheduling class priority
    if (rq->dl.dl_nr_running) {
        next = pick_next_task_dl(rq);
    } else if (rq->rt.rt_nr_running) {
        next = pick_next_task_rt(rq);
    } else {
        next = pick_next_task_cfs(rq);
        if (!next)
            next = rq->idle;
    }
    
    if (likely(prev != next)) {
        rq->nr_switches++;
        rq->curr = next;
        
        // Perform context switch
        context_switch(rq, prev, next);
        
        // Update NUMA statistics
        if (next->numa_preferred_nid != -1)
            update_numa_stats(next);
    } else {
        spin_unlock_irq(&rq->lock);
    }
    
    local_irq_restore(flags);
}

/*
 * Wake up a sleeping task
 */
int try_to_wake_up(struct task_struct *p, unsigned int state, int wake_flags)
{
    unsigned long flags;
    int cpu, orig_cpu, this_cpu, success = 0;
    cpu_runqueue_t *rq;
    
    rq = task_rq_lock(p, &flags);
    
    if (!(p->state & state))
        goto out;
    
    success = 1;
    cpu = task_cpu(p);
    orig_cpu = cpu;
    this_cpu = smp_processor_id();
    
    // Select best CPU for the task
    if (p->sched_class->select_task_rq)
        cpu = p->sched_class->select_task_rq(p, cpu, SD_BALANCE_WAKE, wake_flags);
    
    if (task_cpu(p) != cpu) {
        wake_flags |= WF_MIGRATED;
        set_task_cpu(p, cpu);
    }
    
    // Activate the task
    ttwu_activate(rq, p, ENQUEUE_WAKEUP | ENQUEUE_WAKING);
    ttwu_do_wakeup(rq, p, wake_flags);
    
out:
    task_rq_unlock(rq, p, &flags);
    
    return success;
}

/*
 * Load balancing - find best CPU for task placement
 */
static int select_task_rq_fair(struct task_struct *p, int prev_cpu, int sd_flag, int wake_flags)
{
    int cpu, new_cpu = prev_cpu;
    int want_affine = 0;
    int sync = wake_flags & WF_SYNC;
    
    if (sd_flag & SD_BALANCE_WAKE) {
        record_wakee(p);
        want_affine = !wake_wide(p) && cpumask_test_cpu(cpu, &p->cpus_allowed);
    }
    
    rcu_read_lock();
    for_each_domain(cpu, tmp) {
        if (!(tmp->flags & SD_LOAD_BALANCE))
            break;
        
        // NUMA-aware placement
        if (tmp->flags & SD_NUMA) {
            int numa_cpu = find_numa_cpu(p, prev_cpu);
            if (numa_cpu != -1) {
                new_cpu = numa_cpu;
                break;
            }
        }
        
        // Performance vs efficiency core placement
        if (tmp->flags & SD_ASYM_CPUCAPACITY) {
            new_cpu = find_energy_efficient_cpu(p, prev_cpu);
            break;
        }
        
        if (want_affine && (tmp->flags & SD_WAKE_AFFINE) &&
            cpumask_test_cpu(prev_cpu, sched_domain_span(tmp))) {
            if (wake_affine(tmp, p, cpu, prev_cpu, sync))
                new_cpu = cpu;
            break;
        }
        
        if (tmp->flags & sd_flag)
            new_cpu = find_idlest_cpu(tmp, p, cpu, prev_cpu, sd_flag);
    }
    rcu_read_unlock();
    
    return new_cpu;
}

/*
 * Find energy-efficient CPU for heterogeneous systems
 */
static int find_energy_efficient_cpu(struct task_struct *p, int prev_cpu)
{
    unsigned long cur_energy, prev_energy, best_energy = ULONG_MAX;
    int cpu, best_cpu = prev_cpu;
    
    // For CPU-intensive tasks, prefer performance cores
    if (p->se.avg.util_avg > 750) { // High utilization threshold
        for_each_cpu(cpu, &scheduler.topology.performance_cores) {
            if (!cpumask_test_cpu(cpu, &p->cpus_allowed))
                continue;
            
            cur_energy = compute_energy(p, cpu);
            if (cur_energy < best_energy) {
                best_energy = cur_energy;
                best_cpu = cpu;
            }
        }
    } else {
        // For light tasks, prefer efficiency cores
        for_each_cpu(cpu, &scheduler.topology.efficiency_cores) {
            if (!cpumask_test_cpu(cpu, &p->cpus_allowed))
                continue;
            
            cur_energy = compute_energy(p, cpu);
            if (cur_energy < best_energy) {
                best_energy = cur_energy;
                best_cpu = cpu;
            }
        }
    }
    
    return best_cpu;
}

/*
 * NUMA-aware CPU selection
 */
static int find_numa_cpu(struct task_struct *p, int prev_cpu)
{
    int preferred_nid = numa_preferred_nid(p);
    int cpu;
    
    if (preferred_nid == -1)
        return -1;
    
    // Find idle CPU in preferred NUMA node
    for_each_cpu_and(cpu, cpumask_of_node(preferred_nid), &p->cpus_allowed) {
        if (idle_cpu(cpu))
            return cpu;
    }
    
    // Find least loaded CPU in preferred NUMA node
    cpu = cpumask_first_and(cpumask_of_node(preferred_nid), &p->cpus_allowed);
    if (cpu < nr_cpu_ids)
        return cpu;
    
    return -1;
}

/*
 * Load balancing across CPUs
 */
static void trigger_load_balance(cpu_runqueue_t *rq)
{
    int this_cpu = rq->cpu_id;
    
    if (time_before(jiffies, rq->next_balance))
        return;
    
    if (rq->idle_balance) {
        // Idle balancing
        idle_balance(this_cpu, rq);
    } else {
        // Active balancing
        if (rq->nr_running > 1) {
            int busiest_cpu = find_busiest_cpu(this_cpu);
            if (busiest_cpu != -1)
                active_load_balance_cpu(busiest_cpu);
        }
    }
    
    rq->next_balance = jiffies + scheduler.balance_interval;
}

/*
 * CPU power management integration
 */
static void update_cpu_power_state(uint32_t cpu_id, uint32_t power_state)
{
    cpu_runqueue_t *rq = &scheduler.cpu_rq[cpu_id];
    
    rq->cpu_power = power_state;
    
    // Update CPU capacity based on power state
    switch (power_state) {
        case CPU_POWER_LOW:
            rq->cpu_capacity = 512;  // 50% capacity
            break;
        case CPU_POWER_NORMAL:
            rq->cpu_capacity = 1024; // 100% capacity
            break;
        case CPU_POWER_HIGH:
            rq->cpu_capacity = 1536; // 150% capacity (turbo)
            break;
    }
    
    // Trigger load rebalancing
    trigger_load_balance(rq);
}

/*
 * CPU parking for power efficiency
 */
static bool should_park_cpu(uint32_t cpu_id)
{
    cpu_runqueue_t *rq = &scheduler.cpu_rq[cpu_id];
    
    // Don't park CPU 0
    if (cpu_id == 0)
        return false;
    
    // Don't park if CPU has running tasks
    if (rq->nr_running > 0)
        return false;
    
    // Park efficiency cores first
    if (!rq->is_performance_core && system_load_low())
        return true;
    
    return false;
}

/*
 * Real-time scheduling support
 */
static struct task_struct *pick_next_task_rt(cpu_runqueue_t *rq)
{
    struct task_struct *p;
    struct rt_runqueue *rt_rq = &rq->rt;
    int idx;
    
    if (!rt_rq->rt_nr_running)
        return NULL;
    
    idx = sched_find_first_bit(rt_rq->active.bitmap);
    BUG_ON(idx >= MAX_RT_PRIO);
    
    p = list_entry(rt_rq->active.queue[idx].next,
                   struct task_struct, rt.run_list);
    
    p->se.exec_start = rq->clock;
    
    return p;
}

/*
 * Deadline scheduling support  
 */
static struct task_struct *pick_next_task_dl(cpu_runqueue_t *rq)
{
    struct sched_dl_entity *dl_se;
    struct task_struct *p;
    rb_node_t *left;
    
    if (!rq->dl.dl_nr_running)
        return NULL;
    
    left = rq->dl.dl_tree.leftmost;
    if (!left)
        return NULL;
    
    dl_se = rb_entry(left, struct sched_dl_entity, rb_node);
    p = dl_task_of(dl_se);
    
    p->se.exec_start = rq->clock;
    
    return p;
}

/*
 * System call interface for setting scheduling policy
 */
int sys_sched_setscheduler(pid_t pid, int policy, 
                          const struct sched_param *param)
{
    struct task_struct *p;
    int retval;
    
    if (policy < 0 || policy > SCHED_MAX)
        return -EINVAL;
    
    rcu_read_lock();
    p = find_process_by_pid(pid);
    if (!p) {
        rcu_read_unlock();
        return -ESRCH;
    }
    
    retval = security_task_setscheduler(p);
    if (retval) {
        rcu_read_unlock();
        return retval;
    }
    
    get_task_struct(p);
    rcu_read_unlock();
    
    retval = __sched_setscheduler(p, policy, param, true);
    put_task_struct(p);
    
    return retval;
}

/*
 * Export scheduler statistics for monitoring
 */
void scheduler_get_stats(struct sched_stats *stats)
{
    uint32_t cpu;
    
    memset(stats, 0, sizeof(struct sched_stats));
    
    stats->nr_cpus = scheduler.nr_cpus;
    stats->total_forks = atomic_read(&scheduler.total_forks);
    stats->nr_running = atomic_read(&scheduler.nr_running);
    stats->nr_uninterruptible = atomic_read(&scheduler.nr_uninterruptible);
    
    for (cpu = 0; cpu < scheduler.nr_cpus; cpu++) {
        cpu_runqueue_t *rq = &scheduler.cpu_rq[cpu];
        
        stats->per_cpu[cpu].nr_running = rq->nr_running;
        stats->per_cpu[cpu].nr_switches = rq->nr_switches;
        stats->per_cpu[cpu].cpu_capacity = rq->cpu_capacity;
        stats->per_cpu[cpu].avg_idle = rq->avg_idle;
        stats->per_cpu[cpu].is_performance_core = rq->is_performance_core;
        stats->per_cpu[cpu].numa_node = rq->numa_node;
        stats->per_cpu[cpu].parked = rq->parked;
    }
}

/*
 * Shutdown scheduler - park all CPUs except CPU 0
 */
void scheduler_shutdown(void)
{
    uint32_t cpu;
    
    printk(KERN_INFO "Shutting down scheduler...\n");
    
    for (cpu = 1; cpu < scheduler.nr_cpus; cpu++) {
        park_cpu(cpu);
    }
    
    kfree(scheduler.cpu_rq);
    
    printk(KERN_INFO "Scheduler shutdown complete\n");
}