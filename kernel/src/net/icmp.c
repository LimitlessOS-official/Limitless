/*
 * ICMP (Internet Control Message Protocol) Implementation
 * 
 * Production-grade ICMP implementation with ping, error messages,
 * and diagnostic capabilities.
 * 
 * RFC 792 - Internet Control Message Protocol
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/icmp.h"
#include "net/ip.h"
#include "net/skbuff.h"
#include "kernel.h"
#include <string.h>

/* ICMP statistics */
static struct {
    uint64_t in_msgs;
    uint64_t in_errors;
    uint64_t in_dest_unreachs;
    uint64_t in_time_excds;
    uint64_t in_parm_probs;
    uint64_t in_src_quenchs;
    uint64_t in_redirects;
    uint64_t in_echos;
    uint64_t in_echo_reps;
    uint64_t in_timestamps;
    uint64_t in_timestamp_reps;
    
    uint64_t out_msgs;
    uint64_t out_errors;
    uint64_t out_dest_unreachs;
    uint64_t out_time_excds;
    uint64_t out_parm_probs;
    uint64_t out_src_quenchs;
    uint64_t out_redirects;
    uint64_t out_echos;
    uint64_t out_echo_reps;
    uint64_t out_timestamps;
    uint64_t out_timestamp_reps;
} icmp_stats;

/* Ping request tracking */
#define MAX_PING_REQUESTS 64

typedef struct ping_request {
    uint16_t id;
    uint16_t seq;
    ipv4_addr_t dest;
    uint32_t timestamp;
    void (*callback)(uint16_t id, uint16_t seq, uint32_t rtt);
    int active;
} ping_request_t;

static struct {
    ping_request_t requests[MAX_PING_REQUESTS];
    uint16_t next_id;
} ping_state;

/* ==================== ICMP Header Functions ==================== */

uint16_t icmp_checksum(const void* data, uint32_t len) {
    const uint16_t* p = (const uint16_t*)data;
    uint32_t sum = 0;
    
    while (len > 1) {
        sum += *p++;
        len -= 2;
    }
    
    if (len == 1) {
        sum += *((const uint8_t*)p);
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

int icmp_verify_checksum(const icmphdr_t* icmph, uint32_t len) {
    uint16_t saved = icmph->checksum;
    ((icmphdr_t*)icmph)->checksum = 0;
    
    uint16_t computed = icmp_checksum(icmph, len);
    ((icmphdr_t*)icmph)->checksum = saved;
    
    return computed == saved;
}

/* ==================== ICMP Transmission ==================== */

int icmp_send(ipv4_addr_t dest, uint8_t type, uint8_t code, 
              const void* data, uint32_t data_len) {
    if (data_len > 512) {
        data_len = 512;  /* Limit ICMP data size */
    }
    
    /* Allocate skb */
    struct sk_buff* skb = alloc_skb(sizeof(icmphdr_t) + data_len, 0);
    if (!skb) return -1;
    
    /* Reserve space for headers */
    skb_reserve(skb, sizeof(icmphdr_t) + sizeof(iphdr_t) + 14);
    
    /* Copy data if provided */
    if (data && data_len > 0) {
        memcpy(skb_put(skb, data_len), data, data_len);
    }
    
    /* Build ICMP header */
    icmphdr_t* icmph = (icmphdr_t*)skb_push(skb, sizeof(icmphdr_t));
    memset(icmph, 0, sizeof(icmphdr_t));
    
    icmph->type = type;
    icmph->code = code;
    
    /* Calculate checksum */
    icmph->checksum = 0;
    icmph->checksum = icmp_checksum(icmph, skb->len);
    
    /* Set protocol */
    skb->protocol = IPPROTO_ICMP;
    skb->h.raw = (uint8_t*)icmph;
    
    /* Send via IP */
    int ret = ip_send(dest, skb);
    
    if (ret == 0) {
        icmp_stats.out_msgs++;
        
        switch (type) {
            case ICMP_DEST_UNREACH:
                icmp_stats.out_dest_unreachs++;
                break;
            case ICMP_TIME_EXCEEDED:
                icmp_stats.out_time_excds++;
                break;
            case ICMP_PARAMETERPROB:
                icmp_stats.out_parm_probs++;
                break;
            case ICMP_SOURCE_QUENCH:
                icmp_stats.out_src_quenchs++;
                break;
            case ICMP_REDIRECT:
                icmp_stats.out_redirects++;
                break;
            case ICMP_ECHO:
                icmp_stats.out_echos++;
                break;
            case ICMP_ECHOREPLY:
                icmp_stats.out_echo_reps++;
                break;
            case ICMP_TIMESTAMP:
                icmp_stats.out_timestamps++;
                break;
            case ICMP_TIMESTAMPREPLY:
                icmp_stats.out_timestamp_reps++;
                break;
        }
    } else {
        icmp_stats.out_errors++;
    }
    
    return ret;
}

/* ==================== ICMP Error Messages ==================== */

void icmp_send_dest_unreach(struct sk_buff* skb, uint8_t code) {
    if (!skb) return;
    
    iphdr_t* iph = (iphdr_t*)skb->nh.raw;
    ipv4_addr_t dest = ntohl(iph->saddr);
    
    /* Don't send ICMP errors about ICMP errors */
    if (iph->protocol == IPPROTO_ICMP) {
        return;
    }
    
    kprintf("[ICMP] Sending destination unreachable (code=%u) to %s\n",
            code, ip_addr_to_str(dest, NULL, 0));
    
    /* Include original IP header + 8 bytes of data */
    uint32_t data_len = (iph->ihl * 4) + 8;
    if (data_len > skb->len) {
        data_len = skb->len;
    }
    
    icmp_send(dest, ICMP_DEST_UNREACH, code, iph, data_len);
}

void icmp_send_time_exceeded(struct sk_buff* skb, uint8_t code) {
    if (!skb) return;
    
    iphdr_t* iph = (iphdr_t*)skb->nh.raw;
    ipv4_addr_t dest = ntohl(iph->saddr);
    
    if (iph->protocol == IPPROTO_ICMP) {
        return;
    }
    
    kprintf("[ICMP] Sending time exceeded (code=%u) to %s\n",
            code, ip_addr_to_str(dest, NULL, 0));
    
    uint32_t data_len = (iph->ihl * 4) + 8;
    if (data_len > skb->len) {
        data_len = skb->len;
    }
    
    icmp_send(dest, ICMP_TIME_EXCEEDED, code, iph, data_len);
}

void icmp_send_parameter_problem(struct sk_buff* skb, uint8_t code, uint8_t pointer) {
    if (!skb) return;
    
    iphdr_t* iph = (iphdr_t*)skb->nh.raw;
    ipv4_addr_t dest = ntohl(iph->saddr);
    
    if (iph->protocol == IPPROTO_ICMP) {
        return;
    }
    
    kprintf("[ICMP] Sending parameter problem (code=%u, pointer=%u) to %s\n",
            code, pointer, ip_addr_to_str(dest, NULL, 0));
    
    /* Build ICMP data with pointer */
    uint8_t data[512];
    data[0] = pointer;
    data[1] = data[2] = data[3] = 0;
    
    uint32_t hdr_len = (iph->ihl * 4) + 8;
    if (hdr_len > sizeof(data) - 4) {
        hdr_len = sizeof(data) - 4;
    }
    
    memcpy(data + 4, iph, hdr_len);
    
    icmp_send(dest, ICMP_PARAMETERPROB, code, data, hdr_len + 4);
}

void icmp_send_port_unreach(struct sk_buff* skb) {
    icmp_send_dest_unreach(skb, ICMP_PORT_UNREACH);
}

void icmp_send_net_unreach(ipv4_addr_t dest) {
    icmp_send(dest, ICMP_DEST_UNREACH, ICMP_NET_UNREACH, NULL, 0);
}

void icmp_send_host_unreach(ipv4_addr_t dest) {
    icmp_send(dest, ICMP_DEST_UNREACH, ICMP_HOST_UNREACH, NULL, 0);
}

void icmp_send_proto_unreach(struct sk_buff* skb) {
    icmp_send_dest_unreach(skb, ICMP_PROT_UNREACH);
}

/* ==================== Ping (Echo Request/Reply) ==================== */

int icmp_ping(ipv4_addr_t dest, uint16_t id, uint16_t seq, 
              const void* data, uint32_t data_len,
              void (*callback)(uint16_t, uint16_t, uint32_t)) {
    
    kprintf("[ICMP] Sending ping to %s (id=%u seq=%u len=%u)\n",
            ip_addr_to_str(dest, NULL, 0), id, seq, data_len);
    
    /* Allocate skb */
    struct sk_buff* skb = alloc_skb(sizeof(icmphdr_t) + data_len, 0);
    if (!skb) return -1;
    
    /* Reserve space */
    skb_reserve(skb, sizeof(icmphdr_t) + sizeof(iphdr_t) + 14);
    
    /* Copy data */
    if (data && data_len > 0) {
        memcpy(skb_put(skb, data_len), data, data_len);
    }
    
    /* Build ICMP echo request */
    icmphdr_t* icmph = (icmphdr_t*)skb_push(skb, sizeof(icmphdr_t));
    memset(icmph, 0, sizeof(icmphdr_t));
    
    icmph->type = ICMP_ECHO;
    icmph->code = 0;
    icmph->un.echo.id = htons(id);
    icmph->un.echo.sequence = htons(seq);
    
    /* Calculate checksum */
    icmph->checksum = 0;
    icmph->checksum = icmp_checksum(icmph, skb->len);
    
    skb->protocol = IPPROTO_ICMP;
    skb->h.raw = (uint8_t*)icmph;
    
    /* Register ping request for tracking */
    for (int i = 0; i < MAX_PING_REQUESTS; i++) {
        if (!ping_state.requests[i].active) {
            ping_state.requests[i].id = id;
            ping_state.requests[i].seq = seq;
            ping_state.requests[i].dest = dest;
            ping_state.requests[i].timestamp = get_ticks();
            ping_state.requests[i].callback = callback;
            ping_state.requests[i].active = 1;
            break;
        }
    }
    
    /* Send */
    int ret = ip_send(dest, skb);
    
    if (ret == 0) {
        icmp_stats.out_echos++;
    }
    
    return ret;
}

int icmp_ping_simple(ipv4_addr_t dest, const void* data, uint32_t data_len) {
    uint16_t id = ping_state.next_id++;
    uint16_t seq = 1;
    
    return icmp_ping(dest, id, seq, data, data_len, NULL);
}

void icmp_send_echo_reply(ipv4_addr_t dest, uint16_t id, uint16_t seq,
                          const void* data, uint32_t data_len) {
    
    kprintf("[ICMP] Sending echo reply to %s (id=%u seq=%u)\n",
            ip_addr_to_str(dest, NULL, 0), id, seq);
    
    /* Allocate skb */
    struct sk_buff* skb = alloc_skb(sizeof(icmphdr_t) + data_len, 0);
    if (!skb) return;
    
    skb_reserve(skb, sizeof(icmphdr_t) + sizeof(iphdr_t) + 14);
    
    /* Copy data */
    if (data && data_len > 0) {
        memcpy(skb_put(skb, data_len), data, data_len);
    }
    
    /* Build echo reply */
    icmphdr_t* icmph = (icmphdr_t*)skb_push(skb, sizeof(icmphdr_t));
    memset(icmph, 0, sizeof(icmphdr_t));
    
    icmph->type = ICMP_ECHOREPLY;
    icmph->code = 0;
    icmph->un.echo.id = htons(id);
    icmph->un.echo.sequence = htons(seq);
    
    icmph->checksum = 0;
    icmph->checksum = icmp_checksum(icmph, skb->len);
    
    skb->protocol = IPPROTO_ICMP;
    skb->h.raw = (uint8_t*)icmph;
    
    int ret = ip_send(dest, skb);
    
    if (ret == 0) {
        icmp_stats.out_echo_reps++;
    }
}

/* ==================== ICMP Reception ==================== */

void icmp_rcv(struct sk_buff* skb) {
    if (!skb || skb->len < sizeof(icmphdr_t)) {
        kprintf("[ICMP] Packet too small\n");
        icmp_stats.in_errors++;
        free_skb(skb);
        return;
    }
    
    icmp_stats.in_msgs++;
    
    /* Get IP header */
    iphdr_t* iph = (iphdr_t*)skb->nh.raw;
    ipv4_addr_t saddr = ntohl(iph->saddr);
    
    /* Get ICMP header */
    icmphdr_t* icmph = (icmphdr_t*)skb->data;
    
    /* Verify checksum */
    if (!icmp_verify_checksum(icmph, skb->len)) {
        kprintf("[ICMP] Checksum failed\n");
        icmp_stats.in_errors++;
        free_skb(skb);
        return;
    }
    
    kprintf("[ICMP] Received type=%u code=%u from %s\n",
            icmph->type, icmph->code,
            ip_addr_to_str(saddr, NULL, 0));
    
    /* Update statistics */
    switch (icmph->type) {
        case ICMP_DEST_UNREACH:
            icmp_stats.in_dest_unreachs++;
            break;
        case ICMP_TIME_EXCEEDED:
            icmp_stats.in_time_excds++;
            break;
        case ICMP_PARAMETERPROB:
            icmp_stats.in_parm_probs++;
            break;
        case ICMP_SOURCE_QUENCH:
            icmp_stats.in_src_quenchs++;
            break;
        case ICMP_REDIRECT:
            icmp_stats.in_redirects++;
            break;
        case ICMP_ECHO:
            icmp_stats.in_echos++;
            break;
        case ICMP_ECHOREPLY:
            icmp_stats.in_echo_reps++;
            break;
        case ICMP_TIMESTAMP:
            icmp_stats.in_timestamps++;
            break;
        case ICMP_TIMESTAMPREPLY:
            icmp_stats.in_timestamp_reps++;
            break;
    }
    
    /* Process by type */
    switch (icmph->type) {
        case ICMP_ECHOREPLY:
            icmp_process_echo_reply(skb, icmph, saddr);
            break;
            
        case ICMP_DEST_UNREACH:
            icmp_process_dest_unreach(skb, icmph, saddr);
            break;
            
        case ICMP_SOURCE_QUENCH:
            icmp_process_source_quench(skb, icmph, saddr);
            break;
            
        case ICMP_REDIRECT:
            icmp_process_redirect(skb, icmph, saddr);
            break;
            
        case ICMP_ECHO:
            icmp_process_echo_request(skb, icmph, saddr);
            break;
            
        case ICMP_TIME_EXCEEDED:
            icmp_process_time_exceeded(skb, icmph, saddr);
            break;
            
        case ICMP_PARAMETERPROB:
            icmp_process_parameter_problem(skb, icmph, saddr);
            break;
            
        case ICMP_TIMESTAMP:
            icmp_process_timestamp(skb, icmph, saddr);
            break;
            
        case ICMP_TIMESTAMPREPLY:
            icmp_process_timestamp_reply(skb, icmph, saddr);
            break;
            
        default:
            kprintf("[ICMP] Unknown ICMP type: %u\n", icmph->type);
            free_skb(skb);
            break;
    }
}

/* ==================== ICMP Message Processing ==================== */

void icmp_process_echo_request(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr) {
    uint16_t id = ntohs(icmph->un.echo.id);
    uint16_t seq = ntohs(icmph->un.echo.sequence);
    
    kprintf("[ICMP] Echo request: id=%u seq=%u\n", id, seq);
    
    /* Get data (everything after ICMP header) */
    uint8_t* data = (uint8_t*)(icmph + 1);
    uint32_t data_len = skb->len - sizeof(icmphdr_t);
    
    /* Send echo reply */
    icmp_send_echo_reply(saddr, id, seq, data, data_len);
    
    free_skb(skb);
}

void icmp_process_echo_reply(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr) {
    uint16_t id = ntohs(icmph->un.echo.id);
    uint16_t seq = ntohs(icmph->un.echo.sequence);
    
    kprintf("[ICMP] Echo reply: id=%u seq=%u\n", id, seq);
    
    /* Find matching ping request */
    uint32_t now = get_ticks();
    
    for (int i = 0; i < MAX_PING_REQUESTS; i++) {
        if (ping_state.requests[i].active &&
            ping_state.requests[i].id == id &&
            ping_state.requests[i].seq == seq &&
            ping_state.requests[i].dest == saddr) {
            
            /* Calculate RTT */
            uint32_t rtt = (now - ping_state.requests[i].timestamp) * 10;  /* ticks to ms */
            
            kprintf("[ICMP] Ping reply from %s: seq=%u time=%u ms\n",
                    ip_addr_to_str(saddr, NULL, 0), seq, rtt);
            
            /* Call callback if registered */
            if (ping_state.requests[i].callback) {
                ping_state.requests[i].callback(id, seq, rtt);
            }
            
            /* Mark as inactive */
            ping_state.requests[i].active = 0;
            break;
        }
    }
    
    free_skb(skb);
}

void icmp_process_dest_unreach(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr) {
    const char* code_str = "Unknown";
    
    switch (icmph->code) {
        case ICMP_NET_UNREACH: code_str = "Network Unreachable"; break;
        case ICMP_HOST_UNREACH: code_str = "Host Unreachable"; break;
        case ICMP_PROT_UNREACH: code_str = "Protocol Unreachable"; break;
        case ICMP_PORT_UNREACH: code_str = "Port Unreachable"; break;
        case ICMP_FRAG_NEEDED: code_str = "Fragmentation Needed"; break;
        case ICMP_SR_FAILED: code_str = "Source Route Failed"; break;
        case ICMP_NET_UNKNOWN: code_str = "Network Unknown"; break;
        case ICMP_HOST_UNKNOWN: code_str = "Host Unknown"; break;
    }
    
    kprintf("[ICMP] Destination Unreachable from %s: %s\n",
            ip_addr_to_str(saddr, NULL, 0), code_str);
    
    /* TODO: Notify upper layer protocols (TCP, UDP) */
    
    free_skb(skb);
}

void icmp_process_time_exceeded(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr) {
    const char* code_str = (icmph->code == ICMP_EXC_TTL) ? 
                           "TTL Exceeded" : "Fragment Reassembly Time Exceeded";
    
    kprintf("[ICMP] Time Exceeded from %s: %s\n",
            ip_addr_to_str(saddr, NULL, 0), code_str);
    
    free_skb(skb);
}

void icmp_process_parameter_problem(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr) {
    kprintf("[ICMP] Parameter Problem from %s: pointer=%u\n",
            ip_addr_to_str(saddr, NULL, 0), icmph->un.gateway);
    
    free_skb(skb);
}

void icmp_process_source_quench(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr) {
    kprintf("[ICMP] Source Quench from %s (deprecated)\n",
            ip_addr_to_str(saddr, NULL, 0));
    
    /* Source Quench is deprecated (RFC 6633) */
    free_skb(skb);
}

void icmp_process_redirect(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr) {
    ipv4_addr_t gateway = ntohl(icmph->un.gateway);
    
    kprintf("[ICMP] Redirect from %s: use gateway %s\n",
            ip_addr_to_str(saddr, NULL, 0),
            ip_addr_to_str(gateway, NULL, 0));
    
    /* TODO: Update routing table */
    
    free_skb(skb);
}

void icmp_process_timestamp(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr) {
    kprintf("[ICMP] Timestamp request from %s\n",
            ip_addr_to_str(saddr, NULL, 0));
    
    /* TODO: Implement timestamp reply */
    
    free_skb(skb);
}

void icmp_process_timestamp_reply(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr) {
    kprintf("[ICMP] Timestamp reply from %s\n",
            ip_addr_to_str(saddr, NULL, 0));
    
    free_skb(skb);
}

/* ==================== Statistics ==================== */

void icmp_get_stats(icmp_stats_t* stats) {
    if (!stats) return;
    
    stats->in_msgs = icmp_stats.in_msgs;
    stats->in_errors = icmp_stats.in_errors;
    stats->in_dest_unreachs = icmp_stats.in_dest_unreachs;
    stats->in_echos = icmp_stats.in_echos;
    stats->in_echo_reps = icmp_stats.in_echo_reps;
    
    stats->out_msgs = icmp_stats.out_msgs;
    stats->out_errors = icmp_stats.out_errors;
    stats->out_dest_unreachs = icmp_stats.out_dest_unreachs;
    stats->out_echos = icmp_stats.out_echos;
    stats->out_echo_reps = icmp_stats.out_echo_reps;
}

void icmp_dump_stats(void) {
    kprintf("[ICMP] Statistics:\n");
    kprintf("  Messages In:  %llu (errors: %llu)\n", icmp_stats.in_msgs, icmp_stats.in_errors);
    kprintf("  Messages Out: %llu (errors: %llu)\n", icmp_stats.out_msgs, icmp_stats.out_errors);
    kprintf("  Echo Requests:  in=%llu out=%llu\n", icmp_stats.in_echos, icmp_stats.out_echos);
    kprintf("  Echo Replies:   in=%llu out=%llu\n", icmp_stats.in_echo_reps, icmp_stats.out_echo_reps);
    kprintf("  Dest Unreach:   in=%llu out=%llu\n", icmp_stats.in_dest_unreachs, icmp_stats.out_dest_unreachs);
    kprintf("  Time Exceeded:  in=%llu out=%llu\n", icmp_stats.in_time_excds, icmp_stats.out_time_excds);
}

/* ==================== Initialization ==================== */

int icmp_init(void) {
    kprintf("[ICMP] Initializing ICMP protocol...\n");
    
    /* Reset statistics */
    memset(&icmp_stats, 0, sizeof(icmp_stats));
    
    /* Initialize ping tracking */
    memset(&ping_state, 0, sizeof(ping_state));
    ping_state.next_id = 1;
    
    /* Register with IP layer */
    ip_register_protocol(IPPROTO_ICMP, icmp_rcv);
    
    kprintf("[ICMP] ICMP protocol initialized\n");
    
    return 0;
}
