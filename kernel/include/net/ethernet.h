/*
 * Ethernet Protocol Header
 * IEEE 802.3 Ethernet
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#pragma once
#include "types.h"

/* Ethernet Protocol Types (EtherType) */
#define ETH_P_LOOP      0x0060   /* Ethernet Loopback packet */
#define ETH_P_PUP       0x0200   /* Xerox PUP packet */
#define ETH_P_PUPAT     0x0201   /* Xerox PUP Addr Trans packet */
#define ETH_P_IP        0x0800   /* Internet Protocol packet */
#define ETH_P_X25       0x0805   /* CCITT X.25 */
#define ETH_P_ARP       0x0806   /* Address Resolution packet */
#define ETH_P_BPQ       0x08FF   /* G8BPQ AX.25 Ethernet Packet */
#define ETH_P_IEEEPUP   0x0a00   /* Xerox IEEE802.3 PUP packet */
#define ETH_P_IEEEPUPAT 0x0a01   /* Xerox IEEE802.3 PUP Addr Trans packet */
#define ETH_P_DEC       0x6000   /* DEC Assigned proto */
#define ETH_P_DNA_DL    0x6001   /* DEC DNA Dump/Load */
#define ETH_P_DNA_RC    0x6002   /* DEC DNA Remote Console */
#define ETH_P_DNA_RT    0x6003   /* DEC DNA Routing */
#define ETH_P_LAT       0x6004   /* DEC LAT */
#define ETH_P_DIAG      0x6005   /* DEC Diagnostics */
#define ETH_P_CUST      0x6006   /* DEC Customer use */
#define ETH_P_SCA       0x6007   /* DEC Systems Comms Arch */
#define ETH_P_RARP      0x8035   /* Reverse Addr Res packet */
#define ETH_P_ATALK     0x809B   /* Appletalk DDP */
#define ETH_P_AARP      0x80F3   /* Appletalk AARP */
#define ETH_P_8021Q     0x8100   /* 802.1Q VLAN Extended Header */
#define ETH_P_IPX       0x8137   /* IPX over DIX */
#define ETH_P_IPV6      0x86DD   /* IPv6 over bluebook */
#define ETH_P_PPP_DISC  0x8863   /* PPPoE discovery messages */
#define ETH_P_PPP_SES   0x8864   /* PPPoE session messages */
#define ETH_P_MPLS_UC   0x8847   /* MPLS Unicast traffic */
#define ETH_P_MPLS_MC   0x8848   /* MPLS Multicast traffic */
#define ETH_P_ATMMPOA   0x884c   /* MultiProtocol Over ATM */
#define ETH_P_ATMFATE   0x8884   /* Frame-based ATM Transport over Ethernet */
#define ETH_P_AOE       0x88A2   /* ATA over Ethernet */
#define ETH_P_TIPC      0x88CA   /* TIPC */
#define ETH_P_802_3     0x0001   /* Dummy type for 802.3 frames */
#define ETH_P_AX25      0x0002   /* Dummy protocol id for AX.25 */
#define ETH_P_ALL       0x0003   /* Every packet (be careful!) */

/* Ethernet Header Length */
#define ETH_HLEN        14       /* Total octets in header */
#define ETH_ZLEN        60       /* Min. octets in frame sans FCS */
#define ETH_DATA_LEN    1500     /* Max. octets in payload */
#define ETH_FRAME_LEN   1514     /* Max. octets in frame sans FCS */
#define ETH_FCS_LEN     4        /* Octets in the FCS */

/* Ethernet Address Length */
#define ETH_ALEN        6        /* Octets in one ethernet addr */

#pragma pack(push, 1)

/* Ethernet Frame Header */
typedef struct ethhdr {
    uint8_t  h_dest[ETH_ALEN];   /* Destination MAC address */
    uint8_t  h_source[ETH_ALEN]; /* Source MAC address */
    uint16_t h_proto;            /* EtherType / Length */
} ethhdr_t;

/* 802.1Q VLAN Header */
typedef struct vlanhdr {
    uint16_t h_vlan_TCI;         /* Priority and VLAN ID */
    uint16_t h_vlan_encap_proto; /* Encapsulated protocol */
} vlanhdr_t;

#pragma pack(pop)

/* Forward declarations */
struct sk_buff;
struct net_device;

/* Ethernet Statistics */
typedef struct ethernet_stats {
    uint64_t rx_packets;
    uint64_t rx_bytes;
    uint64_t rx_errors;
    uint64_t rx_dropped;
    uint64_t rx_multicast;
    uint64_t rx_broadcast;
    
    uint64_t tx_packets;
    uint64_t tx_bytes;
    uint64_t tx_errors;
    uint64_t tx_dropped;
} ethernet_stats_t;

/* Ethernet Functions */

/* Initialization */
int ethernet_init(void);

/* Transmission */
int ethernet_send(struct net_device* dev, const uint8_t* dest_mac,
                  uint16_t proto, struct sk_buff* skb);

/* Reception */
void ethernet_rcv(struct sk_buff* skb);

/* MAC Address Utilities */
int ethernet_mac_equal(const uint8_t* mac1, const uint8_t* mac2);
int ethernet_mac_is_broadcast(const uint8_t* mac);
int ethernet_mac_is_multicast(const uint8_t* mac);
int ethernet_mac_is_zero(const uint8_t* mac);
void ethernet_mac_to_str(const uint8_t* mac, char* str, uint32_t len);
int ethernet_str_to_mac(const char* str, uint8_t* mac);

/* Statistics */
void ethernet_get_stats(ethernet_stats_t* stats);
void ethernet_dump_stats(void);
