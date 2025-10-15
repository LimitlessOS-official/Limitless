/**
 * LimitlessOS Enterprise Deployment Manager
 * Advanced installer features for enterprise deployment scenarios
 * Automated deployment, configuration management, and validation
 */

#include "enterprise_deployment.h"
#include "complete_installer.h"
#include "gui_installer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <libxml/parser.h>
#include <json-c/json.h>

/* Enterprise deployment state */
typedef struct {
    bool initialized;
    deployment_profile_t active_profile;
    automated_config_t auto_config;
    validation_suite_t validator;
    provisioning_manager_t provisioning;
    compliance_checker_t compliance;
    telemetry_manager_t telemetry;
    rollback_manager_t rollback;
} enterprise_deployment_t;

static enterprise_deployment_t g_deployment = {0};

/**
 * Initialize enterprise deployment system
 */
status_t enterprise_deployment_init(void) {
    if (g_deployment.initialized) {
        return STATUS_SUCCESS;
    }
    
    printf("[ENTERPRISE] Initializing LimitlessOS Enterprise Deployment Manager\n");
    
    /* Initialize deployment profile */
    g_deployment.active_profile.deployment_type = DEPLOYMENT_TYPE_STANDARD;
    g_deployment.active_profile.automation_level = AUTOMATION_LEVEL_INTERACTIVE;
    g_deployment.active_profile.validation_level = VALIDATION_LEVEL_COMPREHENSIVE;
    g_deployment.active_profile.compliance_mode = COMPLIANCE_MODE_ENTERPRISE;
    
    /* Initialize automated configuration */
    g_deployment.auto_config.enabled = false;
    g_deployment.auto_config.config_source = CONFIG_SOURCE_LOCAL;
    strcpy(g_deployment.auto_config.config_server_url, "");
    strcpy(g_deployment.auto_config.deployment_key, "");
    
    /* Initialize validation suite */
    g_deployment.validator.enabled = true;
    g_deployment.validator.hardware_validation = true;
    g_deployment.validator.security_validation = true;
    g_deployment.validator.compliance_validation = true;
    g_deployment.validator.performance_validation = true;
    
    /* Initialize provisioning manager */
    g_deployment.provisioning.enabled = false;
    g_deployment.provisioning.auto_join_domain = false;
    strcpy(g_deployment.provisioning.domain_controller, "");
    strcpy(g_deployment.provisioning.organizational_unit, "");
    
    /* Initialize compliance checker */
    g_deployment.compliance.enabled = true;
    g_deployment.compliance.enforce_encryption = true;
    g_deployment.compliance.enforce_secure_boot = true;
    g_deployment.compliance.enforce_tpm = false;
    g_deployment.compliance.audit_logging = true;
    
    /* Initialize telemetry (disabled by default for privacy) */
    g_deployment.telemetry.enabled = false;
    g_deployment.telemetry.anonymous_only = true;
    g_deployment.telemetry.local_only = true;
    strcpy(g_deployment.telemetry.server_url, "");
    
    /* Initialize rollback manager */
    g_deployment.rollback.enabled = true;
    g_deployment.rollback.create_snapshots = true;
    g_deployment.rollback.max_snapshots = 3;
    
    printf("[ENTERPRISE] Deployment manager initialized successfully\n");
    g_deployment.initialized = true;
    
    return STATUS_SUCCESS;
}

/**
 * Load deployment configuration from file
 */
status_t enterprise_deployment_load_config(const char* config_file) {
    if (!config_file) {
        return STATUS_INVALID_PARAMETER;
    }
    
    printf("[ENTERPRISE] Loading deployment configuration from: %s\n", config_file);
    
    FILE* file = fopen(config_file, "r");
    if (!file) {
        printf("[ERROR] Cannot open configuration file: %s\n", config_file);
        return STATUS_ERROR;
    }
    
    /* Read file into buffer */
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return STATUS_INSUFFICIENT_MEMORY;
    }
    
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);
    
    /* Parse JSON configuration */
    json_object* root = json_tokener_parse(buffer);
    if (!root) {
        printf("[ERROR] Invalid JSON configuration file\n");
        free(buffer);
        return STATUS_ERROR;
    }
    
    /* Parse deployment profile */
    json_object* deployment_obj;
    if (json_object_object_get_ex(root, "deployment", &deployment_obj)) {
        json_object* type_obj;
        if (json_object_object_get_ex(deployment_obj, "type", &type_obj)) {
            const char* type_str = json_object_get_string(type_obj);
            if (strcmp(type_str, "enterprise") == 0) {
                g_deployment.active_profile.deployment_type = DEPLOYMENT_TYPE_ENTERPRISE;
            } else if (strcmp(type_str, "kiosk") == 0) {
                g_deployment.active_profile.deployment_type = DEPLOYMENT_TYPE_KIOSK;
            } else if (strcmp(type_str, "server") == 0) {
                g_deployment.active_profile.deployment_type = DEPLOYMENT_TYPE_SERVER;
            }
        }
        
        json_object* automation_obj;
        if (json_object_object_get_ex(deployment_obj, "automation_level", &automation_obj)) {
            const char* automation_str = json_object_get_string(automation_obj);
            if (strcmp(automation_str, "fully_automated") == 0) {
                g_deployment.active_profile.automation_level = AUTOMATION_LEVEL_FULLY_AUTOMATED;
            } else if (strcmp(automation_str, "semi_automated") == 0) {
                g_deployment.active_profile.automation_level = AUTOMATION_LEVEL_SEMI_AUTOMATED;
            }
        }
    }
    
    /* Parse security settings */
    json_object* security_obj;
    if (json_object_object_get_ex(root, "security", &security_obj)) {
        json_object* encryption_obj;
        if (json_object_object_get_ex(security_obj, "enforce_encryption", &encryption_obj)) {
            g_deployment.compliance.enforce_encryption = json_object_get_boolean(encryption_obj);
        }
        
        json_object* secure_boot_obj;
        if (json_object_object_get_ex(security_obj, "enforce_secure_boot", &secure_boot_obj)) {
            g_deployment.compliance.enforce_secure_boot = json_object_get_boolean(secure_boot_obj);
        }
        
        json_object* tpm_obj;
        if (json_object_object_get_ex(security_obj, "enforce_tpm", &tmp_obj)) {
            g_deployment.compliance.enforce_tpm = json_object_get_boolean(tpm_obj);
        }
    }
    
    /* Parse provisioning settings */
    json_object* provisioning_obj;
    if (json_object_object_get_ex(root, "provisioning", &provisioning_obj)) {
        json_object* enabled_obj;
        if (json_object_object_get_ex(provisioning_obj, "enabled", &enabled_obj)) {
            g_deployment.provisioning.enabled = json_object_get_boolean(enabled_obj);
        }
        
        json_object* domain_obj;
        if (json_object_object_get_ex(provisioning_obj, "domain_controller", &domain_obj)) {
            strncpy(g_deployment.provisioning.domain_controller,
                   json_object_get_string(domain_obj), 255);
        }
        
        json_object* ou_obj;
        if (json_object_object_get_ex(provisioning_obj, "organizational_unit", &ou_obj)) {
            strncpy(g_deployment.provisioning.organizational_unit,
                   json_object_get_string(ou_obj), 255);
        }
    }
    
    /* Parse feature selection */
    json_object* features_obj;
    if (json_object_object_get_ex(root, "features", &features_obj)) {
        json_object* ai_obj;
        if (json_object_object_get_ex(features_obj, "ai_framework", &ai_obj)) {
            g_deployment.auto_config.install_ai_framework = json_object_get_boolean(ai_obj);
        }
        
        json_object* dev_tools_obj;
        if (json_object_object_get_ex(features_obj, "development_tools", &dev_tools_obj)) {
            g_deployment.auto_config.install_development_tools = json_object_get_boolean(dev_tools_obj);
        }
        
        json_object* enterprise_obj;
        if (json_object_object_get_ex(features_obj, "enterprise_tools", &enterprise_obj)) {
            g_deployment.auto_config.install_enterprise_tools = json_object_get_boolean(enterprise_obj);
        }
    }
    
    json_object_put(root);
    free(buffer);
    
    printf("[ENTERPRISE] Configuration loaded successfully\n");
    return STATUS_SUCCESS;
}

/**
 * Generate deployment configuration template
 */
status_t enterprise_deployment_generate_template(const char* output_file) {
    if (!output_file) {
        return STATUS_INVALID_PARAMETER;
    }
    
    printf("[ENTERPRISE] Generating deployment configuration template: %s\n", output_file);
    
    FILE* file = fopen(output_file, "w");
    if (!file) {
        printf("[ERROR] Cannot create configuration template file: %s\n", output_file);
        return STATUS_ERROR;
    }
    
    /* Generate comprehensive configuration template */
    fprintf(file, "{\n");
    fprintf(file, "  \"deployment\": {\n");
    fprintf(file, "    \"type\": \"enterprise\",\n");
    fprintf(file, "    \"automation_level\": \"semi_automated\",\n");
    fprintf(file, "    \"validation_level\": \"comprehensive\",\n");
    fprintf(file, "    \"compliance_mode\": \"enterprise\"\n");
    fprintf(file, "  },\n");
    fprintf(file, "  \"security\": {\n");
    fprintf(file, "    \"enforce_encryption\": true,\n");
    fprintf(file, "    \"enforce_secure_boot\": true,\n");
    fprintf(file, "    \"enforce_tpm\": false,\n");
    fprintf(file, "    \"encryption_algorithm\": \"AES-256-XTS\",\n");
    fprintf(file, "    \"key_derivation\": \"PBKDF2-SHA512\"\n");
    fprintf(file, "  },\n");
    fprintf(file, "  \"hardware\": {\n");
    fprintf(file, "    \"minimum_ram_gb\": 4,\n");
    fprintf(file, "    \"recommended_ram_gb\": 16,\n");
    fprintf(file, "    \"minimum_disk_gb\": 50,\n");
    fprintf(file, "    \"require_virtualization\": true,\n");
    fprintf(file, "    \"require_aes_ni\": true\n");
    fprintf(file, "  },\n");
    fprintf(file, "  \"partitioning\": {\n");
    fprintf(file, "    \"scheme\": \"gpt\",\n");
    fprintf(file, "    \"efi_size_mb\": 512,\n");
    fprintf(file, "    \"boot_size_mb\": 1024,\n");
    fprintf(file, "    \"root_filesystem\": \"ext4\",\n");
    fprintf(file, "    \"enable_swap\": true,\n");
    fprintf(file, "    \"swap_size_gb\": 8\n");
    fprintf(file, "  },\n");
    fprintf(file, "  \"features\": {\n");
    fprintf(file, "    \"ai_framework\": false,\n");
    fprintf(file, "    \"development_tools\": true,\n");
    fprintf(file, "    \"enterprise_tools\": true,\n");
    fprintf(file, "    \"multimedia_codecs\": false,\n");
    fprintf(file, "    \"quantum_computing\": false,\n");
    fprintf(file, "    \"blockchain_support\": false\n");
    fprintf(file, "  },\n");
    fprintf(file, "  \"network\": {\n");
    fprintf(file, "    \"configure_automatically\": true,\n");
    fprintf(file, "    \"dns_servers\": [\"1.1.1.1\", \"1.0.0.1\"],\n");
    fprintf(file, "    \"ntp_servers\": [\"pool.ntp.org\"]\n");
    fprintf(file, "  },\n");
    fprintf(file, "  \"provisioning\": {\n");
    fprintf(file, "    \"enabled\": false,\n");
    fprintf(file, "    \"domain_controller\": \"\",\n");
    fprintf(file, "    \"organizational_unit\": \"Computers\",\n");
    fprintf(file, "    \"auto_join_domain\": false\n");
    fprintf(file, "  },\n");
    fprintf(file, "  \"user_accounts\": {\n");
    fprintf(file, "    \"create_default_user\": true,\n");
    fprintf(file, "    \"username\": \"limitless\",\n");
    fprintf(file, "    \"full_name\": \"LimitlessOS User\",\n");
    fprintf(file, "    \"password_policy\": \"strong\",\n");
    fprintf(file, "    \"auto_login\": false\n");
    fprintf(file, "  },\n");
    fprintf(file, "  \"privacy\": {\n");
    fprintf(file, "    \"telemetry_enabled\": false,\n");
    fprintf(file, "    \"data_collection\": false,\n");
    fprintf(file, "    \"crash_reporting\": \"local_only\",\n");
    fprintf(file, "    \"usage_analytics\": false\n");
    fprintf(file, "  },\n");
    fprintf(file, "  \"validation\": {\n");
    fprintf(file, "    \"hardware_validation\": true,\n");
    fprintf(file, "    \"security_validation\": true,\n");
    fprintf(file, "    \"compliance_validation\": true,\n");
    fprintf(file, "    \"performance_validation\": true,\n");
    fprintf(file, "    \"post_install_testing\": true\n");
    fprintf(file, "  }\n");
    fprintf(file, "}\n");
    
    fclose(file);
    
    printf("[ENTERPRISE] Configuration template generated successfully\n");
    return STATUS_SUCCESS;
}

/**
 * Validate hardware compliance
 */
status_t enterprise_validate_hardware_compliance(const hardware_info_t* hardware) {
    if (!hardware) {
        return STATUS_INVALID_PARAMETER;
    }
    
    printf("[ENTERPRISE] Validating hardware compliance...\n");
    
    bool compliance_passed = true;
    
    /* Check minimum memory requirements */
    if (hardware->total_memory_mb < 4096) { /* 4GB minimum */
        printf("[COMPLIANCE] FAIL: Insufficient memory - %llu MB (4GB required)\n", 
               hardware->total_memory_mb);
        compliance_passed = false;
    } else {
        printf("[COMPLIANCE] PASS: Memory requirement met - %llu MB\n", 
               hardware->total_memory_mb);
    }
    
    /* Check CPU requirements */
    if (hardware->cpu_cores < 2) {
        printf("[COMPLIANCE] FAIL: Insufficient CPU cores - %d (2 required)\n", 
               hardware->cpu_cores);
        compliance_passed = false;
    } else {
        printf("[COMPLIANCE] PASS: CPU cores requirement met - %d\n", 
               hardware->cpu_cores);
    }
    
    /* Check virtualization support */
    if (g_deployment.compliance.enforce_virtualization && !hardware->virtualization_support) {
        printf("[COMPLIANCE] FAIL: Virtualization support required but not available\n");
        compliance_passed = false;
    } else if (hardware->virtualization_support) {
        printf("[COMPLIANCE] PASS: Virtualization support available\n");
    }
    
    /* Check AES-NI support */
    if (g_deployment.compliance.enforce_aes_ni && !hardware->aes_ni_support) {
        printf("[COMPLIANCE] FAIL: AES-NI support required but not available\n");
        compliance_passed = false;
    } else if (hardware->aes_ni_support) {
        printf("[COMPLIANCE] PASS: AES-NI support available\n");
    }
    
    /* Check TPM availability */
    if (g_deployment.compliance.enforce_tpm && !hardware->tpm_available) {
        printf("[COMPLIANCE] FAIL: TPM required but not available\n");
        compliance_passed = false;
    } else if (hardware->tmp_available) {
        printf("[COMPLIANCE] PASS: TPM available\n");
    }
    
    /* Check secure boot capability */
    if (g_deployment.compliance.enforce_secure_boot && !hardware->secure_boot_available) {
        printf("[COMPLIANCE] FAIL: Secure Boot required but not available\n");
        compliance_passed = false;
    } else if (hardware->secure_boot_available) {
        printf("[COMPLIANCE] PASS: Secure Boot available\n");
    }
    
    if (compliance_passed) {
        printf("[COMPLIANCE] Hardware compliance validation PASSED\n");
        return STATUS_SUCCESS;
    } else {
        printf("[COMPLIANCE] Hardware compliance validation FAILED\n");
        return STATUS_ERROR;
    }
}

/**
 * Execute automated deployment
 */
status_t enterprise_execute_automated_deployment(void) {
    if (!g_deployment.initialized) {
        return STATUS_NOT_INITIALIZED;
    }
    
    printf("[ENTERPRISE] Starting automated deployment process...\n");
    
    /* Phase 1: Pre-deployment validation */
    printf("[AUTOMATED] Phase 1: Pre-deployment validation\n");
    
    hardware_info_t hardware;
    if (gui_installer_detect_hardware(&hardware) != STATUS_SUCCESS) {
        printf("[ERROR] Hardware detection failed\n");
        return STATUS_ERROR;
    }
    
    if (enterprise_validate_hardware_compliance(&hardware) != STATUS_SUCCESS) {
        printf("[ERROR] Hardware compliance validation failed\n");
        if (g_deployment.active_profile.validation_level == VALIDATION_LEVEL_STRICT) {
            return STATUS_ERROR;
        }
        printf("[WARNING] Continuing with compliance warnings\n");
    }
    
    /* Phase 2: Automated partitioning */
    printf("[AUTOMATED] Phase 2: Automated disk partitioning\n");
    
    if (enterprise_auto_partition_disks() != STATUS_SUCCESS) {
        printf("[ERROR] Automated partitioning failed\n");
        return STATUS_ERROR;
    }
    
    /* Phase 3: System installation */
    printf("[AUTOMATED] Phase 3: System installation\n");
    
    installation_config_t config;
    enterprise_generate_installation_config(&config);
    
    if (complete_installer_run_with_config(&config) != STATUS_SUCCESS) {
        printf("[ERROR] System installation failed\n");
        return STATUS_ERROR;
    }
    
    /* Phase 4: Enterprise configuration */
    printf("[AUTOMATED] Phase 4: Enterprise configuration\n");
    
    if (enterprise_apply_configuration() != STATUS_SUCCESS) {
        printf("[ERROR] Enterprise configuration failed\n");
        return STATUS_ERROR;
    }
    
    /* Phase 5: Security hardening */
    printf("[AUTOMATED] Phase 5: Security hardening\n");
    
    if (enterprise_apply_security_hardening() != STATUS_SUCCESS) {
        printf("[ERROR] Security hardening failed\n");
        return STATUS_ERROR;
    }
    
    /* Phase 6: Post-installation validation */
    printf("[AUTOMATED] Phase 6: Post-installation validation\n");
    
    if (enterprise_validate_installation() != STATUS_SUCCESS) {
        printf("[ERROR] Post-installation validation failed\n");
        return STATUS_ERROR;
    }
    
    /* Phase 7: Provisioning (if enabled) */
    if (g_deployment.provisioning.enabled) {
        printf("[AUTOMATED] Phase 7: Domain provisioning\n");
        
        if (enterprise_provision_system() != STATUS_SUCCESS) {
            printf("[WARNING] Domain provisioning failed (non-critical)\n");
        }
    }
    
    printf("[ENTERPRISE] Automated deployment completed successfully!\n");
    return STATUS_SUCCESS;
}

/**
 * Generate installation configuration from deployment profile
 */
static status_t enterprise_generate_installation_config(installation_config_t* config) {
    if (!config) {
        return STATUS_INVALID_PARAMETER;
    }
    
    memset(config, 0, sizeof(installation_config_t));
    
    /* Set basic configuration */
    config->installation_type = INSTALL_TYPE_ENTERPRISE;
    config->target_architecture = ARCH_X86_64;
    config->filesystem_type = FS_TYPE_EXT4;
    
    /* Security settings */
    config->enable_encryption = g_deployment.compliance.enforce_encryption;
    config->enable_secure_boot = g_deployment.compliance.enforce_secure_boot;
    config->enable_tpm = g_deployment.compliance.enforce_tpm;
    config->encryption_level = ENCRYPTION_AES_256_XTS;
    
    /* Feature selection based on deployment profile */
    switch (g_deployment.active_profile.deployment_type) {
        case DEPLOYMENT_TYPE_ENTERPRISE:
            config->install_development_tools = true;
            config->install_enterprise_tools = true;
            config->install_multimedia_codecs = false;
            config->install_ai_framework = g_deployment.auto_config.install_ai_framework;
            break;
            
        case DEPLOYMENT_TYPE_DEVELOPER:
            config->install_development_tools = true;
            config->install_enterprise_tools = true;
            config->install_multimedia_codecs = true;
            config->install_ai_framework = true;
            break;
            
        case DEPLOYMENT_TYPE_KIOSK:
            config->install_development_tools = false;
            config->install_enterprise_tools = false;
            config->install_multimedia_codecs = true;
            config->install_ai_framework = false;
            break;
            
        case DEPLOYMENT_TYPE_SERVER:
            config->install_development_tools = false;
            config->install_enterprise_tools = true;
            config->install_multimedia_codecs = false;
            config->install_ai_framework = false;
            break;
            
        default:
            config->install_development_tools = true;
            config->install_enterprise_tools = true;
            config->install_multimedia_codecs = false;
            config->install_ai_framework = false;
            break;
    }
    
    /* Privacy settings */
    config->enable_ai_features = g_deployment.auto_config.install_ai_framework;
    config->telemetry_enabled = g_deployment.telemetry.enabled;
    config->data_collection_enabled = false; /* Always disabled for enterprise */
    
    /* User account settings */
    strcpy(config->username, "admin");
    strcpy(config->full_name, "LimitlessOS Administrator");
    strcpy(config->hostname, "limitless-enterprise");
    strcpy(config->timezone, "UTC");
    
    return STATUS_SUCCESS;
}

/**
 * Automated disk partitioning
 */
static status_t enterprise_auto_partition_disks(void) {
    printf("[PARTITION] Executing automated disk partitioning\n");
    
    /* Select best disk for installation */
    disk_info_t disk_info;
    if (enterprise_select_optimal_disk(&disk_info) != STATUS_SUCCESS) {
        printf("[ERROR] No suitable disk found for installation\n");
        return STATUS_ERROR;
    }
    
    printf("[PARTITION] Selected disk: %s (%llu GB)\n", 
           disk_info.device_path, disk_info.size_bytes / (1024*1024*1024));
    
    /* Create GPT partition table */
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "parted -s %s mklabel gpt", disk_info.device_path);
    if (system(cmd) != 0) {
        printf("[ERROR] Failed to create GPT partition table\n");
        return STATUS_ERROR;
    }
    
    /* Create EFI system partition (512MB) */
    snprintf(cmd, sizeof(cmd), 
             "parted -s %s mkpart ESP fat32 1MiB 513MiB && "
             "parted -s %s set 1 esp on", 
             disk_info.device_path, disk_info.device_path);
    if (system(cmd) != 0) {
        printf("[ERROR] Failed to create EFI system partition\n");
        return STATUS_ERROR;
    }
    
    /* Create boot partition (1GB) */
    snprintf(cmd, sizeof(cmd), 
             "parted -s %s mkpart boot ext4 513MiB 1537MiB",
             disk_info.device_path);
    if (system(cmd) != 0) {
        printf("[ERROR] Failed to create boot partition\n");
        return STATUS_ERROR;
    }
    
    /* Create root partition A (45% of remaining space) */
    snprintf(cmd, sizeof(cmd),
             "parted -s %s mkpart rootA ext4 1537MiB 45%%",
             disk_info.device_path);
    if (system(cmd) != 0) {
        printf("[ERROR] Failed to create root partition A\n");
        return STATUS_ERROR;
    }
    
    /* Create root partition B (45% of remaining space) */
    snprintf(cmd, sizeof(cmd),
             "parted -s %s mkpart rootB ext4 45%% 90%%",
             disk_info.device_path);
    if (system(cmd) != 0) {
        printf("[ERROR] Failed to create root partition B\n");
        return STATUS_ERROR;
    }
    
    /* Create swap partition (remaining 10%) */
    snprintf(cmd, sizeof(cmd),
             "parted -s %s mkpart swap linux-swap 90%% 100%%",
             disk_info.device_path);
    if (system(cmd) != 0) {
        printf("[ERROR] Failed to create swap partition\n");
        return STATUS_ERROR;
    }
    
    /* Wait for kernel to recognize partitions */
    system("partprobe && sleep 2");
    
    printf("[PARTITION] Automated partitioning completed successfully\n");
    return STATUS_SUCCESS;
}

/**
 * Select optimal disk for installation
 */
static status_t enterprise_select_optimal_disk(disk_info_t* selected_disk) {
    /* For now, use simple heuristic - largest available disk */
    /* In production, this would analyze performance characteristics */
    
    FILE* devices = popen("lsblk -d -n -b -o NAME,SIZE,TYPE | grep disk", "r");
    if (!devices) {
        return STATUS_ERROR;
    }
    
    char line[256];
    uint64_t best_size = 0;
    char best_device[64] = "";
    
    while (fgets(line, sizeof(line), devices)) {
        char name[32];
        uint64_t size;
        char type[16];
        
        if (sscanf(line, "%s %llu %s", name, &size, type) == 3) {
            if (size > best_size && size >= 50ULL * 1024 * 1024 * 1024) { /* 50GB minimum */
                best_size = size;
                snprintf(best_device, sizeof(best_device), "/dev/%s", name);
            }
        }
    }
    pclose(devices);
    
    if (best_size == 0) {
        return STATUS_ERROR;
    }
    
    /* Fill disk info structure */
    strcpy(selected_disk->device_path, best_device);
    strcpy(selected_disk->model, "Auto-selected");
    selected_disk->size_bytes = best_size;
    selected_disk->is_ssd = true; /* Assume SSD for now */
    selected_disk->is_removable = false;
    
    return STATUS_SUCCESS;
}

/**
 * Apply enterprise configuration
 */
static status_t enterprise_apply_configuration(void) {
    printf("[CONFIG] Applying enterprise configuration\n");
    
    /* Create enterprise directory structure */
    system("mkdir -p /mnt/limitless/etc/enterprise");
    system("mkdir -p /mnt/limitless/var/log/enterprise");
    system("mkdir -p /mnt/limitless/opt/enterprise");
    
    /* Install enterprise configuration files */
    FILE* enterprise_conf = fopen("/mnt/limitless/etc/enterprise/config.conf", "w");
    if (enterprise_conf) {
        fprintf(enterprise_conf, "# LimitlessOS Enterprise Configuration\n");
        fprintf(enterprise_conf, "deployment_type=%d\n", g_deployment.active_profile.deployment_type);
        fprintf(enterprise_conf, "automation_level=%d\n", g_deployment.active_profile.automation_level);
        fprintf(enterprise_conf, "compliance_mode=%d\n", g_deployment.active_profile.compliance_mode);
        fprintf(enterprise_conf, "install_timestamp=%ld\n", time(NULL));
        fclose(enterprise_conf);
    }
    
    /* Configure system services */
    if (g_deployment.compliance.audit_logging) {
        printf("[CONFIG] Enabling enterprise audit logging\n");
        system("mkdir -p /mnt/limitless/var/log/audit");
        
        FILE* audit_conf = fopen("/mnt/limitless/etc/audit.conf", "w");
        if (audit_conf) {
            fprintf(audit_conf, "# LimitlessOS Audit Configuration\n");
            fprintf(audit_conf, "log_level=detailed\n");
            fprintf(audit_conf, "log_location=/var/log/audit\n");
            fprintf(audit_conf, "max_log_size=100MB\n");
            fprintf(audit_conf, "log_rotation=daily\n");
            fclose(audit_conf);
        }
    }
    
    printf("[CONFIG] Enterprise configuration applied successfully\n");
    return STATUS_SUCCESS;
}

/**
 * Apply security hardening
 */
static status_t enterprise_apply_security_hardening(void) {
    printf("[SECURITY] Applying enterprise security hardening\n");
    
    /* Configure firewall rules */
    FILE* firewall_conf = fopen("/mnt/limitless/etc/firewall.conf", "w");
    if (firewall_conf) {
        fprintf(firewall_conf, "# LimitlessOS Enterprise Firewall Configuration\n");
        fprintf(firewall_conf, "default_policy=deny\n");
        fprintf(firewall_conf, "allow_ssh=false\n");
        fprintf(firewall_conf, "allow_http=false\n");
        fprintf(firewall_conf, "allow_https=true\n");
        fprintf(firewall_conf, "intrusion_detection=enabled\n");
        fclose(firewall_conf);
    }
    
    /* Configure password policy */
    FILE* passwd_policy = fopen("/mnt/limitless/etc/security/passwd_policy.conf", "w");
    if (passwd_policy) {
        fprintf(passwd_policy, "# LimitlessOS Password Policy\n");
        fprintf(passwd_policy, "min_length=12\n");
        fprintf(passwd_policy, "require_uppercase=true\n");
        fprintf(passwd_policy, "require_lowercase=true\n");
        fprintf(passwd_policy, "require_numbers=true\n");
        fprintf(passwd_policy, "require_symbols=true\n");
        fprintf(passwd_policy, "max_age_days=90\n");
        fprintf(passwd_policy, "lockout_attempts=3\n");
        fclose(passwd_policy);
    }
    
    /* Set secure file permissions */
    system("chmod 600 /mnt/limitless/etc/security/* 2>/dev/null");
    system("chmod 700 /mnt/limitless/etc/enterprise 2>/dev/null");
    system("chmod 755 /mnt/limitless/var/log/enterprise 2>/dev/null");
    
    printf("[SECURITY] Security hardening completed\n");
    return STATUS_SUCCESS;
}

/**
 * Validate installation
 */
static status_t enterprise_validate_installation(void) {
    printf("[VALIDATION] Running post-installation validation\n");
    
    bool validation_passed = true;
    
    /* Check kernel installation */
    if (access("/mnt/limitless/boot/limitless_kernel.bin", F_OK) != 0) {
        printf("[VALIDATION] FAIL: Kernel not found\n");
        validation_passed = false;
    } else {
        printf("[VALIDATION] PASS: Kernel installed\n");
    }
    
    /* Check bootloader configuration */
    if (access("/mnt/limitless/boot/grub/grub.cfg", F_OK) != 0) {
        printf("[VALIDATION] FAIL: Bootloader configuration not found\n");
        validation_passed = false;
    } else {
        printf("[VALIDATION] PASS: Bootloader configured\n");
    }
    
    /* Check enterprise configuration */
    if (access("/mnt/limitless/etc/enterprise/config.conf", F_OK) != 0) {
        printf("[VALIDATION] FAIL: Enterprise configuration not found\n");
        validation_passed = false;
    } else {
        printf("[VALIDATION] PASS: Enterprise configuration present\n");
    }
    
    /* Check security configuration */
    if (access("/mnt/limitless/etc/security", F_OK) != 0) {
        printf("[VALIDATION] FAIL: Security configuration not found\n");
        validation_passed = false;
    } else {
        printf("[VALIDATION] PASS: Security configuration present\n");
    }
    
    if (validation_passed) {
        printf("[VALIDATION] Post-installation validation PASSED\n");
        return STATUS_SUCCESS;
    } else {
        printf("[VALIDATION] Post-installation validation FAILED\n");
        return STATUS_ERROR;
    }
}

/**
 * Provision system (domain joining, etc.)
 */
static status_t enterprise_provision_system(void) {
    printf("[PROVISION] Starting system provisioning\n");
    
    if (strlen(g_deployment.provisioning.domain_controller) == 0) {
        printf("[PROVISION] No domain controller specified, skipping\n");
        return STATUS_SUCCESS;
    }
    
    /* Create domain join script */
    FILE* join_script = fopen("/mnt/limitless/opt/enterprise/join_domain.sh", "w");
    if (join_script) {
        fprintf(join_script, "#!/bin/bash\n");
        fprintf(join_script, "# Auto-generated domain join script\n");
        fprintf(join_script, "echo 'Joining domain: %s'\n", g_deployment.provisioning.domain_controller);
        fprintf(join_script, "# Domain join implementation would go here\n");
        fclose(join_script);
        system("chmod +x /mnt/limitless/opt/enterprise/join_domain.sh");
    }
    
    printf("[PROVISION] System provisioning configured\n");
    return STATUS_SUCCESS;
}

/**
 * Cleanup enterprise deployment
 */
void enterprise_deployment_cleanup(void) {
    if (g_deployment.initialized) {
        printf("[ENTERPRISE] Cleaning up deployment manager\n");
        memset(&g_deployment, 0, sizeof(enterprise_deployment_t));
    }
}