/*
 * LimitlessOS Advanced Networking Subsystem
 * Production TCP/IP stack with Wi-Fi, security, and performance optimization
 * Features: IPv4/IPv6, wireless, VPN, firewall, QoS, AI optimization
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../kernel.h"

/* Networking constants */
#define MAX_NETWORK_INTERFACES  64
#define MAX_PACKET_SIZE         9000    /* Jumbo frame support */
#define MAX_ROUTING_ENTRIES     65536
#define MAX_FIREWALL_RULES      4096
#define MAX_QOS_CLASSES         16
#define MAX_WIRELESS_NETWORKS   256

/* Network protocol types */
typedef enum {
    NET_PROTO_ETHERNET = 0,
    NET_PROTO_ARP,
    NET_PROTO_IP,
    NET_PROTO_ICMP,
    NET_PROTO_IGMP,
    NET_PROTO_TCP,
    NET_PROTO_UDP,
    NET_PROTO_IPV6,
    NET_PROTO_ICMPV6,
    NET_PROTO_DHCP,
    NET_PROTO_DNS,
    NET_PROTO_HTTP,
    NET_PROTO_HTTPS,
    NET_PROTO_SSH,
    NET_PROTO_VPN,
    NET_PROTO_MAX
} network_protocol_t;

/* Network interface types */
typedef enum {
    NET_IF_ETHERNET = 0,
    NET_IF_WIRELESS,
    NET_IF_LOOPBACK,
    NET_IF_PPP,
    NET_IF_VPN,
    NET_IF_BRIDGE,
    NET_IF_VLAN,
    NET_IF_TUN_TAP,
    NET_IF_VIRTUAL,
    NET_IF_MAX
} net_interface_type_t;

/* IPv4 address structure */
typedef struct ipv4_addr {
    union {
        uint32_t addr;
        uint8_t octets[4];
    };
} ipv4_addr_t;

/* IPv6 address structure */
typedef struct ipv6_addr {
    union {
        uint8_t addr[16];
        uint16_t addr16[8];
        uint32_t addr32[4];
    };
} ipv6_addr_t;

/* MAC address structure */
typedef struct mac_addr {
    uint8_t addr[6];
} mac_addr_t;

/* Network packet structure */
typedef struct network_packet {
    /* Buffer management */
    struct {
        uint8_t *data;
        uint8_t *head;
        uint8_t *tail;
        uint8_t *end;
        uint32_t len;
        uint32_t data_len;
        uint32_t truesize;
    } buffer;
    
    /* Network headers */
    struct {
        uint8_t *mac_header;
        uint8_t *network_header;
        uint8_t *transport_header;
        uint8_t *application_header;
    } headers;
    
    /* Protocol information */
    uint16_t protocol;
    uint8_t ip_summed;
    uint8_t csum_valid;
    uint32_t csum;
    
    /* Interface information */
    struct net_interface *input_dev;
    struct net_interface *output_dev;
    
    /* Routing information */
    struct {
        ipv4_addr_t src_ip;
        ipv4_addr_t dst_ip;
        uint16_t src_port;
        uint16_t dst_port;
    } flow;
    
    /* QoS information */
    uint8_t priority;
    uint8_t tos;                /* Type of Service */
    uint32_t mark;              /* Packet mark for firewall/QoS */
    
    /* Security context */
    struct {
        bool encrypted;
        uint32_t sec_mark;
        uint16_t vlan_id;
    } security;
    
    /* Timing information */
    uint64_t timestamp;
    uint64_t hw_timestamp;
    
    /* Fragment information */
    struct {
        bool is_fragment;
        uint16_t fragment_id;
        uint16_t fragment_offset;
        bool more_fragments;
    } fragment;
    
    /* Statistics and debugging */
    uint32_t packet_id;
    uint16_t queue_id;
    
    /* Reference counting */
    atomic_t users;
    
    /* List management */
    struct list_head list;
    
    /* GSO (Generic Segmentation Offload) */
    struct {
        uint16_t size;
        uint16_t segs;
        uint16_t type;
    } gso;
    
} network_packet_t;

/* Wi-Fi security types */
typedef enum {
    WIFI_SEC_NONE = 0,
    WIFI_SEC_WEP,
    WIFI_SEC_WPA,
    WIFI_SEC_WPA2,
    WIFI_SEC_WPA3,
    WIFI_SEC_WPS,
    WIFI_SEC_ENTERPRISE,
    WIFI_SEC_MAX
} wifi_security_t;

/* Wi-Fi network information */
typedef struct wifi_network {
    char ssid[33];              /* Network name */
    mac_addr_t bssid;           /* Access point MAC */
    
    /* Security */
    wifi_security_t security;
    uint8_t key_mgmt;
    uint8_t cipher;
    
    /* Signal information */
    int8_t signal_strength_dbm;
    uint8_t signal_quality;
    uint32_t frequency_mhz;
    uint8_t channel;
    
    /* Capabilities */
    struct {
        bool supports_11n;
        bool supports_11ac;
        bool supports_11ax;
        bool supports_wps;
        bool supports_pmf;      /* Protected Management Frames */
        uint16_t max_data_rate_mbps;
    } caps;
    
    /* Connection state */
    enum {
        WIFI_STATE_DISCONNECTED = 0,
        WIFI_STATE_CONNECTING,
        WIFI_STATE_AUTHENTICATING,
        WIFI_STATE_ASSOCIATING,
        WIFI_STATE_CONNECTED,
        WIFI_STATE_FAILED
    } connection_state;
    
    /* Statistics */
    struct {
        uint64_t bytes_rx;
        uint64_t bytes_tx;
        uint32_t packets_rx;
        uint32_t packets_tx;
        uint32_t retries;
        uint32_t failed;
    } stats;
    
    struct list_head list;
    
} wifi_network_t;

/* Wireless interface structure */
typedef struct wireless_interface {
    struct net_interface *netdev;
    
    /* Hardware information */
    char driver_name[32];
    char firmware_version[32];
    mac_addr_t hw_addr;
    
    /* Current connection */
    wifi_network_t *current_network;
    char current_ssid[33];
    
    /* Scanning */
    struct {
        bool active;
        uint64_t last_scan_time;
        wifi_network_t networks[MAX_WIRELESS_NETWORKS];
        uint32_t network_count;
    } scan;
    
    /* Capabilities */
    struct {
        bool supports_ap_mode;
        bool supports_monitor;
        bool supports_mesh;
        bool supports_p2p;
        uint32_t supported_bands;
        uint32_t max_tx_power_dbm;
        uint8_t supported_channels[64];
        uint8_t num_channels;
    } caps;
    
    /* Power management */
    struct {
        bool enabled;
        uint32_t listen_interval;
        bool supports_wowlan;
    } power_mgmt;
    
    /* Security context */
    struct {
        uint8_t pmk[32];        /* Pairwise Master Key */
        uint8_t ptk[64];        /* Pairwise Transient Key */
        uint8_t gtk[32];        /* Group Temporal Key */
        uint32_t key_replay_counter;
    } security_ctx;
    
} wireless_interface_t;

/* Network interface statistics */
typedef struct net_interface_stats {
    /* Receive statistics */
    uint64_t rx_packets;
    uint64_t rx_bytes;
    uint64_t rx_errors;
    uint64_t rx_dropped;
    uint64_t rx_fifo_errors;
    uint64_t rx_frame_errors;
    uint64_t rx_compressed;
    uint64_t multicast;
    
    /* Transmit statistics */
    uint64_t tx_packets;
    uint64_t tx_bytes;
    uint64_t tx_errors;
    uint64_t tx_dropped;
    uint64_t tx_fifo_errors;
    uint64_t collisions;
    uint64_t tx_carrier_errors;
    uint64_t tx_compressed;
    
    /* Quality metrics */
    uint64_t rx_crc_errors;
    uint64_t rx_length_errors;
    uint64_t rx_missed_errors;
    uint64_t tx_aborted_errors;
    uint64_t tx_window_errors;
    uint64_t tx_heartbeat_errors;
    
} net_interface_stats_t;

/* Network interface structure */
typedef struct net_interface {
    uint32_t ifindex;
    char name[16];
    net_interface_type_t type;
    
    /* Hardware information */
    mac_addr_t hw_addr;
    uint32_t mtu;
    uint16_t hard_header_len;
    uint16_t needed_headroom;
    uint16_t needed_tailroom;
    
    /* State */
    uint32_t flags;
    uint8_t operstate;
    uint8_t link_mode;
    
    /* Addressing */
    struct {
        ipv4_addr_t ipv4_addr;
        ipv4_addr_t ipv4_mask;
        ipv4_addr_t ipv4_broadcast;
        ipv6_addr_t ipv6_addr[8];
        uint8_t ipv6_prefix_len[8];
        uint8_t ipv6_addr_count;
    } addr;
    
    /* Traffic control */
    struct {
        struct net_qdisc *qdisc;
        uint32_t tx_queue_len;
        uint8_t num_tx_queues;
        uint8_t real_num_tx_queues;
    } qdisc;
    
    /* Statistics */
    net_interface_stats_t stats;
    
    /* Wireless extension (if applicable) */
    wireless_interface_t *wireless;
    
    /* Device operations */
    const struct net_device_ops *netdev_ops;
    
    /* Hardware device */
    struct device *dev;
    
    /* Private data */
    void *priv;
    
    struct list_head list;
    
} net_interface_t;

/* Routing table entry */
typedef struct route_entry {
    ipv4_addr_t destination;
    ipv4_addr_t netmask;
    ipv4_addr_t gateway;
    struct net_interface *interface;
    uint32_t metric;
    uint8_t protocol;
    uint32_t flags;
    
    /* Route statistics */
    struct {
        uint64_t packets;
        uint64_t bytes;
        uint64_t last_used;
    } stats;
    
    struct list_head list;
    
} route_entry_t;

/* Firewall rule structure */
typedef struct firewall_rule {
    uint32_t rule_id;
    uint8_t priority;
    
    /* Match criteria */
    struct {
        ipv4_addr_t src_ip;
        ipv4_addr_t src_mask;
        ipv4_addr_t dst_ip;
        ipv4_addr_t dst_mask;
        uint16_t src_port_min;
        uint16_t src_port_max;
        uint16_t dst_port_min;
        uint16_t dst_port_max;
        uint8_t protocol;
        struct net_interface *input_if;
        struct net_interface *output_if;
        uint32_t mark_mask;
        uint32_t mark_value;
    } match;
    
    /* Action */
    enum {
        FW_ACTION_ACCEPT = 0,
        FW_ACTION_DROP,
        FW_ACTION_REJECT,
        FW_ACTION_LOG,
        FW_ACTION_MARK,
        FW_ACTION_NAT,
        FW_ACTION_REDIRECT
    } action;
    
    /* Action parameters */
    union {
        uint32_t mark_value;
        struct {
            ipv4_addr_t new_ip;
            uint16_t new_port;
        } nat;
        uint16_t redirect_port;
    } action_params;
    
    /* Statistics */
    struct {
        uint64_t packets_matched;
        uint64_t bytes_matched;
        uint64_t last_match_time;
    } stats;
    
    struct list_head list;
    
} firewall_rule_t;

/* QoS traffic class */
typedef struct qos_class {
    uint8_t class_id;
    char name[32];
    
    /* Traffic limits */
    uint32_t min_rate_kbps;
    uint32_t max_rate_kbps;
    uint32_t burst_size_kb;
    uint8_t priority;
    
    /* Queue parameters */
    uint32_t queue_size;
    enum {
        QOS_SCHED_FIFO = 0,
        QOS_SCHED_WFQ,          /* Weighted Fair Queuing */
        QOS_SCHED_CBQ,          /* Class-Based Queuing */
        QOS_SCHED_HTB,          /* Hierarchical Token Bucket */
        QOS_SCHED_PFIFO,        /* Packet FIFO */
    } scheduler_type;
    
    /* Classification rules */
    struct {
        uint32_t mark_mask;
        uint32_t mark_value;
        uint8_t dscp;
        uint16_t vlan_priority;
    } classification;
    
    /* Statistics */
    struct {
        uint64_t packets_queued;
        uint64_t packets_dropped;
        uint64_t bytes_sent;
        uint32_t current_queue_len;
        uint32_t max_queue_len;
    } stats;
    
    struct list_head list;
    
} qos_class_t;

/* VPN tunnel structure */
typedef struct vpn_tunnel {
    uint32_t tunnel_id;
    char name[32];
    
    enum {
        VPN_TYPE_IPSEC = 0,
        VPN_TYPE_WIREGUARD,
        VPN_TYPE_OPENVPN,
        VPN_TYPE_L2TP,
        VPN_TYPE_PPTP,
        VPN_TYPE_SSTP
    } type;
    
    /* Endpoints */
    struct {
        ipv4_addr_t local_ip;
        ipv4_addr_t remote_ip;
        uint16_t local_port;
        uint16_t remote_port;
    } endpoints;
    
    /* Tunnel configuration */
    struct {
        ipv4_addr_t tunnel_local_ip;
        ipv4_addr_t tunnel_remote_ip;
        ipv4_addr_t tunnel_netmask;
        uint32_t mtu;
    } tunnel_config;
    
    /* Cryptography */
    struct {
        uint8_t encryption_algorithm;
        uint8_t hash_algorithm;
        uint8_t key[64];
        uint32_t key_len;
        uint64_t seq_number;
        uint32_t spi;           /* Security Parameter Index */
    } crypto;
    
    /* State */
    enum {
        VPN_STATE_DOWN = 0,
        VPN_STATE_CONNECTING,
        VPN_STATE_UP,
        VPN_STATE_ERROR
    } state;
    
    /* Statistics */
    struct {
        uint64_t bytes_in;
        uint64_t bytes_out;
        uint64_t packets_in;
        uint64_t packets_out;
        uint64_t errors;
        uint64_t connection_time;
    } stats;
    
    /* Interface */
    struct net_interface *interface;
    
    struct list_head list;
    
} vpn_tunnel_t;

/* Network AI optimization engine */
typedef struct network_ai_engine {
    bool enabled;
    
    /* Traffic pattern analysis */
    struct {
        float bandwidth_prediction[24];     /* Hourly predictions */
        float latency_prediction[24];
        uint32_t optimal_buffer_size;
        uint32_t congestion_window_size;
    } traffic_analysis;
    
    /* Protocol optimization */
    struct {
        uint32_t tcp_initial_cwnd;
        uint32_t tcp_slow_start_threshold;
        bool tcp_timestamps_enabled;
        bool tcp_sack_enabled;
        uint32_t udp_buffer_size;
    } protocol_tuning;
    
    /* QoS optimization */
    struct {
        uint8_t priority_mapping[8];
        uint32_t bandwidth_allocation[MAX_QOS_CLASSES];
        float loss_tolerance[MAX_QOS_CLASSES];
    } qos_optimization;
    
    /* Security threat detection */
    struct {
        float ddos_probability;
        float port_scan_probability;
        uint32_t suspicious_connections;
        uint64_t blocked_packets;
    } security_analysis;
    
    /* Performance predictions */
    struct {
        uint64_t predicted_throughput_bps;
        uint32_t predicted_latency_ms;
        float network_utilization;
        uint32_t optimal_mtu;
    } performance;
    
    /* Learning statistics */
    uint64_t samples_analyzed;
    uint64_t predictions_made;
    uint64_t correct_predictions;
    float accuracy_percentage;
    
} network_ai_engine_t;

/* Global networking subsystem */
typedef struct networking_subsystem {
    bool initialized;
    
    /* Interface registry */
    struct {
        net_interface_t *interfaces[MAX_NETWORK_INTERFACES];
        uint32_t count;
        struct net_interface *loopback;
        rwlock_t lock;
    } interfaces;
    
    /* Routing */
    struct {
        route_entry_t *ipv4_routes;
        uint32_t ipv4_route_count;
        route_entry_t *ipv6_routes;
        uint32_t ipv6_route_count;
        rwlock_t lock;
    } routing;
    
    /* Firewall */
    struct {
        bool enabled;
        firewall_rule_t *rules;
        uint32_t rule_count;
        uint32_t default_policy;    /* Accept or drop */
        rwlock_t lock;
    } firewall;
    
    /* QoS */
    struct {
        bool enabled;
        qos_class_t *classes;
        uint32_t class_count;
        uint32_t default_class_id;
        rwlock_t lock;
    } qos;
    
    /* VPN */
    struct {
        vpn_tunnel_t *tunnels;
        uint32_t tunnel_count;
        bool ipsec_enabled;
        bool wireguard_enabled;
        rwlock_t lock;
    } vpn;
    
    /* Protocol stacks */
    struct {
        bool ipv4_enabled;
        bool ipv6_enabled;
        bool tcp_enabled;
        bool udp_enabled;
        bool icmp_enabled;
        uint32_t tcp_max_connections;
        uint32_t udp_max_sockets;
    } protocols;
    
    /* AI optimization */
    network_ai_engine_t ai_engine;
    
    /* Global statistics */
    struct {
        uint64_t total_packets_rx;
        uint64_t total_packets_tx;
        uint64_t total_bytes_rx;
        uint64_t total_bytes_tx;
        uint64_t total_errors;
        uint64_t total_drops;
    } stats;
    
    /* Performance monitoring */
    struct {
        uint32_t avg_latency_ms;
        uint64_t throughput_bps;
        uint32_t packet_loss_rate;
        uint32_t congestion_level;
    } performance;
    
} networking_subsystem_t;

/* External networking subsystem */
extern networking_subsystem_t net_subsystem;

/* Core networking functions */
int networking_init(void);
void networking_exit(void);

/* Interface management */
int net_register_interface(struct net_interface *netif);
void net_unregister_interface(struct net_interface *netif);
struct net_interface *net_get_interface_by_name(const char *name);
struct net_interface *net_get_interface_by_index(uint32_t ifindex);
int net_interface_up(struct net_interface *netif);
int net_interface_down(struct net_interface *netif);

/* Packet handling */
struct network_packet *net_alloc_packet(uint32_t size, gfp_t flags);
void net_free_packet(struct network_packet *packet);
int net_transmit_packet(struct net_interface *netif, struct network_packet *packet);
int net_receive_packet(struct net_interface *netif, struct network_packet *packet);

/* Protocol stack */
int net_ipv4_input(struct network_packet *packet);
int net_ipv6_input(struct network_packet *packet);
int net_tcp_input(struct network_packet *packet);
int net_udp_input(struct network_packet *packet);
int net_icmp_input(struct network_packet *packet);

/* Routing */
int net_add_route(const struct route_entry *route);
int net_del_route(ipv4_addr_t destination, ipv4_addr_t netmask);
struct route_entry *net_find_route(ipv4_addr_t destination);
int net_set_default_gateway(ipv4_addr_t gateway, struct net_interface *netif);

/* Firewall */
int net_firewall_enable(void);
void net_firewall_disable(void);
int net_firewall_add_rule(const struct firewall_rule *rule);
int net_firewall_del_rule(uint32_t rule_id);
int net_firewall_check_packet(struct network_packet *packet);

/* QoS */
int net_qos_enable(void);
void net_qos_disable(void);
int net_qos_add_class(const struct qos_class *class);
int net_qos_del_class(uint8_t class_id);
int net_qos_classify_packet(struct network_packet *packet);

/* Wireless networking */
int net_wifi_scan(struct wireless_interface *wif);
int net_wifi_connect(struct wireless_interface *wif, const char *ssid, 
                     const char *password, wifi_security_t security);
int net_wifi_disconnect(struct wireless_interface *wif);
wifi_network_t *net_wifi_get_current_network(struct wireless_interface *wif);

/* VPN support */
int net_vpn_create_tunnel(const struct vpn_tunnel *config);
int net_vpn_destroy_tunnel(uint32_t tunnel_id);
int net_vpn_connect(uint32_t tunnel_id);
int net_vpn_disconnect(uint32_t tunnel_id);

/* Network configuration */
int net_configure_interface(struct net_interface *netif, ipv4_addr_t ip, 
                           ipv4_addr_t netmask, ipv4_addr_t gateway);
int net_dhcp_request(struct net_interface *netif);
int net_dns_resolve(const char *hostname, ipv4_addr_t *ip);
int net_dns_add_server(ipv4_addr_t dns_server);

/* AI optimization */
void net_ai_analyze_traffic(struct network_packet *packet);
void net_ai_optimize_protocols(void);
void net_ai_update_qos(void);
uint32_t net_ai_predict_bandwidth(void);
uint32_t net_ai_predict_latency(void);

/* Security functions */
int net_ipsec_encrypt_packet(struct network_packet *packet, const struct vpn_tunnel *tunnel);
int net_ipsec_decrypt_packet(struct network_packet *packet, const struct vpn_tunnel *tunnel);
int net_tls_encrypt_data(const void *plaintext, size_t len, void *ciphertext, size_t *ciphertext_len);
int net_tls_decrypt_data(const void *ciphertext, size_t len, void *plaintext, size_t *plaintext_len);

/* Performance monitoring */
void net_update_statistics(struct net_interface *netif, struct network_packet *packet, bool tx);
uint32_t net_calculate_bandwidth(struct net_interface *netif);
uint32_t net_measure_latency(ipv4_addr_t target_ip);
void net_print_statistics(void);

/* Socket layer integration */
int net_socket_create(int family, int type, int protocol);
int net_socket_bind(int sockfd, ipv4_addr_t addr, uint16_t port);
int net_socket_listen(int sockfd, int backlog);
int net_socket_accept(int sockfd, ipv4_addr_t *client_addr, uint16_t *client_port);
int net_socket_connect(int sockfd, ipv4_addr_t addr, uint16_t port);
ssize_t net_socket_send(int sockfd, const void *data, size_t len, int flags);
ssize_t net_socket_recv(int sockfd, void *buffer, size_t len, int flags);
int net_socket_close(int sockfd);

/* Network interface flags */
#define NET_IF_UP               0x0001
#define NET_IF_BROADCAST        0x0002
#define NET_IF_DEBUG            0x0004
#define NET_IF_LOOPBACK         0x0008
#define NET_IF_POINTOPOINT      0x0010
#define NET_IF_NOTRAILERS       0x0020
#define NET_IF_RUNNING          0x0040
#define NET_IF_NOARP            0x0080
#define NET_IF_PROMISC          0x0100
#define NET_IF_ALLMULTI         0x0200
#define NET_IF_MASTER           0x0400
#define NET_IF_SLAVE            0x0800
#define NET_IF_MULTICAST        0x1000
#define NET_IF_PORTSEL          0x2000
#define NET_IF_AUTOMEDIA        0x4000
#define NET_IF_DYNAMIC          0x8000

/* Protocol definitions */
#define IPPROTO_IP              0
#define IPPROTO_ICMP            1
#define IPPROTO_TCP             6
#define IPPROTO_UDP             17
#define IPPROTO_IPV6            41
#define IPPROTO_ICMPV6          58

/* Utility macros */
#define NIPQUAD(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3]

#define NIPQUAD_FMT "%u.%u.%u.%u"

#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ARG(mac) (mac).addr[0], (mac).addr[1], (mac).addr[2], \
                     (mac).addr[3], (mac).addr[4], (mac).addr[5]