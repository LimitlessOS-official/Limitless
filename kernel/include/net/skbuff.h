/*
 * Socket Buffer (sk_buff) - Network Packet Management
 * 
 * LimitlessOS implementation inspired by Linux sk_buff but designed for production use.
 * Provides efficient network packet handling with zero-copy capabilities.
 * 
 * Features:
 * - Zero-copy buffer management
 * - Protocol header pointers
 * - Reference counting
 * - Checksum offloading
 * - Scatter-gather I/O support
 * - Memory pooling for performance
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#ifndef NET_SKBUFF_H
#define NET_SKBUFF_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct sk_buff;
struct net_device;
struct socket;

/* sk_buff flags */
#define SKB_FLAG_CLONED         (1 << 0)  /* Buffer is cloned */
#define SKB_FLAG_SHARED         (1 << 1)  /* Buffer is shared */
#define SKB_FLAG_FREED          (1 << 2)  /* Buffer has been freed */
#define SKB_FLAG_CHECKSUM_VALID (1 << 3)  /* Checksum has been validated */
#define SKB_FLAG_CHECKSUM_OFFLOAD (1 << 4) /* Hardware checksum offload */
#define SKB_FLAG_GSO            (1 << 5)  /* Generic segmentation offload */
#define SKB_FLAG_TSO            (1 << 6)  /* TCP segmentation offload */
#define SKB_FLAG_URGENT         (1 << 7)  /* Urgent data */

/* sk_buff priority levels */
#define SKB_PRIORITY_CONTROL    0    /* Control packets (highest) */
#define SKB_PRIORITY_HIGH       1    /* High priority */
#define SKB_PRIORITY_NORMAL     2    /* Normal priority */
#define SKB_PRIORITY_LOW        3    /* Low priority */
#define SKB_PRIORITY_BACKGROUND 4    /* Background traffic (lowest) */

/* Packet types */
#define PACKET_HOST        0  /* Packet for this host */
#define PACKET_BROADCAST   1  /* Physical layer broadcast */
#define PACKET_MULTICAST   2  /* Physical layer multicast */
#define PACKET_OTHERHOST   3  /* Packet for another host */
#define PACKET_OUTGOING    4  /* Outgoing packet */
#define PACKET_LOOPBACK    5  /* Loopback packet */

/* IP protocol types (subset of IANA protocols) */
#define IPPROTO_ICMP       1
#define IPPROTO_IGMP       2
#define IPPROTO_TCP        6
#define IPPROTO_UDP        17
#define IPPROTO_IPV6       41
#define IPPROTO_ICMPV6     58
#define IPPROTO_RAW        255

/* Ethernet types */
#define ETH_P_IP           0x0800  /* IPv4 */
#define ETH_P_ARP          0x0806  /* ARP */
#define ETH_P_IPV6         0x86DD  /* IPv6 */
#define ETH_P_8021Q        0x8100  /* VLAN */

/* Maximum packet sizes */
#define SKB_DATA_ALIGN(X)  (((X) + 15) & ~15)  /* 16-byte alignment */
#define SKB_MAX_ALLOC      (16 * 1024)         /* 16KB max allocation */
#define SKB_MIN_HEADROOM   64                  /* Minimum headroom */
#define SKB_MIN_TAILROOM   32                  /* Minimum tailroom */

/* Scatter-gather list entry */
typedef struct skb_frag {
    void* page;           /* Page pointer */
    uint32_t offset;      /* Offset in page */
    uint32_t size;        /* Fragment size */
} skb_frag_t;

/* Shared info structure (for scatter-gather, GSO, etc.) */
typedef struct skb_shared_info {
    uint32_t nr_frags;                  /* Number of fragments */
    uint32_t gso_size;                  /* GSO segment size */
    uint16_t gso_segs;                  /* Number of GSO segments */
    uint16_t gso_type;                  /* GSO type */
    skb_frag_t frags[17];               /* Fragment array (17 = MAX_SKB_FRAGS) */
    struct sk_buff* frag_list;          /* Linked list of fragments */
} skb_shared_info_t;

/* Socket buffer structure - the heart of network packet management */
typedef struct sk_buff {
    /* Linked list */
    struct sk_buff* next;
    struct sk_buff* prev;
    
    /* Network device */
    struct net_device* dev;
    
    /* Socket association */
    struct socket* sk;
    
    /* Timestamps */
    uint64_t tstamp;                    /* Packet timestamp */
    
    /* Buffer management */
    uint8_t* head;                      /* Start of buffer */
    uint8_t* data;                      /* Start of data */
    uint8_t* tail;                      /* End of data */
    uint8_t* end;                       /* End of buffer */
    
    /* Protocol headers */
    uint8_t* mac_header;                /* Link layer header */
    uint8_t* network_header;            /* Network layer header (IP) */
    uint8_t* transport_header;          /* Transport layer header (TCP/UDP) */
    
    /* Buffer metadata */
    uint32_t len;                       /* Length of actual data */
    uint32_t data_len;                  /* Length of data in fragments */
    uint32_t truesize;                  /* Total buffer size */
    uint32_t mac_len;                   /* Link layer header length */
    
    /* Reference counting */
    uint32_t users;                     /* Reference count */
    
    /* Packet information */
    uint16_t protocol;                  /* Ethernet protocol type */
    uint16_t queue_mapping;             /* TX queue to use */
    uint8_t pkt_type;                   /* Packet classification */
    uint8_t ip_summed;                  /* Checksum status */
    uint8_t priority;                   /* QoS priority */
    uint8_t cloned;                     /* Is this a clone? */
    
    /* Flags */
    uint32_t flags;                     /* Various flags */
    
    /* Checksum */
    uint32_t csum;                      /* Checksum value */
    uint16_t csum_start;                /* Checksum start offset */
    uint16_t csum_offset;               /* Checksum offset */
    
    /* VLAN tagging */
    uint16_t vlan_tci;                  /* VLAN tag control information */
    uint16_t vlan_proto;                /* VLAN protocol */
    
    /* Destructor callback */
    void (*destructor)(struct sk_buff* skb);
    
    /* Private data area */
    uint8_t cb[48];                     /* Control buffer for protocol use */
    
    /* Shared info (for frags) */
    skb_shared_info_t* shinfo;
    
} sk_buff_t;

/* sk_buff memory pool for fast allocation */
typedef struct skb_pool {
    sk_buff_t* free_list;               /* Free buffer list */
    uint32_t total;                     /* Total buffers in pool */
    uint32_t free;                      /* Free buffers */
    uint32_t size;                      /* Buffer size */
    uint32_t allocated;                 /* Total allocated */
    uint32_t freed;                     /* Total freed */
    uint32_t failed;                    /* Failed allocations */
} skb_pool_t;

/* Global sk_buff statistics */
typedef struct skb_stats {
    uint64_t alloc_count;               /* Total allocations */
    uint64_t free_count;                /* Total frees */
    uint64_t clone_count;               /* Total clones */
    uint64_t copy_count;                /* Total copies */
    uint64_t pool_hits;                 /* Pool allocation hits */
    uint64_t pool_misses;               /* Pool allocation misses */
    uint64_t oom_count;                 /* Out of memory errors */
} skb_stats_t;

/* ==================== Core sk_buff Functions ==================== */

/* Allocation and deallocation */
sk_buff_t* alloc_skb(uint32_t size, uint32_t priority);
sk_buff_t* alloc_skb_with_headroom(uint32_t size, uint32_t headroom, uint32_t priority);
void free_skb(sk_buff_t* skb);
void kfree_skb(sk_buff_t* skb);  /* Kernel free wrapper */

/* Reference counting */
sk_buff_t* skb_get(sk_buff_t* skb);      /* Increment refcount */
void skb_put_ref(sk_buff_t* skb);        /* Decrement refcount */
int skb_shared(const sk_buff_t* skb);    /* Check if shared */

/* Cloning and copying */
sk_buff_t* skb_clone(sk_buff_t* skb, uint32_t priority);
sk_buff_t* skb_copy(const sk_buff_t* skb, uint32_t priority);
sk_buff_t* pskb_copy(sk_buff_t* skb, uint32_t priority);  /* Partial copy */

/* Data manipulation */
uint8_t* skb_put(sk_buff_t* skb, uint32_t len);      /* Extend tail */
uint8_t* skb_push(sk_buff_t* skb, uint32_t len);     /* Extend head */
uint8_t* skb_pull(sk_buff_t* skb, uint32_t len);     /* Remove from head */
void skb_reserve(sk_buff_t* skb, uint32_t len);      /* Reserve headroom */
void skb_trim(sk_buff_t* skb, uint32_t len);         /* Trim to length */

/* Header manipulation */
void skb_reset_mac_header(sk_buff_t* skb);
void skb_reset_network_header(sk_buff_t* skb);
void skb_reset_transport_header(sk_buff_t* skb);
void skb_set_mac_header(sk_buff_t* skb, int offset);
void skb_set_network_header(sk_buff_t* skb, int offset);
void skb_set_transport_header(sk_buff_t* skb, int offset);

/* Header access helpers */
static inline uint8_t* skb_mac_header(const sk_buff_t* skb) {
    return skb->mac_header;
}

static inline uint8_t* skb_network_header(const sk_buff_t* skb) {
    return skb->network_header;
}

static inline uint8_t* skb_transport_header(const sk_buff_t* skb) {
    return skb->transport_header;
}

static inline uint32_t skb_headroom(const sk_buff_t* skb) {
    return skb->data - skb->head;
}

static inline uint32_t skb_tailroom(const sk_buff_t* skb) {
    return skb->end - skb->tail;
}

static inline int skb_is_nonlinear(const sk_buff_t* skb) {
    return skb->data_len != 0;
}

/* Checksum functions */
void skb_checksum_complete(sk_buff_t* skb);
uint32_t skb_checksum(const sk_buff_t* skb, int offset, int len, uint32_t csum);
void skb_copy_and_checksum_bits(const sk_buff_t* skb, int offset, uint8_t* to, int len, uint32_t csum);

/* Fragment handling */
int skb_add_frag(sk_buff_t* skb, void* page, uint32_t offset, uint32_t size);
void skb_coalesce_frags(sk_buff_t* skb);
int skb_linearize(sk_buff_t* skb);  /* Convert nonlinear to linear */

/* Queue management */
typedef struct sk_buff_head {
    sk_buff_t* next;
    sk_buff_t* prev;
    uint32_t qlen;                      /* Queue length */
    uint32_t lock;                      /* Spinlock for SMP */
} sk_buff_head_t;

void skb_queue_head_init(sk_buff_head_t* list);
void skb_queue_tail(sk_buff_head_t* list, sk_buff_t* skb);
void skb_queue_head(sk_buff_head_t* list, sk_buff_t* skb);
sk_buff_t* skb_dequeue(sk_buff_head_t* list);
sk_buff_t* skb_dequeue_tail(sk_buff_head_t* list);
sk_buff_t* skb_peek(sk_buff_head_t* list);
sk_buff_t* skb_peek_tail(sk_buff_head_t* list);
void skb_unlink(sk_buff_t* skb, sk_buff_head_t* list);
void skb_queue_purge(sk_buff_head_t* list);
uint32_t skb_queue_len(const sk_buff_head_t* list);
int skb_queue_empty(const sk_buff_head_t* list);

/* Memory pool management */
int skb_pool_init(skb_pool_t* pool, uint32_t count, uint32_t size);
void skb_pool_destroy(skb_pool_t* pool);
sk_buff_t* skb_pool_alloc(skb_pool_t* pool);
void skb_pool_free(skb_pool_t* pool, sk_buff_t* skb);

/* Statistics */
void skb_get_stats(skb_stats_t* stats);
void skb_reset_stats(void);

/* Utilities */
void skb_dump(const sk_buff_t* skb);  /* Debug dump */
int skb_validate(const sk_buff_t* skb);  /* Validate structure */

#ifdef __cplusplus
}
#endif

#endif /* NET_SKBUFF_H */
