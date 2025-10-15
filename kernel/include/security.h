/**
 * Comprehensive Security Framework for LimitlessOS
 * Advanced security system with encryption, access control, and hardening
 */

#pragma once
#include "kernel.h"
#include "auth.h"
#include "process.h"

/* Security policy types */
typedef enum {
    SECURITY_POLICY_DAC = 0,    /* Discretionary Access Control */
    SECURITY_POLICY_MAC,        /* Mandatory Access Control */
    SECURITY_POLICY_RBAC,       /* Role-Based Access Control */
    SECURITY_POLICY_ABAC        /* Attribute-Based Access Control */
} security_policy_type_t;

/* Security levels (classification levels) */
typedef enum {
    SECURITY_LEVEL_UNCLASSIFIED = 0,
    SECURITY_LEVEL_CONFIDENTIAL,
    SECURITY_LEVEL_SECRET,
    SECURITY_LEVEL_TOP_SECRET
} security_level_t;

/* Encryption algorithms */
typedef enum {
    CRYPTO_ALG_AES_128 = 0,
    CRYPTO_ALG_AES_192,
    CRYPTO_ALG_AES_256,
    CRYPTO_ALG_CHACHA20,
    CRYPTO_ALG_RSA_2048,
    CRYPTO_ALG_RSA_3072,
    CRYPTO_ALG_RSA_4096,
    CRYPTO_ALG_ECDSA_P256,
    CRYPTO_ALG_ECDSA_P384,
    CRYPTO_ALG_ED25519
} crypto_algorithm_t;

/* Hash algorithms */
typedef enum {
    HASH_ALG_SHA256 = 0,
    HASH_ALG_SHA384,
    HASH_ALG_SHA512,
    HASH_ALG_BLAKE2B,
    HASH_ALG_ARGON2ID
} hash_algorithm_t;

/* Security context for MAC */
typedef struct security_label {
    security_level_t level;
    char type[32];          /* Security type (e.g., "process", "file", "network") */
    char category[64];      /* Security category */
    char compartment[64];   /* Security compartment */
    uint32_t attributes;    /* Additional security attributes */
} security_label_t;

/* Access control entry */
typedef struct access_control_entry {
    uint32_t principal_id;  /* User, group, or role ID */
    uint32_t resource_id;   /* Resource identifier */
    uint32_t permissions;   /* Permission bitmask */
    uint64_t expiry_time;   /* Expiration time (0 = never) */
    security_label_t label; /* Security label */
} access_control_entry_t;

/* Cryptographic key */
typedef struct crypto_key {
    uint32_t key_id;
    crypto_algorithm_t algorithm;
    uint8_t* key_data;
    size_t key_size;
    uint64_t created_time;
    uint64_t expiry_time;
    uint32_t usage_flags;   /* Key usage permissions */
    security_label_t label;
} crypto_key_t;

/* Security audit event */
typedef struct security_audit_event {
    uint64_t timestamp;
    pid_t pid;
    uid_t uid;
    uint32_t event_type;
    char description[256];
    char object_name[128];
    uint32_t result;        /* Success/failure */
    security_label_t subject_label;
    security_label_t object_label;
} security_audit_event_t;

/* Secure boot state */
typedef struct secure_boot_state {
    bool enabled;
    bool verified;
    uint8_t platform_key_hash[32];
    uint8_t kernel_hash[32];
    uint8_t initrd_hash[32];
    char boot_policy[64];
    uint64_t boot_time;
} secure_boot_state_t;

/* Security configuration */
typedef struct security_config {
    security_policy_type_t policy_type;
    bool enforce_mac;       /* Enforce mandatory access control */
    bool audit_enabled;     /* Enable security auditing */
    bool crypto_required;   /* Require encryption for sensitive data */
    bool secure_boot;       /* Require secure boot verification */
    
    /* Password policy */
    uint32_t min_password_length;
    bool require_uppercase;
    bool require_lowercase;
    bool require_numbers;
    bool require_symbols;
    uint32_t password_expiry_days;
    
    /* Session management */
    uint32_t session_timeout_minutes;
    uint32_t max_failed_logins;
    uint32_t lockout_duration_minutes;
    
    /* Encryption defaults */
    crypto_algorithm_t default_symmetric_alg;
    crypto_algorithm_t default_asymmetric_alg;
    hash_algorithm_t default_hash_alg;
} security_config_t;

#define MAX_SECURITY_KEYS 1024
#define MAX_ACCESS_ENTRIES 8192
#define MAX_AUDIT_EVENTS 16384

/* Security subsystem state */
typedef struct security_system {
    security_config_t config;
    secure_boot_state_t boot_state;
    
    crypto_key_t keys[MAX_SECURITY_KEYS];
    access_control_entry_t acl[MAX_ACCESS_ENTRIES];
    security_audit_event_t audit_log[MAX_AUDIT_EVENTS];
    
    uint32_t next_key_id;
    uint32_t acl_count;
    uint32_t audit_count;
    
    spinlock_t keys_lock;
    spinlock_t acl_lock;
    spinlock_t audit_lock;
    
    bool initialized;
} security_system_t;

/* Security Framework API */

/* Initialization */
status_t security_init(void);
status_t security_shutdown(void);
status_t security_configure(const security_config_t* config);
status_t security_get_config(security_config_t* config);

/* Access Control */
status_t security_check_access(uid_t uid, uint32_t resource_id, uint32_t required_permissions);
status_t security_grant_access(uint32_t principal_id, uint32_t resource_id, uint32_t permissions, uint64_t expiry);
status_t security_revoke_access(uint32_t principal_id, uint32_t resource_id);
status_t security_enumerate_permissions(uid_t uid, uint32_t* resource_ids, uint32_t* permissions, uint32_t* count);

/* Security Labels (MAC) */
status_t security_set_process_label(process_t* proc, const security_label_t* label);
status_t security_get_process_label(process_t* proc, security_label_t* label);
status_t security_check_mac_access(const security_label_t* subject, const security_label_t* object, uint32_t access_type);
status_t security_dominate_label(const security_label_t* high, const security_label_t* low, bool* dominates);

/* Cryptography */
status_t crypto_generate_key(crypto_algorithm_t algorithm, uint32_t* key_id);
status_t crypto_import_key(crypto_algorithm_t algorithm, const uint8_t* key_data, size_t key_size, uint32_t* key_id);
status_t crypto_export_key(uint32_t key_id, uint8_t* key_data, size_t* key_size);
status_t crypto_delete_key(uint32_t key_id);

status_t crypto_encrypt(uint32_t key_id, const uint8_t* plaintext, size_t plaintext_size, 
                       uint8_t* ciphertext, size_t* ciphertext_size);
status_t crypto_decrypt(uint32_t key_id, const uint8_t* ciphertext, size_t ciphertext_size,
                       uint8_t* plaintext, size_t* plaintext_size);

status_t crypto_sign(uint32_t key_id, const uint8_t* data, size_t data_size,
                    uint8_t* signature, size_t* signature_size);
status_t crypto_verify(uint32_t key_id, const uint8_t* data, size_t data_size,
                      const uint8_t* signature, size_t signature_size, bool* valid);

status_t crypto_hash(hash_algorithm_t algorithm, const uint8_t* data, size_t data_size,
                    uint8_t* hash, size_t* hash_size);

/* Secure Boot */
status_t secure_boot_init(void);
status_t secure_boot_verify_kernel(const uint8_t* kernel_image, size_t image_size, bool* verified);
status_t secure_boot_verify_module(const uint8_t* module_image, size_t image_size, bool* verified);
status_t secure_boot_get_state(secure_boot_state_t* state);

/* Security Auditing */
status_t security_audit_event(uint32_t event_type, pid_t pid, uid_t uid, const char* description, 
                             const char* object_name, uint32_t result);
status_t security_get_audit_log(security_audit_event_t* events, uint32_t* count);
status_t security_clear_audit_log(void);

/* Security Hardening */
status_t security_enable_aslr(process_t* proc);           /* Address Space Layout Randomization */
status_t security_enable_dep(process_t* proc);            /* Data Execution Prevention */
status_t security_enable_stack_canaries(process_t* proc); /* Stack overflow protection */
status_t security_enable_cfi(process_t* proc);            /* Control Flow Integrity */

/* Network Security */
status_t security_create_tls_context(uint32_t* context_id);
status_t security_tls_handshake(uint32_t context_id, const uint8_t* peer_cert, size_t cert_size);
status_t security_tls_encrypt(uint32_t context_id, const uint8_t* plaintext, size_t plaintext_size,
                             uint8_t* ciphertext, size_t* ciphertext_size);
status_t security_tls_decrypt(uint32_t context_id, const uint8_t* ciphertext, size_t ciphertext_size,
                             uint8_t* plaintext, size_t* plaintext_size);

/* Security Policy Management */
status_t security_load_policy(const char* policy_file);
status_t security_save_policy(const char* policy_file);
status_t security_validate_policy(const char* policy_data);

/* Role-Based Access Control */
status_t security_create_role(const char* role_name, uint32_t* role_id);
status_t security_delete_role(uint32_t role_id);
status_t security_assign_role(uid_t uid, uint32_t role_id);
status_t security_revoke_role(uid_t uid, uint32_t role_id);
status_t security_check_role_permission(uint32_t role_id, const char* permission);

/* Intrusion Detection */
typedef void (*security_alert_callback_t)(uint32_t alert_type, const char* description);

status_t security_register_alert_callback(security_alert_callback_t callback);
status_t security_detect_anomaly(pid_t pid, const char* anomaly_description);
status_t security_report_attack(const char* attack_type, const char* source);

/* Security Metrics and Monitoring */
typedef struct security_metrics {
    uint64_t login_attempts;
    uint64_t failed_logins;
    uint64_t privilege_escalations;
    uint64_t access_denials;
    uint64_t crypto_operations;
    uint64_t security_violations;
    uint64_t audit_events_generated;
} security_metrics_t;

status_t security_get_metrics(security_metrics_t* metrics);
status_t security_reset_metrics(void);