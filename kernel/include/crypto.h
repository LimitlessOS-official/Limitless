/**
 * LimitlessOS Cryptographic Framework Header
 * Complete SSL/TLS cryptographic support
 */

#pragma once
#include "kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Cryptographic Algorithm Types */
typedef enum {
    CRYPTO_TYPE_SYMMETRIC = 0,  // Symmetric encryption (AES, ChaCha20)
    CRYPTO_TYPE_ASYMMETRIC,     // Asymmetric encryption (RSA, ECDSA)
    CRYPTO_TYPE_HASH,           // Hash functions (SHA-256, SHA-512)
    CRYPTO_TYPE_MAC,            // Message Authentication (HMAC)
    CRYPTO_TYPE_KDF,            // Key Derivation Functions (PBKDF2)
} crypto_algorithm_type_t;

/* Cryptographic Algorithm IDs */
typedef enum {
    /* Symmetric Ciphers */
    CRYPTO_AES128_ECB = 1,
    CRYPTO_AES128_CBC,
    CRYPTO_AES128_CTR,
    CRYPTO_AES128_GCM,
    CRYPTO_AES192_ECB,
    CRYPTO_AES192_CBC, 
    CRYPTO_AES192_CTR,
    CRYPTO_AES192_GCM,
    CRYPTO_AES256_ECB,
    CRYPTO_AES256_CBC,
    CRYPTO_AES256_CTR,
    CRYPTO_AES256_GCM,
    CRYPTO_CHACHA20,
    CRYPTO_CHACHA20_POLY1305,
    
    /* Asymmetric Ciphers */
    CRYPTO_RSA1024,
    CRYPTO_RSA2048,
    CRYPTO_RSA4096,
    CRYPTO_ECDSA_P256,
    CRYPTO_ECDSA_P384,
    CRYPTO_ECDSA_P521,
    CRYPTO_ED25519,
    
    /* Hash Functions */
    CRYPTO_SHA1 = 100,
    CRYPTO_SHA256,
    CRYPTO_SHA384,
    CRYPTO_SHA512,
    CRYPTO_SHA3_256,
    CRYPTO_SHA3_512,
    
    /* MAC Functions */
    CRYPTO_HMAC_SHA1 = 200,
    CRYPTO_HMAC_SHA256,
    CRYPTO_HMAC_SHA384,
    CRYPTO_HMAC_SHA512,
    
    /* Key Derivation */
    CRYPTO_PBKDF2 = 300,
    CRYPTO_HKDF,
    CRYPTO_SCRYPT,
} crypto_algorithm_id_t;

/* Cryptographic Context */
typedef struct crypto_context {
    crypto_algorithm_id_t algorithm;
    const void* algorithm_impl;
    bool initialized;
    
    /* Algorithm-specific state */
    union {
        struct {
            uint32_t rounds;
            uint8_t key[32];
            uint8_t iv[16];
        } aes;
        
        struct {
            uint8_t key[32];
            uint8_t nonce[12];
            uint32_t counter;
        } chacha20;
        
        struct {
            void* public_key;
            void* private_key;
            uint32_t key_size;
        } rsa;
        
        struct {
            uint32_t state[8];
            uint8_t buffer[64];
            uint64_t length;
            uint32_t buffer_len;
        } sha256;
        
        struct {
            uint8_t inner_key[64];
            uint8_t outer_key[64];
            crypto_algorithm_id_t hash_algo;
        } hmac;
    } state;
} crypto_context_t;

/* Algorithm Information */
typedef struct crypto_algorithm_info {
    char name[64];
    crypto_algorithm_type_t type;
    uint32_t key_size;      // Key size in bits
    uint32_t block_size;    // Block size in bytes
} crypto_algorithm_info_t;

/* Random Number Generator State */
typedef struct crypto_random_state {
    uint8_t* entropy_pool;
    uint32_t entropy_pool_size;
    uint32_t pool_index;
    uint64_t reseed_counter;
} crypto_random_state_t;

/* SSL/TLS Cipher Suites */
typedef enum {
    TLS_NULL_WITH_NULL_NULL                = 0x0000,
    TLS_RSA_WITH_AES_128_CBC_SHA           = 0x002F,
    TLS_RSA_WITH_AES_256_CBC_SHA           = 0x0035,
    TLS_RSA_WITH_AES_128_CBC_SHA256        = 0x003C,
    TLS_RSA_WITH_AES_256_CBC_SHA256        = 0x003D,
    TLS_RSA_WITH_AES_128_GCM_SHA256        = 0x009C,
    TLS_RSA_WITH_AES_256_GCM_SHA384        = 0x009D,
    TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256  = 0xC027,
    TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384  = 0xC028,
    TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256  = 0xC02F,
    TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384  = 0xC030,
    TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305   = 0xCCA8,
} tls_cipher_suite_t;

/* SSL/TLS Protocol Versions */
typedef enum {
    TLS_VERSION_1_0 = 0x0301,
    TLS_VERSION_1_1 = 0x0302,
    TLS_VERSION_1_2 = 0x0303,
    TLS_VERSION_1_3 = 0x0304,
} tls_version_t;

/* TLS Constants */
#define TLS_MAX_RECORD_SIZE 16384

/* TLS Session Types */
typedef enum {
    TLS_SESSION_CLIENT = 0,
    TLS_SESSION_SERVER = 1,
} tls_session_type_t;

/* TLS States */
typedef enum {
    TLS_STATE_INIT = 0,
    TLS_STATE_HANDSHAKE,
    TLS_STATE_CONNECTED,
    TLS_STATE_CLOSED,
    TLS_STATE_ERROR,
    TLS_STATE_WAIT_CLIENT_HELLO,
    TLS_STATE_WAIT_SERVER_HELLO,
    TLS_STATE_WAIT_CERTIFICATE,
    TLS_STATE_WAIT_SERVER_HELLO_DONE,
    TLS_STATE_SEND_CLIENT_KEY_EXCHANGE,
} tls_state_t;

/* TLS Certificate */
typedef struct tls_certificate {
    uint8_t* data;
    size_t length;
    char subject[256];
    char issuer[256];
} tls_certificate_t;

/* TLS Session */
typedef struct tls_session {
    uint32_t session_id;
    tls_session_type_t type;
    tls_state_t state;
    tls_version_t version;
    tls_cipher_suite_t cipher_suite;
    
    uint8_t client_random[32];
    uint8_t server_random[32];
    uint8_t master_secret[48];
    
    void* send_buffer;
    void* recv_buffer;
    size_t buffer_size;
    
    int socket_fd;
    tls_certificate_t* certificate;
} tls_session_t;

/* Crypto Algorithm Structure */
typedef struct crypto_algorithm {
    crypto_algorithm_id_t id;
    const char* name;
    crypto_algorithm_type_t type;
    uint32_t key_size;
    uint32_t block_size;
    status_t (*init)(crypto_context_t* ctx, const void* key, uint32_t key_len);
    status_t (*encrypt)(crypto_context_t* ctx, const void* input, void* output, uint32_t length);
    status_t (*decrypt)(crypto_context_t* ctx, const void* input, void* output, uint32_t length);
    status_t (*hash)(const void* input, uint32_t length, void* output);
    void (*cleanup)(crypto_context_t* ctx);
} crypto_algorithm_t;

/* Core Crypto API */
status_t crypto_init(void);
status_t crypto_shutdown(void);

/* Context Management */
status_t crypto_create_context(crypto_algorithm_id_t algorithm, 
                              crypto_context_t** context);
status_t crypto_init_context(crypto_context_t* context, const void* key, 
                           uint32_t key_length);
void crypto_destroy_context(crypto_context_t* context);

/* Encryption/Decryption */
status_t crypto_encrypt(crypto_context_t* context, const void* plaintext, 
                       void* ciphertext, uint32_t length);
status_t crypto_decrypt(crypto_context_t* context, const void* ciphertext, 
                       void* plaintext, uint32_t length);

/* Hashing */
status_t crypto_hash(crypto_algorithm_id_t algorithm, const void* data, 
                    uint32_t length, void* hash_out);

/* Random Number Generation */
status_t crypto_random_bytes(void* buffer, uint32_t length);

/* Algorithm Information */
status_t crypto_get_algorithm_info(crypto_algorithm_id_t algorithm, 
                                  crypto_algorithm_info_t* info);

/* Utility Functions */
bool crypto_secure_compare(const void* a, const void* b, uint32_t length);
void crypto_secure_zero(void* ptr, uint32_t length);

/* Algorithm Registration (Internal) */
status_t crypto_register_aes(void);
status_t crypto_register_rsa(void);
status_t crypto_register_sha(void);
status_t crypto_register_hmac(void);
status_t crypto_register_ecdsa(void);
status_t crypto_register_chacha20(void);

/* Hash Algorithm Sizes */
#define CRYPTO_SHA1_DIGEST_SIZE     20
#define CRYPTO_SHA256_DIGEST_SIZE   32
#define CRYPTO_SHA384_DIGEST_SIZE   48
#define CRYPTO_SHA512_DIGEST_SIZE   64

/* AES Key Sizes */
#define CRYPTO_AES128_KEY_SIZE      16
#define CRYPTO_AES192_KEY_SIZE      24
#define CRYPTO_AES256_KEY_SIZE      32
#define CRYPTO_AES_BLOCK_SIZE       16

/* RSA Key Sizes */
#define CRYPTO_RSA1024_KEY_SIZE     128
#define CRYPTO_RSA2048_KEY_SIZE     256
#define CRYPTO_RSA4096_KEY_SIZE     512

/* ChaCha20 Constants */
#define CRYPTO_CHACHA20_KEY_SIZE    32
#define CRYPTO_CHACHA20_NONCE_SIZE  12
#define CRYPTO_CHACHA20_BLOCK_SIZE  64

#ifdef __cplusplus
}
#endif