#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "kernel.h"

#define DOH_MAX_HOSTNAME 256
#define DOH_MAX_RESOLVERS 4
#define DOH_CACHE_SIZE 128
#define DOH_BUFFER_SIZE 2048

/* DNS record types */
#define DNS_TYPE_A     1
#define DNS_TYPE_AAAA  28
#define DNS_TYPE_CNAME 5
#define DNS_TYPE_MX    15
#define DNS_TYPE_TXT   16

/* DNS response codes */
#define DNS_RCODE_OK      0
#define DNS_RCODE_FORMERR 1
#define DNS_RCODE_SERVFAIL 2
#define DNS_RCODE_NXDOMAIN 3

/* TLS handshake states */
typedef enum {
    TLS_STATE_INIT,
    TLS_STATE_HELLO_SENT,
    TLS_STATE_CERT_RECEIVED,
    TLS_STATE_ESTABLISHED,
    TLS_STATE_ERROR
} tls_state_t;

/* DNS record */
typedef struct dns_record {
    char name[DOH_MAX_HOSTNAME];
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    uint8_t rdata[256];
} dns_record_t;

/* DNS response */
typedef struct dns_response {
    uint16_t transaction_id;
    uint16_t flags;
    uint16_t questions;
    uint16_t answers;
    uint16_t authority;
    uint16_t additional;
    dns_record_t records[16];
    uint8_t record_count;
} dns_response_t;

/* DoH resolver configuration */
typedef struct doh_resolver {
    char url[256];
    char hostname[DOH_MAX_HOSTNAME];
    uint32_t ip_be;
    uint16_t port;
    bool active;
    uint64_t queries_sent;
    uint64_t queries_successful;
    uint64_t queries_failed;
} doh_resolver_t;

/* TLS connection context */
typedef struct tls_connection {
    uint32_t socket_fd;
    tls_state_t state;
    uint8_t client_random[32];
    uint8_t server_random[32];
    uint8_t master_secret[48];
    uint8_t client_write_key[16];
    uint8_t server_write_key[16];
    uint8_t client_write_iv[16];
    uint8_t server_write_iv[16];
    uint32_t client_seq;
    uint32_t server_seq;
    bool established;
} tls_connection_t;

/* Cache entry */
typedef struct doh_cache_entry {
    char hostname[DOH_MAX_HOSTNAME];
    uint16_t type;
    uint32_t ip_be;
    uint64_t expiry_ticks;
    bool valid;
} doh_cache_entry_t;

/* DoH metrics */
typedef struct doh_metrics {
    uint64_t queries_total;
    uint64_t cache_hits;
    uint64_t cache_misses;
    uint64_t tls_handshakes;
    uint64_t http_requests;
    uint64_t dns_packets_parsed;
    uint64_t resolver_failures;
} doh_metrics_t;

/* Public API */
status_t doh_init(void);
status_t doh_add_resolver(const char* url, const char* hostname, uint32_t ip_be, uint16_t port);
status_t doh_resolve(const char* hostname, uint16_t type, uint32_t* out_ip_be);
status_t doh_resolve_multi(const char* hostname, uint16_t type, dns_record_t* out_records, uint8_t* in_out_count);
status_t doh_flush_cache(void);
status_t doh_get_metrics(doh_metrics_t* out);

/* TLS client API */
status_t tls_connect(const char* hostname, uint32_t ip_be, uint16_t port, tls_connection_t* out_conn);
status_t tls_send(tls_connection_t* conn, const uint8_t* data, size_t len);
status_t tls_receive(tls_connection_t* conn, uint8_t* buffer, size_t buffer_size, size_t* out_len);
status_t tls_close(tls_connection_t* conn);

/* DNS packet encoding/decoding */
status_t dns_encode_query(const char* hostname, uint16_t type, uint8_t* buffer, size_t buffer_size, size_t* out_len);
status_t dns_decode_response(const uint8_t* buffer, size_t len, dns_response_t* out_response);