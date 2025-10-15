/*
 * LimitlessOS Modern Package Manager Implementation
 * Supports dependency resolution, delta updates, rollback, and signatures
 * Integrates with app store and system update APIs
 */

#include "package_manager.h"
#include <stdio.h>
#include <string.h>

#define MAX_PACKAGES 2048
static package_entry_t g_packages[MAX_PACKAGES];
static int g_package_count = 0;

int package_manager_install(const char* package_name) {
    printf("Installing package '%s'...\n", package_name);
    // Dependency resolution
    if (!package_manager_resolve_dependencies(package_name)) {
        printf("Dependency resolution failed.\n");
        return -1;
    }
    // Download package
    if (!package_manager_download(package_name)) {
        printf("Download failed.\n");
        return -2;
    }
    // Extract and install
    if (!package_manager_extract_and_install(package_name)) {
        printf("Extraction/installation failed.\n");
        return -3;
    }
    // Register package
    package_manager_register(package_name);
    printf("Package '%s' installed successfully.\n", package_name);
    return 0;
}

int package_manager_remove(const char* package_name) {
    printf("Removing package '%s'...\n", package_name);
    // Check if installed
    if (!package_manager_is_installed(package_name)) {
        printf("Package not installed.\n");
        return -1;
    }
    // Remove files
    if (!package_manager_remove_files(package_name)) {
        printf("File removal failed.\n");
        return -2;
    }
    // Unregister package
    package_manager_unregister(package_name);
    printf("Package '%s' removed successfully.\n", package_name);
    return 0;
}

int package_manager_update(const char* package_name) {
    printf("Updating package '%s'...\n", package_name);
    // Check if installed
    if (!package_manager_is_installed(package_name)) {
        printf("Package not installed.\n");
        return -1;
    }
    // Download update
    if (!package_manager_download_update(package_name)) {
        printf("Download update failed.\n");
        return -2;
    }
    // Apply delta update
    if (!package_manager_apply_delta_update(package_name)) {
        printf("Delta update failed.\n");
        // Rollback
        package_manager_rollback(package_name);
        return -3;
    }
    printf("Package '%s' updated successfully.\n", package_name);
    return 0;
}

int package_manager_verify_signature(const char* package_name) {
    printf("Verifying signature for package '%s'...\n", package_name);
    if (!package_manager_check_signature(package_name)) {
        printf("Signature verification failed.\n");
        return -1;
    }
    printf("Signature verified for package '%s'.\n", package_name);
    return 0;
}

int package_manager_list(package_entry_t* out, int max) {
    if (!out || max == 0) return 0;
    int n = (g_package_count < max) ? g_package_count : max;
    memcpy(out, g_packages, n * sizeof(package_entry_t));
    return n;
}

int package_manager_add(const char* package_name, const char* version) {
    if (!package_name || !version || g_package_count >= MAX_PACKAGES) return -1;
    strncpy(g_packages[g_package_count].name, package_name, sizeof(g_packages[g_package_count].name)-1);
    strncpy(g_packages[g_package_count].version, version, sizeof(g_packages[g_package_count].version)-1);
    g_package_count++;
    return 0;
}
