/*
 * QoS - Quality of Service
 * Traffic Classification and Queueing
 * 
 * Copyright (c) 2025 LimitlessOS Project
 */

#pragma once
#include "types.h"
#include "net/sk_buff.h"

/* QoS Traffic Classes */
#define QOS_CLASS_BE         0   /* Best Effort (default) */
#define QOS_CLASS_BK         1   /* Background */
#define QOS_CLASS_EE         2   /* Excellent Effort */
#define QOS_CLASS_CA         3   /* Critical Applications */
#define QOS_CLASS_VI         4   /* Video, < 100ms latency */
#define QOS_CLASS_VO         5   /* Voice, < 10ms latency */
#define QOS_CLASS_IC         6   /* Internetwork Control */
#define QOS_CLASS_NC         7   /* Network Control */
#define QOS_CLASS_MAX        8

/* Queueing Disciplines */
#define QOS_QDISC_FIFO       0   /* First-In-First-Out */
#define QOS_QDISC_PRIO       1   /* Priority queueing */
#define QOS_QDISC_RED        2   /* Random Early Detection */
#define QOS_QDISC_TBF        3   /* Token Bucket Filter */
#define QOS_QDISC_HTB        4   /* Hierarchical Token Bucket */

/* QoS Marks (DSCP values) */
#define QOS_DSCP_CS0         0   /* Class Selector 0 (Best Effort) */
#define QOS_DSCP_CS1         8   /* Class Selector 1 */
#define QOS_DSCP_AF11        10  /* Assured Forwarding 11 */
#define QOS_DSCP_AF12        12  /* Assured Forwarding 12 */
#define QOS_DSCP_AF13        14  /* Assured Forwarding 13 */
#define QOS_DSCP_CS2         16  /* Class Selector 2 */
#define QOS_DSCP_AF21        18  /* Assured Forwarding 21 */
#define QOS_DSCP_AF22        20  /* Assured Forwarding 22 */
#define QOS_DSCP_AF23        22  /* Assured Forwarding 23 */
#define QOS_DSCP_CS3         24  /* Class Selector 3 */
#define QOS_DSCP_AF31        26  /* Assured Forwarding 31 */
#define QOS_DSCP_AF32        28  /* Assured Forwarding 32 */
#define QOS_DSCP_AF33        30  /* Assured Forwarding 33 */
#define QOS_DSCP_CS4         32  /* Class Selector 4 */
#define QOS_DSCP_AF41        34  /* Assured Forwarding 41 */
#define QOS_DSCP_AF42        36  /* Assured Forwarding 42 */
#define QOS_DSCP_AF43        38  /* Assured Forwarding 43 */
#define QOS_DSCP_CS5         40  /* Class Selector 5 */
#define QOS_DSCP_EF          46  /* Expedited Forwarding */
#define QOS_DSCP_CS6         48  /* Class Selector 6 */
#define QOS_DSCP_CS7         56  /* Class Selector 7 */

/* Maximum queue depth per class */
#define QOS_MAX_QUEUE_DEPTH  256

/* Forward declarations */
struct net_device;

/* QoS Queue */
typedef struct qos_queue {
    struct sk_buff* packets[QOS_MAX_QUEUE_DEPTH];
    uint32_t head;               /* Queue head index */
    uint32_t tail;               /* Queue tail index */
    uint32_t count;              /* Number of packets in queue */
    uint32_t dropped;            /* Packets dropped due to overflow */
    uint64_t bytes;              /* Total bytes queued */
} qos_queue_t;

/* QoS Class Configuration */
typedef struct qos_class {
    uint8_t  class_id;           /* QoS class ID */
    uint8_t  priority;           /* Priority (0-7, higher = more priority) */
    uint32_t bandwidth;          /* Bandwidth limit (bytes/sec) */
    uint32_t burst;              /* Burst size (bytes) */
    qos_queue_t queue;           /* Packet queue */
    
    /* Statistics */
    uint64_t packets_enqueued;
    uint64_t packets_dequeued;
    uint64_t packets_dropped;
    uint64_t bytes_enqueued;
    uint64_t bytes_dequeued;
} qos_class_t;

/* QoS Classification Rule */
typedef struct qos_rule {
    struct qos_rule* next;
    
    /* Match criteria */
    uint32_t match_src_ip;
    uint32_t match_src_mask;
    uint32_t match_dst_ip;
    uint32_t match_dst_mask;
    uint16_t match_src_port_min;
    uint16_t match_src_port_max;
    uint16_t match_dst_port_min;
    uint16_t match_dst_port_max;
    uint8_t  match_protocol;     /* 0 = any */
    uint8_t  match_dscp;         /* DSCP value to match (0xFF = any) */
    
    /* Classification result */
    uint8_t  target_class;       /* QoS class to assign */
    uint8_t  set_dscp;           /* DSCP value to set (0xFF = don't set) */
} qos_rule_t;

/* QoS Statistics */
typedef struct qos_stats {
    uint64_t packets_classified;
    uint64_t packets_enqueued;
    uint64_t packets_dequeued;
    uint64_t packets_dropped;
    uint64_t bytes_enqueued;
    uint64_t bytes_dequeued;
} qos_stats_t;

/* QoS Functions */

/* Initialization */
int qos_init(void);

/* Classification rules */
int qos_add_rule(qos_rule_t* rule);
int qos_delete_rule(qos_rule_t* rule);
void qos_flush_rules(void);

/* Packet classification */
uint8_t qos_classify_packet(struct sk_buff* skb);

/* Queueing */
int qos_enqueue(struct sk_buff* skb, uint8_t class_id);
struct sk_buff* qos_dequeue(void);  /* Dequeue next packet based on priority */
struct sk_buff* qos_dequeue_class(uint8_t class_id);  /* Dequeue from specific class */

/* Class configuration */
int qos_set_class_bandwidth(uint8_t class_id, uint32_t bandwidth, uint32_t burst);
int qos_set_class_priority(uint8_t class_id, uint8_t priority);

/* Statistics */
void qos_get_stats(qos_stats_t* stats);
void qos_get_class_stats(uint8_t class_id, qos_class_t* stats);
void qos_dump_stats(void);
void qos_dump_class_stats(uint8_t class_id);
