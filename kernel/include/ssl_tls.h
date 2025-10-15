/**
 * SSL/TLS Framework Header for LimitlessOS
 * Complete SSL/TLS 1.2 and 1.3 support
 */

#pragma once
#include "kernel.h"
#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TLS Protocol Versions */
typedef enum {
    TLS_VERSION_1_0 = 0x0301,
    TLS_VERSION_1_1 = 0x0302,
    TLS_VERSION_1_2 = 0x0303,
    TLS_VERSION_1_3 = 0x0304,
} tls_version_t;

/* TLS Session Types */
typedef enum {
    TLS_SESSION_CLIENT = 0,
    TLS_SESSION_SERVER,
} tls_session_type_t;

/* TLS Connection States */
typedef enum {
    TLS_STATE_INIT = 0,
    TLS_STATE_HANDSHAKE,
    TLS_STATE_WAIT_CLIENT_HELLO,
    TLS_STATE_WAIT_SERVER_HELLO,
    TLS_STATE_WAIT_CERTIFICATE,
    TLS_STATE_WAIT_SERVER_KEY_EXCHANGE,
    TLS_STATE_WAIT_SERVER_HELLO_DONE,
    TLS_STATE_SEND_CLIENT_KEY_EXCHANGE,
    TLS_STATE_WAIT_FINISHED,
    TLS_STATE_CONNECTED,
    TLS_STATE_CLOSED,
    TLS_STATE_ERROR,
} tls_state_t;

/* TLS Cipher Suites */
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

/* TLS Certificate Types */
typedef enum {
    TLS_CERT_TYPE_RSA = 1,
    TLS_CERT_TYPE_ECDSA = 64,
} tls_certificate_type_t;

/* Maximum sizes */
#define TLS_MAX_RECORD_SIZE         16384
#define TLS_MAX_HANDSHAKE_SIZE      65536
#define TLS_MAX_CERTIFICATE_SIZE    32768
#define TLS_RANDOM_SIZE             32
#define TLS_SESSION_ID_MAX_SIZE     32

/* TLS Certificate */
typedef struct tls_certificate {
    uint32_t certificate_id;
    tls_certificate_type_t type;
    uint8_t* data;
    uint32_t data_length;
    
    /* Certificate chain */
    struct tls_certificate* issuer;
    struct tls_certificate* next;
    
    /* Validity */
    uint64_t not_before;
    uint64_t not_after;
    bool is_valid;
    
    /* Subject information */
    char common_name[256];
    char organization[256];
    char country[8];
    
    /* Key information */
    crypto_context_t* public_key_context;
    crypto_context_t* private_key_context;
} tls_certificate_t;

/* TLS Session */
typedef struct tls_session {
    uint32_t session_id;
    tls_session_type_t type;
    tls_state_t state;
    
    /* Network connection */
    int socket_fd;
    
    /* Protocol info */
    tls_version_t version;
    tls_cipher_suite_t cipher_suite;
    
    /* Random values */
    uint8_t client_random[TLS_RANDOM_SIZE];
    uint8_t server_random[TLS_RANDOM_SIZE];
    
    /* Session keys */
    uint8_t master_secret[48];
    uint8_t client_write_key[32];
    uint8_t server_write_key[32];
    uint8_t client_write_iv[16];
    uint8_t server_write_iv[16];
    
    /* Certificates */
    tls_certificate_t* server_certificate;
    tls_certificate_t* client_certificate;
    
    /* Crypto contexts */
    crypto_context_t* encrypt_context;
    crypto_context_t* decrypt_context;
    crypto_context_t* mac_context;
    
    /* Sequence numbers */
    uint64_t client_seq_num;
    uint64_t server_seq_num;
    
    /* Buffers */
    uint8_t* send_buffer;
    uint8_t* recv_buffer;
    uint32_t send_buffer_size;
    uint32_t recv_buffer_size;
    
    /* Handshake state */
    uint8_t* handshake_messages;
    uint32_t handshake_messages_length;
    
    /* Extensions */
    bool server_name_indication;
    char server_name[256];
    
    bool application_layer_protocol_negotiation;
    char selected_protocol[32];
    
    /* Statistics */
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t records_sent;
    uint64_t records_received;
} tls_session_t;

/* SSL/TLS Core API */
status_t ssl_tls_init(void);
status_t ssl_tls_shutdown(void);

/* Session Management */
status_t tls_create_session(tls_session_type_t type, tls_session_t** session);
status_t tls_set_socket(tls_session_t* session, int socket_fd);
status_t tls_close(tls_session_t* session);

/* Certificate Management */
status_t tls_load_certificate(const char* certificate_path, const char* private_key_path,
                            tls_certificate_t** certificate);
status_t tls_set_certificate(tls_session_t* session, tls_certificate_t* certificate);
status_t tls_verify_certificate(tls_certificate_t* certificate, bool* is_valid);

/* TLS Handshake */
status_t tls_handshake(tls_session_t* session);
status_t tls_process_record(tls_session_t* session, const uint8_t* data, uint32_t length);

/* Data Transfer */
status_t tls_send(tls_session_t* session, const void* data, uint32_t length);
status_t tls_recv(tls_session_t* session, void* buffer, uint32_t buffer_size, 
                 uint32_t* received);

/* Configuration */
status_t tls_set_cipher_suites(tls_session_t* session, const tls_cipher_suite_t* suites,
                              uint32_t count);
status_t tls_set_version(tls_session_t* session, tls_version_t version);
status_t tls_set_server_name(tls_session_t* session, const char* server_name);

/* Internal functions (used by ssl_tls.c) */
status_t tls_send_client_hello(tls_session_t* session);
status_t tls_process_handshake(tls_session_t* session, const uint8_t* data, uint32_t length);
status_t tls_process_server_hello(tls_session_t* session, const uint8_t* data, uint32_t length);
status_t tls_process_certificate(tls_session_t* session, const uint8_t* data, uint32_t length);
status_t tls_process_server_hello_done(tls_session_t* session);
status_t tls_process_finished(tls_session_t* session, const uint8_t* data, uint32_t length);
status_t tls_process_application_data(tls_session_t* session, const uint8_t* data, uint32_t length);
status_t tls_process_alert(tls_session_t* session, const uint8_t* data, uint32_t length);

/* Utility functions */
ssize_t net_send_socket(int socket_fd, const void* data, size_t length);
ssize_t net_recv_socket(int socket_fd, void* buffer, size_t length);

#ifdef __cplusplus
}
#endif