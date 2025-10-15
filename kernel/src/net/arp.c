/*
 * ARP (Address Resolution Protocol) Implementation
 * 
 * Production-grade ARP implementation with caching, timeouts,
 * and gratuitous ARP support.
 * 
 * RFC 826 - Address Resolution Protocol
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/arp.h"
#include "net/ethernet.h"
#include "net/skbuff.h"
#include "net/netdevice.h"
#include "kernel.h"
#include <string.h>

/* ARP Cache */
#define ARP_CACHE_SIZE 256
#define ARP_TIMEOUT (20 * 60 * 100)  /* 20 minutes in ticks (100 ticks/sec) */
#define ARP_RETRY_TIMEOUT (100)      /* 1 second in ticks */
#define ARP_MAX_RETRIES 3

typedef struct arp_entry {
    ipv4_addr_t ip_addr;
    uint8_t mac_addr[6];
    uint32_t timestamp;
    uint32_t flags;
    struct net_device* dev;
    
    /* Pending queue */
    struct sk_buff_head pending;
    uint32_t pending_count;
    
    int valid;
} arp_entry_t;

static struct {
    arp_entry_t cache[ARP_CACHE_SIZE];
    uint32_t count;
    
    /* Statistics */
    uint64_t requests_sent;
    uint64_t requests_rcvd;
    uint64_t replies_sent;
    uint64_t replies_rcvd;
    uint64_t cache_hits;
    uint64_t cache_misses;
    uint64_t invalid_packets;
} arp_state;

/* ARP Flags */
#define ARP_FLAG_COMPLETE   0x01
#define ARP_FLAG_PENDING    0x02
#define ARP_FLAG_PERMANENT  0x04

/* ==================== ARP Cache Management ==================== */

static uint32_t arp_hash(ipv4_addr_t ip) {
    return ip & (ARP_CACHE_SIZE - 1);
}

arp_entry_t* arp_lookup(ipv4_addr_t ip_addr) {
    uint32_t hash = arp_hash(ip_addr);
    
    for (uint32_t i = 0; i < ARP_CACHE_SIZE; i++) {
        uint32_t idx = (hash + i) & (ARP_CACHE_SIZE - 1);
        arp_entry_t* entry = &arp_state.cache[idx];
        
        if (entry->valid && entry->ip_addr == ip_addr) {
            /* Update timestamp on access */
            entry->timestamp = get_ticks();
            arp_state.cache_hits++;
            return entry;
        }
    }
    
    arp_state.cache_misses++;
    return NULL;
}

arp_entry_t* arp_create_entry(ipv4_addr_t ip_addr, struct net_device* dev) {
    uint32_t hash = arp_hash(ip_addr);
    arp_entry_t* oldest = NULL;
    uint32_t oldest_time = UINT32_MAX;
    
    /* Find free slot or oldest non-permanent entry */
    for (uint32_t i = 0; i < ARP_CACHE_SIZE; i++) {
        uint32_t idx = (hash + i) & (ARP_CACHE_SIZE - 1);
        arp_entry_t* entry = &arp_state.cache[idx];
        
        if (!entry->valid) {
            /* Free slot found */
            memset(entry, 0, sizeof(arp_entry_t));
            entry->ip_addr = ip_addr;
            entry->dev = dev;
            entry->valid = 1;
            entry->timestamp = get_ticks();
            skb_queue_head_init(&entry->pending);
            arp_state.count++;
            return entry;
        }
        
        /* Track oldest entry (excluding permanent) */
        if (!(entry->flags & ARP_FLAG_PERMANENT) && entry->timestamp < oldest_time) {
            oldest = entry;
            oldest_time = entry->timestamp;
        }
    }
    
    /* No free slots, evict oldest */
    if (oldest) {
        kprintf("[ARP] Evicting entry for %s\n",
                ip_addr_to_str(oldest->ip_addr, NULL, 0));
        
        /* Free pending queue */
        skb_queue_purge(&oldest->pending);
        
        /* Reuse entry */
        memset(oldest, 0, sizeof(arp_entry_t));
        oldest->ip_addr = ip_addr;
        oldest->dev = dev;
        oldest->valid = 1;
        oldest->timestamp = get_ticks();
        skb_queue_head_init(&oldest->pending);
        
        return oldest;
    }
    
    kprintf("[ARP] Cache full, cannot create entry\n");
    return NULL;
}

int arp_add_entry(ipv4_addr_t ip_addr, const uint8_t* mac_addr, 
                  struct net_device* dev, int permanent) {
    if (!mac_addr || !dev) return -1;
    
    arp_entry_t* entry = arp_lookup(ip_addr);
    if (!entry) {
        entry = arp_create_entry(ip_addr, dev);
        if (!entry) return -1;
    }
    
    memcpy(entry->mac_addr, mac_addr, 6);
    entry->flags = ARP_FLAG_COMPLETE;
    if (permanent) {
        entry->flags |= ARP_FLAG_PERMANENT;
    }
    entry->timestamp = get_ticks();
    
    kprintf("[ARP] Added entry: %s -> %02x:%02x:%02x:%02x:%02x:%02x%s\n",
            ip_addr_to_str(ip_addr, NULL, 0),
            mac_addr[0], mac_addr[1], mac_addr[2],
            mac_addr[3], mac_addr[4], mac_addr[5],
            permanent ? " (permanent)" : "");
    
    /* Send pending packets */
    if (entry->pending_count > 0) {
        kprintf("[ARP] Sending %u pending packets\n", entry->pending_count);
        
        struct sk_buff* skb;
        while ((skb = skb_dequeue(&entry->pending)) != NULL) {
            ethernet_send(dev, mac_addr, skb->protocol, skb);
        }
        
        entry->pending_count = 0;
    }
    
    return 0;
}

int arp_delete_entry(ipv4_addr_t ip_addr) {
    arp_entry_t* entry = arp_lookup(ip_addr);
    if (!entry) return -1;
    
    if (entry->flags & ARP_FLAG_PERMANENT) {
        kprintf("[ARP] Cannot delete permanent entry\n");
        return -1;
    }
    
    kprintf("[ARP] Deleting entry for %s\n",
            ip_addr_to_str(ip_addr, NULL, 0));
    
    skb_queue_purge(&entry->pending);
    entry->valid = 0;
    arp_state.count--;
    
    return 0;
}

void arp_flush_cache(void) {
    kprintf("[ARP] Flushing ARP cache\n");
    
    for (uint32_t i = 0; i < ARP_CACHE_SIZE; i++) {
        arp_entry_t* entry = &arp_state.cache[i];
        
        if (entry->valid && !(entry->flags & ARP_FLAG_PERMANENT)) {
            skb_queue_purge(&entry->pending);
            entry->valid = 0;
        }
    }
    
    /* Recount entries */
    arp_state.count = 0;
    for (uint32_t i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_state.cache[i].valid) {
            arp_state.count++;
        }
    }
}

void arp_age_cache(void) {
    uint32_t now = get_ticks();
    
    for (uint32_t i = 0; i < ARP_CACHE_SIZE; i++) {
        arp_entry_t* entry = &arp_state.cache[i];
        
        if (!entry->valid || (entry->flags & ARP_FLAG_PERMANENT)) {
            continue;
        }
        
        /* Check timeout */
        if (now - entry->timestamp > ARP_TIMEOUT) {
            kprintf("[ARP] Entry timed out: %s\n",
                    ip_addr_to_str(entry->ip_addr, NULL, 0));
            
            skb_queue_purge(&entry->pending);
            entry->valid = 0;
            arp_state.count--;
        }
    }
}

/* ==================== ARP Protocol Functions ==================== */

int arp_request(ipv4_addr_t target_ip, struct net_device* dev) {
    if (!dev) return -1;
    
    kprintf("[ARP] Sending ARP request for %s on %s\n",
            ip_addr_to_str(target_ip, NULL, 0), dev->name);
    
    /* Allocate skb */
    struct sk_buff* skb = alloc_skb(sizeof(arphdr_t), 0);
    if (!skb) return -1;
    
    skb_reserve(skb, sizeof(ethhdr_t) + 14);
    
    /* Build ARP packet */
    arphdr_t* arp = (arphdr_t*)skb_put(skb, sizeof(arphdr_t));
    
    arp->ar_hrd = htons(ARPHRD_ETHER);
    arp->ar_pro = htons(ETH_P_IP);
    arp->ar_hln = 6;
    arp->ar_pln = 4;
    arp->ar_op = htons(ARPOP_REQUEST);
    
    /* Sender: our MAC and IP */
    memcpy(arp->ar_sha, dev->dev_addr, 6);
    /* TODO: Get device IP address */
    uint32_t src_ip = htonl(ip_make_addr(192, 168, 1, 100));
    memcpy(arp->ar_sip, &src_ip, 4);
    
    /* Target: broadcast MAC, requested IP */
    memset(arp->ar_tha, 0xFF, 6);
    uint32_t dst_ip = htonl(target_ip);
    memcpy(arp->ar_tip, &dst_ip, 4);
    
    skb->protocol = htons(ETH_P_ARP);
    
    /* Send as Ethernet broadcast */
    static const uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    int ret = ethernet_send(dev, broadcast_mac, ETH_P_ARP, skb);
    
    if (ret == 0) {
        arp_state.requests_sent++;
    }
    
    return ret;
}

int arp_reply(ipv4_addr_t target_ip, const uint8_t* target_mac,
              struct net_device* dev) {
    if (!target_mac || !dev) return -1;
    
    kprintf("[ARP] Sending ARP reply for %s\n",
            ip_addr_to_str(target_ip, NULL, 0));
    
    /* Allocate skb */
    struct sk_buff* skb = alloc_skb(sizeof(arphdr_t), 0);
    if (!skb) return -1;
    
    skb_reserve(skb, sizeof(ethhdr_t) + 14);
    
    /* Build ARP reply */
    arphdr_t* arp = (arphdr_t*)skb_put(skb, sizeof(arphdr_t));
    
    arp->ar_hrd = htons(ARPHRD_ETHER);
    arp->ar_pro = htons(ETH_P_IP);
    arp->ar_hln = 6;
    arp->ar_pln = 4;
    arp->ar_op = htons(ARPOP_REPLY);
    
    /* Sender: our MAC and IP */
    memcpy(arp->ar_sha, dev->dev_addr, 6);
    uint32_t src_ip = htonl(ip_make_addr(192, 168, 1, 100));
    memcpy(arp->ar_sip, &src_ip, 4);
    
    /* Target: requester's MAC and IP */
    memcpy(arp->ar_tha, target_mac, 6);
    uint32_t dst_ip = htonl(target_ip);
    memcpy(arp->ar_tip, &dst_ip, 4);
    
    skb->protocol = htons(ETH_P_ARP);
    
    /* Send as unicast */
    int ret = ethernet_send(dev, target_mac, ETH_P_ARP, skb);
    
    if (ret == 0) {
        arp_state.replies_sent++;
    }
    
    return ret;
}

int arp_gratuitous(struct net_device* dev) {
    if (!dev) return -1;
    
    kprintf("[ARP] Sending gratuitous ARP on %s\n", dev->name);
    
    /* Gratuitous ARP: announce our IP to update others' caches */
    uint32_t our_ip = ip_make_addr(192, 168, 1, 100);
    
    return arp_request(our_ip, dev);
}

/* ==================== ARP Resolution ==================== */

int arp_resolve(ipv4_addr_t ip_addr, uint8_t* mac_addr, struct net_device* dev,
                struct sk_buff* skb) {
    if (!mac_addr || !dev) return -1;
    
    /* Check cache first */
    arp_entry_t* entry = arp_lookup(ip_addr);
    
    if (entry && (entry->flags & ARP_FLAG_COMPLETE)) {
        /* Cache hit */
        memcpy(mac_addr, entry->mac_addr, 6);
        return 0;
    }
    
    /* Cache miss - need to send ARP request */
    if (!entry) {
        entry = arp_create_entry(ip_addr, dev);
        if (!entry) {
            kprintf("[ARP] Failed to create entry\n");
            if (skb) free_skb(skb);
            return -1;
        }
    }
    
    /* Queue packet if provided */
    if (skb) {
        if (entry->pending_count < 10) {  /* Limit pending queue */
            skb_queue_tail(&entry->pending, skb);
            entry->pending_count++;
        } else {
            kprintf("[ARP] Pending queue full, dropping packet\n");
            free_skb(skb);
        }
    }
    
    /* Send ARP request if not already pending */
    if (!(entry->flags & ARP_FLAG_PENDING)) {
        entry->flags |= ARP_FLAG_PENDING;
        arp_request(ip_addr, dev);
    }
    
    return -2;  /* Resolution pending */
}

/* ==================== ARP Packet Reception ==================== */

void arp_rcv(struct sk_buff* skb) {
    if (!skb || skb->len < sizeof(arphdr_t)) {
        kprintf("[ARP] Packet too small\n");
        arp_state.invalid_packets++;
        free_skb(skb);
        return;
    }
    
    arphdr_t* arp = (arphdr_t*)skb->data;
    
    /* Verify hardware and protocol types */
    if (ntohs(arp->ar_hrd) != ARPHRD_ETHER ||
        ntohs(arp->ar_pro) != ETH_P_IP) {
        kprintf("[ARP] Unsupported hardware/protocol type\n");
        arp_state.invalid_packets++;
        free_skb(skb);
        return;
    }
    
    /* Verify address lengths */
    if (arp->ar_hln != 6 || arp->ar_pln != 4) {
        kprintf("[ARP] Invalid address length\n");
        arp_state.invalid_packets++;
        free_skb(skb);
        return;
    }
    
    /* Get addresses */
    ipv4_addr_t sender_ip, target_ip;
    memcpy(&sender_ip, arp->ar_sip, 4);
    sender_ip = ntohl(sender_ip);
    memcpy(&target_ip, arp->ar_tip, 4);
    target_ip = ntohl(target_ip);
    
    uint16_t op = ntohs(arp->ar_op);
    
    kprintf("[ARP] Received %s: %s (%02x:%02x:%02x:%02x:%02x:%02x) -> %s\n",
            op == ARPOP_REQUEST ? "REQUEST" : "REPLY",
            ip_addr_to_str(sender_ip, NULL, 0),
            arp->ar_sha[0], arp->ar_sha[1], arp->ar_sha[2],
            arp->ar_sha[3], arp->ar_sha[4], arp->ar_sha[5],
            ip_addr_to_str(target_ip, NULL, 0));
    
    /* Update cache with sender's info (unsolicited learning) */
    arp_add_entry(sender_ip, arp->ar_sha, skb->dev, 0);
    
    switch (op) {
        case ARPOP_REQUEST:
            arp_state.requests_rcvd++;
            
            /* Check if request is for us */
            /* TODO: Check against device IP addresses */
            uint32_t our_ip = ip_make_addr(192, 168, 1, 100);
            
            if (target_ip == our_ip) {
                /* Send ARP reply */
                arp_reply(sender_ip, arp->ar_sha, skb->dev);
            }
            break;
            
        case ARPOP_REPLY:
            arp_state.replies_rcvd++;
            /* Cache already updated above */
            break;
            
        default:
            kprintf("[ARP] Unknown operation: %u\n", op);
            arp_state.invalid_packets++;
            break;
    }
    
    free_skb(skb);
}

/* ==================== ARP Table Display ==================== */

void arp_dump_cache(void) {
    kprintf("[ARP] ARP Cache (%u entries):\n", arp_state.count);
    kprintf("  IP Address       MAC Address         Device    Flags  Age\n");
    
    uint32_t now = get_ticks();
    
    for (uint32_t i = 0; i < ARP_CACHE_SIZE; i++) {
        arp_entry_t* entry = &arp_state.cache[i];
        
        if (!entry->valid) continue;
        
        char ip_str[16];
        ip_addr_to_str(entry->ip_addr, ip_str, sizeof(ip_str));
        
        uint32_t age = (now - entry->timestamp) / 100;  /* seconds */
        
        kprintf("  %-15s  %02x:%02x:%02x:%02x:%02x:%02x  %-8s  %c%c%c   %us\n",
                ip_str,
                entry->mac_addr[0], entry->mac_addr[1], entry->mac_addr[2],
                entry->mac_addr[3], entry->mac_addr[4], entry->mac_addr[5],
                entry->dev ? entry->dev->name : "none",
                (entry->flags & ARP_FLAG_COMPLETE) ? 'C' : '-',
                (entry->flags & ARP_FLAG_PENDING) ? 'P' : '-',
                (entry->flags & ARP_FLAG_PERMANENT) ? 'M' : '-',
                age);
    }
}

void arp_get_stats(arp_stats_t* stats) {
    if (!stats) return;
    
    stats->requests_sent = arp_state.requests_sent;
    stats->requests_rcvd = arp_state.requests_rcvd;
    stats->replies_sent = arp_state.replies_sent;
    stats->replies_rcvd = arp_state.replies_rcvd;
    stats->cache_hits = arp_state.cache_hits;
    stats->cache_misses = arp_state.cache_misses;
    stats->cache_entries = arp_state.count;
}

void arp_dump_stats(void) {
    kprintf("[ARP] Statistics:\n");
    kprintf("  Requests:  sent=%llu rcvd=%llu\n",
            arp_state.requests_sent, arp_state.requests_rcvd);
    kprintf("  Replies:   sent=%llu rcvd=%llu\n",
            arp_state.replies_sent, arp_state.replies_rcvd);
    kprintf("  Cache:     hits=%llu misses=%llu entries=%u\n",
            arp_state.cache_hits, arp_state.cache_misses, arp_state.count);
    kprintf("  Invalid packets: %llu\n", arp_state.invalid_packets);
}

/* ==================== Initialization ==================== */

int arp_init(void) {
    kprintf("[ARP] Initializing ARP protocol...\n");
    
    /* Clear cache */
    memset(&arp_state, 0, sizeof(arp_state));
    
    /* Initialize pending queues */
    for (uint32_t i = 0; i < ARP_CACHE_SIZE; i++) {
        skb_queue_head_init(&arp_state.cache[i].pending);
    }
    
    kprintf("[ARP] ARP protocol initialized (cache size: %u)\n", ARP_CACHE_SIZE);
    
    return 0;
}

void arp_cleanup(void) {
    kprintf("[ARP] Cleaning up ARP protocol...\n");
    
    /* Free all pending packets */
    for (uint32_t i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_state.cache[i].valid) {
            skb_queue_purge(&arp_state.cache[i].pending);
        }
    }
    
    arp_dump_stats();
}
