/*
 * NAT - Network Address Translation
 * Source/Destination NAT with Port Translation
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#pragma once
#include "types.h"
#include "net/sk_buff.h"
#include "net/netdevice.h"

/* NAT Types */
#define NAT_TYPE_SNAT        1   /* Source NAT (masquerading) */
#define NAT_TYPE_DNAT        2   /* Destination NAT (port forwarding) */

/* NAT Flags */
#define NAT_FLAG_PERSISTENT  0x01  /* Persistent mapping */
#define NAT_FLAG_RANDOM_PORT 0x02  /* Random port allocation */

/* NAT Connection States */
#define NAT_STATE_NEW        0   /* New connection */
#define NAT_STATE_ESTABLISHED 1  /* Established connection */
#define NAT_STATE_CLOSING    2   /* Connection closing */

/* Maximum NAT table size */
#define NAT_TABLE_SIZE       1024

/* NAT connection tracking entry */
typedef struct nat_entry {
    struct nat_entry* next;      /* Hash chain */
    
    /* Original tuple */
    uint32_t orig_src_ip;
    uint32_t orig_dst_ip;
    uint16_t orig_src_port;
    uint16_t orig_dst_port;
    uint8_t  protocol;           /* TCP, UDP, etc. */
    
    /* Translated tuple */
    uint32_t nat_src_ip;
    uint32_t nat_dst_ip;
    uint16_t nat_src_port;
    uint16_t nat_dst_port;
    
    /* State */
    uint8_t  state;              /* NAT_STATE_* */
    uint8_t  nat_type;           /* NAT_TYPE_SNAT or NAT_TYPE_DNAT */
    uint8_t  flags;              /* NAT_FLAG_* */
    
    /* Timing */
    uint32_t last_seen;          /* Timestamp of last packet */
    uint32_t timeout;            /* Connection timeout */
    
    /* Statistics */
    uint64_t packets;
    uint64_t bytes;
} nat_entry_t;

/* NAT rule for configuration */
typedef struct nat_rule {
    struct nat_rule* next;
    
    /* Match criteria */
    uint32_t match_src_ip;
    uint32_t match_src_mask;
    uint32_t match_dst_ip;
    uint32_t match_dst_mask;
    uint16_t match_src_port_min;
    uint16_t match_src_port_max;
    uint16_t match_dst_port_min;
    uint16_t match_dst_port_max;
    uint8_t  match_protocol;     /* 0 = any */
    
    /* Translation */
    uint32_t nat_ip;             /* New source/dest IP */
    uint16_t nat_port_min;       /* Port range for translation */
    uint16_t nat_port_max;
    uint8_t  nat_type;           /* NAT_TYPE_SNAT or NAT_TYPE_DNAT */
    uint8_t  flags;              /* NAT_FLAG_* */
} nat_rule_t;

/* NAT Statistics */
typedef struct nat_stats {
    uint64_t snat_packets;       /* SNAT packets translated */
    uint64_t dnat_packets;       /* DNAT packets translated */
    uint64_t snat_bytes;
    uint64_t dnat_bytes;
    uint64_t connections;        /* Active connections */
    uint64_t connections_total;  /* Total connections created */
    uint64_t timeouts;           /* Connections timed out */
    uint64_t errors;             /* Translation errors */
} nat_stats_t;

/* NAT Functions */

/* Initialization */
int nat_init(void);

/* Rule management */
int nat_add_rule(nat_rule_t* rule);
int nat_delete_rule(nat_rule_t* rule);
void nat_flush_rules(void);

/* Packet translation */
int nat_translate_outbound(struct sk_buff* skb);  /* SNAT */
int nat_translate_inbound(struct sk_buff* skb);   /* DNAT */

/* Connection tracking */
nat_entry_t* nat_find_entry(uint32_t src_ip, uint32_t dst_ip,
                            uint16_t src_port, uint16_t dst_port,
                            uint8_t protocol);
nat_entry_t* nat_create_entry(uint32_t src_ip, uint32_t dst_ip,
                              uint16_t src_port, uint16_t dst_port,
                              uint8_t protocol, uint8_t nat_type);
void nat_delete_entry(nat_entry_t* entry);
void nat_age_connections(void);

/* Statistics */
void nat_get_stats(nat_stats_t* stats);
void nat_dump_stats(void);
void nat_dump_table(void);
