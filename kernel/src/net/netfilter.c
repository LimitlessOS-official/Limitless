/*
 * Netfilter Implementation
 * Packet Filtering Framework
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/netfilter.h"
#include "net/sk_buff.h"
#include "net/netdevice.h"
#include "kernel/printk.h"
#include "kernel/string.h"
#include "kernel/stdlib.h"

/* Hook lists for each hook point */
static nf_hook_ops_t* hook_lists[NF_IP_NUMHOOKS] = {NULL};

/* Global netfilter statistics */
static netfilter_stats_t nf_stats = {0};

/*
 * Initialize Netfilter
 */
int netfilter_init(void) {
    int i;
    for (i = 0; i < NF_IP_NUMHOOKS; i++) {
        hook_lists[i] = NULL;
    }
    memset(&nf_stats, 0, sizeof(nf_stats));
    printk(KERN_INFO "Netfilter initialized\n");
    return 0;
}

/*
 * Register a netfilter hook
 * 
 * @ops: Hook operations structure
 * @return: 0 on success, negative on error
 */
int nf_register_hook(nf_hook_ops_t* ops) {
    nf_hook_ops_t** list;
    nf_hook_ops_t* elem;
    
    if (!ops || ops->hooknum >= NF_IP_NUMHOOKS) {
        return -1;
    }
    
    list = &hook_lists[ops->hooknum];
    
    /* Insert hook in priority order (lower priority value = higher priority) */
    while (*list) {
        if (ops->priority < (*list)->priority) {
            break;
        }
        list = &(*list)->next;
    }
    
    /* Insert hook */
    ops->next = *list;
    *list = ops;
    
    printk(KERN_DEBUG "Registered netfilter hook at %u with priority %d\n",
           ops->hooknum, ops->priority);
    
    return 0;
}

/*
 * Unregister a netfilter hook
 * 
 * @ops: Hook operations structure
 */
void nf_unregister_hook(nf_hook_ops_t* ops) {
    nf_hook_ops_t** list;
    
    if (!ops || ops->hooknum >= NF_IP_NUMHOOKS) {
        return;
    }
    
    list = &hook_lists[ops->hooknum];
    
    /* Find and remove hook */
    while (*list) {
        if (*list == ops) {
            *list = ops->next;
            ops->next = NULL;
            printk(KERN_DEBUG "Unregistered netfilter hook at %u\n", ops->hooknum);
            return;
        }
        list = &(*list)->next;
    }
}

/*
 * Invoke netfilter hooks
 * 
 * @hooknum: Hook number (NF_IP_PRE_ROUTING, etc.)
 * @skb: Socket buffer
 * @in: Input network device (or NULL)
 * @out: Output network device (or NULL)
 * @return: Verdict (NF_ACCEPT, NF_DROP, etc.)
 */
unsigned int nf_hook_slow(unsigned int hooknum, struct sk_buff* skb,
                          const struct net_device* in,
                          const struct net_device* out) {
    nf_hook_ops_t* hook;
    unsigned int verdict;
    
    if (hooknum >= NF_IP_NUMHOOKS || !skb) {
        return NF_ACCEPT;
    }
    
    hook = hook_lists[hooknum];
    
    /* Call each hook in priority order */
    while (hook) {
        nf_stats.hooks_called++;
        
        verdict = hook->hook(hooknum, skb, in, out, hook->priv);
        
        switch (verdict) {
            case NF_ACCEPT:
                /* Continue to next hook */
                break;
                
            case NF_DROP:
                nf_stats.packets_dropped++;
                return NF_DROP;
                
            case NF_STOLEN:
                /* Hook consumed the packet */
                nf_stats.packets_stolen++;
                return NF_STOLEN;
                
            case NF_QUEUE:
                /* Queue to userspace */
                nf_stats.packets_queued++;
                return NF_QUEUE;
                
            case NF_REPEAT:
                /* Call this hook again */
                continue;
                
            default:
                printk(KERN_WARNING "Invalid netfilter verdict: %u\n", verdict);
                return NF_DROP;
        }
        
        hook = hook->next;
    }
    
    nf_stats.packets_accepted++;
    return NF_ACCEPT;
}

/*
 * Get Netfilter Statistics
 * 
 * @stats: Output statistics structure
 */
void netfilter_get_stats(netfilter_stats_t* stats) {
    if (!stats) {
        return;
    }
    memcpy(stats, &nf_stats, sizeof(netfilter_stats_t));
}

/*
 * Dump Netfilter Statistics
 */
void netfilter_dump_stats(void) {
    printk(KERN_INFO "=== Netfilter Statistics ===\n");
    printk(KERN_INFO "Hooks called: %llu\n", nf_stats.hooks_called);
    printk(KERN_INFO "Packets: accepted=%llu dropped=%llu stolen=%llu queued=%llu\n",
           nf_stats.packets_accepted, nf_stats.packets_dropped,
           nf_stats.packets_stolen, nf_stats.packets_queued);
}
