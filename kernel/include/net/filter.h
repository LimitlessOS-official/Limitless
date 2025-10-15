#pragma once
#include <stdint.h>
#include "kernel.h"

typedef enum { NF_DROP=0, NF_ACCEPT=1 } nf_verdict_t;

typedef struct {
    uint32_t src_ip_be;
    uint32_t dst_ip_be;
    uint8_t  proto;  /* IPPROTO_* */
    uint16_t src_port_be;
    uint16_t dst_port_be;
    uint8_t  direction; /* 0=input,1=output */
} nf_packet_t;

/* Initialize firewall with default policy (DROP by default for Phase 3) */
void nf_init(void);
void nf_set_default_policy(uint8_t direction, nf_verdict_t v);

/* Evaluate a packet and return verdict */
nf_verdict_t nf_check(const nf_packet_t* p);
