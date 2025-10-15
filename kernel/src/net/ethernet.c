/*
 * Ethernet Protocol Implementation
 * IEEE 802.3 Ethernet Frame Handling
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/ethernet.h"
#include "net/arp.h"
#include "net/ip.h"
#include "net/sk_buff.h"
#include "net/netdevice.h"
#include "kernel/printk.h"
#include "kernel/string.h"
#include "kernel/stdlib.h"

/* Global ethernet statistics */
static ethernet_stats_t ethernet_stats = {0};

/* Broadcast MAC address */
static const uint8_t BROADCAST_MAC[ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/*
 * Initialize Ethernet Layer
 */
int ethernet_init(void) {
    memset(&ethernet_stats, 0, sizeof(ethernet_stats));
    printk(KERN_INFO "Ethernet layer initialized\n");
    return 0;
}

/*
 * Send Ethernet Frame
 * 
 * @dev: Network device
 * @dest_mac: Destination MAC address
 * @proto: EtherType (ETH_P_IP, ETH_P_ARP, etc.)
 * @skb: Socket buffer containing payload
 * @return: 0 on success, negative on error
 */
int ethernet_send(struct net_device* dev, const uint8_t* dest_mac,
                  uint16_t proto, struct sk_buff* skb) {
    ethhdr_t* eth;
    int ret;
    
    if (!dev || !dest_mac || !skb) {
        return -1;
    }
    
    /* Reserve space for ethernet header */
    if (skb_headroom(skb) < ETH_HLEN) {
        printk(KERN_ERR "ethernet_send: insufficient headroom\n");
        ethernet_stats.tx_errors++;
        return -1;
    }
    
    /* Push ethernet header */
    eth = (ethhdr_t*)skb_push(skb, ETH_HLEN);
    if (!eth) {
        ethernet_stats.tx_errors++;
        return -1;
    }
    
    /* Fill in ethernet header */
    memcpy(eth->h_dest, dest_mac, ETH_ALEN);
    memcpy(eth->h_source, dev->dev_addr, ETH_ALEN);
    eth->h_proto = htons(proto);
    
    /* Set skb fields */
    skb->dev = dev;
    skb->protocol = proto;
    
    /* Transmit via device driver */
    ret = dev->netdev_ops->ndo_start_xmit(skb, dev);
    if (ret == 0) {
        ethernet_stats.tx_packets++;
        ethernet_stats.tx_bytes += skb->len;
        
        if (ethernet_mac_is_broadcast(dest_mac)) {
            ethernet_stats.rx_broadcast++;  /* Track broadcast frames sent */
        } else if (ethernet_mac_is_multicast(dest_mac)) {
            ethernet_stats.rx_multicast++;  /* Track multicast frames sent */
        }
    } else {
        ethernet_stats.tx_errors++;
        ethernet_stats.tx_dropped++;
    }
    
    return ret;
}

/*
 * Receive Ethernet Frame
 * 
 * Called by device drivers when a frame is received
 * 
 * @skb: Socket buffer containing received frame
 */
void ethernet_rcv(struct sk_buff* skb) {
    ethhdr_t* eth;
    uint16_t proto;
    
    if (!skb) {
        return;
    }
    
    /* Update statistics */
    ethernet_stats.rx_packets++;
    ethernet_stats.rx_bytes += skb->len;
    
    /* Validate frame length */
    if (skb->len < ETH_HLEN) {
        printk(KERN_WARNING "ethernet_rcv: frame too short (%u bytes)\n", skb->len);
        ethernet_stats.rx_errors++;
        ethernet_stats.rx_dropped++;
        skb_free(skb);
        return;
    }
    
    /* Parse ethernet header */
    eth = (ethhdr_t*)skb->data;
    proto = ntohs(eth->h_proto);
    
    /* Track multicast/broadcast */
    if (ethernet_mac_is_broadcast(eth->h_dest)) {
        ethernet_stats.rx_broadcast++;
    } else if (ethernet_mac_is_multicast(eth->h_dest)) {
        ethernet_stats.rx_multicast++;
    }
    
    /* Validate destination MAC (accept broadcast, multicast, or our MAC) */
    if (!ethernet_mac_is_broadcast(eth->h_dest) &&
        !ethernet_mac_is_multicast(eth->h_dest) &&
        !ethernet_mac_equal(eth->h_dest, skb->dev->dev_addr)) {
        /* Not for us - drop silently */
        ethernet_stats.rx_dropped++;
        skb_free(skb);
        return;
    }
    
    /* Remove ethernet header */
    skb_pull(skb, ETH_HLEN);
    
    /* Set protocol in skb */
    skb->protocol = proto;
    
    /* Dispatch to protocol handler */
    switch (proto) {
        case ETH_P_IP:
            /* IPv4 packet */
            ip_rcv(skb);
            break;
            
        case ETH_P_ARP:
            /* ARP packet */
            arp_rcv(skb);
            break;
            
        case ETH_P_IPV6:
            /* IPv6 packet - not yet implemented */
            printk(KERN_DEBUG "ethernet_rcv: IPv6 not yet supported\n");
            ethernet_stats.rx_dropped++;
            skb_free(skb);
            break;
            
        case ETH_P_8021Q:
            /* VLAN-tagged packet - not yet implemented */
            printk(KERN_DEBUG "ethernet_rcv: VLAN not yet supported\n");
            ethernet_stats.rx_dropped++;
            skb_free(skb);
            break;
            
        default:
            /* Unknown protocol */
            printk(KERN_DEBUG "ethernet_rcv: unknown protocol 0x%04x\n", proto);
            ethernet_stats.rx_dropped++;
            skb_free(skb);
            break;
    }
}

/*
 * Compare two MAC addresses
 * 
 * @mac1: First MAC address
 * @mac2: Second MAC address
 * @return: 1 if equal, 0 if not
 */
int ethernet_mac_equal(const uint8_t* mac1, const uint8_t* mac2) {
    if (!mac1 || !mac2) {
        return 0;
    }
    return memcmp(mac1, mac2, ETH_ALEN) == 0;
}

/*
 * Check if MAC address is broadcast (FF:FF:FF:FF:FF:FF)
 * 
 * @mac: MAC address to check
 * @return: 1 if broadcast, 0 if not
 */
int ethernet_mac_is_broadcast(const uint8_t* mac) {
    if (!mac) {
        return 0;
    }
    return memcmp(mac, BROADCAST_MAC, ETH_ALEN) == 0;
}

/*
 * Check if MAC address is multicast (first bit set)
 * 
 * @mac: MAC address to check
 * @return: 1 if multicast, 0 if not
 */
int ethernet_mac_is_multicast(const uint8_t* mac) {
    if (!mac) {
        return 0;
    }
    /* Multicast MAC addresses have LSB of first octet set */
    return (mac[0] & 0x01) != 0;
}

/*
 * Check if MAC address is zero (00:00:00:00:00:00)
 * 
 * @mac: MAC address to check
 * @return: 1 if zero, 0 if not
 */
int ethernet_mac_is_zero(const uint8_t* mac) {
    static const uint8_t ZERO_MAC[ETH_ALEN] = {0};
    if (!mac) {
        return 1;
    }
    return memcmp(mac, ZERO_MAC, ETH_ALEN) == 0;
}

/*
 * Convert MAC address to string
 * 
 * @mac: MAC address
 * @str: Output string buffer (at least 18 bytes)
 * @len: Length of output buffer
 */
void ethernet_mac_to_str(const uint8_t* mac, char* str, uint32_t len) {
    if (!mac || !str || len < 18) {
        return;
    }
    snprintf(str, len, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/*
 * Convert string to MAC address
 * 
 * @str: MAC address string (e.g., "00:11:22:33:44:55")
 * @mac: Output MAC address buffer (at least 6 bytes)
 * @return: 0 on success, negative on error
 */
int ethernet_str_to_mac(const char* str, uint8_t* mac) {
    int i;
    unsigned int values[ETH_ALEN];
    
    if (!str || !mac) {
        return -1;
    }
    
    /* Parse MAC address */
    if (sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
               &values[0], &values[1], &values[2],
               &values[3], &values[4], &values[5]) != ETH_ALEN) {
        return -1;
    }
    
    /* Convert to bytes */
    for (i = 0; i < ETH_ALEN; i++) {
        if (values[i] > 0xFF) {
            return -1;
        }
        mac[i] = (uint8_t)values[i];
    }
    
    return 0;
}

/*
 * Get Ethernet Statistics
 * 
 * @stats: Output statistics structure
 */
void ethernet_get_stats(ethernet_stats_t* stats) {
    if (!stats) {
        return;
    }
    memcpy(stats, &ethernet_stats, sizeof(ethernet_stats_t));
}

/*
 * Dump Ethernet Statistics
 */
void ethernet_dump_stats(void) {
    printk(KERN_INFO "=== Ethernet Statistics ===\n");
    printk(KERN_INFO "RX: packets=%llu bytes=%llu errors=%llu dropped=%llu\n",
           ethernet_stats.rx_packets, ethernet_stats.rx_bytes,
           ethernet_stats.rx_errors, ethernet_stats.rx_dropped);
    printk(KERN_INFO "    multicast=%llu broadcast=%llu\n",
           ethernet_stats.rx_multicast, ethernet_stats.rx_broadcast);
    printk(KERN_INFO "TX: packets=%llu bytes=%llu errors=%llu dropped=%llu\n",
           ethernet_stats.tx_packets, ethernet_stats.tx_bytes,
           ethernet_stats.tx_errors, ethernet_stats.tx_dropped);
}
