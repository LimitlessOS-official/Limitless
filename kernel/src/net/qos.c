/*
 * QoS Implementation
 * Traffic Classification and Priority Queueing
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#include "net/qos.h"
#include "net/ip.h"
#include "net/tcp.h"
#include "net/udp.h"
#include "net/sk_buff.h"
#include "kernel/printk.h"
#include "kernel/string.h"
#include "kernel/stdlib.h"

/* QoS classes */
static qos_class_t qos_classes[QOS_CLASS_MAX];

/* Classification rules */
static qos_rule_t* qos_rules = NULL;

/* Global QoS statistics */
static qos_stats_t qos_stats = {0};

/*
 * Initialize QoS
 */
int qos_init(void) {
    int i;
    
    /* Initialize classes */
    for (i = 0; i < QOS_CLASS_MAX; i++) {
        memset(&qos_classes[i], 0, sizeof(qos_class_t));
        qos_classes[i].class_id = i;
        qos_classes[i].priority = i;  /* Default: higher class = higher priority */
        qos_classes[i].bandwidth = 0;  /* Unlimited */
        qos_classes[i].burst = 0;
    }
    
    /* Set default priorities */
    qos_classes[QOS_CLASS_BE].priority = 0;  /* Best Effort - lowest */
    qos_classes[QOS_CLASS_BK].priority = 1;  /* Background */
    qos_classes[QOS_CLASS_EE].priority = 2;  /* Excellent Effort */
    qos_classes[QOS_CLASS_CA].priority = 3;  /* Critical Applications */
    qos_classes[QOS_CLASS_VI].priority = 4;  /* Video */
    qos_classes[QOS_CLASS_VO].priority = 5;  /* Voice */
    qos_classes[QOS_CLASS_IC].priority = 6;  /* Internetwork Control */
    qos_classes[QOS_CLASS_NC].priority = 7;  /* Network Control - highest */
    
    qos_rules = NULL;
    memset(&qos_stats, 0, sizeof(qos_stats));
    
    printk(KERN_INFO "QoS initialized with %d traffic classes\n", QOS_CLASS_MAX);
    return 0;
}

/*
 * Add classification rule
 * 
 * @rule: Classification rule to add
 * @return: 0 on success, negative on error
 */
int qos_add_rule(qos_rule_t* rule) {
    qos_rule_t* new_rule;
    
    if (!rule || rule->target_class >= QOS_CLASS_MAX) {
        return -1;
    }
    
    /* Allocate and copy rule */
    new_rule = (qos_rule_t*)malloc(sizeof(qos_rule_t));
    if (!new_rule) {
        return -1;
    }
    
    memcpy(new_rule, rule, sizeof(qos_rule_t));
    
    /* Add to head of list */
    new_rule->next = qos_rules;
    qos_rules = new_rule;
    
    printk(KERN_INFO "QoS: Added classification rule -> class %u\n", rule->target_class);
    
    return 0;
}

/*
 * Delete classification rule
 * 
 * @rule: Rule to delete
 * @return: 0 on success, negative on error
 */
int qos_delete_rule(qos_rule_t* rule) {
    qos_rule_t** prev = &qos_rules;
    qos_rule_t* curr = qos_rules;
    
    if (!rule) {
        return -1;
    }
    
    while (curr) {
        if (curr == rule) {
            *prev = curr->next;
            free(curr);
            return 0;
        }
        prev = &curr->next;
        curr = curr->next;
    }
    
    return -1;
}

/*
 * Flush all classification rules
 */
void qos_flush_rules(void) {
    qos_rule_t* rule = qos_rules;
    qos_rule_t* next;
    
    while (rule) {
        next = rule->next;
        free(rule);
        rule = next;
    }
    
    qos_rules = NULL;
}

/*
 * Classify packet
 * 
 * @skb: Socket buffer
 * @return: QoS class ID
 */
uint8_t qos_classify_packet(struct sk_buff* skb) {
    iphdr_t* iph;
    tcphdr_t* tcph;
    udphdr_t* udph;
    qos_rule_t* rule;
    uint32_t src_ip, dst_ip;
    uint16_t src_port = 0, dst_port = 0;
    uint8_t protocol, dscp;
    
    if (!skb || skb->len < sizeof(iphdr_t)) {
        return QOS_CLASS_BE;  /* Default to Best Effort */
    }
    
    iph = (iphdr_t*)skb->data;
    protocol = iph->protocol;
    src_ip = ntohl(iph->saddr);
    dst_ip = ntohl(iph->daddr);
    dscp = (iph->tos >> 2) & 0x3F;  /* Extract DSCP from TOS field */
    
    /* Extract port numbers */
    if (protocol == IPPROTO_TCP && skb->len >= sizeof(iphdr_t) + sizeof(tcphdr_t)) {
        tcph = (tcphdr_t*)(skb->data + (iph->ihl * 4));
        src_port = ntohs(tcph->source);
        dst_port = ntohs(tcph->dest);
    } else if (protocol == IPPROTO_UDP && skb->len >= sizeof(iphdr_t) + sizeof(udphdr_t)) {
        udph = (udphdr_t*)(skb->data + (iph->ihl * 4));
        src_port = ntohs(udph->source);
        dst_port = ntohs(udph->dest);
    }
    
    /* Try to match against classification rules */
    for (rule = qos_rules; rule; rule = rule->next) {
        /* Check protocol */
        if (rule->match_protocol && rule->match_protocol != protocol) {
            continue;
        }
        
        /* Check source IP */
        if ((src_ip & rule->match_src_mask) != (rule->match_src_ip & rule->match_src_mask)) {
            continue;
        }
        
        /* Check destination IP */
        if ((dst_ip & rule->match_dst_mask) != (rule->match_dst_ip & rule->match_dst_mask)) {
            continue;
        }
        
        /* Check source port */
        if (rule->match_src_port_min || rule->match_src_port_max) {
            if (src_port < rule->match_src_port_min || src_port > rule->match_src_port_max) {
                continue;
            }
        }
        
        /* Check destination port */
        if (rule->match_dst_port_min || rule->match_dst_port_max) {
            if (dst_port < rule->match_dst_port_min || dst_port > rule->match_dst_port_max) {
                continue;
            }
        }
        
        /* Check DSCP */
        if (rule->match_dscp != 0xFF && rule->match_dscp != dscp) {
            continue;
        }
        
        /* Rule matched! */
        qos_stats.packets_classified++;
        
        /* Set DSCP if requested */
        if (rule->set_dscp != 0xFF) {
            iph->tos = (iph->tos & 0x03) | (rule->set_dscp << 2);
            /* Recalculate IP checksum */
            iph->check = 0;
            iph->check = ip_checksum(iph, iph->ihl * 4);
        }
        
        return rule->target_class;
    }
    
    /* No rule matched - use DSCP-based default classification */
    if (dscp >= QOS_DSCP_CS6) {
        return QOS_CLASS_NC;  /* Network Control */
    } else if (dscp >= QOS_DSCP_EF) {
        return QOS_CLASS_VO;  /* Voice */
    } else if (dscp >= QOS_DSCP_CS4) {
        return QOS_CLASS_VI;  /* Video */
    } else if (dscp >= QOS_DSCP_CS3) {
        return QOS_CLASS_CA;  /* Critical Applications */
    } else if (dscp >= QOS_DSCP_CS2) {
        return QOS_CLASS_EE;  /* Excellent Effort */
    } else if (dscp == QOS_DSCP_CS1) {
        return QOS_CLASS_BK;  /* Background */
    }
    
    return QOS_CLASS_BE;  /* Best Effort */
}

/*
 * Enqueue packet
 * 
 * @skb: Socket buffer
 * @class_id: QoS class ID
 * @return: 0 on success, negative on error
 */
int qos_enqueue(struct sk_buff* skb, uint8_t class_id) {
    qos_class_t* qc;
    qos_queue_t* q;
    uint32_t next_tail;
    
    if (!skb || class_id >= QOS_CLASS_MAX) {
        return -1;
    }
    
    qc = &qos_classes[class_id];
    q = &qc->queue;
    
    /* Check if queue is full */
    if (q->count >= QOS_MAX_QUEUE_DEPTH) {
        qc->packets_dropped++;
        qos_stats.packets_dropped++;
        q->dropped++;
        skb_free(skb);
        return -1;
    }
    
    /* Enqueue packet */
    q->packets[q->tail] = skb;
    next_tail = (q->tail + 1) % QOS_MAX_QUEUE_DEPTH;
    q->tail = next_tail;
    q->count++;
    q->bytes += skb->len;
    
    /* Update statistics */
    qc->packets_enqueued++;
    qc->bytes_enqueued += skb->len;
    qos_stats.packets_enqueued++;
    qos_stats.bytes_enqueued += skb->len;
    
    return 0;
}

/*
 * Dequeue packet from highest priority non-empty queue
 * 
 * @return: Socket buffer or NULL if all queues empty
 */
struct sk_buff* qos_dequeue(void) {
    int i;
    int highest_priority = -1;
    uint8_t best_class = 0;
    
    /* Find highest priority non-empty queue */
    for (i = 0; i < QOS_CLASS_MAX; i++) {
        if (qos_classes[i].queue.count > 0) {
            if (qos_classes[i].priority > highest_priority) {
                highest_priority = qos_classes[i].priority;
                best_class = i;
            }
        }
    }
    
    if (highest_priority < 0) {
        return NULL;  /* All queues empty */
    }
    
    return qos_dequeue_class(best_class);
}

/*
 * Dequeue packet from specific class
 * 
 * @class_id: QoS class ID
 * @return: Socket buffer or NULL if queue empty
 */
struct sk_buff* qos_dequeue_class(uint8_t class_id) {
    qos_class_t* qc;
    qos_queue_t* q;
    struct sk_buff* skb;
    
    if (class_id >= QOS_CLASS_MAX) {
        return NULL;
    }
    
    qc = &qos_classes[class_id];
    q = &qc->queue;
    
    if (q->count == 0) {
        return NULL;  /* Queue empty */
    }
    
    /* Dequeue packet */
    skb = q->packets[q->head];
    q->packets[q->head] = NULL;
    q->head = (q->head + 1) % QOS_MAX_QUEUE_DEPTH;
    q->count--;
    q->bytes -= skb->len;
    
    /* Update statistics */
    qc->packets_dequeued++;
    qc->bytes_dequeued += skb->len;
    qos_stats.packets_dequeued++;
    qos_stats.bytes_dequeued += skb->len;
    
    return skb;
}

/*
 * Set class bandwidth limit
 * 
 * @class_id: QoS class ID
 * @bandwidth: Bandwidth limit (bytes/sec)
 * @burst: Burst size (bytes)
 * @return: 0 on success, negative on error
 */
int qos_set_class_bandwidth(uint8_t class_id, uint32_t bandwidth, uint32_t burst) {
    if (class_id >= QOS_CLASS_MAX) {
        return -1;
    }
    
    qos_classes[class_id].bandwidth = bandwidth;
    qos_classes[class_id].burst = burst;
    
    return 0;
}

/*
 * Set class priority
 * 
 * @class_id: QoS class ID
 * @priority: Priority (0-7, higher = more priority)
 * @return: 0 on success, negative on error
 */
int qos_set_class_priority(uint8_t class_id, uint8_t priority) {
    if (class_id >= QOS_CLASS_MAX || priority > 7) {
        return -1;
    }
    
    qos_classes[class_id].priority = priority;
    
    return 0;
}

/*
 * Get global QoS statistics
 * 
 * @stats: Output statistics structure
 */
void qos_get_stats(qos_stats_t* stats) {
    if (!stats) {
        return;
    }
    memcpy(stats, &qos_stats, sizeof(qos_stats_t));
}

/*
 * Get per-class statistics
 * 
 * @class_id: QoS class ID
 * @stats: Output statistics structure
 */
void qos_get_class_stats(uint8_t class_id, qos_class_t* stats) {
    if (!stats || class_id >= QOS_CLASS_MAX) {
        return;
    }
    memcpy(stats, &qos_classes[class_id], sizeof(qos_class_t));
}

/*
 * Dump global QoS statistics
 */
void qos_dump_stats(void) {
    printk(KERN_INFO "=== QoS Global Statistics ===\n");
    printk(KERN_INFO "Classified: %llu packets\n", qos_stats.packets_classified);
    printk(KERN_INFO "Enqueued: %llu packets, %llu bytes\n",
           qos_stats.packets_enqueued, qos_stats.bytes_enqueued);
    printk(KERN_INFO "Dequeued: %llu packets, %llu bytes\n",
           qos_stats.packets_dequeued, qos_stats.bytes_dequeued);
    printk(KERN_INFO "Dropped: %llu packets\n", qos_stats.packets_dropped);
}

/*
 * Dump per-class statistics
 * 
 * @class_id: QoS class ID
 */
void qos_dump_class_stats(uint8_t class_id) {
    qos_class_t* qc;
    const char* class_names[] = {
        "Best Effort", "Background", "Excellent Effort", "Critical Applications",
        "Video", "Voice", "Internetwork Control", "Network Control"
    };
    
    if (class_id >= QOS_CLASS_MAX) {
        return;
    }
    
    qc = &qos_classes[class_id];
    
    printk(KERN_INFO "=== QoS Class %u (%s) Statistics ===\n",
           class_id, class_names[class_id]);
    printk(KERN_INFO "Priority: %u\n", qc->priority);
    printk(KERN_INFO "Bandwidth: %u bytes/sec, Burst: %u bytes\n",
           qc->bandwidth, qc->burst);
    printk(KERN_INFO "Queue: %u/%u packets, %llu bytes\n",
           qc->queue.count, QOS_MAX_QUEUE_DEPTH, qc->queue.bytes);
    printk(KERN_INFO "Enqueued: %llu packets, %llu bytes\n",
           qc->packets_enqueued, qc->bytes_enqueued);
    printk(KERN_INFO "Dequeued: %llu packets, %llu bytes\n",
           qc->packets_dequeued, qc->bytes_dequeued);
    printk(KERN_INFO "Dropped: %llu packets (%u queue drops)\n",
           qc->packets_dropped, qc->queue.dropped);
}
