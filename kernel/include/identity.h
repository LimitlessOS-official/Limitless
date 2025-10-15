#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "kernel.h"

#define IDENTITY_MAX_PROVIDERS 16
#define IDENTITY_MAX_PRINCIPALS 256
#define IDENTITY_MAX_SESSIONS 128
#define IDENTITY_MAX_NAME_LEN 128
#define IDENTITY_MAX_CREDENTIAL_LEN 256
#define IDENTITY_CACHE_SIZE 64

/* Provider types */
typedef enum {
    IDENTITY_PROVIDER_LDAP = 1,
    IDENTITY_PROVIDER_AD = 2,
    IDENTITY_PROVIDER_KERBEROS = 3,
    IDENTITY_PROVIDER_SAML = 4,
    IDENTITY_PROVIDER_OAUTH2 = 5
} identity_provider_type_t;

/* Authentication methods */
typedef enum {
    AUTH_METHOD_PASSWORD = 1,
    AUTH_METHOD_CERTIFICATE = 2,
    AUTH_METHOD_KERBEROS_TICKET = 3,
    AUTH_METHOD_TOKEN = 4
} auth_method_t;

/* Principal (user) information */
typedef struct identity_principal {
    uint32_t id;
    char name[IDENTITY_MAX_NAME_LEN];
    char domain[IDENTITY_MAX_NAME_LEN];
    char display_name[IDENTITY_MAX_NAME_LEN];
    char email[IDENTITY_MAX_NAME_LEN];
    uint32_t groups[16];
    uint8_t group_count;
    uint64_t created_time;
    uint64_t last_login;
    bool enabled;
} identity_principal_t;

/* Authentication session */
typedef struct identity_session {
    uint32_t session_id;
    uint32_t principal_id;
    uint32_t provider_id;
    auth_method_t method;
    uint64_t created_time;
    uint64_t expires_time;
    uint64_t last_activity;
    char client_ip[16];
    bool authenticated;
    uint8_t session_key[32];
} identity_session_t;

/* Kerberos ticket */
typedef struct kerberos_ticket {
    char service_principal[IDENTITY_MAX_NAME_LEN];
    char client_principal[IDENTITY_MAX_NAME_LEN];
    uint64_t issued_time;
    uint64_t expires_time;
    uint8_t session_key[32];
    uint8_t ticket_data[512];
    size_t ticket_size;
    bool renewable;
    uint32_t flags;
} kerberos_ticket_t;

/* LDAP connection */
typedef struct ldap_connection {
    uint32_t socket_fd;
    char server[IDENTITY_MAX_NAME_LEN];
    uint16_t port;
    char bind_dn[IDENTITY_MAX_NAME_LEN];
    char base_dn[IDENTITY_MAX_NAME_LEN];
    bool use_tls;
    bool connected;
    uint32_t message_id;
} ldap_connection_t;

/* Identity provider */
typedef struct identity_provider {
    uint32_t id;
    char name[IDENTITY_MAX_NAME_LEN];
    identity_provider_type_t type;
    char server[IDENTITY_MAX_NAME_LEN];
    uint16_t port;
    char domain[IDENTITY_MAX_NAME_LEN];
    char bind_credential[IDENTITY_MAX_CREDENTIAL_LEN];
    bool use_tls;
    bool active;
    uint64_t last_sync_time;
    uint64_t auth_attempts;
    uint64_t auth_successes;
    uint64_t auth_failures;
    
    union {
        struct {
            char base_dn[IDENTITY_MAX_NAME_LEN];
            char user_filter[256];
            char group_filter[256];
        } ldap;
        
        struct {
            char realm[IDENTITY_MAX_NAME_LEN];
            char kdc_server[IDENTITY_MAX_NAME_LEN];
            uint16_t kdc_port;
        } kerberos;
        
        struct {
            char forest[IDENTITY_MAX_NAME_LEN];
            char domain_controller[IDENTITY_MAX_NAME_LEN];
        } ad;
    } config;
} identity_provider_t;

/* Cache entry for resolved principals */
typedef struct identity_cache_entry {
    char principal_name[IDENTITY_MAX_NAME_LEN];
    uint32_t principal_id;
    uint32_t provider_id;
    uint64_t cached_time;
    uint64_t expires_time;
    bool valid;
} identity_cache_entry_t;

/* Identity metrics */
typedef struct identity_metrics {
    uint64_t providers_configured;
    uint64_t principals_cached;
    uint64_t sessions_active;
    uint64_t auth_attempts;
    uint64_t auth_successes;
    uint64_t auth_failures;
    uint64_t ldap_binds;
    uint64_t kerberos_tgts;
    uint64_t cache_hits;
    uint64_t cache_misses;
} identity_metrics_t;

/* Public API */
status_t identity_init(void);
status_t identity_add_provider(const identity_provider_t* provider, uint32_t* out_id);
status_t identity_remove_provider(uint32_t provider_id);
status_t identity_authenticate(uint32_t provider_id, const char* principal, 
                             const char* credential, auth_method_t method, uint32_t* out_session_id);
status_t identity_validate_session(uint32_t session_id, identity_session_t* out_session);
status_t identity_logout(uint32_t session_id);
status_t identity_lookup_principal(const char* name, const char* domain, identity_principal_t* out_principal);
status_t identity_get_provider(uint32_t provider_id, identity_provider_t* out_provider);
status_t identity_sync_provider(uint32_t provider_id);
status_t identity_get_metrics(identity_metrics_t* out);

/* LDAP operations */
status_t ldap_connect(const char* server, uint16_t port, bool use_tls, ldap_connection_t* out_conn);
status_t ldap_bind(ldap_connection_t* conn, const char* dn, const char* password);
status_t ldap_search(ldap_connection_t* conn, const char* base_dn, const char* filter, 
                    identity_principal_t* out_results, uint8_t* in_out_count);
status_t ldap_close(ldap_connection_t* conn);

/* Kerberos operations */
status_t kerberos_get_tgt(const char* principal, const char* password, const char* realm,
                         const char* kdc_server, uint16_t kdc_port, kerberos_ticket_t* out_tgt);
status_t kerberos_get_service_ticket(const kerberos_ticket_t* tgt, const char* service_principal,
                                    kerberos_ticket_t* out_ticket);
status_t kerberos_validate_ticket(const kerberos_ticket_t* ticket, const char* service_key);
status_t kerberos_renew_ticket(kerberos_ticket_t* ticket);