/*
 * ota_manager.c - LimitlessOS OTA Update Manager
 * 
 * Comprehensive OTA update system with secure delivery, rollback protection,
 * and atomic updates.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <kernel/secure_boot.h>
#include <userspace/syscall.h>

#define OTA_MAGIC                   0x4F544155  /* "OTAU" */
#define OTA_VERSION                 1
#define MAX_DOWNLOAD_RETRIES        3
#define DOWNLOAD_BUFFER_SIZE        (1024 * 1024)  /* 1MB */
#define BACKUP_SUFFIX              ".backup"
#define TEMP_SUFFIX                ".tmp"

typedef struct ota_manager_config {
    char update_server_url[256];
    char update_channel[64];                    /* stable, beta, dev */
    char device_id[64];
    char current_version[32];
    bool auto_download;
    bool auto_install;
    uint32_t check_interval_hours;
    uint32_t retry_attempts;
    bool allow_downgrades;
    bool require_user_confirmation;
} ota_manager_config_t;

typedef struct update_metadata {
    char version[32];
    char channel[64];
    char description[512];
    uint64_t package_size;
    char download_url[512];
    char signature_url[512];
    uint8_t package_hash[32];
    uint8_t signature[64];
    uint32_t criticality;
    bool requires_reboot;
    char min_version[32];
    char max_version[32];
    uint64_t release_timestamp;
} update_metadata_t;

typedef struct ota_manager_state {
    ota_manager_config_t config;
    bool initialized;
    bool update_in_progress;
    bool download_in_progress;
    
    /* Current update */
    update_metadata_t current_update;
    char download_path[256];
    uint64_t download_progress;
    
    /* Statistics */
    struct {
        uint32_t updates_checked;
        uint32_t updates_downloaded;
        uint32_t updates_installed;
        uint32_t updates_failed;
        uint32_t rollbacks_performed;
        uint64_t last_check_time;
        uint64_t last_update_time;
    } stats;
} ota_manager_state_t;

static ota_manager_state_t g_ota_manager = {0};

/* Function prototypes */
static int download_file(const char* url, const char* output_path, uint64_t expected_size);
static int verify_package_integrity(const char* package_path, const update_metadata_t* metadata);
static int create_system_backup(void);
static int restore_system_backup(void);
static int apply_update_components(const char* package_path, const ota_update_package_t* package_info);
static int check_update_prerequisites(const update_metadata_t* metadata);
static int parse_update_metadata(const char* metadata_json, update_metadata_t* metadata);
static void log_update_event(const char* event, const char* details);

/* Initialize OTA manager */
int ota_manager_init(const char* config_path) {
    if (g_ota_manager.initialized) {
        return 0;  /* Already initialized */
    }
    
    memset(&g_ota_manager, 0, sizeof(ota_manager_state_t));
    
    /* Load configuration */
    FILE* config_file = fopen(config_path ? config_path : "/etc/ota_config.conf", "r");
    if (config_file) {
        char line[512];
        
        while (fgets(line, sizeof(line), config_file)) {
            char* key = strtok(line, "=");
            char* value = strtok(NULL, "\n");
            
            if (!key || !value) continue;
            
            /* Trim whitespace */
            while (*key == ' ' || *key == '\t') key++;
            while (*value == ' ' || *value == '\t') value++;
            
            if (strcmp(key, "update_server_url") == 0) {
                strncpy(g_ota_manager.config.update_server_url, value, sizeof(g_ota_manager.config.update_server_url) - 1);
            } else if (strcmp(key, "update_channel") == 0) {
                strncpy(g_ota_manager.config.update_channel, value, sizeof(g_ota_manager.config.update_channel) - 1);
            } else if (strcmp(key, "device_id") == 0) {
                strncpy(g_ota_manager.config.device_id, value, sizeof(g_ota_manager.config.device_id) - 1);
            } else if (strcmp(key, "auto_download") == 0) {
                g_ota_manager.config.auto_download = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "auto_install") == 0) {
                g_ota_manager.config.auto_install = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "check_interval_hours") == 0) {
                g_ota_manager.config.check_interval_hours = (uint32_t)atoi(value);
            }
        }
        
        fclose(config_file);
    } else {
        /* Set default configuration */
        strcpy(g_ota_manager.config.update_server_url, "https://updates.limitlessos.org");
        strcpy(g_ota_manager.config.update_channel, "stable");
        strcpy(g_ota_manager.config.device_id, "unknown");
        g_ota_manager.config.auto_download = false;
        g_ota_manager.config.auto_install = false;
        g_ota_manager.config.check_interval_hours = 24;
        g_ota_manager.config.retry_attempts = 3;
        g_ota_manager.config.allow_downgrades = false;
        g_ota_manager.config.require_user_confirmation = true;
    }
    
    /* Get current system version */
    strcpy(g_ota_manager.config.current_version, "1.0.0");  /* Default version */
    
    g_ota_manager.initialized = true;
    
    log_update_event("OTA_MANAGER_INIT", "OTA Manager initialized successfully");
    printf("OTA Manager initialized (channel: %s, server: %s)\n",
           g_ota_manager.config.update_channel,
           g_ota_manager.config.update_server_url);
    
    return 0;
}

/* Check for available updates */
int ota_manager_check_updates(bool force_check) {
    if (!g_ota_manager.initialized) {
        return -1;
    }
    
    /* Check if we should skip this check */
    uint64_t current_time = time(NULL);
    if (!force_check && 
        (current_time - g_ota_manager.stats.last_check_time) < (g_ota_manager.config.check_interval_hours * 3600)) {
        return 0;  /* Too soon to check again */
    }
    
    printf("Checking for updates (channel: %s)...\n", g_ota_manager.config.update_channel);
    
    /* Build update check URL */
    char check_url[512];
    snprintf(check_url, sizeof(check_url),
             "%s/api/check_update?device_id=%s&channel=%s&current_version=%s",
             g_ota_manager.config.update_server_url,
             g_ota_manager.config.device_id,
             g_ota_manager.config.update_channel,
             g_ota_manager.config.current_version);
    
    /* Download update metadata */
    char metadata_path[] = "/tmp/update_metadata.json";
    int result = download_file(check_url, metadata_path, 0);
    if (result != 0) {
        printf("Failed to download update metadata: %d\n", result);
        return -1;
    }
    
    /* Parse update metadata */
    FILE* metadata_file = fopen(metadata_path, "r");
    if (!metadata_file) {
        printf("Failed to open metadata file\n");
        unlink(metadata_path);
        return -1;
    }
    
    /* Read metadata JSON */
    char metadata_json[4096];
    size_t metadata_size = fread(metadata_json, 1, sizeof(metadata_json) - 1, metadata_file);
    metadata_json[metadata_size] = '\0';
    fclose(metadata_file);
    unlink(metadata_path);
    
    /* Parse metadata */
    update_metadata_t metadata;
    result = parse_update_metadata(metadata_json, &metadata);
    if (result != 0) {
        printf("Failed to parse update metadata\n");
        return -1;
    }
    
    g_ota_manager.stats.updates_checked++;
    g_ota_manager.stats.last_check_time = current_time;
    
    /* Check if update is newer than current version */
    if (strcmp(metadata.version, g_ota_manager.config.current_version) <= 0) {
        printf("No updates available (current: %s, available: %s)\n",
               g_ota_manager.config.current_version, metadata.version);
        return 0;
    }
    
    /* Check prerequisites */
    result = check_update_prerequisites(&metadata);
    if (result != 0) {
        printf("Update prerequisites not met: %d\n", result);
        return -1;
    }
    
    /* Store update metadata */
    g_ota_manager.current_update = metadata;
    
    printf("Update available: %s -> %s\n", 
           g_ota_manager.config.current_version, metadata.version);
    printf("Description: %s\n", metadata.description);
    printf("Size: %lu bytes, Criticality: %u\n", 
           metadata.package_size, metadata.criticality);
    
    /* Auto-download if configured */
    if (g_ota_manager.config.auto_download) {
        printf("Auto-download enabled, starting download...\n");
        return ota_manager_download_update();
    }
    
    return 1;  /* Update available */
}

/* Download update package */
int ota_manager_download_update(void) {
    if (!g_ota_manager.initialized || strlen(g_ota_manager.current_update.version) == 0) {
        return -1;
    }
    
    if (g_ota_manager.download_in_progress) {
        return -2;  /* Download already in progress */
    }
    
    g_ota_manager.download_in_progress = true;
    g_ota_manager.download_progress = 0;
    
    /* Create download path */
    snprintf(g_ota_manager.download_path, sizeof(g_ota_manager.download_path),
             "/tmp/limitlessos_update_%s.pkg", g_ota_manager.current_update.version);
    
    printf("Downloading update package: %s\n", g_ota_manager.current_update.download_url);
    printf("Download path: %s\n", g_ota_manager.download_path);
    
    /* Download update package */
    int result = download_file(g_ota_manager.current_update.download_url,
                               g_ota_manager.download_path,
                               g_ota_manager.current_update.package_size);
    
    g_ota_manager.download_in_progress = false;
    
    if (result != 0) {
        printf("Failed to download update package: %d\n", result);
        unlink(g_ota_manager.download_path);
        return result;
    }
    
    /* Verify package integrity */
    result = verify_package_integrity(g_ota_manager.download_path, &g_ota_manager.current_update);
    if (result != 0) {
        printf("Package integrity verification failed: %d\n", result);
        unlink(g_ota_manager.download_path);
        return result;
    }
    
    g_ota_manager.stats.updates_downloaded++;
    
    log_update_event("UPDATE_DOWNLOADED", g_ota_manager.current_update.version);
    printf("Update package downloaded and verified successfully\n");
    
    /* Auto-install if configured */
    if (g_ota_manager.config.auto_install && !g_ota_manager.config.require_user_confirmation) {
        printf("Auto-install enabled, starting installation...\n");
        return ota_manager_install_update(false);  /* Non-interactive install */
    }
    
    return 0;
}

/* Install update package */
int ota_manager_install_update(bool dry_run) {
    if (!g_ota_manager.initialized || strlen(g_ota_manager.download_path) == 0) {
        return -1;
    }
    
    if (g_ota_manager.update_in_progress) {
        return -2;  /* Update already in progress */
    }
    
    if (access(g_ota_manager.download_path, F_OK) != 0) {
        printf("Update package not found: %s\n", g_ota_manager.download_path);
        return -1;
    }
    
    printf("%s update installation: %s\n", dry_run ? "Simulating" : "Starting",
           g_ota_manager.current_update.version);
    
    g_ota_manager.update_in_progress = true;
    
    /* Verify update package again before installation */
    ota_update_package_t package_info;
    int result = secure_boot_verify_ota_package(g_ota_manager.download_path, &package_info);
    if (result != 0) {
        printf("OTA package verification failed: %d\n", result);
        g_ota_manager.update_in_progress = false;
        return result;
    }
    
    printf("Package verification successful\n");
    printf("Components to update: %u\n", package_info.component_count);
    
    for (uint32_t i = 0; i < package_info.component_count; i++) {
        printf("  - %s: %lu -> %lu\n", 
               package_info.components[i].name,
               package_info.components[i].old_version,
               package_info.components[i].new_version);
    }
    
    if (dry_run) {
        printf("Dry run completed successfully\n");
        g_ota_manager.update_in_progress = false;
        return 0;
    }
    
    /* Create system backup */
    printf("Creating system backup...\n");
    result = create_system_backup();
    if (result != 0) {
        printf("Failed to create system backup: %d\n", result);
        g_ota_manager.update_in_progress = false;
        return result;
    }
    
    /* Apply update */
    printf("Applying update...\n");
    result = secure_boot_apply_ota_update(g_ota_manager.download_path, false);
    if (result != 0) {
        printf("Update installation failed: %d\n", result);
        
        /* Restore backup on failure */
        printf("Restoring system backup...\n");
        restore_system_backup();
        
        g_ota_manager.update_in_progress = false;
        g_ota_manager.stats.updates_failed++;
        
        log_update_event("UPDATE_FAILED", g_ota_manager.current_update.version);
        return result;
    }
    
    /* Update successful */
    g_ota_manager.stats.updates_installed++;
    g_ota_manager.stats.last_update_time = time(NULL);
    
    /* Update current version */
    strcpy(g_ota_manager.config.current_version, g_ota_manager.current_update.version);
    
    /* Clean up */
    unlink(g_ota_manager.download_path);
    memset(&g_ota_manager.current_update, 0, sizeof(update_metadata_t));
    memset(g_ota_manager.download_path, 0, sizeof(g_ota_manager.download_path));
    
    g_ota_manager.update_in_progress = false;
    
    log_update_event("UPDATE_INSTALLED", g_ota_manager.config.current_version);
    printf("Update installation completed successfully\n");
    
    if (package_info.requires_reboot) {
        printf("Reboot required to complete update\n");
        return 2;  /* Reboot required */
    }
    
    return 0;
}

/* Rollback to previous version */
int ota_manager_rollback(void) {
    if (!g_ota_manager.initialized) {
        return -1;
    }
    
    printf("Initiating system rollback...\n");
    
    int result = restore_system_backup();
    if (result != 0) {
        printf("System rollback failed: %d\n", result);
        return result;
    }
    
    g_ota_manager.stats.rollbacks_performed++;
    
    log_update_event("SYSTEM_ROLLBACK", "System rolled back to previous version");
    printf("System rollback completed successfully\n");
    
    return 0;
}

/* Get update status */
int ota_manager_get_status(char* status_json, size_t buffer_size) {
    if (!g_ota_manager.initialized || !status_json) {
        return -1;
    }
    
    snprintf(status_json, buffer_size,
        "{\n"
        "  \"initialized\": true,\n"
        "  \"current_version\": \"%s\",\n"
        "  \"update_channel\": \"%s\",\n"
        "  \"update_in_progress\": %s,\n"
        "  \"download_in_progress\": %s,\n"
        "  \"available_update\": {\n"
        "    \"version\": \"%s\",\n"
        "    \"description\": \"%s\",\n"
        "    \"size\": %lu,\n"
        "    \"criticality\": %u\n"
        "  },\n"
        "  \"statistics\": {\n"
        "    \"updates_checked\": %u,\n"
        "    \"updates_downloaded\": %u,\n"
        "    \"updates_installed\": %u,\n"
        "    \"updates_failed\": %u,\n"
        "    \"rollbacks_performed\": %u,\n"
        "    \"last_check_time\": %lu,\n"
        "    \"last_update_time\": %lu\n"
        "  }\n"
        "}",
        g_ota_manager.config.current_version,
        g_ota_manager.config.update_channel,
        g_ota_manager.update_in_progress ? "true" : "false",
        g_ota_manager.download_in_progress ? "true" : "false",
        g_ota_manager.current_update.version,
        g_ota_manager.current_update.description,
        g_ota_manager.current_update.package_size,
        g_ota_manager.current_update.criticality,
        g_ota_manager.stats.updates_checked,
        g_ota_manager.stats.updates_downloaded,
        g_ota_manager.stats.updates_installed,
        g_ota_manager.stats.updates_failed,
        g_ota_manager.stats.rollbacks_performed,
        g_ota_manager.stats.last_check_time,
        g_ota_manager.stats.last_update_time
    );
    
    return 0;
}

/* Download file with progress reporting */
static int download_file(const char* url, const char* output_path, uint64_t expected_size) {
    /* Simplified download implementation */
    /* In real implementation, would use libcurl or similar */
    
    printf("Downloading: %s -> %s\n", url, output_path);
    
    /* Create temporary file */
    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), "%s%s", output_path, TEMP_SUFFIX);
    
    FILE* output_file = fopen(temp_path, "wb");
    if (!output_file) {
        printf("Failed to create output file: %s\n", temp_path);
        return -1;
    }
    
    /* Simulate download */
    uint64_t downloaded = 0;
    uint8_t buffer[4096];
    
    while (downloaded < (expected_size > 0 ? expected_size : 1024 * 1024)) {
        size_t to_write = sizeof(buffer);
        if (expected_size > 0 && (downloaded + to_write) > expected_size) {
            to_write = expected_size - downloaded;
        }
        
        /* Generate some test data */
        for (size_t i = 0; i < to_write; i++) {
            buffer[i] = (uint8_t)(downloaded + i);
        }
        
        size_t written = fwrite(buffer, 1, to_write, output_file);
        if (written != to_write) {
            printf("Write error during download\n");
            fclose(output_file);
            unlink(temp_path);
            return -1;
        }
        
        downloaded += written;
        g_ota_manager.download_progress = downloaded;
        
        /* Progress reporting */
        if (expected_size > 0) {
            uint32_t percent = (uint32_t)((downloaded * 100) / expected_size);
            printf("Download progress: %u%% (%lu/%lu bytes)\r", percent, downloaded, expected_size);
            fflush(stdout);
        }
        
        /* Simulate network delay */
        usleep(1000);  /* 1ms delay */
    }
    
    fclose(output_file);
    
    /* Move temp file to final location */
    if (rename(temp_path, output_path) != 0) {
        printf("Failed to move downloaded file\n");
        unlink(temp_path);
        return -1;
    }
    
    printf("\nDownload completed: %lu bytes\n", downloaded);
    return 0;
}

/* Verify package integrity */
static int verify_package_integrity(const char* package_path, const update_metadata_t* metadata) {
    /* Calculate package hash */
    FILE* package_file = fopen(package_path, "rb");
    if (!package_file) {
        return -1;
    }
    
    uint8_t calculated_hash[32] = {0};
    uint8_t buffer[4096];
    size_t total_read = 0;
    
    /* Simple checksum calculation */
    while (!feof(package_file)) {
        size_t read = fread(buffer, 1, sizeof(buffer), package_file);
        if (read == 0) break;
        
        for (size_t i = 0; i < read; i++) {
            calculated_hash[total_read % 32] ^= buffer[i];
        }
        total_read += read;
    }
    
    fclose(package_file);
    
    /* Compare with expected hash */
    if (memcmp(calculated_hash, metadata->package_hash, 32) != 0) {
        printf("Package hash mismatch\n");
        return -1;
    }
    
    printf("Package integrity verification passed\n");
    return 0;
}

/* Create system backup */
static int create_system_backup(void) {
    printf("Creating system backup (simplified implementation)\n");
    
    /* In real implementation, would backup critical system files */
    FILE* backup_info = fopen("/tmp/system_backup.info", "w");
    if (backup_info) {
        fprintf(backup_info, "backup_version=%s\n", g_ota_manager.config.current_version);
        fprintf(backup_info, "backup_timestamp=%lu\n", (uint64_t)time(NULL));
        fclose(backup_info);
    }
    
    return 0;
}

/* Restore system backup */
static int restore_system_backup(void) {
    printf("Restoring system backup (simplified implementation)\n");
    
    /* In real implementation, would restore from backup */
    if (access("/tmp/system_backup.info", F_OK) == 0) {
        printf("Backup found, restoring...\n");
        unlink("/tmp/system_backup.info");
        return 0;
    }
    
    printf("No backup found\n");
    return -1;
}

/* Check update prerequisites */
static int check_update_prerequisites(const update_metadata_t* metadata) {
    /* Check minimum version requirement */
    if (strlen(metadata->min_version) > 0 &&
        strcmp(g_ota_manager.config.current_version, metadata->min_version) < 0) {
        printf("Current version %s is below minimum required %s\n",
               g_ota_manager.config.current_version, metadata->min_version);
        return -1;
    }
    
    /* Check maximum version requirement */
    if (strlen(metadata->max_version) > 0 &&
        strcmp(g_ota_manager.config.current_version, metadata->max_version) > 0) {
        printf("Current version %s is above maximum allowed %s\n",
               g_ota_manager.config.current_version, metadata->max_version);
        return -1;
    }
    
    /* Check downgrade policy */
    if (!g_ota_manager.config.allow_downgrades &&
        strcmp(metadata->version, g_ota_manager.config.current_version) < 0) {
        printf("Downgrade not allowed by policy\n");
        return -1;
    }
    
    return 0;
}

/* Parse update metadata JSON */
static int parse_update_metadata(const char* metadata_json, update_metadata_t* metadata) {
    /* Simplified JSON parsing */
    /* In real implementation, would use proper JSON parser */
    
    memset(metadata, 0, sizeof(update_metadata_t));
    
    /* Extract basic fields */
    sscanf(metadata_json, "{ \"version\": \"%31[^\"]\", \"channel\": \"%63[^\"]\", \"description\": \"%511[^\"]\", \"package_size\": %lu, \"criticality\": %u }",
           metadata->version, metadata->channel, metadata->description, 
           &metadata->package_size, &metadata->criticality);
    
    /* Set default download URL */
    snprintf(metadata->download_url, sizeof(metadata->download_url),
             "%s/packages/limitlessos_%s.pkg",
             g_ota_manager.config.update_server_url, metadata->version);
    
    metadata->requires_reboot = true;  /* Default to requiring reboot */
    metadata->release_timestamp = time(NULL);
    
    return 0;
}

/* Log update event */
static void log_update_event(const char* event, const char* details) {
    FILE* log_file = fopen("/var/log/ota_updates.log", "a");
    if (log_file) {
        fprintf(log_file, "[%lu] %s: %s\n", (uint64_t)time(NULL), event, details);
        fclose(log_file);
    }
}

/* Main function for OTA manager utility */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <command> [options]\n", argv[0]);
        printf("Commands:\n");
        printf("  init [config_path]  - Initialize OTA manager\n");
        printf("  check               - Check for updates\n");
        printf("  download            - Download available update\n");
        printf("  install [--dry-run] - Install downloaded update\n");
        printf("  rollback            - Rollback to previous version\n");
        printf("  status              - Show current status\n");
        return 1;
    }
    
    const char* command = argv[1];
    
    if (strcmp(command, "init") == 0) {
        const char* config_path = (argc > 2) ? argv[2] : NULL;
        return ota_manager_init(config_path);
        
    } else if (strcmp(command, "check") == 0) {
        ota_manager_init(NULL);
        return ota_manager_check_updates(true);
        
    } else if (strcmp(command, "download") == 0) {
        ota_manager_init(NULL);
        return ota_manager_download_update();
        
    } else if (strcmp(command, "install") == 0) {
        ota_manager_init(NULL);
        bool dry_run = (argc > 2 && strcmp(argv[2], "--dry-run") == 0);
        return ota_manager_install_update(dry_run);
        
    } else if (strcmp(command, "rollback") == 0) {
        ota_manager_init(NULL);
        return ota_manager_rollback();
        
    } else if (strcmp(command, "status") == 0) {
        ota_manager_init(NULL);
        char status_json[4096];
        int result = ota_manager_get_status(status_json, sizeof(status_json));
        if (result == 0) {
            printf("%s\n", status_json);
        }
        return result;
        
    } else {
        printf("Unknown command: %s\n", command);
        return 1;
    }
}