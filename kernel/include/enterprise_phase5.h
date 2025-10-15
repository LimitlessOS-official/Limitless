#pragma once
/* Phase 5: Security and Enterprise Feature Aggregation Header */
#include "security.h"
#include "policy.h"

#ifdef __cplusplus
extern "C" { 
#endif

/* Firewall / Network Security */
status_t enterprise_firewall_init(void);
status_t enterprise_firewall_add_rule(const char* rule_expr);
status_t enterprise_firewall_stats(char* buf, size_t buf_sz);

/* VPN Management */
status_t enterprise_vpn_init(void);
status_t enterprise_vpn_create_profile(const char* name, const char* proto, const char* endpoint);
status_t enterprise_vpn_connect(const char* name);
status_t enterprise_vpn_disconnect(const char* name);

/* DNS over HTTPS */
status_t enterprise_doh_init(const char* bootstrap_ip);
status_t enterprise_doh_set_resolver(const char* url);

/* Identity & Directory Services */
status_t enterprise_identity_init(void);
status_t enterprise_identity_add_provider(const char* name, const char* type, const char* uri);
status_t enterprise_identity_authenticate(const char* provider, const char* principal, const char* secret);

/* Kerberos (stub) */
status_t enterprise_kerberos_init(void);
status_t enterprise_kerberos_get_tgt(const char* principal);

/* FIDO2 / Hardware Auth (stub) */
status_t enterprise_fido2_init(void);
status_t enterprise_fido2_register_key(const char* user, const uint8_t* pubkey, size_t len);

/* Compliance & Audit */
status_t enterprise_compliance_init(void);
status_t enterprise_compliance_record(const char* control_id, const char* status, const char* notes);
status_t enterprise_compliance_generate_report(char* buf, size_t buf_sz);

/* Backup System */
status_t enterprise_backup_init(void);
status_t enterprise_backup_run(const char* target_path);
status_t enterprise_backup_schedule(uint32_t interval_hours);

/* Encryption / Key Rotation Helpers */
status_t enterprise_crypto_key_rotate(uint32_t key_id);
status_t enterprise_crypto_list_keys(char* buf, size_t buf_sz);

/* Enterprise Feature Orchestrator */
status_t enterprise_phase5_init_all(void);

/* Metrics */
typedef struct enterprise_phase5_metrics {
    uint64_t firewall_rules;
    uint64_t vpn_profiles;
    uint64_t vpn_active;
    uint64_t doh_queries;
    uint64_t identity_providers;
    uint64_t kerberos_tgts;
    uint64_t fido2_keys;
    uint64_t compliance_events;
    uint64_t backups_completed;
    uint64_t key_rotations;
} enterprise_phase5_metrics_t;

status_t enterprise_phase5_get_metrics(enterprise_phase5_metrics_t* out);

#ifdef __cplusplus
}
#endif
