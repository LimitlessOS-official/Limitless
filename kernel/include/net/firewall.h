#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "kernel.h"
#include "net/filter.h"

#define FW_MAX_RULES        1024
#define FW_MAX_CHAINS       8
#define FW_MAX_NAME_LEN     32
#define FW_CONN_TRACK_BUCKETS 256

/* Rule actions */
typedef enum { FW_ACT_DROP=0, FW_ACT_ACCEPT=1, FW_ACT_REJECT=2, FW_ACT_LOG=3, FW_ACT_RETURN=4 } fw_action_t;

/* Protocol constants (subset) */
#ifndef IPPROTO_TCP
#define IPPROTO_TCP 6
#endif
#ifndef IPPROTO_UDP
#define IPPROTO_UDP 17
#endif
#ifndef IPPROTO_ICMP
#define IPPROTO_ICMP 1
#endif

/* Match flags */
#define FW_MATCH_SRC_IP   (1u<<0)
#define FW_MATCH_DST_IP   (1u<<1)
#define FW_MATCH_PROTO    (1u<<2)
#define FW_MATCH_SRC_PORT (1u<<3)
#define FW_MATCH_DST_PORT (1u<<4)
#define FW_MATCH_STATE    (1u<<5)

/* Connection state */
typedef enum { FW_STATE_NEW=0, FW_STATE_ESTABLISHED=1, FW_STATE_RELATED=2 } fw_conn_state_t;

/* Rule representation */
typedef struct fw_rule {
    char        name[FW_MAX_NAME_LEN];
    uint32_t    match_flags;     /* FW_MATCH_* */
    uint32_t    src_ip_be;       /* network byte order */
    uint32_t    src_mask_be;     /* CIDR mask */
    uint32_t    dst_ip_be;
    uint32_t    dst_mask_be;
    uint16_t    src_port_be;     /* network order */
    uint16_t    dst_port_be;     /* network order */
    uint8_t     proto;           /* IPPROTO_* */
    uint8_t     direction;       /* 0=in,1=out */
    uint8_t     state_mask;      /* bitmask of fw_conn_state_t */
    fw_action_t action;          /* terminal action */
    uint64_t    hit_count;       /* runtime stats */
    uint64_t    byte_count;      /* aggregate bytes */
    bool        enabled;         /* enable/disable */
} fw_rule_t;

/* Chain */
typedef struct fw_chain {
    char     name[FW_MAX_NAME_LEN];
    uint32_t rule_start;  /* index into global rule array */
    uint32_t rule_count;  /* number of rules in chain */
    bool     built_in;    /* cannot delete if true */
} fw_chain_t;

/* Connection tracking entry */
typedef struct fw_conn_entry {
    uint32_t src_ip_be;
    uint32_t dst_ip_be;
    uint16_t src_port_be;
    uint16_t dst_port_be;
    uint8_t  proto;
    uint8_t  direction; /* original direction */
    fw_conn_state_t state;
    uint64_t last_seen_ticks;
    struct fw_conn_entry* next; /* hash bucket chaining */
} fw_conn_entry_t;

/* Metrics */
typedef struct fw_metrics {
    uint64_t packets_inspected;
    uint64_t packets_dropped;
    uint64_t packets_accepted;
    uint64_t packets_rejected;
    uint64_t rules_defined;
    uint64_t rules_matched;
    uint64_t connections_tracked;
} fw_metrics_t;

/* Public API */
status_t fw_init(void);
status_t fw_add_rule(const fw_rule_t* rule, uint32_t* out_index);
status_t fw_delete_rule(uint32_t index);
status_t fw_enable_rule(uint32_t index, bool enable);
status_t fw_get_rule(uint32_t index, fw_rule_t* out);
status_t fw_replace_rule(uint32_t index, const fw_rule_t* rule);
status_t fw_flush(void);
status_t fw_get_metrics(fw_metrics_t* out);
status_t fw_conntrack_sweep(uint64_t now_ticks, uint64_t timeout_ticks);

/* Packet evaluation (invoked by nf_check via weak symbol) */
int fw_apply_rules(const nf_packet_t* p, nf_verdict_t* out_verdict);

/* Persistence */
status_t fw_save_rules(const char* path);
status_t fw_load_rules(const char* path);
