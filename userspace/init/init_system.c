/*
 * init_system.c - LimitlessOS Service Management and Init System
 * 
 * Provides dependency graph management, restart policies, socket activation,
 * and comprehensive service lifecycle management.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <yaml.h>

#define MAX_SERVICES 256
#define MAX_DEPENDENCIES 32
#define MAX_SERVICE_NAME 128
#define MAX_COMMAND_LINE 512
#define MAX_ENVIRONMENT 64

/* Service states */
typedef enum {
    SERVICE_STOPPED = 0,
    SERVICE_STARTING,
    SERVICE_RUNNING,
    SERVICE_STOPPING,
    SERVICE_FAILED,
    SERVICE_RESTARTING
} service_state_t;

/* Service types */
typedef enum {
    SERVICE_TYPE_SIMPLE = 0,    /* Process doesn't fork */
    SERVICE_TYPE_FORKING,       /* Process forks and exits */
    SERVICE_TYPE_ONESHOT,       /* Process runs once and exits */
    SERVICE_TYPE_NOTIFY,        /* Process sends ready notification */
    SERVICE_TYPE_IDLE           /* Process runs when system is idle */
} service_type_t;

/* Restart policies */
typedef enum {
    RESTART_NO = 0,             /* Never restart */
    RESTART_ON_FAILURE,         /* Restart only on failure */
    RESTART_ON_ABNORMAL,        /* Restart on abnormal exit */
    RESTART_ON_WATCHDOG,        /* Restart on watchdog timeout */
    RESTART_ON_ABORT,           /* Restart on abort signal */
    RESTART_ALWAYS              /* Always restart */
} restart_policy_t;

/* Socket activation */
typedef struct socket_activation {
    char name[64];
    int type;                   /* SOCK_STREAM, SOCK_DGRAM */
    char address[256];          /* Path for unix, addr:port for inet */
    int fd;                     /* Socket file descriptor */
    struct socket_activation* next;
} socket_activation_t;

/* Service configuration */
typedef struct service {
    char name[MAX_SERVICE_NAME];
    char description[256];
    char command[MAX_COMMAND_LINE];
    char working_directory[256];
    char user[64];
    char group[64];
    
    service_type_t type;
    restart_policy_t restart;
    uint32_t restart_delay_ms;
    uint32_t restart_limit;
    uint32_t restart_count;
    
    /* Dependencies */
    char requires[MAX_DEPENDENCIES][MAX_SERVICE_NAME];
    uint32_t requires_count;
    char after[MAX_DEPENDENCIES][MAX_SERVICE_NAME];
    uint32_t after_count;
    char before[MAX_DEPENDENCIES][MAX_SERVICE_NAME];
    uint32_t before_count;
    
    /* Runtime state */
    service_state_t state;
    pid_t pid;
    uint64_t start_time;
    uint64_t stop_time;
    int exit_code;
    
    /* Socket activation */
    socket_activation_t* sockets;
    
    /* Environment */
    char environment[MAX_ENVIRONMENT][256];
    uint32_t env_count;
    
    /* Watchdog */
    uint32_t watchdog_timeout_ms;
    uint64_t last_watchdog_ping;
    
    bool enabled;
    bool auto_restart;
} service_t;

/* Global init system state */
static struct {
    service_t services[MAX_SERVICES];
    uint32_t service_count;
    bool shutdown_requested;
    int shutdown_signal;
    uint64_t boot_time;
    FILE* log_file;
} init_system;

/* Function prototypes */
static status_t load_service_config(const char* config_path);
static status_t start_service(const char* name);
static status_t stop_service(const char* name);
static status_t restart_service(const char* name);
static service_t* find_service(const char* name);
static status_t resolve_dependencies(const char* name, char** start_order);
static status_t setup_socket_activation(service_t* service);
static void service_monitor_loop(void);
static void handle_service_exit(pid_t pid, int status);
static void log_service_event(const char* service_name, const char* event, const char* details);

/* Initialize the init system */
status_t init_system_initialize(void) {
    printf("LimitlessOS Init System v1.0 starting...\n");
    
    init_system.service_count = 0;
    init_system.shutdown_requested = false;
    init_system.boot_time = get_current_time_ms();
    
    /* Open system log */
    init_system.log_file = fopen("/var/log/init.log", "a");
    if (!init_system.log_file) {
        init_system.log_file = stdout;
    }
    
    /* Load service configurations */
    status_t status = load_all_services();
    if (status != STATUS_OK) {
        printf("Failed to load service configurations: %d\n", status);
        return status;
    }
    
    /* Setup signal handlers */
    signal(SIGCHLD, sigchld_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigint_handler);
    
    log_service_event("init", "started", "Init system initialized");
    
    return STATUS_OK;
}

/* Load all service configurations */
status_t load_all_services(void) {
    printf("Loading service configurations...\n");
    
    /* Load services from /etc/services/ directory */
    DIR* service_dir = opendir("/etc/services");
    if (!service_dir) {
        printf("Service directory /etc/services not found\n");
        return STATUS_NOT_FOUND;
    }
    
    struct dirent* entry;
    while ((entry = readdir(service_dir)) != NULL) {
        if (strstr(entry->d_name, ".yaml") || strstr(entry->d_name, ".yml")) {
            char config_path[512];
            snprintf(config_path, sizeof(config_path), "/etc/services/%s", entry->d_name);
            
            status_t status = load_service_config(config_path);
            if (status != STATUS_OK) {
                printf("Failed to load service config %s: %d\n", config_path, status);
                /* Continue loading other services */
            }
        }
    }
    
    closedir(service_dir);
    
    printf("Loaded %u services\n", init_system.service_count);
    return STATUS_OK;
}

/* Load individual service configuration */
static status_t load_service_config(const char* config_path) {
    if (init_system.service_count >= MAX_SERVICES) {
        return STATUS_LIMIT_EXCEEDED;
    }
    
    FILE* file = fopen(config_path, "r");
    if (!file) {
        return STATUS_IO_ERROR;
    }
    
    service_t* service = &init_system.services[init_system.service_count];
    memset(service, 0, sizeof(service_t));
    
    /* Parse YAML configuration */
    yaml_parser_t parser;
    yaml_token_t token;
    
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, file);
    
    char current_key[128] = {0};
    bool in_dependencies = false;
    bool in_environment = false;
    
    while (yaml_parser_scan(&parser, &token)) {
        switch (token.type) {
            case YAML_KEY_TOKEN:
                /* Next token will be a key */
                break;
                
            case YAML_VALUE_TOKEN:
                /* Next token will be a value */
                break;
                
            case YAML_SCALAR_TOKEN: {
                char* value = (char*)token.data.scalar.value;
                
                if (strlen(current_key) == 0) {
                    /* This is a key */
                    strncpy(current_key, value, sizeof(current_key) - 1);
                } else {
                    /* This is a value for current_key */
                    if (strcmp(current_key, "name") == 0) {
                        strncpy(service->name, value, sizeof(service->name) - 1);
                    } else if (strcmp(current_key, "description") == 0) {
                        strncpy(service->description, value, sizeof(service->description) - 1);
                    } else if (strcmp(current_key, "command") == 0) {
                        strncpy(service->command, value, sizeof(service->command) - 1);
                    } else if (strcmp(current_key, "type") == 0) {
                        if (strcmp(value, "simple") == 0) service->type = SERVICE_TYPE_SIMPLE;
                        else if (strcmp(value, "forking") == 0) service->type = SERVICE_TYPE_FORKING;
                        else if (strcmp(value, "oneshot") == 0) service->type = SERVICE_TYPE_ONESHOT;
                        else if (strcmp(value, "notify") == 0) service->type = SERVICE_TYPE_NOTIFY;
                    } else if (strcmp(current_key, "restart") == 0) {
                        if (strcmp(value, "no") == 0) service->restart = RESTART_NO;
                        else if (strcmp(value, "on-failure") == 0) service->restart = RESTART_ON_FAILURE;
                        else if (strcmp(value, "always") == 0) service->restart = RESTART_ALWAYS;
                    } else if (strcmp(current_key, "user") == 0) {
                        strncpy(service->user, value, sizeof(service->user) - 1);
                    } else if (strcmp(current_key, "enabled") == 0) {
                        service->enabled = (strcmp(value, "true") == 0);
                    }
                    
                    /* Clear current key for next iteration */
                    current_key[0] = '\0';
                }
                break;
            }
            
            default:
                break;
        }
        
        yaml_token_delete(&token);
        
        if (token.type == YAML_STREAM_END_TOKEN) {
            break;
        }
    }
    
    yaml_parser_delete(&parser);
    fclose(file);
    
    /* Set defaults */
    if (strlen(service->name) == 0) {
        /* Extract name from filename */
        const char* filename = strrchr(config_path, '/');
        if (filename) {
            filename++; /* Skip '/' */
            strncpy(service->name, filename, sizeof(service->name) - 1);
            char* dot = strrchr(service->name, '.');
            if (dot) *dot = '\0'; /* Remove extension */
        }
    }
    
    service->state = SERVICE_STOPPED;
    service->restart_delay_ms = 1000; /* 1 second default */
    service->restart_limit = 5;
    
    init_system.service_count++;
    
    printf("Loaded service: %s (%s)\n", service->name, service->description);
    return STATUS_OK;
}

/* Start a service */
static status_t start_service(const char* name) {
    service_t* service = find_service(name);
    if (!service) {
        return STATUS_NOT_FOUND;
    }
    
    if (service->state == SERVICE_RUNNING) {
        return STATUS_ALREADY_EXISTS;
    }
    
    printf("Starting service: %s\n", service->name);
    log_service_event(service->name, "starting", "Service start requested");
    
    /* Resolve and start dependencies first */
    char* dep_order[MAX_DEPENDENCIES];
    status_t status = resolve_dependencies(name, dep_order);
    if (status != STATUS_OK) {
        printf("Failed to resolve dependencies for %s: %d\n", name, status);
        return status;
    }
    
    /* Start dependencies */
    for (int i = 0; dep_order[i]; i++) {
        if (strcmp(dep_order[i], name) != 0) { /* Don't start ourselves */
            service_t* dep = find_service(dep_order[i]);
            if (dep && dep->state != SERVICE_RUNNING) {
                status = start_service_internal(dep);
                if (status != STATUS_OK) {
                    printf("Failed to start dependency %s: %d\n", dep_order[i], status);
                    return status;
                }
            }
        }
    }
    
    /* Start the service itself */
    return start_service_internal(service);
}

/* Internal service start implementation */
static status_t start_service_internal(service_t* service) {
    service->state = SERVICE_STARTING;
    service->start_time = get_current_time_ms();
    
    /* Setup socket activation if configured */
    if (service->sockets) {
        status_t status = setup_socket_activation(service);
        if (status != STATUS_OK) {
            service->state = SERVICE_FAILED;
            return status;
        }
    }
    
    /* Fork and exec the service process */
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        
        /* Change user/group if specified */
        if (strlen(service->user) > 0) {
            /* Lookup user and change UID/GID */
            struct passwd* pwd = getpwnam(service->user);
            if (pwd) {
                setuid(pwd->pw_uid);
                setgid(pwd->pw_gid);
            }
        }
        
        /* Change working directory */
        if (strlen(service->working_directory) > 0) {
            chdir(service->working_directory);
        }
        
        /* Setup environment */
        for (uint32_t i = 0; i < service->env_count; i++) {
            putenv(service->environment[i]);
        }
        
        /* Parse command line */
        char* argv[64];
        int argc = parse_command_line(service->command, argv, 64);
        
        /* Execute the service */
        execv(argv[0], argv);
        
        /* If we get here, exec failed */
        printf("Failed to exec %s: %s\n", service->command, strerror(errno));
        exit(1);
        
    } else if (pid > 0) {
        /* Parent process */
        service->pid = pid;
        service->state = SERVICE_RUNNING;
        
        printf("Started service %s (PID %d)\n", service->name, pid);
        log_service_event(service->name, "started", "Service running");
        
        return STATUS_OK;
        
    } else {
        /* Fork failed */
        service->state = SERVICE_FAILED;
        printf("Failed to fork for service %s: %s\n", service->name, strerror(errno));
        return STATUS_FAILED;
    }
}

/* Stop a service */
static status_t stop_service(const char* name) {
    service_t* service = find_service(name);
    if (!service) {
        return STATUS_NOT_FOUND;
    }
    
    if (service->state != SERVICE_RUNNING) {
        return STATUS_INVALID_STATE;
    }
    
    printf("Stopping service: %s\n", service->name);
    log_service_event(service->name, "stopping", "Service stop requested");
    
    service->state = SERVICE_STOPPING;
    
    /* Send SIGTERM to the process */
    if (kill(service->pid, SIGTERM) == 0) {
        /* Wait for graceful shutdown */
        sleep(5); /* 5 second grace period */
        
        /* Check if still running */
        if (kill(service->pid, 0) == 0) {
            /* Force kill */
            printf("Service %s did not stop gracefully, force killing\n", service->name);
            kill(service->pid, SIGKILL);
        }
    }
    
    service->state = SERVICE_STOPPED;
    service->stop_time = get_current_time_ms();
    service->pid = 0;
    
    printf("Stopped service: %s\n", service->name);
    log_service_event(service->name, "stopped", "Service stopped");
    
    return STATUS_OK;
}

/* Setup socket activation */
static status_t setup_socket_activation(service_t* service) {
    socket_activation_t* sock = service->sockets;
    
    while (sock) {
        int fd = socket(AF_UNIX, sock->type, 0);
        if (fd < 0) {
            printf("Failed to create socket for %s: %s\n", service->name, strerror(errno));
            return STATUS_IO_ERROR;
        }
        
        /* Bind to address */
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, sock->address, sizeof(addr.sun_path) - 1);
        
        /* Remove existing socket file */
        unlink(sock->address);
        
        if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            printf("Failed to bind socket %s: %s\n", sock->address, strerror(errno));
            close(fd);
            return STATUS_IO_ERROR;
        }
        
        if (sock->type == SOCK_STREAM) {
            if (listen(fd, 128) < 0) {
                printf("Failed to listen on socket %s: %s\n", sock->address, strerror(errno));
                close(fd);
                return STATUS_IO_ERROR;
            }
        }
        
        sock->fd = fd;
        
        printf("Setup socket activation: %s -> %s\n", sock->name, sock->address);
        sock = sock->next;
    }
    
    return STATUS_OK;
}

/* Resolve service dependencies */
static status_t resolve_dependencies(const char* name, char** start_order) {
    /* Simple topological sort for dependency resolution */
    /* In production, would implement full dependency graph analysis */
    
    service_t* service = find_service(name);
    if (!service) {
        return STATUS_NOT_FOUND;
    }
    
    int order_index = 0;
    
    /* Add dependencies first */
    for (uint32_t i = 0; i < service->requires_count; i++) {
        start_order[order_index++] = strdup(service->requires[i]);
    }
    
    /* Add the service itself */
    start_order[order_index++] = strdup(name);
    start_order[order_index] = NULL;
    
    return STATUS_OK;
}

/* Service monitor main loop */
static void service_monitor_loop(void) {
    printf("Starting service monitor loop\n");
    
    while (!init_system.shutdown_requested) {
        /* Check service health and handle restarts */
        for (uint32_t i = 0; i < init_system.service_count; i++) {
            service_t* service = &init_system.services[i];
            
            if (service->state == SERVICE_RUNNING) {
                /* Check if process is still alive */
                if (kill(service->pid, 0) != 0) {
                    /* Process died */
                    printf("Service %s (PID %d) has died\n", service->name, service->pid);
                    handle_service_exit(service->pid, -1);
                }
                
                /* Check watchdog timeout */
                if (service->watchdog_timeout_ms > 0) {
                    uint64_t now = get_current_time_ms();
                    if (now - service->last_watchdog_ping > service->watchdog_timeout_ms) {
                        printf("Service %s watchdog timeout\n", service->name);
                        log_service_event(service->name, "watchdog-timeout", "Watchdog timeout exceeded");
                        
                        /* Kill and restart service */
                        kill(service->pid, SIGKILL);
                        handle_service_exit(service->pid, -1);
                    }
                }
            }
        }
        
        /* Sleep for 1 second */
        sleep(1);
    }
}

/* Handle service process exit */
static void handle_service_exit(pid_t pid, int status) {
    /* Find service by PID */
    service_t* service = NULL;
    for (uint32_t i = 0; i < init_system.service_count; i++) {
        if (init_system.services[i].pid == pid) {
            service = &init_system.services[i];
            break;
        }
    }
    
    if (!service) {
        return;
    }
    
    service->exit_code = status;
    service->stop_time = get_current_time_ms();
    service->pid = 0;
    
    bool should_restart = false;
    
    /* Determine if service should be restarted */
    switch (service->restart) {
        case RESTART_NO:
            break;
            
        case RESTART_ALWAYS:
            should_restart = true;
            break;
            
        case RESTART_ON_FAILURE:
            should_restart = (status != 0);
            break;
            
        case RESTART_ON_ABNORMAL:
            should_restart = (WIFSIGNALED(status) || (WIFEXITED(status) && WEXITSTATUS(status) != 0));
            break;
            
        default:
            break;
    }
    
    /* Check restart limits */
    if (should_restart) {
        if (service->restart_count >= service->restart_limit) {
            printf("Service %s exceeded restart limit (%u), not restarting\n", 
                   service->name, service->restart_limit);
            should_restart = false;
            service->state = SERVICE_FAILED;
            log_service_event(service->name, "restart-limit-exceeded", "Too many restarts");
        }
    }
    
    if (should_restart) {
        printf("Restarting service %s in %u ms\n", service->name, service->restart_delay_ms);
        service->state = SERVICE_RESTARTING;
        service->restart_count++;
        
        log_service_event(service->name, "restarting", "Service restart scheduled");
        
        /* Schedule restart (simplified - just sleep and restart) */
        usleep(service->restart_delay_ms * 1000);
        start_service_internal(service);
    } else {
        service->state = SERVICE_STOPPED;
        log_service_event(service->name, "stopped", "Service exited");
    }
}

/* Log service events */
static void log_service_event(const char* service_name, const char* event, const char* details) {
    uint64_t timestamp = get_current_time_ms();
    
    fprintf(init_system.log_file, "[%llu] %s: %s - %s\n", 
            timestamp, service_name, event, details);
    fflush(init_system.log_file);
}

/* Signal handlers */
static void sigchld_handler(int sig) {
    pid_t pid;
    int status;
    
    /* Reap all available children */
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        handle_service_exit(pid, status);
    }
}

static void sigterm_handler(int sig) {
    printf("Received SIGTERM, initiating shutdown\n");
    init_system.shutdown_requested = true;
    init_system.shutdown_signal = sig;
}

static void sigint_handler(int sig) {
    printf("Received SIGINT, initiating shutdown\n");
    init_system.shutdown_requested = true;
    init_system.shutdown_signal = sig;
}

/* Main init process entry point */
int main(int argc, char* argv[]) {
    /* Initialize init system */
    status_t status = init_system_initialize();
    if (status != STATUS_OK) {
        printf("Failed to initialize init system: %d\n", status);
        return 1;
    }
    
    /* Start default services */
    printf("Starting default services...\n");
    
    /* Start enabled services */
    for (uint32_t i = 0; i < init_system.service_count; i++) {
        service_t* service = &init_system.services[i];
        if (service->enabled) {
            status = start_service(service->name);
            if (status != STATUS_OK) {
                printf("Failed to start service %s: %d\n", service->name, status);
            }
        }
    }
    
    printf("System initialization complete\n");
    
    /* Enter service monitoring loop */
    service_monitor_loop();
    
    /* Shutdown sequence */
    printf("Shutting down services...\n");
    
    for (uint32_t i = 0; i < init_system.service_count; i++) {
        service_t* service = &init_system.services[i];
        if (service->state == SERVICE_RUNNING) {
            stop_service(service->name);
        }
    }
    
    printf("Init system shutdown complete\n");
    return 0;
}

/* Utility functions */
static service_t* find_service(const char* name) {
    for (uint32_t i = 0; i < init_system.service_count; i++) {
        if (strcmp(init_system.services[i].name, name) == 0) {
            return &init_system.services[i];
        }
    }
    return NULL;
}

static uint64_t get_current_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static int parse_command_line(const char* cmdline, char** argv, int max_args) {
    static char cmd_buffer[MAX_COMMAND_LINE];
    strncpy(cmd_buffer, cmdline, sizeof(cmd_buffer) - 1);
    
    int argc = 0;
    char* token = strtok(cmd_buffer, " \t");
    
    while (token && argc < max_args - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }
    
    argv[argc] = NULL;
    return argc;
}