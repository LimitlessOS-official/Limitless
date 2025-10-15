#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "kernel.h"

#define VPN_MAX_PEERS 64
#define VPN_KEY_SIZE 32  /* symmetric for data channel (placeholder for ChaCha20 key) */
#define VPN_MAX_IFACES 4

/* Supported protocol types */
typedef enum { VPN_PROTO_WIREGUARD=1, VPN_PROTO_IPSEC=2, VPN_PROTO_OPENVPN=3 } vpn_proto_t;

typedef struct vpn_peer_keyset {
    uint8_t static_public[32];
    uint8_t static_private[32];
    uint8_t preshared_key[32];
    uint8_t rx_key[VPN_KEY_SIZE];
    uint8_t tx_key[VPN_KEY_SIZE];
    uint32_t key_epoch;
} vpn_peer_keyset_t;

typedef struct vpn_peer {
    uint32_t id;
    uint32_t ipv4_be;     /* assigned tunnel IP */
    uint16_t listen_port; /* receiving endpoint */
    uint16_t remote_port; /* remote endpoint */
    uint32_t remote_ipv4_be;
    vpn_peer_keyset_t keys;
    uint64_t packets_in;
    uint64_t packets_out;
    uint64_t bytes_in;
    uint64_t bytes_out;
    bool established;
    uint64_t last_handshake_ticks;
} vpn_peer_t;

typedef struct vpn_iface {
    char name[16];
    uint32_t if_id;
    vpn_proto_t proto;
    uint32_t local_ipv4_be;
    uint16_t listen_port;
    vpn_peer_t peers[VPN_MAX_PEERS];
    uint32_t peer_count;
    uint64_t packets_encaps;
    uint64_t packets_decaps;
    uint64_t bytes_encaps;
    uint64_t bytes_decaps;
    bool up;
} vpn_iface_t;

status_t vpn_init(void);
status_t vpn_create_interface(const char* name, vpn_proto_t proto, uint32_t local_ip_be, uint16_t listen_port, uint32_t* out_if_id);
status_t vpn_add_peer(uint32_t if_id, const vpn_peer_t* peer_template, uint32_t* out_peer_id);
status_t vpn_remove_peer(uint32_t if_id, uint32_t peer_id);
status_t vpn_set_peer_keys(uint32_t if_id, uint32_t peer_id, const vpn_peer_keyset_t* keys);
status_t vpn_process_inbound(uint32_t if_id, const uint8_t* packet, size_t len);
status_t vpn_process_outbound(uint32_t if_id, const uint8_t* payload, size_t len, uint32_t dst_peer_id);
status_t vpn_handshake(uint32_t if_id, uint32_t peer_id);
status_t vpn_get_interface(uint32_t if_id, vpn_iface_t* out);

/* Metrics aggregation */
typedef struct vpn_metrics {
    uint64_t ifaces;
    uint64_t peers;
    uint64_t packets_encaps;
    uint64_t packets_decaps;
    uint64_t handshakes;
    uint64_t key_rotations;
} vpn_metrics_t;
status_t vpn_get_metrics(vpn_metrics_t* out);
