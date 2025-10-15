/*
 * ARP (Address Resolution Protocol) Header
 * RFC 826 - Address Resolution Protocol
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#pragma once
#include "types.h"
#include "net/ip.h"

/* ARP Hardware Types */
#define ARPHRD_NETROM     0      /* From KA9Q: NET/ROM pseudo */
#define ARPHRD_ETHER      1      /* Ethernet 10/100Mbps */
#define ARPHRD_EETHER     2      /* Experimental Ethernet */
#define ARPHRD_AX25       3      /* AX.25 Level 2 */
#define ARPHRD_PRONET     4      /* PROnet token ring */
#define ARPHRD_CHAOS      5      /* Chaosnet */
#define ARPHRD_IEEE802    6      /* IEEE 802.2 Ethernet/TR/TB */
#define ARPHRD_ARCNET     7      /* ARCnet */
#define ARPHRD_APPLETLK   8      /* APPLEtalk */
#define ARPHRD_LOOPBACK   772    /* Loopback */

/* ARP Operations */
#define ARPOP_REQUEST     1      /* ARP request */
#define ARPOP_REPLY       2      /* ARP reply */
#define ARPOP_RREQUEST    3      /* RARP request */
#define ARPOP_RREPLY      4      /* RARP reply */
#define ARPOP_InREQUEST   8      /* InARP request */
#define ARPOP_InREPLY     9      /* InARP reply */
#define ARPOP_NAK         10     /* (ATM)ARP NAK */

#pragma pack(push, 1)

/* ARP Packet Header */
typedef struct arphdr {
    uint16_t ar_hrd;             /* Hardware type */
    uint16_t ar_pro;             /* Protocol type */
    uint8_t  ar_hln;             /* Hardware address length */
    uint8_t  ar_pln;             /* Protocol address length */
    uint16_t ar_op;              /* ARP opcode */
    
    /* Ethernet + IPv4 specific fields */
    uint8_t  ar_sha[6];          /* Sender hardware address */
    uint8_t  ar_sip[4];          /* Sender IP address */
    uint8_t  ar_tha[6];          /* Target hardware address */
    uint8_t  ar_tip[4];          /* Target IP address */
} arphdr_t;

/* Legacy ARP packet structure (keep for compatibility) */
typedef struct {
    u16 htype;
    u16 ptype;
    u8  hlen;
    u8  plen;
    u16 oper;
    u8  sha[6];
    u32 spa;
    u8  tha[6];
    u32 tpa;
} arp_pkt_t;

#pragma pack(pop)

/* Forward declarations */
struct sk_buff;
struct net_device;

/* ARP Statistics */
typedef struct arp_stats {
    uint64_t requests_sent;
    uint64_t requests_rcvd;
    uint64_t replies_sent;
    uint64_t replies_rcvd;
    uint64_t cache_hits;
    uint64_t cache_misses;
    uint32_t cache_entries;
} arp_stats_t;

/* ARP Functions */

/* Initialization */
int arp_init(void);
void arp_cleanup(void);

/* Cache Management */
int arp_add_entry(ipv4_addr_t ip_addr, const uint8_t* mac_addr,
                  struct net_device* dev, int permanent);
int arp_delete_entry(ipv4_addr_t ip_addr);
void arp_flush_cache(void);
void arp_age_cache(void);

/* ARP Protocol */
int arp_request(ipv4_addr_t target_ip, struct net_device* dev);
int arp_reply(ipv4_addr_t target_ip, const uint8_t* target_mac,
              struct net_device* dev);
int arp_gratuitous(struct net_device* dev);

/* Address Resolution */
int arp_resolve(ipv4_addr_t ip_addr, uint8_t* mac_addr,
                struct net_device* dev, struct sk_buff* skb);

/* Reception */
void arp_rcv(struct sk_buff* skb);

/* Display */
void arp_dump_cache(void);
void arp_get_stats(arp_stats_t* stats);
void arp_dump_stats(void);