/*
 * Network Device Interface
 * 
 * Production-grade network device abstraction layer.
 * Provides unified interface for all network hardware.
 * 
 * Features:
 * - Multi-queue support (multiple TX/RX queues)
 * - Hardware offloading (checksum, TSO, GSO, etc.)
 * - NAPI (New API) for efficient interrupt handling
 * - Ethtool support for configuration
 * - Statistics and monitoring
 * - Power management
 * - Network namespaces support
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#ifndef NET_NETDEVICE_H
#define NET_NETDEVICE_H

#include <stdint.h>
#include <stddef.h>
#include "net/skbuff.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum device name length */
#define IFNAMSIZ 16

/* Device flags (IFF_* from Linux/BSD) */
#define IFF_UP          (1 << 0)   /* Interface is up */
#define IFF_BROADCAST   (1 << 1)   /* Broadcast address valid */
#define IFF_DEBUG       (1 << 2)   /* Internal debugging flag */
#define IFF_LOOPBACK    (1 << 3)   /* Is a loopback net */
#define IFF_POINTOPOINT (1 << 4)   /* Point-to-point link */
#define IFF_NOTRAILERS  (1 << 5)   /* Avoid use of trailers */
#define IFF_RUNNING     (1 << 6)   /* Resources allocated */
#define IFF_NOARP       (1 << 7)   /* No ARP protocol */
#define IFF_PROMISC     (1 << 8)   /* Receive all packets */
#define IFF_ALLMULTI    (1 << 9)   /* Receive all multicast */
#define IFF_MASTER      (1 << 10)  /* Master of a load balancer */
#define IFF_SLAVE       (1 << 11)  /* Slave of a load balancer */
#define IFF_MULTICAST   (1 << 12)  /* Supports multicast */
#define IFF_PORTSEL     (1 << 13)  /* Can set media type */
#define IFF_AUTOMEDIA   (1 << 14)  /* Auto media select active */
#define IFF_DYNAMIC     (1 << 15)  /* Addresses are dynamic */

/* Hardware types */
#define ARPHRD_ETHER    1    /* Ethernet */
#define ARPHRD_LOOPBACK 772  /* Loopback device */
#define ARPHRD_PPP      512  /* Point-to-Point Protocol */
#define ARPHRD_TUNNEL   768  /* IP tunnel */
#define ARPHRD_TUNNEL6  769  /* IPv6 tunnel */

/* Transmit return codes */
#define NETDEV_TX_OK        0x00  /* Driver took care of packet */
#define NETDEV_TX_BUSY      0x10  /* Driver tx path was busy */
#define NETDEV_TX_LOCKED    0x20  /* Driver tx lock was already taken */

/* Device features (hardware capabilities) */
#define NETIF_F_SG              (1ULL << 0)   /* Scatter/gather IO */
#define NETIF_F_IP_CSUM         (1ULL << 1)   /* Can checksum TCP/UDP over IPv4 */
#define NETIF_F_HW_CSUM         (1ULL << 2)   /* Can checksum all protocols */
#define NETIF_F_IPV6_CSUM       (1ULL << 3)   /* Can checksum TCP/UDP over IPv6 */
#define NETIF_F_HIGHDMA         (1ULL << 4)   /* Can DMA to high memory */
#define NETIF_F_FRAGLIST        (1ULL << 5)   /* Scatter/gather IO */
#define NETIF_F_TSO             (1ULL << 6)   /* Can offload TCP segmentation */
#define NETIF_F_UFO             (1ULL << 7)   /* Can offload UDP fragmentation */
#define NETIF_F_GSO             (1ULL << 8)   /* Generic segmentation offload */
#define NETIF_F_GRO             (1ULL << 9)   /* Generic receive offload */
#define NETIF_F_TSO6            (1ULL << 10)  /* Can offload TCP segmentation (IPv6) */
#define NETIF_F_RXCSUM          (1ULL << 11)  /* Receive checksumming offload */
#define NETIF_F_VLAN_CHAL       (1ULL << 12)  /* VLAN tag insertion/stripping */
#define NETIF_F_LRO             (1ULL << 13)  /* Large receive offload */
#define NETIF_F_NTUPLE          (1ULL << 14)  /* N-tuple filters */
#define NETIF_F_RXHASH          (1ULL << 15)  /* Receive hashing offload */

/* MAC address length */
#define ETH_ALEN 6
#define MAX_ADDR_LEN 32

/* Forward declarations */
struct net_device;
struct net_device_stats;
struct ethtool_ops;
struct netdev_queue;

/* Device statistics */
typedef struct net_device_stats {
    uint64_t rx_packets;           /* Total packets received */
    uint64_t tx_packets;           /* Total packets transmitted */
    uint64_t rx_bytes;             /* Total bytes received */
    uint64_t tx_bytes;             /* Total bytes transmitted */
    uint64_t rx_errors;            /* Bad packets received */
    uint64_t tx_errors;            /* Packet transmit problems */
    uint64_t rx_dropped;           /* No space in buffers */
    uint64_t tx_dropped;           /* No space available */
    uint64_t multicast;            /* Multicast packets received */
    uint64_t collisions;           /* Collisions */
    
    /* Detailed RX errors */
    uint64_t rx_length_errors;
    uint64_t rx_over_errors;       /* Receiver ring overflow */
    uint64_t rx_crc_errors;        /* CRC errors */
    uint64_t rx_frame_errors;      /* Frame alignment errors */
    uint64_t rx_fifo_errors;       /* FIFO overrun */
    uint64_t rx_missed_errors;     /* Missed packets */
    
    /* Detailed TX errors */
    uint64_t tx_aborted_errors;
    uint64_t tx_carrier_errors;
    uint64_t tx_fifo_errors;
    uint64_t tx_heartbeat_errors;
    uint64_t tx_window_errors;
    
    /* Compression */
    uint64_t rx_compressed;
    uint64_t tx_compressed;
} net_device_stats_t;

/* RX handler result codes */
typedef enum {
    RX_HANDLER_CONSUMED,   /* RX handler consumed packet */
    RX_HANDLER_ANOTHER,    /* Try another handler */
    RX_HANDLER_EXACT,      /* Exact match, no other handlers */
    RX_HANDLER_PASS        /* Pass to network stack */
} rx_handler_result_t;

/* NAPI structure for efficient interrupt handling */
typedef struct napi_struct {
    struct napi_struct* next;
    struct net_device* dev;
    int (*poll)(struct napi_struct* napi, int budget);
    int weight;
    int quota;
    uint32_t state;
    uint32_t gro_count;
    sk_buff_head_t gro_list;       /* GRO packet list */
} napi_struct_t;

/* Network device transmit queue */
typedef struct netdev_queue {
    struct net_device* dev;
    sk_buff_head_t queue;          /* Pending packets */
    uint32_t state;                /* Queue state */
    uint64_t trans_start;          /* Last transmission time */
    uint64_t trans_timeout;        /* Transmission timeout */
} netdev_queue_t;

/* Network device operations - driver must implement these */
typedef struct net_device_ops {
    /* Initialization */
    int (*ndo_init)(struct net_device* dev);
    void (*ndo_uninit)(struct net_device* dev);
    
    /* Device control */
    int (*ndo_open)(struct net_device* dev);
    int (*ndo_stop)(struct net_device* dev);
    
    /* Transmit */
    int (*ndo_start_xmit)(sk_buff_t* skb, struct net_device* dev);
    void (*ndo_tx_timeout)(struct net_device* dev);
    
    /* Configuration */
    int (*ndo_set_mac_address)(struct net_device* dev, void* addr);
    int (*ndo_validate_addr)(struct net_device* dev);
    int (*ndo_set_config)(struct net_device* dev, void* cfg);
    
    /* Multicast list management */
    void (*ndo_set_rx_mode)(struct net_device* dev);
    void (*ndo_set_multicast_list)(struct net_device* dev);
    
    /* VLAN */
    int (*ndo_vlan_rx_add_vid)(struct net_device* dev, uint16_t vid);
    int (*ndo_vlan_rx_kill_vid)(struct net_device* dev, uint16_t vid);
    
    /* Statistics */
    net_device_stats_t* (*ndo_get_stats)(struct net_device* dev);
    void (*ndo_get_stats64)(struct net_device* dev, net_device_stats_t* stats);
    
    /* Ethtool operations */
    int (*ndo_set_features)(struct net_device* dev, uint64_t features);
    
    /* Polling (for NAPI) */
    int (*ndo_poll)(struct napi_struct* napi, int budget);
    
    /* Power management */
    int (*ndo_suspend)(struct net_device* dev);
    int (*ndo_resume)(struct net_device* dev);
} net_device_ops_t;

/* Ethtool operations for device configuration */
typedef struct ethtool_ops {
    void (*get_drvinfo)(struct net_device* dev, void* info);
    int (*get_settings)(struct net_device* dev, void* cmd);
    int (*set_settings)(struct net_device* dev, void* cmd);
    uint32_t (*get_link)(struct net_device* dev);
    int (*get_eeprom_len)(struct net_device* dev);
    int (*get_eeprom)(struct net_device* dev, void* eeprom, uint8_t* data);
    int (*set_eeprom)(struct net_device* dev, void* eeprom, uint8_t* data);
    void (*get_ringparam)(struct net_device* dev, void* ring);
    int (*set_ringparam)(struct net_device* dev, void* ring);
    int (*reset)(struct net_device* dev);
} ethtool_ops_t;

/* Main network device structure */
typedef struct net_device {
    /* Device identification */
    char name[IFNAMSIZ];           /* Device name (e.g., "eth0") */
    int ifindex;                   /* Interface index */
    
    /* Device state */
    uint32_t state;                /* Device state flags */
    uint32_t flags;                /* IFF_* flags */
    uint32_t priv_flags;           /* Private flags */
    
    /* Hardware information */
    uint16_t type;                 /* Hardware type (ARPHRD_*) */
    uint8_t addr_len;              /* Hardware address length */
    uint8_t broadcast[MAX_ADDR_LEN]; /* Hardware broadcast address */
    uint8_t dev_addr[MAX_ADDR_LEN];  /* Hardware address */
    uint8_t perm_addr[MAX_ADDR_LEN]; /* Permanent hardware address */
    
    /* Network parameters */
    uint32_t mtu;                  /* Maximum transmission unit */
    uint32_t min_mtu;              /* Minimum MTU */
    uint32_t max_mtu;              /* Maximum MTU */
    
    /* Features */
    uint64_t features;             /* Currently active features */
    uint64_t hw_features;          /* Hardware-supported features */
    uint64_t wanted_features;      /* User-requested features */
    
    /* Queuing */
    uint16_t num_tx_queues;        /* Number of TX queues */
    uint16_t real_num_tx_queues;   /* Currently active TX queues */
    netdev_queue_t* tx_queue;      /* TX queue array */
    
    uint16_t num_rx_queues;        /* Number of RX queues */
    uint16_t real_num_rx_queues;   /* Currently active RX queues */
    
    /* NAPI */
    napi_struct_t* napi_list;      /* NAPI instances */
    
    /* Operations */
    const net_device_ops_t* netdev_ops;    /* Device operations */
    const ethtool_ops_t* ethtool_ops;      /* Ethtool operations */
    
    /* Statistics */
    net_device_stats_t stats;      /* Device statistics */
    
    /* Timers and timeouts */
    uint64_t trans_start;          /* Last transmission time */
    uint64_t last_rx;              /* Last receive time */
    int watchdog_timeo;            /* Watchdog timeout */
    
    /* RX handler */
    rx_handler_result_t (*rx_handler)(sk_buff_t** pskb);
    void* rx_handler_data;
    
    /* Device-specific private data */
    void* priv;                    /* Driver private data */
    size_t priv_size;              /* Size of private data */
    
    /* Link management */
    uint8_t link;                  /* Link status (0=down, 1=up) */
    uint8_t operstate;             /* RFC2863 operational status */
    uint8_t carrier;               /* Carrier detected */
    
    /* Multicast */
    uint32_t mc_count;             /* Multicast address count */
    void* mc_list;                 /* Multicast address list */
    
    /* Power management */
    uint8_t pm_qos_req;            /* PM QoS request */
    
    /* Network namespace */
    void* net_ns;                  /* Network namespace */
    
    /* Device list linkage */
    struct net_device* next;
    struct net_device* prev;
    
} net_device_t;

/* ==================== Network Device Functions ==================== */

/* Device allocation and registration */
net_device_t* alloc_netdev(size_t sizeof_priv, const char* name, void (*setup)(net_device_t*));
net_device_t* alloc_etherdev(size_t sizeof_priv);  /* Allocate Ethernet device */
void free_netdev(net_device_t* dev);
int register_netdev(net_device_t* dev);
void unregister_netdev(net_device_t* dev);

/* Device control */
int netdev_open(net_device_t* dev);
int netdev_close(net_device_t* dev);
int netdev_start(net_device_t* dev);
int netdev_stop(net_device_t* dev);

/* Packet transmission */
int netdev_xmit(sk_buff_t* skb, net_device_t* dev);
void netdev_tx_timeout(net_device_t* dev);
void netdev_tx_sent_queue(netdev_queue_t* queue, uint32_t bytes);
void netdev_tx_completed_queue(netdev_queue_t* queue, uint32_t pkts, uint32_t bytes);

/* Packet reception */
int netif_rx(sk_buff_t* skb);
int netif_receive_skb(sk_buff_t* skb);
void netif_rx_schedule(net_device_t* dev);

/* NAPI functions */
void napi_enable(napi_struct_t* napi);
void napi_disable(napi_struct_t* napi);
void napi_schedule(napi_struct_t* napi);
int napi_poll(napi_struct_t* napi, int budget);
void napi_complete(napi_struct_t* napi);
void napi_gro_receive(napi_struct_t* napi, sk_buff_t* skb);
sk_buff_t* napi_gro_frags(napi_struct_t* napi);

/* Device state management */
void netif_start_queue(net_device_t* dev);
void netif_stop_queue(net_device_t* dev);
void netif_wake_queue(net_device_t* dev);
int netif_queue_stopped(const net_device_t* dev);
int netif_running(const net_device_t* dev);
int netif_carrier_ok(const net_device_t* dev);
void netif_carrier_on(net_device_t* dev);
void netif_carrier_off(net_device_t* dev);

/* Device lookup */
net_device_t* dev_get_by_name(const char* name);
net_device_t* dev_get_by_index(int ifindex);
void dev_put(net_device_t* dev);

/* MAC address management */
int dev_set_mac_address(net_device_t* dev, uint8_t* addr);
int dev_addr_add(net_device_t* dev, uint8_t* addr, uint8_t addr_type);
int dev_addr_del(net_device_t* dev, uint8_t* addr, uint8_t addr_type);
int is_valid_ether_addr(const uint8_t* addr);
void random_ether_addr(uint8_t* addr);

/* Multicast management */
int dev_mc_add(net_device_t* dev, uint8_t* addr);
int dev_mc_del(net_device_t* dev, uint8_t* addr);
void dev_mc_flush(net_device_t* dev);
void dev_set_allmulti(net_device_t* dev, int inc);
void dev_set_promiscuity(net_device_t* dev, int inc);

/* Feature management */
uint64_t netdev_get_features(net_device_t* dev);
int netdev_set_features(net_device_t* dev, uint64_t features);
void netdev_update_features(net_device_t* dev);

/* Statistics */
net_device_stats_t* dev_get_stats(net_device_t* dev);
void dev_get_stats64(net_device_t* dev, net_device_stats_t* stats);

/* Utilities */
void netdev_info(net_device_t* dev, const char* fmt, ...);
void netdev_warn(net_device_t* dev, const char* fmt, ...);
void netdev_err(net_device_t* dev, const char* fmt, ...);
void netdev_dump(const net_device_t* dev);

/* Global network device list */
net_device_t* dev_base_head(void);
void dev_base_lock(void);
void dev_base_unlock(void);

/* Initialization */
int netdev_init(void);
void netdev_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* NET_NETDEVICE_H */
