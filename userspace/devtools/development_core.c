/*
 * LimitlessOS Development Environment Implementation
 * Complete IDE, compiler toolchain, and development tools
 */

#include "development_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

/* Global development environment */
development_environment_t dev_env = {0};

/* Compiler database */
static const struct {
    const char *name;
    const char *executable;
    programming_language_t language;
    const char *debug_flags;
    const char *release_flags;
    const char *warning_flags;
} builtin_compilers[] = {
    {"GCC", "gcc", LANG_C, "-g -O0", "-O2 -DNDEBUG", "-Wall -Wextra -Werror"},
    {"G++", "g++", LANG_CPP, "-g -O0 -std=c++17", "-O2 -DNDEBUG -std=c++17", "-Wall -Wextra -Werror"},
    {"Clang", "clang", LANG_C, "-g -O0", "-O2 -DNDEBUG", "-Wall -Wextra -Werror"},
    {"Clang++", "clang++", LANG_CPP, "-g -O0 -std=c++17", "-O2 -DNDEBUG -std=c++17", "-Wall -Wextra -Werror"},
    {"Rustc", "rustc", LANG_RUST, "-g", "-C opt-level=2", "-W warnings"},
    {"Python", "python3", LANG_PYTHON, "", "-O", ""},
    {"Node.js", "node", LANG_JAVASCRIPT, "", "", ""},
    {"Go", "go build", LANG_GO, "-gcflags=-N", "-ldflags=-s", ""},
    {"Java", "javac", LANG_JAVA, "-g", "-O", "-Xlint:all"},
    {NULL, NULL, LANG_UNKNOWN, NULL, NULL, NULL}
};

/* Language Server configurations */
static const struct {
    const char *name;
    const char *executable;
    programming_language_t language;
} builtin_language_servers[] = {
    {"clangd", "clangd", LANG_C},
    {"clangd", "clangd", LANG_CPP},
    {"rust-analyzer", "rust-analyzer", LANG_RUST},
    {"pylsp", "pylsp", LANG_PYTHON},
    {"typescript-language-server", "typescript-language-server", LANG_TYPESCRIPT},
    {"gopls", "gopls", LANG_GO},
    {"jdtls", "jdtls", LANG_JAVA},
    {NULL, NULL, LANG_UNKNOWN}
};

/* Initialize development environment */
int development_init(void) {
    memset(&dev_env, 0, sizeof(dev_env));
    
    /* Detect available compilers */
    for (int i = 0; builtin_compilers[i].name; i++) {
        if (access(builtin_compilers[i].executable, X_OK) == 0) {
            compiler_info_t compiler = {0};
            
            strncpy(compiler.name, builtin_compilers[i].name, sizeof(compiler.name) - 1);
            strncpy(compiler.executable, builtin_compilers[i].executable, sizeof(compiler.executable) - 1);
            compiler.language = builtin_compilers[i].language;
            
            strncpy(compiler.debug_flags, builtin_compilers[i].debug_flags, sizeof(compiler.debug_flags) - 1);
            strncpy(compiler.release_flags, builtin_compilers[i].release_flags, sizeof(compiler.release_flags) - 1);
            strncpy(compiler.warning_flags, builtin_compilers[i].warning_flags, sizeof(compiler.warning_flags) - 1);
            
            /* Set features */
            compiler.features.debug_info = true;
            compiler.features.optimization = true;
            compiler.features.profiling = true;
            compiler.features.static_analysis = true;
            
            /* Get version */
            char cmd[512];
            snprintf(cmd, sizeof(cmd), "%s --version 2>&1 | head -1", compiler.executable);
            FILE *fp = popen(cmd, "r");
            if (fp) {
                fgets(compiler.version, sizeof(compiler.version), fp);
                pclose(fp);
                
                /* Clean up version string */
                char *newline = strchr(compiler.version, '\n');
                if (newline) *newline = '\0';
            }
            
            dev_env.compilers.compilers[dev_env.compilers.count++] = compiler;
            if (dev_env.compilers.count >= 32) break;
        }
    }
    
    /* Detect language servers */
    for (int i = 0; builtin_language_servers[i].name; i++) {
        if (access(builtin_language_servers[i].executable, X_OK) == 0) {
            language_server_t server = {0};
            
            strncpy(server.name, builtin_language_servers[i].name, sizeof(server.name) - 1);
            strncpy(server.executable, builtin_language_servers[i].executable, sizeof(server.executable) - 1);
            server.language = builtin_language_servers[i].language;
            
            /* Set common capabilities */
            server.capabilities.completion = true;
            server.capabilities.hover = true;
            server.capabilities.signature_help = true;
            server.capabilities.goto_definition = true;
            server.capabilities.goto_references = true;
            server.capabilities.document_symbols = true;
            server.capabilities.formatting = true;
            
            dev_env.language_servers.servers[dev_env.language_servers.count++] = server;
            if (dev_env.language_servers.count >= 16) break;
        }
    }
    
    /* Detect common development tools */
    if (access("/usr/bin/git", X_OK) == 0) {
        strcpy(dev_env.tools.git_path, "/usr/bin/git");
    }
    if (access("/usr/bin/make", X_OK) == 0) {
        strcpy(dev_env.tools.make_path, "/usr/bin/make");
    }
    if (access("/usr/bin/cmake", X_OK) == 0) {
        strcpy(dev_env.tools.cmake_path, "/usr/bin/cmake");
    }
    if (access("/usr/bin/gdb", X_OK) == 0) {
        strcpy(dev_env.tools.gdb_path, "/usr/bin/gdb");
    }
    
    dev_env.initialized = true;
    return 0;
}

/* Cleanup development environment */
void development_exit(void) {
    /* Stop all language servers */
    for (int i = 0; i < dev_env.language_servers.count; i++) {
        lsp_stop_server(&dev_env.language_servers.servers[i]);
    }
    
    memset(&dev_env, 0, sizeof(dev_env));
}

/* Find compiler for language */
struct compiler_info *dev_find_compiler(programming_language_t language) {
    for (int i = 0; i < dev_env.compilers.count; i++) {
        if (dev_env.compilers.compilers[i].language == language) {
            return &dev_env.compilers.compilers[i];
        }
    }
    return NULL;
}

/* Compile project */
int dev_compile_project(struct development_project *project, const char *target) {
    if (!project) return -EINVAL;
    
    struct compiler_info *compiler = dev_find_compiler(project->primary_language);
    if (!compiler) return -ENOENT;
    
    /* Build command line */
    char cmd[4096] = {0};
    size_t cmd_len = 0;
    
    /* Add compiler */
    cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, "%s ", compiler->executable);
    
    /* Add flags based on build type */
    if (project->build.debug_build) {
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, "%s ", compiler->debug_flags);
    } else {
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, "%s ", compiler->release_flags);
    }
    
    cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, "%s ", compiler->warning_flags);
    
    /* Find target configuration */
    int target_index = -1;
    for (int i = 0; i < project->build.target_count; i++) {
        if (strcmp(project->build.targets[i].name, target) == 0) {
            target_index = i;
            break;
        }
    }
    
    if (target_index >= 0) {
        /* Add target-specific flags */
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, "%s ", 
                          project->build.targets[target_index].compiler_flags);
        
        /* Add source files */
        for (int i = 0; i < project->files.source_count; i++) {
            cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, "%s ", 
                              project->files.source_files[i]);
        }
        
        /* Add output specification */
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, "-o %s ", 
                          project->build.targets[target_index].output_path);
        
        /* Add linker flags */
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, "%s", 
                          project->build.targets[target_index].linker_flags);
    }
    
    /* Execute compilation */
    printf("Compiling: %s\n", cmd);
    int result = system(cmd);
    
    return WIFEXITED(result) ? WEXITSTATUS(result) : -1;
}

/* Create new project */
struct development_project *dev_create_project(const char *name, const char *path, 
                                             programming_language_t language) {
    struct development_project *project = calloc(1, sizeof(struct development_project));
    if (!project) return NULL;
    
    strncpy(project->name, name, sizeof(project->name) - 1);
    strncpy(project->path, path, sizeof(project->path) - 1);
    project->primary_language = language;
    
    /* Set default build system based on language */
    switch (language) {
        case LANG_C:
        case LANG_CPP:
            project->build_system = BUILD_MAKE;
            break;
        case LANG_RUST:
            project->build_system = BUILD_CARGO;
            break;
        case LANG_JAVASCRIPT:
        case LANG_TYPESCRIPT:
            project->build_system = BUILD_NPM;
            break;
        case LANG_JAVA:
            project->build_system = BUILD_GRADLE;
            break;
        default:
            project->build_system = BUILD_MAKE;
            break;
    }
    
    /* Create default build target */
    strncpy(project->build.targets[0].name, "default", sizeof(project->build.targets[0].name) - 1);
    project->build.targets[0].type = TARGET_EXECUTABLE;
    snprintf(project->build.targets[0].output_path, sizeof(project->build.targets[0].output_path), 
             "%s/%s", path, name);
    project->build.target_count = 1;
    
    /* Set build directory */
    snprintf(project->build.build_directory, sizeof(project->build.build_directory), 
             "%s/build", path);
    project->build.debug_build = true;
    
    return project;
}

/* Create IDE instance */
ide_t *ide_create(void) {
    ide_t *ide = calloc(1, sizeof(ide_t));
    if (!ide) return NULL;
    
    /* Initialize editor */
    ide->editor.editor = text_editor_create();
    if (!ide->editor.editor) {
        free(ide);
        return NULL;
    }
    
    /* Set default settings */
    strcpy(ide->settings.default_encoding, "UTF-8");
    ide->settings.auto_save = true;
    ide->settings.auto_save_interval_s = 30;
    ide->settings.show_line_numbers = true;
    ide->settings.show_minimap = true;
    ide->settings.word_wrap = false;
    strcpy(ide->settings.font_family, "Consolas");
    ide->settings.font_size = 12;
    ide->settings.auto_indent = true;
    ide->settings.smart_indent = true;
    ide->settings.tab_size = 4;
    ide->settings.use_spaces = true;
    ide->settings.trim_whitespace = true;
    ide->settings.auto_complete_brackets = true;
    
    /* Enable code assistance */
    ide->editor.assistance.autocomplete_enabled = true;
    ide->editor.assistance.syntax_checking_enabled = true;
    ide->editor.assistance.code_formatting_enabled = true;
    ide->editor.assistance.refactoring_enabled = true;
    
    /* Initialize debugger */
    debugger_init(&ide->debugger);
    
    /* Initialize profiler */
    profiler_init(&ide->profiler);
    
    /* Copy available compilers */
    memcpy(&ide->build.compilers, &dev_env.compilers.compilers, 
           dev_env.compilers.count * sizeof(compiler_info_t));
    ide->build.compiler_count = dev_env.compilers.count;
    
    return ide;
}

/* Destroy IDE instance */
void ide_destroy(ide_t *ide) {
    if (!ide) return;
    
    /* Close all projects */
    for (int i = 0; i < ide->projects.count; i++) {
        dev_destroy_project(ide->projects.projects[i]);
    }
    
    /* Cleanup editor */
    if (ide->editor.editor) {
        text_editor_destroy(ide->editor.editor);
    }
    
    /* Stop debugger if running */
    debugger_detach(&ide->debugger);
    
    /* Stop profiler if running */
    profiler_stop(&ide->profiler);
    
    free(ide);
}

/* Initialize debugger */
int debugger_init(debugger_t *debugger) {
    if (!debugger) return -EINVAL;
    
    memset(debugger, 0, sizeof(debugger_t));
    strcpy(debugger->name, "GDB");
    strcpy(debugger->executable, "/usr/bin/gdb");
    
    return 0;
}

/* Start debugging session */
int debugger_start(debugger_t *debugger, const char *executable, const char *args[]) {
    if (!debugger || !executable) return -EINVAL;
    
    /* Create GDB command */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s --batch --quiet %s", debugger->executable, executable);
    
    /* Start GDB process */
    int pipefd[2];
    if (pipe(pipefd) == -1) return -errno;
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process - run GDB */
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        _exit(127);
        
    } else if (pid > 0) {
        /* Parent process */
        close(pipefd[1]);
        debugger->session.attached = true;
        debugger->session.running = false;
        debugger->session.paused = true;
        
        return 0;
    }
    
    return -errno;
}

/* Set breakpoint */
int debugger_set_breakpoint(debugger_t *debugger, const char *file, int line) {
    if (!debugger || !file) return -EINVAL;
    if (debugger->breakpoints.count >= MAX_BREAKPOINTS) return -ENOMEM;
    
    int index = debugger->breakpoints.count++;
    strncpy(debugger->breakpoints.breakpoints[index].file, file, 
            sizeof(debugger->breakpoints.breakpoints[index].file) - 1);
    debugger->breakpoints.breakpoints[index].line = line;
    debugger->breakpoints.breakpoints[index].enabled = true;
    debugger->breakpoints.breakpoints[index].temporary = false;
    debugger->breakpoints.breakpoints[index].hit_count = 0;
    
    return index;
}

/* Initialize profiler */
int profiler_init(profiler_t *profiler) {
    if (!profiler) return -EINVAL;
    
    memset(profiler, 0, sizeof(profiler_t));
    strcpy(profiler->name, "perf");
    
    return 0;
}

/* Start Language Server */
int lsp_start_server(language_server_t *server, programming_language_t language) {
    if (!server) return -EINVAL;
    
    /* Find server for language */
    for (int i = 0; i < dev_env.language_servers.count; i++) {
        if (dev_env.language_servers.servers[i].language == language) {
            *server = dev_env.language_servers.servers[i];
            break;
        }
    }
    
    /* Start server process */
    int stdin_pipe[2], stdout_pipe[2];
    if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1) {
        return -errno;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process - run language server */
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        
        execl(server->executable, server->executable, (char *)NULL);
        _exit(127);
        
    } else if (pid > 0) {
        /* Parent process */
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        
        server->server.stdin_fd = stdin_pipe[1];
        server->server.stdout_fd = stdout_pipe[0];
        server->server.running = true;
        
        return 0;
    }
    
    return -errno;
}

/* Programming language detection */
programming_language_t detect_language_from_extension(const char *filename) {
    if (!filename) return LANG_UNKNOWN;
    
    const char *ext = strrchr(filename, '.');
    if (!ext) return LANG_UNKNOWN;
    ext++; /* Skip the dot */
    
    if (strcmp(ext, "c") == 0) return LANG_C;
    if (strcmp(ext, "cpp") == 0 || strcmp(ext, "cxx") == 0 || strcmp(ext, "cc") == 0) return LANG_CPP;
    if (strcmp(ext, "h") == 0 || strcmp(ext, "hpp") == 0) return LANG_C; /* Could be C or C++ */
    if (strcmp(ext, "rs") == 0) return LANG_RUST;
    if (strcmp(ext, "py") == 0) return LANG_PYTHON;
    if (strcmp(ext, "js") == 0) return LANG_JAVASCRIPT;
    if (strcmp(ext, "ts") == 0) return LANG_TYPESCRIPT;
    if (strcmp(ext, "go") == 0) return LANG_GO;
    if (strcmp(ext, "java") == 0) return LANG_JAVA;
    if (strcmp(ext, "cs") == 0) return LANG_CSHARP;
    if (strcmp(ext, "kt") == 0) return LANG_KOTLIN;
    if (strcmp(ext, "swift") == 0) return LANG_SWIFT;
    if (strcmp(ext, "php") == 0) return LANG_PHP;
    if (strcmp(ext, "rb") == 0) return LANG_RUBY;
    if (strcmp(ext, "pl") == 0) return LANG_PERL;
    if (strcmp(ext, "lua") == 0) return LANG_LUA;
    if (strcmp(ext, "sh") == 0 || strcmp(ext, "bash") == 0) return LANG_SHELL;
    if (strcmp(ext, "asm") == 0 || strcmp(ext, "s") == 0) return LANG_ASSEMBLY;
    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) return LANG_HTML;
    if (strcmp(ext, "css") == 0) return LANG_CSS;
    if (strcmp(ext, "sql") == 0) return LANG_SQL;
    if (strcmp(ext, "json") == 0) return LANG_JSON;
    if (strcmp(ext, "xml") == 0) return LANG_XML;
    if (strcmp(ext, "yaml") == 0 || strcmp(ext, "yml") == 0) return LANG_YAML;
    if (strcmp(ext, "md") == 0) return LANG_MARKDOWN;
    
    return LANG_UNKNOWN;
}

/* Get language name */
const char *programming_language_name(programming_language_t language) {
    static const char *names[] = {
        "Unknown", "C", "C++", "Rust", "Python", "JavaScript", "TypeScript",
        "Go", "Java", "C#", "Kotlin", "Swift", "PHP", "Ruby", "Perl", "Lua",
        "Shell", "Assembly", "HTML", "CSS", "SQL", "JSON", "XML", "YAML", "Markdown"
    };
    
    if (language >= 0 && language < LANG_MAX) {
        return names[language];
    }
    return names[0];
}

/* Get build system name */
const char *build_system_name(build_system_t build_system) {
    static const char *names[] = {
        "Make", "CMake", "Ninja", "Cargo", "npm", "Gradle", "Maven", 
        "Meson", "Autotools", "Custom"
    };
    
    if (build_system >= 0 && build_system < BUILD_MAX) {
        return names[build_system];
    }
    return names[0];
}

/* Build project */
int dev_build_project(struct development_project *project) {
    if (!project) return -EINVAL;
    
    int result = 0;
    char cmd[1024];
    
    switch (project->build_system) {
        case BUILD_MAKE:
            snprintf(cmd, sizeof(cmd), "cd %s && make", project->path);
            break;
            
        case BUILD_CMAKE:
            snprintf(cmd, sizeof(cmd), "cd %s && mkdir -p build && cd build && cmake .. && make", 
                    project->path);
            break;
            
        case BUILD_CARGO:
            snprintf(cmd, sizeof(cmd), "cd %s && cargo build", project->path);
            break;
            
        case BUILD_NPM:
            snprintf(cmd, sizeof(cmd), "cd %s && npm run build", project->path);
            break;
            
        case BUILD_GRADLE:
            snprintf(cmd, sizeof(cmd), "cd %s && ./gradlew build", project->path);
            break;
            
        default:
            return dev_compile_project(project, "default");
    }
    
    printf("Building project: %s\n", cmd);
    result = system(cmd);
    
    return WIFEXITED(result) ? WEXITSTATUS(result) : -1;
}

/* Run project */
int dev_run_project(struct development_project *project) {
    if (!project) return -EINVAL;
    
    /* Find executable target */
    for (int i = 0; i < project->build.target_count; i++) {
        if (project->build.targets[i].type == TARGET_EXECUTABLE) {
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), "cd %s && %s", 
                    project->path, project->build.targets[i].output_path);
            
            printf("Running: %s\n", cmd);
            return system(cmd);
        }
    }
    
    return -ENOENT;
}

/* Open file in IDE */
int ide_open_file(ide_t *ide, const char *filename) {
    if (!ide || !filename) return -EINVAL;
    
    return text_editor_open_file(ide->editor.editor, filename);
}

/* Save file in IDE */
int ide_save_file(ide_t *ide) {
    if (!ide) return -EINVAL;
    
    return text_editor_save_file(ide->editor.editor);
}

/* Find in files */
int ide_find_in_files(ide_t *ide, const char *pattern, const char *directory) {
    if (!ide || !pattern || !directory) return -EINVAL;
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "grep -r \"%s\" %s", pattern, directory);
    
    return system(cmd);
}

/* Goto definition using LSP */
int ide_goto_definition(ide_t *ide, const char *symbol) {
    if (!ide || !symbol) return -EINVAL;
    
    /* This would integrate with the Language Server Protocol */
    /* For now, use simple grep-based search */
    char cmd[1024];
    if (ide->current_project) {
        snprintf(cmd, sizeof(cmd), "grep -rn \"^[^/]*%s\" %s", 
                symbol, ide->current_project->path);
    } else {
        snprintf(cmd, sizeof(cmd), "grep -rn \"^[^/]*%s\" .", symbol);
    }
    
    return system(cmd);
}

/* Install package */
int pkg_install_package(const char *package_name, programming_language_t language) {
    if (!package_name) return -EINVAL;
    
    char cmd[512];
    
    switch (language) {
        case LANG_PYTHON:
            snprintf(cmd, sizeof(cmd), "pip install %s", package_name);
            break;
            
        case LANG_JAVASCRIPT:
        case LANG_TYPESCRIPT:
            snprintf(cmd, sizeof(cmd), "npm install %s", package_name);
            break;
            
        case LANG_RUST:
            snprintf(cmd, sizeof(cmd), "cargo add %s", package_name);
            break;
            
        case LANG_GO:
            snprintf(cmd, sizeof(cmd), "go get %s", package_name);
            break;
            
        default:
            return -ENOTSUP;
    }
    
    printf("Installing package: %s\n", cmd);
    int result = system(cmd);
    
    return WIFEXITED(result) ? WEXITSTATUS(result) : -1;
}

/* Version control operations */
int vcs_init_repository(const char *path, int vcs_type) {
    if (!path) return -EINVAL;
    
    char cmd[512];
    
    switch (vcs_type) {
        case VCS_GIT:
            snprintf(cmd, sizeof(cmd), "cd %s && git init", path);
            break;
            
        default:
            return -ENOTSUP;
    }
    
    return system(cmd);
}

int vcs_commit_changes(const char *path, const char *message) {
    if (!path || !message) return -EINVAL;
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "cd %s && git add -A && git commit -m \"%s\"", 
            path, message);
    
    return system(cmd);
}

/* Format code */
int dev_format_code(const char *filename, programming_language_t language) {
    if (!filename) return -EINVAL;
    
    char cmd[512];
    
    switch (language) {
        case LANG_C:
        case LANG_CPP:
            snprintf(cmd, sizeof(cmd), "clang-format -i %s", filename);
            break;
            
        case LANG_RUST:
            snprintf(cmd, sizeof(cmd), "rustfmt %s", filename);
            break;
            
        case LANG_PYTHON:
            snprintf(cmd, sizeof(cmd), "black %s", filename);
            break;
            
        case LANG_JAVASCRIPT:
        case LANG_TYPESCRIPT:
            snprintf(cmd, sizeof(cmd), "prettier --write %s", filename);
            break;
            
        case LANG_GO:
            snprintf(cmd, sizeof(cmd), "gofmt -w %s", filename);
            break;
            
        default:
            return -ENOTSUP;
    }
    
    return system(cmd);
}

/* Check if file is source code */
bool is_source_file(const char *filename) {
    if (!filename) return false;
    
    programming_language_t lang = detect_language_from_extension(filename);
    return lang != LANG_UNKNOWN && lang != LANG_HTML && lang != LANG_CSS && 
           lang != LANG_JSON && lang != LANG_XML && lang != LANG_YAML && 
           lang != LANG_MARKDOWN;
}

bool is_header_file(const char *filename) {
    if (!filename) return false;
    
    const char *ext = strrchr(filename, '.');
    if (!ext) return false;
    ext++;
    
    return strcmp(ext, "h") == 0 || strcmp(ext, "hpp") == 0 || 
           strcmp(ext, "hxx") == 0 || strcmp(ext, "hh") == 0;
}