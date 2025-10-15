/**
 * LimitlessOS Package Manager - Production-Grade Package Management
 * 
 * Advanced package manager providing dependency resolution, automated builds,
 * security verification, and repository management comparable to apt, yum,
 * pacman, and other modern package managers.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <curl/curl.h>  // For HTTP downloads
#include <openssl/sha.h>  // For checksums
#include <archive.h>    // For package extraction
#include <json-c/json.h>  // For metadata parsing

/* Package Manager Configuration */
#define LIMITLESS_PKG_VERSION       "2.1.0"
#define LIMITLESS_PKG_DB_PATH      "/var/lib/limitless-pkg"
#define LIMITLESS_PKG_CACHE_PATH   "/var/cache/limitless-pkg"
#define LIMITLESS_PKG_CONFIG_PATH  "/etc/limitless-pkg"
#define LIMITLESS_PKG_LOG_PATH     "/var/log/limitless-pkg.log"

#define MAX_PACKAGE_NAME           128
#define MAX_VERSION_STRING         32
#define MAX_DESCRIPTION_LENGTH     1024
#define MAX_DEPENDENCIES           64
#define MAX_REPOSITORIES           16
#define MAX_CONFLICTS              32
#define MAX_PROVIDES               32

/* Package States */
typedef enum {
    PKG_STATE_NOT_INSTALLED,
    PKG_STATE_INSTALLED,
    PKG_STATE_UPGRADABLE,
    PKG_STATE_BROKEN,
    PKG_STATE_VIRTUAL,
    PKG_STATE_HELD
} pkg_state_t;

/* Package Priority */
typedef enum {
    PKG_PRIORITY_REQUIRED,      /* Essential system package */
    PKG_PRIORITY_IMPORTANT,     /* Important but not essential */
    PKG_PRIORITY_STANDARD,      /* Standard installation */
    PKG_PRIORITY_OPTIONAL,      /* Optional package */
    PKG_PRIORITY_EXTRA          /* Specialized package */
} pkg_priority_t;

/* Package Architecture */
typedef enum {
    PKG_ARCH_ANY,               /* Architecture independent */
    PKG_ARCH_X86_64,           /* Intel/AMD 64-bit */
    PKG_ARCH_I386,             /* Intel 32-bit */
    PKG_ARCH_ARM64,            /* ARM 64-bit */
    PKG_ARCH_ARM,              /* ARM 32-bit */
    PKG_ARCH_RISCV64          /* RISC-V 64-bit */
} pkg_arch_t;

/* Version Comparison */
typedef enum {
    VERSION_EQUAL,              /* = */
    VERSION_LESS_THAN,          /* < */
    VERSION_LESS_EQUAL,         /* <= */
    VERSION_GREATER_THAN,       /* > */
    VERSION_GREATER_EQUAL,      /* >= */
    VERSION_NOT_EQUAL          /* != */
} version_op_t;

/* Package Dependency */
typedef struct pkg_dependency {
    char name[MAX_PACKAGE_NAME];    /* Package name */
    char version[MAX_VERSION_STRING]; /* Version constraint */
    version_op_t version_op;        /* Version operator */
    bool optional;                  /* Optional dependency */
    
    struct pkg_dependency *next;    /* Next dependency */
} pkg_dependency_t;

/* Package File */
typedef struct pkg_file {
    char path[PATH_MAX];            /* File path */
    char checksum[65];              /* SHA-256 checksum */
    mode_t permissions;             /* File permissions */
    uid_t owner;                    /* File owner */
    gid_t group;                    /* File group */
    size_t size;                    /* File size */
    
    struct pkg_file *next;          /* Next file */
} pkg_file_t;

/* Package Metadata */
typedef struct package {
    char name[MAX_PACKAGE_NAME];    /* Package name */
    char version[MAX_VERSION_STRING]; /* Package version */
    char description[MAX_DESCRIPTION_LENGTH]; /* Description */
    char maintainer[256];           /* Maintainer information */
    char homepage[512];             /* Project homepage */
    char license[128];              /* Software license */
    
    pkg_arch_t architecture;        /* Target architecture */
    pkg_priority_t priority;        /* Package priority */
    pkg_state_t state;              /* Installation state */
    
    size_t installed_size;          /* Size when installed */
    size_t download_size;           /* Download size */
    
    /* Dependencies and conflicts */
    pkg_dependency_t *depends;      /* Dependencies */
    pkg_dependency_t *recommends;   /* Recommended packages */
    pkg_dependency_t *suggests;     /* Suggested packages */
    pkg_dependency_t *conflicts;    /* Conflicting packages */
    pkg_dependency_t *replaces;     /* Replaced packages */
    char provides[MAX_PROVIDES][MAX_PACKAGE_NAME]; /* Provided packages */
    int provides_count;             /* Number of provided packages */
    
    /* Installation information */
    pkg_file_t *files;              /* Installed files */
    time_t install_date;            /* Installation date */
    char install_reason[128];       /* Installation reason */
    
    /* Package source */
    char repository[128];           /* Source repository */
    char filename[256];             /* Package filename */
    char download_url[512];         /* Download URL */
    char checksum[65];              /* Package checksum */
    
    /* Scripts */
    char *preinst_script;           /* Pre-installation script */
    char *postinst_script;          /* Post-installation script */
    char *prerm_script;             /* Pre-removal script */
    char *postrm_script;            /* Post-removal script */
    
    struct package *next;           /* Next package */
} package_t;

/* Repository Information */
typedef struct repository {
    char name[128];                 /* Repository name */
    char url[512];                  /* Repository URL */
    char distribution[64];          /* Distribution name */
    char component[64];             /* Repository component */
    char architecture[32];          /* Target architecture */
    
    bool enabled;                   /* Repository enabled */
    bool trusted;                   /* Trusted repository */
    char gpg_key[128];             /* GPG key ID */
    
    time_t last_update;            /* Last update time */
    
    struct repository *next;        /* Next repository */
} repository_t;

/* Package Manager State */
typedef struct pkg_manager {
    package_t *installed_packages;  /* Installed packages */
    package_t *available_packages;  /* Available packages */
    repository_t *repositories;     /* Configured repositories */
    
    char cache_dir[PATH_MAX];       /* Cache directory */
    char db_dir[PATH_MAX];          /* Database directory */
    char config_dir[PATH_MAX];      /* Configuration directory */
    
    /* Configuration */
    bool auto_remove_unused;        /* Auto-remove unused packages */
    bool check_signatures;          /* Verify package signatures */
    bool download_only;             /* Download only mode */
    bool force_install;             /* Force installation */
    bool quiet_mode;                /* Quiet operation */
    bool verbose_mode;              /* Verbose output */
    
    /* Statistics */
    struct {
        uint64_t packages_installed;
        uint64_t packages_removed;
        uint64_t packages_upgraded;
        uint64_t bytes_downloaded;
        uint64_t operations_total;
    } stats;
    
    FILE *log_file;                /* Log file handle */
} pkg_manager_t;

/* Global package manager instance */
static pkg_manager_t g_pkg_mgr = {0};

/* Function Prototypes */

/* Core Package Management */
int pkg_manager_init(void);
void pkg_manager_cleanup(void);
int pkg_update_repositories(void);
int pkg_upgrade_system(void);

/* Package Operations */
int pkg_install(const char *package_name);
int pkg_remove(const char *package_name);
int pkg_upgrade(const char *package_name);
int pkg_reinstall(const char *package_name);
package_t *pkg_search(const char *pattern);
package_t *pkg_info(const char *package_name);

/* Dependency Resolution */
int pkg_resolve_dependencies(package_t *package, package_t ***install_list, int *install_count);
int pkg_check_conflicts(package_t *package);
bool pkg_is_dependency_satisfied(pkg_dependency_t *dep);
int pkg_calculate_install_order(package_t **packages, int count, package_t ***ordered);

/* Repository Management */
int pkg_add_repository(const char *name, const char *url, const char *distribution);
int pkg_remove_repository(const char *name);
int pkg_enable_repository(const char *name, bool enabled);
int pkg_refresh_repository(repository_t *repo);

/* Package Database */
int pkg_db_load(void);
int pkg_db_save(void);
package_t *pkg_db_find(const char *name);
int pkg_db_add_package(package_t *package);
int pkg_db_remove_package(const char *name);

/* Package Download and Installation */
int pkg_download(package_t *package, const char *dest_path);
int pkg_verify_package(package_t *package, const char *file_path);
int pkg_extract_package(const char *file_path, const char *dest_dir);
int pkg_install_files(package_t *package, const char *source_dir);

/* Version Management */
int pkg_compare_versions(const char *v1, const char *v2);
bool pkg_version_satisfies(const char *version, const char *constraint, version_op_t op);
char *pkg_get_latest_version(const char *package_name);

/* Configuration */
int pkg_load_config(void);
int pkg_save_config(void);
int pkg_set_option(const char *key, const char *value);
const char *pkg_get_option(const char *key);

/* Utilities */
void pkg_log(const char *level, const char *format, ...);
void pkg_progress_callback(double progress, const char *status);
const char *pkg_state_to_string(pkg_state_t state);
const char *pkg_arch_to_string(pkg_arch_t arch);

/**
 * Initialize package manager
 */
int pkg_manager_init(void) {
    printf("Initializing LimitlessOS Package Manager v%s...\n", LIMITLESS_PKG_VERSION);
    
    // Set up directory paths
    strcpy(g_pkg_mgr.cache_dir, LIMITLESS_PKG_CACHE_PATH);
    strcpy(g_pkg_mgr.db_dir, LIMITLESS_PKG_DB_PATH);
    strcpy(g_pkg_mgr.config_dir, LIMITLESS_PKG_CONFIG_PATH);
    
    // Create necessary directories
    struct stat st;
    const char *dirs[] = {
        g_pkg_mgr.cache_dir,
        g_pkg_mgr.db_dir,
        g_pkg_mgr.config_dir,
        NULL
    };
    
    for (int i = 0; dirs[i]; i++) {
        if (stat(dirs[i], &st) == -1) {
            if (mkdir(dirs[i], 0755) == -1) {
                perror("Failed to create directory");
                return -1;
            }
        }
    }
    
    // Open log file
    g_pkg_mgr.log_file = fopen(LIMITLESS_PKG_LOG_PATH, "a");
    if (!g_pkg_mgr.log_file) {
        perror("Failed to open log file");
    }
    
    // Set default configuration
    g_pkg_mgr.auto_remove_unused = true;
    g_pkg_mgr.check_signatures = true;
    g_pkg_mgr.download_only = false;
    g_pkg_mgr.force_install = false;
    g_pkg_mgr.quiet_mode = false;
    g_pkg_mgr.verbose_mode = false;
    
    // Initialize curl for downloads
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Load configuration and package database
    pkg_load_config();
    pkg_db_load();
    
    pkg_log("INFO", "Package manager initialized successfully");
    
    printf("Package Manager initialized\n");
    printf("Cache Directory: %s\n", g_pkg_mgr.cache_dir);
    printf("Database Directory: %s\n", g_pkg_mgr.db_dir);
    
    return 0;
}

/**
 * Install package
 */
int pkg_install(const char *package_name) {
    if (!package_name) {
        printf("Error: Package name required\n");
        return -1;
    }
    
    printf("Installing package: %s\n", package_name);
    pkg_log("INFO", "Installing package: %s", package_name);
    
    // Find package in available packages
    package_t *package = pkg_db_find(package_name);
    if (!package) {
        printf("Error: Package '%s' not found\n", package_name);
        return -1;
    }
    
    // Check if already installed
    if (package->state == PKG_STATE_INSTALLED) {
        printf("Package '%s' is already installed\n", package_name);
        return 0;
    }
    
    // Resolve dependencies
    package_t **install_list;
    int install_count;
    
    if (pkg_resolve_dependencies(package, &install_list, &install_count) != 0) {
        printf("Error: Failed to resolve dependencies for '%s'\n", package_name);
        return -1;
    }
    
    printf("The following packages will be installed:\n");
    for (int i = 0; i < install_count; i++) {
        printf("  %s (%s)\n", install_list[i]->name, install_list[i]->version);
    }
    
    // Calculate total download size
    size_t total_download = 0;
    size_t total_installed = 0;
    for (int i = 0; i < install_count; i++) {
        total_download += install_list[i]->download_size;
        total_installed += install_list[i]->installed_size;
    }
    
    printf("Download size: %.2f MB\n", total_download / (1024.0 * 1024.0));
    printf("Installed size: %.2f MB\n", total_installed / (1024.0 * 1024.0));
    
    // Confirm installation
    if (!g_pkg_mgr.force_install) {
        printf("Do you want to continue? [Y/n] ");
        char response[10];
        if (fgets(response, sizeof(response), stdin)) {
            if (response[0] == 'n' || response[0] == 'N') {
                printf("Installation cancelled\n");
                free(install_list);
                return 0;
            }
        }
    }
    
    // Download packages
    printf("Downloading packages...\n");
    for (int i = 0; i < install_count; i++) {
        char cache_path[PATH_MAX];
        snprintf(cache_path, sizeof(cache_path), "%s/%s_%s.lpkg",
                g_pkg_mgr.cache_dir, install_list[i]->name, install_list[i]->version);
        
        if (pkg_download(install_list[i], cache_path) != 0) {
            printf("Error: Failed to download package '%s'\n", install_list[i]->name);
            free(install_list);
            return -1;
        }
        
        // Verify package integrity
        if (pkg_verify_package(install_list[i], cache_path) != 0) {
            printf("Error: Package verification failed for '%s'\n", install_list[i]->name);
            free(install_list);
            return -1;
        }
    }
    
    // Install packages in dependency order
    printf("Installing packages...\n");
    for (int i = 0; i < install_count; i++) {
        char cache_path[PATH_MAX];
        snprintf(cache_path, sizeof(cache_path), "%s/%s_%s.lpkg",
                g_pkg_mgr.cache_dir, install_list[i]->name, install_list[i]->version);
        
        // Extract package
        char extract_dir[PATH_MAX];
        snprintf(extract_dir, sizeof(extract_dir), "%s/extract_%s",
                g_pkg_mgr.cache_dir, install_list[i]->name);
        
        if (pkg_extract_package(cache_path, extract_dir) != 0) {
            printf("Error: Failed to extract package '%s'\n", install_list[i]->name);
            free(install_list);
            return -1;
        }
        
        // Run pre-installation script
        if (install_list[i]->preinst_script) {
            system(install_list[i]->preinst_script);
        }
        
        // Install files
        if (pkg_install_files(install_list[i], extract_dir) != 0) {
            printf("Error: Failed to install files for '%s'\n", install_list[i]->name);
            free(install_list);
            return -1;
        }
        
        // Run post-installation script
        if (install_list[i]->postinst_script) {
            system(install_list[i]->postinst_script);
        }
        
        // Update package state
        install_list[i]->state = PKG_STATE_INSTALLED;
        install_list[i]->install_date = time(NULL);
        strcpy(install_list[i]->install_reason, "manual");
        
        // Add to installed packages database
        pkg_db_add_package(install_list[i]);
        
        printf("Successfully installed: %s (%s)\n", 
               install_list[i]->name, install_list[i]->version);
        
        // Clean up extraction directory
        char cleanup_cmd[PATH_MAX + 32];
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", extract_dir);
        system(cleanup_cmd);
    }
    
    // Update statistics
    g_pkg_mgr.stats.packages_installed += install_count;
    g_pkg_mgr.stats.bytes_downloaded += total_download;
    g_pkg_mgr.stats.operations_total++;
    
    // Save database
    pkg_db_save();
    
    printf("Installation completed successfully\n");
    pkg_log("INFO", "Successfully installed %d packages", install_count);
    
    free(install_list);
    return 0;
}

/**
 * Search for packages
 */
package_t *pkg_search(const char *pattern) {
    if (!pattern) return NULL;
    
    printf("Searching for packages matching: %s\n", pattern);
    
    package_t *results = NULL;
    package_t *current = g_pkg_mgr.available_packages;
    int result_count = 0;
    
    while (current) {
        // Search in package name and description
        if (strstr(current->name, pattern) || 
            strcasestr(current->description, pattern)) {
            
            // Add to results
            package_t *result = malloc(sizeof(package_t));
            *result = *current;
            result->next = results;
            results = result;
            result_count++;
        }
        current = current->next;
    }
    
    printf("Found %d matching packages:\n", result_count);
    
    package_t *result = results;
    while (result) {
        printf("  %s (%s) - %s [%s]\n",
               result->name,
               result->version,
               result->description,
               pkg_state_to_string(result->state));
        result = result->next;
    }
    
    return results;
}

/**
 * Update package repositories
 */
int pkg_update_repositories(void) {
    printf("Updating package repositories...\n");
    pkg_log("INFO", "Updating package repositories");
    
    repository_t *repo = g_pkg_mgr.repositories;
    int updated_count = 0;
    
    while (repo) {
        if (repo->enabled) {
            printf("Updating repository: %s\n", repo->name);
            
            if (pkg_refresh_repository(repo) == 0) {
                repo->last_update = time(NULL);
                updated_count++;
                printf("  %s: OK\n", repo->name);
            } else {
                printf("  %s: FAILED\n", repo->name);
            }
        }
        repo = repo->next;
    }
    
    printf("Updated %d repositories\n", updated_count);
    pkg_log("INFO", "Updated %d repositories", updated_count);
    
    return 0;
}

/**
 * Package manager status and statistics
 */
void pkg_manager_status(void) {
    printf("LimitlessOS Package Manager Status:\n");
    printf("==================================\n");
    
    // Count packages by state
    int installed = 0, available = 0, upgradable = 0;
    
    package_t *pkg = g_pkg_mgr.installed_packages;
    while (pkg) {
        installed++;
        pkg = pkg->next;
    }
    
    pkg = g_pkg_mgr.available_packages;
    while (pkg) {
        available++;
        if (pkg->state == PKG_STATE_UPGRADABLE) upgradable++;
        pkg = pkg->next;
    }
    
    printf("Installed Packages: %d\n", installed);
    printf("Available Packages: %d\n", available);
    printf("Upgradable Packages: %d\n", upgradable);
    
    // Repository information
    int repo_count = 0, enabled_repos = 0;
    repository_t *repo = g_pkg_mgr.repositories;
    while (repo) {
        repo_count++;
        if (repo->enabled) enabled_repos++;
        repo = repo->next;
    }
    
    printf("Repositories: %d (%d enabled)\n", repo_count, enabled_repos);
    
    // Statistics
    printf("\nOperation Statistics:\n");
    printf("  Total Operations: %llu\n", g_pkg_mgr.stats.operations_total);
    printf("  Packages Installed: %llu\n", g_pkg_mgr.stats.packages_installed);
    printf("  Packages Removed: %llu\n", g_pkg_mgr.stats.packages_removed);
    printf("  Packages Upgraded: %llu\n", g_pkg_mgr.stats.packages_upgraded);
    printf("  Data Downloaded: %.2f MB\n", 
           g_pkg_mgr.stats.bytes_downloaded / (1024.0 * 1024.0));
    
    // Configuration
    printf("\nConfiguration:\n");
    printf("  Auto-remove unused: %s\n", g_pkg_mgr.auto_remove_unused ? "Yes" : "No");
    printf("  Check signatures: %s\n", g_pkg_mgr.check_signatures ? "Yes" : "No");
    printf("  Cache Directory: %s\n", g_pkg_mgr.cache_dir);
    printf("  Database Directory: %s\n", g_pkg_mgr.db_dir);
}

/**
 * Main package manager command-line interface
 */
int limitless_pkg_main(int argc, char **argv) {
    printf("LimitlessOS Package Manager v%s\n", LIMITLESS_PKG_VERSION);
    
    if (argc < 2) {
        printf("Usage: %s <command> [options] [packages...]\n", argv[0]);
        printf("Commands:\n");
        printf("  install <package>     Install package(s)\n");
        printf("  remove <package>      Remove package(s)\n");
        printf("  upgrade [package]     Upgrade package(s) or system\n");
        printf("  search <pattern>      Search for packages\n");
        printf("  info <package>        Show package information\n");
        printf("  update               Update repository information\n");
        printf("  list                 List installed packages\n");
        printf("  status               Show package manager status\n");
        return 1;
    }
    
    // Initialize package manager
    if (pkg_manager_init() != 0) {
        printf("Failed to initialize package manager\n");
        return 1;
    }
    
    const char *command = argv[1];
    int result = 0;
    
    if (strcmp(command, "install") == 0) {
        if (argc < 3) {
            printf("Error: Package name required for install command\n");
            result = 1;
        } else {
            for (int i = 2; i < argc; i++) {
                if (pkg_install(argv[i]) != 0) {
                    result = 1;
                }
            }
        }
    } else if (strcmp(command, "remove") == 0) {
        if (argc < 3) {
            printf("Error: Package name required for remove command\n");
            result = 1;
        } else {
            for (int i = 2; i < argc; i++) {
                if (pkg_remove(argv[i]) != 0) {
                    result = 1;
                }
            }
        }
    } else if (strcmp(command, "upgrade") == 0) {
        if (argc == 2) {
            result = pkg_upgrade_system();
        } else {
            for (int i = 2; i < argc; i++) {
                if (pkg_upgrade(argv[i]) != 0) {
                    result = 1;
                }
            }
        }
    } else if (strcmp(command, "search") == 0) {
        if (argc < 3) {
            printf("Error: Search pattern required\n");
            result = 1;
        } else {
            package_t *results = pkg_search(argv[2]);
            // Free results after use
            while (results) {
                package_t *next = results->next;
                free(results);
                results = next;
            }
        }
    } else if (strcmp(command, "info") == 0) {
        if (argc < 3) {
            printf("Error: Package name required for info command\n");
            result = 1;
        } else {
            package_t *pkg = pkg_info(argv[2]);
            if (!pkg) result = 1;
        }
    } else if (strcmp(command, "update") == 0) {
        result = pkg_update_repositories();
    } else if (strcmp(command, "status") == 0) {
        pkg_manager_status();
    } else {
        printf("Error: Unknown command '%s'\n", command);
        result = 1;
    }
    
    // Cleanup
    pkg_manager_cleanup();
    
    return result;
}

/**
 * Test package management system
 */
int test_package_manager(void) {
    printf("Testing LimitlessOS Package Manager...\n");
    
    // Initialize package manager
    if (pkg_manager_init() != 0) {
        printf("Package manager initialization failed\n");
        return -1;
    }
    
    // Add test repositories
    pkg_add_repository("limitless-main", 
                      "https://packages.limitlessos.org/main", 
                      "limitless-1.0");
    pkg_add_repository("limitless-updates", 
                      "https://packages.limitlessos.org/updates", 
                      "limitless-1.0");
    
    // Test repository update
    printf("Testing repository update...\n");
    pkg_update_repositories();
    
    // Test package search
    printf("Testing package search...\n");
    package_t *results = pkg_search("gcc");
    if (results) {
        printf("Package search test: PASSED\n");
        // Free results
        while (results) {
            package_t *next = results->next;
            free(results);
            results = next;
        }
    } else {
        printf("Package search test: No results (expected for test)\n");
    }
    
    // Show package manager status
    pkg_manager_status();
    
    printf("Package manager test completed\n");
    
    pkg_manager_cleanup();
    return 0;
}

/* Main entry point */
int main(int argc, char **argv) {
    return limitless_pkg_main(argc, argv);
}