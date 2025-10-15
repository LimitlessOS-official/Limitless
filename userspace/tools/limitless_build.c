/*
 * limitless_build.c - LimitlessOS Comprehensive Build System
 * 
 * Advanced build system with dependency tracking, incremental builds,
 * cross-compilation support, and integrated testing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <libgen.h>

#define MAX_BUILD_TARGETS       1000
#define MAX_DEPENDENCIES        100
#define MAX_BUILD_RULES         500
#define MAX_PATH_LENGTH         1024
#define MAX_COMMAND_LENGTH      4096

/* Build target types */
typedef enum {
    TARGET_TYPE_EXECUTABLE,     /* Executable binary */
    TARGET_TYPE_LIBRARY,        /* Static library */
    TARGET_TYPE_SHARED_LIB,     /* Shared library */
    TARGET_TYPE_KERNEL_MODULE,  /* Kernel module */
    TARGET_TYPE_OBJECT,         /* Object file */
    TARGET_TYPE_CUSTOM          /* Custom target */
} target_type_t;

/* Build rule */
typedef struct build_rule {
    char pattern[256];          /* File pattern (e.g., "%.c") */
    char command[MAX_COMMAND_LENGTH]; /* Build command */
    char dependencies[512];     /* Dependency pattern */
} build_rule_t;

/* Build target */
typedef struct build_target {
    char name[256];             /* Target name */
    target_type_t type;         /* Target type */
    char output_file[MAX_PATH_LENGTH]; /* Output file path */
    
    /* Source files */
    char sources[100][MAX_PATH_LENGTH];
    uint32_t source_count;
    
    /* Dependencies */
    char dependencies[MAX_DEPENDENCIES][256];
    uint32_t dependency_count;
    
    /* Build configuration */
    char compiler[256];         /* Compiler to use */
    char linker[256];           /* Linker to use */
    char cflags[512];           /* Compiler flags */
    char ldflags[512];          /* Linker flags */
    char libs[512];             /* Libraries to link */
    
    /* Timestamps */
    time_t source_timestamp;    /* Latest source modification time */
    time_t output_timestamp;    /* Output file modification time */
    
    /* Build status */
    bool built;                 /* Successfully built */
    bool building;              /* Currently building */
    bool failed;                /* Build failed */
    
    /* Custom build command */
    char custom_command[MAX_COMMAND_LENGTH];
    
} build_target_t;

/* Build configuration */
typedef struct build_config {
    char project_name[256];
    char project_version[64];
    char build_directory[MAX_PATH_LENGTH];
    char source_directory[MAX_PATH_LENGTH];
    char output_directory[MAX_PATH_LENGTH];
    
    /* Toolchain configuration */
    char cc[256];               /* C compiler */
    char cxx[256];              /* C++ compiler */
    char ld[256];               /* Linker */
    char ar[256];               /* Archiver */
    char strip[256];            /* Strip utility */
    
    /* Global flags */
    char global_cflags[512];
    char global_cxxflags[512];
    char global_ldflags[512];
    
    /* Cross-compilation */
    char target_arch[64];       /* Target architecture */
    char cross_prefix[256];     /* Cross-compilation prefix */
    
    /* Build options */
    bool debug_build;           /* Debug build */
    bool optimize_build;        /* Optimization enabled */
    bool verbose_output;        /* Verbose build output */
    bool parallel_build;        /* Parallel build */
    uint32_t parallel_jobs;     /* Number of parallel jobs */
    bool clean_build;           /* Clean build */
    
    /* Testing configuration */
    bool run_tests;             /* Run tests after build */
    char test_command[512];     /* Test command */
    
} build_config_t;

/* Build system state */
typedef struct build_system {
    bool initialized;
    
    build_config_t config;
    
    /* Targets and rules */
    build_target_t targets[MAX_BUILD_TARGETS];
    uint32_t target_count;
    
    build_rule_t rules[MAX_BUILD_RULES];
    uint32_t rule_count;
    
    /* Build statistics */
    struct {
        uint32_t targets_built;
        uint32_t targets_failed;
        uint32_t files_compiled;
        uint64_t build_start_time;
        uint64_t build_end_time;
        uint64_t total_build_time;
    } stats;
    
    /* Dependency graph */
    uint32_t dependency_matrix[MAX_BUILD_TARGETS][MAX_BUILD_TARGETS];
    
} build_system_t;

static build_system_t g_build_system = {0};

/* Function prototypes */
static int load_build_config(const char* config_file);
static int save_build_config(const char* config_file);
static int scan_source_files(const char* directory);
static int add_build_target(const char* name, target_type_t type);
static int add_build_rule(const char* pattern, const char* command, const char* deps);
static build_target_t* find_target(const char* name);
static int build_target(build_target_t* target);
static int build_all_targets(void);
static bool is_target_up_to_date(build_target_t* target);
static time_t get_file_timestamp(const char* filepath);
static int execute_command(const char* command, bool verbose);
static int resolve_dependencies(void);
static int generate_dependency_graph(void);
static int clean_build_artifacts(void);
static void print_build_statistics(void);

/* Initialize build system */
int build_system_init(const char* project_root) {
    if (g_build_system.initialized) {
        return 0;
    }
    
    memset(&g_build_system, 0, sizeof(build_system_t));
    
    /* Set default configuration */
    build_config_t* config = &g_build_system.config;
    strcpy(config->project_name, "LimitlessOS");
    strcpy(config->project_version, "1.0.0");
    
    if (project_root) {
        strcpy(config->source_directory, project_root);
    } else {
        getcwd(config->source_directory, sizeof(config->source_directory));
    }
    
    snprintf(config->build_directory, sizeof(config->build_directory), 
             "%s/build", config->source_directory);
    snprintf(config->output_directory, sizeof(config->output_directory),
             "%s/build", config->source_directory);
    
    /* Set default toolchain */
    strcpy(config->cc, "gcc");
    strcpy(config->cxx, "g++");
    strcpy(config->ld, "ld");
    strcpy(config->ar, "ar");
    strcpy(config->strip, "strip");
    
    /* Set default flags */
    strcpy(config->global_cflags, "-Wall -Wextra -std=c11");
    strcpy(config->global_cxxflags, "-Wall -Wextra -std=c++17");
    strcpy(config->global_ldflags, "");
    
    /* Build options */
    config->debug_build = false;
    config->optimize_build = true;
    config->verbose_output = false;
    config->parallel_build = true;
    config->parallel_jobs = 4;
    config->clean_build = false;
    config->run_tests = false;
    
    g_build_system.initialized = true;
    
    printf("Build system initialized for project: %s\n", config->project_name);
    printf("Source directory: %s\n", config->source_directory);
    printf("Build directory: %s\n", config->build_directory);
    
    return 0;
}

/* Load build configuration */
static int load_build_config(const char* config_file) {
    FILE* file = fopen(config_file, "r");
    if (!file) {
        printf("Warning: Could not load build configuration from %s\n", config_file);
        return -1;
    }
    
    char line[1024];
    build_config_t* config = &g_build_system.config;
    
    printf("Loading build configuration: %s\n", config_file);
    
    while (fgets(line, sizeof(line), file)) {
        /* Remove newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Skip comments and empty lines */
        if (line[0] == '#' || line[0] == '\0') continue;
        
        char* key = strtok(line, "=");
        char* value = strtok(NULL, "");
        
        if (!key || !value) continue;
        
        /* Trim whitespace */
        while (*value == ' ' || *value == '\t') value++;
        
        if (strcmp(key, "PROJECT_NAME") == 0) {
            strncpy(config->project_name, value, sizeof(config->project_name) - 1);
        } else if (strcmp(key, "PROJECT_VERSION") == 0) {
            strncpy(config->project_version, value, sizeof(config->project_version) - 1);
        } else if (strcmp(key, "CC") == 0) {
            strncpy(config->cc, value, sizeof(config->cc) - 1);
        } else if (strcmp(key, "CXX") == 0) {
            strncpy(config->cxx, value, sizeof(config->cxx) - 1);
        } else if (strcmp(key, "CFLAGS") == 0) {
            strncpy(config->global_cflags, value, sizeof(config->global_cflags) - 1);
        } else if (strcmp(key, "CXXFLAGS") == 0) {
            strncpy(config->global_cxxflags, value, sizeof(config->global_cxxflags) - 1);
        } else if (strcmp(key, "LDFLAGS") == 0) {
            strncpy(config->global_ldflags, value, sizeof(config->global_ldflags) - 1);
        } else if (strcmp(key, "DEBUG") == 0) {
            config->debug_build = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        } else if (strcmp(key, "OPTIMIZE") == 0) {
            config->optimize_build = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        } else if (strcmp(key, "VERBOSE") == 0) {
            config->verbose_output = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        } else if (strcmp(key, "PARALLEL_JOBS") == 0) {
            config->parallel_jobs = (uint32_t)atoi(value);
        } else if (strcmp(key, "TARGET_ARCH") == 0) {
            strncpy(config->target_arch, value, sizeof(config->target_arch) - 1);
        } else if (strcmp(key, "CROSS_PREFIX") == 0) {
            strncpy(config->cross_prefix, value, sizeof(config->cross_prefix) - 1);
        }
    }
    
    fclose(file);
    
    /* Apply cross-compilation prefix */
    if (strlen(config->cross_prefix) > 0) {
        char temp[256];
        
        snprintf(temp, sizeof(temp), "%s%s", config->cross_prefix, config->cc);
        strcpy(config->cc, temp);
        
        snprintf(temp, sizeof(temp), "%s%s", config->cross_prefix, config->cxx);
        strcpy(config->cxx, temp);
        
        snprintf(temp, sizeof(temp), "%s%s", config->cross_prefix, config->ld);
        strcpy(config->ld, temp);
        
        snprintf(temp, sizeof(temp), "%s%s", config->cross_prefix, config->ar);
        strcpy(config->ar, temp);
        
        printf("Using cross-compilation toolchain: %s\n", config->cross_prefix);
    }
    
    /* Apply build mode flags */
    if (config->debug_build) {
        strcat(config->global_cflags, " -g -O0 -DDEBUG");
        strcat(config->global_cxxflags, " -g -O0 -DDEBUG");
    }
    
    if (config->optimize_build) {
        strcat(config->global_cflags, " -O2 -DNDEBUG");
        strcat(config->global_cxxflags, " -O2 -DNDEBUG");
    }
    
    printf("Configuration loaded: %s v%s\n", config->project_name, config->project_version);
    return 0;
}

/* Scan for source files */
static int scan_source_files(const char* directory) {
    DIR* dir = opendir(directory);
    if (!dir) {
        return -1;
    }
    
    struct dirent* entry;
    uint32_t files_found = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char* ext = strrchr(entry->d_name, '.');
            if (ext) {
                if (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 || 
                    strcmp(ext, ".cc") == 0 || strcmp(ext, ".cxx") == 0) {
                    
                    char full_path[MAX_PATH_LENGTH];
                    snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);
                    
                    printf("Found source file: %s\n", full_path);
                    files_found++;
                }
            }
        } else if (entry->d_type == DT_DIR && 
                   strcmp(entry->d_name, ".") != 0 && 
                   strcmp(entry->d_name, "..") != 0) {
            
            char subdir_path[MAX_PATH_LENGTH];
            snprintf(subdir_path, sizeof(subdir_path), "%s/%s", directory, entry->d_name);
            files_found += scan_source_files(subdir_path);
        }
    }
    
    closedir(dir);
    return files_found;
}

/* Add build target */
static int add_build_target(const char* name, target_type_t type) {
    if (g_build_system.target_count >= MAX_BUILD_TARGETS) {
        printf("Error: Maximum number of build targets reached\n");
        return -1;
    }
    
    build_target_t* target = &g_build_system.targets[g_build_system.target_count++];
    memset(target, 0, sizeof(build_target_t));
    
    strncpy(target->name, name, sizeof(target->name) - 1);
    target->type = type;
    
    /* Set default compiler and flags based on type */
    switch (type) {
        case TARGET_TYPE_EXECUTABLE:
            strcpy(target->compiler, g_build_system.config.cc);
            strcpy(target->cflags, g_build_system.config.global_cflags);
            snprintf(target->output_file, sizeof(target->output_file),
                     "%s/%s", g_build_system.config.output_directory, name);
            break;
            
        case TARGET_TYPE_LIBRARY:
            strcpy(target->compiler, g_build_system.config.ar);
            snprintf(target->output_file, sizeof(target->output_file),
                     "%s/lib%s.a", g_build_system.config.output_directory, name);
            break;
            
        case TARGET_TYPE_SHARED_LIB:
            strcpy(target->compiler, g_build_system.config.cc);
            strcpy(target->cflags, g_build_system.config.global_cflags);
            strcat(target->cflags, " -fPIC");
            strcpy(target->ldflags, "-shared");
            snprintf(target->output_file, sizeof(target->output_file),
                     "%s/lib%s.so", g_build_system.config.output_directory, name);
            break;
            
        case TARGET_TYPE_KERNEL_MODULE:
            strcpy(target->compiler, g_build_system.config.cc);
            strcpy(target->cflags, "-nostdlib -fno-builtin -fno-stack-protector");
            snprintf(target->output_file, sizeof(target->output_file),
                     "%s/%s.ko", g_build_system.config.output_directory, name);
            break;
            
        default:
            strcpy(target->compiler, g_build_system.config.cc);
            strcpy(target->cflags, g_build_system.config.global_cflags);
            break;
    }
    
    printf("Added build target: %s (type %d)\n", name, type);
    return 0;
}

/* Add build rule */
static int add_build_rule(const char* pattern, const char* command, const char* deps) {
    if (g_build_system.rule_count >= MAX_BUILD_RULES) {
        printf("Error: Maximum number of build rules reached\n");
        return -1;
    }
    
    build_rule_t* rule = &g_build_system.rules[g_build_system.rule_count++];
    
    strncpy(rule->pattern, pattern, sizeof(rule->pattern) - 1);
    strncpy(rule->command, command, sizeof(rule->command) - 1);
    strncpy(rule->dependencies, deps ? deps : "", sizeof(rule->dependencies) - 1);
    
    printf("Added build rule: %s -> %s\n", pattern, command);
    return 0;
}

/* Find target by name */
static build_target_t* find_target(const char* name) {
    for (uint32_t i = 0; i < g_build_system.target_count; i++) {
        if (strcmp(g_build_system.targets[i].name, name) == 0) {
            return &g_build_system.targets[i];
        }
    }
    return NULL;
}

/* Check if target is up to date */
static bool is_target_up_to_date(build_target_t* target) {
    /* Get output file timestamp */
    target->output_timestamp = get_file_timestamp(target->output_file);
    if (target->output_timestamp == 0) {
        return false;  /* Output doesn't exist */
    }
    
    /* Check source file timestamps */
    target->source_timestamp = 0;
    for (uint32_t i = 0; i < target->source_count; i++) {
        time_t src_time = get_file_timestamp(target->sources[i]);
        if (src_time > target->source_timestamp) {
            target->source_timestamp = src_time;
        }
    }
    
    /* Target is up to date if output is newer than all sources */
    return (target->output_timestamp >= target->source_timestamp);
}

/* Get file modification timestamp */
static time_t get_file_timestamp(const char* filepath) {
    struct stat st;
    if (stat(filepath, &st) == 0) {
        return st.st_mtime;
    }
    return 0;
}

/* Execute build command */
static int execute_command(const char* command, bool verbose) {
    if (verbose) {
        printf("Executing: %s\n", command);
    }
    
    int result = system(command);
    if (WIFEXITED(result)) {
        return WEXITSTATUS(result);
    }
    return -1;
}

/* Build single target */
static int build_target(build_target_t* target) {
    if (target->building) {
        printf("Error: Circular dependency detected for target %s\n", target->name);
        return -1;
    }
    
    if (target->built) {
        return 0;  /* Already built */
    }
    
    printf("Building target: %s\n", target->name);
    target->building = true;
    
    /* Build dependencies first */
    for (uint32_t i = 0; i < target->dependency_count; i++) {
        build_target_t* dep = find_target(target->dependencies[i]);
        if (dep) {
            int result = build_target(dep);
            if (result != 0) {
                printf("Error: Failed to build dependency %s for target %s\n",
                       target->dependencies[i], target->name);
                target->building = false;
                target->failed = true;
                return result;
            }
        }
    }
    
    /* Check if target needs rebuilding */
    if (is_target_up_to_date(target)) {
        printf("Target %s is up to date\n", target->name);
        target->built = true;
        target->building = false;
        return 0;
    }
    
    /* Create output directory */
    char* output_dir = dirname(strdup(target->output_file));
    char mkdir_cmd[512];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", output_dir);
    execute_command(mkdir_cmd, false);
    
    /* Build the target */
    char build_command[MAX_COMMAND_LENGTH];
    int result = -1;
    
    switch (target->type) {
        case TARGET_TYPE_EXECUTABLE:
        case TARGET_TYPE_SHARED_LIB:
        {
            /* Compile and link */
            if (target->source_count > 0) {
                /* Build object files first */
                char obj_files[2048] = "";
                
                for (uint32_t i = 0; i < target->source_count; i++) {
                    char obj_file[MAX_PATH_LENGTH];
                    char* base = basename(strdup(target->sources[i]));
                    char* ext = strrchr(base, '.');
                    if (ext) *ext = '\0';
                    
                    snprintf(obj_file, sizeof(obj_file), "%s/%s.o", 
                             g_build_system.config.build_directory, base);
                    
                    /* Compile source to object */
                    snprintf(build_command, sizeof(build_command),
                             "%s %s -c %s -o %s",
                             target->compiler, target->cflags, target->sources[i], obj_file);
                    
                    result = execute_command(build_command, g_build_system.config.verbose_output);
                    if (result != 0) {
                        printf("Error: Failed to compile %s\n", target->sources[i]);
                        break;
                    }
                    
                    strcat(obj_files, obj_file);
                    strcat(obj_files, " ");
                    g_build_system.stats.files_compiled++;
                }
                
                if (result == 0) {
                    /* Link objects */
                    snprintf(build_command, sizeof(build_command),
                             "%s %s %s %s -o %s %s",
                             target->compiler, target->cflags, obj_files,
                             target->ldflags, target->output_file, target->libs);
                    
                    result = execute_command(build_command, g_build_system.config.verbose_output);
                }
            }
            break;
        }
        
        case TARGET_TYPE_LIBRARY:
        {
            /* Create static library */
            if (target->source_count > 0) {
                char obj_files[2048] = "";
                
                /* Compile sources */
                for (uint32_t i = 0; i < target->source_count; i++) {
                    char obj_file[MAX_PATH_LENGTH];
                    char* base = basename(strdup(target->sources[i]));
                    char* ext = strrchr(base, '.');
                    if (ext) *ext = '\0';
                    
                    snprintf(obj_file, sizeof(obj_file), "%s/%s.o", 
                             g_build_system.config.build_directory, base);
                    
                    snprintf(build_command, sizeof(build_command),
                             "%s %s -c %s -o %s",
                             g_build_system.config.cc, target->cflags, target->sources[i], obj_file);
                    
                    result = execute_command(build_command, g_build_system.config.verbose_output);
                    if (result != 0) break;
                    
                    strcat(obj_files, obj_file);
                    strcat(obj_files, " ");
                    g_build_system.stats.files_compiled++;
                }
                
                if (result == 0) {
                    /* Create archive */
                    snprintf(build_command, sizeof(build_command),
                             "%s rcs %s %s", target->compiler, target->output_file, obj_files);
                    
                    result = execute_command(build_command, g_build_system.config.verbose_output);
                }
            }
            break;
        }
        
        case TARGET_TYPE_CUSTOM:
        {
            /* Execute custom command */
            if (strlen(target->custom_command) > 0) {
                result = execute_command(target->custom_command, g_build_system.config.verbose_output);
            }
            break;
        }
        
        default:
            printf("Error: Unsupported target type %d\n", target->type);
            result = -1;
            break;
    }
    
    target->building = false;
    
    if (result == 0) {
        target->built = true;
        g_build_system.stats.targets_built++;
        printf("Successfully built target: %s\n", target->name);
    } else {
        target->failed = true;
        g_build_system.stats.targets_failed++;
        printf("Failed to build target: %s\n", target->name);
    }
    
    return result;
}

/* Build all targets */
static int build_all_targets(void) {
    printf("Building all targets (%u targets)...\n", g_build_system.target_count);
    
    g_build_system.stats.build_start_time = time(NULL);
    
    int overall_result = 0;
    
    for (uint32_t i = 0; i < g_build_system.target_count; i++) {
        build_target_t* target = &g_build_system.targets[i];
        
        if (!target->built && !target->failed) {
            int result = build_target(target);
            if (result != 0) {
                overall_result = -1;
                printf("Build failed for target: %s\n", target->name);
                
                if (!g_build_system.config.verbose_output) {
                    break;  /* Stop on first failure unless verbose */
                }
            }
        }
    }
    
    g_build_system.stats.build_end_time = time(NULL);
    g_build_system.stats.total_build_time = g_build_system.stats.build_end_time - 
                                           g_build_system.stats.build_start_time;
    
    return overall_result;
}

/* Clean build artifacts */
static int clean_build_artifacts(void) {
    printf("Cleaning build artifacts...\n");
    
    char clean_command[512];
    snprintf(clean_command, sizeof(clean_command), "rm -rf %s/*", 
             g_build_system.config.build_directory);
    
    int result = execute_command(clean_command, g_build_system.config.verbose_output);
    
    if (result == 0) {
        printf("Build artifacts cleaned\n");
        
        /* Reset build status */
        for (uint32_t i = 0; i < g_build_system.target_count; i++) {
            g_build_system.targets[i].built = false;
            g_build_system.targets[i].failed = false;
            g_build_system.targets[i].building = false;
        }
    }
    
    return result;
}

/* Print build statistics */
static void print_build_statistics(void) {
    printf("\nBuild Statistics:\n");
    printf("=================\n");
    printf("Total targets: %u\n", g_build_system.target_count);
    printf("Targets built: %u\n", g_build_system.stats.targets_built);
    printf("Targets failed: %u\n", g_build_system.stats.targets_failed);
    printf("Files compiled: %u\n", g_build_system.stats.files_compiled);
    printf("Build time: %lu seconds\n", g_build_system.stats.total_build_time);
    
    if (g_build_system.stats.total_build_time > 0) {
        printf("Average compile rate: %.1f files/second\n",
               (double)g_build_system.stats.files_compiled / g_build_system.stats.total_build_time);
    }
}

/* Main build function */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <command> [options]\n", argv[0]);
        printf("Commands:\n");
        printf("  init [project_root]     - Initialize build system\n");
        printf("  config [config_file]    - Load build configuration\n");
        printf("  add-target <name> <type> - Add build target\n");
        printf("  add-source <target> <source> - Add source to target\n");
        printf("  build [target]          - Build target or all targets\n");
        printf("  clean                   - Clean build artifacts\n");
        printf("  scan [directory]        - Scan for source files\n");
        printf("  stats                   - Show build statistics\n");
        printf("Target types: executable, library, shared_lib, kernel_module, object, custom\n");
        return 1;
    }
    
    const char* command = argv[1];
    
    if (strcmp(command, "init") == 0) {
        const char* project_root = (argc > 2) ? argv[2] : NULL;
        return build_system_init(project_root);
        
    } else if (strcmp(command, "config") == 0) {
        build_system_init(NULL);
        const char* config_file = (argc > 2) ? argv[2] : "build.conf";
        return load_build_config(config_file);
        
    } else if (strcmp(command, "add-target") == 0) {
        if (argc < 4) {
            printf("Usage: add-target <name> <type>\n");
            return 1;
        }
        
        build_system_init(NULL);
        
        const char* name = argv[2];
        const char* type_str = argv[3];
        
        target_type_t type;
        if (strcmp(type_str, "executable") == 0) {
            type = TARGET_TYPE_EXECUTABLE;
        } else if (strcmp(type_str, "library") == 0) {
            type = TARGET_TYPE_LIBRARY;
        } else if (strcmp(type_str, "shared_lib") == 0) {
            type = TARGET_TYPE_SHARED_LIB;
        } else if (strcmp(type_str, "kernel_module") == 0) {
            type = TARGET_TYPE_KERNEL_MODULE;
        } else if (strcmp(type_str, "object") == 0) {
            type = TARGET_TYPE_OBJECT;
        } else if (strcmp(type_str, "custom") == 0) {
            type = TARGET_TYPE_CUSTOM;
        } else {
            printf("Unknown target type: %s\n", type_str);
            return 1;
        }
        
        return add_build_target(name, type);
        
    } else if (strcmp(command, "add-source") == 0) {
        if (argc < 4) {
            printf("Usage: add-source <target> <source_file>\n");
            return 1;
        }
        
        build_system_init(NULL);
        
        const char* target_name = argv[2];
        const char* source_file = argv[3];
        
        build_target_t* target = find_target(target_name);
        if (!target) {
            printf("Target not found: %s\n", target_name);
            return 1;
        }
        
        if (target->source_count >= 100) {
            printf("Too many source files for target %s\n", target_name);
            return 1;
        }
        
        strncpy(target->sources[target->source_count], source_file, MAX_PATH_LENGTH - 1);
        target->source_count++;
        
        printf("Added source %s to target %s\n", source_file, target_name);
        return 0;
        
    } else if (strcmp(command, "build") == 0) {
        build_system_init(NULL);
        
        if (argc > 2) {
            /* Build specific target */
            const char* target_name = argv[2];
            build_target_t* target = find_target(target_name);
            if (!target) {
                printf("Target not found: %s\n", target_name);
                return 1;
            }
            
            int result = build_target(target);
            print_build_statistics();
            return result;
        } else {
            /* Build all targets */
            int result = build_all_targets();
            print_build_statistics();
            return result;
        }
        
    } else if (strcmp(command, "clean") == 0) {
        build_system_init(NULL);
        return clean_build_artifacts();
        
    } else if (strcmp(command, "scan") == 0) {
        build_system_init(NULL);
        const char* directory = (argc > 2) ? argv[2] : g_build_system.config.source_directory;
        int files_found = scan_source_files(directory);
        printf("Found %d source files in %s\n", files_found, directory);
        return 0;
        
    } else if (strcmp(command, "stats") == 0) {
        build_system_init(NULL);
        print_build_statistics();
        return 0;
        
    } else {
        printf("Unknown command: %s\n", command);
        return 1;
    }
}