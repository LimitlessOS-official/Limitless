/*
 * LimitlessOS Advanced Container Runtime Header
 * Enterprise-grade containerization with namespaces, cgroups, Docker API compatibility
 */

#ifndef ADVANCED_CONTAINER_H
#define ADVANCED_CONTAINER_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * NAMESPACE DEFINITIONS
 * ============================================================================ */

/* Namespace types */
typedef enum {
    NS_TYPE_PID = 0,        /* Process ID namespace */
    NS_TYPE_NET = 1,        /* Network namespace */
    NS_TYPE_MNT = 2,        /* Mount namespace */
    NS_TYPE_USER = 3,       /* User namespace */
    NS_TYPE_UTS = 4,        /* UTS (hostname/domain) namespace */
    NS_TYPE_IPC = 5,        /* IPC namespace */
    NS_TYPE_CGROUP = 6,     /* Cgroup namespace */
    NS_TYPE_TIME = 7,       /* Time namespace (enterprise extension) */
    NS_TYPE_MAX = 8
} ns_type_t;

/* Namespace flags for clone() */
#define CLONE_NEWPID    0x20000000  /* New PID namespace */
#define CLONE_NEWNET    0x40000000  /* New network namespace */
#define CLONE_NEWNS     0x00020000  /* New mount namespace */
#define CLONE_NEWUSER   0x10000000  /* New user namespace */
#define CLONE_NEWUTS    0x04000000  /* New UTS namespace */
#define CLONE_NEWIPC    0x08000000  /* New IPC namespace */
#define CLONE_NEWCGROUP 0x02000000  /* New cgroup namespace */

/* Namespace type names for logging */
static const char* namespace_type_names[] = {
    "PID", "Network", "Mount", "User", "UTS", "IPC", "Cgroup", "Time"
};

/* Forward declarations */
typedef struct namespace namespace_t;
typedef struct cgroup cgroup_t;
typedef struct container container_t;

/* ============================================================================
 * CGROUPS V2 DEFINITIONS
 * ============================================================================ */

/* Cgroup controller types */
typedef enum {
    CGROUP_CTRL_CPU = 0,        /* CPU controller */
    CGROUP_CTRL_MEMORY = 1,     /* Memory controller */
    CGROUP_CTRL_IO = 2,         /* Block I/O controller */
    CGROUP_CTRL_PIDS = 3,       /* Process number controller */
    CGROUP_CTRL_CPUSET = 4,     /* CPU set controller */
    CGROUP_CTRL_DEVICES = 5,    /* Device access controller */
    CGROUP_CTRL_FREEZER = 6,    /* Process freezer */
    CGROUP_CTRL_NET_CLS = 7,    /* Network classifier */
    CGROUP_CTRL_NET_PRIO = 8,   /* Network priority */
    CGROUP_CTRL_HUGETLB = 9,    /* Huge pages controller */
    CGROUP_CTRL_PERF = 10,      /* Performance events */
    CGROUP_CTRL_RDMA = 11,      /* RDMA controller */
    CGROUP_CTRL_MAX = 12
} cgroup_controller_t;

/* Cgroup resource limits */
typedef struct cgroup_limits {
    /* CPU limits */
    uint64_t cpu_shares;            /* CPU shares (relative weight) */
    uint64_t cpu_quota_us;          /* CPU quota in microseconds */
    uint64_t cpu_period_us;         /* CPU period in microseconds */
    uint32_t cpu_rt_runtime_us;     /* RT CPU runtime */
    uint32_t cpu_rt_period_us;      /* RT CPU period */
    
    /* Memory limits */
    uint64_t memory_limit_bytes;    /* Memory limit */
    uint64_t memory_soft_limit_bytes; /* Soft memory limit */
    uint64_t memory_swap_limit_bytes; /* Memory + swap limit */
    uint64_t kernel_memory_limit_bytes; /* Kernel memory limit */
    
    /* Block I/O limits */
    uint64_t blkio_weight;          /* Block I/O weight */
    uint64_t blkio_read_bps;        /* Read bytes per second */
    uint64_t blkio_write_bps;       /* Write bytes per second */
    uint64_t blkio_read_iops;       /* Read I/O operations per second */
    uint64_t blkio_write_iops;      /* Write I/O operations per second */
    
    /* Process limits */
    uint32_t pids_max;              /* Maximum number of processes */
    uint32_t pids_current;          /* Current number of processes */
    
    /* Network limits */
    uint64_t net_tx_bytes;          /* Network TX byte limit */
    uint64_t net_rx_bytes;          /* Network RX byte limit */
    uint32_t net_priority;          /* Network priority */
    
    /* Device access */
    bool devices_allow_all;         /* Allow all devices */
    struct {
        char device_path[256];      /* Device path */
        char permissions[8];        /* rwm permissions */
    } device_rules[64];
    uint32_t device_rule_count;
    
} cgroup_limits_t;

/* Cgroup statistics */
typedef struct cgroup_stats {
    /* CPU usage */
    uint64_t cpu_usage_ns;          /* Total CPU usage in nanoseconds */
    uint64_t cpu_user_ns;           /* User CPU time */
    uint64_t cpu_system_ns;         /* System CPU time */
    uint32_t cpu_throttled_count;   /* Number of throttling events */
    uint64_t cpu_throttled_time_ns; /* Total throttled time */
    
    /* Memory usage */
    uint64_t memory_usage_bytes;    /* Current memory usage */
    uint64_t memory_max_usage_bytes; /* Peak memory usage */
    uint64_t memory_failcnt;        /* Memory allocation failures */
    uint64_t memory_cache_bytes;    /* Page cache memory */
    uint64_t memory_rss_bytes;      /* RSS memory */
    uint64_t memory_swap_bytes;     /* Swap usage */
    
    /* Block I/O statistics */
    uint64_t blkio_read_bytes;      /* Bytes read */
    uint64_t blkio_write_bytes;     /* Bytes written */
    uint64_t blkio_read_ops;        /* Read operations */
    uint64_t blkio_write_ops;       /* Write operations */
    uint64_t blkio_sync_ops;        /* Synchronous operations */
    uint64_t blkio_async_ops;       /* Asynchronous operations */
    
    /* Network statistics */
    uint64_t net_tx_bytes;          /* Transmitted bytes */
    uint64_t net_rx_bytes;          /* Received bytes */
    uint64_t net_tx_packets;        /* Transmitted packets */
    uint64_t net_rx_packets;        /* Received packets */
    
} cgroup_stats_t;

/* ============================================================================
 * CONTAINER DEFINITIONS
 * ============================================================================ */

/* Container states */
typedef enum {
    CONTAINER_STATE_CREATED = 0,    /* Container created but not started */
    CONTAINER_STATE_RUNNING = 1,    /* Container is running */
    CONTAINER_STATE_PAUSED = 2,     /* Container is paused */
    CONTAINER_STATE_STOPPED = 3,    /* Container is stopped */
    CONTAINER_STATE_RESTARTING = 4, /* Container is restarting */
    CONTAINER_STATE_REMOVING = 5,   /* Container is being removed */
    CONTAINER_STATE_DEAD = 6        /* Container process is dead */
} container_state_t;

/* Container configuration */
typedef struct container_config {
    char name[256];                 /* Container name */
    char image[512];                /* Container image */
    char hostname[64];              /* Container hostname */
    char working_dir[512];          /* Working directory */
    
    /* Command and environment */
    char* cmd[32];                  /* Command to execute */
    char* env[64];                  /* Environment variables */
    uint32_t cmd_count;
    uint32_t env_count;
    
    /* Networking */
    char network_mode[64];          /* Network mode (bridge, host, none) */
    struct {
        uint16_t container_port;
        uint16_t host_port;
        char protocol[8];           /* tcp, udp */
    } port_mappings[32];
    uint32_t port_mapping_count;
    
    /* Volumes and mounts */
    struct {
        char source[512];           /* Host path */
        char destination[512];      /* Container path */
        char mode[16];              /* ro, rw */
    } mounts[32];
    uint32_t mount_count;
    
    /* Security */
    bool privileged;                /* Privileged container */
    char user[64];                  /* User to run as */
    char* capabilities_add[32];     /* Added capabilities */
    char* capabilities_drop[32];    /* Dropped capabilities */
    uint32_t cap_add_count;
    uint32_t cap_drop_count;
    
    /* Resource limits (cgroup integration) */
    cgroup_limits_t resource_limits;
    
} container_config_t;

/* Container statistics */
typedef struct container_stats {
    /* Basic stats */
    container_state_t state;        /* Current state */
    uint64_t uptime_ns;            /* Container uptime */
    uint32_t process_count;         /* Number of processes */
    int exit_code;                  /* Exit code if stopped */
    
    /* Resource usage (from cgroup) */
    cgroup_stats_t resource_usage;
    
    /* Network statistics */
    uint64_t network_tx_bytes;      /* Network bytes transmitted */
    uint64_t network_rx_bytes;      /* Network bytes received */
    uint64_t network_tx_packets;    /* Network packets transmitted */
    uint64_t network_rx_packets;    /* Network packets received */
    
    /* Filesystem statistics */
    uint64_t fs_read_bytes;         /* Filesystem bytes read */
    uint64_t fs_write_bytes;        /* Filesystem bytes written */
    uint64_t fs_read_ops;           /* Filesystem read operations */
    uint64_t fs_write_ops;          /* Filesystem write operations */
    
} container_stats_t;

/* ============================================================================
 * CORE CONTAINER API
 * ============================================================================ */

/**
 * Initialize advanced container runtime
 * @return 0 on success, negative on error
 */
int advanced_container_init(void);

/**
 * Create a new container
 * @param config Container configuration
 * @return Container handle on success, NULL on error
 */
container_t* container_create(const container_config_t* config);

/**
 * Start a container
 * @param container Container to start
 * @return 0 on success, negative on error
 */
int container_start(container_t* container);

/**
 * Stop a container
 * @param container Container to stop
 * @param timeout_seconds Timeout for graceful shutdown
 * @return 0 on success, negative on error
 */
int container_stop(container_t* container, int timeout_seconds);

/**
 * Pause a container
 * @param container Container to pause
 * @return 0 on success, negative on error
 */
int container_pause(container_t* container);

/**
 * Resume a paused container
 * @param container Container to resume
 * @return 0 on success, negative on error
 */
int container_resume(container_t* container);

/**
 * Remove a container
 * @param container Container to remove
 * @param force Force removal even if running
 * @return 0 on success, negative on error
 */
int container_remove(container_t* container, bool force);

/**
 * Find container by ID
 * @param container_id Container ID (short or full)
 * @return Container handle on success, NULL if not found
 */
container_t* container_find_by_id(const char* container_id);

/**
 * Find container by name
 * @param name Container name
 * @return Container handle on success, NULL if not found
 */
container_t* container_find_by_name(const char* name);

/**
 * Get container statistics
 * @param container Container to get stats for
 * @param stats Output for statistics
 * @return 0 on success, negative on error
 */
int container_get_stats(container_t* container, container_stats_t* stats);

/* ============================================================================
 * NAMESPACE API
 * ============================================================================ */

/**
 * Create a new namespace
 * @param type Namespace type
 * @return Namespace handle on success, NULL on error
 */
namespace_t* namespace_create(ns_type_t type);

/**
 * Enter a namespace
 * @param ns Namespace to enter
 * @param type Namespace type
 * @return 0 on success, negative on error
 */
int namespace_enter(namespace_t* ns, ns_type_t type);

/**
 * Get current namespace for type
 * @param type Namespace type
 * @return Current namespace handle, NULL on error
 */
namespace_t* namespace_current(ns_type_t type);

/**
 * Set namespace isolation mode
 * @param ns Namespace to modify
 * @param isolated Enable strict isolation
 * @return 0 on success, negative on error
 */
int namespace_set_isolated(namespace_t* ns, bool isolated);

/* ============================================================================
 * CGROUPS API
 * ============================================================================ */

/**
 * Create a new cgroup
 * @param name Cgroup name/path
 * @param parent Parent cgroup (NULL for root)
 * @return Cgroup handle on success, NULL on error
 */
cgroup_t* cgroup_create(const char* name, cgroup_t* parent);

/**
 * Add process to cgroup
 * @param cgroup Cgroup to add process to
 * @param pid Process ID to add
 * @return 0 on success, negative on error
 */
int cgroup_add_process(cgroup_t* cgroup, uint32_t pid);

/**
 * Remove process from cgroup
 * @param cgroup Cgroup to remove process from
 * @param pid Process ID to remove
 * @return 0 on success, negative on error
 */
int cgroup_remove_process(cgroup_t* cgroup, uint32_t pid);

/**
 * Set cgroup resource limits
 * @param cgroup Cgroup to modify
 * @param limits Resource limits to set
 * @return 0 on success, negative on error
 */
int cgroup_set_limits(cgroup_t* cgroup, const cgroup_limits_t* limits);

/**
 * Get cgroup resource limits
 * @param cgroup Cgroup to query
 * @param limits Output for current limits
 * @return 0 on success, negative on error
 */
int cgroup_get_limits(cgroup_t* cgroup, cgroup_limits_t* limits);

/**
 * Get cgroup statistics
 * @param cgroup Cgroup to query
 * @param stats Output for statistics
 * @return 0 on success, negative on error
 */
int cgroup_get_stats(cgroup_t* cgroup, cgroup_stats_t* stats);

/**
 * Enable cgroup controller
 * @param cgroup Cgroup to modify
 * @param controller Controller to enable
 * @return 0 on success, negative on error
 */
int cgroup_enable_controller(cgroup_t* cgroup, cgroup_controller_t controller);

/**
 * Disable cgroup controller
 * @param cgroup Cgroup to modify
 * @param controller Controller to disable
 * @return 0 on success, negative on error
 */
int cgroup_disable_controller(cgroup_t* cgroup, cgroup_controller_t controller);

/* ============================================================================
 * DOCKER API COMPATIBILITY
 * ============================================================================ */

/**
 * Create container via Docker API
 * @param json_config JSON configuration string
 * @param container_id_out Output buffer for container ID
 * @return 0 on success, negative on error
 */
int docker_api_create_container(const char* json_config, char* container_id_out);

/**
 * Start container via Docker API
 * @param container_id Container ID to start
 * @return 0 on success, negative on error
 */
int docker_api_start_container(const char* container_id);

/**
 * Stop container via Docker API
 * @param container_id Container ID to stop
 * @param timeout Timeout in seconds
 * @return 0 on success, negative on error
 */
int docker_api_stop_container(const char* container_id, int timeout);

/**
 * List containers via Docker API
 * @param all Include stopped containers
 * @param json_out Output buffer for JSON response
 * @param json_size Size of output buffer
 * @return 0 on success, negative on error
 */
int docker_api_list_containers(bool all, char* json_out, size_t json_size);

/**
 * Get container info via Docker API
 * @param container_id Container ID
 * @param json_out Output buffer for JSON response
 * @param json_size Size of output buffer
 * @return 0 on success, negative on error
 */
int docker_api_inspect_container(const char* container_id, char* json_out, size_t json_size);

/**
 * Get container stats via Docker API
 * @param container_id Container ID
 * @param stream Enable streaming mode
 * @param json_out Output buffer for JSON response
 * @param json_size Size of output buffer
 * @return 0 on success, negative on error
 */
int docker_api_stats_container(const char* container_id, bool stream, 
                              char* json_out, size_t json_size);

/* ============================================================================
 * CONTAINER NETWORKING
 * ============================================================================ */

/**
 * Create container network
 * @param name Network name
 * @param driver Network driver (bridge, overlay, etc.)
 * @return Network ID on success, negative on error
 */
int container_network_create(const char* name, const char* driver);

/**
 * Connect container to network
 * @param container Container to connect
 * @param network_name Network name
 * @param ip_address IP address to assign (NULL for auto)
 * @return 0 on success, negative on error
 */
int container_network_connect(container_t* container, const char* network_name, 
                             const char* ip_address);

/**
 * Disconnect container from network
 * @param container Container to disconnect
 * @param network_name Network name
 * @return 0 on success, negative on error
 */
int container_network_disconnect(container_t* container, const char* network_name);

/**
 * Set up port mapping for container
 * @param container Container to configure
 * @param container_port Container port number
 * @param host_port Host port number
 * @param protocol Protocol (tcp, udp)
 * @return 0 on success, negative on error
 */
int container_setup_port_mapping(container_t* container, uint16_t container_port, 
                                uint16_t host_port, const char* protocol);

/* ============================================================================
 * ENTERPRISE FEATURES
 * ============================================================================ */

/**
 * Enable container monitoring
 * @param container Container to monitor
 * @param enable Enable or disable monitoring
 * @return 0 on success, negative on error
 */
int container_enable_monitoring(container_t* container, bool enable);

/**
 * Set container security profile
 * @param container Container to configure
 * @param profile Security profile name
 * @return 0 on success, negative on error
 */
int container_set_security_profile(container_t* container, const char* profile);

/**
 * Create container snapshot
 * @param container Container to snapshot
 * @param snapshot_name Snapshot name
 * @return 0 on success, negative on error
 */
int container_create_snapshot(container_t* container, const char* snapshot_name);

/**
 * Restore container from snapshot
 * @param container Container to restore
 * @param snapshot_name Snapshot name
 * @return 0 on success, negative on error
 */
int container_restore_snapshot(container_t* container, const char* snapshot_name);

/**
 * Enable container encryption
 * @param container Container to encrypt
 * @param key_id Encryption key ID
 * @return 0 on success, negative on error
 */
int container_enable_encryption(container_t* container, uint32_t key_id);

/**
 * Set container resource quotas
 * @param container Container to configure
 * @param cpu_quota CPU quota (percentage)
 * @param memory_quota Memory quota (bytes)
 * @param io_quota I/O quota (IOPS)
 * @return 0 on success, negative on error
 */
int container_set_resource_quotas(container_t* container, uint32_t cpu_quota, 
                                 uint64_t memory_quota, uint64_t io_quota);

/* ============================================================================
 * STATISTICS AND MONITORING
 * ============================================================================ */

/**
 * Print comprehensive container runtime statistics
 */
void container_print_statistics(void);

/**
 * Get container runtime statistics
 * @param total_containers Output for total container count
 * @param running_containers Output for running container count
 * @param cpu_usage_ns Output for total CPU usage
 * @param memory_usage_bytes Output for total memory usage
 * @return 0 on success, negative on error
 */
int container_get_runtime_stats(uint32_t* total_containers, uint32_t* running_containers,
                               uint64_t* cpu_usage_ns, uint64_t* memory_usage_bytes);

/**
 * Container runtime self-test
 * @return 0 on success, negative on test failure
 */
int container_self_test(void);

/**
 * Shutdown container runtime
 */
void container_runtime_shutdown(void);

#endif /* ADVANCED_CONTAINER_H */