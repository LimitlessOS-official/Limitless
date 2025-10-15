/**
 * LimitlessOS Enterprise Deployment Manager Header
 * Advanced installer features for enterprise deployment scenarios
 */

#ifndef ENTERPRISE_DEPLOYMENT_H
#define ENTERPRISE_DEPLOYMENT_H

#include <stdint.h>
#include <stdbool.h>
#include "installer.h"
#include "gui_installer.h"

/* Deployment Types */
typedef enum {
    DEPLOYMENT_TYPE_STANDARD,          /* Standard workstation */
    DEPLOYMENT_TYPE_ENTERPRISE,        /* Enterprise workstation */
    DEPLOYMENT_TYPE_DEVELOPER,         /* Developer workstation */
    DEPLOYMENT_TYPE_KIOSK,             /* Kiosk/public access */
    DEPLOYMENT_TYPE_SERVER,            /* Server installation */
    DEPLOYMENT_TYPE_CLOUD,             /* Cloud instance */
    DEPLOYMENT_TYPE_EMBEDDED,          /* Embedded system */
    DEPLOYMENT_TYPE_VIRTUAL_MACHINE    /* Virtual machine */
} deployment_type_t;

/* Automation Levels */
typedef enum {
    AUTOMATION_LEVEL_MANUAL,           /* Manual installation */
    AUTOMATION_LEVEL_INTERACTIVE,      /* Interactive with prompts */
    AUTOMATION_LEVEL_SEMI_AUTOMATED,   /* Some automation with user confirmation */
    AUTOMATION_LEVEL_FULLY_AUTOMATED   /* Fully automated, no user interaction */
} automation_level_t;

/* Validation Levels */
typedef enum {
    VALIDATION_LEVEL_MINIMAL,          /* Basic validation only */
    VALIDATION_LEVEL_STANDARD,         /* Standard validation */
    VALIDATION_LEVEL_COMPREHENSIVE,    /* Comprehensive validation */
    VALIDATION_LEVEL_STRICT           /* Strict validation (fail on warnings) */
} validation_level_t;

/* Compliance Modes */
typedef enum {
    COMPLIANCE_MODE_NONE,              /* No compliance enforcement */
    COMPLIANCE_MODE_BASIC,             /* Basic compliance checks */
    COMPLIANCE_MODE_ENTERPRISE,        /* Enterprise compliance */
    COMPLIANCE_MODE_GOVERNMENT,        /* Government compliance */
    COMPLIANCE_MODE_MILITARY          /* Military-grade compliance */
} compliance_mode_t;

/* Configuration Sources */
typedef enum {
    CONFIG_SOURCE_LOCAL,               /* Local configuration file */
    CONFIG_SOURCE_NETWORK,             /* Network-based configuration */
    CONFIG_SOURCE_USB,                 /* USB configuration */
    CONFIG_SOURCE_CLOUD               /* Cloud-based configuration */
} config_source_t;

/* Deployment Profile */
typedef struct {
    deployment_type_t deployment_type;
    automation_level_t automation_level;
    validation_level_t validation_level;
    compliance_mode_t compliance_mode;
    char profile_name[64];
    char description[256];
    uint32_t version;
    time_t created_timestamp;
} deployment_profile_t;

/* Automated Configuration */
typedef struct {
    bool enabled;
    config_source_t config_source;
    char config_file_path[256];
    char config_server_url[256];
    char deployment_key[128];
    
    /* Feature selections */
    bool install_ai_framework;
    bool install_development_tools;
    bool install_enterprise_tools;
    bool install_multimedia_codecs;
    bool install_quantum_computing;
    bool install_blockchain_support;
    
    /* System settings */
    char default_username[64];
    char default_hostname[64];
    char default_timezone[64];
    char dns_servers[4][64];
    
    /* Security settings */
    bool force_encryption;
    bool force_secure_boot;
    bool force_tpm;
    char encryption_passphrase[128];
} automated_config_t;

/* Validation Suite */
typedef struct {
    bool enabled;
    bool hardware_validation;
    bool security_validation;
    bool compliance_validation;
    bool performance_validation;
    bool network_validation;
    bool post_install_testing;
    
    /* Validation results */
    uint32_t tests_passed;
    uint32_t tests_failed;
    uint32_t tests_skipped;
    char last_failure[256];
} validation_suite_t;

/* Provisioning Manager */
typedef struct {
    bool enabled;
    bool auto_join_domain;
    char domain_controller[256];
    char domain_name[128];
    char organizational_unit[128];
    char service_account[64];
    char service_password[128];
    
    /* Certificate management */
    bool auto_enroll_certificates;
    char certificate_authority[256];
    char certificate_template[64];
    
    /* Group policy */
    bool apply_group_policy;
    char group_policy_url[256];
} provisioning_manager_t;

/* Compliance Checker */
typedef struct {
    bool enabled;
    bool enforce_encryption;
    bool enforce_secure_boot;
    bool enforce_tpm;
    bool enforce_virtualization;
    bool enforce_aes_ni;
    bool audit_logging;
    
    /* Compliance standards */
    bool iso27001_compliance;
    bool sox_compliance;
    bool hipaa_compliance;
    bool pci_dss_compliance;
    bool fisma_compliance;
    
    /* Results */
    uint32_t compliance_score;
    bool compliance_passed;
    char compliance_report[1024];
} compliance_checker_t;

/* Telemetry Manager */
typedef struct {
    bool enabled;
    bool anonymous_only;
    bool local_only;
    char server_url[256];
    char api_key[128];
    
    /* Data collection settings */
    bool collect_hardware_info;
    bool collect_performance_metrics;
    bool collect_error_reports;
    bool collect_usage_statistics;
    
    /* Privacy settings */
    bool user_consent_required;
    bool opt_out_available;
    uint32_t retention_days;
} telemetry_manager_t;

/* Rollback Manager */
typedef struct {
    bool enabled;
    bool create_snapshots;
    uint32_t max_snapshots;
    char snapshot_location[256];
    
    /* Rollback triggers */
    bool auto_rollback_on_failure;
    bool auto_rollback_on_boot_failure;
    uint32_t rollback_timeout_minutes;
} rollback_manager_t;

/* Disk Information Extended */
typedef struct {
    char device_path[256];
    char model[128];
    char serial_number[64];
    uint64_t size_bytes;
    uint32_t sector_size;
    bool is_ssd;
    bool is_nvme;
    bool is_removable;
    bool supports_trim;
    bool supports_encryption;
    uint32_t performance_score;
    
    /* Health information */
    uint32_t temperature_celsius;
    uint32_t power_on_hours;
    uint32_t write_cycles;
    bool smart_healthy;
} disk_info_extended_t;

/* Network Configuration */
typedef struct {
    bool configure_automatically;
    char interface_name[32];
    char ip_address[64];
    char subnet_mask[64];
    char gateway[64];
    char dns_primary[64];
    char dns_secondary[64];
    
    /* Wireless settings */
    char wifi_ssid[64];
    char wifi_password[128];
    char wifi_security[32];
    
    /* Enterprise network */
    bool use_802_1x;
    char eap_method[32];
    char eap_username[64];
    char eap_password[128];
} network_config_t;

/* Function Prototypes */

/* Core Enterprise Deployment Functions */
status_t enterprise_deployment_init(void);
status_t enterprise_deployment_load_config(const char* config_file);
status_t enterprise_deployment_generate_template(const char* output_file);
void enterprise_deployment_cleanup(void);

/* Validation Functions */
status_t enterprise_validate_hardware_compliance(const hardware_info_t* hardware);
status_t enterprise_validate_security_requirements(void);
status_t enterprise_validate_network_configuration(const network_config_t* network);
status_t enterprise_validate_installation(void);

/* Automated Deployment Functions */
status_t enterprise_execute_automated_deployment(void);
status_t enterprise_create_deployment_package(const char* output_path);
status_t enterprise_deploy_from_package(const char* package_path);

/* Configuration Management */
status_t enterprise_generate_installation_config(installation_config_t* config);
status_t enterprise_apply_configuration(void);
status_t enterprise_export_configuration(const char* output_file);
status_t enterprise_import_configuration(const char* config_file);

/* Security and Compliance */
status_t enterprise_apply_security_hardening(void);
status_t enterprise_generate_compliance_report(const char* output_file);
status_t enterprise_check_compliance_standards(void);

/* Provisioning Functions */
status_t enterprise_provision_system(void);
status_t enterprise_join_domain(const char* domain, const char* username, const char* password);
status_t enterprise_apply_group_policy(void);
status_t enterprise_enroll_certificates(void);

/* Disk Management */
status_t enterprise_auto_partition_disks(void);
status_t enterprise_select_optimal_disk(disk_info_t* selected_disk);
status_t enterprise_analyze_disk_performance(const char* device_path, uint32_t* performance_score);
status_t enterprise_setup_raid_configuration(void);

/* Monitoring and Telemetry */
status_t enterprise_setup_monitoring(void);
status_t enterprise_configure_telemetry(void);
status_t enterprise_generate_installation_report(const char* output_file);

/* Rollback and Recovery */
status_t enterprise_create_system_snapshot(const char* snapshot_name);
status_t enterprise_restore_system_snapshot(const char* snapshot_name);
status_t enterprise_setup_recovery_environment(void);

/* Utility Functions */
status_t enterprise_detect_virtualization_platform(char* platform, size_t platform_size);
status_t enterprise_optimize_for_virtualization(void);
status_t enterprise_configure_for_cloud_deployment(void);

/* Remote Management */
status_t enterprise_setup_remote_management(void);
status_t enterprise_configure_ssh_access(void);
status_t enterprise_setup_certificate_based_auth(void);

/* Quality Assurance */
status_t enterprise_run_installation_tests(void);
status_t enterprise_verify_system_integrity(void);
status_t enterprise_benchmark_system_performance(void);

/* Documentation and Reporting */
status_t enterprise_generate_installation_documentation(const char* output_dir);
status_t enterprise_create_system_inventory(const char* output_file);
status_t enterprise_generate_security_audit_report(const char* output_file);

/* Integration Functions */
status_t enterprise_integrate_with_existing_infrastructure(void);
status_t enterprise_setup_backup_integration(void);
status_t enterprise_configure_patch_management(void);

#endif /* ENTERPRISE_DEPLOYMENT_H */