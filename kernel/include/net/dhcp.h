#pragma once
#include "kernel.h"
#include "net/net.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal DHCPv4 client (DORA) */
int dhcpv4_acquire(net_if_t* nif, u32* out_ip_be, u32* out_netmask_be, u32* out_gw_be, u32 timeout_ms);

#ifdef __cplusplus
}
#endif
