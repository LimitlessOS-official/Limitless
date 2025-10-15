/*
 * LimitlessOS Advanced Network Stack Header
 * Enterprise-grade networking with comprehensive networking capabilities
 */

#ifndef ADVANCED_NETWORK_H
#define ADVANCED_NETWORK_H

#include <stdint.h>
#include <stdbool.h>

/* Network configuration constants */
#define NET_MAX_INTERFACES      256
#define NET_MAX_ROUTES          8192
#define NET_MAX_CONNECTIONS     65536
#define NET_MAX_NAMESPACES      256
#define NET_MAX_QOS_CLASSES     64
#define NET_MAX_FILTERS         1024
#define NET_PACKET_POOL_SIZE    32768
#define NET_BUFFER_SIZE         9000  /* Jumbo frame support */

/* QoS classes */
typedef enum {
    QOS_CLASS_BEST_EFFORT = 0,
    QOS_CLASS_BULK = 1,
    QOS_CLASS_INTERACTIVE = 2,
    QOS_CLASS_VOICE = 3,
    QOS_CLASS_VIDEO = 4,
    QOS_CLASS_CONTROL = 5,
    QOS_CLASS_CRITICAL = 6,
    QOS_CLASS_ENTERPRISE = 7
} qos_class_t;

/* Forward declarations */
typedef struct network_packet network_packet_t;
typedef struct network_interface network_interface_t;
typedef struct tcp_connection tcp_connection_t;
typedef struct network_namespace network_namespace_t;

/* ============================================================================
 * CORE NETWORK API
 * ============================================================================ */

/**
 * Initialize advanced network stack
 * @return 0 on success, negative on error
 */
int advanced_network_init(void);

/**
 * Process incoming packet
 * @param packet Packet to process
 * @return 0 on success, negative on error
 */
int network_process_packet(network_packet_t* packet);

/**
 * Create network interface
 * @param name Interface name
 * @return Interface pointer on success, NULL on error
 */
network_interface_t* network_create_interface(const char* name);

/**
 * Configure network interface
 * @param iface Interface to configure
 * @param ip_addr IPv4 address
 * @param netmask Network mask
 * @param gateway Gateway address
 * @return 0 on success, negative on error
 */
int network_configure_interface(network_interface_t* iface, uint32_t ip_addr,
                               uint32_t netmask, uint32_t gateway);

/**
 * Create network namespace
 * @param name Namespace name
 * @return Namespace pointer on success, NULL on error
 */
network_namespace_t* network_create_namespace(const char* name);

/**
 * Enable enterprise mode
 * @param enable true to enable, false to disable
 */
void network_enable_enterprise_mode(bool enable);

/**
 * Print network statistics
 */
void network_print_statistics(void);

/**
 * Network stack self-test
 * @return 0 on success, negative on error
 */
int network_self_test(void);

/**
 * Shutdown network stack
 */
void network_shutdown(void);

/* ============================================================================
 * QOS AND TRAFFIC SHAPING API
 * ============================================================================ */

/**
 * Configure QoS class
 * @param class_id QoS class identifier
 * @param priority Priority level (0-255)
 * @param bandwidth Guaranteed bandwidth in bps
 * @param max_bandwidth Maximum bandwidth in bps
 * @return 0 on success, negative on error
 */
int network_configure_qos_class(qos_class_t class_id, uint32_t priority,
                                uint64_t bandwidth, uint64_t max_bandwidth);

/**
 * Classify packet for QoS
 * @param packet Packet to classify
 * @return QoS class
 */
qos_class_t network_classify_packet(network_packet_t* packet);

/**
 * Apply traffic shaping
 * @param packet Packet to shape
 * @param qos_class QoS class configuration
 * @return true if packet allowed, false if dropped
 */
bool network_apply_traffic_shaping(network_packet_t* packet, qos_class_t qos_class);

/* ============================================================================
 * SECURITY FILTERING API
 * ============================================================================ */

/**
 * Add security filter rule
 * @param rule_name Name of the rule
 * @param src_ip Source IP address (0 for any)
 * @param dst_ip Destination IP address (0 for any)
 * @param protocol Protocol (0 for any)
 * @param action Action to take (0=drop, 1=allow)
 * @return 0 on success, negative on error
 */
int network_add_security_filter(const char* rule_name, uint32_t src_ip,
                               uint32_t dst_ip, uint8_t protocol, int action);

/**
 * Filter packet through security rules
 * @param packet Packet to filter
 * @return true if packet allowed, false if blocked
 */
bool network_security_filter_packet(network_packet_t* packet);

/* ============================================================================
 * NETWORK NAMESPACE API
 * ============================================================================ */

/**
 * Switch to network namespace
 * @param namespace Namespace to switch to
 * @return 0 on success, negative on error
 */
int network_switch_namespace(network_namespace_t* namespace);

/**
 * Find network namespace by name
 * @param name Namespace name
 * @return Namespace pointer on success, NULL if not found
 */
network_namespace_t* network_find_namespace(const char* name);

/**
 * Add interface to namespace
 * @param namespace Target namespace
 * @param interface Interface to add
 * @return 0 on success, negative on error
 */
int network_namespace_add_interface(network_namespace_t* namespace,
                                   network_interface_t* interface);

/* ============================================================================
 * SOCKET AND CONNECTION API
 * ============================================================================ */

/**
 * Create TCP connection
 * @param local_ip Local IP address
 * @param local_port Local port
 * @param remote_ip Remote IP address  
 * @param remote_port Remote port
 * @return Connection pointer on success, NULL on error
 */
tcp_connection_t* network_create_tcp_connection(uint32_t local_ip, uint16_t local_port,
                                               uint32_t remote_ip, uint16_t remote_port);

/**
 * Find TCP connection
 * @param local_ip Local IP address
 * @param local_port Local port
 * @param remote_ip Remote IP address
 * @param remote_port Remote port
 * @return Connection pointer if found, NULL otherwise
 */
tcp_connection_t* network_find_tcp_connection(uint32_t local_ip, uint16_t local_port,
                                             uint32_t remote_ip, uint16_t remote_port);

/**
 * Destroy TCP connection
 * @param connection Connection to destroy
 */
void network_destroy_tcp_connection(tcp_connection_t* connection);

/* ============================================================================
 * HIGH-PERFORMANCE PACKET PROCESSING
 * ============================================================================ */

/**
 * Allocate network packet
 * @param size Packet size
 * @return Packet pointer on success, NULL on error
 */
network_packet_t* network_allocate_packet(uint32_t size);

/**
 * Free network packet
 * @param packet Packet to free
 */
void network_free_packet(network_packet_t* packet);

/**
 * Process Ethernet frame
 * @param packet Packet containing Ethernet frame
 * @return 0 on success, negative on error
 */
int network_process_ethernet_frame(network_packet_t* packet);

/**
 * Process IPv4 packet
 * @param packet Packet containing IPv4 packet
 * @return 0 on success, negative on error
 */
int network_process_ipv4_packet(network_packet_t* packet);

/**
 * Process TCP segment
 * @param packet Packet containing TCP segment
 * @return 0 on success, negative on error
 */
int network_process_tcp_segment(network_packet_t* packet);

/**
 * Process UDP datagram
 * @param packet Packet containing UDP datagram
 * @return 0 on success, negative on error
 */
int network_process_udp_datagram(network_packet_t* packet);

/* ============================================================================
 * ROUTING AND ARP API
 * ============================================================================ */

/**
 * Add routing entry
 * @param destination Destination network
 * @param netmask Network mask
 * @param gateway Gateway address
 * @param interface Output interface
 * @param metric Route metric
 * @return 0 on success, negative on error
 */
int network_add_route(uint32_t destination, uint32_t netmask, uint32_t gateway,
                     network_interface_t* interface, uint32_t metric);

/**
 * Lookup route for destination
 * @param destination Destination IP address
 * @return Route information on success, NULL if no route found
 */
void* network_lookup_route(uint32_t destination);

/**
 * Add ARP entry
 * @param ip_addr IP address
 * @param mac_addr MAC address (6 bytes)
 * @return 0 on success, negative on error
 */
int network_add_arp_entry(uint32_t ip_addr, const uint8_t* mac_addr);

/**
 * Lookup ARP entry
 * @param ip_addr IP address to lookup
 * @param mac_addr Output buffer for MAC address (6 bytes)
 * @return 0 on success, negative if not found
 */
int network_lookup_arp_entry(uint32_t ip_addr, uint8_t* mac_addr);

/* ============================================================================
 * ENTERPRISE FEATURES
 * ============================================================================ */

/**
 * Enable network monitoring
 * @param enable true to enable, false to disable
 */
void network_enable_monitoring(bool enable);

/**
 * Set network security level
 * @param level Security level (0-10)
 * @return 0 on success, negative on error
 */
int network_set_security_level(uint32_t level);

/**
 * Enable network encryption
 * @param enable true to enable, false to disable
 * @return 0 on success, negative on error
 */
int network_enable_encryption(bool enable);

/**
 * Set tenant ID for multi-tenancy
 * @param interface Interface to configure
 * @param tenant_id Tenant identifier
 * @return 0 on success, negative on error
 */
int network_set_tenant_id(network_interface_t* interface, uint32_t tenant_id);

/**
 * Get network framework statistics
 * @param rx_packets Output for received packets count
 * @param tx_packets Output for transmitted packets count
 * @param rx_bytes Output for received bytes count
 * @param tx_bytes Output for transmitted bytes count
 * @return 0 on success, negative on error
 */
int network_get_statistics(uint64_t* rx_packets, uint64_t* tx_packets,
                          uint64_t* rx_bytes, uint64_t* tx_bytes);

#endif /* ADVANCED_NETWORK_H */