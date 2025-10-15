#pragma once
#include "kernel.h"
#include "net/net.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IPv4/IPv6 common wrappers */
void ip_stack_init(void);

/* IPv4 */
int  ip4_output(net_if_t* nif, u8 proto, u32 dst_ip_be, const void* payload, u16 plen);
void ip4_input(net_if_t* nif, const u8* pkt, u32 len);

/* IPv6 minimal scaffolding */
typedef struct {
    u32 v_tc_fl;   /* version(4)/traffic class(8)/flow label(20) */
    u16 payload_len;
    u8  next_hdr;
    u8  hop_limit;
    u8  src[16];
    u8  dst[16];
} ipv6_hdr_t;

int  ip6_output(net_if_t* nif, u8 next_hdr, const u8 dst[16], const void* payload, u16 plen);
void ip6_input(net_if_t* nif, const u8* pkt, u32 len);

#ifdef __cplusplus
}
#endif
