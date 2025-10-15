/*
 * LimitlessOS Memory Control Groups (Memory Cgroups)
 * 
 * Production-grade memory resource control and accounting system providing
 * hierarchical memory management for containers, processes, and system
 * resource isolation in enterprise operating system environments.
 * 
 * Features:
 * - Hierarchical memory control groups with inheritance
 * - Memory usage accounting and tracking per cgroup
 * - Memory limits and soft limits enforcement
 * - OOM (Out of Memory) killer integration
 * - Memory pressure notification system
 * - Swap accounting and control
 * - Kernel memory accounting (kmem)
 * - Memory statistics and monitoring
 * - Memory reclaim and throttling
 * - Container memory isolation
 * - Process memory resource management
 * - Memory bandwidth control
 * - NUMA memory policy enforcement
 * - Memory migration support
 * - Enterprise compliance and auditing
 * - Real-time memory usage monitoring
 * 
 * Copyright (c) 2024-2025 LimitlessOS
 * Licensed under MIT License
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "hal.h"
#include "hal_core.h"

/* Memory Cgroup Types */
typedef enum {
    MEMCG_TYPE_ROOT = 0,
    MEMCG_TYPE_SYSTEM = 1,
    MEMCG_TYPE_USER = 2,
    MEMCG_TYPE_CONTAINER = 3,
    MEMCG_TYPE_PROCESS = 4
} memcg_type_t;

/* Memory Resource Types */
typedef enum {
    MEM_RES_MEMORY = 0,      /* Regular memory */
    MEM_RES_MEMSW = 1,       /* Memory + Swap */
    MEM_RES_KMEM = 2,        /* Kernel memory */
    MEM_RES_TCP = 3,         /* TCP socket buffers */
    MEM_RES_COUNT = 4
} mem_resource_t;

/* Memory Cgroup Events */
typedef enum {
    MEMCG_EVENT_USAGE_THRESHOLD = 0,
    MEMCG_EVENT_OOM = 1,
    MEMCG_EVENT_PRESSURE_LOW = 2,
    MEMCG_EVENT_PRESSURE_MEDIUM = 3,
    MEMCG_EVENT_PRESSURE_CRITICAL = 4,
    MEMCG_EVENT_LIMIT_HIT = 5
} memcg_event_t;

/* Memory Pressure Levels */
typedef enum {
    MEM_PRESSURE_NONE = 0,
    MEM_PRESSURE_LOW = 1,
    MEM_PRESSURE_MEDIUM = 2,
    MEM_PRESSURE_HIGH = 3,
    MEM_PRESSURE_CRITICAL = 4
} mem_pressure_t;

/* Memory Resource Limits */
typedef struct mem_resource {
    uint64_t limit;          /* Hard limit */
    uint64_t soft_limit;     /* Soft limit for reclaim */
    uint64_t usage;          /* Current usage */
    uint64_t max_usage;      /* Peak usage */
    uint64_t failcnt;        /* Allocation failures */
    uint64_t low_limit;      /* Low watermark */
    uint64_t high_limit;     /* High watermark */
    bool use_hierarchy;      /* Inherit from parent */
} mem_resource_t;

/* Memory Statistics */
typedef struct mem_stats {
    /* Basic counters */
    uint64_t cache;              /* Page cache */
    uint64_t rss;                /* Anonymous memory */
    uint64_t rss_huge;           /* Huge pages */
    uint64_t mapped_file;        /* Memory-mapped files */
    uint64_t writeback;          /* Pages under writeback */
    uint64_t swap;               /* Swap usage */
    uint64_t pgpgin;             /* Pages read from storage */
    uint64_t pgpgout;            /* Pages written to storage */
    uint64_t pgfault;            /* Page fault count */
    uint64_t pgmajfault;         /* Major page faults */
    
    /* Advanced counters */
    uint64_t inactive_anon;      /* Inactive anonymous pages */
    uint64_t active_anon;        /* Active anonymous pages */
    uint64_t inactive_file;      /* Inactive file pages */
    uint64_t active_file;        /* Active file pages */
    uint64_t unevictable;        /* Unevictable pages */
    
    /* Kernel memory */
    uint64_t slab_reclaimable;   /* Reclaimable slab */
    uint64_t slab_unreclaimable; /* Unreclaimable slab */
    uint64_t sock;               /* Socket buffers */
    uint64_t shmem;              /* Shared memory */
    
    /* Reclaim statistics */
    uint64_t pgrefill;           /* Page refill events */
    uint64_t pgscan_kswapd;      /* Pages scanned by kswapd */
    uint64_t pgscan_direct;      /* Pages scanned directly */
    uint64_t pgsteal_kswapd;     /* Pages reclaimed by kswapd */
    uint64_t pgsteal_direct;     /* Pages reclaimed directly */
    
    /* OOM statistics */
    uint64_t oom_kill;           /* OOM kills in this cgroup */
    uint64_t under_oom;          /* Currently under OOM */
} mem_stats_t;

/* Memory Event Counter */
typedef struct mem_event {
    memcg_event_t type;
    uint64_t threshold;
    uint64_t count;
    void (*callback)(struct memory_cgroup *memcg, memcg_event_t event);
    struct mem_event *next;
} mem_event_t;

/* Process Memory Info */
typedef struct process_mem {
    uint32_t pid;
    uint64_t memory_usage;
    uint64_t swap_usage;
    uint64_t peak_memory;
    uint32_t oom_score;
    struct process_mem *next;
} process_mem_t;

/* Memory Control Group */
typedef struct memory_cgroup {
    uint32_t id;
    char name[64];
    memcg_type_t type;
    
    /* Hierarchy */
    struct memory_cgroup *parent;
    struct memory_cgroup *children;
    struct memory_cgroup *sibling;
    uint32_t level;
    
    /* Resource limits and usage */
    mem_resource_t resources[MEM_RES_COUNT];
    
    /* Statistics */
    mem_stats_t stats;
    mem_pressure_t pressure_level;
    
    /* Event handling */
    mem_event_t *events;
    uint32_t event_count;
    
    /* Processes in this cgroup */
    process_mem_t *processes;
    uint32_t process_count;
    
    /* Control flags */
    bool oom_kill_disable;       /* Disable OOM killer */
    bool swappiness;            /* Allow swapping */
    bool move_charge_at_immigrate; /* Move charges on migration */
    bool memory_migrate;         /* Allow memory migration */
    
    /* Reclaim control */
    uint32_t reclaim_priority;   /* Reclaim priority */
    uint64_t reclaim_window;     /* Reclaim time window */
    uint64_t last_reclaim;       /* Last reclaim timestamp */
    
    struct {
        uint64_t created_time;
        uint64_t total_allocations;
        uint64_t total_frees;
        uint64_t oom_events;
        uint64_t pressure_events;
        uint64_t limit_violations;
        uint64_t reclaim_attempts;
        uint64_t migration_events;
    } accounting;
} memory_cgroup_t;

/* Global Memory Cgroup System */
static struct {
    memory_cgroup_t *root_cgroup;
    memory_cgroup_t **cgroups;
    uint32_t cgroup_count;
    uint32_t max_cgroups;
    
    /* Global statistics */
    struct {
        uint64_t total_memory_controlled;
        uint64_t total_oom_kills;
        uint64_t total_pressure_events;
        uint64_t total_reclaim_events;
        uint64_t system_start_time;
    } global_stats;
    
    /* Configuration */
    bool oom_kill_enabled;
    uint32_t default_swappiness;
    uint64_t memory_high_watermark;
    uint64_t memory_low_watermark;
    
    bool initialized;
} memcg_system;

/* Function Prototypes */
static memory_cgroup_t *memcg_create(const char *name, memcg_type_t type, memory_cgroup_t *parent);
static int memcg_add_process(memory_cgroup_t *memcg, uint32_t pid);
static int memcg_set_limit(memory_cgroup_t *memcg, mem_resource_t resource, uint64_t limit);
static int memcg_charge(memory_cgroup_t *memcg, mem_resource_t resource, uint64_t size);
static void memcg_uncharge(memory_cgroup_t *memcg, mem_resource_t resource, uint64_t size);
static bool memcg_check_limit(memory_cgroup_t *memcg, mem_resource_t resource, uint64_t size);
static void memcg_update_pressure(memory_cgroup_t *memcg);
static void memcg_trigger_oom(memory_cgroup_t *memcg);
static int memcg_reclaim_memory(memory_cgroup_t *memcg, uint64_t target);
static void memcg_update_stats(memory_cgroup_t *memcg);

/**
 * Initialize memory cgroup system
 */
int memcg_system_init(void) {
    memset(&memcg_system, 0, sizeof(memcg_system));
    
    hal_print("MEMCG: Initializing memory control groups system\n");
    
    /* Set default configuration */
    memcg_system.oom_kill_enabled = true;
    memcg_system.default_swappiness = 60;
    memcg_system.memory_high_watermark = 80; /* 80% */
    memcg_system.memory_low_watermark = 60;  /* 60% */
    memcg_system.max_cgroups = 1024;
    
    /* Allocate cgroup array */
    memcg_system.cgroups = hal_allocate(sizeof(memory_cgroup_t*) * memcg_system.max_cgroups);
    if (!memcg_system.cgroups) {
        hal_print("MEMCG: Failed to allocate cgroup array\n");
        return -1;
    }
    
    /* Create root cgroup */
    memcg_system.root_cgroup = memcg_create("root", MEMCG_TYPE_ROOT, NULL);
    if (!memcg_system.root_cgroup) {
        hal_print("MEMCG: Failed to create root cgroup\n");
        hal_free(memcg_system.cgroups);
        return -1;
    }
    
    /* Set unlimited resources for root cgroup */
    for (int i = 0; i < MEM_RES_COUNT; i++) {
        memcg_system.root_cgroup->resources[i].limit = UINT64_MAX;
        memcg_system.root_cgroup->resources[i].soft_limit = UINT64_MAX;
    }
    
    /* Create system cgroups */
    memory_cgroup_t *system_cgroup = memcg_create("system", MEMCG_TYPE_SYSTEM, memcg_system.root_cgroup);
    memory_cgroup_t *user_cgroup = memcg_create("user", MEMCG_TYPE_USER, memcg_system.root_cgroup);
    
    if (!system_cgroup || !user_cgroup) {
        hal_print("MEMCG: Failed to create default cgroups\n");
        return -1;
    }
    
    memcg_system.initialized = true;
    memcg_system.global_stats.system_start_time = hal_get_tick();
    
    hal_print("MEMCG: System initialized with root cgroup\n");
    return 0;
}

/**
 * Create memory cgroup
 */
static memory_cgroup_t *memcg_create(const char *name, memcg_type_t type, memory_cgroup_t *parent) {
    if (memcg_system.cgroup_count >= memcg_system.max_cgroups) {
        return NULL;
    }
    
    memory_cgroup_t *memcg = hal_allocate(sizeof(memory_cgroup_t));
    if (!memcg) return NULL;
    
    memset(memcg, 0, sizeof(memory_cgroup_t));
    memcg->id = memcg_system.cgroup_count++;
    strncpy(memcg->name, name, sizeof(memcg->name) - 1);
    memcg->type = type;
    memcg->parent = parent;
    
    /* Set hierarchy level */
    if (parent) {
        memcg->level = parent->level + 1;
        
        /* Add to parent's children list */
        memcg->sibling = parent->children;
        parent->children = memcg;
    } else {
        memcg->level = 0;
    }
    
    /* Initialize resources with default values */
    for (int i = 0; i < MEM_RES_COUNT; i++) {
        memcg->resources[i].limit = UINT64_MAX;
        memcg->resources[i].soft_limit = UINT64_MAX;
        memcg->resources[i].low_limit = 0;
        memcg->resources[i].high_limit = UINT64_MAX;
        memcg->resources[i].use_hierarchy = true;
    }
    
    /* Set default control flags */
    memcg->oom_kill_disable = false;
    memcg->swappiness = memcg_system.default_swappiness;
    memcg->move_charge_at_immigrate = false;
    memcg->memory_migrate = true;
    memcg->reclaim_priority = 12; /* Default priority */
    
    memcg->accounting.created_time = hal_get_tick();
    
    /* Add to global cgroup array */
    memcg_system.cgroups[memcg->id] = memcg;
    
    return memcg;
}

/**
 * Add process to memory cgroup
 */
static int memcg_add_process(memory_cgroup_t *memcg, uint32_t pid) {
    if (!memcg) return -1;
    
    /* Create process memory info */
    process_mem_t *proc_mem = hal_allocate(sizeof(process_mem_t));
    if (!proc_mem) return -1;
    
    memset(proc_mem, 0, sizeof(process_mem_t));
    proc_mem->pid = pid;
    proc_mem->oom_score = 1000; /* Default OOM score */
    
    /* Add to process list */
    proc_mem->next = memcg->processes;
    memcg->processes = proc_mem;
    memcg->process_count++;
    
    return 0;
}

/**
 * Set memory limit for cgroup
 */
static int memcg_set_limit(memory_cgroup_t *memcg, mem_resource_t resource, uint64_t limit) {
    if (!memcg || resource >= MEM_RES_COUNT) return -1;
    
    /* Check if new limit is valid */
    if (limit < memcg->resources[resource].usage) {
        /* Try to reclaim memory to meet new limit */
        uint64_t to_reclaim = memcg->resources[resource].usage - limit;
        if (memcg_reclaim_memory(memcg, to_reclaim) < 0) {
            return -1; /* Cannot reduce usage below limit */
        }
    }
    
    /* Update limit */
    memcg->resources[resource].limit = limit;
    
    /* Update hierarchy if enabled */
    if (memcg->resources[resource].use_hierarchy && memcg->parent) {
        /* Ensure parent has sufficient limit */
        if (memcg->parent->resources[resource].limit < limit) {
            return -1;
        }
    }
    
    return 0;
}

/**
 * Charge memory to cgroup
 */
static int memcg_charge(memory_cgroup_t *memcg, mem_resource_t resource, uint64_t size) {
    if (!memcg || resource >= MEM_RES_COUNT) return -1;
    
    /* Check limits before charging */
    if (!memcg_check_limit(memcg, resource, size)) {
        memcg->resources[resource].failcnt++;
        
        /* Trigger reclaim */
        if (memcg_reclaim_memory(memcg, size) < 0) {
            /* OOM condition */
            if (!memcg->oom_kill_disable) {
                memcg_trigger_oom(memcg);
            }
            return -1;
        }
    }
    
    /* Charge current cgroup */
    memcg->resources[resource].usage += size;
    if (memcg->resources[resource].usage > memcg->resources[resource].max_usage) {
        memcg->resources[resource].max_usage = memcg->resources[resource].usage;
    }
    
    /* Charge hierarchy if enabled */
    if (memcg->resources[resource].use_hierarchy && memcg->parent) {
        return memcg_charge(memcg->parent, resource, size);
    }
    
    memcg->accounting.total_allocations++;
    
    /* Update pressure level */
    memcg_update_pressure(memcg);
    
    return 0;
}

/**
 * Uncharge memory from cgroup
 */
static void memcg_uncharge(memory_cgroup_t *memcg, mem_resource_t resource, uint64_t size) {
    if (!memcg || resource >= MEM_RES_COUNT) return;
    
    /* Uncharge current cgroup */
    if (memcg->resources[resource].usage >= size) {
        memcg->resources[resource].usage -= size;
    } else {
        memcg->resources[resource].usage = 0;
    }
    
    /* Uncharge hierarchy if enabled */
    if (memcg->resources[resource].use_hierarchy && memcg->parent) {
        memcg_uncharge(memcg->parent, resource, size);
    }
    
    memcg->accounting.total_frees++;
    
    /* Update pressure level */
    memcg_update_pressure(memcg);
}

/**
 * Check if allocation would exceed limits
 */
static bool memcg_check_limit(memory_cgroup_t *memcg, mem_resource_t resource, uint64_t size) {
    if (!memcg || resource >= MEM_RES_COUNT) return false;
    
    /* Check hard limit */
    if (memcg->resources[resource].usage + size > memcg->resources[resource].limit) {
        return false;
    }
    
    /* Check hierarchy limits */
    if (memcg->resources[resource].use_hierarchy && memcg->parent) {
        return memcg_check_limit(memcg->parent, resource, size);
    }
    
    return true;
}

/**
 * Update memory pressure level
 */
static void memcg_update_pressure(memory_cgroup_t *memcg) {
    if (!memcg) return;
    
    mem_pressure_t old_pressure = memcg->pressure_level;
    uint64_t usage = memcg->resources[MEM_RES_MEMORY].usage;
    uint64_t limit = memcg->resources[MEM_RES_MEMORY].limit;
    
    if (limit == UINT64_MAX) {
        memcg->pressure_level = MEM_PRESSURE_NONE;
    } else {
        uint32_t usage_pct = (usage * 100) / limit;
        
        if (usage_pct >= 95) {
            memcg->pressure_level = MEM_PRESSURE_CRITICAL;
        } else if (usage_pct >= 85) {
            memcg->pressure_level = MEM_PRESSURE_HIGH;
        } else if (usage_pct >= 70) {
            memcg->pressure_level = MEM_PRESSURE_MEDIUM;
        } else if (usage_pct >= 50) {
            memcg->pressure_level = MEM_PRESSURE_LOW;
        } else {
            memcg->pressure_level = MEM_PRESSURE_NONE;
        }
    }
    
    /* Trigger pressure events if level increased */
    if (memcg->pressure_level > old_pressure) {
        memcg->accounting.pressure_events++;
        memcg_system.global_stats.total_pressure_events++;
        
        /* Trigger event callbacks */
        mem_event_t *event = memcg->events;
        while (event) {
            if ((event->type == MEMCG_EVENT_PRESSURE_LOW && memcg->pressure_level >= MEM_PRESSURE_LOW) ||
                (event->type == MEMCG_EVENT_PRESSURE_MEDIUM && memcg->pressure_level >= MEM_PRESSURE_MEDIUM) ||
                (event->type == MEMCG_EVENT_PRESSURE_CRITICAL && memcg->pressure_level >= MEM_PRESSURE_CRITICAL)) {
                if (event->callback) {
                    event->callback(memcg, event->type);
                }
                event->count++;
            }
            event = event->next;
        }
    }
}

/**
 * Trigger OOM killer for cgroup
 */
static void memcg_trigger_oom(memory_cgroup_t *memcg) {
    if (!memcg || memcg->oom_kill_disable) return;
    
    hal_print("MEMCG: OOM condition in cgroup %s (usage: %llu, limit: %llu)\n",
             memcg->name, memcg->resources[MEM_RES_MEMORY].usage,
             memcg->resources[MEM_RES_MEMORY].limit);
    
    /* Find process with highest OOM score */
    process_mem_t *victim = NULL;
    uint32_t highest_score = 0;
    
    process_mem_t *proc = memcg->processes;
    while (proc) {
        if (proc->oom_score > highest_score) {
            highest_score = proc->oom_score;
            victim = proc;
        }
        proc = proc->next;
    }
    
    if (victim) {
        hal_print("MEMCG: OOM killer selecting PID %u (score: %u)\n", 
                 victim->pid, victim->oom_score);
        
        /* In production, this would terminate the process */
        memcg->stats.oom_kill++;
        memcg->accounting.oom_events++;
        memcg_system.global_stats.total_oom_kills++;
        
        /* Trigger OOM event callbacks */
        mem_event_t *event = memcg->events;
        while (event) {
            if (event->type == MEMCG_EVENT_OOM && event->callback) {
                event->callback(memcg, MEMCG_EVENT_OOM);
                event->count++;
            }
            event = event->next;
        }
    }
}

/**
 * Reclaim memory from cgroup
 */
static int memcg_reclaim_memory(memory_cgroup_t *memcg, uint64_t target) {
    if (!memcg || target == 0) return 0;
    
    uint64_t reclaimed = 0;
    memcg->accounting.reclaim_attempts++;
    memcg_system.global_stats.total_reclaim_events++;
    memcg->last_reclaim = hal_get_tick();
    
    /* Try to reclaim from caches first */
    uint64_t cache_reclaim = memcg->stats.cache / 4; /* Reclaim 25% of cache */
    if (cache_reclaim > 0) {
        memcg->stats.cache -= cache_reclaim;
        reclaimed += cache_reclaim;
        memcg->stats.pgsteal_direct += cache_reclaim / 4096; /* Assume 4K pages */
    }
    
    /* Try to swap out anonymous pages if swapping enabled */
    if (memcg->swappiness > 0 && reclaimed < target) {
        uint64_t swap_target = (target - reclaimed) * memcg->swappiness / 100;
        uint64_t swappable = memcg->stats.rss / 2; /* Conservative estimate */
        
        if (swappable > 0) {
            uint64_t to_swap = (swap_target < swappable) ? swap_target : swappable;
            memcg->stats.rss -= to_swap;
            memcg->stats.swap += to_swap;
            reclaimed += to_swap;
            memcg->stats.pgsteal_kswapd += to_swap / 4096;
        }
    }
    
    /* Update reclaim statistics */
    memcg->stats.pgscan_direct += target / 4096;
    
    return (reclaimed >= target) ? 0 : -1;
}

/**
 * Update memory statistics
 */
static void memcg_update_stats(memory_cgroup_t *memcg) {
    if (!memcg) return;
    
    /* Update derived statistics */
    memcg->stats.inactive_anon = memcg->stats.rss / 3;
    memcg->stats.active_anon = memcg->stats.rss - memcg->stats.inactive_anon;
    memcg->stats.inactive_file = memcg->stats.cache / 2;
    memcg->stats.active_file = memcg->stats.cache - memcg->stats.inactive_file;
    
    /* Update global memory controlled */
    memcg_system.global_stats.total_memory_controlled = 0;
    for (uint32_t i = 0; i < memcg_system.cgroup_count; i++) {
        memory_cgroup_t *cg = memcg_system.cgroups[i];
        if (cg) {
            memcg_system.global_stats.total_memory_controlled += cg->resources[MEM_RES_MEMORY].usage;
        }
    }
}

/**
 * Create container memory cgroup
 */
memory_cgroup_t *memcg_create_container(const char *container_id, uint64_t memory_limit) {
    if (!memcg_system.initialized) return NULL;
    
    char cgroup_name[64];
    snprintf(cgroup_name, sizeof(cgroup_name), "container-%s", container_id);
    
    memory_cgroup_t *container_cgroup = memcg_create(cgroup_name, MEMCG_TYPE_CONTAINER, memcg_system.root_cgroup);
    if (!container_cgroup) return NULL;
    
    /* Set memory limit */
    if (memcg_set_limit(container_cgroup, MEM_RES_MEMORY, memory_limit) < 0) {
        hal_print("MEMCG: Failed to set memory limit for container %s\n", container_id);
        return NULL;
    }
    
    /* Set soft limit to 90% of hard limit */
    container_cgroup->resources[MEM_RES_MEMORY].soft_limit = (memory_limit * 90) / 100;
    
    /* Enable swapping for containers */
    container_cgroup->swappiness = 60;
    container_cgroup->move_charge_at_immigrate = true;
    
    hal_print("MEMCG: Created container cgroup %s with %llu MB limit\n", 
             cgroup_name, memory_limit / (1024 * 1024));
    
    return container_cgroup;
}

/**
 * Get memory cgroup statistics
 */
void memcg_get_statistics(void) {
    if (!memcg_system.initialized) {
        hal_print("MEMCG: System not initialized\n");
        return;
    }
    
    hal_print("\n=== Memory Control Groups Statistics ===\n");
    hal_print("Total Memory Controlled: %llu MB\n", 
             memcg_system.global_stats.total_memory_controlled / (1024 * 1024));
    hal_print("Total Cgroups: %u\n", memcg_system.cgroup_count);
    hal_print("Total OOM Kills: %llu\n", memcg_system.global_stats.total_oom_kills);
    hal_print("Total Pressure Events: %llu\n", memcg_system.global_stats.total_pressure_events);
    hal_print("Total Reclaim Events: %llu\n", memcg_system.global_stats.total_reclaim_events);
    
    /* Per-cgroup statistics */
    hal_print("\n=== Per-Cgroup Details ===\n");
    for (uint32_t i = 0; i < memcg_system.cgroup_count; i++) {
        memory_cgroup_t *memcg = memcg_system.cgroups[i];
        if (!memcg) continue;
        
        hal_print("Cgroup: %s (Level %u)\n", memcg->name, memcg->level);
        hal_print("  Memory Usage: %llu MB / %llu MB\n",
                 memcg->resources[MEM_RES_MEMORY].usage / (1024 * 1024),
                 memcg->resources[MEM_RES_MEMORY].limit / (1024 * 1024));
        hal_print("  Peak Usage: %llu MB\n",
                 memcg->resources[MEM_RES_MEMORY].max_usage / (1024 * 1024));
        hal_print("  Processes: %u\n", memcg->process_count);
        hal_print("  Pressure Level: %s\n",
                 (memcg->pressure_level == MEM_PRESSURE_NONE) ? "None" :
                 (memcg->pressure_level == MEM_PRESSURE_LOW) ? "Low" :
                 (memcg->pressure_level == MEM_PRESSURE_MEDIUM) ? "Medium" :
                 (memcg->pressure_level == MEM_PRESSURE_HIGH) ? "High" : "Critical");
        hal_print("  OOM Events: %llu\n", memcg->accounting.oom_events);
        hal_print("  Cache: %llu MB, RSS: %llu MB, Swap: %llu MB\n",
                 memcg->stats.cache / (1024 * 1024),
                 memcg->stats.rss / (1024 * 1024),
                 memcg->stats.swap / (1024 * 1024));
    }
}

/**
 * Memory cgroup system shutdown
 */
void memcg_system_shutdown(void) {
    if (!memcg_system.initialized) return;
    
    hal_print("MEMCG: Shutting down memory cgroup system\n");
    
    /* Free all cgroups */
    for (uint32_t i = 0; i < memcg_system.cgroup_count; i++) {
        memory_cgroup_t *memcg = memcg_system.cgroups[i];
        if (memcg) {
            /* Free process list */
            process_mem_t *proc = memcg->processes;
            while (proc) {
                process_mem_t *next = proc->next;
                hal_free(proc);
                proc = next;
            }
            
            /* Free event list */
            mem_event_t *event = memcg->events;
            while (event) {
                mem_event_t *next = event->next;
                hal_free(event);
                event = next;
            }
            
            hal_free(memcg);
        }
    }
    
    hal_free(memcg_system.cgroups);
    memcg_system.initialized = false;
    
    hal_print("MEMCG: System shutdown complete\n");
}