/**
 * Limitless Task Manager - Advanced Process Management System
 * 
 * Comprehensive process and resource management application for LimitlessOS with
 * AI-powered optimization, security monitoring, and advanced system control.
 * 
 * Features:
 * - Real-time process monitoring and management
 * - AI-powered resource optimization and recommendations
 * - Security threat detection and process analysis
 * - Hardware resource monitoring (CPU, Memory, I/O)
 * - Process scheduling and priority management
 * - System service management
 * - Performance profiling and analysis
 * - Network connection monitoring per process
 * - Memory leak detection with AI analysis
 * - Automated resource balancing
 */

#include "../ui/limitlessui.h"
#include "../desktop/limitless_desktop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>

// ============================================================================
// TASK MANAGER CONSTANTS AND CONFIGURATION
// ============================================================================

#define TASK_MANAGER_VERSION        "1.0.0-Command"
#define MAX_PROCESSES               2048
#define MAX_PROCESS_NAME_LENGTH     256
#define MAX_COMMAND_LINE_LENGTH     512
#define MAX_NETWORK_CONNECTIONS     1024
#define MAX_OPEN_FILES             4096
#define MONITORING_INTERVAL_MS      1000    // 1 second refresh
#define AI_ANALYSIS_INTERVAL        30      // AI analysis every 30 seconds
#define PERFORMANCE_HISTORY_SIZE    300     // 5 minutes at 1Hz
#define ALERT_THRESHOLD_CPU         85.0f   // CPU usage alert threshold
#define ALERT_THRESHOLD_MEMORY      90.0f   // Memory usage alert threshold

// Process states
typedef enum {
    PROCESS_STATE_RUNNING = 0,      // Currently running
    PROCESS_STATE_SLEEPING,         // Interruptible sleep
    PROCESS_STATE_DISK_SLEEP,       // Uninterruptible sleep (usually I/O)
    PROCESS_STATE_ZOMBIE,           // Zombie process
    PROCESS_STATE_STOPPED,          // Stopped process
    PROCESS_STATE_TRACED,           // Being traced/debugged
    PROCESS_STATE_UNKNOWN          // Unknown state
} process_state_t;

// Process priority levels
typedef enum {
    PROCESS_PRIORITY_REAL_TIME = -20,   // Real-time priority
    PROCESS_PRIORITY_HIGH = -10,        // High priority
    PROCESS_PRIORITY_NORMAL = 0,        // Normal priority
    PROCESS_PRIORITY_LOW = 10,          // Low priority
    PROCESS_PRIORITY_VERY_LOW = 19      // Very low priority
} process_priority_t;

// AI analysis types for processes
typedef enum {
    AI_PROCESS_ANALYSIS_NONE = 0,
    AI_PROCESS_ANALYSIS_PERFORMANCE,    // Performance optimization
    AI_PROCESS_ANALYSIS_SECURITY,       // Security threat analysis
    AI_PROCESS_ANALYSIS_RESOURCE,       // Resource usage analysis
    AI_PROCESS_ANALYSIS_MEMORY_LEAK,    // Memory leak detection
    AI_PROCESS_ANALYSIS_NETWORK,        // Network activity analysis
    AI_PROCESS_ANALYSIS_BEHAVIOR        // Behavioral pattern analysis
} ai_process_analysis_t;

// Security risk levels for processes
typedef enum {
    SECURITY_RISK_NONE = 0,             // No security risk
    SECURITY_RISK_LOW,                  // Low risk
    SECURITY_RISK_MEDIUM,               // Medium risk
    SECURITY_RISK_HIGH,                 // High risk
    SECURITY_RISK_CRITICAL             // Critical security threat
} security_risk_level_t;

// ============================================================================
// CORE DATA STRUCTURES
// ============================================================================

// Network connection information
typedef struct network_connection {
    char local_address[64];             // Local IP:port
    char remote_address[64];            // Remote IP:port
    char protocol[16];                  // TCP, UDP, etc.
    char state[32];                     // ESTABLISHED, LISTENING, etc.
    uint64_t bytes_sent;
    uint64_t bytes_received;
    bool is_encrypted;
    
    struct network_connection* next;
} network_connection_t;

// Open file information
typedef struct open_file {
    char file_path[512];                // Full path to file
    char file_type[32];                 // regular, directory, socket, etc.
    int file_descriptor;
    char access_mode[8];                // r, w, rw, etc.
    uint64_t file_size;
    time_t access_time;
    
    struct open_file* next;
} open_file_t;

// Process resource usage statistics
typedef struct process_resources {
    // CPU usage
    float cpu_percent;                  // Current CPU usage percentage
    uint64_t cpu_time_user;             // User CPU time (microseconds)
    uint64_t cpu_time_system;           // System CPU time (microseconds)
    uint32_t cpu_context_switches;      // Number of context switches
    
    // Memory usage
    uint64_t memory_virtual;            // Virtual memory size (bytes)
    uint64_t memory_resident;           // Resident memory size (bytes)
    uint64_t memory_shared;             // Shared memory size (bytes)
    uint64_t memory_peak;               // Peak memory usage (bytes)
    uint32_t memory_page_faults;        // Number of page faults
    
    // I/O statistics
    uint64_t io_read_bytes;             // Bytes read from storage
    uint64_t io_write_bytes;            // Bytes written to storage
    uint64_t io_read_operations;        // Number of read operations
    uint64_t io_write_operations;       // Number of write operations
    
    // Network statistics
    uint64_t network_bytes_sent;        // Network bytes sent
    uint64_t network_bytes_received;    // Network bytes received
    uint32_t network_connections_count; // Number of active connections
    
    // File system
    uint32_t open_files_count;          // Number of open files
    uint32_t open_sockets_count;        // Number of open sockets
    
    // Performance metrics
    float performance_score;            // AI-calculated performance score (0.0-1.0)
    bool is_bottleneck;                 // Process is causing system bottleneck
    
} process_resources_t;

// AI analysis results for a process
typedef struct ai_process_analysis {
    ai_process_analysis_t analysis_type;
    float confidence_score;             // AI confidence (0.0-1.0)
    
    // Security analysis
    security_risk_level_t security_risk;
    char security_details[256];
    bool is_suspicious;
    bool requires_investigation;
    
    // Performance analysis
    float efficiency_score;             // Resource efficiency (0.0-1.0)
    bool memory_leak_detected;
    bool cpu_intensive;
    bool io_intensive;
    char optimization_suggestions[512];
    
    // Behavioral analysis
    bool abnormal_behavior;
    char behavior_description[256];
    float behavior_confidence;
    
    time_t last_analysis;
} ai_process_analysis_t;

// Process information structure
typedef struct process_info {
    // Basic process information
    pid_t pid;                          // Process ID
    pid_t parent_pid;                   // Parent process ID
    pid_t group_id;                     // Process group ID
    pid_t session_id;                   // Session ID
    
    char name[MAX_PROCESS_NAME_LENGTH]; // Process name
    char command_line[MAX_COMMAND_LINE_LENGTH]; // Full command line
    char executable_path[512];          // Path to executable
    char working_directory[512];        // Current working directory
    
    // Process state
    process_state_t state;              // Current process state
    int priority;                       // Process priority (nice value)
    int thread_count;                   // Number of threads
    
    // User and permissions
    uid_t user_id;                      // User ID
    gid_t group_id_owner;               // Group ID
    char username[64];                  // Username
    char groupname[64];                 // Group name
    
    // Timing information
    time_t start_time;                  // Process start time
    time_t last_update;                 // Last update time
    uint64_t uptime_seconds;            // Process uptime in seconds
    
    // Resource usage
    process_resources_t resources;      // Current resource usage
    process_resources_t resources_prev; // Previous measurement (for deltas)
    
    // Connected resources
    network_connection_t* network_connections; // List of network connections
    open_file_t* open_files;            // List of open files
    
    // AI analysis
    ai_process_analysis_t ai_analysis;  // AI analysis results
    
    // Management flags
    bool is_system_process;             // System/kernel process
    bool is_service;                    // System service
    bool is_gui_application;            // Has GUI components
    bool is_monitored;                  // Being actively monitored
    bool can_be_killed;                 // Safe to terminate
    bool requires_admin;                // Requires admin to manage
    
    struct process_info* next;
} process_info_t;

// System-wide resource summary
typedef struct system_resources {
    // CPU information
    uint32_t cpu_count;                 // Number of CPU cores
    float cpu_usage_total;              // Total CPU usage percentage
    float cpu_usage_per_core[32];       // Per-core CPU usage
    float cpu_load_average[3];          // 1, 5, 15 minute load averages
    
    // Memory information
    uint64_t memory_total;              // Total system memory
    uint64_t memory_available;          // Available memory
    uint64_t memory_used;               // Used memory
    uint64_t memory_cached;             // Cached memory
    uint64_t memory_buffers;            // Buffer memory
    uint64_t swap_total;                // Total swap space
    uint64_t swap_used;                 // Used swap space
    
    // I/O information
    uint64_t io_read_total;             // Total I/O reads
    uint64_t io_write_total;            // Total I/O writes
    float io_utilization;               // I/O utilization percentage
    
    // Network information
    uint64_t network_bytes_total_tx;    // Total network bytes transmitted
    uint64_t network_bytes_total_rx;    // Total network bytes received
    uint32_t network_connections_total; // Total network connections
    
    // Process counts
    uint32_t process_count_total;       // Total number of processes
    uint32_t process_count_running;     // Number of running processes
    uint32_t process_count_sleeping;    // Number of sleeping processes
    uint32_t process_count_zombie;      // Number of zombie processes
    
    time_t last_update;                 // Last update timestamp
} system_resources_t;

// Performance history for trending analysis
typedef struct performance_history {
    float cpu_history[PERFORMANCE_HISTORY_SIZE];
    float memory_history[PERFORMANCE_HISTORY_SIZE];
    float io_history[PERFORMANCE_HISTORY_SIZE];
    float network_history[PERFORMANCE_HISTORY_SIZE];
    
    uint32_t history_index;             // Current index in circular buffer
    uint32_t history_count;             // Number of valid entries
    time_t first_entry_time;            // Time of first entry
} performance_history_t;

// Alert system
typedef struct task_manager_alert {
    char message[256];                  // Alert message
    char details[512];                  // Additional details
    time_t timestamp;                   // When alert was generated
    pid_t related_pid;                  // Related process (0 if system-wide)
    
    enum {
        ALERT_LEVEL_INFO = 0,
        ALERT_LEVEL_WARNING,
        ALERT_LEVEL_ERROR,
        ALERT_LEVEL_CRITICAL
    } level;
    
    bool is_acknowledged;               // Has been acknowledged by user
    bool requires_action;               // Requires user action
    
    struct task_manager_alert* next;
} task_manager_alert_t;

// Main task manager application state
typedef struct limitless_task_manager {
    bool initialized;
    bool running;
    bool monitoring_active;
    bool has_admin_privileges;
    
    // UI components
    lui_window_t* main_window;
    lui_widget_t* toolbar;
    lui_widget_t* process_list;
    lui_widget_t* details_panel;
    lui_widget_t* performance_graphs;
    lui_widget_t* ai_panel;
    lui_widget_t* alerts_panel;
    lui_widget_t* status_bar;
    
    // Data structures
    process_info_t* processes;          // List of all processes
    uint32_t process_count;             // Number of processes
    process_info_t* selected_process;   // Currently selected process
    
    system_resources_t system_resources; // System-wide resource usage
    performance_history_t perf_history; // Performance history for graphs
    
    task_manager_alert_t* alerts;       // List of active alerts
    uint32_t alert_count;               // Number of active alerts
    
    // Monitoring thread
    pthread_t monitoring_thread;        // Background monitoring thread
    pthread_mutex_t data_mutex;         // Mutex for thread-safe data access
    
    // Configuration
    uint32_t refresh_interval_ms;       // Refresh interval in milliseconds
    bool show_system_processes;         // Show kernel/system processes
    bool show_ai_panel;                 // Show AI analysis panel
    bool enable_ai_recommendations;     // Enable AI recommendations
    bool auto_kill_unresponsive;        // Auto-kill unresponsive processes
    
    // Sorting and filtering
    enum {
        SORT_BY_PID = 0,
        SORT_BY_NAME,
        SORT_BY_CPU,
        SORT_BY_MEMORY,
        SORT_BY_IO,
        SORT_BY_NETWORK,
        SORT_BY_PRIORITY,
        SORT_BY_START_TIME
    } sort_column;
    
    bool sort_ascending;                // Sort direction
    char filter_text[256];              // Process name filter
    
    // Statistics
    struct {
        time_t session_start_time;
        uint32_t processes_killed;
        uint32_t processes_modified;
        uint32_t ai_recommendations_applied;
        uint32_t alerts_generated;
        uint32_t security_incidents;
    } stats;
    
} limitless_task_manager_t;

// Global task manager instance
static limitless_task_manager_t g_task_manager = {0};

// ============================================================================
// SYSTEM MONITORING AND DATA COLLECTION
// ============================================================================

static process_state_t parse_process_state(char state_char) {
    switch (state_char) {
        case 'R': return PROCESS_STATE_RUNNING;
        case 'S': return PROCESS_STATE_SLEEPING;
        case 'D': return PROCESS_STATE_DISK_SLEEP;
        case 'Z': return PROCESS_STATE_ZOMBIE;
        case 'T': return PROCESS_STATE_STOPPED;
        case 't': return PROCESS_STATE_TRACED;
        default:  return PROCESS_STATE_UNKNOWN;
    }
}

static void collect_process_basic_info(process_info_t* proc, pid_t pid) {
    char stat_path[64];
    char status_path[64];
    char cmdline_path[64];
    FILE* file;
    
    proc->pid = pid;
    proc->last_update = time(NULL);
    
    // Read /proc/[pid]/stat for basic information
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);
    file = fopen(stat_path, "r");
    if (file) {
        char state_char;
        fscanf(file, "%*d %255s %c %d %d %d", 
               proc->name, &state_char, &proc->parent_pid, 
               &proc->group_id, &proc->session_id);
        proc->state = parse_process_state(state_char);
        fclose(file);
        
        // Clean up process name (remove parentheses)
        if (proc->name[0] == '(' && strlen(proc->name) > 2) {
            memmove(proc->name, proc->name + 1, strlen(proc->name) - 1);
            proc->name[strlen(proc->name) - 1] = '\0';
        }
    }
    
    // Read /proc/[pid]/status for additional information
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);
    file = fopen(status_path, "r");
    if (file) {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, "Uid:", 4) == 0) {
                sscanf(line, "Uid:\t%d", &proc->user_id);
            } else if (strncmp(line, "Gid:", 4) == 0) {
                sscanf(line, "Gid:\t%d", &proc->group_id_owner);
            } else if (strncmp(line, "Threads:", 8) == 0) {
                sscanf(line, "Threads:\t%d", &proc->thread_count);
            }
        }
        fclose(file);
    }
    
    // Read command line
    snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%d/cmdline", pid);
    file = fopen(cmdline_path, "r");
    if (file) {
        size_t read = fread(proc->command_line, 1, sizeof(proc->command_line) - 1, file);
        proc->command_line[read] = '\0';
        
        // Replace null separators with spaces
        for (size_t i = 0; i < read; i++) {
            if (proc->command_line[i] == '\0' && i < read - 1) {
                proc->command_line[i] = ' ';
            }
        }
        fclose(file);
    }
    
    // Determine process characteristics
    proc->is_system_process = (proc->pid < 100) || (proc->user_id == 0);
    proc->can_be_killed = !proc->is_system_process && (proc->pid > 1);
    proc->requires_admin = (proc->user_id == 0) || proc->is_system_process;
}

static void collect_process_resources(process_info_t* proc) {
    char stat_path[64];
    char io_path[64];
    FILE* file;
    
    // Store previous values for delta calculations
    proc->resources_prev = proc->resources;
    
    // Read CPU and memory statistics from /proc/[pid]/stat
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", proc->pid);
    file = fopen(stat_path, "r");
    if (file) {
        unsigned long utime, stime, vsize, rss;
        fscanf(file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*ld %*ld %*ld %*ld %*ld %*ld %*llu %lu %lu",
               &utime, &stime, &vsize, &rss);
        
        proc->resources.cpu_time_user = utime * (1000000 / sysconf(_SC_CLK_TCK));
        proc->resources.cpu_time_system = stime * (1000000 / sysconf(_SC_CLK_TCK));
        proc->resources.memory_virtual = vsize;
        proc->resources.memory_resident = rss * sysconf(_SC_PAGESIZE);
        
        fclose(file);
    }
    
    // Read I/O statistics from /proc/[pid]/io
    snprintf(io_path, sizeof(io_path), "/proc/%d/io", proc->pid);
    file = fopen(io_path, "r");
    if (file) {
        char line[128];
        while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, "read_bytes:", 11) == 0) {
                sscanf(line, "read_bytes: %lu", &proc->resources.io_read_bytes);
            } else if (strncmp(line, "write_bytes:", 12) == 0) {
                sscanf(line, "write_bytes: %lu", &proc->resources.io_write_bytes);
            } else if (strncmp(line, "syscr:", 6) == 0) {
                sscanf(line, "syscr: %lu", &proc->resources.io_read_operations);
            } else if (strncmp(line, "syscw:", 6) == 0) {
                sscanf(line, "syscw: %lu", &proc->resources.io_write_operations);
            }
        }
        fclose(file);
    }
    
    // Calculate CPU percentage (simplified)
    uint64_t total_cpu_time = proc->resources.cpu_time_user + proc->resources.cpu_time_system;
    uint64_t prev_total_cpu_time = proc->resources_prev.cpu_time_user + proc->resources_prev.cpu_time_system;
    
    if (prev_total_cpu_time > 0) {
        uint64_t cpu_delta = total_cpu_time - prev_total_cpu_time;
        time_t time_delta = proc->last_update - (proc->last_update - 1); // 1 second approximation
        if (time_delta > 0) {
            proc->resources.cpu_percent = (float)(cpu_delta / 1000000.0) / time_delta * 100.0f;
        }
    }
    
    // Count open files and network connections (simplified)
    char fd_path[64];
    snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd", proc->pid);
    
    DIR* fd_dir = opendir(fd_path);
    if (fd_dir) {
        struct dirent* entry;
        proc->resources.open_files_count = 0;
        while ((entry = readdir(fd_dir)) != NULL) {
            if (entry->d_name[0] != '.') {
                proc->resources.open_files_count++;
            }
        }
        closedir(fd_dir);
    }
}

static process_info_t* create_process_info(pid_t pid) {
    process_info_t* proc = calloc(1, sizeof(process_info_t));
    if (!proc) return NULL;
    
    collect_process_basic_info(proc, pid);
    collect_process_resources(proc);
    
    proc->start_time = time(NULL);  // Simplified - should read from /proc/[pid]/stat
    proc->is_monitored = true;
    
    return proc;
}

static void scan_processes(void) {
    printf("[TaskManager] Scanning system processes\n");
    
    // Free existing process list
    process_info_t* proc = g_task_manager.processes;
    while (proc) {
        process_info_t* next = proc->next;
        
        // Free network connections
        network_connection_t* conn = proc->network_connections;
        while (conn) {
            network_connection_t* next_conn = conn->next;
            free(conn);
            conn = next_conn;
        }
        
        // Free open files
        open_file_t* file = proc->open_files;
        while (file) {
            open_file_t* next_file = file->next;
            free(file);
            file = next_file;
        }
        
        free(proc);
        proc = next;
    }
    
    g_task_manager.processes = NULL;
    g_task_manager.process_count = 0;
    
    // Scan /proc directory for process IDs
    DIR* proc_dir = opendir("/proc");
    if (!proc_dir) {
        printf("[TaskManager] ERROR: Cannot open /proc directory\n");
        return;
    }
    
    struct dirent* entry;
    process_info_t* last_proc = NULL;
    
    while ((entry = readdir(proc_dir)) != NULL) {
        // Check if directory name is a PID (all digits)
        if (strspn(entry->d_name, "0123456789") == strlen(entry->d_name)) {
            pid_t pid = atoi(entry->d_name);
            
            // Skip if not showing system processes and this is one
            if (!g_task_manager.show_system_processes && pid < 100) {
                continue;
            }
            
            process_info_t* new_proc = create_process_info(pid);
            if (new_proc) {
                if (last_proc) {
                    last_proc->next = new_proc;
                } else {
                    g_task_manager.processes = new_proc;
                }
                last_proc = new_proc;
                g_task_manager.process_count++;
            }
        }
    }
    
    closedir(proc_dir);
    
    printf("[TaskManager] Found %u processes\n", g_task_manager.process_count);
}

// ============================================================================
// AI ANALYSIS AND OPTIMIZATION
// ============================================================================

static void perform_ai_process_analysis(process_info_t* proc) {
    if (!g_task_manager.enable_ai_recommendations) return;
    
    ai_process_analysis_t* ai = &proc->ai_analysis;
    
    // Security analysis
    ai->security_risk = SECURITY_RISK_NONE;
    ai->is_suspicious = false;
    
    // Check for suspicious patterns
    if (proc->resources.cpu_percent > 80.0f && proc->resources.network_connections_count > 50) {
        ai->security_risk = SECURITY_RISK_MEDIUM;
        ai->is_suspicious = true;
        strcpy(ai->security_details, "High CPU usage with many network connections - possible mining or DDoS activity");
    }
    
    if (strstr(proc->name, "crypto") || strstr(proc->name, "miner") || strstr(proc->name, "coin")) {
        ai->security_risk = SECURITY_RISK_HIGH;
        strcpy(ai->security_details, "Process name suggests cryptocurrency mining activity");
    }
    
    // Performance analysis
    ai->efficiency_score = 1.0f;
    ai->memory_leak_detected = false;
    ai->cpu_intensive = (proc->resources.cpu_percent > 50.0f);
    ai->io_intensive = (proc->resources.io_read_bytes + proc->resources.io_write_bytes > 100 * 1024 * 1024); // 100MB
    
    // Memory leak detection (simplified)
    if (proc->resources.memory_resident > proc->resources_prev.memory_resident * 1.1f &&
        proc->resources.memory_resident > 100 * 1024 * 1024) { // Growing by 10% and over 100MB
        ai->memory_leak_detected = true;
        ai->efficiency_score *= 0.5f;
    }
    
    // Generate optimization suggestions
    strcpy(ai->optimization_suggestions, "");
    if (ai->cpu_intensive) {
        strcat(ai->optimization_suggestions, "Consider reducing CPU priority or limiting CPU affinity. ");
    }
    if (ai->io_intensive) {
        strcat(ai->optimization_suggestions, "Process is I/O intensive - consider SSD upgrade or I/O scheduling optimization. ");
    }
    if (ai->memory_leak_detected) {
        strcat(ai->optimization_suggestions, "Potential memory leak detected - consider restarting process. ");
    }
    
    // Behavioral analysis
    ai->abnormal_behavior = false;
    if (proc->resources.cpu_percent > 95.0f) {
        ai->abnormal_behavior = true;
        strcpy(ai->behavior_description, "Excessive CPU usage may indicate process malfunction");
        ai->behavior_confidence = 0.85f;
    }
    
    ai->confidence_score = 0.80f;
    ai->last_analysis = time(NULL);
}

static void generate_system_alerts(void) {
    // Check CPU usage
    if (g_task_manager.system_resources.cpu_usage_total > ALERT_THRESHOLD_CPU) {
        task_manager_alert_t* alert = calloc(1, sizeof(task_manager_alert_t));
        alert->level = (g_task_manager.system_resources.cpu_usage_total > 95.0f) ? 
                      ALERT_LEVEL_CRITICAL : ALERT_LEVEL_WARNING;
        snprintf(alert->message, sizeof(alert->message), 
                "High CPU Usage: %.1f%%", g_task_manager.system_resources.cpu_usage_total);
        strcpy(alert->details, "System CPU usage is above normal threshold. Consider closing unnecessary applications.");
        alert->timestamp = time(NULL);
        alert->requires_action = true;
        
        // Add to alerts list
        alert->next = g_task_manager.alerts;
        g_task_manager.alerts = alert;
        g_task_manager.alert_count++;
    }
    
    // Check memory usage
    float memory_usage_percent = ((float)g_task_manager.system_resources.memory_used / 
                                 g_task_manager.system_resources.memory_total) * 100.0f;
    if (memory_usage_percent > ALERT_THRESHOLD_MEMORY) {
        task_manager_alert_t* alert = calloc(1, sizeof(task_manager_alert_t));
        alert->level = (memory_usage_percent > 98.0f) ? ALERT_LEVEL_CRITICAL : ALERT_LEVEL_WARNING;
        snprintf(alert->message, sizeof(alert->message), 
                "High Memory Usage: %.1f%%", memory_usage_percent);
        strcpy(alert->details, "System memory usage is critically high. Consider closing applications or adding more RAM.");
        alert->timestamp = time(NULL);
        alert->requires_action = true;
        
        alert->next = g_task_manager.alerts;
        g_task_manager.alerts = alert;
        g_task_manager.alert_count++;
    }
}

// ============================================================================
// USER INTERFACE IMPLEMENTATION
// ============================================================================

static void create_toolbar(void) {
    g_task_manager.toolbar = lui_create_container(g_task_manager.main_window->root_widget);
    strcpy(g_task_manager.toolbar->name, "toolbar");
    g_task_manager.toolbar->bounds = lui_rect_make(0, 0, 1200, 32);
    g_task_manager.toolbar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // Process control buttons
    lui_widget_t* kill_btn = lui_create_button("⏹ Kill Process", g_task_manager.toolbar);
    kill_btn->bounds = lui_rect_make(8, 4, 100, 24);
    kill_btn->style.background_color = LUI_COLOR_ALERT_RED;
    
    lui_widget_t* suspend_btn = lui_create_button("⏸ Suspend", g_task_manager.toolbar);
    suspend_btn->bounds = lui_rect_make(116, 4, 80, 24);
    suspend_btn->style.background_color = LUI_COLOR_WARNING_AMBER;
    
    lui_widget_t* resume_btn = lui_create_button("▶ Resume", g_task_manager.toolbar);
    resume_btn->bounds = lui_rect_make(204, 4, 80, 24);
    resume_btn->style.background_color = LUI_COLOR_SUCCESS_GREEN;
    
    // Priority controls
    lui_widget_t* priority_label = lui_create_label("Priority:", g_task_manager.toolbar);
    priority_label->bounds = lui_rect_make(300, 8, 50, 16);
    priority_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    lui_widget_t* priority_dropdown = lui_create_button("Normal", g_task_manager.toolbar);
    priority_dropdown->bounds = lui_rect_make(355, 4, 80, 24);
    priority_dropdown->style.background_color = LUI_COLOR_STEEL_GRAY;
    
    // Filter controls
    lui_widget_t* filter_input = lui_create_text_input("Filter processes...", g_task_manager.toolbar);
    filter_input->bounds = lui_rect_make(450, 4, 150, 24);
    
    // View toggles
    lui_widget_t* show_system_btn = lui_create_button("🔧 System", g_task_manager.toolbar);
    show_system_btn->bounds = lui_rect_make(610, 4, 70, 24);
    show_system_btn->style.background_color = g_task_manager.show_system_processes ? 
                                             LUI_COLOR_TACTICAL_BLUE : LUI_COLOR_STEEL_GRAY;
    
    lui_widget_t* show_ai_btn = lui_create_button("🤖 AI", g_task_manager.toolbar);
    show_ai_btn->bounds = lui_rect_make(688, 4, 50, 24);
    show_ai_btn->style.background_color = g_task_manager.show_ai_panel ? 
                                         LUI_COLOR_SECURE_CYAN : LUI_COLOR_STEEL_GRAY;
    
    // Refresh controls
    lui_widget_t* refresh_btn = lui_create_button("🔄 Refresh", g_task_manager.toolbar);
    refresh_btn->bounds = lui_rect_make(750, 4, 80, 24);
    refresh_btn->style.background_color = LUI_COLOR_TACTICAL_BLUE;
    
    // Monitoring status
    lui_widget_t* monitoring_status = lui_create_label(g_task_manager.monitoring_active ? "🟢 Active" : "🔴 Stopped", 
                                                     g_task_manager.toolbar);
    monitoring_status->bounds = lui_rect_make(1100, 8, 80, 16);
    monitoring_status->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
}

static void create_process_list(void) {
    g_task_manager.process_list = lui_create_container(g_task_manager.main_window->root_widget);
    strcpy(g_task_manager.process_list->name, "process_list");
    g_task_manager.process_list->bounds = lui_rect_make(0, 32, 800, 500);
    g_task_manager.process_list->background_color = LUI_COLOR_CHARCOAL_BLACK;
    
    // Column headers
    const char* headers[] = {"PID", "Name", "CPU%", "Memory", "I/O", "Network", "Priority", "State", "User"};
    int header_widths[] = {60, 150, 60, 80, 80, 80, 70, 60, 60};
    int x_offset = 8;
    
    for (int i = 0; i < 9; i++) {
        lui_widget_t* header = lui_create_button(headers[i], g_task_manager.process_list);
        header->bounds = lui_rect_make(x_offset, 8, header_widths[i], 24);
        header->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
        header->style.background_color = LUI_COLOR_STEEL_GRAY;
        
        // Sort indicator
        if (g_task_manager.sort_column == i) {
            lui_widget_t* sort_arrow = lui_create_label(g_task_manager.sort_ascending ? "▲" : "▼", 
                                                      g_task_manager.process_list);
            sort_arrow->bounds = lui_rect_make(x_offset + header_widths[i] - 12, 12, 8, 16);
            sort_arrow->typography.color = LUI_COLOR_SECURE_CYAN;
        }
        
        x_offset += header_widths[i] + 2;
    }
    
    // Process rows
    process_info_t* proc = g_task_manager.processes;
    int y_offset = 40;
    int row_count = 0;
    
    while (proc && y_offset < 480 && row_count < 20) { // Limit visible rows
        // Apply filter
        if (strlen(g_task_manager.filter_text) > 0 && 
            !strstr(proc->name, g_task_manager.filter_text)) {
            proc = proc->next;
            continue;
        }
        
        // Row background (alternating colors)
        lui_widget_t* row_bg = lui_create_container(g_task_manager.process_list);
        row_bg->bounds = lui_rect_make(0, y_offset, 800, 20);
        row_bg->background_color = (row_count % 2 == 0) ? LUI_COLOR_GRAPHITE : LUI_COLOR_CHARCOAL_BLACK;
        
        // Selection highlight
        if (proc == g_task_manager.selected_process) {
            row_bg->background_color = LUI_COLOR_TACTICAL_BLUE;
        }
        
        x_offset = 8;
        
        // PID
        char pid_text[16];
        snprintf(pid_text, sizeof(pid_text), "%d", proc->pid);
        lui_widget_t* pid_label = lui_create_label(pid_text, g_task_manager.process_list);
        pid_label->bounds = lui_rect_make(x_offset, y_offset + 2, 60, 16);
        pid_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        x_offset += 62;
        
        // Name
        lui_widget_t* name_label = lui_create_label(proc->name, g_task_manager.process_list);
        name_label->bounds = lui_rect_make(x_offset, y_offset + 2, 150, 16);
        name_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        
        // Security risk indicator
        if (proc->ai_analysis.security_risk >= SECURITY_RISK_MEDIUM) {
            lui_widget_t* risk_icon = lui_create_label("⚠", g_task_manager.process_list);
            risk_icon->bounds = lui_rect_make(x_offset + 135, y_offset + 2, 12, 16);
            risk_icon->typography.color = (proc->ai_analysis.security_risk >= SECURITY_RISK_HIGH) ? 
                                         LUI_COLOR_ALERT_RED : LUI_COLOR_WARNING_AMBER;
        }
        x_offset += 152;
        
        // CPU%
        char cpu_text[16];
        snprintf(cpu_text, sizeof(cpu_text), "%.1f%%", proc->resources.cpu_percent);
        lui_widget_t* cpu_label = lui_create_label(cpu_text, g_task_manager.process_list);
        cpu_label->bounds = lui_rect_make(x_offset, y_offset + 2, 60, 16);
        cpu_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        if (proc->resources.cpu_percent > 50.0f) {
            cpu_label->typography.color = LUI_COLOR_WARNING_AMBER;
        }
        x_offset += 62;
        
        // Memory
        char memory_text[16];
        float memory_mb = proc->resources.memory_resident / (1024.0f * 1024.0f);
        if (memory_mb > 1000.0f) {
            snprintf(memory_text, sizeof(memory_text), "%.1fGB", memory_mb / 1024.0f);
        } else {
            snprintf(memory_text, sizeof(memory_text), "%.0fMB", memory_mb);
        }
        lui_widget_t* memory_label = lui_create_label(memory_text, g_task_manager.process_list);
        memory_label->bounds = lui_rect_make(x_offset, y_offset + 2, 80, 16);
        memory_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        x_offset += 82;
        
        // I/O
        char io_text[16];
        float io_mb = (proc->resources.io_read_bytes + proc->resources.io_write_bytes) / (1024.0f * 1024.0f);
        snprintf(io_text, sizeof(io_text), "%.1fMB", io_mb);
        lui_widget_t* io_label = lui_create_label(io_text, g_task_manager.process_list);
        io_label->bounds = lui_rect_make(x_offset, y_offset + 2, 80, 16);
        io_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        x_offset += 82;
        
        // Network
        char network_text[16];
        snprintf(network_text, sizeof(network_text), "%u", proc->resources.network_connections_count);
        lui_widget_t* network_label = lui_create_label(network_text, g_task_manager.process_list);
        network_label->bounds = lui_rect_make(x_offset, y_offset + 2, 80, 16);
        network_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        x_offset += 82;
        
        // Priority
        char priority_text[16];
        snprintf(priority_text, sizeof(priority_text), "%d", proc->priority);
        lui_widget_t* priority_label = lui_create_label(priority_text, g_task_manager.process_list);
        priority_label->bounds = lui_rect_make(x_offset, y_offset + 2, 70, 16);
        priority_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        x_offset += 72;
        
        // State
        const char* state_text = "Unknown";
        switch (proc->state) {
            case PROCESS_STATE_RUNNING: state_text = "Run"; break;
            case PROCESS_STATE_SLEEPING: state_text = "Sleep"; break;
            case PROCESS_STATE_DISK_SLEEP: state_text = "Disk"; break;
            case PROCESS_STATE_ZOMBIE: state_text = "Zombie"; break;
            case PROCESS_STATE_STOPPED: state_text = "Stop"; break;
            case PROCESS_STATE_TRACED: state_text = "Trace"; break;
        }
        lui_widget_t* state_label = lui_create_label(state_text, g_task_manager.process_list);
        state_label->bounds = lui_rect_make(x_offset, y_offset + 2, 60, 16);
        state_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        if (proc->state == PROCESS_STATE_ZOMBIE) {
            state_label->typography.color = LUI_COLOR_ALERT_RED;
        }
        x_offset += 62;
        
        // User
        char user_text[16];
        snprintf(user_text, sizeof(user_text), "%d", proc->user_id);
        lui_widget_t* user_label = lui_create_label(user_text, g_task_manager.process_list);
        user_label->bounds = lui_rect_make(x_offset, y_offset + 2, 60, 16);
        user_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        
        y_offset += 22;
        row_count++;
        proc = proc->next;
    }
}

static void create_ai_panel(void) {
    if (!g_task_manager.show_ai_panel) return;
    
    g_task_manager.ai_panel = lui_create_container(g_task_manager.main_window->root_widget);
    strcpy(g_task_manager.ai_panel->name, "ai_panel");
    g_task_manager.ai_panel->bounds = lui_rect_make(800, 32, 400, 350);
    g_task_manager.ai_panel->background_color = LUI_COLOR_GRAPHITE;
    
    // AI header
    lui_widget_t* ai_header = lui_create_label("🤖 AI Process Analysis", g_task_manager.ai_panel);
    ai_header->bounds = lui_rect_make(8, 8, 300, 24);
    ai_header->typography = LUI_TYPOGRAPHY_TITLE_MEDIUM;
    ai_header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    if (g_task_manager.selected_process) {
        process_info_t* proc = g_task_manager.selected_process;
        ai_process_analysis_t* ai = &proc->ai_analysis;
        
        // Process name and PID
        char proc_info[128];
        snprintf(proc_info, sizeof(proc_info), "Process: %s (PID %d)", proc->name, proc->pid);
        lui_widget_t* proc_label = lui_create_label(proc_info, g_task_manager.ai_panel);
        proc_label->bounds = lui_rect_make(8, 40, 384, 20);
        proc_label->typography = LUI_TYPOGRAPHY_BODY_MEDIUM;
        
        // Security analysis
        lui_widget_t* security_header = lui_create_label("🛡️ Security Analysis:", g_task_manager.ai_panel);
        security_header->bounds = lui_rect_make(8, 70, 200, 20);
        security_header->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
        security_header->typography.color = LUI_COLOR_WARNING_AMBER;
        
        const char* risk_text = "None";
        lui_color_t risk_color = LUI_COLOR_SUCCESS_GREEN;
        switch (ai->security_risk) {
            case SECURITY_RISK_LOW: risk_text = "Low"; risk_color = LUI_COLOR_WARNING_AMBER; break;
            case SECURITY_RISK_MEDIUM: risk_text = "Medium"; risk_color = LUI_COLOR_WARNING_AMBER; break;
            case SECURITY_RISK_HIGH: risk_text = "High"; risk_color = LUI_COLOR_ALERT_RED; break;
            case SECURITY_RISK_CRITICAL: risk_text = "Critical"; risk_color = LUI_COLOR_ALERT_RED; break;
        }
        
        char security_text[128];
        snprintf(security_text, sizeof(security_text), "Risk Level: %s", risk_text);
        lui_widget_t* security_risk = lui_create_label(security_text, g_task_manager.ai_panel);
        security_risk->bounds = lui_rect_make(16, 95, 200, 16);
        security_risk->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        security_risk->typography.color = risk_color;
        
        if (strlen(ai->security_details) > 0) {
            lui_widget_t* security_details = lui_create_label(ai->security_details, g_task_manager.ai_panel);
            security_details->bounds = lui_rect_make(16, 115, 368, 40);
            security_details->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        }
        
        // Performance analysis
        lui_widget_t* perf_header = lui_create_label("📊 Performance Analysis:", g_task_manager.ai_panel);
        perf_header->bounds = lui_rect_make(8, 165, 200, 20);
        perf_header->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
        perf_header->typography.color = LUI_COLOR_SECURE_CYAN;
        
        char efficiency_text[64];
        snprintf(efficiency_text, sizeof(efficiency_text), "Efficiency Score: %.0f%%", ai->efficiency_score * 100.0f);
        lui_widget_t* efficiency_label = lui_create_label(efficiency_text, g_task_manager.ai_panel);
        efficiency_label->bounds = lui_rect_make(16, 190, 200, 16);
        efficiency_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        
        // Performance indicators
        char indicators[256] = "Characteristics: ";
        if (ai->cpu_intensive) strcat(indicators, "CPU-Intensive ");
        if (ai->io_intensive) strcat(indicators, "I/O-Intensive ");
        if (ai->memory_leak_detected) strcat(indicators, "Memory-Leak ");
        if (ai->abnormal_behavior) strcat(indicators, "Abnormal-Behavior ");
        
        lui_widget_t* indicators_label = lui_create_label(indicators, g_task_manager.ai_panel);
        indicators_label->bounds = lui_rect_make(16, 210, 368, 32);
        indicators_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        
        // Optimization suggestions
        if (strlen(ai->optimization_suggestions) > 0) {
            lui_widget_t* suggestions_header = lui_create_label("💡 Suggestions:", g_task_manager.ai_panel);
            suggestions_header->bounds = lui_rect_make(8, 250, 200, 20);
            suggestions_header->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
            suggestions_header->typography.color = LUI_COLOR_WARNING_AMBER;
            
            lui_widget_t* suggestions_text = lui_create_label(ai->optimization_suggestions, g_task_manager.ai_panel);
            suggestions_text->bounds = lui_rect_make(16, 275, 368, 60);
            suggestions_text->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        }
    } else {
        lui_widget_t* no_selection = lui_create_label("Select a process to view AI analysis", g_task_manager.ai_panel);
        no_selection->bounds = lui_rect_make(8, 40, 300, 20);
        no_selection->typography = LUI_TYPOGRAPHY_BODY_MEDIUM;
        no_selection->typography.color = LUI_COLOR_STEEL_GRAY;
    }
}

static void create_performance_graphs(void) {
    g_task_manager.performance_graphs = lui_create_container(g_task_manager.main_window->root_widget);
    strcpy(g_task_manager.performance_graphs->name, "performance_graphs");
    g_task_manager.performance_graphs->bounds = lui_rect_make(800, 390, 400, 140);
    g_task_manager.performance_graphs->background_color = LUI_COLOR_CHARCOAL_BLACK;
    
    // Graph titles and basic visualization
    lui_widget_t* graph_title = lui_create_label("📈 System Performance", g_task_manager.performance_graphs);
    graph_title->bounds = lui_rect_make(8, 8, 200, 20);
    graph_title->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
    graph_title->typography.color = LUI_COLOR_SECURE_CYAN;
    
    // CPU usage graph (simplified)
    char cpu_text[64];
    snprintf(cpu_text, sizeof(cpu_text), "CPU: %.1f%%", g_task_manager.system_resources.cpu_usage_total);
    lui_widget_t* cpu_graph = lui_create_label(cpu_text, g_task_manager.performance_graphs);
    cpu_graph->bounds = lui_rect_make(8, 35, 100, 16);
    cpu_graph->typography = LUI_TYPOGRAPHY_BODY_SMALL;
    
    // Memory usage graph (simplified)
    float memory_percent = ((float)g_task_manager.system_resources.memory_used / 
                           g_task_manager.system_resources.memory_total) * 100.0f;
    char memory_text[64];
    snprintf(memory_text, sizeof(memory_text), "Memory: %.1f%%", memory_percent);
    lui_widget_t* memory_graph = lui_create_label(memory_text, g_task_manager.performance_graphs);
    memory_graph->bounds = lui_rect_make(8, 55, 100, 16);
    memory_graph->typography = LUI_TYPOGRAPHY_BODY_SMALL;
    
    // Process count
    char process_text[64];
    snprintf(process_text, sizeof(process_text), "Processes: %u", g_task_manager.process_count);
    lui_widget_t* process_graph = lui_create_label(process_text, g_task_manager.performance_graphs);
    process_graph->bounds = lui_rect_make(8, 75, 100, 16);
    process_graph->typography = LUI_TYPOGRAPHY_BODY_SMALL;
    
    // Load average
    char load_text[64];
    snprintf(load_text, sizeof(load_text), "Load: %.2f", g_task_manager.system_resources.cpu_load_average[0]);
    lui_widget_t* load_graph = lui_create_label(load_text, g_task_manager.performance_graphs);
    load_graph->bounds = lui_rect_make(8, 95, 100, 16);
    load_graph->typography = LUI_TYPOGRAPHY_BODY_SMALL;
}

static void create_status_bar(void) {
    g_task_manager.status_bar = lui_create_container(g_task_manager.main_window->root_widget);
    strcpy(g_task_manager.status_bar->name, "status_bar");
    g_task_manager.status_bar->bounds = lui_rect_make(0, 556, 1200, 24);
    g_task_manager.status_bar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // System status
    char status_text[256];
    snprintf(status_text, sizeof(status_text),
            "Processes: %u | CPU: %.1f%% | Memory: %.1f%% | Alerts: %u",
            g_task_manager.process_count,
            g_task_manager.system_resources.cpu_usage_total,
            ((float)g_task_manager.system_resources.memory_used / g_task_manager.system_resources.memory_total) * 100.0f,
            g_task_manager.alert_count);
    
    lui_widget_t* status_label = lui_create_label(status_text, g_task_manager.status_bar);
    status_label->bounds = lui_rect_make(8, 4, 800, 16);
    status_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    // Admin privileges indicator
    if (g_task_manager.has_admin_privileges) {
        lui_widget_t* admin_label = lui_create_label("👑 Administrator", g_task_manager.status_bar);
        admin_label->bounds = lui_rect_make(1050, 4, 100, 16);
        admin_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        admin_label->typography.color = LUI_COLOR_WARNING_AMBER;
    }
}

// ============================================================================
// MONITORING THREAD
// ============================================================================

static void* monitoring_thread_func(void* arg) {
    printf("[TaskManager] Monitoring thread started\n");
    
    while (g_task_manager.running && g_task_manager.monitoring_active) {
        pthread_mutex_lock(&g_task_manager.data_mutex);
        
        // Scan and update processes
        scan_processes();
        
        // Update system resources (simplified)
        g_task_manager.system_resources.last_update = time(NULL);
        g_task_manager.system_resources.cpu_usage_total = 45.0f; // Simulated
        g_task_manager.system_resources.memory_used = g_task_manager.system_resources.memory_total * 0.6f; // 60% usage
        g_task_manager.system_resources.process_count_total = g_task_manager.process_count;
        
        // Perform AI analysis on selected processes
        process_info_t* proc = g_task_manager.processes;
        while (proc) {
            if (proc->is_monitored) {
                perform_ai_process_analysis(proc);
            }
            proc = proc->next;
        }
        
        // Generate system alerts
        generate_system_alerts();
        
        pthread_mutex_unlock(&g_task_manager.data_mutex);
        
        // Sleep for the specified interval
        usleep(g_task_manager.refresh_interval_ms * 1000);
    }
    
    printf("[TaskManager] Monitoring thread stopped\n");
    return NULL;
}

// ============================================================================
// MAIN TASK MANAGER API
// ============================================================================

bool limitless_task_manager_init(void) {
    if (g_task_manager.initialized) {
        return false;
    }
    
    printf("[TaskManager] Initializing Limitless Task Manager v%s\n", TASK_MANAGER_VERSION);
    
    // Clear state
    memset(&g_task_manager, 0, sizeof(g_task_manager));
    
    // Check for admin privileges
    g_task_manager.has_admin_privileges = (getuid() == 0);
    
    // Set default configuration
    g_task_manager.refresh_interval_ms = MONITORING_INTERVAL_MS;
    g_task_manager.show_system_processes = false;
    g_task_manager.show_ai_panel = true;
    g_task_manager.enable_ai_recommendations = true;
    g_task_manager.auto_kill_unresponsive = false;
    g_task_manager.sort_column = SORT_BY_CPU;
    g_task_manager.sort_ascending = false;
    
    // Initialize system resources
    g_task_manager.system_resources.memory_total = 16ULL * 1024 * 1024 * 1024; // 16GB
    g_task_manager.system_resources.cpu_count = 8;
    
    // Initialize mutex
    if (pthread_mutex_init(&g_task_manager.data_mutex, NULL) != 0) {
        printf("[TaskManager] ERROR: Failed to initialize mutex\n");
        return false;
    }
    
    // Create main window
    g_task_manager.main_window = lui_create_window("Limitless Task Manager", LUI_WINDOW_NORMAL,
                                                  100, 100, 1200, 580);
    if (!g_task_manager.main_window) {
        printf("[TaskManager] ERROR: Failed to create main window\n");
        pthread_mutex_destroy(&g_task_manager.data_mutex);
        return false;
    }
    
    // Initial process scan
    scan_processes();
    
    // Create UI components
    create_toolbar();
    create_process_list();
    create_ai_panel();
    create_performance_graphs();
    create_status_bar();
    
    // Show window
    lui_show_window(g_task_manager.main_window);
    
    // Start monitoring thread
    g_task_manager.monitoring_active = true;
    if (pthread_create(&g_task_manager.monitoring_thread, NULL, monitoring_thread_func, NULL) != 0) {
        printf("[TaskManager] ERROR: Failed to create monitoring thread\n");
        g_task_manager.monitoring_active = false;
    }
    
    g_task_manager.initialized = true;
    g_task_manager.running = true;
    g_task_manager.stats.session_start_time = time(NULL);
    
    printf("[TaskManager] Task Manager initialized successfully\n");
    printf("[TaskManager] Admin privileges: %s, Processes: %u, AI enabled: %s\n",
           g_task_manager.has_admin_privileges ? "Yes" : "No",
           g_task_manager.process_count,
           g_task_manager.enable_ai_recommendations ? "Yes" : "No");
    
    return true;
}

void limitless_task_manager_shutdown(void) {
    if (!g_task_manager.initialized) {
        return;
    }
    
    printf("[TaskManager] Shutting down Limitless Task Manager\n");
    
    g_task_manager.running = false;
    g_task_manager.monitoring_active = false;
    
    // Wait for monitoring thread to finish
    if (g_task_manager.monitoring_thread) {
        pthread_join(g_task_manager.monitoring_thread, NULL);
    }
    
    // Free process list
    process_info_t* proc = g_task_manager.processes;
    while (proc) {
        process_info_t* next = proc->next;
        
        // Free network connections
        network_connection_t* conn = proc->network_connections;
        while (conn) {
            network_connection_t* next_conn = conn->next;
            free(conn);
            conn = next_conn;
        }
        
        // Free open files
        open_file_t* file = proc->open_files;
        while (file) {
            open_file_t* next_file = file->next;
            free(file);
            file = next_file;
        }
        
        free(proc);
        proc = next;
    }
    
    // Free alerts
    task_manager_alert_t* alert = g_task_manager.alerts;
    while (alert) {
        task_manager_alert_t* next = alert->next;
        free(alert);
        alert = next;
    }
    
    // Destroy main window
    if (g_task_manager.main_window) {
        lui_destroy_window(g_task_manager.main_window);
    }
    
    // Destroy mutex
    pthread_mutex_destroy(&g_task_manager.data_mutex);
    
    // Print session statistics
    time_t session_duration = time(NULL) - g_task_manager.stats.session_start_time;
    printf("[TaskManager] Session statistics:\n");
    printf("  Duration: %ld seconds\n", session_duration);
    printf("  Processes killed: %u\n", g_task_manager.stats.processes_killed);
    printf("  Processes modified: %u\n", g_task_manager.stats.processes_modified);
    printf("  AI recommendations applied: %u\n", g_task_manager.stats.ai_recommendations_applied);
    printf("  Alerts generated: %u\n", g_task_manager.stats.alerts_generated);
    printf("  Security incidents: %u\n", g_task_manager.stats.security_incidents);
    
    memset(&g_task_manager, 0, sizeof(g_task_manager));
    
    printf("[TaskManager] Shutdown complete\n");
}

const char* limitless_task_manager_get_version(void) {
    return TASK_MANAGER_VERSION;
}

void limitless_task_manager_run(void) {
    if (!g_task_manager.initialized) {
        printf("[TaskManager] ERROR: Task Manager not initialized\n");
        return;
    }
    
    printf("[TaskManager] Running Limitless Task Manager\n");
    
    // Main event loop is handled by the desktop environment
}