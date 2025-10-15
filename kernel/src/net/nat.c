/*
 * NAT Implementation
 * Network Address Translation with Port Mapping
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/nat.h"
#include "net/ip.h"
#include "net/tcp.h"
#include "net/udp.h"
#include "net/sk_buff.h"
#include "kernel/printk.h"
#include "kernel/string.h"
#include "kernel/stdlib.h"

/* NAT connection hash table */
static nat_entry_t* nat_table[NAT_TABLE_SIZE] = {0};

/* NAT rules list */
static nat_rule_t* nat_rules = NULL;

/* Global NAT statistics */
static nat_stats_t nat_stats = {0};

/* Next available port for SNAT */
static uint16_t next_snat_port = 1024;

/* Connection timeout (5 minutes for TCP established, 30 seconds for UDP) */
#define NAT_TIMEOUT_TCP_ESTABLISHED  300000  /* 5 minutes */
#define NAT_TIMEOUT_TCP_CLOSING       30000  /* 30 seconds */
#define NAT_TIMEOUT_UDP               30000  /* 30 seconds */

/*
 * Hash function for NAT table
 */
static uint32_t nat_hash(uint32_t src_ip, uint32_t dst_ip,
                        uint16_t src_port, uint16_t dst_port,
                        uint8_t protocol) {
    uint32_t hash = src_ip ^ dst_ip ^ ((uint32_t)src_port << 16) ^ dst_port ^ protocol;
    return hash % NAT_TABLE_SIZE;
}

/*
 * Initialize NAT
 */
int nat_init(void) {
    memset(nat_table, 0, sizeof(nat_table));
    memset(&nat_stats, 0, sizeof(nat_stats));
    nat_rules = NULL;
    next_snat_port = 1024;
    printk(KERN_INFO "NAT initialized\n");
    return 0;
}

/*
 * Add NAT rule
 * 
 * @rule: NAT rule to add
 * @return: 0 on success, negative on error
 */
int nat_add_rule(nat_rule_t* rule) {
    nat_rule_t* new_rule;
    
    if (!rule) {
        return -1;
    }
    
    /* Allocate and copy rule */
    new_rule = (nat_rule_t*)malloc(sizeof(nat_rule_t));
    if (!new_rule) {
        return -1;
    }
    
    memcpy(new_rule, rule, sizeof(nat_rule_t));
    
    /* Add to head of list */
    new_rule->next = nat_rules;
    nat_rules = new_rule;
    
    printk(KERN_INFO "NAT: Added %s rule\n",
           rule->nat_type == NAT_TYPE_SNAT ? "SNAT" : "DNAT");
    
    return 0;
}

/*
 * Delete NAT rule
 * 
 * @rule: NAT rule to delete
 * @return: 0 on success, negative on error
 */
int nat_delete_rule(nat_rule_t* rule) {
    nat_rule_t** prev = &nat_rules;
    nat_rule_t* curr = nat_rules;
    
    if (!rule) {
        return -1;
    }
    
    while (curr) {
        if (curr == rule) {
            *prev = curr->next;
            free(curr);
            return 0;
        }
        prev = &curr->next;
        curr = curr->next;
    }
    
    return -1;
}

/*
 * Flush all NAT rules
 */
void nat_flush_rules(void) {
    nat_rule_t* rule = nat_rules;
    nat_rule_t* next;
    
    while (rule) {
        next = rule->next;
        free(rule);
        rule = next;
    }
    
    nat_rules = NULL;
}

/*
 * Find NAT entry in table
 * 
 * @src_ip: Source IP address
 * @dst_ip: Destination IP address
 * @src_port: Source port
 * @dst_port: Destination port
 * @protocol: Protocol (TCP/UDP)
 * @return: NAT entry or NULL if not found
 */
nat_entry_t* nat_find_entry(uint32_t src_ip, uint32_t dst_ip,
                            uint16_t src_port, uint16_t dst_port,
                            uint8_t protocol) {
    uint32_t hash = nat_hash(src_ip, dst_ip, src_port, dst_port, protocol);
    nat_entry_t* entry = nat_table[hash];
    
    while (entry) {
        if (entry->orig_src_ip == src_ip &&
            entry->orig_dst_ip == dst_ip &&
            entry->orig_src_port == src_port &&
            entry->orig_dst_port == dst_port &&
            entry->protocol == protocol) {
            return entry;
        }
        entry = entry->next;
    }
    
    return NULL;
}

/*
 * Allocate port for SNAT
 */
static uint16_t nat_alloc_port(void) {
    uint16_t port = next_snat_port++;
    if (next_snat_port >= 65535) {
        next_snat_port = 1024;
    }
    return port;
}

/*
 * Create NAT entry
 * 
 * @src_ip: Source IP address
 * @dst_ip: Destination IP address
 * @src_port: Source port
 * @dst_port: Destination port
 * @protocol: Protocol (TCP/UDP)
 * @nat_type: NAT type (SNAT/DNAT)
 * @return: NAT entry or NULL on error
 */
nat_entry_t* nat_create_entry(uint32_t src_ip, uint32_t dst_ip,
                              uint16_t src_port, uint16_t dst_port,
                              uint8_t protocol, uint8_t nat_type) {
    uint32_t hash;
    nat_entry_t* entry;
    nat_rule_t* rule;
    
    /* Find matching rule */
    for (rule = nat_rules; rule; rule = rule->next) {
        if (rule->nat_type != nat_type) {
            continue;
        }
        
        /* Check if packet matches rule */
        if (rule->match_protocol && rule->match_protocol != protocol) {
            continue;
        }
        
        if ((src_ip & rule->match_src_mask) != (rule->match_src_ip & rule->match_src_mask)) {
            continue;
        }
        
        if ((dst_ip & rule->match_dst_mask) != (rule->match_dst_ip & rule->match_dst_mask)) {
            continue;
        }
        
        /* Found matching rule - create entry */
        entry = (nat_entry_t*)malloc(sizeof(nat_entry_t));
        if (!entry) {
            return NULL;
        }
        
        memset(entry, 0, sizeof(nat_entry_t));
        
        /* Original tuple */
        entry->orig_src_ip = src_ip;
        entry->orig_dst_ip = dst_ip;
        entry->orig_src_port = src_port;
        entry->orig_dst_port = dst_port;
        entry->protocol = protocol;
        
        /* Translated tuple */
        if (nat_type == NAT_TYPE_SNAT) {
            entry->nat_src_ip = rule->nat_ip;
            entry->nat_dst_ip = dst_ip;
            entry->nat_src_port = nat_alloc_port();
            entry->nat_dst_port = dst_port;
        } else {  /* DNAT */
            entry->nat_src_ip = src_ip;
            entry->nat_dst_ip = rule->nat_ip;
            entry->nat_src_port = src_port;
            entry->nat_dst_port = rule->nat_port_min;  /* Simplified: use first port */
        }
        
        entry->state = NAT_STATE_NEW;
        entry->nat_type = nat_type;
        entry->flags = rule->flags;
        
        /* Set timeout */
        if (protocol == IPPROTO_TCP) {
            entry->timeout = NAT_TIMEOUT_TCP_ESTABLISHED;
        } else {
            entry->timeout = NAT_TIMEOUT_UDP;
        }
        
        /* Add to hash table */
        hash = nat_hash(src_ip, dst_ip, src_port, dst_port, protocol);
        entry->next = nat_table[hash];
        nat_table[hash] = entry;
        
        nat_stats.connections++;
        nat_stats.connections_total++;
        
        return entry;
    }
    
    return NULL;  /* No matching rule */
}

/*
 * Delete NAT entry
 * 
 * @entry: NAT entry to delete
 */
void nat_delete_entry(nat_entry_t* entry) {
    uint32_t hash;
    nat_entry_t** prev;
    nat_entry_t* curr;
    
    if (!entry) {
        return;
    }
    
    hash = nat_hash(entry->orig_src_ip, entry->orig_dst_ip,
                   entry->orig_src_port, entry->orig_dst_port,
                   entry->protocol);
    
    prev = &nat_table[hash];
    curr = nat_table[hash];
    
    while (curr) {
        if (curr == entry) {
            *prev = curr->next;
            free(curr);
            nat_stats.connections--;
            return;
        }
        prev = &curr->next;
        curr = curr->next;
    }
}

/*
 * Translate outbound packet (SNAT)
 * 
 * @skb: Socket buffer
 * @return: 0 on success, negative on error
 */
int nat_translate_outbound(struct sk_buff* skb) {
    iphdr_t* iph;
    tcphdr_t* tcph;
    udphdr_t* udph;
    nat_entry_t* entry;
    uint32_t src_ip, dst_ip;
    uint16_t src_port = 0, dst_port = 0;
    uint8_t protocol;
    
    if (!skb || skb->len < sizeof(iphdr_t)) {
        return -1;
    }
    
    iph = (iphdr_t*)skb->data;
    protocol = iph->protocol;
    src_ip = ntohl(iph->saddr);
    dst_ip = ntohl(iph->daddr);
    
    /* Extract port numbers */
    if (protocol == IPPROTO_TCP && skb->len >= sizeof(iphdr_t) + sizeof(tcphdr_t)) {
        tcph = (tcphdr_t*)(skb->data + (iph->ihl * 4));
        src_port = ntohs(tcph->source);
        dst_port = ntohs(tcph->dest);
    } else if (protocol == IPPROTO_UDP && skb->len >= sizeof(iphdr_t) + sizeof(udphdr_t)) {
        udph = (udphdr_t*)(skb->data + (iph->ihl * 4));
        src_port = ntohs(udph->source);
        dst_port = ntohs(udph->dest);
    }
    
    /* Find or create NAT entry */
    entry = nat_find_entry(src_ip, dst_ip, src_port, dst_port, protocol);
    if (!entry) {
        entry = nat_create_entry(src_ip, dst_ip, src_port, dst_port, protocol, NAT_TYPE_SNAT);
        if (!entry) {
            return 0;  /* No matching rule */
        }
    }
    
    /* Update state */
    entry->state = NAT_STATE_ESTABLISHED;
    entry->last_seen = 0;  /* TODO: get current time */
    entry->packets++;
    entry->bytes += skb->len;
    
    /* Translate source IP and port */
    iph->saddr = htonl(entry->nat_src_ip);
    
    if (protocol == IPPROTO_TCP) {
        tcph = (tcphdr_t*)(skb->data + (iph->ihl * 4));
        tcph->source = htons(entry->nat_src_port);
        /* Recalculate TCP checksum */
        tcph->check = 0;
        tcph->check = tcp_checksum(iph, tcph);
    } else if (protocol == IPPROTO_UDP) {
        udph = (udphdr_t*)(skb->data + (iph->ihl * 4));
        udph->source = htons(entry->nat_src_port);
        /* Recalculate UDP checksum */
        udph->check = 0;
        udph->check = udp_checksum(iph, udph);
    }
    
    /* Recalculate IP checksum */
    iph->check = 0;
    iph->check = ip_checksum(iph, iph->ihl * 4);
    
    nat_stats.snat_packets++;
    nat_stats.snat_bytes += skb->len;
    
    return 1;  /* Packet translated */
}

/*
 * Translate inbound packet (DNAT)
 * 
 * @skb: Socket buffer
 * @return: 0 on success, negative on error
 */
int nat_translate_inbound(struct sk_buff* skb) {
    iphdr_t* iph;
    tcphdr_t* tcph;
    udphdr_t* udph;
    nat_entry_t* entry;
    uint32_t src_ip, dst_ip;
    uint16_t src_port = 0, dst_port = 0;
    uint8_t protocol;
    int i;
    
    if (!skb || skb->len < sizeof(iphdr_t)) {
        return -1;
    }
    
    iph = (iphdr_t*)skb->data;
    protocol = iph->protocol;
    src_ip = ntohl(iph->saddr);
    dst_ip = ntohl(iph->daddr);
    
    /* Extract port numbers */
    if (protocol == IPPROTO_TCP && skb->len >= sizeof(iphdr_t) + sizeof(tcphdr_t)) {
        tcph = (tcphdr_t*)(skb->data + (iph->ihl * 4));
        src_port = ntohs(tcph->source);
        dst_port = ntohs(tcph->dest);
    } else if (protocol == IPPROTO_UDP && skb->len >= sizeof(iphdr_t) + sizeof(udphdr_t)) {
        udph = (udphdr_t*)(skb->data + (iph->ihl * 4));
        src_port = ntohs(udph->source);
        dst_port = ntohs(udph->dest);
    }
    
    /* Find existing NAT entry (reverse lookup) */
    for (i = 0; i < NAT_TABLE_SIZE; i++) {
        for (entry = nat_table[i]; entry; entry = entry->next) {
            if (entry->nat_dst_ip == dst_ip &&
                entry->nat_dst_port == dst_port &&
                entry->orig_src_ip == src_ip &&
                entry->orig_src_port == src_port &&
                entry->protocol == protocol &&
                entry->nat_type == NAT_TYPE_SNAT) {
                
                /* Reverse translate */
                iph->daddr = htonl(entry->orig_src_ip);
                
                if (protocol == IPPROTO_TCP) {
                    tcph = (tcphdr_t*)(skb->data + (iph->ihl * 4));
                    tcph->dest = htons(entry->orig_src_port);
                    tcph->check = 0;
                    tcph->check = tcp_checksum(iph, tcph);
                } else if (protocol == IPPROTO_UDP) {
                    udph = (udphdr_t*)(skb->data + (iph->ihl * 4));
                    udph->dest = htons(entry->orig_src_port);
                    udph->check = 0;
                    udph->check = udp_checksum(iph, udph);
                }
                
                iph->check = 0;
                iph->check = ip_checksum(iph, iph->ihl * 4);
                
                entry->packets++;
                entry->bytes += skb->len;
                
                nat_stats.dnat_packets++;
                nat_stats.dnat_bytes += skb->len;
                
                return 1;
            }
        }
    }
    
    return 0;  /* No NAT entry found */
}

/*
 * Age NAT connections (remove timed-out entries)
 */
void nat_age_connections(void) {
    int i;
    nat_entry_t** prev;
    nat_entry_t* entry;
    nat_entry_t* next;
    uint32_t now = 0;  /* TODO: get current time */
    
    for (i = 0; i < NAT_TABLE_SIZE; i++) {
        prev = &nat_table[i];
        entry = nat_table[i];
        
        while (entry) {
            next = entry->next;
            
            /* Check timeout */
            if (now - entry->last_seen > entry->timeout) {
                *prev = next;
                free(entry);
                nat_stats.connections--;
                nat_stats.timeouts++;
            } else {
                prev = &entry->next;
            }
            
            entry = next;
        }
    }
}

/*
 * Get NAT Statistics
 * 
 * @stats: Output statistics structure
 */
void nat_get_stats(nat_stats_t* stats) {
    if (!stats) {
        return;
    }
    memcpy(stats, &nat_stats, sizeof(nat_stats_t));
}

/*
 * Dump NAT Statistics
 */
void nat_dump_stats(void) {
    printk(KERN_INFO "=== NAT Statistics ===\n");
    printk(KERN_INFO "SNAT: packets=%llu bytes=%llu\n",
           nat_stats.snat_packets, nat_stats.snat_bytes);
    printk(KERN_INFO "DNAT: packets=%llu bytes=%llu\n",
           nat_stats.dnat_packets, nat_stats.dnat_bytes);
    printk(KERN_INFO "Connections: active=%llu total=%llu timeouts=%llu errors=%llu\n",
           nat_stats.connections, nat_stats.connections_total,
           nat_stats.timeouts, nat_stats.errors);
}

/*
 * Dump NAT Connection Table
 */
void nat_dump_table(void) {
    int i;
    nat_entry_t* entry;
    int count = 0;
    
    printk(KERN_INFO "=== NAT Connection Table ===\n");
    
    for (i = 0; i < NAT_TABLE_SIZE; i++) {
        for (entry = nat_table[i]; entry; entry = entry->next) {
            printk(KERN_INFO "[%d] %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u => "
                   "%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u (%s)\n",
                   count++,
                   (entry->orig_src_ip >> 24) & 0xFF,
                   (entry->orig_src_ip >> 16) & 0xFF,
                   (entry->orig_src_ip >> 8) & 0xFF,
                   entry->orig_src_ip & 0xFF,
                   entry->orig_src_port,
                   (entry->orig_dst_ip >> 24) & 0xFF,
                   (entry->orig_dst_ip >> 16) & 0xFF,
                   (entry->orig_dst_ip >> 8) & 0xFF,
                   entry->orig_dst_ip & 0xFF,
                   entry->orig_dst_port,
                   (entry->nat_src_ip >> 24) & 0xFF,
                   (entry->nat_src_ip >> 16) & 0xFF,
                   (entry->nat_src_ip >> 8) & 0xFF,
                   entry->nat_src_ip & 0xFF,
                   entry->nat_src_port,
                   (entry->nat_dst_ip >> 24) & 0xFF,
                   (entry->nat_dst_ip >> 16) & 0xFF,
                   (entry->nat_dst_ip >> 8) & 0xFF,
                   entry->nat_dst_ip & 0xFF,
                   entry->nat_dst_port,
                   entry->nat_type == NAT_TYPE_SNAT ? "SNAT" : "DNAT");
        }
    }
    
    printk(KERN_INFO "Total entries: %d\n", count);
}
