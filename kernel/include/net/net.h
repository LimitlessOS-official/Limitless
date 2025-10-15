#ifndef NETIF_UP
#define NETIF_UP        0x1
#endif
#ifndef NETIF_LOOPBACK
#define NETIF_LOOPBACK  0x8
#endif
#ifndef K_EHOSTUNREACH
#define K_EHOSTUNREACH  -113
#endif

#pragma once
#include "kernel.h"

#define AF_UNSPEC 0
#define AF_INET   2
#define SOCK_DGRAM 1
#define SOCK_STREAM 2

typedef struct { u32 s_addr; } in_addr_t; /* stored in network byte order */
typedef struct {
    u16 sin_family;
    u16 sin_port;
    in_addr_t sin_addr;
    u8  sin_zero[8];
} sockaddr_in_t;
typedef u16 in_port_t;
#ifndef NET_SOCK_T_DEFINED
#define NET_SOCK_T_DEFINED
typedef u32 sock_t; /* socket handle (matches net/socket.h) */
#endif

/* Network buffer (lightweight MBuf-like) */
typedef struct net_buf {
    u8* data;
    u32 len;
    u32 cap;
    u32 headroom;
    u32 tailroom;
} net_buf_t;

/* Net buffer helpers (implemented in net/core.c) */
net_buf_t* net_buf_alloc(u32 size, u32 headroom);
void       net_buf_free(net_buf_t* b);
int        net_buf_reserve_head(net_buf_t* b, u32 bytes);
int        net_buf_push(net_buf_t* b, const void* data, u32 len);
u8*        net_buf_push_uninit(net_buf_t* b, u32 len);
int        net_buf_trim_head(net_buf_t* b, u32 len);

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal NIC interface */
typedef struct net_if {
    char name[16];
    u8   mac[6];
    u32  ifindex;
    u32  mtu;
    u32  flags;
    // Direct IPv4 fields for legacy code compatibility
    u32  ip_be;
    u32  netmask_be;
    u32  gateway_be;
    int (*send)(struct net_if* nif, struct net_buf* buf, u32 plen);
    // Modern ops struct
    struct {
        int (*xmit)(struct net_if* nif, struct net_buf* buf, u16 ethertype, const u8 dst_mac[6]);
        void (*poll)(struct net_if* nif);
    } ops;
    struct {
        in_addr_t s_addr;
    } ipv4_addr, ipv4_netmask, ipv4_gateway;
    void* drv;         /* driver private */
} net_if_t;

/* Registration and configuration */
void net_init(void);
int  net_if_register(net_if_t* nif);
void net_if_config_static(net_if_t* nif, u32 ip_be, u32 netmask_be, u32 gateway_be);

/* Utility to fetch first non-loopback interface (for DHCP bootstrapping) */
net_if_t* net_if_first(void);

/* RX ingress from drivers */
void net_ingress(net_if_t* nif, struct net_buf* buf);

/* Poll callback registration (for drivers providing RX polling) */
void net_register_poll_cb(void (*cb)(void*), void* arg);

/* Manual pump to process RX while blocking in syscalls (busy-wait systems) */
void net_pump(void);

/* ARP helpers */
int  arp_resolve(net_if_t* nif, u32 ip_be, u8 mac_out[6]); /* returns 0 on success */

/* IPv4 TX helpers */
int  ipv4_send_packet(net_if_t* nif, u8 proto, u32 dst_ip_be, const void* payload, u16 plen);

/* UDP sockets API (kernel-internal) */
int  udp_socket_open(void);
int  udp_sendto(int sid, const void* data, u32 len, u32 dst_ip_be, u16 dst_port_be);
int  udp_recvfrom(sock_t s, void* buf, u32 len, u32* src_ip_be, u16* src_port_be);

/* ICMP RAW sockets API (echo) */
int  icmp_socket_open(void);
int  icmp_sendto(int sid, const void* data, u32 len, u32 dst_ip_be);
int  icmp_recvfrom(int sid, void* buf, u32 len, u32* src_ip_be);

/* Utilities */
static inline u16 net_htons(u16 x){ return (u16)((x<<8)|(x>>8)); }
static inline u32 net_htonl(u32 x){ return (x<<24)|((x&0xFF00)<<8)|((x&0xFF0000)>>8)|((x>>24)&0xFF); }
static inline u16 net_ntohs(u16 x){ return net_htons(x); }
static inline u32 net_ntohl(u32 x){ return net_htonl(x); }

#ifdef __cplusplus
}
#endif