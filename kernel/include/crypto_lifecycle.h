#ifndef LIMITLESS_CRYPTO_LIFECYCLE_H
#define LIMITLESS_CRYPTO_LIFECYCLE_H

#include "types.h"

/* Maximum keys in the system */
#define CRYPTO_MAX_KEYS 1024
#define CRYPTO_KEY_ID_LEN 32
#define CRYPTO_KEY_DATA_LEN 64
#define CRYPTO_KEY_LABEL_LEN 128

/* Key types */
typedef enum {
    CRYPTO_KEY_TYPE_SYMMETRIC = 1,
    CRYPTO_KEY_TYPE_ASYMMETRIC_PRIVATE = 2,
    CRYPTO_KEY_TYPE_ASYMMETRIC_PUBLIC = 3,
    CRYPTO_KEY_TYPE_HMAC = 4,
    CRYPTO_KEY_TYPE_DERIVE = 5
} crypto_key_type_t;

/* Key algorithms */
typedef enum {
    CRYPTO_ALG_AES128 = 1,
    CRYPTO_ALG_AES256 = 2,
    CRYPTO_ALG_RSA2048 = 3,
    CRYPTO_ALG_RSA4096 = 4,
    CRYPTO_ALG_ECDSA_P256 = 5,
    CRYPTO_ALG_ECDSA_P384 = 6,
    CRYPTO_ALG_ECDH_P256 = 7,
    CRYPTO_ALG_CHACHA20 = 8,
    CRYPTO_ALG_HMAC_SHA256 = 9,
    CRYPTO_ALG_PBKDF2 = 10
} crypto_algorithm_t;

/* Key status */
typedef enum {
    CRYPTO_KEY_STATUS_ACTIVE = 1,
    CRYPTO_KEY_STATUS_EXPIRED = 2,
    CRYPTO_KEY_STATUS_REVOKED = 3,
    CRYPTO_KEY_STATUS_COMPROMISED = 4,
    CRYPTO_KEY_STATUS_PENDING_ROTATION = 5
} crypto_key_status_t;

/* Key usage flags */
typedef enum {
    CRYPTO_USAGE_ENCRYPT = 0x01,
    CRYPTO_USAGE_DECRYPT = 0x02,
    CRYPTO_USAGE_SIGN = 0x04,
    CRYPTO_USAGE_VERIFY = 0x08,
    CRYPTO_USAGE_DERIVE = 0x10,
    CRYPTO_USAGE_WRAP = 0x20,
    CRYPTO_USAGE_UNWRAP = 0x40
} crypto_key_usage_t;

/* Key metadata structure */
typedef struct {
    char key_id[CRYPTO_KEY_ID_LEN];
    char label[CRYPTO_KEY_LABEL_LEN];
    crypto_key_type_t type;
    crypto_algorithm_t algorithm;
    crypto_key_status_t status;
    uint32_t usage_flags;
    
    /* Key material */
    uint8_t key_data[CRYPTO_KEY_DATA_LEN];
    size_t key_length;
    
    /* Lifecycle timestamps */
    uint64_t created_time;
    uint64_t last_used_time;
    uint64_t expires_time;
    uint64_t rotation_time;
    
    /* Usage tracking */
    uint64_t usage_count;
    uint64_t max_usage_count;
    
    /* Security attributes */
    uint32_t owner_uid;
    uint32_t security_level;
    bool exportable;
    bool hardware_backed;
    
    /* Rotation policy */
    uint32_t rotation_interval_days;
    bool auto_rotate;
    char rotation_policy[64];
    
    /* Parent key for derived keys */
    char parent_key_id[CRYPTO_KEY_ID_LEN];
    
    /* Validation */
    uint32_t checksum;
} crypto_key_metadata_t;

/* Key generation request */
typedef struct {
    char label[CRYPTO_KEY_LABEL_LEN];
    crypto_key_type_t type;
    crypto_algorithm_t algorithm;
    uint32_t usage_flags;
    
    /* Lifecycle policy */
    uint32_t expires_in_days;
    uint32_t rotation_interval_days;
    uint64_t max_usage_count;
    bool auto_rotate;
    
    /* Security attributes */
    uint32_t security_level;
    bool exportable;
    bool hardware_backed;
    
    /* Key derivation (if applicable) */
    char parent_key_id[CRYPTO_KEY_ID_LEN];
    uint8_t derivation_data[32];
    size_t derivation_length;
} crypto_key_generation_request_t;

/* Key rotation result */
typedef struct {
    char old_key_id[CRYPTO_KEY_ID_LEN];
    char new_key_id[CRYPTO_KEY_ID_LEN];
    uint64_t rotation_time;
    bool success;
    char error_message[128];
} crypto_key_rotation_result_t;

/* Audit event for key operations */
typedef struct {
    char key_id[CRYPTO_KEY_ID_LEN];
    char operation[32];
    uint32_t user_id;
    uint32_t process_id;
    uint64_t timestamp;
    bool success;
    char details[128];
} crypto_audit_event_t;

/* Key lifecycle statistics */
typedef struct {
    uint32_t total_keys;
    uint32_t active_keys;
    uint32_t expired_keys;
    uint32_t revoked_keys;
    uint32_t keys_due_rotation;
    uint64_t keys_generated_today;
    uint64_t keys_rotated_today;
    uint64_t total_key_operations;
    uint64_t failed_operations;
    float average_key_age_days;
} crypto_lifecycle_stats_t;

/* Function declarations */
status_t crypto_lifecycle_init(void);
status_t crypto_lifecycle_shutdown(void);

/* Key generation and management */
status_t crypto_generate_key(const crypto_key_generation_request_t* request, char* out_key_id);
status_t crypto_import_key(const crypto_key_metadata_t* key_data, char* out_key_id);
status_t crypto_export_key(const char* key_id, crypto_key_metadata_t* out_key_data);
status_t crypto_delete_key(const char* key_id);

/* Key metadata operations */
status_t crypto_get_key_metadata(const char* key_id, crypto_key_metadata_t* out_metadata);
status_t crypto_update_key_metadata(const char* key_id, const crypto_key_metadata_t* metadata);
status_t crypto_list_keys(crypto_key_metadata_t* out_keys, uint32_t* in_out_count);
status_t crypto_find_keys_by_label(const char* label, char out_key_ids[][CRYPTO_KEY_ID_LEN], uint32_t* in_out_count);

/* Key usage and validation */
status_t crypto_use_key(const char* key_id, crypto_key_usage_t usage, const uint8_t** out_key_data, size_t* out_key_length);
status_t crypto_validate_key(const char* key_id, bool* out_valid);
status_t crypto_check_key_usage_limits(const char* key_id, bool* out_within_limits);

/* Key rotation and lifecycle management */
status_t crypto_rotate_key(const char* key_id, crypto_key_rotation_result_t* out_result);
status_t crypto_check_rotation_due(char out_key_ids[][CRYPTO_KEY_ID_LEN], uint32_t* in_out_count);
status_t crypto_auto_rotate_keys(void);
status_t crypto_expire_key(const char* key_id);
status_t crypto_revoke_key(const char* key_id, const char* reason);

/* Key derivation */
status_t crypto_derive_key(const char* parent_key_id, const uint8_t* derivation_data, size_t data_length,
                          const crypto_key_generation_request_t* request, char* out_key_id);

/* Security and access control */
status_t crypto_set_key_owner(const char* key_id, uint32_t owner_uid);
status_t crypto_check_key_access(const char* key_id, uint32_t user_id, crypto_key_usage_t usage, bool* out_allowed);
status_t crypto_backup_key_store(const char* backup_path, const char* encryption_key);
status_t crypto_restore_key_store(const char* backup_path, const char* encryption_key);

/* Auditing and monitoring */
status_t crypto_audit_key_operation(const char* key_id, const char* operation, uint32_t user_id, bool success, const char* details);
status_t crypto_get_key_audit_log(const char* key_id, crypto_audit_event_t* out_events, uint32_t* in_out_count);
status_t crypto_get_lifecycle_statistics(crypto_lifecycle_stats_t* out_stats);
status_t crypto_reset_statistics(void);

/* Key store integrity */
status_t crypto_verify_key_store_integrity(bool* out_valid);
status_t crypto_rebuild_key_store_checksums(void);
status_t crypto_secure_erase_key(const char* key_id);

/* Configuration and policy */
typedef struct {
    uint32_t default_rotation_interval_days;
    uint64_t default_max_usage_count;
    bool enforce_rotation_policy;
    bool audit_all_operations;
    uint32_t key_cache_size;
    uint32_t max_concurrent_operations;
} crypto_lifecycle_config_t;

status_t crypto_set_lifecycle_config(const crypto_lifecycle_config_t* config);
status_t crypto_get_lifecycle_config(crypto_lifecycle_config_t* out_config);

/* Background maintenance */
status_t crypto_start_maintenance_thread(void);
status_t crypto_stop_maintenance_thread(void);
status_t crypto_run_maintenance_cycle(void);

/* Hardware security module integration */
status_t crypto_hsm_generate_key(const crypto_key_generation_request_t* request, char* out_key_id);
status_t crypto_hsm_get_key_handle(const char* key_id, void** out_handle);
status_t crypto_hsm_is_available(bool* out_available);

/* Utility functions */
status_t crypto_generate_key_id(char* out_key_id);
status_t crypto_validate_algorithm_params(crypto_algorithm_t algorithm, size_t key_length, bool* out_valid);
uint32_t crypto_calculate_key_strength(crypto_algorithm_t algorithm, size_t key_length);

#endif /* LIMITLESS_CRYPTO_LIFECYCLE_H */