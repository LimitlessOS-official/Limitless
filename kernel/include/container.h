/*
 * container.h - LimitlessOS Container Support Header
 * 
 * Defines container structures, namespaces, and cgroups interfaces
 * for full container runtime support.
 */

#ifndef LIMITLESSOS_CONTAINER_H
#define LIMITLESSOS_CONTAINER_H

#include "kernel.h"
#include "process.h"

/* Forward declarations */
typedef struct cgroup cgroup_t;
typedef struct namespace namespace_t;

/* Container limits */
#define MAX_CONTAINERS              256
#define MAX_CONTAINER_IMAGES        128
#define MAX_CONTAINER_COMMAND_ARGS  32
#define MAX_CONTAINER_ENV_VARS      64
#define MAX_CONTAINER_MOUNTS        32

/* Container namespace flags */
typedef enum {
    CONTAINER_NS_PID     = (1 << 0),
    CONTAINER_NS_MOUNT   = (1 << 1),
    CONTAINER_NS_NETWORK = (1 << 2),
    CONTAINER_NS_UTS     = (1 << 3),
    CONTAINER_NS_IPC     = (1 << 4),
    CONTAINER_NS_USER    = (1 << 5),
    CONTAINER_NS_CGROUP  = (1 << 6),
    CONTAINER_NS_ALL     = 0x7F,
} container_namespace_flags_t;

/* Container states */
typedef enum {
    CONTAINER_STATE_CREATED = 0,
    CONTAINER_STATE_STARTING,
    CONTAINER_STATE_RUNNING,
    CONTAINER_STATE_STOPPING,
    CONTAINER_STATE_STOPPED,
    CONTAINER_STATE_EXITED,
    CONTAINER_STATE_ERROR,
} container_state_t;

/* Namespace types */
typedef enum {
    NS_TYPE_PID = 0,
    NS_TYPE_MOUNT,
    NS_TYPE_NETWORK,
    NS_TYPE_UTS,
    NS_TYPE_IPC,
    NS_TYPE_USER,
    NS_TYPE_CGROUP,
} namespace_type_t;

/* Forward declarations */
typedef struct namespace namespace_t;
typedef struct cgroup cgroup_t;
typedef struct cgroup_hierarchy cgroup_hierarchy_t;
typedef struct container_image container_image_t;

/* Container mount */
typedef struct container_mount {
    char source[256];
    char target[256];
    char type[32];
    uint32_t flags;
    char options[128];
} container_mount_t;

/* Container environment variable */
typedef struct container_env_var {
    char name[64];
    char value[256];
} container_env_var_t;

/* Container network configuration */
typedef struct container_network_config {
    bool use_host_network;
    char bridge_name[32];
    char ip_address[16];
    char netmask[16];
    char gateway[16];
    uint16_t* port_mappings;
    uint32_t port_mapping_count;
} container_network_config_t;

/* Container configuration */
typedef struct container_config {
    char name[64];
    char image_name[128];
    char command[MAX_CONTAINER_COMMAND_ARGS][128];
    char working_dir[256];
    char hostname[64];
    
    /* Security */
    uid_t user_id;
    gid_t group_id;
    uint64_t capabilities;
    bool privileged;
    
    /* Namespaces */
    container_namespace_flags_t namespaces;
    
    /* Environment */
    container_env_var_t environment[MAX_CONTAINER_ENV_VARS];
    uint32_t env_count;
    
    /* Mounts */
    container_mount_t mounts[MAX_CONTAINER_MOUNTS];
    uint32_t mount_count;
    
    /* Resource limits */
    uint64_t memory_limit;      /* Bytes */
    uint32_t cpu_shares;        /* Relative CPU weight */
    uint32_t cpu_period;        /* CFS period in microseconds */
    uint32_t cpu_quota;         /* CFS quota in microseconds */
    uint32_t max_open_files;
    uint32_t max_threads;
    uint32_t max_processes;
    
    /* Filesystem */
    bool use_overlay_fs;
    bool read_only_root;
    
    /* Network */
    container_network_config_t network;
    
    /* Runtime options */
    bool auto_remove;           /* Remove container when it exits */
    bool interactive;           /* Keep STDIN open */
    bool tty;                  /* Allocate pseudo-TTY */
} container_config_t;

/* Container namespaces */
typedef struct container_namespaces {
    namespace_t* pid_ns;
    namespace_t* mount_ns;
    namespace_t* net_ns;
    namespace_t* uts_ns;
    namespace_t* ipc_ns;
    namespace_t* user_ns;
    namespace_t* cgroup_ns;
} container_namespaces_t;

/* Container statistics */
typedef struct container_stats {
    uint64_t memory_usage;
    uint64_t memory_limit;
    uint64_t cpu_usage_ns;
    uint64_t cpu_throttled_ns;
    uint64_t network_rx_bytes;
    uint64_t network_tx_bytes;
    uint64_t block_read_bytes;
    uint64_t block_write_bytes;
    uint32_t pids_current;
    uint32_t pids_limit;
} container_stats_t;

/* Container */
typedef struct container {
    uint64_t id;
    char name[64];
    char image_name[128];
    container_state_t state;
    
    container_config_t config;
    container_namespaces_t namespaces;
    
    /* Process management */
    process_t* init_process;
    pid_t* child_pids;
    uint32_t child_count;
    
    /* Resource management */
    cgroup_t* cgroup;
    
    /* Filesystem */
    char root_path[256];
    
    /* Network */
    void* network_interface;
    
    /* Timestamps */
    uint64_t created_time;
    uint64_t started_time;
    uint64_t stopped_time;
    
    /* Runtime state */
    int exit_code;
    char exit_reason[128];
    
    /* Statistics */
    container_stats_t stats;
} container_t;

/* Container image layer */
typedef struct container_layer {
    char digest[65];            /* SHA256 digest */
    uint64_t size;
    char mount_path[256];
    bool read_only;
    struct container_layer* parent;
} container_layer_t;

/* Container image */
typedef struct container_image {
    char name[128];
    char tag[32];
    char digest[65];
    uint64_t size;
    
    container_layer_t* layers;
    uint32_t layer_count;
    
    /* Metadata */
    char created[32];
    char author[128];
    char architecture[32];
    char os[32];
    
    /* Configuration */
    container_config_t default_config;
    
    /* Registry info */
    char registry_url[256];
    bool is_local;
} container_image_t;

/* Container info for listing */
typedef struct container_info {
    uint64_t id;
    char name[64];
    char image[128];
    container_state_t state;
    uint64_t created_time;
    uint64_t started_time;
    uint64_t stopped_time;
    uint64_t memory_usage;
    uint64_t cpu_usage;
} container_info_t;

/* CGroup controller */
typedef struct cgroup_controller {
    char name[32];
    bool enabled;
    void* private_data;
} cgroup_controller_t;

/* CGroup */
typedef struct cgroup {
    char path[256];
    cgroup_hierarchy_t* hierarchy;
    cgroup_controller_t controllers[8];
    uint32_t controller_count;
    
    /* Resource limits */
    uint64_t memory_limit;
    uint32_t cpu_shares;
    uint32_t cpu_period;
    uint32_t cpu_quota;
    
    /* Process list */
    pid_t* pids;
    uint32_t pid_count;
    uint32_t max_pids;
    
    spinlock_t lock;
} cgroup_t;

/* CGroup hierarchy */
typedef struct cgroup_hierarchy {
    cgroup_t root;
    cgroup_controller_t* controllers;
    uint32_t controller_count;
    char mount_point[256];
    spinlock_t lock;
} cgroup_hierarchy_t;

/* Namespace */
typedef struct namespace {
    namespace_type_t type;
    uint32_t id;
    uint32_t ref_count;
    
    union {
        struct {
            pid_t next_pid;
            pid_t max_pid;
        } pid_ns;
        
        struct {
            char hostname[64];
            char domainname[64];
        } uts_ns;
        
        struct {
            void* mount_tree;
        } mount_ns;
        
        struct {
            void* network_stack;
        } net_ns;
        
        struct {
            uid_t uid_map[32][3];   /* [inside, outside, length] */
            gid_t gid_map[32][3];
            uint32_t uid_map_count;
            uint32_t gid_map_count;
        } user_ns;
        
        struct {
            void* ipc_objects;
        } ipc_ns;
    } data;
    
    spinlock_t lock;
} namespace_t;

/* Core Container API */
status_t container_init(void);
status_t container_shutdown(void);

/* Container Management */
status_t container_create(const container_config_t* config, container_t** container_out);
status_t container_start(container_t* container);
status_t container_stop(container_t* container, uint32_t timeout_seconds);
status_t container_pause(container_t* container);
status_t container_resume(container_t* container);
status_t container_remove(container_t* container, bool force);

/* Container Runtime (Docker-compatible) */
status_t container_runtime_create(const char* name, const char* image, 
                                const char** command, const char* const* env);
status_t container_runtime_run(const char* image, const char** command);
status_t container_runtime_exec(const char* container_name, const char** command);

/* Container Information */
container_t* container_find_by_name(const char* name);
container_t* container_find_by_id(uint64_t id);
status_t container_list(container_info_t** containers, uint32_t* count);
status_t container_get_stats(container_t* container, container_stats_t* stats);
status_t container_get_logs(container_t* container, char** log_data, uint32_t* log_size);

/* Image Management */
status_t container_image_pull(const char* image_name, const char* tag);
status_t container_image_list(container_image_t** images, uint32_t* count);
status_t container_image_remove(const char* image_name, const char* tag);
status_t container_image_build(const char* dockerfile_path, const char* image_name, const char* tag);

/* Namespace Operations */
status_t namespace_init(void);
status_t namespace_create(namespace_type_t type, namespace_t** ns_out);
status_t namespace_destroy(namespace_t* ns);
status_t namespace_enter(namespace_t* ns);
status_t namespace_set_hostname(namespace_t* ns, const char* hostname);
status_t namespace_map_user(namespace_t* ns, uid_t inside_uid, uid_t outside_uid);
status_t namespace_map_group(namespace_t* ns, gid_t inside_gid, gid_t outside_gid);

/* CGroup Operations */
status_t cgroup_init_hierarchy(cgroup_hierarchy_t* hierarchy);
status_t cgroup_create_controller(cgroup_hierarchy_t* hierarchy, const char* name);
status_t cgroup_create(cgroup_hierarchy_t* hierarchy, const char* path, cgroup_t** cgroup_out);
status_t cgroup_destroy(cgroup_t* cgroup);
status_t cgroup_add_process(cgroup_t* cgroup, pid_t pid);
status_t cgroup_remove_process(cgroup_t* cgroup, pid_t pid);
status_t cgroup_set_memory_limit(cgroup_t* cgroup, uint64_t limit);
status_t cgroup_set_cpu_shares(cgroup_t* cgroup, uint32_t shares);
status_t cgroup_set_cpu_cfs(cgroup_t* cgroup, uint32_t period, uint32_t quota);
status_t cgroup_get_memory_usage(cgroup_t* cgroup, uint64_t* usage);
status_t cgroup_get_cpu_usage(cgroup_t* cgroup, uint64_t* usage);

/* Internal functions */
status_t container_create_namespaces(container_t* container);
status_t container_destroy_namespaces(container_t* container);
status_t container_create_cgroup(container_t* container);
status_t container_destroy_cgroup(container_t* container);
status_t container_setup_filesystem(container_t* container);
status_t container_mount_overlay_fs(container_t* container, const char* target);
status_t container_create_init_process(container_t* container);
status_t container_apply_resource_limits(container_t* container);
status_t container_setup_network(container_t* container);
status_t container_apply_security(container_t* container);
void container_kill_all_processes(container_t* container);

/* Process integration */
status_t process_set_namespaces(process_t* process, const container_namespaces_t* namespaces);
status_t process_chroot(process_t* process, const char* root_path);
status_t process_set_cwd(process_t* process, const char* path);
status_t process_set_env(process_t* process, const char* name, const char* value);

/* VFS integration */
status_t vfs_bind_mount(const char* source, const char* target, uint32_t flags);

/* Event callbacks */
void container_on_process_exit(process_t* process);

#endif /* LIMITLESSOS_CONTAINER_H */