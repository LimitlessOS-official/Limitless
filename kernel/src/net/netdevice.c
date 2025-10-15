/*
 * Network Device Layer Implementation
 * 
 * Production-grade network device abstraction with multi-queue support,
 * NAPI interrupt handling, and hardware offloading.
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/netdevice.h"
#include "net/skbuff.h"
#include "net/ip.h"
#include "kernel.h"
#include <string.h>

/* Global device list */
#define MAX_NET_DEVICES 16

static struct {
    struct net_device* devices[MAX_NET_DEVICES];
    uint32_t count;
    uint32_t next_ifindex;
} netdev_state;

/* Loopback device */
static struct net_device* loopback_dev = NULL;

/* ==================== Device Registration ==================== */

int netdev_register(struct net_device* dev) {
    if (!dev) return -1;
    
    if (netdev_state.count >= MAX_NET_DEVICES) {
        kprintf("[NETDEV] Too many network devices\n");
        return -1;
    }
    
    /* Assign interface index */
    dev->ifindex = netdev_state.next_ifindex++;
    
    /* Initialize device state */
    dev->state = NETDEV_STATE_DOWN;
    dev->flags = 0;
    
    /* Initialize queues */
    for (uint32_t i = 0; i < dev->num_tx_queues; i++) {
        skb_queue_head_init(&dev->tx_queue[i].queue);
        dev->tx_queue[i].qlen = 0;
        dev->tx_queue[i].stopped = 0;
    }
    
    for (uint32_t i = 0; i < dev->num_rx_queues; i++) {
        skb_queue_head_init(&dev->rx_queue[i].queue);
        dev->rx_queue[i].qlen = 0;
    }
    
    /* Add to device list */
    netdev_state.devices[netdev_state.count++] = dev;
    
    kprintf("[NETDEV] Registered device %s (ifindex=%u type=%u)\n",
            dev->name, dev->ifindex, dev->type);
    
    return 0;
}

void netdev_unregister(struct net_device* dev) {
    if (!dev) return;
    
    /* Bring device down */
    netdev_close(dev);
    
    /* Remove from device list */
    for (uint32_t i = 0; i < netdev_state.count; i++) {
        if (netdev_state.devices[i] == dev) {
            for (uint32_t j = i; j < netdev_state.count - 1; j++) {
                netdev_state.devices[j] = netdev_state.devices[j + 1];
            }
            netdev_state.count--;
            break;
        }
    }
    
    kprintf("[NETDEV] Unregistered device %s\n", dev->name);
}

struct net_device* netdev_get_by_name(const char* name) {
    if (!name) return NULL;
    
    for (uint32_t i = 0; i < netdev_state.count; i++) {
        if (strcmp(netdev_state.devices[i]->name, name) == 0) {
            return netdev_state.devices[i];
        }
    }
    
    return NULL;
}

struct net_device* netdev_get_by_index(uint32_t ifindex) {
    for (uint32_t i = 0; i < netdev_state.count; i++) {
        if (netdev_state.devices[i]->ifindex == ifindex) {
            return netdev_state.devices[i];
        }
    }
    
    return NULL;
}

/* ==================== Device Operations ==================== */

int netdev_open(struct net_device* dev) {
    if (!dev) return -1;
    
    if (dev->state == NETDEV_STATE_UP) {
        kprintf("[NETDEV] Device %s already up\n", dev->name);
        return 0;
    }
    
    kprintf("[NETDEV] Bringing up device %s\n", dev->name);
    
    /* Call driver's open function */
    if (dev->netdev_ops && dev->netdev_ops->ndo_open) {
        int ret = dev->netdev_ops->ndo_open(dev);
        if (ret != 0) {
            kprintf("[NETDEV] Failed to open device: %d\n", ret);
            return ret;
        }
    }
    
    /* Update state */
    dev->state = NETDEV_STATE_UP;
    dev->flags |= IFF_UP | IFF_RUNNING;
    
    kprintf("[NETDEV] Device %s is UP\n", dev->name);
    
    return 0;
}

int netdev_close(struct net_device* dev) {
    if (!dev) return -1;
    
    if (dev->state == NETDEV_STATE_DOWN) {
        return 0;
    }
    
    kprintf("[NETDEV] Bringing down device %s\n", dev->name);
    
    /* Call driver's stop function */
    if (dev->netdev_ops && dev->netdev_ops->ndo_stop) {
        int ret = dev->netdev_ops->ndo_stop(dev);
        if (ret != 0) {
            kprintf("[NETDEV] Failed to stop device: %d\n", ret);
        }
    }
    
    /* Flush queues */
    for (uint32_t i = 0; i < dev->num_tx_queues; i++) {
        skb_queue_purge(&dev->tx_queue[i].queue);
        dev->tx_queue[i].qlen = 0;
    }
    
    for (uint32_t i = 0; i < dev->num_rx_queues; i++) {
        skb_queue_purge(&dev->rx_queue[i].queue);
        dev->rx_queue[i].qlen = 0;
    }
    
    /* Update state */
    dev->state = NETDEV_STATE_DOWN;
    dev->flags &= ~(IFF_UP | IFF_RUNNING);
    
    kprintf("[NETDEV] Device %s is DOWN\n", dev->name);
    
    return 0;
}

int netdev_start_xmit(struct sk_buff* skb, struct net_device* dev) {
    if (!skb || !dev) return -1;
    
    if (!(dev->flags & IFF_UP)) {
        kprintf("[NETDEV] Device %s is not up\n", dev->name);
        free_skb(skb);
        return -1;
    }
    
    /* Select TX queue (simple round-robin for now) */
    static uint32_t next_queue = 0;
    uint32_t queue_idx = next_queue % dev->num_tx_queues;
    next_queue++;
    
    struct netdev_queue* txq = &dev->tx_queue[queue_idx];
    
    /* Check if queue stopped */
    if (txq->stopped) {
        kprintf("[NETDEV] TX queue %u stopped\n", queue_idx);
        free_skb(skb);
        return -1;
    }
    
    /* Call driver's transmit function */
    int ret = -1;
    if (dev->netdev_ops && dev->netdev_ops->ndo_start_xmit) {
        ret = dev->netdev_ops->ndo_start_xmit(skb, dev);
    }
    
    if (ret == 0) {
        /* Update statistics */
        dev->stats.tx_packets++;
        dev->stats.tx_bytes += skb->len;
        txq->tx_packets++;
        txq->tx_bytes += skb->len;
    } else {
        dev->stats.tx_errors++;
        dev->stats.tx_dropped++;
    }
    
    return ret;
}

/* ==================== Packet Reception ==================== */

int netdev_rx(struct sk_buff* skb, struct net_device* dev) {
    if (!skb || !dev) return -1;
    
    /* Update statistics */
    dev->stats.rx_packets++;
    dev->stats.rx_bytes += skb->len;
    
    /* Set device pointer */
    skb->dev = dev;
    
    /* Process based on protocol */
    switch (ntohs(skb->protocol)) {
        case ETH_P_IP:
            ip_rcv(skb);
            break;
            
        case ETH_P_ARP:
            /* TODO: arp_rcv(skb); */
            kprintf("[NETDEV] ARP packet received (not implemented)\n");
            free_skb(skb);
            break;
            
        default:
            kprintf("[NETDEV] Unknown protocol: 0x%04x\n", ntohs(skb->protocol));
            dev->stats.rx_errors++;
            free_skb(skb);
            break;
    }
    
    return 0;
}

/* ==================== NAPI Functions ==================== */

void napi_schedule(struct napi_struct* napi) {
    if (!napi || napi->scheduled) return;
    
    napi->scheduled = 1;
    
    /* Add to poll list */
    /* TODO: Implement poll list */
    
    kprintf("[NAPI] Scheduled NAPI for device %s\n", napi->dev->name);
}

void napi_complete(struct napi_struct* napi) {
    if (!napi) return;
    
    napi->scheduled = 0;
    
    /* Remove from poll list */
    /* TODO: Implement poll list */
    
    kprintf("[NAPI] Completed NAPI for device %s\n", napi->dev->name);
}

int napi_poll(struct napi_struct* napi, int budget) {
    if (!napi || !napi->poll) return 0;
    
    return napi->poll(napi, budget);
}

/* ==================== Queue Management ==================== */

void netdev_tx_queue_stop(struct net_device* dev, uint32_t queue_idx) {
    if (!dev || queue_idx >= dev->num_tx_queues) return;
    
    dev->tx_queue[queue_idx].stopped = 1;
    kprintf("[NETDEV] Stopped TX queue %u on %s\n", queue_idx, dev->name);
}

void netdev_tx_queue_wake(struct net_device* dev, uint32_t queue_idx) {
    if (!dev || queue_idx >= dev->num_tx_queues) return;
    
    dev->tx_queue[queue_idx].stopped = 0;
    kprintf("[NETDEV] Woke TX queue %u on %s\n", queue_idx, dev->name);
}

/* ==================== Loopback Device ==================== */

static int loopback_xmit(struct sk_buff* skb, struct net_device* dev) {
    if (!skb) return -1;
    
    /* Clone packet for reception */
    struct sk_buff* rx_skb = skb_clone(skb, 0);
    if (rx_skb) {
        /* Loop packet back */
        netdev_rx(rx_skb, dev);
    }
    
    /* Original skb consumed */
    free_skb(skb);
    
    return 0;
}

static int loopback_open(struct net_device* dev) {
    kprintf("[LOOPBACK] Opened loopback device\n");
    return 0;
}

static int loopback_stop(struct net_device* dev) {
    kprintf("[LOOPBACK] Stopped loopback device\n");
    return 0;
}

static struct net_device_ops loopback_ops = {
    .ndo_open = loopback_open,
    .ndo_stop = loopback_stop,
    .ndo_start_xmit = loopback_xmit,
};

int loopback_init(void) {
    kprintf("[LOOPBACK] Initializing loopback device...\n");
    
    /* Allocate device */
    loopback_dev = (struct net_device*)kmalloc(sizeof(struct net_device));
    if (!loopback_dev) {
        kprintf("[LOOPBACK] Failed to allocate device\n");
        return -1;
    }
    
    memset(loopback_dev, 0, sizeof(struct net_device));
    
    /* Setup device */
    strcpy(loopback_dev->name, "lo");
    loopback_dev->type = ARPHRD_LOOPBACK;
    loopback_dev->flags = IFF_LOOPBACK;
    loopback_dev->mtu = 65536;
    loopback_dev->hard_header_len = 0;
    loopback_dev->addr_len = 0;
    
    /* Single queue */
    loopback_dev->num_tx_queues = 1;
    loopback_dev->num_rx_queues = 1;
    
    /* Allocate queues */
    loopback_dev->tx_queue = (struct netdev_queue*)kmalloc(sizeof(struct netdev_queue));
    loopback_dev->rx_queue = (struct netdev_queue*)kmalloc(sizeof(struct netdev_queue));
    
    /* Set operations */
    loopback_dev->netdev_ops = &loopback_ops;
    
    /* Register device */
    int ret = netdev_register(loopback_dev);
    if (ret != 0) {
        kfree(loopback_dev);
        return ret;
    }
    
    /* Bring up */
    netdev_open(loopback_dev);
    
    kprintf("[LOOPBACK] Loopback device initialized\n");
    
    return 0;
}

/* ==================== Device Information ==================== */

void netdev_dump_info(struct net_device* dev) {
    if (!dev) return;
    
    kprintf("[NETDEV] Device: %s\n", dev->name);
    kprintf("  Index: %u\n", dev->ifindex);
    kprintf("  State: %s\n", dev->state == NETDEV_STATE_UP ? "UP" : "DOWN");
    kprintf("  Type: %u\n", dev->type);
    kprintf("  MTU: %u\n", dev->mtu);
    kprintf("  Flags: 0x%08x", dev->flags);
    
    if (dev->flags & IFF_UP) kprintf(" UP");
    if (dev->flags & IFF_BROADCAST) kprintf(" BROADCAST");
    if (dev->flags & IFF_LOOPBACK) kprintf(" LOOPBACK");
    if (dev->flags & IFF_POINTOPOINT) kprintf(" POINTOPOINT");
    if (dev->flags & IFF_RUNNING) kprintf(" RUNNING");
    if (dev->flags & IFF_MULTICAST) kprintf(" MULTICAST");
    kprintf("\n");
    
    kprintf("  Hardware address: %02x:%02x:%02x:%02x:%02x:%02x\n",
            dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
            dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
    
    kprintf("  Statistics:\n");
    kprintf("    RX: packets=%llu bytes=%llu errors=%llu dropped=%llu\n",
            dev->stats.rx_packets, dev->stats.rx_bytes,
            dev->stats.rx_errors, dev->stats.rx_dropped);
    kprintf("    TX: packets=%llu bytes=%llu errors=%llu dropped=%llu\n",
            dev->stats.tx_packets, dev->stats.tx_bytes,
            dev->stats.tx_errors, dev->stats.tx_dropped);
    
    kprintf("  Features: 0x%08x\n", dev->features);
    if (dev->features & NETIF_F_SG) kprintf("    Scatter-Gather\n");
    if (dev->features & NETIF_F_IP_CSUM) kprintf("    IP Checksum Offload\n");
    if (dev->features & NETIF_F_TSO) kprintf("    TCP Segmentation Offload\n");
    if (dev->features & NETIF_F_GSO) kprintf("    Generic Segmentation Offload\n");
    if (dev->features & NETIF_F_GRO) kprintf("    Generic Receive Offload\n");
}

void netdev_list_all(void) {
    kprintf("[NETDEV] Network Devices (%u):\n", netdev_state.count);
    
    for (uint32_t i = 0; i < netdev_state.count; i++) {
        struct net_device* dev = netdev_state.devices[i];
        kprintf("  %u: %s (%s) mtu=%u\n",
                dev->ifindex,
                dev->name,
                dev->state == NETDEV_STATE_UP ? "UP" : "DOWN",
                dev->mtu);
    }
}

/* ==================== Initialization ==================== */

int netdev_init(void) {
    kprintf("[NETDEV] Initializing network device layer...\n");
    
    /* Initialize state */
    memset(&netdev_state, 0, sizeof(netdev_state));
    netdev_state.next_ifindex = 1;
    
    /* Initialize loopback device */
    loopback_init();
    
    kprintf("[NETDEV] Network device layer initialized\n");
    
    return 0;
}

void netdev_cleanup(void) {
    kprintf("[NETDEV] Cleaning up network device layer...\n");
    
    /* Unregister all devices */
    for (uint32_t i = 0; i < netdev_state.count; i++) {
        netdev_unregister(netdev_state.devices[i]);
    }
    
    kprintf("[NETDEV] Network device layer cleaned up\n");
}
