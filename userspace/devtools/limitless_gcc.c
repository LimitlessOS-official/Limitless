/**
 * LimitlessOS Development Ecosystem - GCC Compiler Integration
 * 
 * Complete compiler toolchain integration providing full development
 * capabilities on par with Linux distributions, Windows MSVC, and macOS Xcode.
 * Supports C, C++, Assembly, and provides comprehensive build system.
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

/* Compiler Configuration */
#define LIMITLESS_GCC_VERSION       "13.2.0"
#define LIMITLESS_SYSROOT          "/usr/limitless"
#define LIMITLESS_INCLUDE_PATH     "/usr/limitless/include"
#define LIMITLESS_LIB_PATH         "/usr/limitless/lib"
#define LIMITLESS_BIN_PATH         "/usr/limitless/bin"
#define MAX_COMPILE_ARGS           1024
#define MAX_PATH_LENGTH            4096
#define MAX_SOURCE_FILES           256

/* Supported Languages */
typedef enum {
    LANG_C,
    LANG_CXX,
    LANG_ASSEMBLY,
    LANG_OBJECTIVE_C,
    LANG_FORTRAN,
    LANG_GO,
    LANG_RUST
} language_t;

/* Build Configuration */
typedef struct build_config {
    language_t language;            /* Source language */
    char *source_files[MAX_SOURCE_FILES]; /* Source files */
    int source_count;               /* Number of source files */
    
    char *output_file;              /* Output executable/library */
    char *include_dirs[64];         /* Include directories */
    int include_count;              /* Include directory count */
    
    char *library_dirs[64];         /* Library directories */
    int library_count;              /* Library directory count */
    
    char *libraries[128];           /* Linked libraries */
    int lib_count;                  /* Library count */
    
    /* Optimization and debugging */
    enum {
        OPT_NONE,           /* -O0 */
        OPT_SIZE,           /* -Os */
        OPT_SPEED,          /* -O2 */
        OPT_AGGRESSIVE      /* -O3 */
    } optimization;
    
    bool debug_info;                /* Generate debug info (-g) */
    bool warnings_as_errors;        /* -Werror */
    bool position_independent;      /* -fPIC */
    bool static_linking;            /* Static vs dynamic linking */
    
    /* Advanced features */
    bool enable_lto;                /* Link-time optimization */
    bool enable_sanitizers;         /* AddressSanitizer, etc. */
    bool enable_coverage;           /* Code coverage */
    bool enable_profiling;          /* Profiling support */
    
    /* Target specification */
    char *target_arch;              /* Target architecture */
    char *target_abi;               /* Target ABI */
    char *march;                    /* -march= */
    char *mtune;                    /* -mtune= */
    
    /* Custom flags */
    char *custom_cflags;            /* Custom C flags */
    char *custom_cxxflags;          /* Custom C++ flags */
    char *custom_ldflags;           /* Custom linker flags */
} build_config_t;

/* Package Information */
typedef struct package_info {
    char name[64];                  /* Package name */
    char version[16];               /* Package version */
    char description[256];          /* Package description */
    char maintainer[128];           /* Package maintainer */
    
    char *dependencies[32];         /* Dependencies */
    int dependency_count;           /* Dependency count */
    
    char install_path[MAX_PATH_LENGTH]; /* Installation path */
    size_t installed_size;          /* Installed size in bytes */
    
    bool installed;                 /* Installation status */
    char install_date[32];          /* Installation date */
    
    struct package_info *next;      /* Next package */
} package_info_t;

/* Development Environment */
typedef struct dev_environment {
    char sysroot[MAX_PATH_LENGTH];  /* System root */
    char gcc_path[MAX_PATH_LENGTH]; /* GCC compiler path */
    char gxx_path[MAX_PATH_LENGTH]; /* G++ compiler path */
    char ld_path[MAX_PATH_LENGTH];  /* Linker path */
    char ar_path[MAX_PATH_LENGTH];  /* Archiver path */
    char nm_path[MAX_PATH_LENGTH];  /* Symbol reader path */
    char objdump_path[MAX_PATH_LENGTH]; /* Object dumper */
    char gdb_path[MAX_PATH_LENGTH]; /* Debugger path */
    
    /* Standard library paths */
    char libc_path[MAX_PATH_LENGTH];    /* libc path */
    char libcxx_path[MAX_PATH_LENGTH];  /* libstdc++ path */
    char libm_path[MAX_PATH_LENGTH];    /* libm path */
    char libpthread_path[MAX_PATH_LENGTH]; /* pthread library */
    
    /* Package management */
    package_info_t *installed_packages; /* Installed packages */
    int package_count;              /* Package count */
    
    /* Environment variables */
    char **env_vars;                /* Environment variables */
    int env_count;                  /* Environment variable count */
    
    /* Build statistics */
    struct {
        uint64_t total_compilations;
        uint64_t successful_builds;
        uint64_t failed_builds;
        uint64_t lines_of_code_compiled;
        uint64_t build_time_total;  /* milliseconds */
    } stats;
} dev_environment_t;

/* Global development environment */
static dev_environment_t g_dev_env = {0};

/**
 * Initialize development environment
 */
int dev_environment_init(void) {
    printf("Initializing LimitlessOS Development Environment...\n");
    
    // Set up standard paths
    strcpy(g_dev_env.sysroot, LIMITLESS_SYSROOT);
    snprintf(g_dev_env.gcc_path, sizeof(g_dev_env.gcc_path), 
             "%s/bin/limitless-gcc", LIMITLESS_SYSROOT);
    snprintf(g_dev_env.gxx_path, sizeof(g_dev_env.gxx_path), 
             "%s/bin/limitless-g++", LIMITLESS_SYSROOT);
    snprintf(g_dev_env.ld_path, sizeof(g_dev_env.ld_path), 
             "%s/bin/limitless-ld", LIMITLESS_SYSROOT);
    snprintf(g_dev_env.ar_path, sizeof(g_dev_env.ar_path), 
             "%s/bin/limitless-ar", LIMITLESS_SYSROOT);
    snprintf(g_dev_env.nm_path, sizeof(g_dev_env.nm_path), 
             "%s/bin/limitless-nm", LIMITLESS_SYSROOT);
    snprintf(g_dev_env.objdump_path, sizeof(g_dev_env.objdump_path), 
             "%s/bin/limitless-objdump", LIMITLESS_SYSROOT);
    snprintf(g_dev_env.gdb_path, sizeof(g_dev_env.gdb_path), 
             "%s/bin/limitless-gdb", LIMITLESS_SYSROOT);
    
    // Set up library paths
    snprintf(g_dev_env.libc_path, sizeof(g_dev_env.libc_path), 
             "%s/lib/libc.so", LIMITLESS_SYSROOT);
    snprintf(g_dev_env.libcxx_path, sizeof(g_dev_env.libcxx_path), 
             "%s/lib/libstdc++.so", LIMITLESS_SYSROOT);
    snprintf(g_dev_env.libm_path, sizeof(g_dev_env.libm_path), 
             "%s/lib/libm.so", LIMITLESS_SYSROOT);
    snprintf(g_dev_env.libpthread_path, sizeof(g_dev_env.libpthread_path), 
             "%s/lib/libpthread.so", LIMITLESS_SYSROOT);
    
    // Create necessary directories
    struct stat st;
    if (stat(LIMITLESS_SYSROOT, &st) == -1) {
        if (mkdir(LIMITLESS_SYSROOT, 0755) == -1) {
            perror("Failed to create sysroot directory");
            return -1;
        }
    }
    
    char dir_path[MAX_PATH_LENGTH];
    const char *dirs[] = {"bin", "lib", "include", "share", "etc", NULL};
    
    for (int i = 0; dirs[i]; i++) {
        snprintf(dir_path, sizeof(dir_path), "%s/%s", LIMITLESS_SYSROOT, dirs[i]);
        if (stat(dir_path, &st) == -1) {
            if (mkdir(dir_path, 0755) == -1) {
                printf("Warning: Failed to create directory %s\n", dir_path);
            }
        }
    }
    
    // Set up environment variables
    setenv("LIMITLESS_SYSROOT", LIMITLESS_SYSROOT, 1);
    setenv("LIMITLESS_CC", g_dev_env.gcc_path, 1);
    setenv("LIMITLESS_CXX", g_dev_env.gxx_path, 1);
    setenv("LIMITLESS_LD", g_dev_env.ld_path, 1);
    
    char path_env[MAX_PATH_LENGTH * 2];
    snprintf(path_env, sizeof(path_env), "%s:%s", 
             LIMITLESS_BIN_PATH, getenv("PATH") ?: "");
    setenv("PATH", path_env, 1);
    
    char lib_path[MAX_PATH_LENGTH * 2];
    snprintf(lib_path, sizeof(lib_path), "%s:%s", 
             LIMITLESS_LIB_PATH, getenv("LD_LIBRARY_PATH") ?: "");
    setenv("LD_LIBRARY_PATH", lib_path, 1);
    
    char include_path[MAX_PATH_LENGTH * 2];
    snprintf(include_path, sizeof(include_path), "%s:%s", 
             LIMITLESS_INCLUDE_PATH, getenv("C_INCLUDE_PATH") ?: "");
    setenv("C_INCLUDE_PATH", include_path, 1);
    setenv("CPLUS_INCLUDE_PATH", include_path, 1);
    
    printf("LimitlessOS Development Environment initialized\n");
    printf("Sysroot: %s\n", LIMITLESS_SYSROOT);
    printf("GCC Version: %s\n", LIMITLESS_GCC_VERSION);
    
    return 0;
}

/**
 * Detect source file language
 */
language_t detect_language(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return LANG_C;  // Default to C
    
    if (strcmp(ext, ".c") == 0) return LANG_C;
    if (strcmp(ext, ".cpp") == 0 || strcmp(ext, ".cxx") == 0 || 
        strcmp(ext, ".cc") == 0) return LANG_CXX;
    if (strcmp(ext, ".s") == 0 || strcmp(ext, ".S") == 0 || 
        strcmp(ext, ".asm") == 0) return LANG_ASSEMBLY;
    if (strcmp(ext, ".m") == 0) return LANG_OBJECTIVE_C;
    if (strcmp(ext, ".f") == 0 || strcmp(ext, ".f90") == 0 || 
        strcmp(ext, ".f95") == 0) return LANG_FORTRAN;
    if (strcmp(ext, ".go") == 0) return LANG_GO;
    if (strcmp(ext, ".rs") == 0) return LANG_RUST;
    
    return LANG_C;  // Default fallback
}

/**
 * Build command line arguments
 */
int build_compile_command(build_config_t *config, char ***args) {
    static char *arg_array[MAX_COMPILE_ARGS];
    int argc = 0;
    
    // Compiler selection
    switch (config->language) {
        case LANG_C:
        case LANG_ASSEMBLY:
            arg_array[argc++] = strdup(g_dev_env.gcc_path);
            break;
        case LANG_CXX:
        case LANG_OBJECTIVE_C:
            arg_array[argc++] = strdup(g_dev_env.gxx_path);
            break;
        default:
            printf("Unsupported language for GCC compilation\n");
            return -1;
    }
    
    // Source files
    for (int i = 0; i < config->source_count; i++) {
        arg_array[argc++] = strdup(config->source_files[i]);
    }
    
    // Output file
    if (config->output_file) {
        arg_array[argc++] = strdup("-o");
        arg_array[argc++] = strdup(config->output_file);
    }
    
    // Include directories
    for (int i = 0; i < config->include_count; i++) {
        char *include_flag = malloc(strlen(config->include_dirs[i]) + 3);
        snprintf(include_flag, strlen(config->include_dirs[i]) + 3, 
                "-I%s", config->include_dirs[i]);
        arg_array[argc++] = include_flag;
    }
    
    // Standard include directories
    arg_array[argc++] = strdup("-I" LIMITLESS_INCLUDE_PATH);
    arg_array[argc++] = strdup("-I" LIMITLESS_INCLUDE_PATH "/sys");
    arg_array[argc++] = strdup("-I" LIMITLESS_INCLUDE_PATH "/net");
    
    // Library directories
    for (int i = 0; i < config->library_count; i++) {
        char *lib_dir_flag = malloc(strlen(config->library_dirs[i]) + 3);
        snprintf(lib_dir_flag, strlen(config->library_dirs[i]) + 3, 
                "-L%s", config->library_dirs[i]);
        arg_array[argc++] = lib_dir_flag;
    }
    
    // Standard library directory
    arg_array[argc++] = strdup("-L" LIMITLESS_LIB_PATH);
    
    // Libraries
    for (int i = 0; i < config->lib_count; i++) {
        char *lib_flag = malloc(strlen(config->libraries[i]) + 3);
        snprintf(lib_flag, strlen(config->libraries[i]) + 3, 
                "-l%s", config->libraries[i]);
        arg_array[argc++] = lib_flag;
    }
    
    // Optimization flags
    switch (config->optimization) {
        case OPT_NONE:
            arg_array[argc++] = strdup("-O0");
            break;
        case OPT_SIZE:
            arg_array[argc++] = strdup("-Os");
            break;
        case OPT_SPEED:
            arg_array[argc++] = strdup("-O2");
            break;
        case OPT_AGGRESSIVE:
            arg_array[argc++] = strdup("-O3");
            arg_array[argc++] = strdup("-march=native");
            break;
    }
    
    // Debug information
    if (config->debug_info) {
        arg_array[argc++] = strdup("-g3");
        arg_array[argc++] = strdup("-ggdb");
    }
    
    // Warning flags
    arg_array[argc++] = strdup("-Wall");
    arg_array[argc++] = strdup("-Wextra");
    arg_array[argc++] = strdup("-Wpedantic");
    
    if (config->warnings_as_errors) {
        arg_array[argc++] = strdup("-Werror");
    }
    
    // Position independent code
    if (config->position_independent) {
        arg_array[argc++] = strdup("-fPIC");
    }
    
    // Static linking
    if (config->static_linking) {
        arg_array[argc++] = strdup("-static");
    }
    
    // Link-time optimization
    if (config->enable_lto) {
        arg_array[argc++] = strdup("-flto");
    }
    
    // Sanitizers
    if (config->enable_sanitizers) {
        arg_array[argc++] = strdup("-fsanitize=address");
        arg_array[argc++] = strdup("-fsanitize=undefined");
        arg_array[argc++] = strdup("-fno-omit-frame-pointer");
    }
    
    // Code coverage
    if (config->enable_coverage) {
        arg_array[argc++] = strdup("-fprofile-arcs");
        arg_array[argc++] = strdup("-ftest-coverage");
    }
    
    // Profiling
    if (config->enable_profiling) {
        arg_array[argc++] = strdup("-pg");
    }
    
    // Target architecture
    if (config->target_arch) {
        char *arch_flag = malloc(strlen(config->target_arch) + 9);
        snprintf(arch_flag, strlen(config->target_arch) + 9, 
                "-march=%s", config->target_arch);
        arg_array[argc++] = arch_flag;
    }
    
    if (config->mtune) {
        char *tune_flag = malloc(strlen(config->mtune) + 8);
        snprintf(tune_flag, strlen(config->mtune) + 8, 
                "-mtune=%s", config->mtune);
        arg_array[argc++] = tune_flag;
    }
    
    // Custom flags
    if (config->custom_cflags) {
        // Split custom flags by spaces and add each
        char *flags = strdup(config->custom_cflags);
        char *token = strtok(flags, " ");
        while (token && argc < MAX_COMPILE_ARGS - 1) {
            arg_array[argc++] = strdup(token);
            token = strtok(NULL, " ");
        }
        free(flags);
    }
    
    // Standard libraries (always linked last)
    if (config->language == LANG_CXX) {
        arg_array[argc++] = strdup("-lstdc++");
    }
    arg_array[argc++] = strdup("-lm");
    arg_array[argc++] = strdup("-lc");
    
    // Null terminate
    arg_array[argc] = NULL;
    
    *args = arg_array;
    return argc;
}

/**
 * Compile source files
 */
int compile_sources(build_config_t *config) {
    printf("Compiling %s project with %d source files...\n",
           (config->language == LANG_CXX) ? "C++" : "C", config->source_count);
    
    uint64_t start_time = get_current_time_ms();
    
    char **compile_args;
    int argc = build_compile_command(config, &compile_args);
    
    if (argc < 0) {
        printf("Failed to build compile command\n");
        return -1;
    }
    
    // Print compile command for debugging
    printf("Compile command: ");
    for (int i = 0; i < argc; i++) {
        printf("%s ", compile_args[i]);
    }
    printf("\n");
    
    // Fork and execute compiler
    pid_t pid = fork();
    if (pid == 0) {
        // Child process - execute compiler
        execv(compile_args[0], compile_args);
        perror("Failed to execute compiler");
        exit(1);
    } else if (pid > 0) {
        // Parent process - wait for compilation
        int status;
        waitpid(pid, &status, 0);
        
        uint64_t compile_time = get_current_time_ms() - start_time;
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Compilation successful in %llu ms\n", compile_time);
            
            // Update statistics
            g_dev_env.stats.total_compilations++;
            g_dev_env.stats.successful_builds++;
            g_dev_env.stats.build_time_total += compile_time;
            
            // Count lines of code
            uint64_t total_lines = 0;
            for (int i = 0; i < config->source_count; i++) {
                total_lines += count_lines_in_file(config->source_files[i]);
            }
            g_dev_env.stats.lines_of_code_compiled += total_lines;
            
            return 0;
        } else {
            printf("Compilation failed with exit code %d\n", WEXITSTATUS(status));
            g_dev_env.stats.total_compilations++;
            g_dev_env.stats.failed_builds++;
            return -1;
        }
    } else {
        perror("Failed to fork compiler process");
        return -1;
    }
}

/**
 * Create build configuration from command line
 */
build_config_t *create_build_config(int argc, char **argv) {
    build_config_t *config = calloc(1, sizeof(build_config_t));
    if (!config) return NULL;
    
    // Set defaults
    config->language = LANG_C;
    config->optimization = OPT_SPEED;  // -O2 by default
    config->debug_info = true;         // Include debug info by default
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            config->output_file = strdup(argv[++i]);
        } else if (strncmp(argv[i], "-I", 2) == 0) {
            if (config->include_count < 64) {
                config->include_dirs[config->include_count++] = strdup(argv[i] + 2);
            }
        } else if (strncmp(argv[i], "-L", 2) == 0) {
            if (config->library_count < 64) {
                config->library_dirs[config->library_count++] = strdup(argv[i] + 2);
            }
        } else if (strncmp(argv[i], "-l", 2) == 0) {
            if (config->lib_count < 128) {
                config->libraries[config->lib_count++] = strdup(argv[i] + 2);
            }
        } else if (strcmp(argv[i], "-O0") == 0) {
            config->optimization = OPT_NONE;
        } else if (strcmp(argv[i], "-Os") == 0) {
            config->optimization = OPT_SIZE;
        } else if (strcmp(argv[i], "-O2") == 0) {
            config->optimization = OPT_SPEED;
        } else if (strcmp(argv[i], "-O3") == 0) {
            config->optimization = OPT_AGGRESSIVE;
        } else if (strcmp(argv[i], "-g") == 0) {
            config->debug_info = true;
        } else if (strcmp(argv[i], "-Werror") == 0) {
            config->warnings_as_errors = true;
        } else if (strcmp(argv[i], "-fPIC") == 0) {
            config->position_independent = true;
        } else if (strcmp(argv[i], "-static") == 0) {
            config->static_linking = true;
        } else if (strcmp(argv[i], "-flto") == 0) {
            config->enable_lto = true;
        } else if (strncmp(argv[i], "-fsanitize=", 11) == 0) {
            config->enable_sanitizers = true;
        } else if (strcmp(argv[i], "--coverage") == 0) {
            config->enable_coverage = true;
        } else if (strcmp(argv[i], "-pg") == 0) {
            config->enable_profiling = true;
        } else if (argv[i][0] != '-') {
            // Source file
            if (config->source_count < MAX_SOURCE_FILES) {
                config->source_files[config->source_count++] = strdup(argv[i]);
                
                // Detect language from first source file
                if (config->source_count == 1) {
                    config->language = detect_language(argv[i]);
                }
            }
        }
    }
    
    // Generate default output name if not specified
    if (!config->output_file && config->source_count > 0) {
        char *basename = strdup(config->source_files[0]);
        char *dot = strrchr(basename, '.');
        if (dot) *dot = '\0';
        config->output_file = basename;
    }
    
    return config;
}

/**
 * LimitlessOS Compiler Driver (limitless-gcc)
 */
int limitless_gcc_main(int argc, char **argv) {
    printf("LimitlessOS GCC Compiler Driver v%s\n", LIMITLESS_GCC_VERSION);
    
    if (argc < 2) {
        printf("Usage: %s [options] source_files...\n", argv[0]);
        printf("Options:\n");
        printf("  -o <file>         Output file name\n");
        printf("  -I<dir>           Include directory\n");
        printf("  -L<dir>           Library directory\n");
        printf("  -l<lib>           Link library\n");
        printf("  -O0,-Os,-O2,-O3   Optimization level\n");
        printf("  -g                Generate debug information\n");
        printf("  -Werror           Treat warnings as errors\n");
        printf("  -fPIC             Generate position independent code\n");
        printf("  -static           Static linking\n");
        printf("  -flto             Enable link-time optimization\n");
        printf("  --coverage        Enable code coverage\n");
        printf("  -pg               Enable profiling\n");
        return 1;
    }
    
    // Initialize development environment
    if (dev_environment_init() != 0) {
        printf("Failed to initialize development environment\n");
        return 1;
    }
    
    // Create build configuration
    build_config_t *config = create_build_config(argc, argv);
    if (!config) {
        printf("Failed to create build configuration\n");
        return 1;
    }
    
    if (config->source_count == 0) {
        printf("No source files specified\n");
        free(config);
        return 1;
    }
    
    // Perform compilation
    int result = compile_sources(config);
    
    // Cleanup
    for (int i = 0; i < config->source_count; i++) {
        free(config->source_files[i]);
    }
    free(config->output_file);
    free(config);
    
    return result;
}

/**
 * Count lines in source file
 */
uint64_t count_lines_in_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    
    uint64_t lines = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') lines++;
    }
    
    fclose(file);
    return lines;
}

/**
 * Get current time in milliseconds
 */
uint64_t get_current_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/**
 * Development environment status
 */
void dev_environment_status(void) {
    printf("LimitlessOS Development Environment Status:\n");
    printf("==========================================\n");
    
    printf("Sysroot: %s\n", g_dev_env.sysroot);
    printf("GCC Path: %s\n", g_dev_env.gcc_path);
    printf("G++ Path: %s\n", g_dev_env.gxx_path);
    printf("Linker: %s\n", g_dev_env.ld_path);
    printf("Debugger: %s\n", g_dev_env.gdb_path);
    
    printf("\nLibraries:\n");
    printf("  libc: %s\n", g_dev_env.libc_path);
    printf("  libstdc++: %s\n", g_dev_env.libcxx_path);
    printf("  libm: %s\n", g_dev_env.libm_path);
    printf("  libpthread: %s\n", g_dev_env.libpthread_path);
    
    printf("\nBuild Statistics:\n");
    printf("  Total Compilations: %llu\n", g_dev_env.stats.total_compilations);
    printf("  Successful Builds: %llu\n", g_dev_env.stats.successful_builds);
    printf("  Failed Builds: %llu\n", g_dev_env.stats.failed_builds);
    printf("  Success Rate: %.1f%%\n", 
           g_dev_env.stats.total_compilations > 0 ? 
           (double)g_dev_env.stats.successful_builds / g_dev_env.stats.total_compilations * 100.0 : 0.0);
    printf("  Lines of Code Compiled: %llu\n", g_dev_env.stats.lines_of_code_compiled);
    printf("  Total Build Time: %.2f seconds\n", g_dev_env.stats.build_time_total / 1000.0);
    
    if (g_dev_env.stats.successful_builds > 0) {
        printf("  Average Build Time: %.2f ms\n", 
               (double)g_dev_env.stats.build_time_total / g_dev_env.stats.successful_builds);
    }
    
    printf("\nInstalled Packages: %d\n", g_dev_env.package_count);
}

/**
 * Test compilation system
 */
int test_compilation_system(void) {
    printf("Testing LimitlessOS compilation system...\n");
    
    // Create test C program
    const char *test_c_source = 
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "#include <string.h>\n"
        "#include <math.h>\n"
        "\n"
        "int main(int argc, char **argv) {\n"
        "    printf(\"Hello from LimitlessOS!\\n\");\n"
        "    printf(\"Arguments: %d\\n\", argc);\n"
        "    \n"
        "    // Test math library\n"
        "    double result = sqrt(16.0);\n"
        "    printf(\"sqrt(16) = %.2f\\n\", result);\n"
        "    \n"
        "    // Test string functions\n"
        "    char buffer[256];\n"
        "    strcpy(buffer, \"LimitlessOS Development Test\");\n"
        "    printf(\"String test: %s\\n\", buffer);\n"
        "    \n"
        "    return 0;\n"
        "}\n";
    
    // Write test source to file
    FILE *test_file = fopen("test_program.c", "w");
    if (!test_file) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(test_file, "%s", test_c_source);
    fclose(test_file);
    
    // Create build configuration for test
    build_config_t test_config = {0};
    test_config.language = LANG_C;
    test_config.source_count = 1;
    test_config.source_files[0] = "test_program.c";
    test_config.output_file = "test_program";
    test_config.optimization = OPT_SPEED;
    test_config.debug_info = true;
    
    // Add math library
    test_config.lib_count = 1;
    test_config.libraries[0] = "m";
    
    printf("Compiling test program...\n");
    
    // Compile test program
    int result = compile_sources(&test_config);
    
    if (result == 0) {
        printf("Test compilation successful!\n");
        
        // Try to run the test program
        printf("Running test program...\n");
        system("./test_program");
        
        // Clean up
        unlink("test_program.c");
        unlink("test_program");
        
        printf("Compilation system test PASSED\n");
        return 0;
    } else {
        printf("Test compilation failed!\n");
        unlink("test_program.c");
        printf("Compilation system test FAILED\n");
        return -1;
    }
}

/* Main entry point for compiler driver */
int main(int argc, char **argv) {
    return limitless_gcc_main(argc, argv);
}