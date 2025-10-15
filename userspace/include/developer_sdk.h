/**
 * LimitlessOS Developer SDK
 * Comprehensive development environment and toolchain
 */

#ifndef LIMITLESS_DEVELOPER_SDK_H
#define LIMITLESS_DEVELOPER_SDK_H

#include <stdint.h>
#include <stdbool.h>

/* SDK Version */
#define LIMITLESS_SDK_VERSION_MAJOR 2
#define LIMITLESS_SDK_VERSION_MINOR 0

/* Maximum limits */
#define MAX_PROJECTS                256
#define MAX_SOURCE_FILES            65536
#define MAX_BUILD_TARGETS           128
#define MAX_DEPENDENCIES            1024
#define MAX_DEBUGGER_SESSIONS       64
#define MAX_BREAKPOINTS            512
#define MAX_WATCH_EXPRESSIONS      256
#define MAX_CODE_COMPLETIONS       1000
#define MAX_LANGUAGE_SERVERS       32

/* Supported Languages */
typedef enum {
    LANG_UNKNOWN,
    LANG_C,
    LANG_CPP,
    LANG_RUST,
    LANG_GO,
    LANG_PYTHON,
    LANG_JAVASCRIPT,
    LANG_TYPESCRIPT,
    LANG_JAVA,
    LANG_KOTLIN,
    LANG_SWIFT,
    LANG_CSHARP,
    LANG_FSHARP,
    LANG_ASSEMBLY,
    LANG_SHELL,
    LANG_MARKDOWN,
    LANG_JSON,
    LANG_YAML,
    LANG_XML,
    LANG_HTML,
    LANG_CSS,
    LANG_SQL,
    LANG_PERL,
    LANG_RUBY,
    LANG_PHP,
    LANG_DART,
    LANG_SCALA,
    LANG_HASKELL,
    LANG_OCaml,
    LANG_ERLANG,
    LANG_ELIXIR,
    LANG_CLOJURE,
    LANG_LUA,
    LANG_R,
    LANG_MATLAB,
    LANG_JULIA,
    LANG_FORTRAN,
    LANG_COBOL,
    LANG_ADA,
    LANG_PASCAL,
    LANG_VERILOG,
    LANG_VHDL,
    LANG_QUANTUM,           /* Quantum programming language */
    LANG_LIMITLESS          /* LimitlessOS native language */
} programming_language_t;

/* Project Types */
typedef enum {
    PROJECT_TYPE_EXECUTABLE,
    PROJECT_TYPE_LIBRARY_STATIC,
    PROJECT_TYPE_LIBRARY_DYNAMIC,
    PROJECT_TYPE_KERNEL_MODULE,
    PROJECT_TYPE_DEVICE_DRIVER,
    PROJECT_TYPE_SYSTEM_SERVICE,
    PROJECT_TYPE_APPLICATION,
    PROJECT_TYPE_WEB_APPLICATION,
    PROJECT_TYPE_MOBILE_APPLICATION,
    PROJECT_TYPE_GAME,
    PROJECT_TYPE_FIRMWARE,
    PROJECT_TYPE_BOOTLOADER,
    PROJECT_TYPE_MICROKERNEL,
    PROJECT_TYPE_CONTAINER_IMAGE,
    PROJECT_TYPE_PACKAGE,
    PROJECT_TYPE_DOCUMENTATION,
    PROJECT_TYPE_TEST_SUITE
} project_type_t;

/* Build System Types */
typedef enum {
    BUILD_SYSTEM_MAKE,
    BUILD_SYSTEM_CMAKE,
    BUILD_SYSTEM_NINJA,
    BUILD_SYSTEM_BAZEL,
    BUILD_SYSTEM_BUCK,
    BUILD_SYSTEM_GRADLE,
    BUILD_SYSTEM_MAVEN,
    BUILD_SYSTEM_CARGO,        /* Rust */
    BUILD_SYSTEM_GO_BUILD,     /* Go */
    BUILD_SYSTEM_NPM,          /* Node.js */
    BUILD_SYSTEM_YARN,         /* JavaScript */
    BUILD_SYSTEM_MESON,
    BUILD_SYSTEM_SCONS,
    BUILD_SYSTEM_WAF,
    BUILD_SYSTEM_XMAKE,
    BUILD_SYSTEM_LIMITLESS     /* LimitlessOS native build system */
} build_system_t;

/* Target Architectures */
typedef enum {
    TARGET_ARCH_X86,
    TARGET_ARCH_X86_64,
    TARGET_ARCH_ARM,
    TARGET_ARCH_ARM64,
    TARGET_ARCH_RISCV32,
    TARGET_ARCH_RISCV64,
    TARGET_ARCH_MIPS,
    TARGET_ARCH_MIPS64,
    TARGET_ARCH_POWERPC,
    TARGET_ARCH_POWERPC64,
    TARGET_ARCH_SPARC,
    TARGET_ARCH_SPARC64,
    TARGET_ARCH_S390X,
    TARGET_ARCH_WASM32,        /* WebAssembly */
    TARGET_ARCH_WASM64,
    TARGET_ARCH_QUANTUM        /* Quantum computing target */
} target_architecture_t;

/* Optimization Levels */
typedef enum {
    OPT_LEVEL_NONE,            /* -O0 */
    OPT_LEVEL_FAST,            /* -O1 */
    OPT_LEVEL_FASTER,          /* -O2 */
    OPT_LEVEL_FASTEST,         /* -O3 */
    OPT_LEVEL_SIZE,            /* -Os */
    OPT_LEVEL_AGGRESSIVE,      /* -Ofast */
    OPT_LEVEL_DEBUG,           /* -Og */
    OPT_LEVEL_QUANTUM          /* Quantum optimization */
} optimization_level_t;

/* Debug Information Types */
typedef enum {
    DEBUG_INFO_NONE,
    DEBUG_INFO_DWARF,
    DEBUG_INFO_PDB,            /* Microsoft Program Database */
    DEBUG_INFO_STABS,
    DEBUG_INFO_LIMITLESS       /* LimitlessOS enhanced debug info */
} debug_info_type_t;

/* Version Control Systems */
typedef enum {
    VCS_NONE,
    VCS_GIT,
    VCS_SUBVERSION,
    VCS_MERCURIAL,
    VCS_BAZAAR,
    VCS_PERFORCE,
    VCS_CVS,
    VCS_LIMITLESS              /* LimitlessOS distributed VCS */
} version_control_t;

/* Source File Information */
typedef struct source_file {
    uint32_t id;
    char path[512];            /* Full file path */
    char name[256];            /* File name */
    programming_language_t language;
    
    /* File properties */
    uint64_t size;             /* File size in bytes */
    uint64_t lines;            /* Number of lines */
    uint64_t modified_time;    /* Last modified timestamp */
    uint32_t checksum;         /* File checksum */
    
    /* Syntax analysis */
    bool syntax_valid;
    uint32_t syntax_error_count;
    void* syntax_tree;         /* Abstract syntax tree */
    
    /* Editor state */
    bool is_open;
    uint32_t cursor_line;
    uint32_t cursor_column;
    bool is_modified;
    
    /* Version control */
    version_control_t vcs;
    char vcs_revision[64];
    bool is_tracked;
    bool has_changes;
    
    struct source_file* next;
} source_file_t;

/* Build Configuration */
typedef struct build_config {
    char name[64];             /* Configuration name (Debug, Release, etc.) */
    target_architecture_t target_arch;
    optimization_level_t optimization;
    debug_info_type_t debug_info;
    
    /* Compiler flags */
    char c_flags[512];
    char cpp_flags[512];
    char link_flags[512];
    char defines[1024];        /* Preprocessor definitions */
    char include_paths[2048];  /* Include directory paths */
    char library_paths[2048];  /* Library directory paths */
    char libraries[1024];      /* Libraries to link */
    
    /* Output settings */
    char output_directory[256];
    char output_name[128];
    
    /* Build options */
    bool enable_warnings;
    bool warnings_as_errors;
    bool enable_static_analysis;
    bool enable_profiling;
    bool enable_coverage;
    bool enable_sanitizers;
    uint32_t parallel_jobs;
    
    struct build_config* next;
} build_config_t;

/* Project Dependency */
typedef struct dependency {
    char name[128];            /* Dependency name */
    char version[32];          /* Required version */
    char source_url[512];      /* Source URL or repository */
    bool is_system;            /* System-provided dependency */
    bool is_optional;          /* Optional dependency */
    
    /* Package information */
    char package_manager[32];  /* Package manager (apt, yum, pacman, etc.) */
    char package_name[128];    /* Package name in manager */
    
    struct dependency* next;
} dependency_t;

/* Build Target */
typedef struct build_target {
    char name[128];            /* Target name */
    project_type_t type;       /* Target type */
    
    /* Source files */
    source_file_t* source_files;
    uint32_t source_file_count;
    
    /* Build configuration */
    build_config_t* configs;
    build_config_t* active_config;
    
    /* Dependencies */
    dependency_t* dependencies;
    uint32_t dependency_count;
    
    /* Build state */
    enum {
        TARGET_STATE_NOT_BUILT,
        TARGET_STATE_BUILDING,
        TARGET_STATE_BUILT,
        TARGET_STATE_FAILED
    } build_state;
    
    uint64_t last_build_time;
    uint32_t build_duration_ms;
    bool needs_rebuild;
    
    struct build_target* next;
} build_target_t;

/* Development Project */
typedef struct dev_project {
    uint32_t id;
    char name[128];            /* Project name */
    char path[512];            /* Project root path */
    char description[512];     /* Project description */
    
    project_type_t type;       /* Project type */
    programming_language_t primary_language;
    build_system_t build_system;
    
    /* Version information */
    char version[32];
    char author[128];
    char license[64];
    char website[256];
    
    /* Build targets */
    build_target_t* targets;
    uint32_t target_count;
    build_target_t* active_target;
    
    /* Source files */
    source_file_t* all_source_files;
    uint32_t total_source_files;
    
    /* Version control */
    version_control_t vcs;
    char vcs_repository[512];
    char vcs_branch[64];
    
    /* Project settings */
    char editor_config[256];   /* Editor configuration file */
    char build_script[256];    /* Build script path */
    char test_script[256];     /* Test script path */
    
    /* Statistics */
    uint64_t creation_time;
    uint64_t last_opened_time;
    uint32_t total_builds;
    uint32_t successful_builds;
    uint64_t total_build_time;
    
    struct dev_project* next;
} dev_project_t;

/* Code Completion Item */
typedef struct completion_item {
    char label[128];           /* Completion label */
    char detail[256];          /* Additional detail */
    char documentation[512];   /* Documentation string */
    
    enum {
        COMPLETION_TYPE_VARIABLE,
        COMPLETION_TYPE_FUNCTION,
        COMPLETION_TYPE_METHOD,
        COMPLETION_TYPE_CLASS,
        COMPLETION_TYPE_INTERFACE,
        COMPLETION_TYPE_ENUM,
        COMPLETION_TYPE_STRUCT,
        COMPLETION_TYPE_KEYWORD,
        COMPLETION_TYPE_SNIPPET,
        COMPLETION_TYPE_MODULE,
        COMPLETION_TYPE_NAMESPACE
    } type;
    
    char insert_text[256];     /* Text to insert */
    uint32_t priority;         /* Completion priority */
    
} completion_item_t;

/* Language Server */
typedef struct language_server {
    uint32_t id;
    programming_language_t language;
    char name[64];
    char executable[256];      /* Server executable path */
    char arguments[512];       /* Command line arguments */
    
    /* Capabilities */
    bool supports_completion;
    bool supports_hover;
    bool supports_signature_help;
    bool supports_goto_definition;
    bool supports_goto_references;
    bool supports_formatting;
    bool supports_rename;
    bool supports_code_actions;
    bool supports_diagnostics;
    
    /* Server state */
    bool running;
    uint32_t process_id;
    void* connection;          /* Communication channel */
    
    struct language_server* next;
} language_server_t;

/* Debugger Session */
typedef struct debugger_session {
    uint32_t id;
    char name[64];
    programming_language_t language;
    
    /* Target process */
    uint32_t target_pid;
    char target_executable[512];
    char target_arguments[1024];
    char working_directory[256];
    
    /* Debugger state */
    enum {
        DEBUG_STATE_NOT_STARTED,
        DEBUG_STATE_RUNNING,
        DEBUG_STATE_PAUSED,
        DEBUG_STATE_TERMINATED
    } state;
    
    /* Current execution state */
    uint32_t current_thread_id;
    uint64_t current_address;
    char current_function[128];
    char current_file[512];
    uint32_t current_line;
    
    /* Breakpoints */
    struct {
        uint32_t id;
        char file[512];
        uint32_t line;
        uint64_t address;
        bool enabled;
        char condition[256];
        uint32_t hit_count;
    } breakpoints[MAX_BREAKPOINTS];
    uint32_t breakpoint_count;
    
    /* Watch expressions */
    struct {
        uint32_t id;
        char expression[256];
        char value[512];
        char type[64];
        bool valid;
    } watch_expressions[MAX_WATCH_EXPRESSIONS];
    uint32_t watch_count;
    
    /* Call stack */
    struct {
        uint64_t address;
        char function[128];
        char file[512];
        uint32_t line;
        void* frame_pointer;
    }* call_stack;
    uint32_t call_stack_depth;
    
    /* Variables */
    void* local_variables;
    void* global_variables;
    
    struct debugger_session* next;
} debugger_session_t;

/* Package Information */
typedef struct package_info {
    char name[128];            /* Package name */
    char version[32];          /* Package version */
    char description[512];     /* Package description */
    char author[128];          /* Package author */
    char license[64];          /* Package license */
    char homepage[256];        /* Package homepage */
    
    /* Dependencies */
    dependency_t* dependencies;
    uint32_t dependency_count;
    
    /* Files */
    char** files;              /* Package files */
    uint32_t file_count;
    
    /* Installation info */
    bool installed;
    char install_path[512];
    uint64_t install_time;
    uint64_t size;
    
    struct package_info* next;
} package_info_t;

/* Package Manager */
typedef struct package_manager {
    char name[64];             /* Package manager name */
    char repository_url[512];  /* Repository URL */
    
    /* Operations */
    status_t (*install)(const char* package_name, const char* version);
    status_t (*uninstall)(const char* package_name);
    status_t (*update)(const char* package_name);
    status_t (*search)(const char* query, package_info_t** results, uint32_t* count);
    status_t (*list_installed)(package_info_t** packages, uint32_t* count);
    status_t (*get_info)(const char* package_name, package_info_t* info);
    
    struct package_manager* next;
} package_manager_t;

/* Code Analysis Result */
typedef struct analysis_result {
    enum {
        ANALYSIS_ERROR,
        ANALYSIS_WARNING,
        ANALYSIS_INFO,
        ANALYSIS_HINT
    } severity;
    
    char message[512];         /* Analysis message */
    char file[512];            /* Source file */
    uint32_t line;             /* Line number */
    uint32_t column;           /* Column number */
    char rule_id[64];          /* Analysis rule ID */
    
} analysis_result_t;

/* Performance Profiling Data */
typedef struct profiling_data {
    char function_name[128];   /* Function name */
    uint64_t call_count;       /* Number of calls */
    uint64_t total_time_ns;    /* Total execution time */
    uint64_t self_time_ns;     /* Self execution time */
    float cpu_percentage;      /* CPU usage percentage */
    
    struct profiling_data* caller;   /* Calling function */
    struct profiling_data* callees;  /* Called functions */
    
} profiling_data_t;

/* Developer Tools */
typedef struct dev_tools {
    bool initialized;
    uint32_t version;
    
    /* Projects */
    dev_project_t* projects;
    uint32_t project_count;
    dev_project_t* active_project;
    
    /* Language servers */
    language_server_t* language_servers;
    uint32_t language_server_count;
    
    /* Debugger sessions */
    debugger_session_t* debugger_sessions;
    uint32_t debugger_session_count;
    
    /* Package managers */
    package_manager_t* package_managers;
    uint32_t package_manager_count;
    
    /* Compiler toolchains */
    struct {
        programming_language_t language;
        char compiler_path[256];
        char version[32];
        bool available;
    } compilers[64];
    uint32_t compiler_count;
    
    /* Build system support */
    struct {
        build_system_t type;
        char executable[256];
        char version[32];
        bool available;
    } build_systems[16];
    uint32_t build_system_count;
    
    /* IDE settings */
    struct {
        char theme[64];
        char font_family[64];
        uint32_t font_size;
        bool auto_save;
        bool show_line_numbers;
        bool show_whitespace;
        bool enable_word_wrap;
        uint32_t tab_size;
        bool use_spaces_for_tabs;
        bool enable_auto_completion;
        bool enable_syntax_highlighting;
    } ide_settings;
    
    /* Statistics */
    struct {
        uint32_t projects_created;
        uint32_t builds_executed;
        uint32_t debug_sessions_started;
        uint64_t lines_of_code_written;
        uint64_t total_development_time;
    } statistics;
    
} dev_tools_t;

/* Global developer tools */
extern dev_tools_t dev_tools;

/* Core SDK API */
status_t dev_tools_init(void);
void dev_tools_shutdown(void);

/* Project Management */
dev_project_t* dev_create_project(const char* name, const char* path, project_type_t type, programming_language_t language);
void dev_destroy_project(dev_project_t* project);
status_t dev_open_project(const char* path, dev_project_t** project);
status_t dev_save_project(dev_project_t* project);
status_t dev_close_project(dev_project_t* project);
dev_project_t* dev_get_project_by_id(uint32_t id);
dev_project_t* dev_get_active_project(void);
void dev_set_active_project(dev_project_t* project);

/* Source File Management */
source_file_t* dev_add_source_file(dev_project_t* project, const char* path);
void dev_remove_source_file(dev_project_t* project, source_file_t* file);
source_file_t* dev_get_source_file(dev_project_t* project, const char* path);
status_t dev_save_source_file(source_file_t* file);
status_t dev_reload_source_file(source_file_t* file);

/* Build Management */
build_target_t* dev_add_build_target(dev_project_t* project, const char* name, project_type_t type);
void dev_remove_build_target(dev_project_t* project, build_target_t* target);
build_config_t* dev_add_build_config(build_target_t* target, const char* name);
void dev_remove_build_config(build_target_t* target, build_config_t* config);
status_t dev_build_target(build_target_t* target, build_config_t* config);
status_t dev_clean_target(build_target_t* target);
status_t dev_rebuild_target(build_target_t* target);

/* Compiler Integration */
status_t dev_detect_compilers(void);
bool dev_is_compiler_available(programming_language_t language);
status_t dev_compile_file(source_file_t* file, build_config_t* config);
status_t dev_link_objects(build_target_t* target, build_config_t* config);

/* Language Server Integration */
status_t dev_start_language_server(programming_language_t language);
void dev_stop_language_server(programming_language_t language);
language_server_t* dev_get_language_server(programming_language_t language);
status_t dev_get_completions(source_file_t* file, uint32_t line, uint32_t column, completion_item_t** completions, uint32_t* count);
status_t dev_get_hover_info(source_file_t* file, uint32_t line, uint32_t column, char* info, size_t info_size);
status_t dev_goto_definition(source_file_t* file, uint32_t line, uint32_t column, char* target_file, uint32_t* target_line);
status_t dev_find_references(source_file_t* file, uint32_t line, uint32_t column, void** references, uint32_t* count);
status_t dev_format_code(source_file_t* file);
status_t dev_rename_symbol(source_file_t* file, uint32_t line, uint32_t column, const char* new_name);

/* Debugger Integration */
debugger_session_t* dev_start_debug_session(build_target_t* target, const char* arguments);
void dev_stop_debug_session(debugger_session_t* session);
status_t dev_debug_continue(debugger_session_t* session);
status_t dev_debug_step_over(debugger_session_t* session);
status_t dev_debug_step_into(debugger_session_t* session);
status_t dev_debug_step_out(debugger_session_t* session);
status_t dev_debug_pause(debugger_session_t* session);
status_t dev_set_breakpoint(debugger_session_t* session, const char* file, uint32_t line);
status_t dev_remove_breakpoint(debugger_session_t* session, uint32_t breakpoint_id);
status_t dev_add_watch_expression(debugger_session_t* session, const char* expression);
status_t dev_remove_watch_expression(debugger_session_t* session, uint32_t watch_id);
status_t dev_evaluate_expression(debugger_session_t* session, const char* expression, char* result, size_t result_size);

/* Package Management */
status_t dev_register_package_manager(package_manager_t* manager);
package_manager_t* dev_get_package_manager(const char* name);
status_t dev_install_package(const char* package_name, const char* version);
status_t dev_uninstall_package(const char* package_name);
status_t dev_update_package(const char* package_name);
status_t dev_search_packages(const char* query, package_info_t** results, uint32_t* count);
status_t dev_list_installed_packages(package_info_t** packages, uint32_t* count);

/* Code Analysis */
status_t dev_analyze_code(source_file_t* file, analysis_result_t** results, uint32_t* count);
status_t dev_analyze_project(dev_project_t* project, analysis_result_t** results, uint32_t* count);
status_t dev_run_static_analysis(build_target_t* target, analysis_result_t** results, uint32_t* count);

/* Testing */
status_t dev_run_tests(build_target_t* target);
status_t dev_run_unit_tests(build_target_t* target);
status_t dev_run_integration_tests(build_target_t* target);
status_t dev_generate_test_report(build_target_t* target, const char* report_file);

/* Profiling */
status_t dev_start_profiling(debugger_session_t* session);
status_t dev_stop_profiling(debugger_session_t* session);
status_t dev_get_profiling_data(debugger_session_t* session, profiling_data_t** data, uint32_t* count);
status_t dev_generate_profiling_report(debugger_session_t* session, const char* report_file);

/* Version Control Integration */
status_t dev_init_repository(dev_project_t* project, version_control_t vcs);
status_t dev_clone_repository(const char* url, const char* local_path, version_control_t vcs);
status_t dev_commit_changes(dev_project_t* project, const char* message);
status_t dev_push_changes(dev_project_t* project);
status_t dev_pull_changes(dev_project_t* project);
status_t dev_create_branch(dev_project_t* project, const char* branch_name);
status_t dev_switch_branch(dev_project_t* project, const char* branch_name);
status_t dev_merge_branch(dev_project_t* project, const char* branch_name);

/* Documentation Generation */
status_t dev_generate_documentation(dev_project_t* project, const char* output_dir);
status_t dev_generate_api_docs(build_target_t* target, const char* output_dir);

/* Code Generation */
status_t dev_generate_class(dev_project_t* project, const char* class_name, programming_language_t language);
status_t dev_generate_interface(dev_project_t* project, const char* interface_name, programming_language_t language);
status_t dev_generate_makefile(build_target_t* target);
status_t dev_generate_cmake_lists(build_target_t* target);

/* Utilities */
const char* dev_get_language_name(programming_language_t language);
const char* dev_get_project_type_name(project_type_t type);
const char* dev_get_build_system_name(build_system_t build_system);
programming_language_t dev_detect_language_from_file(const char* filename);
bool dev_is_source_file(const char* filename);
bool dev_is_header_file(const char* filename);

/* Statistics and Reporting */
void dev_print_project_statistics(dev_project_t* project);
void dev_print_build_statistics(build_target_t* target);
void dev_generate_project_report(dev_project_t* project, const char* report_file);

#endif /* LIMITLESS_DEVELOPER_SDK_H */