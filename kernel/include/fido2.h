#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "kernel.h"

#define FIDO2_MAX_CREDENTIALS 256
#define FIDO2_MAX_DEVICES 16
#define FIDO2_CREDENTIAL_ID_SIZE 64
#define FIDO2_CHALLENGE_SIZE 32
#define FIDO2_RP_ID_SIZE 256
#define FIDO2_USER_ID_SIZE 64
#define FIDO2_SIGNATURE_SIZE 128

/* FIDO2 algorithm identifiers */
#define FIDO2_ALG_ES256 -7    /* ECDSA w/ SHA-256 */
#define FIDO2_ALG_RS256 -257  /* RSASSA-PKCS1-v1_5 w/ SHA-256 */
#define FIDO2_ALG_EDDSA -8    /* EdDSA */

/* Authenticator transport methods */
typedef enum {
    FIDO2_TRANSPORT_USB = 1,
    FIDO2_TRANSPORT_NFC = 2,
    FIDO2_TRANSPORT_BLE = 3,
    FIDO2_TRANSPORT_INTERNAL = 4
} fido2_transport_t;

/* User verification requirements */
typedef enum {
    FIDO2_UV_REQUIRED = 1,
    FIDO2_UV_PREFERRED = 2,
    FIDO2_UV_DISCOURAGED = 3
} fido2_user_verification_t;

/* Authenticator attachment */
typedef enum {
    FIDO2_ATTACH_PLATFORM = 1,
    FIDO2_ATTACH_CROSS_PLATFORM = 2
} fido2_attachment_t;

/* FIDO2 credential */
typedef struct fido2_credential {
    uint8_t id[FIDO2_CREDENTIAL_ID_SIZE];
    size_t id_length;
    char rp_id[FIDO2_RP_ID_SIZE];
    uint8_t user_id[FIDO2_USER_ID_SIZE];
    size_t user_id_length;
    char user_name[256];
    char user_display_name[256];
    
    /* Public key */
    int32_t algorithm;
    uint8_t public_key[65]; /* Uncompressed P-256 point or Ed25519 key */
    size_t public_key_length;
    
    /* Private key (stored securely) */
    uint8_t private_key[32];
    
    /* Metadata */
    uint32_t sign_count;
    uint64_t created_time;
    uint64_t last_used_time;
    bool resident_key;
    bool user_verification_required;
    
    /* Extensions */
    bool hmac_secret_extension;
    uint8_t hmac_secret[32];
} fido2_credential_t;

/* FIDO2 authenticator device */
typedef struct fido2_device {
    uint32_t device_id;
    char name[128];
    fido2_transport_t transport;
    fido2_attachment_t attachment;
    
    /* Capabilities */
    bool supports_resident_keys;
    bool supports_user_verification;
    bool supports_user_presence;
    bool supports_client_pin;
    bool supports_hmac_secret;
    
    /* State */
    bool initialized;
    bool pin_set;
    uint8_t pin_retries;
    uint32_t pin_token;
    
    /* Credentials stored on this device */
    uint32_t credential_count;
    uint32_t max_credentials;
    
    /* Statistics */
    uint64_t registrations;
    uint64_t authentications;
    uint64_t last_activity;
} fido2_device_t;

/* FIDO2 registration request */
typedef struct fido2_make_credential_request {
    char rp_id[FIDO2_RP_ID_SIZE];
    char rp_name[256];
    
    uint8_t user_id[FIDO2_USER_ID_SIZE];
    size_t user_id_length;
    char user_name[256];
    char user_display_name[256];
    
    uint8_t challenge[FIDO2_CHALLENGE_SIZE];
    size_t challenge_length;
    
    int32_t algorithms[8];
    uint8_t algorithm_count;
    
    fido2_user_verification_t user_verification;
    bool resident_key_required;
    
    /* Exclude list */
    uint8_t exclude_credentials[16][FIDO2_CREDENTIAL_ID_SIZE];
    size_t exclude_lengths[16];
    uint8_t exclude_count;
} fido2_make_credential_request_t;

/* FIDO2 authentication request */
typedef struct fido2_get_assertion_request {
    char rp_id[FIDO2_RP_ID_SIZE];
    uint8_t challenge[FIDO2_CHALLENGE_SIZE];
    size_t challenge_length;
    
    fido2_user_verification_t user_verification;
    bool user_presence_required;
    
    /* Allow list */
    uint8_t allow_credentials[16][FIDO2_CREDENTIAL_ID_SIZE];
    size_t allow_lengths[16];
    uint8_t allow_count;
} fido2_get_assertion_request_t;

/* FIDO2 attestation response */
typedef struct fido2_attestation_response {
    uint8_t credential_id[FIDO2_CREDENTIAL_ID_SIZE];
    size_t credential_id_length;
    
    uint8_t public_key[65];
    size_t public_key_length;
    int32_t algorithm;
    
    uint8_t authenticator_data[256];
    size_t authenticator_data_length;
    
    uint8_t attestation_signature[FIDO2_SIGNATURE_SIZE];
    size_t signature_length;
    
    /* Client data JSON hash */
    uint8_t client_data_hash[32];
} fido2_attestation_response_t;

/* FIDO2 assertion response */
typedef struct fido2_assertion_response {
    uint8_t credential_id[FIDO2_CREDENTIAL_ID_SIZE];
    size_t credential_id_length;
    
    uint8_t authenticator_data[256];
    size_t authenticator_data_length;
    
    uint8_t signature[FIDO2_SIGNATURE_SIZE];
    size_t signature_length;
    
    uint8_t client_data_hash[32];
    uint32_t sign_count;
} fido2_assertion_response_t;

/* FIDO2 metrics */
typedef struct fido2_metrics {
    uint64_t devices_registered;
    uint64_t credentials_created;
    uint64_t authentications_performed;
    uint64_t user_verification_attempts;
    uint64_t user_presence_tests;
    uint64_t pin_verifications;
    uint64_t resident_key_operations;
} fido2_metrics_t;

/* Public API */
status_t fido2_init(void);
status_t fido2_register_device(const fido2_device_t* device, uint32_t* out_device_id);
status_t fido2_make_credential(uint32_t device_id, const fido2_make_credential_request_t* request,
                              fido2_attestation_response_t* out_response);
status_t fido2_get_assertion(uint32_t device_id, const fido2_get_assertion_request_t* request,
                            fido2_assertion_response_t* out_response);
status_t fido2_verify_attestation(const fido2_attestation_response_t* response, bool* out_valid);
status_t fido2_verify_assertion(const fido2_assertion_response_t* response, 
                               const uint8_t* public_key, size_t key_length, bool* out_valid);
status_t fido2_set_pin(uint32_t device_id, const char* pin);
status_t fido2_verify_pin(uint32_t device_id, const char* pin);
status_t fido2_get_device_info(uint32_t device_id, fido2_device_t* out_device);
status_t fido2_enumerate_credentials(uint32_t device_id, fido2_credential_t* out_credentials, uint8_t* in_out_count);
status_t fido2_delete_credential(uint32_t device_id, const uint8_t* credential_id, size_t id_length);
status_t fido2_get_metrics(fido2_metrics_t* out);

/* Cryptographic operations */
status_t fido2_generate_keypair(int32_t algorithm, uint8_t* out_public_key, uint8_t* out_private_key);
status_t fido2_sign_data(int32_t algorithm, const uint8_t* private_key, const uint8_t* data, size_t data_length,
                        uint8_t* out_signature, size_t* in_out_signature_length);
status_t fido2_verify_signature(int32_t algorithm, const uint8_t* public_key, const uint8_t* data, size_t data_length,
                               const uint8_t* signature, size_t signature_length, bool* out_valid);