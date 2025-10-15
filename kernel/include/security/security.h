/**
 * Security Framework Header for LimitlessOS
 * 
 * Comprehensive security implementation including user/group system,
 * MAC (Mandatory Access Control) framework, ASLR, DEP, code signing,
 * process sandboxing, and cryptographic subsystems.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#ifndef __SECURITY_H__
#define __SECURITY_H__

#include <stdint.h>
#include <stdbool.h>
#include "smp.h"

/* User and Group Management */
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef uint64_t capabilities_t;

#define UID_ROOT        0
#define GID_ROOT        0
#define UID_NOBODY      65534
#define GID_NOBODY      65534

/* Process Capabilities (Linux-compatible) */
#define CAP_CHOWN                0   /* Change file ownership */
#define CAP_DAC_OVERRIDE         1   /* Override DAC access */
#define CAP_DAC_READ_SEARCH      2   /* Override DAC for read/search */
#define CAP_FOWNER              3   /* File owner privileges */
#define CAP_FSETID              4   /* Set file capabilities */
#define CAP_KILL                5   /* Send signals to any process */
#define CAP_SETGID              6   /* Change group IDs */
#define CAP_SETUID              7   /* Change user IDs */
#define CAP_SETPCAP             8   /* Transfer capabilities */
#define CAP_LINUX_IMMUTABLE     9   /* Set immutable/append flags */
#define CAP_NET_BIND_SERVICE    10  /* Bind to privileged ports */
#define CAP_NET_BROADCAST       11  /* Network broadcast */
#define CAP_NET_ADMIN           12  /* Network administration */
#define CAP_NET_RAW             13  /* Raw network access */
#define CAP_IPC_LOCK            14  /* Lock memory */
#define CAP_IPC_OWNER           15  /* IPC ownership */
#define CAP_SYS_MODULE          16  /* Load/unload modules */
#define CAP_SYS_RAWIO           17  /* Raw I/O access */
#define CAP_SYS_CHROOT          18  /* Change root directory */
#define CAP_SYS_PTRACE          19  /* Process tracing */
#define CAP_SYS_PACCT           20  /* Process accounting */
#define CAP_SYS_ADMIN           21  /* System administration */
#define CAP_SYS_BOOT            22  /* System reboot */
#define CAP_SYS_NICE            23  /* Process priority */
#define CAP_SYS_RESOURCE        24  /* Resource limits */
#define CAP_SYS_TIME            25  /* System time */
#define CAP_SYS_TTY_CONFIG      26  /* TTY configuration */
#define CAP_MKNOD               27  /* Create device nodes */
#define CAP_LEASE               28  /* File leases */
#define CAP_AUDIT_WRITE         29  /* Audit log writes */
#define CAP_AUDIT_CONTROL       30  /* Audit configuration */
#define CAP_SETFCAP             31  /* Set file capabilities */

#define CAP_LAST_CAP            CAP_SETFCAP
#define CAP_ALL_BITS            ((1ULL << (CAP_LAST_CAP + 1)) - 1)

/* User credentials structure */
typedef struct user_cred {
    uid_t uid;                      /* Real user ID */
    gid_t gid;                      /* Real group ID */
    uid_t euid;                     /* Effective user ID */
    gid_t egid;                     /* Effective group ID */
    uid_t suid;                     /* Saved user ID */
    gid_t sgid;                     /* Saved group ID */
    uid_t fsuid;                    /* Filesystem user ID */
    gid_t fsgid;                    /* Filesystem group ID */
    
    uint32_t ngroups;               /* Number of supplementary groups */
    gid_t *groups;                  /* Supplementary group list */
    
    capabilities_t cap_effective;   /* Effective capabilities */
    capabilities_t cap_permitted;   /* Permitted capabilities */
    capabilities_t cap_inheritable; /* Inheritable capabilities */
    capabilities_t cap_bset;        /* Capability bounding set */
    
    atomic_t usage;                 /* Reference count */
    spinlock_t lock;                /* Credential lock */
} user_cred_t;

/* Security Context for MAC */
typedef struct security_context {
    char *user;                     /* SELinux/MAC user */
    char *role;                     /* SELinux role */
    char *type;                     /* SELinux type */
    char *range;                    /* MLS/MCS range */
    uint32_t sid;                   /* Security identifier */
} security_context_t;

/* Process Security Information */
typedef struct process_security {
    user_cred_t *cred;              /* Process credentials */
    security_context_t *context;    /* MAC security context */
    
    /* Sandbox information */
    uint32_t sandbox_flags;         /* Sandboxing restrictions */
    char *sandbox_profile;          /* Sandbox profile name */
    
    /* Code signing */
    bool code_signed;               /* Process is code signed */
    char *signature_id;             /* Code signature identifier */
    
    /* Execution control */
    bool no_new_privs;              /* No new privileges flag */
    bool secure_exec;               /* Secure execution mode */
    
    atomic_t usage;                 /* Reference count */
} process_security_t;

/* Memory Protection */
typedef struct memory_region {
    uintptr_t start;                /* Region start address */
    uintptr_t end;                  /* Region end address */
    uint32_t prot;                  /* Protection flags */
    uint32_t flags;                 /* Memory flags */
    
    /* ASLR information */
    bool aslr_enabled;              /* ASLR enabled for region */
    uintptr_t random_offset;        /* Random offset applied */
    
    /* Stack protection */
    bool stack_canary;              /* Stack canary protection */
    bool nx_enabled;                /* NX/DEP enabled */
    
    struct memory_region *next;     /* Next region */
} memory_region_t;

/* ASLR Configuration */
typedef struct aslr_config {
    bool enabled;                   /* ASLR globally enabled */
    uint32_t entropy_bits;          /* Entropy bits for randomization */
    
    /* Per-region ASLR settings */
    bool randomize_stack;           /* Randomize stack location */
    bool randomize_heap;            /* Randomize heap location */
    bool randomize_mmap;            /* Randomize mmap base */
    bool randomize_exec;            /* Randomize executable base */
    bool randomize_brk;             /* Randomize brk base */
    
    /* Entropy sources */
    uint64_t random_seed;           /* Random seed */
    uint32_t entropy_pool[32];      /* Entropy pool */
    spinlock_t entropy_lock;        /* Entropy lock */
} aslr_config_t;

/* Code Signing */
typedef struct code_signature {
    uint8_t hash[32];               /* SHA-256 hash */
    uint32_t hash_type;             /* Hash algorithm */
    
    uint8_t *signature;             /* Digital signature */
    size_t signature_len;           /* Signature length */
    
    char *issuer;                   /* Certificate issuer */
    char *subject;                  /* Certificate subject */
    uint64_t not_before;            /* Valid from timestamp */
    uint64_t not_after;             /* Valid until timestamp */
    
    uint32_t flags;                 /* Signature flags */
    bool verified;                  /* Signature verified */
} code_signature_t;

/* Cryptographic Key */
typedef struct crypto_key {
    uint32_t algorithm;             /* Algorithm identifier */
    uint32_t key_size;              /* Key size in bits */
    uint8_t *key_data;              /* Key material */
    uint32_t flags;                 /* Key flags */
    
    char *name;                     /* Key name */
    uid_t owner;                    /* Key owner */
    uint32_t permissions;           /* Access permissions */
    
    uint64_t created;               /* Creation timestamp */
    uint64_t expires;               /* Expiration timestamp */
    
    atomic_t usage;                 /* Reference count */
} crypto_key_t;

/* Cryptographic Context */
typedef struct crypto_context {
    uint32_t algorithm;             /* Crypto algorithm */
    crypto_key_t *key;              /* Associated key */
    
    void *state;                    /* Algorithm state */
    size_t state_size;              /* State size */
    
    uint32_t flags;                 /* Context flags */
    bool initialized;               /* Context initialized */
} crypto_context_t;

/* Audit Event */
typedef struct audit_event {
    uint32_t type;                  /* Event type */
    uint64_t timestamp;             /* Event timestamp */
    uid_t uid;                      /* User ID */
    gid_t gid;                      /* Group ID */
    pid_t pid;                      /* Process ID */
    
    char *message;                  /* Audit message */
    char *object;                   /* Object name */
    char *subject;                  /* Subject name */
    
    uint32_t result;                /* Operation result */
    uint32_t flags;                 /* Event flags */
    
    struct audit_event *next;       /* Next event in queue */
} audit_event_t;

/* Security Policy */
typedef struct security_policy {
    char *name;                     /* Policy name */
    uint32_t version;               /* Policy version */
    
    /* MAC policy rules */
    void *mac_rules;                /* MAC rule database */
    size_t mac_rules_size;          /* Rule database size */
    
    /* Capability restrictions */
    capabilities_t default_caps;    /* Default capabilities */
    capabilities_t restricted_caps; /* Restricted capabilities */
    
    /* Code signing policy */
    bool require_signatures;        /* Require code signatures */
    bool enforce_signatures;        /* Enforce signature validation */
    
    /* Sandbox policy */
    bool default_sandbox;           /* Default sandboxing */
    char *sandbox_profile;          /* Default sandbox profile */
    
    uint32_t flags;                 /* Policy flags */
    bool active;                    /* Policy active */
} security_policy_t;

/* Sandbox Profile */
typedef struct sandbox_profile {
    char *name;                     /* Profile name */
    uint32_t version;               /* Profile version */
    
    /* File system access */
    char **allowed_paths;           /* Allowed file paths */
    char **denied_paths;            /* Denied file paths */
    uint32_t fs_flags;              /* File system flags */
    
    /* Network access */
    bool network_client;            /* Allow outbound connections */
    bool network_server;            /* Allow inbound connections */
    uint16_t *allowed_ports;        /* Allowed port list */
    
    /* System calls */
    bool *allowed_syscalls;         /* Allowed syscall bitmap */
    bool default_allow;             /* Default allow/deny */
    
    /* Resource limits */
    uint64_t max_memory;            /* Maximum memory usage */
    uint32_t max_files;             /* Maximum open files */
    uint32_t max_threads;           /* Maximum threads */
    
    uint32_t flags;                 /* Profile flags */
} sandbox_profile_t;

/* Security Hooks (LSM-style) */
typedef struct security_operations {
    /* Process operations */
    int (*process_create)(process_security_t *parent, process_security_t *child);
    int (*process_exec)(process_security_t *security, const char *path);
    void (*process_free)(process_security_t *security);
    
    /* File operations */
    int (*file_permission)(const char *path, int mask);
    int (*file_open)(const char *path, int flags);
    int (*file_mmap)(memory_region_t *region, int prot);
    
    /* Network operations */
    int (*socket_create)(int family, int type, int protocol);
    int (*socket_connect)(int sockfd, const struct sockaddr *addr);
    int (*socket_bind)(int sockfd, const struct sockaddr *addr);
    
    /* IPC operations */
    int (*ipc_permission)(int ipcid, int flag);
    int (*msg_queue_associate)(int msqid, int flag);
    int (*shm_associate)(int shmid, int flag);
    
    /* System operations */
    int (*capable)(capabilities_t cap);
    int (*syslog)(int type);
    int (*module_request)(const char *name);
} security_operations_t;

/* Global Security State */
typedef struct security_state {
    /* Current security policy */
    security_policy_t *current_policy;
    
    /* User management */
    user_cred_t **user_table;       /* User credential table */
    uint32_t max_users;             /* Maximum users */
    spinlock_t user_lock;           /* User table lock */
    
    /* ASLR configuration */
    aslr_config_t aslr;             /* ASLR settings */
    
    /* Code signing */
    crypto_key_t **signing_keys;    /* Code signing keys */
    uint32_t num_keys;              /* Number of keys */
    
    /* Audit subsystem */
    audit_event_t *audit_queue;     /* Audit event queue */
    spinlock_t audit_lock;          /* Audit lock */
    bool audit_enabled;             /* Audit enabled */
    
    /* Sandbox profiles */
    sandbox_profile_t **profiles;   /* Sandbox profiles */
    uint32_t num_profiles;          /* Number of profiles */
    
    /* Security hooks */
    security_operations_t *ops;     /* Security operations */
    
    /* Statistics */
    atomic_long_t sec_violations;   /* Security violations */
    atomic_long_t audit_events;     /* Total audit events */
    atomic_long_t cap_checks;       /* Capability checks */
} security_state_t;

/* Crypto Algorithm Identifiers */
#define CRYPTO_ALG_AES          1
#define CRYPTO_ALG_RSA          2
#define CRYPTO_ALG_SHA256       3
#define CRYPTO_ALG_SHA512       4
#define CRYPTO_ALG_HMAC_SHA256  5
#define CRYPTO_ALG_ECDSA        6
#define CRYPTO_ALG_CHACHA20     7
#define CRYPTO_ALG_POLY1305     8

/* Memory Protection Flags */
#define PROT_NONE               0x0
#define PROT_READ               0x1
#define PROT_WRITE              0x2
#define PROT_EXEC               0x4
#define PROT_GROWSDOWN          0x01000000
#define PROT_GROWSUP            0x02000000

/* Sandbox Flags */
#define SANDBOX_NETWORK         0x1     /* Network access allowed */
#define SANDBOX_FILESYSTEM      0x2     /* File system access */
#define SANDBOX_IPC             0x4     /* IPC access */
#define SANDBOX_PTRACE          0x8     /* Debugging access */
#define SANDBOX_STRICT          0x80000000  /* Strict enforcement */

/* Audit Event Types */
#define AUDIT_SYSCALL           1000    /* System call */
#define AUDIT_FS_WATCH          1001    /* File system watch */
#define AUDIT_PATH              1002    /* File path */
#define AUDIT_IPC               1003    /* IPC event */
#define AUDIT_SOCKETCALL        1004    /* Socket operation */
#define AUDIT_CONFIG_CHANGE     1005    /* Configuration change */
#define AUDIT_LOGIN             1006    /* User login */
#define AUDIT_USER_AUTH         1007    /* User authentication */
#define AUDIT_CRYPTO_KEY_USER   1008    /* Crypto key usage */

/* Function Prototypes */

/* Core Security */
int security_init(void);
void security_shutdown(void);

/* User/Group Management */
user_cred_t *cred_alloc(void);
void cred_free(user_cred_t *cred);
user_cred_t *prepare_creds(void);
int commit_creds(user_cred_t *new_cred);
bool capable(capabilities_t cap);
bool capable_wrt_inode_uidgid(const struct inode *inode, capabilities_t cap);

/* Process Security */
process_security_t *security_alloc_process(void);
void security_free_process(process_security_t *security);
int security_process_exec(process_security_t *security, const char *path);
int security_check_permission(const char *path, int mask);

/* Memory Protection */
void aslr_init(void);
uintptr_t aslr_randomize_address(uintptr_t base, size_t size);
int memory_protect_region(memory_region_t *region, int prot);
bool check_stack_canary(uintptr_t *canary_ptr);

/* Code Signing */
int code_sign_init(void);
code_signature_t *verify_code_signature(const void *code, size_t size);
int load_signing_certificate(const char *cert_path);
bool is_code_signature_valid(code_signature_t *sig);

/* Cryptography */
int crypto_init(void);
crypto_context_t *crypto_alloc_context(uint32_t algorithm);
void crypto_free_context(crypto_context_t *ctx);
int crypto_set_key(crypto_context_t *ctx, const uint8_t *key, size_t key_len);
int crypto_encrypt(crypto_context_t *ctx, const uint8_t *plaintext, 
                  size_t plen, uint8_t *ciphertext, size_t *clen);
int crypto_decrypt(crypto_context_t *ctx, const uint8_t *ciphertext, 
                  size_t clen, uint8_t *plaintext, size_t *plen);
int crypto_hash(uint32_t algorithm, const uint8_t *data, size_t len, 
               uint8_t *hash, size_t hash_len);

/* Audit Subsystem */
int audit_init(void);
void audit_log_event(uint32_t type, const char *message);
void audit_log_syscall(int syscall_num, int result);
void audit_log_file_access(const char *path, int mask, int result);

/* Sandbox */
int sandbox_init(void);
sandbox_profile_t *sandbox_load_profile(const char *name);
int sandbox_apply_profile(process_security_t *security, sandbox_profile_t *profile);
bool sandbox_check_syscall(process_security_t *security, int syscall_num);
bool sandbox_check_path_access(process_security_t *security, const char *path, int mask);

/* MAC Framework */
int mac_init(void);
int mac_check_file_permission(security_context_t *subject, 
                             security_context_t *object, int mask);
int mac_check_process_transition(security_context_t *old_ctx, 
                                security_context_t *new_ctx);
security_context_t *mac_get_context(const char *path);
int mac_set_context(const char *path, security_context_t *context);

/* System Calls */
long sys_setuid(uid_t uid);
long sys_setgid(gid_t gid);
long sys_getuid(void);
long sys_getgid(void);
long sys_geteuid(void);
long sys_getegid(void);
long sys_setgroups(size_t size, const gid_t *list);
long sys_getgroups(size_t size, gid_t *list);
long sys_capget(cap_user_header_t header, cap_user_data_t data);
long sys_capset(cap_user_header_t header, const cap_user_data_t data);

/* Security Statistics */
void security_show_stats(void);

/* Utility Functions */
bool security_check_capability(capabilities_t cap);
int security_validate_path(const char *path);
void security_log_violation(const char *type, const char *details);

#endif /* __SECURITY_H__ */