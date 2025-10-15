/**
 * Advanced Network Stack for LimitlessOS
 * 
 * Implements comprehensive TCP/IP stack with IPv6 support, advanced routing,
 * firewall capabilities, network namespaces, and high-performance packet
 * processing to match modern networking requirements.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#ifndef __NETWORK_STACK_H__
#define __NETWORK_STACK_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "smp.h"
#include "mm/advanced.h"

/* Protocol constants */
#define ETH_ALEN            6           /* Ethernet address length */
#define ETH_HLEN            14          /* Ethernet header length */
#define ETH_ZLEN            60          /* Ethernet minimum frame size */
#define ETH_DATA_LEN        1500        /* Ethernet maximum data length */
#define ETH_FRAME_LEN       1514        /* Ethernet maximum frame length */

#define IP_MAXLEN           65535       /* Maximum IP packet length */
#define IPV6_MAXLEN         65535       /* Maximum IPv6 packet length */
#define TCP_MAXWIN          65535       /* Maximum TCP window size */
#define UDP_MAXLEN          65507       /* Maximum UDP datagram length */

/* Ethernet types */
#define ETH_P_IP            0x0800      /* Internet Protocol packet */
#define ETH_P_ARP           0x0806      /* Address Resolution Protocol */
#define ETH_P_IPV6          0x86DD      /* IPv6 over bluebook */
#define ETH_P_8021Q         0x8100      /* 802.1Q VLAN Extended Header */

/* IP protocols */
#define IPPROTO_ICMP        1           /* Internet Control Message Protocol */
#define IPPROTO_TCP         6           /* Transmission Control Protocol */
#define IPPROTO_UDP         17          /* User Datagram Protocol */
#define IPPROTO_IPV6        41          /* IPv6 in IPv4 */
#define IPPROTO_ICMPV6      58          /* ICMPv6 */

/* Socket types */
#define SOCK_STREAM         1           /* TCP socket */
#define SOCK_DGRAM          2           /* UDP socket */
#define SOCK_RAW            3           /* Raw socket */
#define SOCK_PACKET         10          /* Packet socket */

/* Address families */
#define AF_UNSPEC           0           /* Unspecified */
#define AF_INET             2           /* IPv4 */
#define AF_INET6            10          /* IPv6 */
#define AF_PACKET           17          /* Packet family */

/* TCP states */
typedef enum {
    TCP_CLOSED,
    TCP_LISTEN,
    TCP_SYN_SENT,
    TCP_SYN_RECV,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT1,
    TCP_FIN_WAIT2,
    TCP_CLOSE_WAIT,
    TCP_CLOSING,
    TCP_LAST_ACK,
    TCP_TIME_WAIT
} tcp_state_t;

/* Forward declarations */
typedef struct sk_buff sk_buff_t;
typedef struct net_device net_device_t;
typedef struct socket socket_t;
typedef struct inet_sock inet_sock_t;
typedef struct tcp_sock tcp_sock_t;
typedef struct udp_sock udp_sock_t;
typedef struct route_entry route_entry_t;
typedef struct netfilter_hook netfilter_hook_t;

/**
 * Network buffer structure (socket buffer)
 */
struct sk_buff {
    /* Buffer management */
    uint8_t *head;                      /* Buffer head */
    uint8_t *data;                      /* Data pointer */
    uint8_t *tail;                      /* Tail pointer */
    uint8_t *end;                       /* Buffer end */
    uint32_t len;                       /* Data length */
    uint32_t data_len;                  /* Data length in fragments */
    uint32_t truesize;                  /* True buffer size */
    
    /* Network device */
    net_device_t *dev;                  /* Input/output device */
    
    /* Protocol headers */
    struct {
        uint8_t *mac;                   /* MAC header */
        uint8_t *network;               /* Network header (IP) */
        uint8_t *transport;             /* Transport header (TCP/UDP) */
    } headers;
    
    /* Packet information */
    uint16_t protocol;                  /* Ethernet protocol */
    uint8_t ip_summed;                  /* Checksum status */
    uint8_t pkt_type;                   /* Packet type */
    
    /* VLAN information */
    uint16_t vlan_tci;                  /* VLAN tag control info */
    bool vlan_tagged;                   /* VLAN tagged */
    
    /* Timestamps */
    uint64_t tstamp;                    /* Packet timestamp */
    
    /* Security context */
    void *secpath;                      /* IPsec security path */
    
    /* Fragment information */
    struct {
        uint32_t frag_off;              /* Fragment offset */
        bool more_frags;                /* More fragments flag */
        uint16_t frag_id;               /* Fragment ID */
    } frag;
    
    /* Socket */
    socket_t *sk;                       /* Associated socket */
    
    /* Reference counting */
    atomic_t users;                     /* Reference count */
    
    /* List linkage */
    sk_buff_t *next;                    /* Next in list */
    sk_buff_t *prev;                    /* Previous in list */
    
    /* Destructor */
    void (*destructor)(sk_buff_t *skb);
    
    /* Private data */
    uint8_t cb[48];                     /* Control buffer */
};

/**
 * Network device structure
 */
struct net_device {
    char name[16];                      /* Device name */
    uint32_t ifindex;                   /* Interface index */
    
    /* Hardware information */
    uint8_t dev_addr[ETH_ALEN];         /* Hardware address */
    uint8_t broadcast[ETH_ALEN];        /* Broadcast address */
    uint16_t type;                      /* Hardware type */
    uint16_t hard_header_len;           /* Hardware header length */
    uint32_t mtu;                       /* Maximum transmission unit */
    uint32_t min_mtu;                   /* Minimum MTU */
    uint32_t max_mtu;                   /* Maximum MTU */
    
    /* Device flags */
    uint32_t flags;                     /* Interface flags */
    uint32_t priv_flags;                /* Private flags */
    
    /* Device state */
    enum {
        NETDEV_STATE_PRESENT = 1,
        NETDEV_STATE_DOWN,
        NETDEV_STATE_DORMANT,
        NETDEV_STATE_TESTING,
        NETDEV_STATE_UP,
        NETDEV_STATE_UNKNOWN
    } state;
    
    /* Network operations */
    struct {
        int (*open)(net_device_t *dev);
        int (*stop)(net_device_t *dev);
        int (*hard_start_xmit)(sk_buff_t *skb, net_device_t *dev);
        int (*set_mac_address)(net_device_t *dev, void *addr);
        int (*set_mtu)(net_device_t *dev, int new_mtu);
        void (*get_stats)(net_device_t *dev);
        int (*ioctl)(net_device_t *dev, struct ifreq *ifr, int cmd);
    } netdev_ops;
    
    /* Ethernet operations */
    struct {
        int (*setup)(net_device_t *dev);
        int (*validate_addr)(net_device_t *dev);
        uint16_t (*type_trans)(sk_buff_t *skb, net_device_t *dev);
        int (*header)(sk_buff_t *skb, net_device_t *dev,
                     unsigned short type, const void *daddr,
                     const void *saddr, unsigned len);
        int (*rebuild_header)(sk_buff_t *skb);
        int (*cache_update)(struct hh_cache *hh,
                           const net_device_t *dev,
                           const unsigned char *haddr);
    } header_ops;
    
    /* Traffic control */
    struct {
        void *qdisc;                    /* Queuing discipline */
        spinlock_t tx_lock;             /* TX lock */
        uint32_t tx_queue_len;          /* TX queue length */
        sk_buff_t *tx_queue;            /* TX queue head */
        sk_buff_t *tx_queue_tail;       /* TX queue tail */
    } qdisc;
    
    /* Statistics */
    struct {
        uint64_t rx_packets;            /* Received packets */
        uint64_t tx_packets;            /* Transmitted packets */
        uint64_t rx_bytes;              /* Received bytes */
        uint64_t tx_bytes;              /* Transmitted bytes */
        uint64_t rx_errors;             /* Receive errors */
        uint64_t tx_errors;             /* Transmit errors */
        uint64_t rx_dropped;            /* Received packets dropped */
        uint64_t tx_dropped;            /* Transmitted packets dropped */
        uint64_t collisions;            /* Collision count */
        uint64_t rx_crc_errors;         /* CRC errors */
        uint64_t rx_frame_errors;       /* Frame errors */
        uint64_t tx_carrier_errors;     /* Carrier errors */
    } stats;
    
    /* Per-CPU statistics */
    struct {
        uint64_t rx_packets;
        uint64_t tx_packets;
        uint64_t rx_bytes;
        uint64_t tx_bytes;
    } *percpu_stats;
    
    /* Network namespace */
    void *nd_net;                       /* Network namespace */
    
    /* Device lock */
    spinlock_t lock;
    
    /* Private data */
    void *priv;                         /* Driver private data */
    
    /* List linkage */
    net_device_t *next;                 /* Next device */
};

/**
 * Socket structure
 */
struct socket {
    uint32_t family;                    /* Address family */
    uint32_t type;                      /* Socket type */
    uint32_t protocol;                  /* Protocol */
    uint32_t flags;                     /* Socket flags */
    
    /* Socket state */
    enum {
        SS_FREE = 0,                    /* Not allocated */
        SS_UNCONNECTED,                 /* Unconnected to any socket */
        SS_CONNECTING,                  /* In process of connecting */
        SS_CONNECTED,                   /* Connected to socket */
        SS_DISCONNECTING               /* In process of disconnecting */
    } state;
    
    /* Socket operations */
    const struct proto_ops *ops;        /* Protocol operations */
    
    /* Protocol control block */
    void *sk;                           /* Protocol-specific socket */
    
    /* File operations */
    struct file *file;                  /* Associated file */
    
    /* Wait queues */
    wait_queue_head_t wait;             /* Socket wait queue */
    
    /* Socket lock */
    spinlock_t lock;
};

/**
 * Internet socket structure
 */
struct inet_sock {
    socket_t sock;                      /* Base socket */
    
    /* Address information */
    uint32_t inet_saddr;                /* Source address */
    uint32_t inet_daddr;                /* Destination address */
    uint16_t inet_sport;                /* Source port */
    uint16_t inet_dport;                /* Destination port */
    uint16_t inet_id;                   /* IP ID */
    
    /* Socket options */
    struct {
        bool reuseaddr;                 /* SO_REUSEADDR */
        bool reuseport;                 /* SO_REUSEPORT */
        bool keepalive;                 /* SO_KEEPALIVE */
        bool broadcast;                 /* SO_BROADCAST */
        uint32_t rcvbuf;                /* SO_RCVBUF */
        uint32_t sndbuf;                /* SO_SNDBUF */
        uint32_t rcvtimeo;              /* SO_RCVTIMEO */
        uint32_t sndtimeo;              /* SO_SNDTIMEO */
    } opts;
    
    /* Receive/transmit queues */
    struct {
        sk_buff_t *head;                /* Queue head */
        sk_buff_t *tail;                /* Queue tail */
        uint32_t qlen;                  /* Queue length */
        uint32_t memory;                /* Memory usage */
        spinlock_t lock;                /* Queue lock */
    } rx_queue, tx_queue;
    
    /* Network device */
    net_device_t *bound_dev_if;         /* Bound device */
    
    /* Multicast */
    struct {
        uint32_t multiaddr[32];         /* Multicast addresses */
        uint32_t multicount;            /* Multicast count */
        uint8_t mc_ttl;                 /* Multicast TTL */
        uint8_t mc_loop;                /* Multicast loopback */
    } mc;
    
    /* Fragment assembly */
    struct {
        sk_buff_t *queue;               /* Fragment queue */
        uint32_t len;                   /* Total length */
        uint32_t meat;                  /* Received data */
        spinlock_t lock;                /* Fragment lock */
    } frag;
};

/**
 * TCP socket structure
 */
struct tcp_sock {
    inet_sock_t inet;                   /* Base inet socket */
    
    /* TCP state */
    tcp_state_t state;                  /* Connection state */
    
    /* Sequence numbers */
    uint32_t snd_una;                   /* Send unacknowledged */
    uint32_t snd_nxt;                   /* Send next */
    uint32_t snd_wnd;                   /* Send window */
    uint32_t snd_wl1;                   /* Sequence for window update */
    uint32_t snd_wl2;                   /* Ack sequence for window update */
    uint32_t iss;                       /* Initial send sequence number */
    
    uint32_t rcv_nxt;                   /* Receive next */
    uint32_t rcv_wnd;                   /* Receive window */
    uint32_t rcv_wup;                   /* Receive window update point */
    uint32_t irs;                       /* Initial receive sequence number */
    
    /* Congestion control */
    struct {
        uint32_t cwnd;                  /* Congestion window */
        uint32_t ssthresh;              /* Slow start threshold */
        uint32_t snd_cwnd_clamp;        /* Congestion window clamp */
        uint32_t mss_cache;             /* Cached effective mss */
        uint16_t advmss;                /* Advertised MSS */
        uint8_t ca_state;               /* Congestion avoidance state */
        uint8_t retransmits;            /* Number of retransmits */
    } cong;
    
    /* Timers */
    struct {
        struct timer_list retransmit;   /* Retransmission timer */
        struct timer_list delack;       /* Delayed ack timer */
        struct timer_list probe;        /* Zero window probe timer */
        struct timer_list keepalive;    /* Keepalive timer */
    } timers;
    
    /* Round trip time */
    struct {
        uint32_t srtt;                  /* Smoothed round trip time */
        uint32_t mdev;                  /* Medium deviation */
        uint32_t mdev_max;              /* Maximum deviation */
        uint32_t rttvar;                /* Round trip time variance */
        uint32_t rto;                   /* Retransmission timeout */
    } rtt;
    
    /* TCP options */
    struct {
        bool timestamps;                /* Timestamp option */
        bool sack;                      /* SACK option */
        bool window_scaling;            /* Window scaling option */
        uint8_t snd_wscale;             /* Send window scale */
        uint8_t rcv_wscale;             /* Receive window scale */
        uint16_t user_mss;              /* User specified MSS */
    } options;
    
    /* Out-of-order queue */
    struct {
        sk_buff_t *queue;               /* OOO queue head */
        uint32_t memory;                /* Memory usage */
    } ooo;
    
    /* Retransmission queue */
    struct {
        sk_buff_t *queue;               /* Retrans queue head */
        uint32_t packets_out;           /* Packets in flight */
        uint32_t retrans_out;           /* Retransmitted packets */
        uint32_t lost_out;              /* Lost packets */
        uint32_t sacked_out;            /* SACKed packets */
    } retrans;
};

/**
 * UDP socket structure
 */
struct udp_sock {
    inet_sock_t inet;                   /* Base inet socket */
    
    /* UDP-specific options */
    bool no_check6_tx;                  /* Skip UDP checksum on TX for IPv6 */
    bool no_check6_rx;                  /* Skip UDP checksum on RX for IPv6 */
    
    /* Encapsulation */
    struct {
        int (*encap_rcv)(socket_t *sk, sk_buff_t *skb);
        void (*encap_destroy)(socket_t *sk);
        uint16_t encap_type;            /* Encapsulation type */
    } encap;
};

/**
 * IPv4 address structure
 */
typedef struct {
    uint32_t s_addr;                    /* IPv4 address */
} ipv4_addr_t;

/**
 * IPv6 address structure
 */
typedef struct {
    uint8_t s6_addr[16];                /* IPv6 address */
} ipv6_addr_t;

/**
 * Socket address structures
 */
typedef struct sockaddr_in {
    uint16_t sin_family;                /* AF_INET */
    uint16_t sin_port;                  /* Port number */
    ipv4_addr_t sin_addr;               /* IPv4 address */
    uint8_t sin_zero[8];                /* Padding */
} sockaddr_in_t;

typedef struct sockaddr_in6 {
    uint16_t sin6_family;               /* AF_INET6 */
    uint16_t sin6_port;                 /* Port number */
    uint32_t sin6_flowinfo;             /* Flow information */
    ipv6_addr_t sin6_addr;              /* IPv6 address */
    uint32_t sin6_scope_id;             /* Scope ID */
} sockaddr_in6_t;

/**
 * Routing table entry
 */
struct route_entry {
    /* Destination */
    ipv4_addr_t dest;                   /* Destination address */
    ipv4_addr_t netmask;                /* Network mask */
    ipv4_addr_t gateway;                /* Gateway address */
    
    /* Interface */
    net_device_t *dev;                  /* Output device */
    uint32_t ifindex;                   /* Interface index */
    
    /* Metrics */
    uint32_t metric;                    /* Route metric */
    uint32_t mtu;                       /* Path MTU */
    
    /* Flags */
    uint32_t flags;                     /* Route flags */
    
    /* Reference counting */
    atomic_t refcnt;                    /* Reference count */
    
    /* Cache information */
    uint64_t expires;                   /* Cache expiration */
    uint64_t lastuse;                   /* Last use time */
    
    /* List linkage */
    route_entry_t *next;                /* Next in hash bucket */
};

/**
 * IPv6 routing table entry
 */
typedef struct route6_entry {
    /* Destination */
    ipv6_addr_t dest;                   /* Destination address */
    uint8_t dest_len;                   /* Prefix length */
    ipv6_addr_t gateway;                /* Gateway address */
    
    /* Interface */
    net_device_t *dev;                  /* Output device */
    uint32_t ifindex;                   /* Interface index */
    
    /* Metrics */
    uint32_t metric;                    /* Route metric */
    uint32_t mtu;                       /* Path MTU */
    
    /* Flags */
    uint32_t flags;                     /* Route flags */
    
    /* Reference counting */
    atomic_t refcnt;                    /* Reference count */
    
    /* Cache information */
    uint64_t expires;                   /* Cache expiration */
    uint64_t lastuse;                   /* Last use time */
    
    /* List linkage */
    struct route6_entry *next;          /* Next in hash bucket */
} route6_entry_t;

/**
 * Netfilter hook structure
 */
struct netfilter_hook {
    int (*hook)(unsigned int hooknum, sk_buff_t *skb,
               const net_device_t *in, const net_device_t *out,
               int (*okfn)(sk_buff_t *));
    int pf;                             /* Protocol family */
    unsigned int hooknum;               /* Hook number */
    int priority;                       /* Hook priority */
    netfilter_hook_t *next;             /* Next hook */
};

/**
 * Network namespace structure
 */
typedef struct net_namespace {
    uint32_t id;                        /* Namespace ID */
    
    /* Device list */
    net_device_t *dev_base_head;        /* Device list head */
    uint32_t dev_index_head;            /* Next device index */
    
    /* Routing tables */
    route_entry_t **ip_route_table;     /* IPv4 routing table */
    route6_entry_t **ip6_route_table;   /* IPv6 routing table */
    
    /* Protocol statistics */
    struct {
        uint64_t ip_forwarded;          /* IP packets forwarded */
        uint64_t ip_delivered;          /* IP packets delivered */
        uint64_t tcp_connections;       /* TCP connections */
        uint64_t udp_packets;           /* UDP packets */
    } stats;
    
    /* Namespace lock */
    spinlock_t lock;
    
    /* Reference counting */
    atomic_t count;
    
    /* List linkage */
    struct net_namespace *next;
} net_namespace_t;

/* Global network stack state */
extern struct {
    /* Device management */
    net_device_t *dev_base;             /* Device list */
    uint32_t dev_index_generator;       /* Device index generator */
    spinlock_t dev_base_lock;           /* Device list lock */
    
    /* Socket management */
    socket_t **inet_hash_table;         /* IPv4 socket hash table */
    socket_t **inet6_hash_table;        /* IPv6 socket hash table */
    spinlock_t inet_hash_lock;          /* Socket hash lock */
    
    /* Routing */
    route_entry_t **route_table;        /* IPv4 routing table */
    route6_entry_t **route6_table;      /* IPv6 routing table */
    spinlock_t route_lock;              /* Routing table lock */
    
    /* Netfilter */
    netfilter_hook_t *nf_hooks[32];     /* Netfilter hooks */
    spinlock_t nf_lock;                 /* Netfilter lock */
    
    /* Network namespaces */
    net_namespace_t *namespace_list;    /* Namespace list */
    spinlock_t namespace_lock;          /* Namespace lock */
    
    /* Statistics */
    struct {
        atomic_long_t packets_received; /* Total packets received */
        atomic_long_t packets_sent;     /* Total packets sent */
        atomic_long_t bytes_received;   /* Total bytes received */
        atomic_long_t bytes_sent;       /* Total bytes sent */
        atomic_long_t socket_count;     /* Active socket count */
        atomic_long_t route_lookups;    /* Route lookups */
        atomic_long_t route_cache_hits; /* Route cache hits */
    } stats;
} net_stack;

/* Function declarations */

/* Network stack initialization */
int network_stack_init(void);
int ethernet_init(void);
int ipv4_init(void);
int ipv6_init(void);
int tcp_init(void);
int udp_init(void);
int netfilter_init(void);

/* Socket operations */
int sys_socket(int family, int type, int protocol);
int sys_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int sys_listen(int sockfd, int backlog);
int sys_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int sys_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t sys_recv(int sockfd, void *buf, size_t len, int flags);
ssize_t sys_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t sys_recvfrom(int sockfd, void *buf, size_t len, int flags,
                    struct sockaddr *src_addr, socklen_t *addrlen);
ssize_t sys_sendto(int sockfd, const void *buf, size_t len, int flags,
                  const struct sockaddr *dest_addr, socklen_t addrlen);
int sys_setsockopt(int sockfd, int level, int optname,
                  const void *optval, socklen_t optlen);
int sys_getsockopt(int sockfd, int level, int optname,
                  void *optval, socklen_t *optlen);
int sys_shutdown(int sockfd, int how);
int sys_close_socket(int sockfd);

/* Network device management */
net_device_t *alloc_netdev(size_t priv_size, const char *name);
void free_netdev(net_device_t *dev);
int register_netdev(net_device_t *dev);
void unregister_netdev(net_device_t *dev);
net_device_t *dev_get_by_name(const char *name);
net_device_t *dev_get_by_index(uint32_t ifindex);
int dev_open(net_device_t *dev);
int dev_close(net_device_t *dev);

/* Packet buffer management */
sk_buff_t *alloc_skb(size_t size, gfp_t gfp_mask);
void kfree_skb(sk_buff_t *skb);
sk_buff_t *skb_clone(sk_buff_t *skb, gfp_t gfp_mask);
sk_buff_t *skb_copy(const sk_buff_t *skb, gfp_t gfp_mask);
void *skb_put(sk_buff_t *skb, size_t len);
void *skb_push(sk_buff_t *skb, size_t len);
void *skb_pull(sk_buff_t *skb, size_t len);
void skb_reserve(sk_buff_t *skb, size_t len);

/* Packet transmission and reception */
int netif_rx(sk_buff_t *skb);
int netif_receive_skb(sk_buff_t *skb);
int dev_queue_xmit(sk_buff_t *skb);
void netif_tx_wake_queue(net_device_t *dev);
void netif_tx_stop_queue(net_device_t *dev);

/* Protocol processing */
int ethernet_rcv(sk_buff_t *skb, net_device_t *dev);
int ip_rcv(sk_buff_t *skb, net_device_t *dev);
int ipv6_rcv(sk_buff_t *skb, net_device_t *dev);
int tcp_v4_rcv(sk_buff_t *skb);
int tcp_v6_rcv(sk_buff_t *skb);
int udp_rcv(sk_buff_t *skb);
int udp_v6_rcv(sk_buff_t *skb);

/* Routing */
route_entry_t *ip_route_output(ipv4_addr_t dest, ipv4_addr_t src,
                              uint8_t tos, net_device_t *oif);
route6_entry_t *ip6_route_output(ipv6_addr_t *dest, ipv6_addr_t *src,
                                net_device_t *oif);
int ip_route_input(sk_buff_t *skb, ipv4_addr_t dest, ipv4_addr_t src,
                  uint8_t tos, net_device_t *dev);
int ip6_route_input(sk_buff_t *skb, ipv6_addr_t *dest, ipv6_addr_t *src,
                   net_device_t *dev);
int ip_route_add(ipv4_addr_t dest, ipv4_addr_t netmask, ipv4_addr_t gateway,
                net_device_t *dev, uint32_t metric);
int ip6_route_add(ipv6_addr_t *dest, uint8_t dest_len, ipv6_addr_t *gateway,
                 net_device_t *dev, uint32_t metric);

/* ARP */
int arp_rcv(sk_buff_t *skb, net_device_t *dev);
int arp_send(int type, int ptype, ipv4_addr_t dest_ip, net_device_t *dev,
            ipv4_addr_t src_ip, const unsigned char *dest_hw,
            const unsigned char *src_hw, const unsigned char *target_hw);
int arp_resolve(ipv4_addr_t dest_ip, net_device_t *dev, uint8_t *dest_mac);

/* ICMP */
int icmp_rcv(sk_buff_t *skb);
int icmpv6_rcv(sk_buff_t *skb);
void icmp_send(sk_buff_t *skb_in, int type, int code, uint32_t info);
void icmpv6_send(sk_buff_t *skb, uint8_t type, uint8_t code, uint32_t offset);

/* Netfilter/Firewall */
int nf_register_hook(netfilter_hook_t *hook);
void nf_unregister_hook(netfilter_hook_t *hook);
int nf_hook_slow(int pf, unsigned int hook, sk_buff_t *skb,
                net_device_t *indev, net_device_t *outdev,
                int (*okfn)(sk_buff_t *));

/* Network namespaces */
net_namespace_t *create_net_namespace(void);
void destroy_net_namespace(net_namespace_t *net);
int switch_net_namespace(net_namespace_t *net);
net_namespace_t *get_current_net_namespace(void);

/* Quality of Service */
int qdisc_create(net_device_t *dev, const char *kind);
int qdisc_change(net_device_t *dev, void *opt);
int qdisc_stats(net_device_t *dev, void *stats);

/* Network security */
int packet_filter(sk_buff_t *skb, net_device_t *dev);
int connection_track(sk_buff_t *skb);
int nat_process(sk_buff_t *skb);

/* Performance and monitoring */
void network_show_stats(void);
void socket_show_stats(void);
void route_show_table(void);
int network_performance_test(void);

/* Checksum functions */
uint16_t ip_checksum(const void *data, size_t len);
uint16_t tcp_checksum(const void *tcphdr, size_t len,
                     ipv4_addr_t src, ipv4_addr_t dest);
uint16_t udp_checksum(const void *udphdr, size_t len,
                     ipv4_addr_t src, ipv4_addr_t dest);

/* Utility functions */
uint32_t inet_addr(const char *cp);
char *inet_ntoa(ipv4_addr_t addr);
int inet_pton(int af, const char *src, void *dst);
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);

/* High-performance packet processing */
int setup_packet_ring(socket_t *sk, int version);
int packet_mmap(socket_t *sk, unsigned long addr, unsigned long len);
int packet_sendmsg_spkt(socket_t *sk, struct msghdr *msg, size_t len);

/* Inline helper functions */
static inline bool skb_is_nonlinear(const sk_buff_t *skb) {
    return skb->data_len;
}

static inline unsigned int skb_headlen(const sk_buff_t *skb) {
    return skb->len - skb->data_len;
}

static inline void skb_reset_mac_header(sk_buff_t *skb) {
    skb->headers.mac = skb->data;
}

static inline void skb_reset_network_header(sk_buff_t *skb) {
    skb->headers.network = skb->data;
}

static inline void skb_reset_transport_header(sk_buff_t *skb) {
    skb->headers.transport = skb->data;
}

static inline void skb_set_network_header(sk_buff_t *skb, int offset) {
    skb->headers.network = skb->data + offset;
}

static inline void skb_set_transport_header(sk_buff_t *skb, int offset) {
    skb->headers.transport = skb->data + offset;
}

static inline uint8_t *skb_mac_header(const sk_buff_t *skb) {
    return skb->headers.mac;
}

static inline uint8_t *skb_network_header(const sk_buff_t *skb) {
    return skb->headers.network;
}

static inline uint8_t *skb_transport_header(const sk_buff_t *skb) {
    return skb->headers.transport;
}

#endif /* __NETWORK_STACK_H__ */