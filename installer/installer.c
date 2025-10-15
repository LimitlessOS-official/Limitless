/* LimitlessOS Installer - Integration Bridge */

#include "installer.h"
#include "intelligent_installer.h"
#include <stdio.h>
#include <string.h>

// Forward declarations for installer steps
int installer_validate_disk(const char* device_path);
int installer_partition_disk(const char* device_path, partition_scheme_t scheme);
int installer_copy_os_files(const char* device_path);
int installer_configure_bootloader(const char* device_path);
int installer_finalize_install(const char* device_path);

status_t installer_init(void) {
    // Initialize both basic and intelligent installer subsystems
    status_t status_basic = STATUS_OK;
    status_t status_ai = installer_init();
    if (status_ai != STATUS_OK) {
        printf("Warning: Intelligent installer failed to initialize (status=%d)\n", status_ai);
        return status_ai;
    }
    return status_basic;
}

int installer_install_system(install_config_t* config) {
    // If advanced/enterprise mode, delegate to intelligent installer
    if (config->mode == INSTALL_MODE_ADVANCED) {
        installation_config_t ai_config;
        memset(&ai_config, 0, sizeof(ai_config));
        // Map basic config to advanced config
        strncpy(ai_config.installation_name, config->hostname, sizeof(ai_config.installation_name)-1);
        strncpy(ai_config.target_architecture, "x86_64", sizeof(ai_config.target_architecture)-1);
        // Map disk info
        storage_device_t disk = {0};
        strncpy(disk.device_path, config->target_disk.device_path, sizeof(disk.device_path)-1);
        strncpy(disk.model, config->target_disk.model, sizeof(disk.model)-1);
        disk.capacity_bytes = config->target_disk.size_bytes;
        disk.type = config->target_disk.is_ssd ? STORAGE_TYPE_SSD : STORAGE_TYPE_HDD;
        ai_config.target_device = &disk;
        // Map partition layout
        partition_t part = {0};
        part.filesystem = FS_TYPE_EXT4;
        part.size_bytes = config->layout.root_size_mb * 1024 * 1024;
        ai_config.partitions = &part;
        ai_config.partition_count = 1;
        // Map user
        user_profile_t user = {0};
        strncpy(user.name, config->user.username, sizeof(user.name)-1);
        strncpy(user.full_name, config->user.full_name, sizeof(user.full_name)-1);
        ai_config.primary_user = &user;
        ai_config.user_count = 1;
        // Map system config
        strncpy(ai_config.hostname, config->hostname, sizeof(ai_config.hostname)-1);
        strncpy(ai_config.root_password_hash, config->user.password_hash, sizeof(ai_config.root_password_hash)-1);
        ai_config.enable_root_login = config->user.is_admin;
        ai_config.enable_ssh = true;
        ai_config.enable_firewall = true;
        // Map privacy and security
        ai_config.enable_full_disk_encryption = config->enable_encryption;
        ai_config.privacy_settings.ai_system_enabled = false;
        // Add more mappings as needed
        return installer_prepare_installation(&ai_config);
    }
    // Otherwise, perform basic install
    printf("Performing basic install for disk: %s\n", config->target_disk.device_path);
    // Step 1: Validate disk
    if (!installer_validate_disk(config->target_disk.device_path)) {
        printf("Disk validation failed.\n");
        return -1;
    }
    // Step 2: Partition disk
    if (!installer_partition_disk(config->target_disk.device_path, config->layout.scheme)) {
        printf("Partitioning failed.\n");
        return -2;
    }
    // Step 3: Copy OS files
    if (!installer_copy_os_files(config->target_disk.device_path)) {
        printf("File copy failed.\n");
        return -3;
    }
    // Step 4: Configure bootloader
    if (!installer_configure_bootloader(config->target_disk.device_path)) {
        printf("Bootloader configuration failed.\n");
        return -4;
    }
    // Step 5: Finalize
    if (!installer_finalize_install(config->target_disk.device_path)) {
        printf("Finalization failed.\n");
        return -5;
    }
    printf("Basic install completed successfully.\n");
    return 0;
}

// Other installer functions can be similarly bridged or implemented
