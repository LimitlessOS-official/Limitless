#include <stddef.h>
#include "installer_status.h"
/**
 * LimitlessOS Intelligent Installer System
 * AI-powered installer with hardware detection, automatic partitioning,
 * driver selection, and comprehensive privacy controls
 */

 #ifndef LIMITLESS_INTELLIGENT_INSTALLER_H
 #define LIMITLESS_INTELLIGENT_INSTALLER_H

#include <stdint.h>
#include <stdbool.h>

/* Installer Version */
#define INSTALLER_VERSION_MAJOR 2
#define INSTALLER_VERSION_MINOR 0

/* Maximum limits */
#define MAX_STORAGE_DEVICES     32
#define MAX_PARTITIONS         128
#define MAX_DRIVERS            256
#define MAX_HARDWARE_DEVICES   512
#define MAX_INSTALLATION_STEPS 64
#define MAX_USER_PROFILES      16
#define MAX_LANGUAGE_PACKS     64

/* Installation Types */
typedef enum {
    INSTALL_TYPE_FULL,              /* Complete installation */
    INSTALL_TYPE_MINIMAL,           /* Minimal installation */
    INSTALL_TYPE_CUSTOM,            /* Custom installation */
    INSTALL_TYPE_ENTERPRISE,        /* Enterprise installation */
    INSTALL_TYPE_DEVELOPER,         /* Developer workstation */
    INSTALL_TYPE_SERVER,            /* Server installation */
    INSTALL_TYPE_EMBEDDED,          /* Embedded system */
    INSTALL_TYPE_CLOUD,             /* Cloud instance */
    INSTALL_TYPE_VIRTUAL_MACHINE,   /* Virtual machine */
    INSTALL_TYPE_UPGRADE,           /* System upgrade */
    INSTALL_TYPE_DUAL_BOOT,         /* Dual boot setup */
    INSTALL_TYPE_RECOVERY           /* Recovery installation */
} installation_type_t;

/* Hardware Detection Categories */
typedef enum {
    HARDWARE_CPU,
    HARDWARE_MEMORY,
    HARDWARE_STORAGE,
    HARDWARE_GRAPHICS,
    HARDWARE_AUDIO,
    HARDWARE_NETWORK,
    HARDWARE_USB,
    HARDWARE_BLUETOOTH,
    HARDWARE_WIFI,
    HARDWARE_SENSORS,
    HARDWARE_POWER,
    HARDWARE_THERMAL,
    HARDWARE_SECURITY,
    HARDWARE_AI_ACCELERATOR,
    HARDWARE_QUANTUM_PROCESSOR,
    HARDWARE_BIOMETRIC,
    HARDWARE_DISPLAY,
    HARDWARE_INPUT_DEVICE,
    HARDWARE_PRINTER,
    HARDWARE_CAMERA,
    HARDWARE_MICROPHONE,
    HARDWARE_SPEAKER
} hardware_category_t;

/* Storage Device Types */
typedef enum {
    STORAGE_TYPE_HDD,               /* Hard Disk Drive */
    STORAGE_TYPE_SSD,               /* Solid State Drive */
    STORAGE_TYPE_NVME,              /* NVMe SSD */
    STORAGE_TYPE_EMMC,              /* eMMC storage */
    STORAGE_TYPE_SD_CARD,           /* SD Card */
    STORAGE_TYPE_USB,               /* USB storage */
    STORAGE_TYPE_OPTICAL,           /* Optical drive */
    STORAGE_TYPE_FLOPPY,            /* Floppy disk */
    STORAGE_TYPE_NETWORK,           /* Network storage */
    STORAGE_TYPE_CLOUD,             /* Cloud storage */
    STORAGE_TYPE_QUANTUM            /* Quantum storage */
} storage_device_type_t;

/* Filesystem Types */
typedef enum {
    FS_TYPE_EXT4,
    FS_TYPE_BTRFS,
    FS_TYPE_ZFS,
    FS_TYPE_XFS,
    FS_TYPE_F2FS,
    FS_TYPE_NTFS,
    FS_TYPE_FAT32,
    FS_TYPE_EXFAT,
    FS_TYPE_APFS,
    FS_TYPE_HFS_PLUS,
    FS_TYPE_LIMITLESS_FS,           /* LimitlessOS native filesystem */
    FS_TYPE_QUANTUM_FS              /* Quantum-encrypted filesystem */
} filesystem_type_t;

/* Partition Types */
typedef enum {
    PARTITION_TYPE_ROOT,            /* Root partition */
    PARTITION_TYPE_BOOT,            /* Boot partition */
    PARTITION_TYPE_EFI,             /* EFI System Partition */
    PARTITION_TYPE_SWAP,            /* Swap partition */
    PARTITION_TYPE_HOME,            /* Home partition */
    PARTITION_TYPE_VAR,             /* Variable data partition */
    PARTITION_TYPE_TMP,             /* Temporary files partition */
    PARTITION_TYPE_OPT,             /* Optional software partition */
    PARTITION_TYPE_USR,             /* User programs partition */
    PARTITION_TYPE_DATA,            /* Data partition */
    PARTITION_TYPE_BACKUP,          /* Backup partition */
    PARTITION_TYPE_RECOVERY,        /* Recovery partition */
    PARTITION_TYPE_ENTERPRISE,      /* Enterprise data partition */
    PARTITION_TYPE_AI_MODELS,       /* AI models partition */
    PARTITION_TYPE_QUANTUM_DATA,    /* Quantum data partition */
    PARTITION_TYPE_SECURE_VAULT     /* Secure encrypted vault */
} partition_type_t;

/* Driver Categories */
typedef enum {
    DRIVER_CATEGORY_GRAPHICS,
    DRIVER_CATEGORY_AUDIO,
    DRIVER_CATEGORY_NETWORK,
    DRIVER_CATEGORY_STORAGE,
    DRIVER_CATEGORY_INPUT,
    DRIVER_CATEGORY_USB,
    DRIVER_CATEGORY_BLUETOOTH,
    DRIVER_CATEGORY_WIFI,
    DRIVER_CATEGORY_CELLULAR,
    DRIVER_CATEGORY_SENSORS,
    DRIVER_CATEGORY_POWER,
    DRIVER_CATEGORY_THERMAL,
    DRIVER_CATEGORY_SECURITY,
    DRIVER_CATEGORY_BIOMETRIC,
    DRIVER_CATEGORY_AI_ACCELERATOR,
    DRIVER_CATEGORY_QUANTUM,
    DRIVER_CATEGORY_VIRTUALIZATION,
    DRIVER_CATEGORY_FIRMWARE
} driver_category_t;

/* Privacy Settings */
typedef struct installer_privacy_settings {
    /* AI System Controls */
    bool ai_system_enabled;         /* Enable AI features */
    bool ai_hardware_detection;     /* Use AI for hardware detection */
    bool ai_driver_selection;       /* Use AI for driver selection */
    bool ai_performance_optimization; /* Use AI for performance optimization */
    bool ai_predictive_maintenance; /* Enable predictive maintenance */
    bool ai_usage_analytics;        /* Collect usage analytics */
    
    /* Data Collection */
    bool telemetry_enabled;         /* System telemetry */
    bool crash_reports_enabled;     /* Crash reporting */
    bool performance_metrics_enabled; /* Performance metrics collection */
    bool hardware_survey_enabled;   /* Hardware survey participation */
    bool improvement_program_enabled; /* Product improvement program */
    
    /* Network and Connectivity */
    bool automatic_updates_enabled; /* Automatic system updates */
    bool cloud_sync_enabled;        /* Cloud synchronization */
    bool remote_support_enabled;    /* Remote support access */
    bool network_diagnostics_enabled; /* Network diagnostics */
    
    /* Location and Sensors */
    bool location_services_enabled; /* Location services */
    bool sensor_data_collection;    /* Sensor data collection */
    bool biometric_data_storage;    /* Store biometric data locally */
    
    /* Enterprise Features */
    bool enterprise_reporting;      /* Enterprise usage reporting */
    bool compliance_monitoring;     /* Compliance monitoring */
    bool audit_logging_extended;    /* Extended audit logging */
    
    /* Quantum and Advanced Features */
    bool quantum_security_enabled;  /* Quantum security features */
    bool homomorphic_encryption;    /* Homomorphic encryption for privacy */
    bool zero_knowledge_proofs;     /* Zero-knowledge authentication */
    
    /* User Control */
    bool granular_permissions;      /* Granular permission control */
    bool data_minimization;         /* Minimize data collection */
    bool opt_out_analytics;         /* Opt-out of all analytics */
    bool local_processing_only;     /* Process data locally only */
    
} installer_privacy_settings_t;

/* Hardware Device Information */
typedef struct hardware_device {
    uint32_t id;                    /* Device ID */
    hardware_category_t category;   /* Device category */
    char name[128];                 /* Device name */
    char vendor[64];                /* Vendor name */
    char model[64];                 /* Model name */
    char driver_name[64];           /* Required driver name */
    
    /* Hardware identifiers */
    uint32_t vendor_id;             /* PCI/USB vendor ID */
    uint32_t device_id;             /* PCI/USB device ID */
    uint32_t subsystem_vendor_id;   /* Subsystem vendor ID */
    uint32_t subsystem_device_id;   /* Subsystem device ID */
    char pci_path[64];              /* PCI device path */
    char usb_path[64];              /* USB device path */
    
    /* Capabilities */
    struct {
        bool supports_64bit;        /* 64-bit support */
        bool supports_virtualization; /* Hardware virtualization */
        bool supports_ai_acceleration; /* AI acceleration */
        bool supports_quantum;      /* Quantum computing */
        bool supports_encryption;   /* Hardware encryption */
        bool supports_biometric;    /* Biometric authentication */
        bool supports_tpm;          /* TPM support */
        bool supports_secure_boot;  /* Secure boot support */
    } capabilities;
    
    /* Power and thermal */
    uint32_t power_consumption_mw;  /* Power consumption in milliwatts */
    int32_t operating_temp_min;     /* Minimum operating temperature */
    int32_t operating_temp_max;     /* Maximum operating temperature */
    
    /* Status */
    bool detected;                  /* Device detected */
    bool driver_available;          /* Driver available */
    bool driver_loaded;             /* Driver loaded */
    bool functional;                /* Device functional */
    
    struct hardware_device* next;
} hardware_device_t;

/* Storage Device Information */
typedef struct storage_device {
    uint32_t id;                    /* Device ID */
    storage_device_type_t type;     /* Device type */
    char name[128];                 /* Device name */
    char model[64];                 /* Model name */
    char serial[64];                /* Serial number */
    char device_path[64];           /* Device path (/dev/sdX) */
    
    /* Capacity and geometry */
    uint64_t capacity_bytes;        /* Total capacity in bytes */
    uint64_t usable_bytes;          /* Usable capacity in bytes */
    uint32_t sector_size;           /* Sector size in bytes */
    uint64_t sector_count;          /* Total sectors */
    
    /* Performance characteristics */
    uint32_t read_speed_mbps;       /* Sequential read speed */
    uint32_t write_speed_mbps;      /* Sequential write speed */
    uint32_t random_read_iops;      /* Random read IOPS */
    uint32_t random_write_iops;     /* Random write IOPS */
    uint32_t latency_us;            /* Average latency in microseconds */
    
    /* Health and wear */
    uint8_t health_percentage;      /* Device health (0-100%) */
    uint64_t power_on_hours;        /* Power-on hours */
    uint64_t write_cycles;          /* Write cycles (for SSDs) */
    bool smart_supported;           /* SMART monitoring support */
    
    /* Security features */
    bool encryption_supported;      /* Hardware encryption support */
    bool secure_erase_supported;    /* Secure erase support */
    bool opal_encryption;           /* OPAL encryption support */
    
    /* Partitioning */
    bool has_partition_table;       /* Has partition table */
    char partition_table_type[16];  /* Partition table type (GPT/MBR) */
    uint32_t partition_count;       /* Number of partitions */
    
    /* Installation suitability */
    bool suitable_for_installation; /* Suitable for OS installation */
    bool recommended_for_installation; /* Recommended by AI */
    uint8_t suitability_score;      /* AI suitability score (0-100) */
    
    struct storage_device* next;
} storage_device_t;

/* Partition Information */
typedef struct partition {
    uint32_t id;                    /* Partition ID */
    uint32_t storage_device_id;     /* Parent storage device ID */
    partition_type_t type;          /* Partition type */
    char label[64];                 /* Partition label */
    char device_path[64];           /* Partition device path */
    
    /* Layout */
    uint64_t start_sector;          /* Starting sector */
    uint64_t end_sector;            /* Ending sector */
    uint64_t size_bytes;            /* Size in bytes */
    uint8_t partition_number;       /* Partition number */
    
    /* Filesystem */
    filesystem_type_t filesystem;   /* Filesystem type */
    char mount_point[128];          /* Mount point */
    char filesystem_options[256];   /* Filesystem options */
    
    /* Encryption */
    bool encrypted;                 /* Partition encrypted */
    char encryption_type[32];       /* Encryption type (LUKS, etc.) */
    bool quantum_encryption;        /* Quantum encryption enabled */
    
    /* AI recommendations */
    bool ai_recommended;            /* AI recommended this partition */
    uint8_t optimization_score;     /* AI optimization score */
    char ai_rationale[256];         /* AI reasoning for recommendation */
    
    struct partition* next;
} partition_t;

/* Driver Information */
typedef struct driver_info {
    uint32_t id;                    /* Driver ID */
    driver_category_t category;     /* Driver category */
    char name[128];                 /* Driver name */
    char version[32];               /* Driver version */
    char vendor[64];                /* Driver vendor */
    char description[256];          /* Driver description */
    char filename[128];             /* Driver filename */
    
    /* Hardware compatibility */
    uint32_t supported_vendor_ids[16]; /* Supported vendor IDs */
    uint32_t supported_device_ids[32]; /* Supported device IDs */
    uint32_t vendor_id_count;       /* Number of supported vendor IDs */
    uint32_t device_id_count;       /* Number of supported device IDs */
    
    /* Driver properties */
    bool is_signed;                 /* Driver is signed */
    bool opensource;                /* Open source driver */
    bool proprietary;               /* Proprietary driver */
    bool recommended;               /* Recommended by AI */
    bool required;                  /* Required for basic functionality */
    bool optional;                  /* Optional driver */
    
    /* Installation */
    uint64_t size_bytes;            /* Driver package size */
    char download_url[512];         /* Download URL */
    char checksum[128];             /* Package checksum */
    bool installed;                 /* Driver installed */
    bool loaded;                    /* Driver loaded */
    
    struct driver_info* next;
} driver_info_t;

/* Installation Step */
typedef struct installation_step {
    uint32_t id;                    /* Step ID */
    char name[128];                 /* Step name */
    char description[512];          /* Step description */
    
    /* Execution */
    status_t (*execute)(void* context); /* Step execution function */
    void* context;                  /* Step context */
    
    /* Progress tracking */
    uint8_t progress_percentage;    /* Step progress (0-100%) */
    bool completed;                 /* Step completed */
    bool skipped;                   /* Step skipped */
    bool failed;                    /* Step failed */
    char error_message[256];        /* Error message if failed */
    
    /* Timing */
    uint64_t start_time;            /* Step start time */
    uint64_t end_time;              /* Step end time */
    uint64_t estimated_duration;    /* AI estimated duration */
    
    /* Dependencies */
    uint32_t dependencies[16];      /* Dependent step IDs */
    uint32_t dependency_count;      /* Number of dependencies */
    
    struct installation_step* next;
} installation_step_t;

/* User Profile */
typedef struct user_profile {
    uint32_t id;                    /* Profile ID */
    char name[128];                 /* Profile name */
    char full_name[256];            /* User full name */
    char email[256];                /* Email address */
    
    /* User preferences */
    installation_type_t preferred_installation_type;
    char preferred_language[32];    /* Language code */
    char preferred_timezone[64];    /* Timezone */
    char preferred_keyboard_layout[32]; /* Keyboard layout */
    
    /* Privacy preferences */
    installer_privacy_settings_t privacy_settings;
    
    /* Enterprise settings */
    bool enterprise_user;           /* Enterprise user */
    char domain[128];               /* Enterprise domain */
    char organizational_unit[128];  /* Organizational unit */
    char enterprise_policy_server[256]; /* Policy server URL */
    
    /* AI preferences */
    bool allow_ai_optimization;     /* Allow AI optimization */
    bool allow_ai_recommendations; /* Allow AI recommendations */
    bool allow_behavioral_learning; /* Allow AI to learn from behavior */
    
    struct user_profile* next;
} user_profile_t;

/* Installation Configuration */
typedef struct installation_config {
    /* Basic configuration */
    installation_type_t installation_type;
    char installation_name[128];    /* Installation name/identifier */
    char target_architecture[32];   /* Target architecture (x86_64, aarch64, etc.) */
    
    /* Target storage */
    storage_device_t* target_device; /* Target storage device */
    partition_t* partitions;        /* Partition layout */
    uint32_t partition_count;       /* Number of partitions */
    
    /* User configuration */
    user_profile_t* primary_user;   /* Primary user profile */
    user_profile_t* additional_users; /* Additional user profiles */
    uint32_t user_count;            /* Number of user profiles */
    
    /* System configuration */
    char hostname[128];             /* System hostname */
    char root_password_hash[256];   /* Root password hash */
    bool enable_root_login;         /* Enable root login */
    bool enable_ssh;                /* Enable SSH server */
    bool enable_firewall;           /* Enable firewall */
    
    /* Network configuration */
    struct {
        bool use_dhcp;              /* Use DHCP */
        char static_ip[64];         /* Static IP address */
        char netmask[64];           /* Network mask */
        char gateway[64];           /* Default gateway */
        char dns_servers[256];      /* DNS servers */
        char wifi_ssid[128];        /* WiFi SSID */
        char wifi_password[256];    /* WiFi password */
    } network;
    
    /* Software selection */
    struct {
        bool desktop_environment;   /* Install desktop environment */
        bool development_tools;     /* Install development tools */
        bool multimedia_codecs;     /* Install multimedia codecs */
        bool office_suite;          /* Install office suite */
        bool games;                 /* Install games */
        bool ai_tools;              /* Install AI/ML tools */
        bool quantum_tools;         /* Install quantum computing tools */
        bool enterprise_tools;      /* Install enterprise tools */
        bool virtualization;        /* Install virtualization support */
    } software;
    
    /* Hardware drivers */
    driver_info_t* selected_drivers; /* Selected drivers */
    uint32_t driver_count;          /* Number of selected drivers */
    
    /* Privacy and security */
    installer_privacy_settings_t privacy_settings;
    bool enable_full_disk_encryption; /* Full disk encryption */
    bool enable_secure_boot;        /* Secure boot */
    bool enable_tpm;                /* TPM usage */
    bool enable_quantum_encryption; /* Quantum encryption */
    
    /* AI configuration */
    struct {
        bool enable_ai_assistant;   /* Enable AI assistant */
        bool enable_predictive_maintenance; /* Predictive maintenance */
        bool enable_performance_optimization; /* Performance optimization */
        bool enable_security_monitoring; /* Security monitoring */
        float ai_aggressiveness;    /* AI aggressiveness level (0.0-1.0) */
    } ai_config;
    
} installation_config_t;

/* Installer State */
typedef struct intelligent_installer {
    bool initialized;
    uint32_t version;
    
    /* Hardware detection */
    hardware_device_t* detected_hardware; /* Detected hardware */
    uint32_t hardware_device_count;  /* Number of detected devices */
    storage_device_t* storage_devices; /* Available storage devices */
    uint32_t storage_device_count;   /* Number of storage devices */
    
    /* Driver database */
    driver_info_t* available_drivers; /* Available drivers */
    uint32_t available_driver_count; /* Number of available drivers */
    
    /* Installation process */
    installation_config_t* config;  /* Installation configuration */
    installation_step_t* steps;     /* Installation steps */
    uint32_t step_count;            /* Number of steps */
    uint32_t current_step;          /* Current step index */
    
    /* Progress tracking */
    uint8_t overall_progress;       /* Overall progress (0-100%) */
    bool installation_running;      /* Installation in progress */
    bool installation_completed;    /* Installation completed */
    bool installation_failed;       /* Installation failed */
    char failure_reason[512];       /* Failure reason */
    
    /* AI features */
    struct {
        bool hardware_detection_ai; /* AI hardware detection enabled */
        bool partition_ai;          /* AI partitioning enabled */
        bool driver_selection_ai;   /* AI driver selection enabled */
        bool optimization_ai;       /* AI optimization enabled */
        void* ai_model_context;     /* AI model context */
        float detection_confidence; /* Hardware detection confidence */
    } ai_features;
    
    /* Statistics */
    struct {
        uint64_t total_installations;
        uint64_t successful_installations;
        uint64_t failed_installations;
        uint64_t average_install_time_minutes;
        uint32_t most_common_hardware_vendor;
        uint32_t most_common_storage_type;
        float ai_accuracy_rate;
    } statistics;
    
} intelligent_installer_t;

/* Global installer instance */
extern intelligent_installer_t installer;

/* Core Installer API */
status_t installer_init(void);
void installer_shutdown(void);
bool installer_is_initialized(void);

/* Hardware Detection */
status_t installer_detect_hardware(void);
status_t installer_detect_storage_devices(void);
status_t installer_analyze_hardware_compatibility(void);
hardware_device_t* installer_get_hardware_by_category(hardware_category_t category);
storage_device_t* installer_get_storage_device_by_id(uint32_t id);
status_t installer_benchmark_storage_devices(void);

/* AI-Powered Features */
status_t installer_enable_ai_detection(bool enable);
status_t installer_ai_recommend_partitioning(storage_device_t* device, partition_t** partitions, uint32_t* count);
status_t installer_ai_select_drivers(driver_info_t** selected_drivers, uint32_t* count);
status_t installer_ai_optimize_configuration(installation_config_t* config);
status_t installer_ai_estimate_installation_time(installation_config_t* config, uint64_t* estimated_minutes);
status_t installer_ai_predict_performance(installation_config_t* config, float* performance_score);

/* Driver Management */
status_t installer_scan_available_drivers(void);
status_t installer_download_driver(driver_info_t* driver);
status_t installer_install_driver(driver_info_t* driver);
status_t installer_verify_driver_signature(driver_info_t* driver, bool* valid);
driver_info_t* installer_find_driver_for_device(hardware_device_t* device);
status_t installer_get_recommended_drivers(driver_info_t** drivers, uint32_t* count);

/* Partitioning */
status_t installer_create_partition_scheme(storage_device_t* device, installation_type_t type, partition_t** partitions, uint32_t* count);
status_t installer_create_partition(storage_device_t* device, partition_type_t type, uint64_t size_bytes, partition_t** partition);
status_t installer_format_partition(partition_t* partition, filesystem_type_t filesystem);
status_t installer_encrypt_partition(partition_t* partition, const char* passphrase);
status_t installer_mount_partition(partition_t* partition, const char* mount_point);
status_t installer_validate_partition_layout(partition_t* partitions, uint32_t count, bool* valid);

/* Installation Configuration */
installation_config_t* installer_create_config(installation_type_t type);
void installer_destroy_config(installation_config_t* config);
status_t installer_save_config(installation_config_t* config, const char* filename);
status_t installer_load_config(const char* filename, installation_config_t** config);
status_t installer_validate_config(installation_config_t* config, bool* valid);

/* User Management */
user_profile_t* installer_create_user_profile(const char* username);
void installer_destroy_user_profile(user_profile_t* profile);
status_t installer_add_user_to_config(installation_config_t* config, user_profile_t* profile);
status_t installer_configure_user_privacy(user_profile_t* profile, const installer_privacy_settings_t* settings);

/* Privacy Controls */
status_t installer_set_privacy_defaults(installer_privacy_settings_t* settings);
status_t installer_show_privacy_dialog(installer_privacy_settings_t* settings, bool* accepted);
status_t installer_apply_privacy_settings(const installer_privacy_settings_t* settings);
status_t installer_export_privacy_settings(const installer_privacy_settings_t* settings, const char* filename);
status_t installer_import_privacy_settings(const char* filename, installer_privacy_settings_t* settings);

/* Installation Process */
status_t installer_prepare_installation(installation_config_t* config);
status_t installer_start_installation(void);
status_t installer_pause_installation(void);
status_t installer_resume_installation(void);
status_t installer_cancel_installation(void);
uint8_t installer_get_progress(void);
status_t installer_get_current_step(installation_step_t** step);

/* Installation Steps */
status_t installer_add_step(const char* name, const char* description, status_t (*execute)(void*), void* context);
status_t installer_execute_step(uint32_t step_id);
status_t installer_skip_step(uint32_t step_id);
status_t installer_retry_step(uint32_t step_id);

/* Enterprise Features */
status_t installer_apply_enterprise_policy(const char* policy_url);
status_t installer_configure_domain_join(const char* domain, const char* username, const char* password);
status_t installer_install_enterprise_certificates(const char* cert_bundle_path);
status_t installer_configure_enterprise_network(const char* config_file);

/* Quantum Security */
status_t installer_enable_quantum_security(bool enable);
status_t installer_generate_quantum_keys(void);
status_t installer_configure_quantum_encryption(partition_t* partition);
status_t installer_verify_quantum_integrity(void);

/* Recovery and Backup */
status_t installer_create_recovery_partition(storage_device_t* device, uint64_t size_bytes);
status_t installer_backup_existing_system(const char* backup_path);
status_t installer_create_system_restore_point(void);
status_t installer_enable_automatic_backups(bool enable);

/* Utilities */
const char* installer_get_hardware_category_name(hardware_category_t category);
const char* installer_get_storage_type_name(storage_device_type_t type);
const char* installer_get_filesystem_name(filesystem_type_t filesystem);
const char* installer_get_partition_type_name(partition_type_t type);
const char* installer_get_installation_type_name(installation_type_t type);
uint64_t installer_bytes_to_human_readable(uint64_t bytes, char* buffer, size_t buffer_size);

/* Statistics and Reporting */
void installer_print_hardware_summary(void);
void installer_print_storage_summary(void);
void installer_print_installation_summary(void);
status_t installer_generate_installation_report(const char* report_file);
status_t installer_export_hardware_profile(const char* profile_file);

/* User Interface Helpers */
status_t installer_show_hardware_detection_progress(void);
status_t installer_show_partitioning_wizard(storage_device_t* device, partition_t** partitions, uint32_t* count);
status_t installer_show_driver_selection_dialog(driver_info_t* available_drivers, uint32_t count, driver_info_t** selected_drivers, uint32_t* selected_count);
status_t installer_show_installation_progress(void);
status_t installer_show_completion_dialog(void);

#endif /* LIMITLESS_INTELLIGENT_INSTALLER_H */