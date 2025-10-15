/*
 * LimitlessOS Advanced Networking & Communication
 * Next-generation networking stack with IPv6, QoS, SDN, and wireless protocols
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sys/socket.h>

/* Network Protocol Support */
typedef enum {
    NET_PROTOCOL_IPV4 = 0,
    NET_PROTOCOL_IPV6,
    NET_PROTOCOL_ICMP,
    NET_PROTOCOL_ICMPV6,
    NET_PROTOCOL_TCP,
    NET_PROTOCOL_UDP,
    NET_PROTOCOL_SCTP,
    NET_PROTOCOL_QUIC,
    NET_PROTOCOL_HTTP2,
    NET_PROTOCOL_HTTP3,
    NET_PROTOCOL_WEBSOCKET,
    NET_PROTOCOL_MAX
} network_protocol_t;

/* Wireless Standards */
typedef enum {
    WIRELESS_NONE = 0,
    WIRELESS_802_11A,
    WIRELESS_802_11B,
    WIRELESS_802_11G,
    WIRELESS_802_11N,
    WIRELESS_802_11AC,
    WIRELESS_802_11AX,      /* Wi-Fi 6 */
    WIRELESS_802_11BE,      /* Wi-Fi 7 */
    WIRELESS_BLUETOOTH_4,
    WIRELESS_BLUETOOTH_5,
    WIRELESS_BLUETOOTH_LE,
    WIRELESS_ZIGBEE,
    WIRELESS_THREAD,
    WIRELESS_MATTER,
    WIRELESS_5G_NR,
    WIRELESS_LTE_CAT_M,
    WIRELESS_LORA,
    WIRELESS_MAX
} wireless_standard_t;

/* QoS Traffic Classes */
typedef enum {
    QOS_CLASS_BEST_EFFORT = 0,
    QOS_CLASS_BACKGROUND,
    QOS_CLASS_EXCELLENT_EFFORT,
    QOS_CLASS_CRITICAL_APPLICATION,
    QOS_CLASS_VIDEO,
    QOS_CLASS_VOICE,
    QOS_CLASS_INTERACTIVE_MULTIMEDIA,
    QOS_CLASS_MISSION_CRITICAL,
    QOS_CLASS_NETWORK_CONTROL,
    QOS_CLASS_MAX
} qos_traffic_class_t;

/* VPN Types */
typedef enum {
    VPN_TYPE_OPENVPN = 0,
    VPN_TYPE_WIREGUARD,
    VPN_TYPE_IPSEC,
    VPN_TYPE_L2TP,
    VPN_TYPE_PPTP,
    VPN_TYPE_SSTP,
    VPN_TYPE_SOFTETHER,
    VPN_TYPE_MAX
} vpn_type_t;

/* Network Interface Types */
typedef enum {
    INTERFACE_TYPE_ETHERNET = 0,
    INTERFACE_TYPE_WIRELESS,
    INTERFACE_TYPE_BLUETOOTH,
    INTERFACE_TYPE_CELLULAR,
    INTERFACE_TYPE_LOOPBACK,
    INTERFACE_TYPE_BRIDGE,
    INTERFACE_TYPE_BOND,
    INTERFACE_TYPE_VLAN,
    INTERFACE_TYPE_TUNNEL,
    INTERFACE_TYPE_VPN,
    INTERFACE_TYPE_MAX
} interface_type_t;

/* SDN Controller Types */
typedef enum {
    SDN_CONTROLLER_OPENFLOW = 0,
    SDN_CONTROLLER_P4,
    SDN_CONTROLLER_ONOS,
    SDN_CONTROLLER_OPENDAYLIGHT,
    SDN_CONTROLLER_FLOODLIGHT,
    SDN_CONTROLLER_MAX
} sdn_controller_t;

/* Network Address Structure */
typedef struct network_address {
    sa_family_t family;         /* AF_INET or AF_INET6 */
    union {
        struct in_addr ipv4;    /* IPv4 address */
        struct in6_addr ipv6;   /* IPv6 address */
    } addr;
    uint16_t port;              /* Port number */
    uint32_t scope_id;          /* IPv6 scope ID */
} network_address_t;

/* Quality of Service Configuration */
typedef struct qos_config {
    bool enabled;               /* QoS enabled */
    
    /* Traffic shaping */
    struct {
        uint64_t max_bandwidth; /* Maximum bandwidth bps */
        uint32_t burst_size;    /* Burst size bytes */
        uint32_t latency_target; /* Target latency ms */
        float packet_loss_target; /* Target packet loss % */
    } shaping;
    
    /* Traffic classes */
    struct {
        struct {
            qos_traffic_class_t class;
            uint32_t min_bandwidth; /* Minimum guaranteed bandwidth */
            uint32_t max_bandwidth; /* Maximum bandwidth */
            uint32_t priority;      /* Priority level (0-7) */
            uint32_t max_latency;   /* Maximum latency ms */
            float max_jitter;       /* Maximum jitter ms */
            float max_loss;         /* Maximum packet loss % */
        } classes[QOS_CLASS_MAX];
        int class_count;
    } traffic_classes;
    
    /* Packet marking */
    struct {
        bool dscp_marking;      /* DSCP marking enabled */
        bool cos_marking;       /* CoS marking enabled */
        uint8_t default_dscp;   /* Default DSCP value */
        uint8_t default_cos;    /* Default CoS value */
    } marking;
    
    /* Congestion control */
    struct {
        enum {
            CONGESTION_RED = 0, /* Random Early Detection */
            CONGESTION_WRED,    /* Weighted RED */
            CONGESTION_BLUE,    /* BLUE algorithm */
            CONGESTION_PIE,     /* Proportional Integral Enhanced */
            CONGESTION_CODEL,   /* Controlled Delay */
            CONGESTION_FQ_CODEL /* Fair Queuing CoDel */
        } algorithm;
        
        uint32_t min_threshold; /* Minimum queue threshold */
        uint32_t max_threshold; /* Maximum queue threshold */
        float drop_probability; /* Drop probability */
    } congestion_control;
    
} qos_config_t;

/* Network Interface Configuration */
typedef struct network_interface {
    char name[16];              /* Interface name (eth0, wlan0, etc.) */
    char description[128];      /* Interface description */
    interface_type_t type;      /* Interface type */
    
    /* Hardware information */
    struct {
        uint8_t mac_address[6]; /* MAC address */
        uint32_t mtu;           /* Maximum Transmission Unit */
        uint64_t speed;         /* Link speed in bps */
        bool full_duplex;       /* Full duplex support */
        bool auto_negotiation;  /* Auto-negotiation enabled */
    } hardware;
    
    /* Network configuration */
    struct {
        bool dhcp_enabled;      /* DHCP client enabled */
        network_address_t ipv4_address; /* IPv4 address */
        network_address_t ipv4_netmask; /* IPv4 netmask */
        network_address_t ipv4_gateway; /* IPv4 gateway */
        
        network_address_t ipv6_addresses[8]; /* IPv6 addresses */
        int ipv6_address_count; /* Number of IPv6 addresses */
        network_address_t ipv6_gateway; /* IPv6 gateway */
        
        network_address_t dns_servers[8]; /* DNS servers */
        int dns_server_count;   /* Number of DNS servers */
        
        char domain_name[256];  /* Domain name */
        char hostname[256];     /* Hostname */
    } config;
    
    /* State information */
    struct {
        bool up;                /* Interface is up */
        bool connected;         /* Link is connected */
        bool carrier;           /* Carrier detected */
        uint64_t rx_packets;    /* Received packets */
        uint64_t tx_packets;    /* Transmitted packets */
        uint64_t rx_bytes;      /* Received bytes */
        uint64_t tx_bytes;      /* Transmitted bytes */
        uint64_t rx_errors;     /* Receive errors */
        uint64_t tx_errors;     /* Transmit errors */
        uint64_t rx_dropped;    /* Dropped receive packets */
        uint64_t tx_dropped;    /* Dropped transmit packets */
    } state;
    
    /* Quality of Service */
    qos_config_t qos;
    
    /* Wireless configuration (if applicable) */
    struct {
        wireless_standard_t standard; /* Wireless standard */
        char ssid[33];          /* SSID */
        char bssid[18];         /* BSSID */
        uint32_t frequency;     /* Operating frequency MHz */
        uint32_t channel;       /* Wireless channel */
        int32_t signal_strength; /* Signal strength dBm */
        int32_t noise_level;    /* Noise level dBm */
        float signal_quality;   /* Signal quality % */
        
        /* Security */
        enum {
            WIRELESS_SECURITY_NONE = 0,
            WIRELESS_SECURITY_WEP,
            WIRELESS_SECURITY_WPA,
            WIRELESS_SECURITY_WPA2,
            WIRELESS_SECURITY_WPA3,
            WIRELESS_SECURITY_WPA_ENTERPRISE,
            WIRELESS_SECURITY_OWE,  /* Opportunistic Wireless Encryption */
            WIRELESS_SECURITY_SAE   /* Simultaneous Authentication of Equals */
        } security_type;
        
        char passphrase[256];   /* Network passphrase */
        uint8_t pmk[32];        /* Pairwise Master Key */
        
        /* Advanced features */
        bool wps_enabled;       /* Wi-Fi Protected Setup */
        bool wps_pbc;           /* Push Button Configuration */
        bool mesh_enabled;      /* Mesh networking */
        bool roaming_enabled;   /* Fast roaming */
        
    } wireless;
    
    /* Cellular configuration (if applicable) */
    struct {
        enum {
            CELLULAR_2G = 0,
            CELLULAR_3G,
            CELLULAR_4G_LTE,
            CELLULAR_5G_NR,
            CELLULAR_5G_MMWAVE
        } technology;
        
        char carrier[64];       /* Carrier name */
        char imei[16];          /* IMEI number */
        char imsi[16];          /* IMSI number */
        char iccid[20];         /* SIM card ICCID */
        
        struct {
            char apn[64];       /* Access Point Name */
            char username[64];  /* APN username */
            char password[64];  /* APN password */
        } apn_config;
        
        struct {
            int32_t rssi;       /* Received Signal Strength */
            int32_t rsrp;       /* Reference Signal Received Power */
            int32_t rsrq;       /* Reference Signal Received Quality */
            int32_t sinr;       /* Signal to Interference plus Noise Ratio */
            uint32_t cell_id;   /* Cell ID */
            uint32_t tracking_area; /* Tracking Area Code */
        } signal_info;
        
    } cellular;
    
} network_interface_t;

/* Firewall Rule */
typedef struct firewall_rule {
    uint32_t id;                /* Rule ID */
    char name[128];             /* Rule name */
    bool enabled;               /* Rule enabled */
    
    /* Rule action */
    enum {
        FIREWALL_ALLOW = 0,
        FIREWALL_DENY,
        FIREWALL_REJECT,
        FIREWALL_DROP,
        FIREWALL_LOG,
        FIREWALL_REDIRECT
    } action;
    
    /* Traffic direction */
    enum {
        TRAFFIC_INBOUND = 0,
        TRAFFIC_OUTBOUND,
        TRAFFIC_FORWARD,
        TRAFFIC_ANY
    } direction;
    
    /* Protocol matching */
    network_protocol_t protocol; /* Protocol to match */
    
    /* Address matching */
    struct {
        network_address_t src_address; /* Source address */
        network_address_t src_mask;    /* Source netmask */
        uint16_t src_port_min;         /* Source port range min */
        uint16_t src_port_max;         /* Source port range max */
        
        network_address_t dst_address; /* Destination address */
        network_address_t dst_mask;    /* Destination netmask */
        uint16_t dst_port_min;         /* Destination port range min */
        uint16_t dst_port_max;         /* Destination port range max */
    } match;
    
    /* Advanced matching */
    struct {
        char interface_in[16];  /* Input interface */
        char interface_out[16]; /* Output interface */
        uint8_t dscp_value;     /* DSCP value */
        uint32_t packet_size_min; /* Minimum packet size */
        uint32_t packet_size_max; /* Maximum packet size */
        bool established;       /* Match established connections */
        bool related;           /* Match related connections */
    } advanced;
    
    /* Statistics */
    struct {
        uint64_t packets_matched; /* Packets matched */
        uint64_t bytes_matched;   /* Bytes matched */
        uint64_t last_match;      /* Last match timestamp */
    } stats;
    
} firewall_rule_t;

/* VPN Configuration */
typedef struct vpn_config {
    char name[128];             /* VPN connection name */
    vpn_type_t type;            /* VPN type */
    bool enabled;               /* VPN enabled */
    bool auto_connect;          /* Auto-connect on startup */
    
    /* Server configuration */
    struct {
        char server_address[256]; /* Server address/hostname */
        uint16_t server_port;     /* Server port */
        network_protocol_t protocol; /* Protocol (TCP/UDP) */
    } server;
    
    /* Authentication */
    struct {
        enum {
            VPN_AUTH_PASSWORD = 0,
            VPN_AUTH_CERTIFICATE,
            VPN_AUTH_PSK,       /* Pre-shared key */
            VPN_AUTH_TOKEN,
            VPN_AUTH_BIOMETRIC
        } method;
        
        char username[128];     /* Username */
        char password[256];     /* Password */
        char certificate_file[512]; /* Certificate file path */
        char private_key_file[512]; /* Private key file path */
        char ca_certificate_file[512]; /* CA certificate file */
        char psk[256];          /* Pre-shared key */
    } auth;
    
    /* Encryption */
    struct {
        enum {
            VPN_CIPHER_AES128 = 0,
            VPN_CIPHER_AES256,
            VPN_CIPHER_CHACHA20,
            VPN_CIPHER_BLOWFISH
        } cipher;
        
        enum {
            VPN_HASH_SHA1 = 0,
            VPN_HASH_SHA256,
            VPN_HASH_SHA512,
            VPN_BLAKE2B
        } hash;
        
        bool compression;       /* Data compression */
        bool forward_secrecy;   /* Perfect Forward Secrecy */
    } encryption;
    
    /* Routing */
    struct {
        bool redirect_gateway;  /* Redirect default gateway */
        bool route_all_traffic; /* Route all traffic through VPN */
        
        struct {
            network_address_t network; /* Network address */
            network_address_t netmask; /* Network mask */
        } routes[32];
        int route_count;        /* Number of routes */
        
        network_address_t dns_servers[4]; /* VPN DNS servers */
        int dns_server_count;   /* Number of DNS servers */
    } routing;
    
    /* Connection state */
    struct {
        enum {
            VPN_STATE_DISCONNECTED = 0,
            VPN_STATE_CONNECTING,
            VPN_STATE_CONNECTED,
            VPN_STATE_DISCONNECTING,
            VPN_STATE_ERROR
        } state;
        
        uint64_t connect_time;  /* Connection timestamp */
        uint64_t duration;      /* Connection duration */
        network_address_t local_ip; /* Assigned local IP */
        network_address_t remote_ip; /* Remote gateway IP */
        
        uint64_t bytes_sent;    /* Bytes sent */
        uint64_t bytes_received; /* Bytes received */
        
        char error_message[256]; /* Last error message */
    } connection;
    
} vpn_config_t;

/* SDN Flow Entry */
typedef struct sdn_flow_entry {
    uint64_t flow_id;           /* Flow identifier */
    uint32_t table_id;          /* Flow table ID */
    uint16_t priority;          /* Flow priority */
    uint32_t timeout_idle;      /* Idle timeout */
    uint32_t timeout_hard;      /* Hard timeout */
    
    /* Match fields */
    struct {
        uint16_t in_port;       /* Input port */
        uint8_t eth_src[6];     /* Ethernet source */
        uint8_t eth_dst[6];     /* Ethernet destination */
        uint16_t eth_type;      /* Ethernet type */
        uint16_t vlan_vid;      /* VLAN ID */
        uint8_t vlan_pcp;       /* VLAN priority */
        uint32_t ipv4_src;      /* IPv4 source */
        uint32_t ipv4_dst;      /* IPv4 destination */
        uint8_t ip_proto;       /* IP protocol */
        uint8_t ip_dscp;        /* IP DSCP */
        uint16_t tcp_src;       /* TCP source port */
        uint16_t tcp_dst;       /* TCP destination port */
        uint16_t udp_src;       /* UDP source port */
        uint16_t udp_dst;       /* UDP destination port */
    } match;
    
    /* Actions */
    struct {
        enum {
            SDN_ACTION_OUTPUT = 0,
            SDN_ACTION_DROP,
            SDN_ACTION_FLOOD,
            SDN_ACTION_FORWARD,
            SDN_ACTION_CONTROLLER,
            SDN_ACTION_SET_VLAN,
            SDN_ACTION_SET_QUEUE,
            SDN_ACTION_REWRITE
        } actions[16];
        uint32_t action_data[16]; /* Action-specific data */
        int action_count;       /* Number of actions */
    } actions;
    
    /* Statistics */
    struct {
        uint64_t packet_count;  /* Packets matched */
        uint64_t byte_count;    /* Bytes matched */
        uint64_t duration_sec;  /* Flow duration */
        uint64_t duration_nsec; /* Flow duration (nanoseconds) */
    } stats;
    
} sdn_flow_entry_t;

/* Network Monitoring */
typedef struct network_monitor {
    bool enabled;               /* Monitoring enabled */
    
    /* Performance metrics */
    struct {
        uint64_t total_packets_rx; /* Total packets received */
        uint64_t total_packets_tx; /* Total packets transmitted */
        uint64_t total_bytes_rx;   /* Total bytes received */
        uint64_t total_bytes_tx;   /* Total bytes transmitted */
        
        uint32_t current_connections; /* Current connections */
        uint32_t peak_connections;    /* Peak connections */
        
        float avg_latency_ms;      /* Average latency */
        float packet_loss_rate;    /* Packet loss rate */
        float throughput_mbps;     /* Current throughput */
        
        struct {
            uint64_t tcp_connections;   /* TCP connections */
            uint64_t udp_packets;       /* UDP packets */
            uint64_t icmp_packets;      /* ICMP packets */
            uint64_t dns_queries;       /* DNS queries */
            uint64_t http_requests;     /* HTTP requests */
            uint64_t https_requests;    /* HTTPS requests */
        } protocol_stats;
        
    } metrics;
    
    /* Traffic analysis */
    struct {
        struct {
            network_address_t address; /* Host address */
            uint64_t bytes_sent;       /* Bytes sent */
            uint64_t bytes_received;   /* Bytes received */
            uint32_t connections;      /* Active connections */
        } top_talkers[100];
        int top_talker_count;
        
        struct {
            uint16_t port;          /* Port number */
            network_protocol_t protocol; /* Protocol */
            uint64_t packet_count;  /* Packet count */
            uint64_t byte_count;    /* Byte count */
        } top_protocols[50];
        int top_protocol_count;
        
    } analysis;
    
    /* Security monitoring */
    struct {
        struct {
            enum {
                SECURITY_EVENT_PORT_SCAN = 0,
                SECURITY_EVENT_DOS_ATTACK,
                SECURITY_EVENT_BRUTE_FORCE,
                SECURITY_EVENT_MALWARE,
                SECURITY_EVENT_INTRUSION,
                SECURITY_EVENT_DATA_EXFILTRATION
            } type;
            
            network_address_t source; /* Source address */
            network_address_t target; /* Target address */
            uint64_t timestamp;       /* Event timestamp */
            char description[256];    /* Event description */
            uint32_t severity;        /* Severity level (0-10) */
        } security_events[1000];
        int security_event_count;
        
        uint64_t blocked_connections; /* Blocked connections */
        uint64_t suspicious_packets;  /* Suspicious packets */
        
    } security;
    
} network_monitor_t;

/* Advanced Networking System */
typedef struct advanced_network_system {
    bool initialized;
    
    /* Network interfaces */
    struct {
        network_interface_t interfaces[32];
        int interface_count;
        
        char default_interface[16]; /* Default interface name */
        network_address_t default_gateway_v4; /* Default IPv4 gateway */
        network_address_t default_gateway_v6; /* Default IPv6 gateway */
    } interfaces;
    
    /* Routing table */
    struct {
        struct {
            network_address_t destination; /* Destination network */
            network_address_t netmask;     /* Network mask */
            network_address_t gateway;     /* Gateway address */
            char interface[16];            /* Output interface */
            uint32_t metric;               /* Route metric */
            uint32_t flags;                /* Route flags */
        } routes[1024];
        int route_count;
    } routing;
    
    /* Firewall */
    struct {
        bool enabled;               /* Firewall enabled */
        enum {
            FIREWALL_POLICY_ALLOW = 0,
            FIREWALL_POLICY_DENY
        } default_policy;
        
        firewall_rule_t rules[4096];
        int rule_count;
        
        /* Connection tracking */
        struct {
            bool enabled;           /* Connection tracking enabled */
            uint32_t max_connections; /* Maximum tracked connections */
            uint32_t timeout_tcp;   /* TCP timeout */
            uint32_t timeout_udp;   /* UDP timeout */
        } connection_tracking;
        
    } firewall;
    
    /* VPN connections */
    struct {
        vpn_config_t connections[16];
        int connection_count;
        
        char active_connection[128]; /* Active VPN connection */
        bool kill_switch;            /* VPN kill switch */
        bool auto_reconnect;         /* Auto-reconnect */
    } vpn;
    
    /* Software Defined Networking */
    struct {
        bool enabled;               /* SDN enabled */
        sdn_controller_t controller; /* SDN controller type */
        
        char controller_address[256]; /* Controller address */
        uint16_t controller_port;     /* Controller port */
        bool secure_channel;          /* Secure channel to controller */
        
        sdn_flow_entry_t flow_table[8192];
        int flow_count;
        
        /* OpenFlow configuration */
        struct {
            uint8_t version;        /* OpenFlow version */
            uint64_t datapath_id;   /* Datapath ID */
            uint32_t buffer_size;   /* Buffer size */
            uint16_t capabilities;  /* Switch capabilities */
        } openflow;
        
    } sdn;
    
    /* Network monitoring */
    network_monitor_t monitoring;
    
    /* Advanced features */
    struct {
        /* Network Function Virtualization */
        struct {
            bool enabled;           /* NFV enabled */
            char orchestrator[64];  /* NFV orchestrator */
        } nfv;
        
        /* Segment Routing */
        struct {
            bool enabled;           /* Segment routing enabled */
            bool ipv6_sr;          /* IPv6 segment routing */
            bool mpls_sr;          /* MPLS segment routing */
        } segment_routing;
        
        /* MPLS */
        struct {
            bool enabled;           /* MPLS enabled */
            uint32_t label_space;   /* MPLS label space */
        } mpls;
        
        /* Network slicing */
        struct {
            bool enabled;           /* Network slicing enabled */
            struct {
                char name[64];      /* Slice name */
                uint32_t slice_id;  /* Slice identifier */
                qos_config_t qos;   /* Slice QoS */
            } slices[16];
            int slice_count;
        } network_slicing;
        
    } advanced;
    
    /* Configuration */
    struct {
        bool ipv6_enabled;          /* IPv6 support enabled */
        bool multicast_enabled;     /* Multicast support enabled */
        bool zero_conf_enabled;     /* Zero-configuration networking */
        
        uint32_t tcp_window_size;   /* TCP window size */
        uint32_t udp_buffer_size;   /* UDP buffer size */
        
        /* DNS configuration */
        struct {
            bool dns_over_https;    /* DNS over HTTPS */
            bool dns_over_tls;      /* DNS over TLS */
            char dns_servers[8][256]; /* DNS servers */
            int dns_server_count;
        } dns;
        
        /* Time synchronization */
        struct {
            bool ntp_enabled;       /* NTP enabled */
            char ntp_servers[4][256]; /* NTP servers */
            int ntp_server_count;
        } time_sync;
        
    } config;
    
} advanced_network_system_t;

/* Global networking system */
extern advanced_network_system_t network_system;

/* Core networking functions */
int network_init(void);
void network_exit(void);
int network_start_services(void);
int network_stop_services(void);

/* Interface management */
int network_interface_enumerate(void);
network_interface_t *network_interface_find(const char *name);
int network_interface_up(const char *name);
int network_interface_down(const char *name);
int network_interface_configure(const char *name, const struct sockaddr *addr, const struct sockaddr *netmask);
int network_interface_set_mtu(const char *name, uint32_t mtu);
int network_interface_get_stats(const char *name, void *stats);

/* IPv4/IPv6 configuration */
int network_set_ipv4_address(const char *interface, const char *address, const char *netmask);
int network_set_ipv6_address(const char *interface, const char *address, int prefix_length);
int network_add_default_route(const char *gateway, const char *interface);
int network_del_default_route(const char *gateway, const char *interface);
int network_add_static_route(const char *destination, const char *netmask, const char *gateway, const char *interface);
int network_del_static_route(const char *destination, const char *netmask, const char *gateway);

/* DNS configuration */
int network_set_dns_servers(const char *servers[], int count);
int network_set_domain_name(const char *domain);
int network_set_hostname(const char *hostname);
int network_resolve_hostname(const char *hostname, struct sockaddr *addr);
int network_reverse_resolve(const struct sockaddr *addr, char *hostname, size_t size);

/* DHCP client */
int network_dhcp_start(const char *interface);
int network_dhcp_stop(const char *interface);
int network_dhcp_renew(const char *interface);
int network_dhcp_release(const char *interface);
int network_dhcp_get_lease_info(const char *interface, void *lease_info);

/* Wireless networking */
int wireless_scan_networks(const char *interface, void *scan_results, int max_results);
int wireless_connect_network(const char *interface, const char *ssid, const char *passphrase);
int wireless_disconnect_network(const char *interface);
int wireless_get_signal_info(const char *interface, void *signal_info);
int wireless_set_power_management(const char *interface, bool enabled);
int wireless_create_hotspot(const char *interface, const char *ssid, const char *passphrase);

/* Cellular networking */
int cellular_connect(const char *interface, const char *apn, const char *username, const char *password);
int cellular_disconnect(const char *interface);
int cellular_get_signal_strength(const char *interface, int32_t *rssi);
int cellular_get_network_info(const char *interface, void *network_info);
int cellular_set_preferred_network(const char *interface, const char *carrier);

/* Quality of Service */
int qos_init(void);
int qos_enable_interface(const char *interface);
int qos_disable_interface(const char *interface);
int qos_set_bandwidth_limit(const char *interface, uint64_t upload_bps, uint64_t download_bps);
int qos_add_traffic_rule(const char *interface, qos_traffic_class_t class, const char *rule);
int qos_remove_traffic_rule(const char *interface, uint32_t rule_id);
int qos_get_statistics(const char *interface, void *stats);

/* Firewall management */
int firewall_init(void);
int firewall_enable(void);
int firewall_disable(void);
int firewall_add_rule(const firewall_rule_t *rule);
int firewall_remove_rule(uint32_t rule_id);
int firewall_modify_rule(uint32_t rule_id, const firewall_rule_t *new_rule);
int firewall_get_rules(firewall_rule_t *rules, int max_rules);
int firewall_get_statistics(void *stats);
int firewall_reset_counters(void);

/* VPN management */
int vpn_init(void);
int vpn_add_connection(const vpn_config_t *config);
int vpn_remove_connection(const char *name);
int vpn_connect(const char *name);
int vpn_disconnect(const char *name);
int vpn_get_status(const char *name, void *status);
int vpn_get_connections(char connections[][128], int max_connections);

/* Software Defined Networking */
int sdn_init(void);
int sdn_set_controller(sdn_controller_t controller, const char *address, uint16_t port);
int sdn_add_flow_entry(const sdn_flow_entry_t *flow);
int sdn_remove_flow_entry(uint64_t flow_id);
int sdn_modify_flow_entry(uint64_t flow_id, const sdn_flow_entry_t *new_flow);
int sdn_get_flow_table(sdn_flow_entry_t *flows, int max_flows);
int sdn_send_packet_in(const void *packet_data, size_t packet_size);

/* Network monitoring */
int network_monitor_init(void);
int network_monitor_start(void);
int network_monitor_stop(void);
int network_monitor_get_metrics(void *metrics);
int network_monitor_get_top_talkers(void *top_talkers, int max_count);
int network_monitor_get_protocol_stats(void *protocol_stats);
int network_monitor_get_security_events(void *events, int max_events);

/* Traffic shaping */
int traffic_shaper_init(void);
int traffic_shaper_add_class(const char *interface, qos_traffic_class_t class, uint32_t rate, uint32_t ceil);
int traffic_shaper_remove_class(const char *interface, qos_traffic_class_t class);
int traffic_shaper_add_filter(const char *interface, const char *filter_spec, qos_traffic_class_t class);
int traffic_shaper_remove_filter(const char *interface, uint32_t filter_id);

/* Load balancing */
int load_balancer_init(void);
int load_balancer_add_backend(const char *name, const char *address, uint16_t port, uint32_t weight);
int load_balancer_remove_backend(const char *name, const char *address, uint16_t port);
int load_balancer_set_algorithm(const char *name, const char *algorithm);
int load_balancer_get_statistics(const char *name, void *stats);

/* Network security */
int network_security_init(void);
int network_security_enable_intrusion_detection(void);
int network_security_disable_intrusion_detection(void);
int network_security_add_blacklist_ip(const char *ip_address);
int network_security_remove_blacklist_ip(const char *ip_address);
int network_security_scan_vulnerabilities(void);
int network_security_get_threat_report(void *report);

/* Performance optimization */
int network_optimize_tcp_stack(void);
int network_enable_jumbo_frames(const char *interface);
int network_disable_jumbo_frames(const char *interface);
int network_set_interrupt_coalescing(const char *interface, uint32_t usecs, uint32_t frames);
int network_enable_gro(const char *interface);
int network_disable_gro(const char *interface);

/* Utility functions */
const char *network_protocol_name(network_protocol_t protocol);
const char *interface_type_name(interface_type_t type);
const char *wireless_standard_name(wireless_standard_t standard);
const char *vpn_type_name(vpn_type_t type);
int network_address_from_string(const char *addr_str, network_address_t *addr);
int network_address_to_string(const network_address_t *addr, char *str, size_t size);
bool network_address_is_multicast(const network_address_t *addr);
bool network_address_is_broadcast(const network_address_t *addr);
uint32_t network_calculate_checksum(const void *data, size_t length);