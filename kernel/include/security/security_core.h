/*
 * LimitlessOS Security Framework
 * Production security subsystem with mandatory access control and hardening
 * Features: MAC (SELinux-like), sandboxing, secure boot, TPM, kernel hardening
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../kernel.h"

/* Security constants */
#define MAX_SECURITY_CONTEXTS   1024
#define MAX_SECURITY_LABELS     256
#define MAX_SECURITY_RULES      8192
#define MAX_SANDBOX_PROCESSES   512
#define MAX_TPM_KEYS            128
#define MAX_AUDIT_ENTRIES       65536

/* Security subsystem states */
typedef enum {
    SECURITY_DISABLED = 0,
    SECURITY_PERMISSIVE,        /* Log violations but don't enforce */
    SECURITY_ENFORCING,         /* Enforce all security policies */
    SECURITY_MAX
} security_mode_t;

/* Access control models */
typedef enum {
    SECURITY_MODEL_DAC = 0,     /* Discretionary Access Control */
    SECURITY_MODEL_MAC,         /* Mandatory Access Control */
    SECURITY_MODEL_RBAC,        /* Role-Based Access Control */
    SECURITY_MODEL_ABAC,        /* Attribute-Based Access Control */
    SECURITY_MODEL_MAX
} security_model_t;

/* Security label types */
typedef enum {
    SECURITY_LABEL_USER = 0,
    SECURITY_LABEL_ROLE,
    SECURITY_LABEL_TYPE,
    SECURITY_LABEL_LEVEL,
    SECURITY_LABEL_CATEGORY,
    SECURITY_LABEL_MAX
} security_label_type_t;

/* Permission types */
typedef enum {
    PERM_READ = 0x0001,
    PERM_WRITE = 0x0002,
    PERM_EXECUTE = 0x0004,
    PERM_APPEND = 0x0008,
    PERM_CREATE = 0x0010,
    PERM_DELETE = 0x0020,
    PERM_SEARCH = 0x0040,
    PERM_SETATTR = 0x0080,
    PERM_GETATTR = 0x0100,
    PERM_RELABEL = 0x0200,
    PERM_TRANSITION = 0x0400,
    PERM_ENTRYPOINT = 0x0800,
    PERM_ALL = 0xFFFF
} security_permission_t;

/* Object classes */
typedef enum {
    CLASS_FILE = 0,
    CLASS_DIRECTORY,
    CLASS_SOCKET,
    CLASS_PROCESS,
    CLASS_THREAD,
    CLASS_SHARED_MEMORY,
    CLASS_SEMAPHORE,
    CLASS_MESSAGE_QUEUE,
    CLASS_DEVICE,
    CLASS_FILESYSTEM,
    CLASS_NETWORK_INTERFACE,
    CLASS_CAPABILITY,
    CLASS_SYSTEM,
    CLASS_MAX
} security_class_t;

/* Audit event types */
typedef enum {
    AUDIT_ACCESS_GRANTED = 0,
    AUDIT_ACCESS_DENIED,
    AUDIT_POLICY_LOAD,
    AUDIT_POLICY_CHANGE,
    AUDIT_ROLE_CHANGE,
    AUDIT_USER_LOGIN,
    AUDIT_USER_LOGOUT,
    AUDIT_SYSTEM_BOOT,
    AUDIT_SYSTEM_SHUTDOWN,
    AUDIT_INTEGRITY_VIOLATION,
    AUDIT_SANDBOX_VIOLATION,
    AUDIT_TPM_EVENT,
    AUDIT_MAX
} audit_event_type_t;

/* TPM algorithms */
typedef enum {
    TPM_ALG_RSA = 0,
    TPM_ALG_ECC,
    TPM_ALG_SHA1,
    TPM_ALG_SHA256,
    TPM_ALG_SHA384,
    TPM_ALG_SHA512,
    TPM_ALG_AES,
    TPM_ALG_MAX
} tpm_algorithm_t;

/* Security context */
typedef struct security_context {
    uint32_t context_id;        /* Unique context ID */
    char user[64];              /* User identity */
    char role[64];              /* Role identity */
    char type[64];              /* Type identity */
    char level[32];             /* Security level */
    char categories[256];       /* Security categories */
    
    /* Context properties */
    uint32_t sensitivity_level; /* Numerical sensitivity */
    uint32_t category_mask;     /* Category bitmask */
    uint32_t flags;            /* Context flags */
    
    /* MLS (Multi-Level Security) */
    struct {
        uint32_t low_level;     /* Low watermark */
        uint32_t high_level;    /* High watermark */
        uint32_t current_level; /* Current level */
    } mls;
    
    /* Reference counting */
    atomic_t ref_count;
    
    /* Hash for quick lookup */
    struct hlist_node hash;
    
} security_context_t;

/* Security policy rule */
typedef struct security_rule {
    uint32_t rule_id;           /* Rule ID */
    security_class_t source_class;  /* Source object class */
    security_class_t target_class;  /* Target object class */
    
    /* Source context pattern */
    char source_user[64];
    char source_role[64];
    char source_type[64];
    
    /* Target context pattern */
    char target_user[64];
    char target_role[64];
    char target_type[64];
    
    /* Permissions */
    uint32_t allowed_perms;     /* Allowed permissions */
    uint32_t denied_perms;      /* Explicitly denied permissions */
    
    /* Rule conditions */
    struct {
        bool time_based;        /* Time-based rule */
        uint64_t start_time;    /* Rule start time */
        uint64_t end_time;      /* Rule end time */
        
        bool network_based;     /* Network-based rule */
        uint32_t src_network;   /* Source network */
        uint32_t dst_network;   /* Destination network */
        
        bool process_based;     /* Process-based rule */
        char executable[256];   /* Executable name pattern */
        
    } conditions;
    
    /* Rule metadata */
    uint32_t priority;          /* Rule priority */
    bool enabled;              /* Rule is enabled */
    uint64_t created_time;     /* Creation time */
    uint64_t modified_time;    /* Last modification */
    
    /* Usage statistics */
    atomic64_t hit_count;      /* How many times rule was matched */
    uint64_t last_used;        /* Last time rule was used */
    
    struct list_head list;
    
} security_rule_t;

/* Sandbox configuration */
typedef struct sandbox_config {
    uint32_t sandbox_id;        /* Sandbox ID */
    char name[64];              /* Sandbox name */
    
    /* Resource limits */
    struct {
        uint64_t max_memory;    /* Maximum memory usage */
        uint32_t max_processes; /* Maximum number of processes */
        uint32_t max_files;     /* Maximum open files */
        uint32_t max_sockets;   /* Maximum sockets */
        uint64_t max_disk_read; /* Maximum disk read per second */
        uint64_t max_disk_write; /* Maximum disk write per second */
        uint64_t max_network_rx; /* Maximum network receive per second */
        uint64_t max_network_tx; /* Maximum network transmit per second */
    } limits;
    
    /* Filesystem access */
    struct {
        char allowed_paths[32][256]; /* Allowed filesystem paths */
        uint32_t path_count;
        bool read_only;         /* Sandbox is read-only */
        bool no_exec;          /* No execution allowed */
    } filesystem;
    
    /* Network access */
    struct {
        bool network_allowed;   /* Network access allowed */
        uint16_t allowed_ports[64]; /* Allowed network ports */
        uint32_t port_count;
        char allowed_hosts[16][256]; /* Allowed hostnames */
        uint32_t host_count;
    } network;
    
    /* System call filtering */
    struct {
        bool whitelist_mode;    /* True for whitelist, false for blacklist */
        uint32_t syscall_mask[16]; /* Syscall bitmask */
        uint32_t blocked_syscalls[64]; /* Blocked syscalls */
        uint32_t blocked_count;
    } syscalls;
    
    /* Capabilities */
    struct {
        uint64_t permitted;     /* Permitted capabilities */
        uint64_t effective;     /* Effective capabilities */
        uint64_t inheritable;   /* Inheritable capabilities */
        uint64_t bounding;      /* Bounding set */
    } capabilities;
    
    /* Process properties */
    struct {
        bool no_new_privs;      /* No new privileges */
        bool no_setuid;         /* No setuid/setgid */
        bool no_ptrace;         /* No ptrace */
        bool no_core_dump;      /* No core dumps */
    } restrictions;
    
    /* Temporal constraints */
    struct {
        bool time_limited;      /* Time-limited sandbox */
        uint64_t start_time;    /* Start time */
        uint64_t duration_ms;   /* Duration in milliseconds */
    } temporal;
    
} sandbox_config_t;

/* Sandbox instance */
typedef struct sandbox {
    sandbox_config_t config;    /* Sandbox configuration */
    
    /* Active processes */
    struct {
        struct process *processes[MAX_SANDBOX_PROCESSES];
        uint32_t count;
        rwlock_t lock;
    } processes;
    
    /* Resource usage tracking */
    struct {
        atomic64_t memory_used;
        atomic64_t disk_read;
        atomic64_t disk_written;
        atomic64_t network_rx;
        atomic64_t network_tx;
        atomic_t files_open;
        atomic_t sockets_open;
        uint64_t start_time;
    } usage;
    
    /* Violation tracking */
    struct {
        atomic_t violations;
        uint64_t last_violation;
        char last_violation_type[64];
    } violations;
    
    struct list_head list;
    
} sandbox_t;

/* TPM key information */
typedef struct tpm_key {
    uint32_t key_handle;        /* TPM key handle */
    char name[64];              /* Key name */
    tpm_algorithm_t algorithm;  /* Key algorithm */
    uint32_t key_size;          /* Key size in bits */
    
    /* Key properties */
    struct {
        bool signing_key;       /* Key can sign */
        bool encryption_key;    /* Key can encrypt */
        bool storage_key;       /* Key can wrap other keys */
        bool attestation_key;   /* Key can attest */
        bool platform_key;      /* Platform key */
        bool owner_key;         /* Owner key */
    } properties;
    
    /* Key policy */
    struct {
        bool auth_required;     /* Authentication required */
        bool admin_required;    /* Admin privileges required */
        uint32_t usage_count;   /* Usage counter */
        uint32_t max_usage;     /* Maximum usage count */
    } policy;
    
    /* Key metadata */
    uint64_t created_time;      /* Creation time */
    uint64_t last_used;         /* Last usage time */
    atomic_t ref_count;         /* Reference count */
    
    struct list_head list;
    
} tpm_key_t;

/* Audit entry */
typedef struct audit_entry {
    uint32_t entry_id;          /* Entry ID */
    audit_event_type_t type;    /* Event type */
    uint64_t timestamp;         /* Event timestamp */
    
    /* Subject information */
    struct {
        uint32_t pid;           /* Process ID */
        uint32_t uid;           /* User ID */
        uint32_t gid;           /* Group ID */
        char comm[16];          /* Command name */
        security_context_t *context; /* Security context */
    } subject;
    
    /* Object information */
    struct {
        char name[256];         /* Object name */
        security_class_t class; /* Object class */
        security_context_t *context; /* Object context */
    } object;
    
    /* Access information */
    struct {
        uint32_t requested_perms; /* Requested permissions */
        uint32_t granted_perms;   /* Granted permissions */
        int result;             /* Access result */
    } access;
    
    /* Additional data */
    char details[512];          /* Event details */
    
    struct list_head list;
    
} audit_entry_t;

/* Integrity measurement */
typedef struct integrity_measurement {
    uint32_t measurement_id;    /* Measurement ID */
    char name[256];             /* Object name (file, process, etc.) */
    
    /* Hash information */
    struct {
        uint32_t algorithm;     /* Hash algorithm */
        uint8_t hash[64];       /* Hash value */
        size_t hash_len;        /* Hash length */
    } hash;
    
    /* Measurement metadata */
    uint64_t timestamp;         /* Measurement time */
    bool verified;              /* Measurement verified */
    bool trusted;               /* Object is trusted */
    
    /* Template information */
    char template_name[32];     /* Template name */
    uint32_t template_version;  /* Template version */
    
    struct list_head list;
    
} integrity_measurement_t;

/* Global security subsystem */
typedef struct security_subsystem {
    bool initialized;
    security_mode_t mode;       /* Current security mode */
    security_model_t model;     /* Access control model */
    
    /* Security contexts */
    struct {
        security_context_t *contexts[MAX_SECURITY_CONTEXTS];
        uint32_t count;
        struct hash_table *hash_table;
        rwlock_t lock;
    } contexts;
    
    /* Security policy */
    struct {
        security_rule_t rules[MAX_SECURITY_RULES];
        uint32_t rule_count;
        uint32_t policy_version;
        rwlock_t lock;
        bool loaded;
    } policy;
    
    /* Sandboxes */
    struct {
        sandbox_t *sandboxes[MAX_SANDBOX_PROCESSES];
        uint32_t count;
        uint32_t next_sandbox_id;
        rwlock_t lock;
    } sandboxes;
    
    /* TPM subsystem */
    struct {
        bool available;
        bool enabled;
        tmp_key_t *keys[MAX_TPM_KEYS];
        uint32_t key_count;
        struct crypto_hash *pcr_hash; /* PCR hash function */
        uint8_t pcr_values[24][32];   /* PCR values */
        rwlock_t lock;
    } tpm;
    
    /* Audit subsystem */
    struct {
        bool enabled;
        audit_entry_t *entries[MAX_AUDIT_ENTRIES];
        uint32_t count;
        uint32_t head;
        uint32_t tail;
        uint32_t next_entry_id;
        struct workqueue_struct *audit_workqueue;
        spinlock_t lock;
    } audit;
    
    /* Integrity subsystem */
    struct {
        bool enabled;
        struct list_head measurements;
        uint32_t measurement_count;
        struct crypto_hash *hash_tfm;
        rwlock_t lock;
    } integrity;
    
    /* Kernel hardening features */
    struct {
        bool kaslr_enabled;     /* Kernel ASLR */
        bool smep_enabled;      /* Supervisor Mode Execution Prevention */
        bool smap_enabled;      /* Supervisor Mode Access Prevention */
        bool cfi_enabled;       /* Control Flow Integrity */
        bool stack_protector;   /* Stack protection */
        bool heap_protection;   /* Heap protection */
        bool wx_protection;     /* W^X protection */
    } hardening;
    
    /* Statistics */
    struct {
        atomic64_t access_checks;
        atomic64_t access_granted;
        atomic64_t access_denied;
        atomic64_t policy_violations;
        atomic64_t sandbox_violations;
        atomic64_t integrity_violations;
        atomic64_t audit_events;
    } stats;
    
    /* Configuration */
    struct {
        bool debug_enabled;
        bool verbose_logging;
        uint32_t audit_buffer_size;
        uint32_t context_cache_size;
    } config;
    
} security_subsystem_t;

/* External security subsystem */
extern security_subsystem_t security_subsystem;

/* Core security functions */
int security_init(void);
void security_exit(void);

/* Security mode management */
int security_set_mode(security_mode_t mode);
security_mode_t security_get_mode(void);
int security_set_model(security_model_t model);
security_model_t security_get_model(void);

/* Security context management */
struct security_context *security_context_create(const char *user, const char *role, 
                                                const char *type, const char *level);
void security_context_destroy(struct security_context *ctx);
struct security_context *security_context_lookup(uint32_t context_id);
struct security_context *security_context_find(const char *user, const char *role, const char *type);
int security_context_transition(struct security_context *from, struct security_context *to,
                               security_class_t class, const char *name);

/* Access control */
bool security_check_permission(struct security_context *subject_ctx,
                             struct security_context *object_ctx,
                             security_class_t class,
                             uint32_t requested_perms);
int security_compute_av(struct security_context *scontext,
                       struct security_context *tcontext,
                       security_class_t tclass,
                       uint32_t *allowed,
                       uint32_t *denied);

/* Policy management */
int security_load_policy(const void *policy_data, size_t size);
int security_add_rule(const struct security_rule *rule);
int security_remove_rule(uint32_t rule_id);
int security_update_rule(uint32_t rule_id, const struct security_rule *rule);
struct security_rule *security_find_rule(uint32_t rule_id);
int security_validate_policy(void);

/* Sandbox management */
int security_create_sandbox(const struct sandbox_config *config, uint32_t *sandbox_id);
int security_destroy_sandbox(uint32_t sandbox_id);
int security_enter_sandbox(uint32_t sandbox_id, struct process *process);
int security_exit_sandbox(uint32_t sandbox_id, struct process *process);
bool security_check_sandbox_access(uint32_t sandbox_id, const char *path, uint32_t perms);
int security_update_sandbox_limits(uint32_t sandbox_id, const struct sandbox_config *config);

/* TPM operations */
int security_tpm_init(void);
void security_tpm_cleanup(void);
bool security_tpm_available(void);
int security_tpm_create_key(const char *name, tmp_algorithm_t algorithm, uint32_t key_size, uint32_t *key_handle);
int security_tpm_delete_key(uint32_t key_handle);
int security_tpm_sign(uint32_t key_handle, const void *data, size_t data_len, void *signature, size_t *sig_len);
int security_tpm_verify(uint32_t key_handle, const void *data, size_t data_len, const void *signature, size_t sig_len);
int security_tpm_encrypt(uint32_t key_handle, const void *plaintext, size_t plain_len, void *ciphertext, size_t *cipher_len);
int security_tpm_decrypt(uint32_t key_handle, const void *ciphertext, size_t cipher_len, void *plaintext, size_t *plain_len);
int security_tpm_extend_pcr(uint32_t pcr_index, const void *hash, size_t hash_len);
int security_tpm_read_pcr(uint32_t pcr_index, void *value, size_t *value_len);

/* Audit system */
int security_audit_init(void);
void security_audit_cleanup(void);
void security_audit_log(audit_event_type_t type, struct security_context *subject,
                       struct security_context *object, security_class_t class,
                       uint32_t requested_perms, int result, const char *details);
int security_audit_get_events(struct audit_entry *entries, uint32_t max_entries, uint32_t *count);
int security_audit_clear_log(void);
void security_audit_enable(bool enable);

/* Integrity measurement */
int security_integrity_init(void);
void security_integrity_cleanup(void);
int security_integrity_measure_file(const char *path);
int security_integrity_measure_process(struct process *proc);
int security_integrity_verify_measurement(const char *name);
int security_integrity_get_measurements(struct integrity_measurement *measurements, uint32_t max_count, uint32_t *count);
void security_integrity_enable(bool enable);

/* Secure boot */
int security_secure_boot_init(void);
bool security_secure_boot_enabled(void);
int security_verify_kernel_signature(const void *kernel, size_t size);
int security_verify_module_signature(const void *module, size_t size);

/* Kernel hardening */
int security_hardening_init(void);
void security_enable_kaslr(bool enable);
void security_enable_smep(bool enable);
void security_enable_smap(bool enable);
void security_enable_cfi(bool enable);
void security_enable_stack_protector(bool enable);
bool security_check_stack_canary(void);

/* LSM (Linux Security Module) interface */
int security_inode_permission(struct fs_inode *inode, int mask);
int security_file_permission(struct fs_file *file, int mask);
int security_task_create(struct process *parent, struct process *child);
int security_task_setuid(struct process *proc, uint32_t uid);
int security_task_setgid(struct process *proc, uint32_t gid);
int security_socket_create(int family, int type, int protocol);
int security_socket_bind(struct socket *sock, struct sockaddr *address, int addrlen);
int security_socket_connect(struct socket *sock, struct sockaddr *address, int addrlen);

/* Security hooks for system calls */
int security_sys_open(const char *pathname, int flags, uint16_t mode);
int security_sys_read(int fd, void *buf, size_t count);
int security_sys_write(int fd, const void *buf, size_t count);
int security_sys_execve(const char *filename, char *const argv[], char *const envp[]);
int security_sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int security_sys_mprotect(void *addr, size_t len, int prot);

/* Capability system */
bool security_capable(struct process *proc, int capability);
int security_capget(struct process *proc, uint64_t *permitted, uint64_t *effective, uint64_t *inheritable);
int security_capset(struct process *proc, uint64_t permitted, uint64_t effective, uint64_t inheritable);

/* Resource limits and quotas */
int security_set_rlimit(struct process *proc, int resource, const struct rlimit *rlim);
int security_get_rlimit(struct process *proc, int resource, struct rlimit *rlim);

/* Configuration and tuning */
int security_set_config(const struct security_subsystem *config);
int security_get_config(struct security_subsystem *config);
void security_enable_debug(bool enable);
void security_enable_verbose_logging(bool enable);

/* Statistics and monitoring */
void security_print_statistics(void);
void security_print_policy_info(void);
void security_print_context_cache(void);
void security_print_sandbox_info(void);
int security_get_performance_stats(void *stats, size_t size);

/* Utility functions */
const char *security_class_name(security_class_t class);
const char *security_permission_name(uint32_t perm);
const char *audit_event_type_name(audit_event_type_t type);
const char *tpm_algorithm_name(tmp_algorithm_t algorithm);
uint32_t security_context_hash(const struct security_context *ctx);
bool security_context_equal(const struct security_context *ctx1, const struct security_context *ctx2);

/* Advanced features */
int security_enable_mandatory_access_control(bool enable);
int security_set_default_context(const struct security_context *ctx);
int security_transition_context(struct process *proc, const struct security_context *new_ctx);
int security_label_filesystem(const char *mountpoint, const struct security_context *ctx);
int security_relabel_object(const char *path, const struct security_context *new_ctx);

/* Hot-plug and dynamic policy updates */
void security_policy_reload_notify(void);
int security_add_dynamic_rule(const struct security_rule *rule, uint32_t duration_ms);
void security_cleanup_expired_rules(void);