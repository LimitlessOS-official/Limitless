#pragma once
#include "kernel.h"

typedef struct {
    int firewall_in_default;   /* 0=DROP,1=ACCEPT */
    int firewall_out_default;  /* 0=DROP,1=ACCEPT */
    int updates_allowed;       /* 0/1 */
    /* Update window (naive: allow any time if 0, else restrict to [start,end) UTC hours) */
    int update_window_start;   /* 0..23 or 0 to disable */
    int update_window_end;     /* 1..24 or 0 to disable */
} policy_t;

/* Initialize global policy by reading /etc/policy.json if present, else defaults */
void policy_init(void);

/* Accessors */
const policy_t* policy_current(void);
int policy_updates_permitted(void);
