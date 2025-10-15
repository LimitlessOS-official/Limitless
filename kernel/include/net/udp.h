#pragma once
#include "kernel.h"
#include "net/net.h"

#pragma pack(push,1)
typedef struct {
    u16 src_port;
    u16 dst_port;
    u16 len;
    u16 csum;
} udp_hdr_t;
#pragma pack(pop)

void udp_init(void);
void udp_input(net_if_t* nif, in_addr_t src, in_addr_t dst, net_buf_t* buf);
int  udp_send(sock_t s, const void* data, u32 len, const sockaddr_in_t* dst);
int  udp_bind(sock_t s, const sockaddr_in_t* addr);