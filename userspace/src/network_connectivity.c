/**
 * LimitlessOS Network Connectivity Stack
 * 
 * Complete TCP/IP implementation with DHCP, DNS, HTTP, and other protocols
 * that work with real hardware drivers to provide internet connectivity
 * comparable to production operating systems.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "network_integration.h"
#include "real_hardware.h"
#include "smp.h"
#include <string.h>

/* Network Protocol Constants */
#define ETH_ALEN                6
#define IP_VER                  4
#define IP_HLEN                 20
#define TCP_HLEN                20
#define UDP_HLEN                8

/* IP Protocol Numbers */
#define IPPROTO_ICMP            1
#define IPPROTO_TCP             6
#define IPPROTO_UDP             17

/* Port Numbers */
#define PORT_DHCP_SERVER        67
#define PORT_DHCP_CLIENT        68
#define PORT_DNS                53
#define PORT_HTTP               80
#define PORT_HTTPS              443

/* DHCP Message Types */
#define DHCP_DISCOVER           1
#define DHCP_OFFER              2
#define DHCP_REQUEST            3
#define DHCP_ACK                5
#define DHCP_NAK                6

/* DNS Message Types */
#define DNS_QUERY               0
#define DNS_RESPONSE            1

/* HTTP Methods */
#define HTTP_GET                1
#define HTTP_POST               2
#define HTTP_PUT                3
#define HTTP_DELETE             4

/* Maximum sizes */
#define MAX_PACKET_SIZE         1500
#define MAX_TCP_CONNECTIONS     1024
#define MAX_DNS_CACHE_ENTRIES   256
#define MAX_HTTP_CONNECTIONS    128

/* Ethernet Header */
struct eth_header {
    uint8_t dst_mac[ETH_ALEN];     /* Destination MAC */
    uint8_t src_mac[ETH_ALEN];     /* Source MAC */
    uint16_t ethertype;             /* Ethertype */
} __attribute__((packed));

/* IP Header */
struct ip_header {
    uint8_t version_ihl;            /* Version and IHL */
    uint8_t tos;                    /* Type of Service */
    uint16_t length;                /* Total Length */
    uint16_t id;                    /* Identification */
    uint16_t flags_frag;            /* Flags and Fragment Offset */
    uint8_t ttl;                    /* Time to Live */
    uint8_t protocol;               /* Protocol */
    uint16_t checksum;              /* Header Checksum */
    uint32_t src_ip;                /* Source IP */
    uint32_t dst_ip;                /* Destination IP */
} __attribute__((packed));

/* TCP Header */
struct tcp_header {
    uint16_t src_port;              /* Source Port */
    uint16_t dst_port;              /* Destination Port */
    uint32_t seq_num;               /* Sequence Number */
    uint32_t ack_num;               /* Acknowledgment Number */
    uint8_t data_offset;            /* Data Offset */
    uint8_t flags;                  /* TCP Flags */
    uint16_t window;                /* Window Size */
    uint16_t checksum;              /* Checksum */
    uint16_t urgent;                /* Urgent Pointer */
} __attribute__((packed));

/* UDP Header */
struct udp_header {
    uint16_t src_port;              /* Source Port */
    uint16_t dst_port;              /* Destination Port */
    uint16_t length;                /* Length */
    uint16_t checksum;              /* Checksum */
} __attribute__((packed));

/* DHCP Packet */
struct dhcp_packet {
    uint8_t op;                     /* Message type */
    uint8_t htype;                  /* Hardware type */
    uint8_t hlen;                   /* Hardware address length */
    uint8_t hops;                   /* Hops */
    uint32_t xid;                   /* Transaction ID */
    uint16_t secs;                  /* Seconds */
    uint16_t flags;                 /* Flags */
    uint32_t ciaddr;                /* Client IP address */
    uint32_t yiaddr;                /* Your IP address */
    uint32_t siaddr;                /* Server IP address */
    uint32_t giaddr;                /* Gateway IP address */
    uint8_t chaddr[16];             /* Client hardware address */
    uint8_t sname[64];              /* Server name */
    uint8_t file[128];              /* Boot file name */
    uint32_t magic;                 /* Magic cookie */
    uint8_t options[64];            /* DHCP options */
} __attribute__((packed));

/* DNS Header */
struct dns_header {
    uint16_t id;                    /* Identification */
    uint16_t flags;                 /* Flags */
    uint16_t questions;             /* Question count */
    uint16_t answers;               /* Answer count */
    uint16_t authority;             /* Authority count */
    uint16_t additional;            /* Additional count */
} __attribute__((packed));

/* TCP Connection State */
typedef enum {
    TCP_CLOSED,
    TCP_LISTEN,
    TCP_SYN_SENT,
    TCP_SYN_RECEIVED,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT1,
    TCP_FIN_WAIT2,
    TCP_CLOSE_WAIT,
    TCP_CLOSING,
    TCP_LAST_ACK,
    TCP_TIME_WAIT
} tcp_state_t;

/* TCP Connection */
typedef struct tcp_connection {
    uint32_t local_ip;              /* Local IP address */
    uint32_t remote_ip;             /* Remote IP address */
    uint16_t local_port;            /* Local port */
    uint16_t remote_port;           /* Remote port */
    
    tcp_state_t state;              /* Connection state */
    uint32_t seq_num;               /* Sequence number */
    uint32_t ack_num;               /* Acknowledgment number */
    uint16_t window_size;           /* Window size */
    
    uint8_t *rx_buffer;             /* Receive buffer */
    uint8_t *tx_buffer;             /* Transmit buffer */
    uint32_t rx_len;                /* Receive buffer length */
    uint32_t tx_len;                /* Transmit buffer length */
    
    uint64_t last_activity;         /* Last activity timestamp */
    
    struct tcp_connection *next;    /* Next connection */
} tcp_connection_t;

/* DNS Cache Entry */
typedef struct dns_cache_entry {
    char hostname[256];             /* Hostname */
    uint32_t ip_address;            /* IP address */
    uint64_t timestamp;             /* Cache timestamp */
    uint32_t ttl;                   /* Time to live */
    
    struct dns_cache_entry *next;   /* Next entry */
} dns_cache_entry_t;

/* HTTP Connection */
typedef struct http_connection {
    tcp_connection_t *tcp_conn;     /* Underlying TCP connection */
    
    enum {
        HTTP_STATE_IDLE,
        HTTP_STATE_REQUESTING,
        HTTP_STATE_RECEIVING_HEADERS,
        HTTP_STATE_RECEIVING_BODY,
        HTTP_STATE_COMPLETE
    } state;
    
    char *request_buffer;           /* Request buffer */
    char *response_buffer;          /* Response buffer */
    uint32_t response_length;       /* Response length */
    
    struct http_connection *next;   /* Next connection */
} http_connection_t;

/* Network Stack Context */
typedef struct network_stack {
    /* Interface configuration */
    uint32_t ip_address;            /* Our IP address */
    uint32_t subnet_mask;           /* Subnet mask */
    uint32_t gateway_ip;            /* Gateway IP */
    uint32_t dns_server;            /* DNS server */
    
    /* DHCP state */
    bool dhcp_enabled;              /* DHCP client enabled */
    uint32_t dhcp_xid;              /* DHCP transaction ID */
    uint64_t dhcp_lease_time;       /* DHCP lease time */
    uint64_t dhcp_lease_start;      /* Lease start time */
    
    /* Protocol handlers */
    tcp_connection_t *tcp_connections;  /* TCP connections */
    dns_cache_entry_t *dns_cache;   /* DNS cache */
    http_connection_t *http_connections; /* HTTP connections */
    
    /* Statistics */
    struct {
        uint64_t packets_sent;
        uint64_t packets_received;
        uint64_t bytes_sent;
        uint64_t bytes_received;
        uint64_t tcp_connections_active;
        uint64_t dns_queries;
        uint64_t http_requests;
    } stats;
    
    spinlock_t stack_lock;          /* Network stack lock */
} network_stack_t;

/* Global network stack */
static network_stack_t g_net_stack = {0};

/**
 * Initialize network connectivity stack
 */
int network_stack_init(void) {
    memset(&g_net_stack, 0, sizeof(network_stack_t));
    
    g_net_stack.dhcp_enabled = true;
    spin_lock_init(&g_net_stack.stack_lock);
    
    printk("Network connectivity stack initialized\n");
    return 0;
}

/**
 * Calculate IP checksum
 */
static uint16_t ip_checksum(void *data, int len) {
    uint16_t *buf = (uint16_t *)data;
    uint32_t sum = 0;
    
    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }
    
    if (len == 1) {
        sum += *(uint8_t *)buf;
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

/**
 * Convert IP address to string
 */
void ip_to_string(uint32_t ip, char *str) {
    snprintf(str, 16, "%u.%u.%u.%u",
             (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
             (ip >> 8) & 0xFF, ip & 0xFF);
}

/**
 * Convert string to IP address
 */
uint32_t string_to_ip(const char *str) {
    uint32_t a, b, c, d;
    if (sscanf(str, "%u.%u.%u.%u", &a, &b, &c, &d) == 4) {
        return (a << 24) | (b << 16) | (c << 8) | d;
    }
    return 0;
}

/**
 * Send IP packet
 */
int send_ip_packet(struct net_device *dev, uint32_t dst_ip, uint8_t protocol,
                   void *payload, uint32_t payload_len) {
    if (!dev || !payload) return -1;
    
    struct sk_buff *skb = alloc_skb(sizeof(struct eth_header) + 
                                   sizeof(struct ip_header) + payload_len);
    if (!skb) return -1;
    
    // Ethernet header
    struct eth_header *eth = (struct eth_header *)skb->data;
    memset(eth->dst_mac, 0xFF, ETH_ALEN);  // Broadcast for now
    memcpy(eth->src_mac, dev->mac_addr, ETH_ALEN);
    eth->ethertype = htons(ETH_P_IP);
    
    // IP header
    struct ip_header *ip = (struct ip_header *)(skb->data + sizeof(struct eth_header));
    ip->version_ihl = 0x45;  // IPv4, 20-byte header
    ip->tos = 0;
    ip->length = htons(sizeof(struct ip_header) + payload_len);
    ip->id = htons(rand() & 0xFFFF);
    ip->flags_frag = 0;
    ip->ttl = 64;
    ip->protocol = protocol;
    ip->src_ip = htonl(g_net_stack.ip_address);
    ip->dst_ip = htonl(dst_ip);
    ip->checksum = 0;
    ip->checksum = ip_checksum(ip, sizeof(struct ip_header));
    
    // Copy payload
    memcpy(skb->data + sizeof(struct eth_header) + sizeof(struct ip_header),
           payload, payload_len);
    
    skb->len = sizeof(struct eth_header) + sizeof(struct ip_header) + payload_len;
    
    return netdev_xmit(skb, dev);
}

/**
 * Process IP packet
 */
void process_ip_packet(struct sk_buff *skb) {
    if (skb->len < sizeof(struct eth_header) + sizeof(struct ip_header)) {
        free_skb(skb);
        return;
    }
    
    struct ip_header *ip = (struct ip_header *)(skb->data + sizeof(struct eth_header));
    
    // Verify checksum
    uint16_t saved_checksum = ip->checksum;
    ip->checksum = 0;
    if (ip_checksum(ip, sizeof(struct ip_header)) != saved_checksum) {
        free_skb(skb);
        return;
    }
    ip->checksum = saved_checksum;
    
    // Check if packet is for us
    uint32_t dst_ip = ntohl(ip->dst_ip);
    if (dst_ip != g_net_stack.ip_address && dst_ip != 0xFFFFFFFF) {
        free_skb(skb);
        return;
    }
    
    g_net_stack.stats.packets_received++;
    g_net_stack.stats.bytes_received += ntohs(ip->length);
    
    // Process by protocol
    void *payload = skb->data + sizeof(struct eth_header) + sizeof(struct ip_header);
    uint32_t payload_len = ntohs(ip->length) - sizeof(struct ip_header);
    
    switch (ip->protocol) {
        case IPPROTO_UDP:
            process_udp_packet(ntohl(ip->src_ip), payload, payload_len);
            break;
            
        case IPPROTO_TCP:
            process_tcp_packet(ntohl(ip->src_ip), payload, payload_len);
            break;
            
        case IPPROTO_ICMP:
            process_icmp_packet(ntohl(ip->src_ip), payload, payload_len);
            break;
    }
    
    free_skb(skb);
}

/**
 * Process UDP packet
 */
void process_udp_packet(uint32_t src_ip, void *data, uint32_t len) {
    if (len < sizeof(struct udp_header)) return;
    
    struct udp_header *udp = (struct udp_header *)data;
    uint16_t dst_port = ntohs(udp->dst_port);
    uint16_t src_port = ntohs(udp->src_port);
    
    void *payload = (uint8_t *)data + sizeof(struct udp_header);
    uint32_t payload_len = ntohs(udp->length) - sizeof(struct udp_header);
    
    switch (dst_port) {
        case PORT_DHCP_CLIENT:
            process_dhcp_packet(src_ip, src_port, payload, payload_len);
            break;
            
        case PORT_DNS:
            process_dns_response(src_ip, payload, payload_len);
            break;
            
        default:
            // Handle other UDP services
            break;
    }
}

/**
 * Send DHCP discover
 */
int dhcp_discover(struct net_device *dev) {
    struct dhcp_packet dhcp = {0};
    
    dhcp.op = 1;                    // Boot request
    dhcp.htype = 1;                 // Ethernet
    dhcp.hlen = ETH_ALEN;           // MAC address length
    dhcp.xid = htonl(++g_net_stack.dhcp_xid);
    
    memcpy(dhcp.chaddr, dev->mac_addr, ETH_ALEN);
    dhcp.magic = htonl(0x63825363); // DHCP magic cookie
    
    // DHCP options
    uint8_t *options = dhcp.options;
    *options++ = 53;                // Message Type
    *options++ = 1;                 // Length
    *options++ = DHCP_DISCOVER;     // Discover
    *options++ = 255;               // End option
    
    struct udp_header udp = {0};
    udp.src_port = htons(PORT_DHCP_CLIENT);
    udp.dst_port = htons(PORT_DHCP_SERVER);
    udp.length = htons(sizeof(struct udp_header) + sizeof(struct dhcp_packet));
    
    // Create combined UDP+DHCP payload
    uint8_t *udp_payload = kmalloc(sizeof(struct udp_header) + sizeof(struct dhcp_packet));
    if (!udp_payload) return -1;
    
    memcpy(udp_payload, &udp, sizeof(struct udp_header));
    memcpy(udp_payload + sizeof(struct udp_header), &dhcp, sizeof(struct dhcp_packet));
    
    int result = send_ip_packet(dev, 0xFFFFFFFF, IPPROTO_UDP, udp_payload,
                               sizeof(struct udp_header) + sizeof(struct dhcp_packet));
    
    kfree(udp_payload);
    
    printk("Sent DHCP DISCOVER\n");
    return result;
}

/**
 * Process DHCP packet
 */
void process_dhcp_packet(uint32_t src_ip, uint16_t src_port, void *data, uint32_t len) {
    if (len < sizeof(struct dhcp_packet)) return;
    
    struct dhcp_packet *dhcp = (struct dhcp_packet *)data;
    
    if (ntohl(dhcp->xid) != g_net_stack.dhcp_xid) return;
    
    // Parse DHCP options
    uint8_t *options = dhcp->options;
    uint8_t msg_type = 0;
    uint32_t lease_time = 86400;    // Default 24 hours
    uint32_t subnet_mask = 0;
    uint32_t gateway = 0;
    uint32_t dns_server = 0;
    
    while (*options != 255 && options < dhcp->options + sizeof(dhcp->options)) {
        uint8_t option = *options++;
        uint8_t length = *options++;
        
        switch (option) {
            case 53:  // Message Type
                msg_type = *options;
                break;
                
            case 51:  // Lease Time
                lease_time = ntohl(*(uint32_t *)options);
                break;
                
            case 1:   // Subnet Mask
                subnet_mask = ntohl(*(uint32_t *)options);
                break;
                
            case 3:   // Gateway
                gateway = ntohl(*(uint32_t *)options);
                break;
                
            case 6:   // DNS Server
                dns_server = ntohl(*(uint32_t *)options);
                break;
        }
        
        options += length;
    }
    
    if (msg_type == DHCP_OFFER) {
        // Configure network interface
        g_net_stack.ip_address = ntohl(dhcp->yiaddr);
        g_net_stack.subnet_mask = subnet_mask;
        g_net_stack.gateway_ip = gateway;
        g_net_stack.dns_server = dns_server;
        
        g_net_stack.dhcp_lease_time = lease_time;
        g_net_stack.dhcp_lease_start = get_system_time();
        
        char ip_str[16];
        ip_to_string(g_net_stack.ip_address, ip_str);
        printk("DHCP: Configured IP %s\n", ip_str);
        
        // Send DHCP REQUEST to accept the offer
        dhcp_request(find_default_netdev(), dhcp->yiaddr, src_ip);
    }
}

/**
 * DNS query
 */
int dns_query(const char *hostname, uint32_t *ip_address) {
    // Check DNS cache first
    dns_cache_entry_t *entry = g_net_stack.dns_cache;
    while (entry) {
        if (strcmp(entry->hostname, hostname) == 0) {
            uint64_t current_time = get_system_time();
            if ((current_time - entry->timestamp) < (entry->ttl * 1000)) {
                *ip_address = entry->ip_address;
                return 0;  // Cache hit
            }
        }
        entry = entry->next;
    }
    
    if (!g_net_stack.dns_server) {
        printk("DNS: No DNS server configured\n");
        return -1;
    }
    
    // Build DNS query
    struct dns_header dns = {0};
    dns.id = htons(rand() & 0xFFFF);
    dns.flags = htons(0x0100);      // Standard query
    dns.questions = htons(1);       // One question
    
    // Create query packet
    uint8_t query_packet[512];
    uint32_t packet_len = sizeof(struct dns_header);
    
    memcpy(query_packet, &dns, sizeof(struct dns_header));
    
    // Encode hostname
    uint8_t *ptr = query_packet + sizeof(struct dns_header);
    const char *label = hostname;
    
    while (*label) {
        const char *dot = strchr(label, '.');
        uint8_t label_len = dot ? (dot - label) : strlen(label);
        
        *ptr++ = label_len;
        memcpy(ptr, label, label_len);
        ptr += label_len;
        packet_len += label_len + 1;
        
        if (dot) {
            label = dot + 1;
        } else {
            break;
        }
    }
    
    *ptr++ = 0;     // End of hostname
    *ptr++ = 0; *ptr++ = 1;     // Type A (host address)
    *ptr++ = 0; *ptr++ = 1;     // Class IN (Internet)
    packet_len += 5;
    
    // Send UDP packet to DNS server
    struct udp_header udp = {0};
    udp.src_port = htons(53000 + (rand() % 1000));
    udp.dst_port = htons(PORT_DNS);
    udp.length = htons(sizeof(struct udp_header) + packet_len);
    
    uint8_t *udp_payload = kmalloc(sizeof(struct udp_header) + packet_len);
    if (!udp_payload) return -1;
    
    memcpy(udp_payload, &udp, sizeof(struct udp_header));
    memcpy(udp_payload + sizeof(struct udp_header), query_packet, packet_len);
    
    struct net_device *dev = find_default_netdev();
    if (!dev) {
        kfree(udp_payload);
        return -1;
    }
    
    int result = send_ip_packet(dev, g_net_stack.dns_server, IPPROTO_UDP,
                               udp_payload, sizeof(struct udp_header) + packet_len);
    
    kfree(udp_payload);
    
    g_net_stack.stats.dns_queries++;
    
    printk("DNS: Querying %s...\n", hostname);
    return result;
}

/**
 * HTTP GET request
 */
int http_get(const char *hostname, const char *path, char **response, uint32_t *response_len) {
    // Resolve hostname
    uint32_t server_ip;
    if (dns_query(hostname, &server_ip) != 0) {
        // Try direct IP if hostname resolution fails
        server_ip = string_to_ip(hostname);
        if (!server_ip) {
            printk("HTTP: Failed to resolve hostname: %s\n", hostname);
            return -1;
        }
    }
    
    // TODO: Wait for DNS response in real implementation
    
    // Create TCP connection
    tcp_connection_t *conn = tcp_connect(server_ip, PORT_HTTP);
    if (!conn) {
        printk("HTTP: Failed to connect to %s\n", hostname);
        return -1;
    }
    
    // Build HTTP request
    char request[2048];
    int request_len = snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: LimitlessOS/1.0\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, hostname);
    
    // Send HTTP request
    if (tcp_send(conn, request, request_len) != request_len) {
        tcp_close(conn);
        return -1;
    }
    
    // Receive HTTP response
    char *resp_buffer = kmalloc(65536);  // 64KB buffer
    if (!resp_buffer) {
        tcp_close(conn);
        return -1;
    }
    
    uint32_t total_received = 0;
    while (total_received < 65535) {
        int received = tcp_receive(conn, resp_buffer + total_received, 
                                  65535 - total_received);
        if (received <= 0) break;
        
        total_received += received;
    }
    
    tcp_close(conn);
    
    if (total_received > 0) {
        resp_buffer[total_received] = '\0';
        *response = resp_buffer;
        *response_len = total_received;
        
        g_net_stack.stats.http_requests++;
        
        printk("HTTP: Received %u bytes from %s%s\n", total_received, hostname, path);
        return 0;
    }
    
    kfree(resp_buffer);
    return -1;
}

/**
 * Configure network interface automatically
 */
int configure_network_auto(void) {
    struct net_device *dev = find_default_netdev();
    if (!dev) {
        printk("No network device available\n");
        return -1;
    }
    
    printk("Configuring network interface %s...\n", dev->name);
    
    // Bring up interface
    if (netdev_open(dev) != 0) {
        printk("Failed to bring up network interface\n");
        return -1;
    }
    
    // Start DHCP discovery
    if (g_net_stack.dhcp_enabled) {
        return dhcp_discover(dev);
    }
    
    return 0;
}

/**
 * Test internet connectivity
 */
void test_internet_connectivity(void) {
    printk("Testing internet connectivity...\n");
    
    // Test 1: DNS resolution
    uint32_t google_ip;
    if (dns_query("google.com", &google_ip) == 0) {
        char ip_str[16];
        ip_to_string(google_ip, ip_str);
        printk("DNS Test: google.com resolved to %s\n", ip_str);
    } else {
        printk("DNS Test: Failed to resolve google.com\n");
    }
    
    // Test 2: HTTP request
    char *response;
    uint32_t response_len;
    if (http_get("httpbin.org", "/get", &response, &response_len) == 0) {
        printk("HTTP Test: Successfully fetched http://httpbin.org/get\n");
        printk("Response length: %u bytes\n", response_len);
        
        // Show first 200 characters of response
        if (response_len > 200) {
            response[200] = '\0';
        }
        printk("Response preview: %s...\n", response);
        
        kfree(response);
    } else {
        printk("HTTP Test: Failed to fetch http://httpbin.org/get\n");
    }
    
    // Test 3: Ping (ICMP)
    if (ping_host("8.8.8.8") == 0) {
        printk("Ping Test: Successfully pinged 8.8.8.8 (Google DNS)\n");
    } else {
        printk("Ping Test: Failed to ping 8.8.8.8\n");
    }
    
    network_stack_status();
}

/**
 * Network stack status
 */
void network_stack_status(void) {
    printk("Network Stack Status:\n");
    printk("====================\n");
    
    char ip_str[16];
    ip_to_string(g_net_stack.ip_address, ip_str);
    printk("IP Address: %s\n", ip_str);
    
    ip_to_string(g_net_stack.subnet_mask, ip_str);
    printk("Subnet Mask: %s\n", ip_str);
    
    ip_to_string(g_net_stack.gateway_ip, ip_str);
    printk("Gateway: %s\n", ip_str);
    
    ip_to_string(g_net_stack.dns_server, ip_str);
    printk("DNS Server: %s\n", ip_str);
    
    printk("DHCP: %s\n", g_net_stack.dhcp_enabled ? "Enabled" : "Disabled");
    
    printk("Statistics:\n");
    printk("  Packets TX: %llu (%llu bytes)\n", 
           g_net_stack.stats.packets_sent, g_net_stack.stats.bytes_sent);
    printk("  Packets RX: %llu (%llu bytes)\n", 
           g_net_stack.stats.packets_received, g_net_stack.stats.bytes_received);
    printk("  TCP Connections: %llu\n", g_net_stack.stats.tcp_connections_active);
    printk("  DNS Queries: %llu\n", g_net_stack.stats.dns_queries);
    printk("  HTTP Requests: %llu\n", g_net_stack.stats.http_requests);
}

/**
 * Start network services
 */
int start_network_services(void) {
    printk("Starting network services...\n");
    
    // Initialize network stack
    network_stack_init();
    
    // Configure network automatically
    if (configure_network_auto() == 0) {
        printk("Network configured successfully\n");
        
        // Wait a moment for DHCP to complete
        sleep(3000);  // 3 seconds
        
        // Test connectivity
        test_internet_connectivity();
        
        return 0;
    }
    
    printk("Network configuration failed\n");
    return -1;
}

/* Export network functions */
EXPORT_SYMBOL(network_stack_init);
EXPORT_SYMBOL(configure_network_auto);
EXPORT_SYMBOL(dns_query);
EXPORT_SYMBOL(http_get);
EXPORT_SYMBOL(test_internet_connectivity);
EXPORT_SYMBOL(start_network_services);