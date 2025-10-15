/*
 * LimitlessOS Package Management & Software Distribution System
 * Modern package manager with dependency resolution, security, and enterprise features
 */

#ifndef LIMITLESSOS_PACKAGE_MANAGER_H
#define LIMITLESSOS_PACKAGE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>

/* Package States */
typedef enum {
    PKG_STATE_NOT_INSTALLED = 0,
    PKG_STATE_INSTALLED,
    PKG_STATE_UPGRADEABLE,
    PKG_STATE_BROKEN,
    PKG_STATE_PENDING_INSTALL,
    PKG_STATE_PENDING_REMOVE,
    PKG_STATE_PENDING_UPGRADE,
    PKG_STATE_HELD,
    PKG_STATE_MAX
} package_state_t;

/* Package Types */
typedef enum {
    PKG_TYPE_APPLICATION = 0,
    PKG_TYPE_LIBRARY,
    PKG_TYPE_DEVELOPMENT,
    PKG_TYPE_SYSTEM,
    PKG_TYPE_KERNEL_MODULE,
    PKG_TYPE_FIRMWARE,
    PKG_TYPE_FONT,
    PKG_TYPE_THEME,
    PKG_TYPE_LANGUAGE_PACK,
    PKG_TYPE_DOCUMENTATION,
    PKG_TYPE_MAX
} package_type_t;

/* Package Architectures */
typedef enum {
    PKG_ARCH_ANY = 0,
    PKG_ARCH_X86_64,
    PKG_ARCH_ARM64,
    PKG_ARCH_RISCV64,
    PKG_ARCH_I386,
    PKG_ARCH_MAX
} package_arch_t;

/* Package Priority Levels */
typedef enum {
    PKG_PRIORITY_REQUIRED = 0,     /* Essential system packages */
    PKG_PRIORITY_IMPORTANT,        /* Important system packages */
    PKG_PRIORITY_STANDARD,         /* Standard packages */
    PKG_PRIORITY_OPTIONAL,         /* Optional packages */
    PKG_PRIORITY_EXTRA,           /* Extra packages */
    PKG_PRIORITY_MAX
} package_priority_t;

/* Dependency Types */
typedef enum {
    DEP_TYPE_DEPENDS = 0,         /* Hard dependency */
    DEP_TYPE_RECOMMENDS,          /* Recommended dependency */
    DEP_TYPE_SUGGESTS,            /* Suggested dependency */
    DEP_TYPE_CONFLICTS,           /* Package conflicts */
    DEP_TYPE_BREAKS,              /* Package breaks */
    DEP_TYPE_REPLACES,            /* Package replaces */
    DEP_TYPE_PROVIDES,            /* Virtual package provides */
    DEP_TYPE_MAX
} dependency_type_t;

/* Version Comparison Operators */
typedef enum {
    VERSION_OP_EQ = 0,            /* Equal (=) */
    VERSION_OP_LT,                /* Less than (<) */
    VERSION_OP_LE,                /* Less than or equal (<=) */
    VERSION_OP_GT,                /* Greater than (>) */
    VERSION_OP_GE,                /* Greater than or equal (>=) */
    VERSION_OP_NE,                /* Not equal (!=) */
    VERSION_OP_MAX
} version_operator_t;

/* Security Scan Results */
typedef enum {
    SECURITY_CLEAN = 0,
    SECURITY_LOW_RISK,
    SECURITY_MEDIUM_RISK,
    SECURITY_HIGH_RISK,
    SECURITY_CRITICAL_RISK,
    SECURITY_MAX
} security_risk_t;

/* Package Version Structure */
typedef struct {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    uint32_t build;
    char suffix[32];              /* -alpha, -beta, -rc1, etc. */
} package_version_t;

/* Package Dependency */
typedef struct {
    char name[128];               /* Package name */
    dependency_type_t type;       /* Dependency type */
    version_operator_t operator;  /* Version operator */
    package_version_t version;    /* Required version */
    bool optional;                /* Optional dependency */
} package_dependency_t;

/* Package File Information */
typedef struct {
    char path[512];               /* File path */
    char checksum[65];            /* SHA-256 checksum */
    uint64_t size;                /* File size in bytes */
    uint32_t permissions;         /* File permissions */
    time_t mtime;                 /* Modification time */
    bool is_config;               /* Configuration file */
    bool is_executable;           /* Executable file */
} package_file_t;

/* Package Metadata */
typedef struct {
    /* Basic information */
    char name[128];               /* Package name */
    package_version_t version;    /* Package version */
    package_type_t type;          /* Package type */
    package_arch_t architecture;  /* Target architecture */
    package_priority_t priority;  /* Package priority */
    
    /* Description and metadata */
    char short_description[256];  /* Brief description */
    char long_description[1024];  /* Detailed description */
    char homepage[512];           /* Project homepage */
    char maintainer[256];         /* Package maintainer */
    char license[128];            /* Software license */
    char section[64];             /* Package section/category */
    
    /* Size information */
    uint64_t installed_size;      /* Installed size in bytes */
    uint64_t download_size;       /* Download size in bytes */
    
    /* Dependencies */
    uint32_t dependency_count;
    package_dependency_t dependencies[64];
    
    /* File list */
    uint32_t file_count;
    package_file_t *files;        /* Dynamically allocated file list */
    
    /* Installation information */
    package_state_t state;        /* Current package state */
    time_t install_time;          /* Installation timestamp */
    time_t build_time;            /* Package build timestamp */
    char build_host[256];         /* Build host information */
    
    /* Security information */
    security_risk_t security_risk; /* Security risk level */
    uint32_t vulnerability_count;  /* Number of known vulnerabilities */
    char signature[256];          /* Package signature */
    bool signature_valid;         /* Signature verification status */
    
    /* Repository information */
    char repository[128];         /* Source repository */
    char repository_url[512];     /* Repository URL */
    uint32_t download_count;      /* Download statistics */
    
    /* Upgrade information */
    package_version_t available_version; /* Available upgrade version */
    bool auto_upgrade;            /* Automatic upgrade enabled */
} package_metadata_t;

/* Repository Configuration */
typedef struct {
    char name[128];               /* Repository name */
    char url[512];                /* Repository URL */
    char distribution[64];        /* Distribution name */
    char components[256];         /* Repository components */
    bool enabled;                 /* Repository enabled */
    bool trusted;                 /* Trusted repository */
    char gpg_key[256];           /* GPG key for verification */
    uint32_t priority;            /* Repository priority */
    time_t last_update;          /* Last update timestamp */
} repository_config_t;

/* Transaction Types */
typedef enum {
    TRANSACTION_INSTALL = 0,
    TRANSACTION_REMOVE,
    TRANSACTION_UPGRADE,
    TRANSACTION_DOWNGRADE,
    TRANSACTION_REINSTALL,
    TRANSACTION_CONFIGURE,
    TRANSACTION_MAX
} transaction_type_t;

/* Transaction Status */
typedef enum {
    TRANSACTION_STATUS_PENDING = 0,
    TRANSACTION_STATUS_DOWNLOADING,
    TRANSACTION_STATUS_VERIFYING,
    TRANSACTION_STATUS_INSTALLING,
    TRANSACTION_STATUS_CONFIGURING,
    TRANSACTION_STATUS_COMPLETED,
    TRANSACTION_STATUS_FAILED,
    TRANSACTION_STATUS_ROLLED_BACK,
    TRANSACTION_STATUS_MAX
} transaction_status_t;

/* Package Transaction */
typedef struct {
    char id[64];                  /* Transaction ID */
    transaction_type_t type;      /* Transaction type */
    transaction_status_t status;  /* Current status */
    time_t start_time;            /* Transaction start time */
    time_t end_time;              /* Transaction end time */
    
    /* Packages involved */
    uint32_t package_count;
    char packages[64][128];       /* Package names */
    
    /* Progress information */
    uint32_t total_packages;      /* Total packages to process */
    uint32_t completed_packages;  /* Completed packages */
    uint64_t total_bytes;         /* Total bytes to download */
    uint64_t downloaded_bytes;    /* Bytes downloaded */
    
    /* Error information */
    int error_code;               /* Error code if failed */
    char error_message[256];      /* Error description */
    
    /* Rollback information */
    bool rollback_available;      /* Can be rolled back */
    char rollback_data[512];      /* Rollback information */
} package_transaction_t;

/* Package Manager Configuration */
typedef struct {
    /* General settings */
    bool auto_update_cache;       /* Automatically update package cache */
    bool auto_upgrade;            /* Enable automatic upgrades */
    bool auto_remove_unused;      /* Remove unused dependencies */
    uint32_t cache_retention_days; /* Cache retention period */
    
    /* Repository settings */
    uint32_t repository_count;
    repository_config_t repositories[16];
    
    /* Download settings */
    char cache_directory[512];    /* Package cache directory */
    char temp_directory[512];     /* Temporary directory */
    uint32_t max_download_threads; /* Maximum download threads */
    uint32_t max_download_speed;  /* Maximum download speed (KB/s) */
    uint32_t download_timeout;    /* Download timeout (seconds) */
    uint32_t max_retries;         /* Maximum download retries */
    
    /* Security settings */
    bool verify_signatures;       /* Verify package signatures */
    bool security_scanning;       /* Enable security scanning */
    bool block_untrusted;         /* Block untrusted packages */
    uint32_t min_security_level;  /* Minimum security level */
    
    /* Installation settings */
    bool atomic_operations;       /* Use atomic transactions */
    bool create_backups;          /* Create backup before changes */
    bool preserve_config;         /* Preserve configuration files */
    uint32_t transaction_timeout; /* Transaction timeout (seconds) */
    
    /* Logging and monitoring */
    bool detailed_logging;        /* Enable detailed logging */
    char log_directory[512];      /* Log directory */
    uint32_t log_retention_days;  /* Log retention period */
    bool performance_monitoring;  /* Monitor performance metrics */
} package_manager_config_t;

/* Package Database */
typedef struct {
    bool initialized;
    pthread_mutex_t lock;
    
    /* Configuration */
    package_manager_config_t config;
    
    /* Package database */
    uint32_t package_count;
    package_metadata_t *packages; /* Dynamic array of packages */
    
    /* Transaction management */
    uint32_t transaction_count;
    package_transaction_t transactions[256];
    char current_transaction[64]; /* Current transaction ID */
    
    /* Cache management */
    char cache_path[512];         /* Package cache path */
    uint64_t cache_size;          /* Current cache size */
    uint64_t cache_limit;         /* Cache size limit */
    
    /* Statistics */
    struct {
        uint64_t total_installs;
        uint64_t total_removes;
        uint64_t total_upgrades;
        uint64_t total_downloads;
        uint64_t total_bytes_downloaded;
        uint64_t failed_transactions;
        time_t last_cache_update;
        time_t last_security_scan;
    } stats;
    
    /* Threading */
    pthread_t download_thread;
    pthread_t security_thread;
    pthread_t cleanup_thread;
    bool threads_running;
} package_manager_t;

/* Search Criteria */
typedef struct {
    char name_pattern[256];       /* Name pattern to search */
    char description_pattern[256]; /* Description pattern */
    package_type_t type;          /* Package type filter */
    package_arch_t architecture;  /* Architecture filter */
    package_state_t state;        /* Installation state filter */
    bool installed_only;          /* Search installed packages only */
    bool available_only;          /* Search available packages only */
} package_search_t;

/* Update Information */
typedef struct {
    uint32_t total_packages;      /* Total packages available */
    uint32_t upgradeable_packages; /* Packages with upgrades */
    uint32_t security_updates;    /* Security updates available */
    uint64_t total_download_size; /* Total download size */
    time_t last_check;            /* Last update check */
} update_info_t;

/* Function Prototypes */

/* System Initialization */
int package_manager_init(void);
int package_manager_cleanup(void);
int package_manager_load_config(const char *config_file);
int package_manager_save_config(const char *config_file);

/* Repository Management */
int repository_add(const char *name, const char *url, const char *distribution);
int repository_remove(const char *name);
int repository_enable(const char *name, bool enabled);
int repository_update_cache(const char *name);
int repository_update_all(void);
int repository_list(repository_config_t *repos, uint32_t max_count, uint32_t *count);

/* Package Information */
package_metadata_t *package_find(const char *name);
int package_search(const package_search_t *criteria, package_metadata_t *results, 
                  uint32_t max_results, uint32_t *count);
int package_list_installed(package_metadata_t *results, uint32_t max_results, uint32_t *count);
int package_list_available(package_metadata_t *results, uint32_t max_results, uint32_t *count);
int package_get_info(const char *name, package_metadata_t *info);

/* Package Installation */
int package_install(const char *name, const package_version_t *version);
int package_install_file(const char *package_file);
int package_install_multiple(const char **names, uint32_t count);
int package_remove(const char *name);
int package_remove_multiple(const char **names, uint32_t count);
int package_upgrade(const char *name);
int package_upgrade_all(void);
int package_downgrade(const char *name, const package_version_t *version);
int package_reinstall(const char *name);

/* Dependency Resolution */
int dependency_resolve(const char *package_name, char **dependencies, uint32_t *count);
int dependency_check_conflicts(const char *package_name);
int dependency_find_broken(char **broken_packages, uint32_t *count);
int dependency_fix_broken(void);

/* Transaction Management */
int transaction_begin(transaction_type_t type, const char *description);
int transaction_add_package(const char *transaction_id, const char *package_name);
int transaction_commit(const char *transaction_id);
int transaction_rollback(const char *transaction_id);
int transaction_get_status(const char *transaction_id, package_transaction_t *status);
int transaction_list_active(package_transaction_t *transactions, uint32_t max_count, uint32_t *count);

/* Security Features */
int security_scan_package(const char *package_name, security_risk_t *risk);
int security_scan_system(void);
int security_check_vulnerabilities(const char *package_name);
int security_apply_updates(void);
int package_verify_signature(const char *package_file);

/* Update Management */
int update_check_available(update_info_t *info);
int update_download_packages(const char **package_names, uint32_t count);
int update_install_security(void);
int update_schedule_automatic(uint32_t hour, uint32_t minute);

/* Cache Management */
int cache_update(void);
int cache_clean(void);
int cache_clean_packages(void);
int cache_rebuild(void);
uint64_t cache_get_size(void);
int cache_set_limit(uint64_t limit_bytes);

/* Configuration Management */
int config_set_repository_priority(const char *name, uint32_t priority);
int config_set_auto_upgrade(bool enabled);
int config_set_security_level(uint32_t level);
int config_add_trusted_key(const char *key_file);

/* Package Creation and Building */
int package_build_from_source(const char *source_dir, const char *build_script);
int package_create_metadata(const char *package_dir, package_metadata_t *metadata);
int package_create_archive(const char *package_dir, const char *output_file);
int package_sign(const char *package_file, const char *key_file);

/* Import/Export */
int package_export_list(const char *output_file);
int package_import_list(const char *input_file);
int package_clone_system(const char *target_system);

/* Statistics and Monitoring */
int package_get_statistics(void *stats_buffer, size_t buffer_size);
int package_monitor_performance(void);
int package_generate_report(const char *report_file);

/* Utilities */
const char *package_state_name(package_state_t state);
const char *package_type_name(package_type_t type);
const char *transaction_status_name(transaction_status_t status);
const char *security_risk_name(security_risk_t risk);
int version_compare(const package_version_t *v1, const package_version_t *v2);
int version_parse(const char *version_string, package_version_t *version);
char *version_to_string(const package_version_t *version);
bool package_is_installed(const char *name);
bool package_is_upgradeable(const char *name);
uint64_t package_calculate_dependencies_size(const char *name);

#endif /* LIMITLESSOS_PACKAGE_MANAGER_H */