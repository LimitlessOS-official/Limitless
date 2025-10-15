#pragma once
#include "kernel.h"
#include "net/net.h"

#pragma pack(push,1)
typedef struct {
    u8  ver_ihl;
    u8  tos;
    u16 total_len;
    u16 id;
    u16 frag_off;
    u8  ttl;
    u8  proto;
    u16 hdr_checksum;
    u32 src;
    u32 dst;
} ipv4_hdr_t;
#pragma pack(pop)

/* IPv4 core */
void ipv4_init(void);
int  ipv4_output(net_if_t* nif, in_addr_t src, in_addr_t dst, u8 proto, net_buf_t* payload);
void ipv4_input(net_if_t* nif, net_buf_t* buf);