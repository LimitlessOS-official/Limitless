/*
 * LimitlessOS Development Environment
 * Complete development tools and IDE infrastructure
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../apps/application_core.h"

/* Development tools constants */
#define MAX_PROJECTS            64
#define MAX_BUILD_TARGETS       32
#define MAX_BREAKPOINTS         256
#define MAX_WATCH_EXPRESSIONS   128
#define MAX_CALL_STACK_DEPTH    64
#define MAX_COMPILER_OUTPUT     65536

/* Programming languages */
typedef enum {
    LANG_UNKNOWN = 0,
    LANG_C,
    LANG_CPP,
    LANG_RUST,
    LANG_PYTHON,
    LANG_JAVASCRIPT,
    LANG_TYPESCRIPT,
    LANG_GO,
    LANG_JAVA,
    LANG_CSHARP,
    LANG_KOTLIN,
    LANG_SWIFT,
    LANG_PHP,
    LANG_RUBY,
    LANG_PERL,
    LANG_LUA,
    LANG_SHELL,
    LANG_ASSEMBLY,
    LANG_HTML,
    LANG_CSS,
    LANG_SQL,
    LANG_JSON,
    LANG_XML,
    LANG_YAML,
    LANG_MARKDOWN,
    LANG_MAX
} programming_language_t;

/* Build systems */
typedef enum {
    BUILD_MAKE = 0,
    BUILD_CMAKE,
    BUILD_NINJA,
    BUILD_CARGO,
    BUILD_NPM,
    BUILD_GRADLE,
    BUILD_MAVEN,
    BUILD_MESON,
    BUILD_AUTOTOOLS,
    BUILD_CUSTOM,
    BUILD_MAX
} build_system_t;

/* Compiler types */
typedef struct compiler_info {
    char name[64];              /* Compiler name */
    char executable[256];       /* Compiler executable */
    char version[32];           /* Version string */
    programming_language_t language; /* Target language */
    
    /* Supported features */
    struct {
        bool debug_info;        /* Debug information generation */
        bool optimization;      /* Code optimization */
        bool profiling;         /* Profiling support */
        bool static_analysis;   /* Static analysis */
        bool cross_compilation; /* Cross-compilation */
        bool lto;              /* Link-time optimization */
    } features;
    
    /* Default flags */
    char debug_flags[256];
    char release_flags[256];
    char warning_flags[256];
    
} compiler_info_t;

/* Debugger interface */
typedef struct debugger {
    char name[64];              /* Debugger name */
    char executable[256];       /* Debugger executable */
    
    /* Current debugging session */
    struct {
        struct process *target_process;
        bool attached;
        bool running;
        bool paused;
        
        /* Current state */
        uint64_t current_address;
        char current_function[128];
        char current_file[512];
        int current_line;
        
    } session;
    
    /* Breakpoints */
    struct {
        struct {
            uint64_t address;
            char file[512];
            int line;
            bool enabled;
            bool temporary;
            char condition[256];
            int hit_count;
        } breakpoints[MAX_BREAKPOINTS];
        int count;
    } breakpoints;
    
    /* Watch expressions */
    struct {
        struct {
            char expression[256];
            char value[512];
            bool valid;
        } watches[MAX_WATCH_EXPRESSIONS];
        int count;
    } watches;
    
    /* Call stack */
    struct {
        struct {
            uint64_t address;
            char function[128];
            char file[512];
            int line;
            char arguments[512];
        } frames[MAX_CALL_STACK_DEPTH];
        int depth;
        int current_frame;
    } call_stack;
    
    /* Memory view */
    struct {
        uint64_t base_address;
        size_t size;
        void *data;
        bool valid;
    } memory;
    
} debugger_t;

/* Code profiler */
typedef struct profiler {
    char name[64];
    bool active;
    
    /* Performance data */
    struct {
        struct {
            char function_name[128];
            uint64_t call_count;
            uint64_t total_time_ns;
            uint64_t self_time_ns;
            float percentage;
        } functions[1024];
        int function_count;
        
        uint64_t total_runtime_ns;
        uint64_t samples_taken;
        
    } performance;
    
    /* Memory profiling */
    struct {
        struct {
            uint64_t address;
            size_t size;
            char function[128];
            char file[512];
            int line;
            uint64_t timestamp;
        } allocations[4096];
        int allocation_count;
        
        uint64_t total_allocated;
        uint64_t total_freed;
        uint64_t peak_usage;
        
    } memory;
    
    /* Coverage analysis */
    struct {
        struct {
            char file[512];
            int *line_hits;
            int line_count;
            float coverage_percent;
        } files[256];
        int file_count;
        
        float overall_coverage;
        
    } coverage;
    
} profiler_t;

/* Project structure */
typedef struct development_project {
    char name[128];             /* Project name */
    char path[1024];            /* Project root path */
    programming_language_t primary_language;
    build_system_t build_system;
    
    /* Project files */
    struct {
        char source_files[512][512];
        char header_files[256][512];
        char resource_files[128][512];
        char config_files[32][512];
        int source_count;
        int header_count;
        int resource_count;
        int config_count;
    } files;
    
    /* Build configuration */
    struct {
        struct {
            char name[64];      /* Target name */
            enum {
                TARGET_EXECUTABLE = 0,
                TARGET_LIBRARY,
                TARGET_TEST,
                TARGET_CUSTOM
            } type;
            char output_path[512];
            char compiler_flags[512];
            char linker_flags[512];
            char dependencies[32][128];
            int dependency_count;
        } targets[MAX_BUILD_TARGETS];
        int target_count;
        
        char build_directory[512];
        bool debug_build;
        
    } build;
    
    /* Version control */
    struct {
        enum {
            VCS_NONE = 0,
            VCS_GIT,
            VCS_SVN,
            VCS_MERCURIAL
        } type;
        
        char repository_url[512];
        char current_branch[128];
        char current_commit[64];
        bool has_changes;
        
        struct {
            char files[128][512];
            int count;
        } modified_files;
        
    } vcs;
    
    /* Dependencies */
    struct {
        struct {
            char name[128];
            char version[32];
            char url[512];
            bool installed;
        } packages[64];
        int count;
    } dependencies;
    
    /* Recent activity */
    struct {
        char recent_files[32][512];
        int file_count;
        uint64_t last_opened;
    } recent;
    
} development_project_t;

/* Integrated Development Environment */
typedef struct ide {
    struct window *main_window;
    
    /* Current project */
    development_project_t *current_project;
    
    /* Open projects */
    struct {
        development_project_t *projects[MAX_PROJECTS];
        int count;
    } projects;
    
    /* Editor */
    struct {
        text_editor_t *editor;
        
        /* Code assistance */
        struct {
            bool autocomplete_enabled;
            bool syntax_checking_enabled;
            bool code_formatting_enabled;
            bool refactoring_enabled;
            
            struct {
                char suggestions[32][256];
                int count;
                int selected;
                bool visible;
            } autocomplete;
            
            struct {
                struct {
                    char file[512];
                    int line;
                    int column;
                    char message[512];
                    enum {
                        SEVERITY_ERROR = 0,
                        SEVERITY_WARNING,
                        SEVERITY_INFO,
                        SEVERITY_HINT
                    } severity;
                } diagnostics[256];
                int count;
            } diagnostics;
            
        } assistance;
        
    } editor;
    
    /* Build system */
    struct {
        compiler_info_t compilers[16];
        int compiler_count;
        
        struct {
            bool building;
            char output[MAX_COMPILER_OUTPUT];
            int exit_code;
            uint64_t start_time;
            uint64_t duration_ms;
        } current_build;
        
        struct {
            char target_name[64];
            bool success;
            uint64_t timestamp;
            char output[1024];
        } build_history[32];
        int history_count;
        
    } build;
    
    /* Debugger integration */
    debugger_t debugger;
    
    /* Profiler integration */
    profiler_t profiler;
    
    /* Version control */
    struct {
        bool enabled;
        char current_branch[128];
        bool has_changes;
        
        struct {
            char files[64][512];
            enum {
                STATUS_MODIFIED = 0,
                STATUS_ADDED,
                STATUS_DELETED,
                STATUS_RENAMED,
                STATUS_UNTRACKED
            } status[64];
            int count;
        } changes;
        
        struct {
            char hash[64];
            char message[256];
            char author[128];
            uint64_t timestamp;
        } commits[128];
        int commit_count;
        
    } vcs;
    
    /* Plugin system */
    struct {
        struct {
            char name[128];
            char version[32];
            char description[512];
            bool enabled;
            void *plugin_data;
        } plugins[64];
        int count;
    } plugins;
    
    /* Settings */
    struct {
        char default_encoding[32];
        bool auto_save;
        int auto_save_interval_s;
        bool show_line_numbers;
        bool show_minimap;
        bool word_wrap;
        char font_family[64];
        int font_size;
        struct theme code_theme;
        
        /* Behavior */
        bool auto_indent;
        bool smart_indent;
        int tab_size;
        bool use_spaces;
        bool trim_whitespace;
        bool auto_complete_brackets;
        
    } settings;
    
} ide_t;

/* Language Server Protocol (LSP) */
typedef struct language_server {
    char name[64];
    char executable[256];
    programming_language_t language;
    
    /* Server process */
    struct {
        struct process *process;
        int stdin_fd;
        int stdout_fd;
        int stderr_fd;
        bool running;
    } server;
    
    /* Capabilities */
    struct {
        bool completion;
        bool hover;
        bool signature_help;
        bool goto_definition;
        bool goto_references;
        bool document_symbols;
        bool workspace_symbols;
        bool code_actions;
        bool code_lens;
        bool formatting;
        bool range_formatting;
        bool rename;
        bool folding_range;
        bool selection_range;
    } capabilities;
    
    /* Active requests */
    struct {
        struct {
            int request_id;
            char method[64];
            uint64_t timestamp;
        } requests[32];
        int count;
    } requests;
    
} language_server_t;

/* Package manager */
typedef struct package_manager {
    char name[64];              /* Manager name (npm, cargo, pip, etc.) */
    char executable[256];       /* Manager executable */
    programming_language_t language;
    
    /* Repositories */
    struct {
        char urls[16][512];
        int count;
    } repositories;
    
    /* Installed packages */
    struct {
        struct {
            char name[128];
            char version[32];
            char description[512];
            uint64_t install_date;
            bool user_installed;
        } packages[1024];
        int count;
    } installed;
    
    /* Available updates */
    struct {
        struct {
            char name[128];
            char current_version[32];
            char new_version[32];
        } updates[256];
        int count;
    } updates;
    
} package_manager_t;

/* Global development environment */
typedef struct development_environment {
    bool initialized;
    
    /* Compilers */
    struct {
        compiler_info_t compilers[32];
        int count;
    } compilers;
    
    /* Language servers */
    struct {
        language_server_t servers[16];
        int count;
    } language_servers;
    
    /* Package managers */
    struct {
        package_manager_t managers[16];
        int count;
    } package_managers;
    
    /* Global tools */
    struct {
        char git_path[256];
        char make_path[256];
        char cmake_path[256];
        char gdb_path[256];
        char valgrind_path[256];
        char lldb_path[256];
    } tools;
    
    /* Documentation */
    struct {
        struct {
            char name[128];
            char path[512];
            programming_language_t language;
        } docs[32];
        int count;
    } documentation;
    
} development_environment_t;

/* External development environment */
extern development_environment_t dev_env;

/* Core development functions */
int development_init(void);
void development_exit(void);

/* Compiler management */
int dev_register_compiler(const struct compiler_info *compiler);
struct compiler_info *dev_find_compiler(programming_language_t language);
int dev_compile_project(struct development_project *project, const char *target);
int dev_get_compiler_output(char *output, size_t size);

/* Project management */
struct development_project *dev_create_project(const char *name, const char *path, programming_language_t language);
void dev_destroy_project(struct development_project *project);
int dev_open_project(const char *path);
int dev_close_project(struct development_project *project);
int dev_save_project(struct development_project *project);
int dev_build_project(struct development_project *project);
int dev_clean_project(struct development_project *project);
int dev_run_project(struct development_project *project);

/* IDE functions */
ide_t *ide_create(void);
void ide_destroy(ide_t *ide);
int ide_open_file(ide_t *ide, const char *filename);
int ide_save_file(ide_t *ide);
int ide_close_file(ide_t *ide);
int ide_find_in_files(ide_t *ide, const char *pattern, const char *directory);
int ide_replace_in_files(ide_t *ide, const char *find_text, const char *replace_text);
int ide_goto_line(ide_t *ide, int line);
int ide_goto_definition(ide_t *ide, const char *symbol);
int ide_find_references(ide_t *ide, const char *symbol);

/* Debugger functions */
int debugger_init(debugger_t *debugger);
int debugger_attach(debugger_t *debugger, struct process *process);
int debugger_detach(debugger_t *debugger);
int debugger_start(debugger_t *debugger, const char *executable, const char *args[]);
int debugger_stop(debugger_t *debugger);
int debugger_continue(debugger_t *debugger);
int debugger_step_over(debugger_t *debugger);
int debugger_step_into(debugger_t *debugger);
int debugger_step_out(debugger_t *debugger);
int debugger_set_breakpoint(debugger_t *debugger, const char *file, int line);
int debugger_remove_breakpoint(debugger_t *debugger, const char *file, int line);
int debugger_add_watch(debugger_t *debugger, const char *expression);
int debugger_remove_watch(debugger_t *debugger, const char *expression);
int debugger_evaluate_expression(debugger_t *debugger, const char *expression, char *result, size_t size);

/* Profiler functions */
int profiler_init(profiler_t *profiler);
int profiler_start(profiler_t *profiler, struct process *process);
int profiler_stop(profiler_t *profiler);
int profiler_get_performance_data(profiler_t *profiler, void *data, size_t size);
int profiler_get_memory_data(profiler_t *profiler, void *data, size_t size);
int profiler_generate_report(profiler_t *profiler, const char *filename);

/* Language Server Protocol */
int lsp_start_server(language_server_t *server, programming_language_t language);
int lsp_stop_server(language_server_t *server);
int lsp_send_request(language_server_t *server, const char *method, const char *params);
int lsp_get_completion(language_server_t *server, const char *file, int line, int column, char results[][256], int max_results);
int lsp_get_hover_info(language_server_t *server, const char *file, int line, int column, char *info, size_t size);
int lsp_goto_definition(language_server_t *server, const char *file, int line, int column, char *def_file, int *def_line);
int lsp_find_references(language_server_t *server, const char *file, int line, int column, void *references, int max_refs);

/* Version control */
int vcs_init_repository(const char *path, int vcs_type);
int vcs_clone_repository(const char *url, const char *path);
int vcs_commit_changes(const char *path, const char *message);
int vcs_push_changes(const char *path);
int vcs_pull_changes(const char *path);
int vcs_create_branch(const char *path, const char *branch_name);
int vcs_switch_branch(const char *path, const char *branch_name);
int vcs_get_status(const char *path, char status[][512], int max_files);
int vcs_get_log(const char *path, void *commits, int max_commits);

/* Package management */
int pkg_install_package(const char *package_name, programming_language_t language);
int pkg_uninstall_package(const char *package_name, programming_language_t language);
int pkg_update_package(const char *package_name, programming_language_t language);
int pkg_list_packages(programming_language_t language, char packages[][128], int max_packages);
int pkg_search_packages(const char *query, programming_language_t language, char results[][256], int max_results);

/* Code analysis */
int dev_analyze_code(const char *filename, programming_language_t language);
int dev_check_syntax(const char *filename, programming_language_t language);
int dev_format_code(const char *filename, programming_language_t language);
int dev_refactor_rename(const char *filename, const char *old_name, const char *new_name);
int dev_find_unused_code(const char *directory);
int dev_generate_documentation(const char *directory, const char *output_path);

/* Build systems */
int build_detect_system(const char *project_path);
int build_configure_project(const char *project_path, build_system_t build_system);
int build_generate_makefile(const char *project_path);
int build_clean_build_files(const char *project_path);

/* Testing framework */
int test_discover_tests(const char *project_path);
int test_run_tests(const char *project_path, const char *test_pattern);
int test_run_single_test(const char *project_path, const char *test_name);
int test_get_test_results(void *results, size_t size);
int test_generate_coverage_report(const char *project_path, const char *output_path);

/* Documentation tools */
int doc_generate_api_docs(const char *source_path, const char *output_path, programming_language_t language);
int doc_extract_comments(const char *filename, char comments[][512], int max_comments);
int doc_validate_documentation(const char *project_path);

/* Utility functions */
const char *programming_language_name(programming_language_t language);
const char *build_system_name(build_system_t build_system);
programming_language_t detect_language_from_extension(const char *filename);
build_system_t detect_build_system(const char *project_path);
int get_compiler_version(const char *compiler, char *version, size_t size);
bool is_source_file(const char *filename);
bool is_header_file(const char *filename);

/* Plugin interface */
typedef struct dev_plugin {
    char name[128];
    char version[32];
    char author[128];
    char description[512];
    
    /* Plugin callbacks */
    int (*init)(void);
    void (*cleanup)(void);
    int (*on_file_open)(const char *filename);
    int (*on_file_save)(const char *filename);
    int (*on_build_start)(const char *project_path);
    int (*on_build_complete)(const char *project_path, int result);
    
} dev_plugin_t;

int dev_load_plugin(const char *plugin_path);
int dev_unload_plugin(const char *plugin_name);
int dev_list_plugins(char plugins[][128], int max_plugins);
int dev_enable_plugin(const char *plugin_name);
int dev_disable_plugin(const char *plugin_name);