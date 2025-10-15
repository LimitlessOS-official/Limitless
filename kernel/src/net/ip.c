/*
 * IP (Internet Protocol) Layer Implementation
 * 
 * Production-grade IPv4 implementation with routing, fragmentation,
 * and reassembly support.
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/ip.h"
#include "net/skbuff.h"
#include "net/netdevice.h"
#include "kernel.h"
#include <string.h>

/* IP statistics */
static struct {
    uint64_t in_receives;
    uint64_t in_delivers;
    uint64_t in_discards;
    uint64_t in_hdr_errors;
    uint64_t out_requests;
    uint64_t out_discards;
    uint64_t out_no_routes;
    uint64_t frag_oks;
    uint64_t frag_fails;
    uint64_t frag_creates;
    uint64_t reasm_oks;
    uint64_t reasm_fails;
} ip_stats;

/* Routing table */
#define MAX_ROUTES 256
static struct {
    ipv4_route_t routes[MAX_ROUTES];
    uint32_t count;
} ip_routing_table;

/* Fragment reassembly queue */
#define MAX_FRAGS 64
static struct {
    ip_frag_t frags[MAX_FRAGS];
    uint32_t count;
} ip_frag_queue;

/* Protocol handlers */
#define MAX_PROTOCOLS 256
static ip_protocol_handler_t ip_protocol_handlers[MAX_PROTOCOLS];

/* ==================== IP Address Utilities ==================== */

ipv4_addr_t ip_make_addr(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    return ((uint32_t)a << 24) | ((uint32_t)b << 16) | 
           ((uint32_t)c << 8) | (uint32_t)d;
}

void ip_addr_to_bytes(ipv4_addr_t addr, uint8_t bytes[4]) {
    bytes[0] = (addr >> 24) & 0xFF;
    bytes[1] = (addr >> 16) & 0xFF;
    bytes[2] = (addr >> 8) & 0xFF;
    bytes[3] = addr & 0xFF;
}

char* ip_addr_to_str(ipv4_addr_t addr, char* buf, uint32_t buflen) {
    static char static_buf[16];
    if (!buf) {
        buf = static_buf;
        buflen = sizeof(static_buf);
    }
    
    uint8_t bytes[4];
    ip_addr_to_bytes(addr, bytes);
    
    snprintf(buf, buflen, "%u.%u.%u.%u", bytes[0], bytes[1], bytes[2], bytes[3]);
    return buf;
}

ipv4_addr_t ip_str_to_addr(const char* str) {
    if (!str) return 0;
    
    uint32_t a = 0, b = 0, c = 0, d = 0;
    
    /* Simple parsing */
    int count = 0;
    uint32_t* current = &a;
    
    for (const char* p = str; *p; p++) {
        if (*p >= '0' && *p <= '9') {
            *current = (*current * 10) + (*p - '0');
        } else if (*p == '.') {
            count++;
            if (count == 1) current = &b;
            else if (count == 2) current = &c;
            else if (count == 3) current = &d;
        }
    }
    
    if (count != 3) return 0;
    
    return ip_make_addr(a, b, c, d);
}

int ip_addr_is_local(ipv4_addr_t addr) {
    /* Check if address is on any local interface */
    /* TODO: Implement interface lookup */
    return 0;
}

int ip_addr_is_broadcast(ipv4_addr_t addr) {
    return addr == IP_ADDR_BROADCAST;
}

int ip_addr_is_multicast(ipv4_addr_t addr) {
    return (addr & 0xF0000000) == 0xE0000000;  /* 224.0.0.0/4 */
}

/* ==================== IP Header Functions ==================== */

uint16_t ip_checksum(const void* data, uint32_t len) {
    const uint16_t* p = (const uint16_t*)data;
    uint32_t sum = 0;
    
    /* Sum all 16-bit words */
    while (len > 1) {
        sum += *p++;
        len -= 2;
    }
    
    /* Add odd byte if present */
    if (len == 1) {
        sum += *((const uint8_t*)p);
    }
    
    /* Fold 32-bit sum to 16 bits */
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

int ip_verify_checksum(const iphdr_t* iph) {
    uint16_t saved_check = iph->check;
    ((iphdr_t*)iph)->check = 0;
    
    uint16_t computed = ip_checksum(iph, iph->ihl * 4);
    ((iphdr_t*)iph)->check = saved_check;
    
    return computed == saved_check;
}

/* ==================== Routing ==================== */

int ip_route_add(ipv4_addr_t dest, ipv4_addr_t mask, ipv4_addr_t gateway, 
                 struct net_device* dev, uint32_t metric) {
    if (ip_routing_table.count >= MAX_ROUTES) {
        kprintf("[IP] Routing table full\n");
        return -1;
    }
    
    ipv4_route_t* route = &ip_routing_table.routes[ip_routing_table.count++];
    route->dest = dest;
    route->mask = mask;
    route->gateway = gateway;
    route->dev = dev;
    route->metric = metric;
    route->flags = 0;
    
    if (gateway == 0) {
        route->flags |= IP_ROUTE_LOCAL;
    }
    
    kprintf("[IP] Added route: %s/%s via %s metric %u\n",
            ip_addr_to_str(dest, NULL, 0),
            ip_addr_to_str(mask, NULL, 0),
            gateway ? ip_addr_to_str(gateway, NULL, 0) : "direct",
            metric);
    
    return 0;
}

ipv4_route_t* ip_route_lookup(ipv4_addr_t dest) {
    ipv4_route_t* best = NULL;
    uint32_t best_mask = 0;
    
    /* Find most specific matching route (longest prefix match) */
    for (uint32_t i = 0; i < ip_routing_table.count; i++) {
        ipv4_route_t* route = &ip_routing_table.routes[i];
        
        if ((dest & route->mask) == (route->dest & route->mask)) {
            /* Match found */
            uint32_t bits = __builtin_popcount(route->mask);
            if (bits > best_mask) {
                best = route;
                best_mask = bits;
            }
        }
    }
    
    return best;
}

int ip_route_del(ipv4_addr_t dest, ipv4_addr_t mask) {
    for (uint32_t i = 0; i < ip_routing_table.count; i++) {
        ipv4_route_t* route = &ip_routing_table.routes[i];
        
        if (route->dest == dest && route->mask == mask) {
            /* Remove route by shifting */
            for (uint32_t j = i; j < ip_routing_table.count - 1; j++) {
                ip_routing_table.routes[j] = ip_routing_table.routes[j + 1];
            }
            ip_routing_table.count--;
            
            kprintf("[IP] Removed route: %s/%s\n",
                    ip_addr_to_str(dest, NULL, 0),
                    ip_addr_to_str(mask, NULL, 0));
            
            return 0;
        }
    }
    
    return -1;
}

void ip_route_dump(void) {
    kprintf("[IP] Routing table (%u entries):\n", ip_routing_table.count);
    kprintf("  Destination     Gateway         Mask            Metric  Dev\n");
    
    for (uint32_t i = 0; i < ip_routing_table.count; i++) {
        ipv4_route_t* route = &ip_routing_table.routes[i];
        
        char dest[16], gw[16], mask[16];
        ip_addr_to_str(route->dest, dest, sizeof(dest));
        ip_addr_to_str(route->gateway, gw, sizeof(gw));
        ip_addr_to_str(route->mask, mask, sizeof(mask));
        
        kprintf("  %-15s %-15s %-15s %-7u %s\n",
                dest,
                route->gateway ? gw : "*",
                mask,
                route->metric,
                route->dev ? route->dev->name : "none");
    }
}

/* ==================== IP Transmission ==================== */

int ip_send(ipv4_addr_t daddr, struct sk_buff* skb) {
    if (!skb) return -1;
    
    /* Look up route */
    ipv4_route_t* route = ip_route_lookup(daddr);
    if (!route) {
        kprintf("[IP] No route to host %s\n", ip_addr_to_str(daddr, NULL, 0));
        ip_stats.out_no_routes++;
        free_skb(skb);
        return -1;
    }
    
    /* Get source address from device */
    /* TODO: Get actual device address */
    ipv4_addr_t saddr = ip_make_addr(192, 168, 1, 100);
    
    /* Build IP header */
    iphdr_t* iph = (iphdr_t*)skb_push(skb, sizeof(iphdr_t));
    memset(iph, 0, sizeof(iphdr_t));
    
    iph->version = 4;
    iph->ihl = 5;  /* 20 bytes */
    iph->tos = 0;
    iph->tot_len = htons(skb->len);
    iph->id = htons(ip_get_next_id());
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = skb->protocol;
    iph->saddr = htonl(saddr);
    iph->daddr = htonl(daddr);
    
    /* Calculate checksum */
    iph->check = 0;
    iph->check = ip_checksum(iph, sizeof(iphdr_t));
    
    /* Set skb metadata */
    skb->nh.raw = (uint8_t*)iph;
    
    kprintf("[IP] Sending packet: %s -> %s proto=%u len=%u\n",
            ip_addr_to_str(saddr, NULL, 0),
            ip_addr_to_str(daddr, NULL, 0),
            skb->protocol,
            skb->len);
    
    /* Check if fragmentation needed */
    if (skb->len > route->dev->mtu) {
        return ip_fragment(skb, route->dev);
    }
    
    /* Send to link layer */
    ipv4_addr_t next_hop = route->gateway ? route->gateway : daddr;
    int ret = ip_output(skb, route->dev, next_hop);
    
    if (ret == 0) {
        ip_stats.out_requests++;
    } else {
        ip_stats.out_discards++;
    }
    
    return ret;
}

int ip_output(struct sk_buff* skb, struct net_device* dev, ipv4_addr_t next_hop) {
    if (!skb || !dev) return -1;
    
    /* Resolve next hop to MAC address (via ARP) */
    /* For now, just pass to device */
    skb->dev = dev;
    
    /* TODO: ARP resolution */
    /* arp_resolve(next_hop, dev, skb); */
    
    /* For now, just send directly */
    return netdev_start_xmit(skb, dev);
}

/* ==================== IP Reception ==================== */

void ip_rcv(struct sk_buff* skb) {
    if (!skb) return;
    
    ip_stats.in_receives++;
    
    /* Verify minimum length */
    if (skb->len < sizeof(iphdr_t)) {
        kprintf("[IP] Packet too small\n");
        ip_stats.in_hdr_errors++;
        free_skb(skb);
        return;
    }
    
    /* Get IP header */
    iphdr_t* iph = (iphdr_t*)skb->data;
    
    /* Verify version */
    if (iph->version != 4) {
        kprintf("[IP] Invalid IP version: %u\n", iph->version);
        ip_stats.in_hdr_errors++;
        free_skb(skb);
        return;
    }
    
    /* Verify header length */
    if (iph->ihl < 5) {
        kprintf("[IP] Invalid header length: %u\n", iph->ihl);
        ip_stats.in_hdr_errors++;
        free_skb(skb);
        return;
    }
    
    /* Verify checksum */
    if (!ip_verify_checksum(iph)) {
        kprintf("[IP] Checksum failed\n");
        ip_stats.in_hdr_errors++;
        free_skb(skb);
        return;
    }
    
    /* Get addresses */
    ipv4_addr_t saddr = ntohl(iph->saddr);
    ipv4_addr_t daddr = ntohl(iph->daddr);
    
    kprintf("[IP] Received packet: %s -> %s proto=%u len=%u\n",
            ip_addr_to_str(saddr, NULL, 0),
            ip_addr_to_str(daddr, NULL, 0),
            iph->protocol,
            ntohs(iph->tot_len));
    
    /* Check if packet is for us */
    if (!ip_addr_is_local(daddr) && 
        !ip_addr_is_broadcast(daddr) && 
        !ip_addr_is_multicast(daddr)) {
        /* Not for us, forward if routing enabled */
        kprintf("[IP] Packet not for us, dropping\n");
        ip_stats.in_discards++;
        free_skb(skb);
        return;
    }
    
    /* Handle fragmentation */
    if (iph->frag_off & htons(IP_MF | IP_OFFMASK)) {
        skb = ip_defragment(skb);
        if (!skb) {
            /* Waiting for more fragments */
            return;
        }
        iph = (iphdr_t*)skb->data;
    }
    
    /* Set network header pointer */
    skb->nh.raw = (uint8_t*)iph;
    
    /* Strip IP header */
    skb_pull(skb, iph->ihl * 4);
    skb->h.raw = skb->data;
    
    /* Deliver to protocol handler */
    ip_protocol_handler_t handler = ip_protocol_handlers[iph->protocol];
    if (handler) {
        handler(skb);
        ip_stats.in_delivers++;
    } else {
        kprintf("[IP] No handler for protocol %u\n", iph->protocol);
        ip_stats.in_discards++;
        free_skb(skb);
    }
}

/* ==================== Fragmentation ==================== */

static uint16_t ip_id_counter = 0;

uint16_t ip_get_next_id(void) {
    return ip_id_counter++;
}

int ip_fragment(struct sk_buff* skb, struct net_device* dev) {
    if (!skb || !dev) return -1;
    
    iphdr_t* iph = (iphdr_t*)skb->nh.raw;
    uint32_t mtu = dev->mtu;
    uint32_t hlen = iph->ihl * 4;
    uint32_t data_len = skb->len - hlen;
    uint32_t frag_size = (mtu - hlen) & ~7;  /* Must be multiple of 8 */
    
    if (frag_size == 0) {
        kprintf("[IP] MTU too small for fragmentation\n");
        ip_stats.frag_fails++;
        free_skb(skb);
        return -1;
    }
    
    kprintf("[IP] Fragmenting packet: len=%u mtu=%u frag_size=%u\n",
            skb->len, mtu, frag_size);
    
    uint32_t offset = 0;
    uint16_t id = ip_get_next_id();
    
    while (data_len > 0) {
        uint32_t chunk = (data_len > frag_size) ? frag_size : data_len;
        int more_frags = (data_len > frag_size) ? 1 : 0;
        
        /* Create fragment */
        struct sk_buff* frag = alloc_skb(hlen + chunk, 0);
        if (!frag) {
            kprintf("[IP] Failed to allocate fragment\n");
            ip_stats.frag_fails++;
            return -1;
        }
        
        /* Copy IP header */
        memcpy(skb_put(frag, hlen), iph, hlen);
        
        /* Copy data */
        memcpy(skb_put(frag, chunk), skb->data + hlen + offset, chunk);
        
        /* Update IP header */
        iphdr_t* frag_iph = (iphdr_t*)frag->data;
        frag_iph->tot_len = htons(hlen + chunk);
        frag_iph->id = htons(id);
        frag_iph->frag_off = htons((offset / 8) | (more_frags ? IP_MF : 0));
        
        /* Recalculate checksum */
        frag_iph->check = 0;
        frag_iph->check = ip_checksum(frag_iph, hlen);
        
        /* Send fragment */
        frag->dev = dev;
        netdev_start_xmit(frag, dev);
        
        ip_stats.frag_creates++;
        
        offset += chunk;
        data_len -= chunk;
    }
    
    ip_stats.frag_oks++;
    free_skb(skb);
    
    return 0;
}

struct sk_buff* ip_defragment(struct sk_buff* skb) {
    if (!skb) return NULL;
    
    iphdr_t* iph = (iphdr_t*)skb->data;
    uint16_t id = ntohs(iph->id);
    uint16_t frag_off = ntohs(iph->frag_off);
    int more_frags = frag_off & IP_MF;
    uint16_t offset = (frag_off & IP_OFFMASK) * 8;
    
    kprintf("[IP] Fragment: id=%u offset=%u more=%d\n", id, offset, more_frags);
    
    /* Find existing fragment queue */
    ip_frag_t* frag_entry = NULL;
    for (uint32_t i = 0; i < ip_frag_queue.count; i++) {
        if (ip_frag_queue.frags[i].id == id &&
            ip_frag_queue.frags[i].saddr == ntohl(iph->saddr) &&
            ip_frag_queue.frags[i].daddr == ntohl(iph->daddr)) {
            frag_entry = &ip_frag_queue.frags[i];
            break;
        }
    }
    
    /* Create new fragment queue if needed */
    if (!frag_entry) {
        if (ip_frag_queue.count >= MAX_FRAGS) {
            kprintf("[IP] Fragment queue full\n");
            ip_stats.reasm_fails++;
            free_skb(skb);
            return NULL;
        }
        
        frag_entry = &ip_frag_queue.frags[ip_frag_queue.count++];
        frag_entry->id = id;
        frag_entry->saddr = ntohl(iph->saddr);
        frag_entry->daddr = ntohl(iph->daddr);
        frag_entry->total_len = 0;
        frag_entry->received = 0;
        frag_entry->timestamp = get_ticks();
        skb_queue_head_init(&frag_entry->fragments);
    }
    
    /* Add fragment to queue */
    skb_queue_tail(&frag_entry->fragments, skb);
    frag_entry->received++;
    
    if (!more_frags) {
        /* Last fragment - we know total length */
        uint32_t hlen = iph->ihl * 4;
        frag_entry->total_len = offset + (skb->len - hlen);
    }
    
    /* Check if reassembly complete */
    if (frag_entry->total_len > 0 && ip_check_complete(frag_entry)) {
        /* Reassemble packet */
        struct sk_buff* complete = ip_reassemble(frag_entry);
        
        /* Remove from queue */
        for (uint32_t i = 0; i < ip_frag_queue.count; i++) {
            if (&ip_frag_queue.frags[i] == frag_entry) {
                for (uint32_t j = i; j < ip_frag_queue.count - 1; j++) {
                    ip_frag_queue.frags[j] = ip_frag_queue.frags[j + 1];
                }
                ip_frag_queue.count--;
                break;
            }
        }
        
        ip_stats.reasm_oks++;
        return complete;
    }
    
    /* Waiting for more fragments */
    return NULL;
}

int ip_check_complete(ip_frag_t* frag) {
    /* Check if we have all fragments from 0 to total_len */
    uint32_t covered = 0;
    
    struct sk_buff* skb = frag->fragments.next;
    while (skb != (struct sk_buff*)&frag->fragments) {
        iphdr_t* iph = (iphdr_t*)skb->data;
        uint16_t frag_off = ntohs(iph->frag_off);
        uint16_t offset = (frag_off & IP_OFFMASK) * 8;
        uint32_t hlen = iph->ihl * 4;
        uint32_t len = skb->len - hlen;
        
        /* Check if this fragment fills a gap */
        if (offset <= covered) {
            covered = offset + len;
        }
        
        skb = skb->next;
    }
    
    return covered >= frag->total_len;
}

struct sk_buff* ip_reassemble(ip_frag_t* frag) {
    /* Allocate new skb for complete packet */
    struct sk_buff* complete = alloc_skb(frag->total_len + 20, 0);  /* +20 for IP header */
    if (!complete) {
        kprintf("[IP] Failed to allocate reassembly buffer\n");
        ip_stats.reasm_fails++;
        return NULL;
    }
    
    /* Copy first fragment's IP header */
    struct sk_buff* first = frag->fragments.next;
    iphdr_t* first_iph = (iphdr_t*)first->data;
    memcpy(skb_put(complete, first_iph->ihl * 4), first_iph, first_iph->ihl * 4);
    
    /* Copy all fragment data in order */
    struct sk_buff* skb = first;
    while (skb != (struct sk_buff*)&frag->fragments) {
        iphdr_t* iph = (iphdr_t*)skb->data;
        uint16_t frag_off = ntohs(iph->frag_off);
        uint16_t offset = (frag_off & IP_OFFMASK) * 8;
        uint32_t hlen = iph->ihl * 4;
        uint32_t len = skb->len - hlen;
        
        memcpy(complete->data + 20 + offset, skb->data + hlen, len);
        
        skb = skb->next;
    }
    
    /* Update IP header */
    iphdr_t* complete_iph = (iphdr_t*)complete->data;
    complete_iph->tot_len = htons(20 + frag->total_len);
    complete_iph->frag_off = 0;
    complete_iph->check = 0;
    complete_iph->check = ip_checksum(complete_iph, 20);
    
    /* Free fragment queue */
    skb_queue_purge(&frag->fragments);
    
    kprintf("[IP] Reassembled packet: len=%u\n", 20 + frag->total_len);
    
    return complete;
}

/* ==================== Protocol Registration ==================== */

int ip_register_protocol(uint8_t protocol, ip_protocol_handler_t handler) {
    if (ip_protocol_handlers[protocol]) {
        kprintf("[IP] Protocol %u already registered\n", protocol);
        return -1;
    }
    
    ip_protocol_handlers[protocol] = handler;
    kprintf("[IP] Registered protocol %u\n", protocol);
    
    return 0;
}

void ip_unregister_protocol(uint8_t protocol) {
    ip_protocol_handlers[protocol] = NULL;
    kprintf("[IP] Unregistered protocol %u\n", protocol);
}

/* ==================== Initialization ==================== */

int ip_init(void) {
    kprintf("[IP] Initializing IP layer...\n");
    
    /* Initialize statistics */
    memset(&ip_stats, 0, sizeof(ip_stats));
    
    /* Initialize routing table */
    ip_routing_table.count = 0;
    
    /* Add default routes */
    /* Loopback */
    ip_route_add(ip_make_addr(127, 0, 0, 0),
                 ip_make_addr(255, 0, 0, 0),
                 0, NULL, 1);
    
    /* Initialize fragment queue */
    ip_frag_queue.count = 0;
    
    /* Clear protocol handlers */
    memset(ip_protocol_handlers, 0, sizeof(ip_protocol_handlers));
    
    kprintf("[IP] IP layer initialized\n");
    
    return 0;
}

void ip_get_stats(ip_stats_t* stats) {
    if (!stats) return;
    
    stats->in_receives = ip_stats.in_receives;
    stats->in_delivers = ip_stats.in_delivers;
    stats->in_discards = ip_stats.in_discards;
    stats->in_hdr_errors = ip_stats.in_hdr_errors;
    stats->out_requests = ip_stats.out_requests;
    stats->out_discards = ip_stats.out_discards;
    stats->out_no_routes = ip_stats.out_no_routes;
    stats->frag_oks = ip_stats.frag_oks;
    stats->frag_fails = ip_stats.frag_fails;
    stats->frag_creates = ip_stats.frag_creates;
    stats->reasm_oks = ip_stats.reasm_oks;
    stats->reasm_fails = ip_stats.reasm_fails;
}
