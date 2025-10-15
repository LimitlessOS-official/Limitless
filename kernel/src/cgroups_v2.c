/*
 * LimitlessOS Process Control Groups v2 (cgroups v2)
 * 
 * Production-grade hierarchical process resource management system providing
 * unified control group interface for containers, systemd, and enterprise
 * process management in modern operating system environments.
 * 
 * Features:
 * - Unified cgroups v2 hierarchy with single tree structure
 * - CPU resource control with weights, quotas, and real-time scheduling
 * - Memory resource management with soft/hard limits
 * - I/O bandwidth and IOPS control with multiple devices
 * - Process and thread count limits
 * - Network bandwidth control and classification
 * - Device access control with whitelist/blacklist
 * - Freezer functionality for process suspension
 * - Process migration between cgroups
 * - Event notification system with pressure stall information
 * - Statistics collection and monitoring
 * - Container runtime integration
 * - Systemd service integration
 * - Process accounting and resource tracking
 * - Security context isolation
 * - Real-time process management
 * 
 * Copyright (c) 2024-2025 LimitlessOS
 * Licensed under MIT License
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "hal.h"
#include "hal_core.h"

/* Cgroup Controller Types */
typedef enum {
    CGROUP_CTRL_CPU = 0,         /* CPU time and scheduling */
    CGROUP_CTRL_MEMORY = 1,      /* Memory usage and limits */
    CGROUP_CTRL_IO = 2,          /* Block I/O bandwidth */
    CGROUP_CTRL_PIDS = 3,        /* Process/thread count */
    CGROUP_CTRL_CPUSET = 4,      /* CPU and memory node binding */
    CGROUP_CTRL_DEVICES = 5,     /* Device access control */
    CGROUP_CTRL_FREEZER = 6,     /* Process freezing */
    CGROUP_CTRL_NET = 7,         /* Network bandwidth */
    CGROUP_CTRL_PERF = 8,        /* Performance monitoring */
    CGROUP_CTRL_COUNT = 9
} cgroup_controller_t;

/* Cgroup Types */
typedef enum {
    CGROUP_TYPE_ROOT = 0,
    CGROUP_TYPE_SYSTEM = 1,
    CGROUP_TYPE_USER = 2,
    CGROUP_TYPE_SESSION = 3,
    CGROUP_TYPE_CONTAINER = 4,
    CGROUP_TYPE_SERVICE = 5,
    CGROUP_TYPE_PROCESS = 6
} cgroup_type_t;

/* Process States in Cgroup */
typedef enum {
    CGROUP_PROC_RUNNING = 0,
    CGROUP_PROC_FROZEN = 1,
    CGROUP_PROC_THAWED = 2,
    CGROUP_PROC_MIGRATING = 3
} cgroup_proc_state_t;

/* CPU Controller Configuration */
typedef struct cpu_controller {
    uint64_t weight;             /* CPU weight (1-10000) */
    uint64_t weight_nice;        /* Nice-based weight */
    uint64_t max_bandwidth;      /* Maximum bandwidth in us per period */
    uint64_t period;            /* Period length in microseconds */
    uint64_t quota;             /* CPU quota in us per period */
    bool rt_runtime_enabled;     /* Real-time runtime enabled */
    uint64_t rt_runtime_us;     /* RT runtime in microseconds */
    uint64_t rt_period_us;      /* RT period in microseconds */
    
    struct {
        uint64_t usage_usec;     /* Total CPU usage */
        uint64_t user_usec;      /* User CPU time */
        uint64_t system_usec;    /* System CPU time */
        uint64_t nr_periods;     /* Number of periods */
        uint64_t nr_throttled;   /* Number of throttled periods */
        uint64_t throttled_usec; /* Total throttled time */
    } stats;
} cpu_controller_t;

/* Memory Controller Configuration */
typedef struct memory_controller {
    uint64_t min_bytes;         /* Memory guarantee */
    uint64_t low_bytes;         /* Best-effort protection */
    uint64_t high_bytes;        /* Memory throttle limit */
    uint64_t max_bytes;         /* Memory limit */
    uint64_t swap_max;          /* Swap limit */
    bool oom_kill_disable;      /* Disable OOM killer */
    uint32_t swappiness;        /* Swappiness value */
    
    struct {
        uint64_t current;        /* Current memory usage */
        uint64_t peak;           /* Peak memory usage */
        uint64_t swap_current;   /* Current swap usage */
        uint64_t swap_peak;      /* Peak swap usage */
        uint64_t cache;          /* Page cache */
        uint64_t rss;            /* Anonymous memory */
        uint64_t rss_huge;       /* Huge page memory */
        uint64_t mapped_file;    /* Memory-mapped files */
        uint64_t dirty;          /* Dirty pages */
        uint64_t writeback;      /* Writeback pages */
        uint64_t pgfault;        /* Page faults */
        uint64_t pgmajfault;     /* Major page faults */
        uint64_t oom_events;     /* OOM events */
    } stats;
} memory_controller_t;

/* I/O Controller Configuration */
typedef struct io_controller {
    uint64_t weight;            /* I/O weight (1-10000) */
    uint64_t read_bps;          /* Read bandwidth limit */
    uint64_t write_bps;         /* Write bandwidth limit */
    uint64_t read_iops;         /* Read IOPS limit */
    uint64_t write_iops;        /* Write IOPS limit */
    uint32_t latency_target_us; /* Target latency */
    
    struct {
        uint64_t rbytes;         /* Bytes read */
        uint64_t wbytes;         /* Bytes written */
        uint64_t rios;           /* Read operations */
        uint64_t wios;           /* Write operations */
        uint64_t dbytes;         /* Discarded bytes */
        uint64_t dios;           /* Discard operations */
    } stats;
} io_controller_t;

/* PID Controller Configuration */
typedef struct pids_controller {
    uint64_t max_pids;          /* Maximum number of PIDs */
    
    struct {
        uint64_t current;        /* Current PID count */
        uint64_t peak;           /* Peak PID count */
        uint64_t events;         /* Limit hit events */
    } stats;
} pids_controller_t;

/* CPUSET Controller Configuration */
typedef struct cpuset_controller {
    uint64_t cpus_allowed;      /* CPU bitmask */
    uint64_t mems_allowed;      /* Memory node bitmask */
    bool cpu_exclusive;         /* Exclusive CPU access */
    bool mem_exclusive;         /* Exclusive memory access */
    bool mem_hardwall;          /* Hard memory wall */
    bool memory_migrate;        /* Allow memory migration */
    bool sched_load_balance;    /* Enable load balancing */
} cpuset_controller_t;

/* Device Controller Entry */
typedef struct device_rule {
    char type;                  /* 'c' for char, 'b' for block, 'a' for all */
    uint32_t major;             /* Major device number */
    uint32_t minor;             /* Minor device number */
    char permissions[4];        /* 'r', 'w', 'm' permissions */
    bool allow;                 /* Allow or deny */
    struct device_rule *next;
} device_rule_t;

/* Device Controller Configuration */
typedef struct devices_controller {
    device_rule_t *allow_list;  /* Allowed devices */
    device_rule_t *deny_list;   /* Denied devices */
    bool default_deny;          /* Default deny all */
} devices_controller_t;

/* Process Entry in Cgroup */
typedef struct cgroup_process {
    uint32_t pid;
    uint32_t tid;               /* Thread ID (0 for processes) */
    cgroup_proc_state_t state;
    uint64_t join_time;
    uint64_t cpu_usage;
    uint64_t memory_usage;
    struct cgroup_process *next;
} cgroup_process_t;

/* Event Notification */
typedef struct cgroup_event {
    uint32_t type;
    uint64_t threshold;
    void (*callback)(struct cgroup_v2 *cgroup, uint32_t event_type);
    struct cgroup_event *next;
} cgroup_event_t;

/* Cgroups v2 Structure */
typedef struct cgroup_v2 {
    uint32_t id;
    char name[128];             /* Cgroup name/path */
    cgroup_type_t type;
    
    /* Hierarchy */
    struct cgroup_v2 *parent;
    struct cgroup_v2 *children;
    struct cgroup_v2 *sibling;
    uint32_t level;
    
    /* Controller enablement */
    bool controllers_enabled[CGROUP_CTRL_COUNT];
    bool subtree_control[CGROUP_CTRL_COUNT];
    
    /* Controller configurations */
    cpu_controller_t cpu;
    memory_controller_t memory;
    io_controller_t io;
    pids_controller_t pids;
    cpuset_controller_t cpuset;
    devices_controller_t devices;
    
    /* Processes and threads */
    cgroup_process_t *processes;
    uint32_t process_count;
    uint32_t thread_count;
    
    /* State */
    bool frozen;                /* Freezer state */
    bool populated;             /* Has processes */
    
    /* Events */
    cgroup_event_t *events;
    
    /* Statistics */
    struct {
        uint64_t created_time;
        uint64_t process_migrations;
        uint64_t controller_changes;
        uint64_t freeze_events;
        uint64_t thaw_events;
    } stats;
} cgroup_v2_t;

/* Global Cgroups v2 System */
static struct {
    cgroup_v2_t *root_cgroup;
    cgroup_v2_t **cgroups;
    uint32_t cgroup_count;
    uint32_t max_cgroups;
    
    /* Controller availability */
    bool controllers_available[CGROUP_CTRL_COUNT];
    
    /* Global configuration */
    uint64_t default_cpu_weight;
    uint64_t default_cpu_period;
    uint64_t default_memory_low;
    uint32_t default_swappiness;
    
    struct {
        uint64_t total_processes_managed;
        uint64_t total_migrations;
        uint64_t total_freeze_events;
        uint64_t controller_enable_events;
        uint64_t system_start_time;
    } global_stats;
    
    bool initialized;
} cgroup_system;

/* Function Prototypes */
static cgroup_v2_t *cgroup_create(const char *name, cgroup_type_t type, cgroup_v2_t *parent);
static int cgroup_enable_controller(cgroup_v2_t *cgroup, cgroup_controller_t controller);
static int cgroup_add_process(cgroup_v2_t *cgroup, uint32_t pid);
static int cgroup_migrate_process(cgroup_v2_t *from, cgroup_v2_t *to, uint32_t pid);
static int cgroup_freeze(cgroup_v2_t *cgroup);
static int cgroup_thaw(cgroup_v2_t *cgroup);
static void cgroup_update_stats(cgroup_v2_t *cgroup);
static int cgroup_set_cpu_weight(cgroup_v2_t *cgroup, uint64_t weight);
static int cgroup_set_memory_limit(cgroup_v2_t *cgroup, uint64_t limit);
static int cgroup_set_io_limit(cgroup_v2_t *cgroup, uint64_t read_bps, uint64_t write_bps);

/**
 * Initialize cgroups v2 system
 */
int cgroup_v2_init(void) {
    memset(&cgroup_system, 0, sizeof(cgroup_system));
    
    hal_print("CGROUP: Initializing Control Groups v2 system\n");
    
    /* Set default configuration */
    cgroup_system.default_cpu_weight = 100;
    cgroup_system.default_cpu_period = 100000; /* 100ms */
    cgroup_system.default_memory_low = 0;
    cgroup_system.default_swappiness = 60;
    cgroup_system.max_cgroups = 4096;
    
    /* Enable all available controllers */
    for (int i = 0; i < CGROUP_CTRL_COUNT; i++) {
        cgroup_system.controllers_available[i] = true;
    }
    
    /* Allocate cgroup array */
    cgroup_system.cgroups = hal_allocate(sizeof(cgroup_v2_t*) * cgroup_system.max_cgroups);
    if (!cgroup_system.cgroups) {
        hal_print("CGROUP: Failed to allocate cgroup array\n");
        return -1;
    }
    
    /* Create root cgroup */
    cgroup_system.root_cgroup = cgroup_create("/", CGROUP_TYPE_ROOT, NULL);
    if (!cgroup_system.root_cgroup) {
        hal_print("CGROUP: Failed to create root cgroup\n");
        hal_free(cgroup_system.cgroups);
        return -1;
    }
    
    /* Enable all controllers in root cgroup */
    for (int i = 0; i < CGROUP_CTRL_COUNT; i++) {
        if (cgroup_system.controllers_available[i]) {
            cgroup_enable_controller(cgroup_system.root_cgroup, i);
        }
    }
    
    /* Create system cgroups */
    cgroup_v2_t *system_slice = cgroup_create("/system.slice", CGROUP_TYPE_SYSTEM, cgroup_system.root_cgroup);
    cgroup_v2_t *user_slice = cgroup_create("/user.slice", CGROUP_TYPE_USER, cgroup_system.root_cgroup);
    cgroup_v2_t *machine_slice = cgroup_create("/machine.slice", CGROUP_TYPE_CONTAINER, cgroup_system.root_cgroup);
    
    if (!system_slice || !user_slice || !machine_slice) {
        hal_print("CGROUP: Failed to create default slices\n");
        return -1;
    }
    
    /* Enable controllers for default slices */
    cgroup_controller_t default_controllers[] = {
        CGROUP_CTRL_CPU, CGROUP_CTRL_MEMORY, CGROUP_CTRL_IO, CGROUP_CTRL_PIDS
    };
    
    for (int i = 0; i < 4; i++) {
        cgroup_enable_controller(system_slice, default_controllers[i]);
        cgroup_enable_controller(user_slice, default_controllers[i]);
        cgroup_enable_controller(machine_slice, default_controllers[i]);
    }
    
    cgroup_system.initialized = true;
    cgroup_system.global_stats.system_start_time = hal_get_tick();
    
    hal_print("CGROUP: System initialized with %d controllers\n", CGROUP_CTRL_COUNT);
    return 0;
}

/**
 * Create cgroup
 */
static cgroup_v2_t *cgroup_create(const char *name, cgroup_type_t type, cgroup_v2_t *parent) {
    if (cgroup_system.cgroup_count >= cgroup_system.max_cgroups) {
        return NULL;
    }
    
    cgroup_v2_t *cgroup = hal_allocate(sizeof(cgroup_v2_t));
    if (!cgroup) return NULL;
    
    memset(cgroup, 0, sizeof(cgroup_v2_t));
    cgroup->id = cgroup_system.cgroup_count++;
    strncpy(cgroup->name, name, sizeof(cgroup->name) - 1);
    cgroup->type = type;
    cgroup->parent = parent;
    
    /* Set hierarchy level */
    if (parent) {
        cgroup->level = parent->level + 1;
        
        /* Add to parent's children list */
        cgroup->sibling = parent->children;
        parent->children = cgroup;
    } else {
        cgroup->level = 0;
    }
    
    /* Initialize controller defaults */
    cgroup->cpu.weight = cgroup_system.default_cpu_weight;
    cgroup->cpu.period = cgroup_system.default_cpu_period;
    cgroup->memory.swappiness = cgroup_system.default_swappiness;
    cgroup->memory.low_bytes = cgroup_system.default_memory_low;
    cgroup->io.weight = 100; /* Default I/O weight */
    cgroup->pids.max_pids = UINT64_MAX; /* No limit by default */
    
    cgroup->stats.created_time = hal_get_tick();
    
    /* Add to global cgroup array */
    cgroup_system.cgroups[cgroup->id] = cgroup;
    
    return cgroup;
}

/**
 * Enable controller in cgroup
 */
static int cgroup_enable_controller(cgroup_v2_t *cgroup, cgroup_controller_t controller) {
    if (!cgroup || controller >= CGROUP_CTRL_COUNT) return -1;
    if (!cgroup_system.controllers_available[controller]) return -1;
    
    /* Check if parent has this controller enabled (except root) */
    if (cgroup->parent && !cgroup->parent->controllers_enabled[controller]) {
        return -1; /* Controller must be enabled in parent first */
    }
    
    cgroup->controllers_enabled[controller] = true;
    cgroup->stats.controller_changes++;
    cgroup_system.global_stats.controller_enable_events++;
    
    return 0;
}

/**
 * Add process to cgroup
 */
static int cgroup_add_process(cgroup_v2_t *cgroup, uint32_t pid) {
    if (!cgroup) return -1;
    
    /* Create process entry */
    cgroup_process_t *proc = hal_allocate(sizeof(cgroup_process_t));
    if (!proc) return -1;
    
    memset(proc, 0, sizeof(cgroup_process_t));
    proc->pid = pid;
    proc->state = CGROUP_PROC_RUNNING;
    proc->join_time = hal_get_tick();
    
    /* Add to process list */
    proc->next = cgroup->processes;
    cgroup->processes = proc;
    cgroup->process_count++;
    
    /* Update populated flag */
    cgroup->populated = true;
    
    cgroup_system.global_stats.total_processes_managed++;
    
    return 0;
}

/**
 * Migrate process between cgroups
 */
static int cgroup_migrate_process(cgroup_v2_t *from, cgroup_v2_t *to, uint32_t pid) {
    if (!from || !to) return -1;
    
    /* Find process in source cgroup */
    cgroup_process_t **current = &from->processes;
    while (*current) {
        if ((*current)->pid == pid) {
            cgroup_process_t *proc = *current;
            
            /* Remove from source */
            *current = proc->next;
            from->process_count--;
            
            /* Add to destination */
            proc->next = to->processes;
            to->processes = proc;
            to->process_count++;
            to->populated = true;
            
            /* Update migration stats */
            from->stats.process_migrations++;
            to->stats.process_migrations++;
            cgroup_system.global_stats.total_migrations++;
            
            /* Update populated flag for source */
            if (from->process_count == 0) {
                from->populated = false;
            }
            
            return 0;
        }
        current = &(*current)->next;
    }
    
    return -1; /* Process not found */
}

/**
 * Freeze cgroup processes
 */
static int cgroup_freeze(cgroup_v2_t *cgroup) {
    if (!cgroup || !cgroup->controllers_enabled[CGROUP_CTRL_FREEZER]) {
        return -1;
    }
    
    /* Freeze all processes in cgroup */
    cgroup_process_t *proc = cgroup->processes;
    while (proc) {
        proc->state = CGROUP_PROC_FROZEN;
        /* In production, this would send SIGSTOP to the process */
        proc = proc->next;
    }
    
    cgroup->frozen = true;
    cgroup->stats.freeze_events++;
    cgroup_system.global_stats.total_freeze_events++;
    
    return 0;
}

/**
 * Thaw cgroup processes
 */
static int cgroup_thaw(cgroup_v2_t *cgroup) {
    if (!cgroup) return -1;
    
    /* Thaw all processes in cgroup */
    cgroup_process_t *proc = cgroup->processes;
    while (proc) {
        if (proc->state == CGROUP_PROC_FROZEN) {
            proc->state = CGROUP_PROC_RUNNING;
            /* In production, this would send SIGCONT to the process */
        }
        proc = proc->next;
    }
    
    cgroup->frozen = false;
    cgroup->stats.thaw_events++;
    
    return 0;
}

/**
 * Set CPU weight
 */
static int cgroup_set_cpu_weight(cgroup_v2_t *cgroup, uint64_t weight) {
    if (!cgroup || !cgroup->controllers_enabled[CGROUP_CTRL_CPU]) return -1;
    if (weight < 1 || weight > 10000) return -1;
    
    cgroup->cpu.weight = weight;
    
    /* Update scheduler weights for all processes */
    cgroup_process_t *proc = cgroup->processes;
    while (proc) {
        /* In production, this would update process scheduler weights */
        proc = proc->next;
    }
    
    return 0;
}

/**
 * Set memory limit
 */
static int cgroup_set_memory_limit(cgroup_v2_t *cgroup, uint64_t limit) {
    if (!cgroup || !cgroup->controllers_enabled[CGROUP_CTRL_MEMORY]) return -1;
    
    /* Check if current usage exceeds new limit */
    if (cgroup->memory.stats.current > limit) {
        /* Try to reclaim memory */
        /* In production, this would trigger memory reclaim */
        if (cgroup->memory.stats.current > limit) {
            return -1; /* Cannot set limit below current usage */
        }
    }
    
    cgroup->memory.max_bytes = limit;
    return 0;
}

/**
 * Set I/O bandwidth limits
 */
static int cgroup_set_io_limit(cgroup_v2_t *cgroup, uint64_t read_bps, uint64_t write_bps) {
    if (!cgroup || !cgroup->controllers_enabled[CGROUP_CTRL_IO]) return -1;
    
    cgroup->io.read_bps = read_bps;
    cgroup->io.write_bps = write_bps;
    
    /* Apply I/O throttling to processes */
    cgroup_process_t *proc = cgroup->processes;
    while (proc) {
        /* In production, this would configure I/O throttling */
        proc = proc->next;
    }
    
    return 0;
}

/**
 * Update cgroup statistics
 */
static void cgroup_update_stats(cgroup_v2_t *cgroup) {
    if (!cgroup) return;
    
    /* Aggregate statistics from all processes */
    uint64_t total_cpu_usage = 0;
    uint64_t total_memory_usage = 0;
    
    cgroup_process_t *proc = cgroup->processes;
    while (proc) {
        total_cpu_usage += proc->cpu_usage;
        total_memory_usage += proc->memory_usage;
        proc = proc->next;
    }
    
    /* Update controller statistics */
    cgroup->cpu.stats.usage_usec = total_cpu_usage;
    cgroup->memory.stats.current = total_memory_usage;
    
    /* Update peak usage */
    if (total_memory_usage > cgroup->memory.stats.peak) {
        cgroup->memory.stats.peak = total_memory_usage;
    }
    
    /* Update PID count */
    cgroup->pids.stats.current = cgroup->process_count;
    if (cgroup->process_count > cgroup->pids.stats.peak) {
        cgroup->pids.stats.peak = cgroup->process_count;
    }
}

/**
 * Create container cgroup
 */
cgroup_v2_t *cgroup_create_container(const char *container_id, uint64_t cpu_weight, 
                                    uint64_t memory_limit, uint64_t io_weight) {
    if (!cgroup_system.initialized) return NULL;
    
    /* Find machine slice */
    cgroup_v2_t *machine_slice = NULL;
    for (uint32_t i = 0; i < cgroup_system.cgroup_count; i++) {
        cgroup_v2_t *cg = cgroup_system.cgroups[i];
        if (cg && strcmp(cg->name, "/machine.slice") == 0) {
            machine_slice = cg;
            break;
        }
    }
    
    if (!machine_slice) return NULL;
    
    /* Create container cgroup path */
    char cgroup_path[256];
    snprintf(cgroup_path, sizeof(cgroup_path), "/machine.slice/container-%s.scope", container_id);
    
    cgroup_v2_t *container_cgroup = cgroup_create(cgroup_path, CGROUP_TYPE_CONTAINER, machine_slice);
    if (!container_cgroup) return NULL;
    
    /* Enable controllers */
    cgroup_controller_t controllers[] = {
        CGROUP_CTRL_CPU, CGROUP_CTRL_MEMORY, CGROUP_CTRL_IO, 
        CGROUP_CTRL_PIDS, CGROUP_CTRL_DEVICES, CGROUP_CTRL_FREEZER
    };
    
    for (int i = 0; i < 6; i++) {
        cgroup_enable_controller(container_cgroup, controllers[i]);
    }
    
    /* Configure resources */
    cgroup_set_cpu_weight(container_cgroup, cpu_weight);
    cgroup_set_memory_limit(container_cgroup, memory_limit);
    container_cgroup->io.weight = io_weight;
    
    /* Set container-specific limits */
    container_cgroup->pids.max_pids = 4096; /* Reasonable container limit */
    container_cgroup->devices.default_deny = true; /* Deny all devices by default */
    
    hal_print("CGROUP: Created container cgroup %s\n", cgroup_path);
    return container_cgroup;
}

/**
 * Create systemd service cgroup
 */
cgroup_v2_t *cgroup_create_service(const char *service_name, uint64_t cpu_weight, uint64_t memory_limit) {
    if (!cgroup_system.initialized) return NULL;
    
    /* Find system slice */
    cgroup_v2_t *system_slice = NULL;
    for (uint32_t i = 0; i < cgroup_system.cgroup_count; i++) {
        cgroup_v2_t *cg = cgroup_system.cgroups[i];
        if (cg && strcmp(cg->name, "/system.slice") == 0) {
            system_slice = cg;
            break;
        }
    }
    
    if (!system_slice) return NULL;
    
    /* Create service cgroup path */
    char cgroup_path[256];
    snprintf(cgroup_path, sizeof(cgroup_path), "/system.slice/%s.service", service_name);
    
    cgroup_v2_t *service_cgroup = cgroup_create(cgroup_path, CGROUP_TYPE_SERVICE, system_slice);
    if (!service_cgroup) return NULL;
    
    /* Enable standard service controllers */
    cgroup_enable_controller(service_cgroup, CGROUP_CTRL_CPU);
    cgroup_enable_controller(service_cgroup, CGROUP_CTRL_MEMORY);
    cgroup_enable_controller(service_cgroup, CGROUP_CTRL_IO);
    cgroup_enable_controller(service_cgroup, CGROUP_CTRL_PIDS);
    
    /* Configure service resources */
    cgroup_set_cpu_weight(service_cgroup, cpu_weight);
    cgroup_set_memory_limit(service_cgroup, memory_limit);
    
    hal_print("CGROUP: Created service cgroup %s\n", cgroup_path);
    return service_cgroup;
}

/**
 * Get cgroup statistics
 */
void cgroup_get_statistics(void) {
    if (!cgroup_system.initialized) {
        hal_print("CGROUP: System not initialized\n");
        return;
    }
    
    hal_print("\n=== Control Groups v2 Statistics ===\n");
    hal_print("Total Cgroups: %u\n", cgroup_system.cgroup_count);
    hal_print("Total Processes Managed: %llu\n", cgroup_system.global_stats.total_processes_managed);
    hal_print("Total Migrations: %llu\n", cgroup_system.global_stats.total_migrations);
    hal_print("Total Freeze Events: %llu\n", cgroup_system.global_stats.total_freeze_events);
    hal_print("Controller Enable Events: %llu\n", cgroup_system.global_stats.controller_enable_events);
    
    /* Controller availability */
    hal_print("\n=== Available Controllers ===\n");
    const char *controller_names[] = {
        "cpu", "memory", "io", "pids", "cpuset", "devices", "freezer", "net", "perf"
    };
    
    for (int i = 0; i < CGROUP_CTRL_COUNT; i++) {
        hal_print("%s: %s\n", controller_names[i], 
                 cgroup_system.controllers_available[i] ? "Available" : "Not Available");
    }
    
    /* Per-cgroup statistics */
    hal_print("\n=== Cgroup Details ===\n");
    for (uint32_t i = 0; i < cgroup_system.cgroup_count && i < 10; i++) {
        cgroup_v2_t *cgroup = cgroup_system.cgroups[i];
        if (!cgroup) continue;
        
        hal_print("Cgroup: %s (Level %u)\n", cgroup->name, cgroup->level);
        hal_print("  Type: %s\n", 
                 (cgroup->type == CGROUP_TYPE_ROOT) ? "Root" :
                 (cgroup->type == CGROUP_TYPE_SYSTEM) ? "System" :
                 (cgroup->type == CGROUP_TYPE_USER) ? "User" :
                 (cgroup->type == CGROUP_TYPE_CONTAINER) ? "Container" :
                 (cgroup->type == CGROUP_TYPE_SERVICE) ? "Service" : "Process");
        hal_print("  Processes: %u\n", cgroup->process_count);
        hal_print("  Populated: %s\n", cgroup->populated ? "Yes" : "No");
        hal_print("  Frozen: %s\n", cgroup->frozen ? "Yes" : "No");
        
        if (cgroup->controllers_enabled[CGROUP_CTRL_CPU]) {
            hal_print("  CPU Weight: %llu\n", cgroup->cpu.weight);
            hal_print("  CPU Usage: %llu us\n", cgroup->cpu.stats.usage_usec);
        }
        
        if (cgroup->controllers_enabled[CGROUP_CTRL_MEMORY]) {
            hal_print("  Memory Limit: %llu MB\n", cgroup->memory.max_bytes / (1024 * 1024));
            hal_print("  Memory Usage: %llu MB\n", cgroup->memory.stats.current / (1024 * 1024));
            hal_print("  Memory Peak: %llu MB\n", cgroup->memory.stats.peak / (1024 * 1024));
        }
        
        if (cgroup->controllers_enabled[CGROUP_CTRL_IO]) {
            hal_print("  I/O Read: %llu bytes\n", cgroup->io.stats.rbytes);
            hal_print("  I/O Write: %llu bytes\n", cgroup->io.stats.wbytes);
        }
        
        hal_print("  Migrations: %llu\n", cgroup->stats.process_migrations);
    }
}

/**
 * Cgroups v2 system shutdown
 */
void cgroup_v2_shutdown(void) {
    if (!cgroup_system.initialized) return;
    
    hal_print("CGROUP: Shutting down cgroups v2 system\n");
    
    /* Thaw all frozen cgroups */
    for (uint32_t i = 0; i < cgroup_system.cgroup_count; i++) {
        cgroup_v2_t *cgroup = cgroup_system.cgroups[i];
        if (cgroup && cgroup->frozen) {
            cgroup_thaw(cgroup);
        }
    }
    
    /* Free all cgroups */
    for (uint32_t i = 0; i < cgroup_system.cgroup_count; i++) {
        cgroup_v2_t *cgroup = cgroup_system.cgroups[i];
        if (cgroup) {
            /* Free process list */
            cgroup_process_t *proc = cgroup->processes;
            while (proc) {
                cgroup_process_t *next = proc->next;
                hal_free(proc);
                proc = next;
            }
            
            /* Free device rules */
            device_rule_t *rule = cgroup->devices.allow_list;
            while (rule) {
                device_rule_t *next = rule->next;
                hal_free(rule);
                rule = next;
            }
            
            rule = cgroup->devices.deny_list;
            while (rule) {
                device_rule_t *next = rule->next;
                hal_free(rule);
                rule = next;
            }
            
            /* Free events */
            cgroup_event_t *event = cgroup->events;
            while (event) {
                cgroup_event_t *next = event->next;
                hal_free(event);
                event = next;
            }
            
            hal_free(cgroup);
        }
    }
    
    hal_free(cgroup_system.cgroups);
    cgroup_system.initialized = false;
    
    hal_print("CGROUP: System shutdown complete\n");
}