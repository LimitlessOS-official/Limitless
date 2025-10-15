/*
 * Netfilter - Packet Filtering Framework
 * Linux-compatible netfilter hooks
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#pragma once
#include "types.h"
#include "net/sk_buff.h"

/* Netfilter Hooks */
#define NF_IP_PRE_ROUTING    0   /* After sanity checks, before routing decision */
#define NF_IP_LOCAL_IN       1   /* After routing decision if packet is for us */
#define NF_IP_FORWARD        2   /* If packet is to be forwarded */
#define NF_IP_LOCAL_OUT      3   /* Packets coming from local processes */
#define NF_IP_POST_ROUTING   4   /* After routing decision, before putting on wire */
#define NF_IP_NUMHOOKS       5

/* Netfilter Verdicts */
#define NF_DROP              0   /* Drop the packet */
#define NF_ACCEPT            1   /* Continue traversal as normal */
#define NF_STOLEN            2   /* Packet consumed by hook function */
#define NF_QUEUE             3   /* Queue packet to userspace */
#define NF_REPEAT            4   /* Call this hook function again */
#define NF_STOP              5   /* Deprecated, use NF_ACCEPT */

/* Hook priorities (lower = higher priority) */
#define NF_IP_PRI_FIRST           -2147483648
#define NF_IP_PRI_CONNTRACK_DEFRAG  -400
#define NF_IP_PRI_RAW              -300
#define NF_IP_PRI_SELINUX_FIRST    -225
#define NF_IP_PRI_CONNTRACK        -200
#define NF_IP_PRI_MANGLE           -150
#define NF_IP_PRI_NAT_DST          -100
#define NF_IP_PRI_FILTER             0
#define NF_IP_PRI_SECURITY          50
#define NF_IP_PRI_NAT_SRC           100
#define NF_IP_PRI_SELINUX_LAST      225
#define NF_IP_PRI_CONNTRACK_HELPER  300
#define NF_IP_PRI_LAST            2147483647

/* Forward declarations */
struct sk_buff;
struct net_device;

/* Netfilter hook function */
typedef unsigned int (*nf_hookfn)(unsigned int hooknum,
                                  struct sk_buff* skb,
                                  const struct net_device* in,
                                  const struct net_device* out,
                                  void* priv);

/* Netfilter hook entry */
typedef struct nf_hook_ops {
    struct nf_hook_ops* next;    /* Next hook in list */
    nf_hookfn hook;              /* Hook function */
    void* priv;                  /* Private data */
    unsigned int hooknum;        /* Hook number (NF_IP_*) */
    int priority;                /* Hook priority */
} nf_hook_ops_t;

/* Netfilter Statistics */
typedef struct netfilter_stats {
    uint64_t hooks_called;       /* Total hooks called */
    uint64_t packets_dropped;    /* Packets dropped */
    uint64_t packets_accepted;   /* Packets accepted */
    uint64_t packets_stolen;     /* Packets stolen */
    uint64_t packets_queued;     /* Packets queued */
} netfilter_stats_t;

/* Netfilter Functions */

/* Initialization */
int netfilter_init(void);

/* Hook registration */
int nf_register_hook(nf_hook_ops_t* ops);
void nf_unregister_hook(nf_hook_ops_t* ops);

/* Hook invocation */
unsigned int nf_hook_slow(unsigned int hooknum, struct sk_buff* skb,
                          const struct net_device* in,
                          const struct net_device* out);

/* Inline hook invocation for performance */
static inline unsigned int nf_hook(unsigned int hooknum, struct sk_buff* skb,
                                   const struct net_device* in,
                                   const struct net_device* out) {
    return nf_hook_slow(hooknum, skb, in, out);
}

/* Statistics */
void netfilter_get_stats(netfilter_stats_t* stats);
void netfilter_dump_stats(void);
