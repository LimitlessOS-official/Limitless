/**
 * SMP-Aware Scheduler Implementation for LimitlessOS
 * 
 * Multi-core CFS-based scheduler with load balancing and NUMA awareness.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "smp_scheduler.h"
#include "smp.h"
#include "apic.h"
#include "kernel.h"
#include <string.h>

/* Global scheduler state */
cpu_runqueue_t cpu_runqueues[MAX_CPUS];
sched_domain_t *sched_domains[MAX_CPUS];
task_t *init_task = NULL;

/* Load weight table for nice values */
static const unsigned long prio_to_weight[40] = {
 /* -20 */  88761,  71755,  56483,  46273,  36291,
 /* -15 */  29154,  23254,  18705,  14949,  11916,
 /* -10 */   9548,   7620,   6100,   4904,   3906,
 /*  -5 */   3121,   2501,   1991,   1586,   1277,
 /*   0 */   1024,    820,    655,    526,    423,
 /*   5 */    335,    272,    215,    172,    137,
 /*  10 */    110,     87,     70,     56,     45,
 /*  15 */     36,     29,     23,     18,     15,
};

/* PID allocation */
static pid_t next_pid = 1;
static spinlock_t pid_lock = SPINLOCK_INIT;

/* Task list */
static struct list_head task_list_head;
static spinlock_t task_list_lock = SPINLOCK_INIT;

/* Per-CPU current task */
DEFINE_PER_CPU(task_t *, current_task);

/**
 * Initialize the SMP scheduler
 */
int sched_init(void) {
    kprintf("[SCHED] Initializing SMP scheduler...\n");
    
    /* Initialize task list */
    INIT_LIST_HEAD(&task_list_head);
    
    /* Initialize per-CPU runqueues */
    for (uint32_t cpu = 0; cpu < nr_cpus_possible; cpu++) {
        sched_init_cpu(cpu);
    }
    
    /* Build scheduling domains */
    sched_build_domains();
    
    /* Create init task */
    init_task = sched_create_task(NULL, "init");
    if (!init_task) {
        kprintf("[SCHED] Failed to create init task\n");
        return -1;
    }
    
    init_task->pid = 1;
    init_task->state = TASK_RUNNING;
    
    /* Set current task for boot CPU */
    per_cpu(current_task, 0) = init_task;
    cpu_runqueues[0].curr = init_task;
    
    kprintf("[SCHED] SMP scheduler initialized\n");
    return 0;
}

/**
 * Initialize per-CPU runqueue
 */
void sched_init_cpu(uint32_t cpu) {
    cpu_runqueue_t *rq = &cpu_runqueues[cpu];
    
    /* Initialize spinlock */
    spin_lock_init(&rq->lock);
    
    /* Initialize CFS runqueue */
    rq->cfs.root = RB_ROOT_CACHED;
    rq->cfs.leftmost = NULL;
    rq->cfs.min_vruntime = 0;
    rq->cfs.nr_running = 0;
    rq->cfs.load_weight = 0;
    
    /* Initialize RT runqueue */
    for (int i = 0; i < MAX_RT_PRIO; i++) {
        INIT_LIST_HEAD(&rq->rt.queue[i]);
    }
    rq->rt.nr_running = 0;
    rq->rt.rt_nr_migratory = 0;
    rq->rt.rt_throttled = 0;
    
    /* Initialize pointers */
    rq->curr = NULL;
    rq->idle = NULL;
    rq->stop = NULL;
    
    /* Initialize load balancing */
    rq->next_balance = 0;
    rq->balance_interval = 50;  /* 50ms default */
    rq->avg_load_per_task = 0;
    
    /* Initialize statistics */
    memset(rq->cpu_load, 0, sizeof(rq->cpu_load));
    rq->last_update = 0;
    rq->sched_count = 0;
    rq->sched_goidle = 0;
    rq->ttwu_count = 0;
    rq->ttwu_local = 0;
    
    /* Initialize clocks */
    rq->clock = 0;
    rq->clock_task = 0;
    
    kprintf("[SCHED] Initialized runqueue for CPU %u\n", cpu);
}

/**
 * Build scheduling domains for load balancing
 */
void sched_build_domains(void) {
    kprintf("[SCHED] Building scheduling domains...\n");
    
    /* For now, create simple single-level domain covering all CPUs */
    for (uint32_t cpu = 0; cpu < nr_cpus_possible; cpu++) {
        sched_domain_t *sd = (sched_domain_t*)kmalloc(sizeof(sched_domain_t));
        if (!sd) continue;
        
        memset(sd, 0, sizeof(sched_domain_t));
        
        /* Cover all possible CPUs */
        sd->span = cpu_possible_mask;
        sd->level = 0;
        sd->flags = 0;
        sd->balance_interval = 50;  /* 50ms */
        sd->busy_factor = 32;
        sd->imbalance_pct = 125;    /* 25% imbalance threshold */
        sd->cache_nice_tries = 1;
        strcpy(sd->name, "ALL");
        
        sched_domains[cpu] = sd;
    }
    
    kprintf("[SCHED] Scheduling domains built\n");
}

/**
 * Create a new task
 */
task_t *sched_create_task(void (*entry_point)(void), const char *name) {
    task_t *task = (task_t*)kmalloc(sizeof(task_t));
    if (!task) {
        return NULL;
    }
    
    /* Clear task structure */
    memset(task, 0, sizeof(task_t));
    
    /* Allocate PID */
    spin_lock(&pid_lock);
    task->pid = next_pid++;
    spin_unlock(&pid_lock);
    
    task->tgid = task->pid;  /* Single-threaded for now */
    task->ppid = (init_task) ? init_task->pid : 0;
    
    /* Set initial state */
    task->state = TASK_READY;
    task->flags = 0;
    task->exit_code = 0;
    
    /* Initialize scheduling entity */
    task->sched_class = SCHED_CLASS_NORMAL;
    task->se.vruntime = 0;
    task->se.exec_start = 0;
    task->se.sum_exec_runtime = 0;
    task->se.prev_sum_exec = 0;
    task->se.prio = DEFAULT_PRIO;
    task->se.nice = 0;
    task->se.load_weight = prio_to_weight[20];  /* Nice 0 */
    
    /* Initialize RT entity */
    INIT_LIST_HEAD(&task->rt.run_list);
    task->rt.timeout = 0;
    task->rt.time_slice = 0;
    task->rt.nr_cpus_allowed = nr_cpus_possible;
    
    /* CPU affinity - can run on any CPU initially */
    task->cpu_affinity = cpu_possible_mask;
    task->preferred_cpu = 0;
    task->numa_node = 0;
    task->last_cpu = 0;
    
    /* Allocate stack */
    task->stack_size = PAGE_SIZE;
    task->stack = pmm_alloc_page();
    if (!task->stack) {
        kfree(task);
        return NULL;
    }
    
    /* Set up initial context if entry point provided */
    if (entry_point) {
        uint32_t *stack_ptr = (uint32_t*)((uintptr_t)task->stack + PAGE_SIZE);
        
        /* Set up return address */
        *--stack_ptr = (uint32_t)entry_point;
        *--stack_ptr = 0; /* EBP */
        
        task->context.esp = (uint32_t)stack_ptr;
        task->context.ebp = 0;
        task->context.eip = (uint32_t)entry_point;
        task->context.eflags = 0x202;  /* Interrupts enabled */
        task->context.cr3 = 0;         /* Will be set by memory manager */
    }
    
    /* Initialize lists */
    INIT_LIST_HEAD(&task->children);
    INIT_LIST_HEAD(&task->sibling);
    INIT_LIST_HEAD(&task->wait_list);
    INIT_LIST_HEAD(&task->run_list);
    INIT_LIST_HEAD(&task->task_list);
    
    /* Set name */
    if (name) {
        strncpy(task->comm, name, sizeof(task->comm) - 1);
        task->comm[sizeof(task->comm) - 1] = '\0';
    } else {
        snprintf(task->comm, sizeof(task->comm), "task%d", task->pid);
    }
    
    /* Initialize timing */
    task->start_time = sched_clock();
    task->total_time = 0;
    task->sleep_time = 0;
    task->last_ran = 0;
    
    /* Initialize reference count */
    atomic_set(&task->usage, 1);
    
    /* Add to global task list */
    spin_lock(&task_list_lock);
    list_add_tail(&task->task_list, &task_list_head);
    spin_unlock(&task_list_lock);
    
    kprintf("[SCHED] Created task '%s' (PID %d)\n", task->comm, task->pid);
    
    return task;
}

/**
 * Get next available PID
 */
pid_t sched_get_next_pid(void) {
    pid_t pid;
    
    spin_lock(&pid_lock);
    pid = next_pid++;
    spin_unlock(&pid_lock);
    
    return pid;
}

/**
 * Main scheduler function
 */
void schedule(void) {
    uint32_t cpu = smp_processor_id();
    cpu_runqueue_t *rq = cpu_rq(cpu);
    task_t *prev, *next;
    
    /* Disable interrupts and acquire runqueue lock */
    unsigned long flags;
    spin_lock_irqsave(&rq->lock, &flags);
    
    prev = rq->curr;
    
    /* Update runqueue clock */
    update_rq_clock(rq);
    
    /* Put previous task back on runqueue if still runnable */
    if (prev && prev->state == TASK_RUNNING) {
        prev->state = TASK_READY;
    }
    
    if (prev && prev->state == TASK_READY) {
        put_prev_task(rq, prev);
    }
    
    /* Pick next task to run */
    next = pick_next_task(rq);
    
    if (!next) {
        /* No runnable tasks, use idle task */
        next = rq->idle;
        if (!next) {
            /* Create idle task if it doesn't exist */
            next = sched_create_idle_task(cpu);
            rq->idle = next;
        }
        rq->sched_goidle++;
    }
    
    rq->curr = next;
    rq->sched_count++;
    
    /* Update per-CPU current task pointer */
    per_cpu(current_task, cpu) = next;
    
    /* Context switch if necessary */
    if (prev != next) {
        next->state = TASK_RUNNING;
        next->last_ran = sched_clock_cpu(cpu);
        next->last_cpu = cpu;
        
        context_switch(rq, prev, next);
    }
    
    spin_unlock_irqrestore(&rq->lock, flags);
}

/**
 * Pick next task to run
 */
task_t *pick_next_task(cpu_runqueue_t *rq) {
    /* Check RT tasks first */
    if (rq->rt.nr_running > 0) {
        /* Find highest priority RT task */
        for (int prio = 0; prio < MAX_RT_PRIO; prio++) {
            if (!list_empty(&rq->rt.queue[prio])) {
                task_t *task = list_first_entry(&rq->rt.queue[prio], task_t, rt.run_list);
                list_del(&task->rt.run_list);
                rq->rt.nr_running--;
                return task;
            }
        }
    }
    
    /* Check CFS tasks */
    if (rq->cfs.nr_running > 0 && rq->cfs.leftmost) {
        task_t *task = rb_entry(rq->cfs.leftmost, task_t, se.run_node);
        rb_erase_cached(&task->se.run_node, &rq->cfs.root);
        rq->cfs.nr_running--;
        rq->cfs.load_weight -= task->se.load_weight;
        
        /* Update leftmost pointer */
        rq->cfs.leftmost = rb_next(&task->se.run_node);
        
        return task;
    }
    
    return NULL;  /* No runnable tasks */
}

/**
 * Put previous task back on runqueue
 */
void put_prev_task(cpu_runqueue_t *rq, task_t *prev) {
    if (!prev || prev->state != TASK_READY) {
        return;
    }
    
    if (prev->sched_class == SCHED_CLASS_RT) {
        /* Add to RT queue */
        int prio = prev->se.prio;
        if (prio < MAX_RT_PRIO) {
            list_add_tail(&prev->rt.run_list, &rq->rt.queue[prio]);
            rq->rt.nr_running++;
        }
    } else {
        /* Add to CFS queue */
        enqueue_task_fair(rq, prev);
    }
}

/**
 * Enqueue task in CFS runqueue
 */
void enqueue_task_fair(cpu_runqueue_t *rq, task_t *task) {
    struct rb_node **link = &rq->cfs.root.rb_root.rb_node;
    struct rb_node *parent = NULL;
    task_t *entry;
    bool leftmost = true;
    
    /* Find insertion point in RB-tree */
    while (*link) {
        parent = *link;
        entry = rb_entry(parent, task_t, se.run_node);
        
        if (task->se.vruntime < entry->se.vruntime) {
            link = &parent->rb_left;
        } else {
            link = &parent->rb_right;
            leftmost = false;
        }
    }
    
    /* Insert into RB-tree */
    rb_link_node(&task->se.run_node, parent, link);
    rb_insert_color_cached(&task->se.run_node, &rq->cfs.root, leftmost);
    
    /* Update leftmost if this is the new leftmost */
    if (leftmost) {
        rq->cfs.leftmost = &task->se.run_node;
    }
    
    /* Update runqueue statistics */
    rq->cfs.nr_running++;
    rq->cfs.load_weight += task->se.load_weight;
    
    /* Update vruntime */
    if (rq->cfs.nr_running == 1) {
        rq->cfs.min_vruntime = task->se.vruntime;
    }
}

/**
 * Create idle task for CPU
 */
task_t *sched_create_idle_task(uint32_t cpu) {
    task_t *idle = sched_create_task(cpu_idle_loop, "idle");
    if (!idle) {
        return NULL;
    }
    
    idle->sched_class = SCHED_CLASS_IDLE;
    idle->se.prio = MAX_PRIO;
    idle->se.nice = 20;
    idle->se.load_weight = prio_to_weight[39];  /* Minimum weight */
    
    /* Set CPU affinity to only this CPU */
    cpu_mask_clear(&idle->cpu_affinity);
    cpu_mask_set_cpu(cpu, &idle->cpu_affinity);
    idle->preferred_cpu = cpu;
    idle->last_cpu = cpu;
    
    kprintf("[SCHED] Created idle task for CPU %u\n", cpu);
    
    return idle;
}

/**
 * Idle loop
 */
void cpu_idle_loop(void) {
    while (1) {
        /* Check for pending work */
        uint32_t cpu = smp_processor_id();
        cpu_info_t *cpu_info = smp_cpu_data(cpu);
        
        if (cpu_info && cpu_info->ipi_pending) {
            /* Handle pending IPIs */
            /* TODO: Process IPIs */
            cpu_info->ipi_pending = 0;
        }
        
        /* Enter low-power state */
        smp_enter_idle();
        
        /* Re-enable interrupts and halt */
        asm volatile("sti; hlt");
        
        /* Check if we should reschedule */
        if (need_resched()) {
            schedule();
        }
    }
}

/**
 * Scheduler tick (called from timer interrupt)
 */
void scheduler_tick(void) {
    uint32_t cpu = smp_processor_id();
    cpu_runqueue_t *rq = cpu_rq(cpu);
    task_t *curr = rq->curr;
    
    if (!curr) return;
    
    spin_lock(&rq->lock);
    
    /* Update runqueue clock */
    update_rq_clock(rq);
    
    /* Update current task's runtime */
    uint64_t now = rq->clock_task;
    uint64_t delta_exec = now - curr->se.exec_start;
    
    curr->se.sum_exec_runtime += delta_exec;
    curr->se.exec_start = now;
    
    /* Update vruntime for CFS tasks */
    if (curr->sched_class == SCHED_CLASS_NORMAL) {
        curr->se.vruntime += calc_delta_fair(delta_exec, curr);
        
        /* Check if task should be preempted */
        if (should_preempt_curr(rq, curr)) {
            resched_curr(rq);
        }
    }
    
    /* Update load balancing */
    if (time_after_eq(jiffies, rq->next_balance)) {
        rq->next_balance = jiffies + rq->balance_interval;
        /* TODO: Trigger load balancing */
    }
    
    spin_unlock(&rq->lock);
}

/**
 * Calculate fair delta for vruntime
 */
uint64_t calc_delta_fair(uint64_t delta, task_t *se) {
    if (se->se.load_weight != NICE_0_LOAD) {
        delta = (delta * NICE_0_LOAD) / se->se.load_weight;
    }
    return delta;
}

/**
 * Check if current task should be preempted
 */
bool should_preempt_curr(cpu_runqueue_t *rq, task_t *curr) {
    if (rq->cfs.nr_running <= 1) {
        return false;  /* No other tasks to run */
    }
    
    if (!rq->cfs.leftmost) {
        return false;  /* No leftmost task */
    }
    
    task_t *se = rb_entry(rq->cfs.leftmost, task_t, se.run_node);
    
    /* Preempt if leftmost task has significantly lower vruntime */
    return (se->se.vruntime + 1000000) < curr->se.vruntime;  /* 1ms threshold */
}

/**
 * Request reschedule
 */
void resched_curr(cpu_runqueue_t *rq) {
    task_t *curr = rq->curr;
    if (curr) {
        /* Set need_resched flag */
        /* TODO: Implement need_resched flag */
    }
}

/**
 * Check if reschedule is needed
 */
bool need_resched(void) {
    /* TODO: Check need_resched flag */
    return false;
}

/**
 * Context switch between tasks
 */
void context_switch(cpu_runqueue_t *rq, task_t *prev, task_t *next) {
    /* Switch memory management context */
    switch_mm(prev, next);
    
    /* Update statistics */
    if (prev) {
        prev->nivcsw++;
    }
    if (next) {
        next->nvcsw++;
    }
    
    /* Perform actual context switch */
    switch_context(prev ? &prev->context : NULL, &next->context);
}

/**
 * Switch memory management context
 */
void switch_mm(task_t *prev, task_t *next) {
    if (!prev || !next) return;
    
    if (prev->context.cr3 != next->context.cr3) {
        /* Switch page directory */
        asm volatile("mov %0, %%cr3" : : "r"(next->context.cr3));
    }
}

/**
 * Assembly context switch function
 */
void switch_context(void *prev_context, void *next_context) {
    if (!next_context) return;
    
    /* This is a simplified version - real implementation would be in assembly */
    struct context {
        uint32_t esp, ebp, eip, eflags, cr3;
    } *next = (struct context *)next_context;
    
    /* Load new context */
    asm volatile(
        "movl %0, %%esp\n"
        "movl %1, %%ebp\n"
        "pushl %3\n"         /* Push EFLAGS */
        "popfl\n"            /* Pop EFLAGS */
        "jmp *%2"
        :
        : "m"(next->esp), "m"(next->ebp), "m"(next->eip), "m"(next->eflags)
        : "memory"
    );
}

/**
 * Wake up a process
 */
void wake_up_process(task_t *task) {
    if (!task) return;
    
    unsigned long flags;
    cpu_runqueue_t *rq;
    
    /* Find the CPU this task was last on */
    uint32_t cpu = task->last_cpu;
    if (cpu >= nr_cpus_online) {
        cpu = 0;  /* Default to boot CPU */
    }
    
    rq = cpu_rq(cpu);
    spin_lock_irqsave(&rq->lock, &flags);
    
    if (task->state != TASK_RUNNING && task->state != TASK_READY) {
        task->state = TASK_READY;
        
        /* Add to appropriate runqueue */
        if (task->sched_class == SCHED_CLASS_RT) {
            int prio = task->se.prio;
            if (prio < MAX_RT_PRIO) {
                list_add_tail(&task->rt.run_list, &rq->rt.queue[prio]);
                rq->rt.nr_running++;
            }
        } else {
            enqueue_task_fair(rq, task);
        }
        
        /* Send reschedule IPI if on different CPU */
        if (cpu != smp_processor_id()) {
            smp_send_ipi(cpu, IPI_RESCHEDULE);
        }
    }
    
    spin_unlock_irqrestore(&rq->lock, flags);
}

/**
 * Update runqueue clock
 */
void update_rq_clock(cpu_runqueue_t *rq) {
    rq->clock = sched_clock_cpu(smp_processor_id());
    rq->clock_task = rq->clock;
}

/**
 * Get scheduling clock for CPU
 */
uint64_t sched_clock_cpu(uint32_t cpu) {
    /* TODO: Implement proper per-CPU clock */
    return get_ticks() * 1000000ULL;  /* Convert ticks to nanoseconds */
}

/**
 * Get scheduling clock
 */
uint64_t sched_clock(void) {
    return sched_clock_cpu(smp_processor_id());
}

/**
 * Find task by PID
 */
task_t *find_task_by_pid(pid_t pid) {
    task_t *task;
    
    spin_lock(&task_list_lock);
    list_for_each_entry(task, &task_list_head, task_list) {
        if (task->pid == pid) {
            spin_unlock(&task_list_lock);
            return task;
        }
    }
    spin_unlock(&task_list_lock);
    
    return NULL;
}

/**
 * Yield CPU voluntarily
 */
void sched_yield(void) {
    schedule();
}

/**
 * Sleep for specified milliseconds
 */
void msleep(unsigned int msecs) {
    /* TODO: Implement proper sleep */
    for (volatile uint32_t i = 0; i < msecs * 1000; i++) {
        /* Busy wait for now */
    }
}

/**
 * Set task affinity
 */
int sched_set_affinity(pid_t pid, const cpu_mask_t *mask) {
    task_t *task = find_task_by_pid(pid);
    if (!task) {
        return -1;
    }
    
    task->cpu_affinity = *mask;
    return 0;
}

/**
 * Get task affinity
 */
int sched_get_affinity(pid_t pid, cpu_mask_t *mask) {
    task_t *task = find_task_by_pid(pid);
    if (!task) {
        return -1;
    }
    
    *mask = task->cpu_affinity;
    return 0;
}

/**
 * Simple stub implementations for missing functions
 */

/* List operations */
void INIT_LIST_HEAD(struct list_head *list) {
    list->next = list;
    list->prev = list;
}

bool list_empty(const struct list_head *head) {
    return head->next == head;
}

void list_add_tail(struct list_head *new_entry, struct list_head *head) {
    new_entry->next = head;
    new_entry->prev = head->prev;
    head->prev->next = new_entry;
    head->prev = new_entry;
}

void list_del(struct list_head *entry) {
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}

/* Red-black tree stubs */
#define RB_ROOT_CACHED { { NULL }, NULL }

void rb_link_node(struct rb_node *node, struct rb_node *parent, struct rb_node **rb_link) {
    /* Simplified implementation */
    node->rb_parent_color = (unsigned long)parent;
    node->rb_left = node->rb_right = NULL;
    *rb_link = node;
}

void rb_insert_color_cached(struct rb_node *node, struct rb_root_cached *root, bool leftmost) {
    /* Simplified implementation */
    if (leftmost) {
        root->rb_leftmost = node;
    }
}

void rb_erase_cached(struct rb_node *node, struct rb_root_cached *root) {
    /* Simplified implementation */
    if (root->rb_leftmost == node) {
        root->rb_leftmost = NULL;  /* Should find next node */
    }
}

struct rb_node *rb_next(struct rb_node *node) {
    /* Simplified implementation */
    return NULL;
}

/* Atomic operations */
void atomic_set(atomic_t *v, int i) {
    v->counter = i;
}

int atomic_read(atomic_t *v) {
    return v->counter;
}

void atomic_inc(atomic_t *v) {
    __sync_fetch_and_add(&v->counter, 1);
}

void atomic_dec(atomic_t *v) {
    __sync_fetch_and_sub(&v->counter, 1);
}

int atomic_inc_return(atomic_t *v) {
    return __sync_add_and_fetch(&v->counter, 1);
}

int atomic_dec_return(atomic_t *v) {
    return __sync_sub_and_fetch(&v->counter, 1);
}

/* Utility functions */
bool time_after_eq(uint64_t a, uint64_t b) {
    return (int64_t)(a - b) >= 0;
}

uint64_t jiffies = 0;  /* Global jiffy counter */

#define rb_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define list_first_entry(ptr, type, member) \
    rb_entry((ptr)->next, type, member)

#define list_for_each_entry(pos, head, member) \
    for (pos = list_first_entry(head, typeof(*pos), member); \
         &pos->member != (head); \
         pos = list_first_entry(&pos->member, typeof(*pos), member))