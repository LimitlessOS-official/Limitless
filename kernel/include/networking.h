/*
 * LimitlessOS Advanced Networking Stack - Header Definitions
 * Enterprise networking protocols and structures
 */

#ifndef LIMITLESSOS_NETWORKING_H
#define LIMITLESSOS_NETWORKING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Forward declarations
struct list_head;
struct mutex;
struct device;
struct work_struct;
struct workqueue_struct;

// Network statistics structure for user space
struct networking_statistics {
    uint64_t packets_processed;
    uint64_t bytes_processed;
    uint64_t tcp_connections_opened;
    uint64_t tcp_connections_closed;
    uint64_t quic_connections_opened;
    uint64_t routing_lookups;
    uint64_t firewall_hits;
    uint64_t security_violations;
    uint32_t active_connections;
    uint32_t peak_connections;
    
    uint32_t total_interfaces;
    uint32_t tcp_connections_active;
    uint32_t quic_connections_active;
    
    bool ipv4_enabled;
    bool ipv6_enabled;
    bool quic_enabled;
    bool security_enabled;
    bool qos_enabled;
};

// QoS configuration structure for user space
struct qos_config {
    uint32_t traffic_class;
    uint32_t bandwidth_limit;
    uint32_t burst_size;
    uint32_t priority;
    bool enabled;
};

// Socket address structures
struct sockaddr_in {
    uint16_t sin_family;
    uint16_t sin_port;
    uint32_t sin_addr;
    uint8_t sin_zero[8];
};

struct sockaddr_in6 {
    uint16_t sin6_family;
    uint16_t sin6_port;
    uint32_t sin6_flowinfo;
    uint8_t sin6_addr[16];
    uint32_t sin6_scope_id;
};

// Generic socket address
struct sockaddr {
    uint16_t sa_family;
    char sa_data[14];
};

// Network interface information for user space
struct interface_info {
    uint32_t interface_id;
    char name[16];
    uint8_t mac_address[6];
    uint32_t mtu;
    uint32_t flags;
    
    struct {
        uint32_t address;
        uint32_t netmask;
        uint32_t gateway;
        bool dhcp_enabled;
    } ipv4;
    
    struct {
        uint8_t address[16];
        uint32_t prefix_length;
        uint8_t gateway[16];
        bool autoconfig;
    } ipv6;
    
    struct {
        uint64_t rx_packets;
        uint64_t tx_packets;
        uint64_t rx_bytes;
        uint64_t tx_bytes;
        uint64_t rx_errors;
        uint64_t tx_errors;
        uint32_t speed_mbps;
        bool link_up;
    } stats;
};

// Routing entry for user space
struct route_entry {
    uint32_t destination;
    uint32_t netmask;
    uint32_t gateway;
    uint32_t interface_id;
    uint32_t metric;
    uint32_t flags;
};

// Firewall rule for user space
struct firewall_rule {
    uint32_t action;        // 0=allow, 1=deny, 2=drop
    uint32_t src_ip;
    uint32_t src_mask;
    uint32_t dst_ip;
    uint32_t dst_mask;
    uint16_t src_port_min;
    uint16_t src_port_max;
    uint16_t dst_port_min;
    uint16_t dst_port_max;
    uint32_t protocol;
    bool enabled;
};

// VLAN configuration
struct vlan_config {
    uint16_t vlan_id;
    uint32_t interface_id;
    char name[16];
    bool enabled;
};

// VPN configuration
struct vpn_config {
    uint32_t vpn_type;      // 0=IPsec, 1=WireGuard, 2=OpenVPN
    uint32_t local_ip;
    uint32_t remote_ip;
    uint16_t local_port;
    uint16_t remote_port;
    uint8_t key[32];
    bool enabled;
};

// Function prototypes for networking subsystem
int networking_subsystem_init(void);
void networking_subsystem_shutdown(void);
void networking_get_statistics(struct networking_statistics *stats);

// Socket API prototypes
long sys_socket_create(int family, int type, int protocol);
long sys_socket_bind(int sockfd, const struct sockaddr *addr, uint32_t addrlen);
long sys_socket_listen(int sockfd, int backlog);
long sys_socket_accept(int sockfd, struct sockaddr *addr, uint32_t *addrlen);
long sys_socket_connect(int sockfd, const struct sockaddr *addr, uint32_t addrlen);
long sys_socket_send(int sockfd, const void *buf, size_t len, int flags);
long sys_socket_recv(int sockfd, void *buf, size_t len, int flags);
long sys_socket_close(int sockfd);

// Network management system calls
long sys_network_get_statistics(struct networking_statistics *stats);
long sys_interface_get_info(uint32_t interface_id, struct interface_info *info);
long sys_interface_configure(uint32_t interface_id, struct interface_info *config);
long sys_route_add(struct route_entry *route);
long sys_route_delete(struct route_entry *route);
long sys_firewall_add_rule(struct firewall_rule *rule);
long sys_firewall_delete_rule(uint32_t rule_id);
long sys_qos_configure(uint32_t interface_id, struct qos_config *config);
long sys_vlan_configure(struct vlan_config *config);
long sys_vpn_configure(struct vpn_config *config);

// Protocol constants
#define ETH_P_IP        0x0800
#define ETH_P_IPV6      0x86DD
#define ETH_P_ARP       0x0806

// IP protocol numbers
#define IPPROTO_ICMP    1
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#define IPPROTO_IPV6    41
#define IPPROTO_ICMPV6  58
#define IPPROTO_SCTP    132

// Socket options
#define SOL_SOCKET      1
#define SO_REUSEADDR    2
#define SO_KEEPALIVE    9
#define SO_BROADCAST    6
#define SO_LINGER       13
#define SO_RCVBUF       8
#define SO_SNDBUF       7

// TCP options
#define TCP_NODELAY     1
#define TCP_MAXSEG      2
#define TCP_CORK        3
#define TCP_KEEPIDLE    4
#define TCP_KEEPINTVL   5
#define TCP_KEEPCNT     6
#define TCP_USER_TIMEOUT 18
#define TCP_CONGESTION  13

// Interface flags
#define IFF_UP          0x1
#define IFF_BROADCAST   0x2
#define IFF_DEBUG       0x4
#define IFF_LOOPBACK    0x8
#define IFF_POINTOPOINT 0x10
#define IFF_RUNNING     0x40
#define IFF_NOARP       0x80
#define IFF_PROMISC     0x100
#define IFF_ALLMULTI    0x200
#define IFF_MULTICAST   0x1000

// Route flags
#define RTF_UP          0x1
#define RTF_GATEWAY     0x2
#define RTF_HOST        0x4
#define RTF_REJECT      0x200
#define RTF_STATIC      0x800
#define RTF_DYNAMIC     0x10

// Error codes
#define EAFNOSUPPORT    97
#define EADDRINUSE      98
#define EADDRNOTAVAIL   99
#define ENETDOWN        100
#define ENETUNREACH     101
#define ECONNABORTED    103
#define ECONNRESET      104
#define ENOBUFS         105
#define EISCONN         106
#define ENOTCONN        107
#define ETIMEDOUT       110
#define ECONNREFUSED    111
#define EHOSTDOWN       112
#define EHOSTUNREACH    113
#define EALREADY        114
#define EINPROGRESS     115

#endif // LIMITLESSOS_NETWORKING_H