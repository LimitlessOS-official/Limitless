/*
 * package_manager.h - LimitlessOS Package Management System
 * 
 * Provides complete package format, dependency resolution, signature verification,
 * and installation management for the LimitlessOS ecosystem.
 */

#ifndef _PACKAGE_MANAGER_H
#define _PACKAGE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#define PACKAGE_MAGIC       0x4C4D5450  /* "LMTP" - LimitlessOS Package */
#define PACKAGE_VERSION     1
#define MAX_PACKAGE_NAME    128
#define MAX_VERSION_STRING  64
#define MAX_DESCRIPTION     512
#define MAX_DEPENDENCIES    32
#define MAX_FILES           1024
#define SIGNATURE_SIZE      64
#define HASH_SIZE          32

/* Package format structures */
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t header_size;
    uint32_t data_size;
    uint32_t file_count;
    uint32_t dependency_count;
    uint8_t  signature[SIGNATURE_SIZE];
    uint8_t  content_hash[HASH_SIZE];
} package_header_t;

typedef struct {
    char name[MAX_PACKAGE_NAME];
    char version[MAX_VERSION_STRING];
    char description[MAX_DESCRIPTION];
    char maintainer[128];
    char homepage[256];
    uint64_t installed_size;
    uint64_t download_size;
    uint32_t flags;
    uint32_t build_date;
} package_metadata_t;

typedef struct {
    char name[MAX_PACKAGE_NAME];
    char version_constraint[MAX_VERSION_STRING];  /* ">=1.0.0", "~2.1", etc */
    uint32_t flags;
} package_dependency_t;

typedef struct {
    char path[512];
    uint64_t size;
    uint64_t offset;
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint8_t  hash[HASH_SIZE];
    uint32_t flags;
} package_file_entry_t;

typedef struct {
    package_header_t header;
    package_metadata_t metadata;
    package_dependency_t dependencies[MAX_DEPENDENCIES];
    package_file_entry_t files[MAX_FILES];
    void* data_section;
} package_t;

/* Package repository structures */
typedef struct {
    char name[MAX_PACKAGE_NAME];
    char version[MAX_VERSION_STRING];
    char arch[32];
    uint64_t size;
    uint8_t hash[HASH_SIZE];
    char url[512];
    uint32_t timestamp;
} repo_package_entry_t;

typedef struct {
    char name[128];
    char url[512];
    char description[256];
    uint32_t package_count;
    repo_package_entry_t* packages;
    uint8_t public_key[32];  /* Ed25519 public key */
    bool enabled;
} package_repository_t;

/* Package database structures */
typedef struct installed_package {
    package_metadata_t metadata;
    uint64_t install_date;
    uint32_t install_reason;  /* AUTO, MANUAL, DEPENDENCY */
    struct installed_package* next;
} installed_package_t;

/* Dependency resolution structures */
typedef struct dep_node {
    char name[MAX_PACKAGE_NAME];
    char version[MAX_VERSION_STRING];
    struct dep_node** dependencies;
    uint32_t dep_count;
    uint32_t state;  /* UNVISITED, VISITING, VISITED */
    struct dep_node* next;
} dependency_node_t;

typedef struct {
    dependency_node_t* nodes;
    uint32_t node_count;
    char** install_order;
    uint32_t install_count;
} dependency_graph_t;

/* Package manager operations */
typedef enum {
    PKG_OP_INSTALL,
    PKG_OP_REMOVE,
    PKG_OP_UPDATE,
    PKG_OP_SEARCH,
    PKG_OP_LIST,
    PKG_OP_INFO,
    PKG_OP_VERIFY
} package_operation_t;

/* Function prototypes */

/* Package format operations */
status_t package_create(const char* manifest_file, const char* output_file);
status_t package_extract(const char* package_file, const char* dest_dir);
status_t package_verify_signature(package_t* pkg, const uint8_t* public_key);
status_t package_validate_integrity(package_t* pkg);

/* Repository management */
status_t repo_add(const char* name, const char* url, const uint8_t* public_key);
status_t repo_remove(const char* name);
status_t repo_update(const char* name);
status_t repo_sync_all(void);
package_t* repo_find_package(const char* name, const char* version);

/* Dependency resolution */
dependency_graph_t* resolve_dependencies(const char* package_name);
status_t check_conflicts(dependency_graph_t* graph);
char** get_install_order(dependency_graph_t* graph);
void free_dependency_graph(dependency_graph_t* graph);

/* Installation management */
status_t install_package(const char* package_name);
status_t remove_package(const char* package_name, bool remove_deps);
status_t update_package(const char* package_name);
status_t update_system(void);

/* Package database */
status_t pkgdb_init(void);
status_t pkgdb_add_installed(package_t* pkg);
status_t pkgdb_remove_installed(const char* name);
installed_package_t* pkgdb_find_installed(const char* name);
installed_package_t* pkgdb_list_installed(void);
status_t pkgdb_save(void);

/* Utility functions */
bool version_compare(const char* v1, const char* v2, const char* constraint);
bool package_installed(const char* name);
uint64_t calculate_download_size(dependency_graph_t* graph);
void print_package_info(package_t* pkg);

#endif /* _PACKAGE_MANAGER_H */