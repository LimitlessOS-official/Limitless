/*
 * secure_boot.h - LimitlessOS Advanced Secure Boot Header
 * 
 * Comprehensive secure boot system with Ed25519 signatures, TPM integration,
 * rollback protection, and OTA updates.
 */

#ifndef _KERNEL_SECURE_BOOT_H
#define _KERNEL_SECURE_BOOT_H

#include <kernel/types.h>
#include <kernel/status.h>
#include <kernel/spinlock.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SECURE_BOOT_VERSION_MAJOR       1
#define SECURE_BOOT_VERSION_MINOR       0
#define SECURE_BOOT_VERSION_PATCH       0

#define MAX_BOOT_CERTIFICATES           16
#define MAX_BOOT_COMPONENTS             32
#define MAX_ATTESTATION_COMPONENTS      16
#define SECURE_BOOT_HASH_SIZE           32
#define SECURE_BOOT_SIGNATURE_SIZE      64
#define SECURE_BOOT_KEY_SIZE            32
#define SECURE_BOOT_KEY_ID_SIZE         16

/* Secure boot component information */
typedef struct secure_boot_component {
    char name[64];                              /* Component name */
    uint64_t version;                           /* Component version */
    uint32_t stage;                             /* Boot stage */
    uint8_t hash[SECURE_BOOT_HASH_SIZE];        /* Component hash */
    bool verified;                              /* Signature verified */
    uint64_t verification_time;                 /* When verified */
} secure_boot_component_t;

/* Boot measurements structure */
typedef struct secure_boot_measurements {
    uint8_t bootloader_hash[32];
    uint8_t kernel_hash[32];
    uint8_t initrd_hash[32];
    uint8_t config_hash[32];
    bool measurements_valid;
    bool secure_boot_enabled;
    bool verification_passed;
} secure_boot_measurements_t;

/* Boot verification chain */
typedef struct secure_boot_chain {
    bool enabled;
    bool verification_passed;
    uint32_t component_count;
    secure_boot_component_t components[MAX_BOOT_COMPONENTS];
} secure_boot_chain_t;

/* Boot policy configuration */
typedef struct secure_boot_policy {
    bool require_signed_kernel;
    bool require_signed_modules;
    bool allow_unsigned_debug;
    bool enforce_module_whitelist;
    char trusted_root_ca[128];
} secure_boot_policy_t;

/* Advanced secure boot status */
typedef struct secure_boot_status {
    bool enabled;                               /* Secure boot enabled */
    bool enforced;                              /* Verification enforced */
    bool tpm_available;                         /* TPM 2.0 available */
    uint32_t public_key_count;                  /* Number of trusted keys */
    uint32_t boot_component_count;              /* Components verified */
    uint32_t rollback_entry_count;              /* Rollback entries */
    
    /* PCR measurements */
    uint8_t pcr_values[8][SECURE_BOOT_HASH_SIZE];
    
    /* Statistics */
    uint32_t successful_boots;
    uint32_t failed_verifications;
    uint32_t rollback_violations;
    uint32_t key_rotations;
    uint32_t tmp_operations;
    uint64_t last_boot_time;
} secure_boot_status_t;

/* TPM attestation data */
typedef struct secure_boot_attestation {
    bool secure_boot_enabled;                   /* Secure boot status */
    bool tmp_available;                         /* TPM availability */
    bool quote_valid;                           /* Quote validity */
    uint64_t timestamp;                         /* Quote timestamp */
    
    /* Attestation data */
    uint8_t nonce[32];                          /* Random nonce */
    uint8_t quote_signature[256];               /* TPM quote signature */
    
    /* PCR values at time of quote */
    uint8_t pcr_values[8][SECURE_BOOT_HASH_SIZE];
    bool pcr_extended[8];
    
    /* Boot component information */
    uint32_t component_count;
    secure_boot_component_t components[MAX_ATTESTATION_COMPONENTS];
} secure_boot_attestation_t;

/* OTA update component */
typedef struct ota_component {
    char name[64];                              /* Component name */
    uint64_t old_version;                       /* Current version */
    uint64_t new_version;                       /* Target version */
    uint64_t offset;                            /* Offset in package */
    uint64_t size;                              /* Component size */
    uint8_t hash[SECURE_BOOT_HASH_SIZE];        /* Component hash */
} ota_component_t;

/* OTA update package */
typedef struct ota_update_package {
    char version[32];                           /* Update version */
    char description[256];                      /* Update description */
    uint64_t timestamp;                         /* Package timestamp */
    uint64_t size;                              /* Total package size */
    uint8_t hash[SECURE_BOOT_HASH_SIZE];        /* Package hash */
    uint8_t signature[SECURE_BOOT_SIGNATURE_SIZE]; /* Package signature */
    
    /* Update components */
    uint32_t component_count;
    ota_component_t components[32];
    
    /* Update flags */
    bool requires_reboot;
    bool rollback_safe;
    uint32_t criticality;                       /* 0=low, 1=medium, 2=high, 3=critical */
} ota_update_package_t;

/* Public key entry */
typedef struct secure_boot_public_key {
    uint8_t key_id[SECURE_BOOT_KEY_ID_SIZE];    /* Key identifier */
    uint8_t public_key[SECURE_BOOT_KEY_SIZE];   /* Ed25519 public key */
    uint64_t valid_from;                        /* Key valid from */
    uint64_t valid_until;                       /* Key valid until */
    uint32_t key_usage;                         /* Key usage flags */
    bool revoked;                               /* Key revoked */
    char description[128];                      /* Key description */
} secure_boot_public_key_t;

/* Key usage flags */
#define SECURE_BOOT_KEY_USAGE_BOOT              (1 << 0)  /* Boot component signing */
#define SECURE_BOOT_KEY_USAGE_UPDATE            (1 << 1)  /* OTA update signing */
#define SECURE_BOOT_KEY_USAGE_MODULE            (1 << 2)  /* Kernel module signing */
#define SECURE_BOOT_KEY_USAGE_ROLLBACK          (1 << 3)  /* Rollback protection */

/* Rollback protection entry */
typedef struct secure_boot_rollback {
    char component_name[64];                    /* Component name */
    uint64_t min_version;                       /* Minimum allowed version */
    uint64_t last_boot_version;                 /* Last booted version */
    uint32_t rollback_count;                    /* Rollback attempt count */
    uint64_t last_update_time;                  /* Last update time */
} secure_boot_rollback_t;

/*
 * Basic Secure Boot Functions (existing system)
 */

/* Initialize secure boot system */
status_t secure_boot_init(void);

/* Enable secure boot verification */
status_t secure_boot_enable(void);

/* Add trusted certificate */
status_t secure_boot_add_certificate(const uint8_t* public_key, uint32_t key_size,
                                    const char* issuer, uint64_t expiry_time);

/* Verify component signature */
status_t secure_boot_verify_component(const char* component_name, const uint8_t* data, uint32_t data_size,
                                     const uint8_t* signature, uint32_t signature_size);

/* Measure boot component */
status_t secure_boot_measure_component(const char* component_name, const uint8_t* data, uint32_t data_size);

/* Validate complete boot chain */
status_t secure_boot_validate_chain(void);

/* Get boot measurements */
status_t secure_boot_get_measurements(secure_boot_measurements_t* measurements);

/* Get verification chain */
status_t secure_boot_get_chain(secure_boot_chain_t* chain);

/* Set/get boot policy */
status_t secure_boot_set_policy(const secure_boot_policy_t* policy);
status_t secure_boot_get_policy(secure_boot_policy_t* policy);

/* Check if secure boot is verified */
bool secure_boot_is_verified(void);

/* Shutdown secure boot system */
status_t secure_boot_shutdown(void);

/*
 * Advanced Secure Boot Functions (new enterprise features)
 */

/* Initialize advanced secure boot system */
status_t secure_boot_advanced_init(void);

/* Verify component with advanced features (Ed25519, rollback protection) */
status_t secure_boot_advanced_verify_component(const char* name, const void* data, size_t size,
                                              const uint8_t* signature, uint64_t version);

/* Public key management */
status_t secure_boot_add_public_key(const uint8_t* key_id, const uint8_t* public_key,
                                   uint64_t valid_from, uint64_t valid_until,
                                   const char* description);

status_t secure_boot_revoke_public_key(const uint8_t* key_id);

status_t secure_boot_list_public_keys(secure_boot_public_key_t* keys, uint32_t max_keys, uint32_t* key_count);

/* Rollback protection */
status_t secure_boot_get_rollback_info(const char* component_name, secure_boot_rollback_t* rollback_info);

status_t secure_boot_update_rollback_version(const char* component_name, uint64_t new_min_version);

/* OTA update system */
status_t secure_boot_verify_ota_package(const char* package_path, ota_update_package_t* package_info);

status_t secure_boot_apply_ota_update(const char* package_path, bool dry_run);

status_t secure_boot_create_ota_package(const char* output_path, const ota_update_package_t* package_info,
                                       const uint8_t* private_key);

/* TPM attestation */
status_t secure_boot_get_attestation_quote(secure_boot_attestation_t* attestation);

status_t secure_boot_verify_attestation_quote(const secure_boot_attestation_t* attestation,
                                             const uint8_t* expected_pcr_values,
                                             const uint8_t* aik_public_key);

/* Boot integrity monitoring */
status_t secure_boot_start_integrity_monitoring(void);

status_t secure_boot_stop_integrity_monitoring(void);

status_t secure_boot_get_integrity_violations(uint32_t* violation_count, char** violation_log);

/* Advanced status and configuration */
void secure_boot_advanced_get_status(secure_boot_status_t* status);

status_t secure_boot_set_enforcement(bool enabled);

status_t secure_boot_export_measurements(const char* output_path);

status_t secure_boot_import_trusted_keys(const char* keystore_path, const uint8_t* master_key);

/* Event notifications */
typedef enum {
    SECURE_BOOT_EVENT_VERIFICATION_SUCCESS,
    SECURE_BOOT_EVENT_VERIFICATION_FAILURE,
    SECURE_BOOT_EVENT_ROLLBACK_VIOLATION,
    SECURE_BOOT_EVENT_KEY_ROTATION,
    SECURE_BOOT_EVENT_OTA_UPDATE_START,
    SECURE_BOOT_EVENT_OTA_UPDATE_COMPLETE,
    SECURE_BOOT_EVENT_TPM_ERROR,
    SECURE_BOOT_EVENT_INTEGRITY_VIOLATION
} secure_boot_event_type_t;

typedef struct secure_boot_event {
    secure_boot_event_type_t type;
    uint64_t timestamp;
    char component_name[64];
    char description[256];
    uint32_t error_code;
} secure_boot_event_t;

typedef void (*secure_boot_event_callback_t)(const secure_boot_event_t* event);

status_t secure_boot_register_event_callback(secure_boot_event_callback_t callback);

status_t secure_boot_unregister_event_callback(secure_boot_event_callback_t callback);

/* Boot completion notification */
void secure_boot_notify_boot_complete(void);
void secure_boot_advanced_notify_boot_complete(void);

/* Utility functions */
status_t secure_boot_calculate_component_hash(const void* data, size_t size, uint8_t* hash);

status_t secure_boot_verify_ed25519_signature(const void* data, size_t data_size,
                                             const uint8_t* signature, const uint8_t* public_key);

status_t secure_boot_generate_key_pair(uint8_t* public_key, uint8_t* private_key);

/* Configuration macros */
#define SECURE_BOOT_CONFIG_ENFORCE_SIGNATURES   (1 << 0)
#define SECURE_BOOT_CONFIG_REQUIRE_TPM          (1 << 1)
#define SECURE_BOOT_CONFIG_ALLOW_DEBUG_BOOT     (1 << 2)
#define SECURE_BOOT_CONFIG_ENABLE_ATTESTATION   (1 << 3)
#define SECURE_BOOT_CONFIG_STRICT_ROLLBACK      (1 << 4)

/* Debug and testing functions (only in debug builds) */
#ifdef CONFIG_SECURE_BOOT_DEBUG
status_t secure_boot_debug_add_test_key(void);
status_t secure_boot_debug_disable_enforcement(void);
status_t secure_boot_debug_simulate_rollback(const char* component_name, uint64_t version);
void secure_boot_debug_dump_state(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_SECURE_BOOT_H */