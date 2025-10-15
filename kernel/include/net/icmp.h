/*
 * ICMP (Internet Control Message Protocol) Header
 * RFC 792 - Internet Control Message Protocol
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#pragma once
#include "types.h"
#include "net/ip.h"

/* ICMP Message Types */
#define ICMP_ECHOREPLY          0    /* Echo Reply */
#define ICMP_DEST_UNREACH       3    /* Destination Unreachable */
#define ICMP_SOURCE_QUENCH      4    /* Source Quench (deprecated) */
#define ICMP_REDIRECT           5    /* Redirect */
#define ICMP_ECHO               8    /* Echo Request */
#define ICMP_TIME_EXCEEDED      11   /* Time Exceeded */
#define ICMP_PARAMETERPROB      12   /* Parameter Problem */
#define ICMP_TIMESTAMP          13   /* Timestamp Request */
#define ICMP_TIMESTAMPREPLY     14   /* Timestamp Reply */
#define ICMP_INFO_REQUEST       15   /* Information Request (obsolete) */
#define ICMP_INFO_REPLY         16   /* Information Reply (obsolete) */
#define ICMP_ADDRESS            17   /* Address Mask Request */
#define ICMP_ADDRESSREPLY       18   /* Address Mask Reply */

/* Destination Unreachable Codes */
#define ICMP_NET_UNREACH        0    /* Network Unreachable */
#define ICMP_HOST_UNREACH       1    /* Host Unreachable */
#define ICMP_PROT_UNREACH       2    /* Protocol Unreachable */
#define ICMP_PORT_UNREACH       3    /* Port Unreachable */
#define ICMP_FRAG_NEEDED        4    /* Fragmentation Needed and DF set */
#define ICMP_SR_FAILED          5    /* Source Route Failed */
#define ICMP_NET_UNKNOWN        6    /* Destination Network Unknown */
#define ICMP_HOST_UNKNOWN       7    /* Destination Host Unknown */
#define ICMP_HOST_ISOLATED      8    /* Source Host Isolated */
#define ICMP_NET_ANO            9    /* Network Administratively Prohibited */
#define ICMP_HOST_ANO           10   /* Host Administratively Prohibited */
#define ICMP_NET_UNR_TOS        11   /* Network Unreachable for TOS */
#define ICMP_HOST_UNR_TOS       12   /* Host Unreachable for TOS */
#define ICMP_PKT_FILTERED       13   /* Packet Filtered */
#define ICMP_PREC_VIOLATION     14   /* Precedence Violation */
#define ICMP_PREC_CUTOFF        15   /* Precedence Cutoff */

/* Time Exceeded Codes */
#define ICMP_EXC_TTL            0    /* TTL Exceeded in Transit */
#define ICMP_EXC_FRAGTIME       1    /* Fragment Reassembly Time Exceeded */

/* Redirect Codes */
#define ICMP_REDIR_NET          0    /* Redirect for Network */
#define ICMP_REDIR_HOST         1    /* Redirect for Host */
#define ICMP_REDIR_NETTOS       2    /* Redirect for TOS and Network */
#define ICMP_REDIR_HOSTTOS      3    /* Redirect for TOS and Host */

#pragma pack(push, 1)

/* ICMP Header */
typedef struct icmphdr {
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    
    union {
        struct {
            uint16_t id;
            uint16_t sequence;
        } echo;
        
        uint32_t gateway;
        
        struct {
            uint16_t unused;
            uint16_t mtu;
        } frag;
    } un;
} icmphdr_t;

#pragma pack(pop)

/* Forward declarations */
struct sk_buff;

/* ICMP Statistics */
typedef struct icmp_stats {
    uint64_t in_msgs;
    uint64_t in_errors;
    uint64_t in_dest_unreachs;
    uint64_t in_echos;
    uint64_t in_echo_reps;
    
    uint64_t out_msgs;
    uint64_t out_errors;
    uint64_t out_dest_unreachs;
    uint64_t out_echos;
    uint64_t out_echo_reps;
} icmp_stats_t;

/* ICMP Functions */

/* Initialization */
int icmp_init(void);

/* Reception */
void icmp_rcv(struct sk_buff* skb);

/* Transmission */
int icmp_send(ipv4_addr_t dest, uint8_t type, uint8_t code,
              const void* data, uint32_t data_len);

/* Ping (Echo Request/Reply) */
int icmp_ping(ipv4_addr_t dest, uint16_t id, uint16_t seq,
              const void* data, uint32_t data_len,
              void (*callback)(uint16_t id, uint16_t seq, uint32_t rtt));

int icmp_ping_simple(ipv4_addr_t dest, const void* data, uint32_t data_len);

void icmp_send_echo_reply(ipv4_addr_t dest, uint16_t id, uint16_t seq,
                          const void* data, uint32_t data_len);

/* Error Messages */
void icmp_send_dest_unreach(struct sk_buff* skb, uint8_t code);
void icmp_send_time_exceeded(struct sk_buff* skb, uint8_t code);
void icmp_send_parameter_problem(struct sk_buff* skb, uint8_t code, uint8_t pointer);

/* Convenience Functions */
void icmp_send_port_unreach(struct sk_buff* skb);
void icmp_send_net_unreach(ipv4_addr_t dest);
void icmp_send_host_unreach(ipv4_addr_t dest);
void icmp_send_proto_unreach(struct sk_buff* skb);

/* Message Processing */
void icmp_process_echo_request(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr);
void icmp_process_echo_reply(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr);
void icmp_process_dest_unreach(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr);
void icmp_process_time_exceeded(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr);
void icmp_process_parameter_problem(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr);
void icmp_process_source_quench(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr);
void icmp_process_redirect(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr);
void icmp_process_timestamp(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr);
void icmp_process_timestamp_reply(struct sk_buff* skb, icmphdr_t* icmph, ipv4_addr_t saddr);

/* Utilities */
uint16_t icmp_checksum(const void* data, uint32_t len);
int icmp_verify_checksum(const icmphdr_t* icmph, uint32_t len);

/* Statistics */
void icmp_get_stats(icmp_stats_t* stats);
void icmp_dump_stats(void);