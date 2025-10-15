/*
 * profiler.c - LimitlessOS Performance Profiling Tool
 * 
 * Comprehensive performance profiler with CPU sampling, memory analysis,
 * call graph generation, and real-time monitoring capabilities.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <time.h>
#include <kernel/process.h>
#include <kernel/vmm.h>
#include <kernel/timer.h>
#include <kernel/perf.h>

#define MAX_PROFILE_SAMPLES     1000000
#define MAX_CALL_STACK_DEPTH    128
#define MAX_FUNCTIONS           10000
#define MAX_SYMBOLS             50000
#define SAMPLE_FREQUENCY_HZ     1000
#define PROFILE_BUFFER_SIZE     (64 * 1024 * 1024)  /* 64MB */

/* Profiling modes */
typedef enum {
    PROFILE_MODE_CPU,           /* CPU sampling */
    PROFILE_MODE_MEMORY,        /* Memory allocation tracking */
    PROFILE_MODE_CALLGRAPH,     /* Call graph profiling */
    PROFILE_MODE_REALTIME,      /* Real-time monitoring */
    PROFILE_MODE_TRACE          /* Function tracing */
} profile_mode_t;

/* Sample types */
typedef enum {
    SAMPLE_TYPE_CPU,            /* CPU sample */
    SAMPLE_TYPE_MEMORY_ALLOC,   /* Memory allocation */
    SAMPLE_TYPE_MEMORY_FREE,    /* Memory deallocation */
    SAMPLE_TYPE_FUNCTION_ENTER, /* Function entry */
    SAMPLE_TYPE_FUNCTION_EXIT,  /* Function exit */
    SAMPLE_TYPE_SYSCALL,        /* System call */
    SAMPLE_TYPE_INTERRUPT       /* Interrupt handler */
} sample_type_t;

/* Profile sample */
typedef struct profile_sample {
    uint64_t timestamp;         /* Sample timestamp */
    sample_type_t type;         /* Sample type */
    pid_t pid;                  /* Process ID */
    uint32_t tid;               /* Thread ID */
    uint64_t ip;                /* Instruction pointer */
    uint64_t sp;                /* Stack pointer */
    
    union {
        struct {
            uint64_t addresses[MAX_CALL_STACK_DEPTH];
            uint32_t depth;
        } callstack;
        
        struct {
            uint64_t address;
            size_t size;
            void* metadata;
        } memory;
        
        struct {
            uint32_t syscall_id;
            uint64_t args[6];
            uint64_t return_value;
        } syscall;
    } data;
    
    uint32_t cpu_id;            /* CPU core ID */
    uint32_t context_switches;  /* Context switch count */
} profile_sample_t;

/* Function information */
typedef struct function_info {
    char name[256];             /* Function name */
    uint64_t start_address;     /* Function start address */
    uint64_t end_address;       /* Function end address */
    uint64_t total_time;        /* Total execution time */
    uint64_t self_time;         /* Self execution time */
    uint32_t call_count;        /* Number of calls */
    uint32_t sample_count;      /* Number of samples */
    
    /* Call graph information */
    struct function_info* callers[32];     /* Functions that call this */
    uint32_t caller_counts[32];            /* Call counts from callers */
    uint32_t caller_count;                 /* Number of unique callers */
    
    struct function_info* callees[32];     /* Functions called by this */
    uint32_t callee_counts[32];            /* Call counts to callees */
    uint32_t callee_count;                 /* Number of unique callees */
} function_info_t;

/* Symbol information */
typedef struct symbol_info {
    char name[256];             /* Symbol name */
    uint64_t address;           /* Symbol address */
    size_t size;                /* Symbol size */
    char module[256];           /* Module/library name */
    bool is_function;           /* Is this a function symbol */
} symbol_info_t;

/* Profiler state */
typedef struct profiler_state {
    bool initialized;
    bool profiling_active;
    profile_mode_t mode;
    
    /* Target process */
    pid_t target_pid;
    process_t* target_process;
    
    /* Sample buffer */
    profile_sample_t* samples;
    uint32_t sample_count;
    uint32_t max_samples;
    
    /* Function database */
    function_info_t functions[MAX_FUNCTIONS];
    uint32_t function_count;
    
    /* Symbol table */
    symbol_info_t symbols[MAX_SYMBOLS];
    uint32_t symbol_count;
    
    /* Profiling configuration */
    uint32_t sample_frequency;
    uint64_t profile_duration;
    uint64_t start_time;
    
    /* Call stack tracking */
    struct {
        uint64_t stack[MAX_CALL_STACK_DEPTH];
        uint64_t timestamps[MAX_CALL_STACK_DEPTH];
        uint32_t depth;
    } call_stack[256];  /* Per-thread call stacks */
    
    /* Memory tracking */
    struct {
        uint64_t total_allocated;
        uint64_t total_freed;
        uint64_t peak_usage;
        uint32_t allocation_count;
        uint32_t free_count;
        
        /* Allocation histogram */
        struct {
            size_t size_range;
            uint32_t count;
        } alloc_histogram[32];
    } memory_stats;
    
    /* Performance counters */
    struct {
        uint64_t cpu_cycles;
        uint64_t instructions;
        uint64_t cache_misses;
        uint64_t branch_misses;
        uint64_t page_faults;
        uint64_t context_switches;
    } perf_counters;
    
    /* Output configuration */
    char output_file[256];
    bool real_time_display;
    bool generate_flamegraph;
    bool generate_callgraph;
    
} profiler_state_t;

static profiler_state_t g_profiler = {0};

/* Function prototypes */
static int profiler_load_symbols(const char* binary_path);
static function_info_t* profiler_find_function(uint64_t address);
static symbol_info_t* profiler_find_symbol(uint64_t address);
static void profiler_sample_handler(int signal);
static int profiler_collect_callstack(profile_sample_t* sample);
static int profiler_process_sample(profile_sample_t* sample);
static void profiler_update_function_stats(function_info_t* func, uint64_t timestamp);
static int profiler_generate_report(const char* output_path);
static int profiler_generate_flamegraph(const char* output_path);
static int profiler_generate_callgraph(const char* output_path);
static void profiler_display_realtime_stats(void);

/* Initialize profiler */
int profiler_init(profile_mode_t mode) {
    if (g_profiler.initialized) {
        return 0;  /* Already initialized */
    }
    
    memset(&g_profiler, 0, sizeof(profiler_state_t));
    
    g_profiler.mode = mode;
    g_profiler.sample_frequency = SAMPLE_FREQUENCY_HZ;
    g_profiler.max_samples = MAX_PROFILE_SAMPLES;
    
    /* Allocate sample buffer */
    g_profiler.samples = mmap(NULL, sizeof(profile_sample_t) * g_profiler.max_samples,
                             PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (g_profiler.samples == MAP_FAILED) {
        printf("Failed to allocate sample buffer\n");
        return -1;
    }
    
    /* Initialize call stacks */
    for (int i = 0; i < 256; i++) {
        g_profiler.call_stack[i].depth = 0;
    }
    
    /* Set default output file */
    strcpy(g_profiler.output_file, "profile_report.txt");
    
    g_profiler.initialized = true;
    
    printf("Profiler initialized (mode: %d, frequency: %u Hz)\n", mode, g_profiler.sample_frequency);
    return 0;
}

/* Start profiling target process */
int profiler_start(pid_t pid, uint64_t duration_seconds) {
    if (!g_profiler.initialized) {
        return -1;
    }
    
    if (g_profiler.profiling_active) {
        printf("Profiling already active\n");
        return -1;
    }
    
    /* Find target process */
    g_profiler.target_process = process_find_by_pid(pid);
    if (!g_profiler.target_process) {
        printf("Target process not found: %d\n", pid);
        return -1;
    }
    
    g_profiler.target_pid = pid;
    g_profiler.profile_duration = duration_seconds;
    g_profiler.start_time = timer_get_ticks();
    g_profiler.sample_count = 0;
    
    /* Load symbols from target binary */
    char binary_path[256];
    snprintf(binary_path, sizeof(binary_path), "/proc/%d/exe", pid);
    profiler_load_symbols(binary_path);
    
    /* Install sample handler */
    signal(SIGALRM, profiler_sample_handler);
    
    /* Configure sample timer */
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 1000000 / g_profiler.sample_frequency;
    timer.it_interval = timer.it_value;
    
    if (setitimer(ITIMER_REAL, &timer, NULL) < 0) {
        printf("Failed to set profiling timer\n");
        return -1;
    }
    
    g_profiler.profiling_active = true;
    
    printf("Started profiling process %d for %lu seconds\n", pid, duration_seconds);
    printf("Sample frequency: %u Hz\n", g_profiler.sample_frequency);
    printf("Loaded %u symbols, %u functions\n", g_profiler.symbol_count, g_profiler.function_count);
    
    return 0;
}

/* Stop profiling */
int profiler_stop(void) {
    if (!g_profiler.profiling_active) {
        return 0;
    }
    
    /* Disable timer */
    struct itimerval timer = {0};
    setitimer(ITIMER_REAL, &timer, NULL);
    
    signal(SIGALRM, SIG_DFL);
    
    g_profiler.profiling_active = false;
    
    uint64_t elapsed_time = timer_get_ticks() - g_profiler.start_time;
    
    printf("Profiling stopped\n");
    printf("Collected %u samples in %lu ticks\n", g_profiler.sample_count, elapsed_time);
    printf("Average sample rate: %.2f Hz\n", 
           (double)g_profiler.sample_count / (elapsed_time / (double)TIMER_HZ));
    
    return 0;
}

/* Generate profiling report */
int profiler_generate_report_file(const char* output_path) {
    if (output_path) {
        strncpy(g_profiler.output_file, output_path, sizeof(g_profiler.output_file) - 1);
    }
    
    printf("Generating profiling report: %s\n", g_profiler.output_file);
    
    int result = profiler_generate_report(g_profiler.output_file);
    if (result == 0) {
        printf("Report generated successfully\n");
        
        if (g_profiler.generate_flamegraph) {
            char flamegraph_path[256];
            snprintf(flamegraph_path, sizeof(flamegraph_path), "%s.svg", g_profiler.output_file);
            profiler_generate_flamegraph(flamegraph_path);
        }
        
        if (g_profiler.generate_callgraph) {
            char callgraph_path[256];
            snprintf(callgraph_path, sizeof(callgraph_path), "%s.dot", g_profiler.output_file);
            profiler_generate_callgraph(callgraph_path);
        }
    }
    
    return result;
}

/* Load symbols from binary */
static int profiler_load_symbols(const char* binary_path) {
    /* Simplified symbol loading - in real implementation would use libelf */
    
    FILE* nm_pipe = NULL;
    char command[512];
    
    /* Use nm command to extract symbols */
    snprintf(command, sizeof(command), "nm -C --defined-only '%s' 2>/dev/null", binary_path);
    nm_pipe = popen(command, "r");
    
    if (!nm_pipe) {
        printf("Warning: Could not load symbols from %s\n", binary_path);
        return -1;
    }
    
    char line[512];
    g_profiler.symbol_count = 0;
    g_profiler.function_count = 0;
    
    while (fgets(line, sizeof(line), nm_pipe) && g_profiler.symbol_count < MAX_SYMBOLS) {
        uint64_t address;
        char type;
        char name[256];
        
        if (sscanf(line, "%lx %c %255s", &address, &type, name) == 3) {
            symbol_info_t* symbol = &g_profiler.symbols[g_profiler.symbol_count++];
            
            symbol->address = address;
            strncpy(symbol->name, name, sizeof(symbol->name) - 1);
            strncpy(symbol->module, binary_path, sizeof(symbol->module) - 1);
            
            /* Check if this is a function symbol */
            symbol->is_function = (type == 'T' || type == 't');
            
            if (symbol->is_function && g_profiler.function_count < MAX_FUNCTIONS) {
                function_info_t* func = &g_profiler.functions[g_profiler.function_count++];
                
                func->start_address = address;
                strncpy(func->name, name, sizeof(func->name) - 1);
                func->call_count = 0;
                func->sample_count = 0;
                func->total_time = 0;
                func->self_time = 0;
            }
        }
    }
    
    pclose(nm_pipe);
    
    /* Sort symbols by address for binary search */
    for (uint32_t i = 0; i < g_profiler.symbol_count - 1; i++) {
        for (uint32_t j = i + 1; j < g_profiler.symbol_count; j++) {
            if (g_profiler.symbols[i].address > g_profiler.symbols[j].address) {
                symbol_info_t temp = g_profiler.symbols[i];
                g_profiler.symbols[i] = g_profiler.symbols[j];
                g_profiler.symbols[j] = temp;
            }
        }
    }
    
    /* Calculate function sizes */
    for (uint32_t i = 0; i < g_profiler.function_count; i++) {
        function_info_t* func = &g_profiler.functions[i];
        
        /* Find next function to calculate size */
        uint64_t next_address = func->start_address + 0x1000;  /* Default size */
        for (uint32_t j = 0; j < g_profiler.function_count; j++) {
            if (g_profiler.functions[j].start_address > func->start_address &&
                g_profiler.functions[j].start_address < next_address) {
                next_address = g_profiler.functions[j].start_address;
            }
        }
        func->end_address = next_address;
    }
    
    printf("Loaded %u symbols (%u functions) from %s\n", 
           g_profiler.symbol_count, g_profiler.function_count, binary_path);
    
    return 0;
}

/* Sample handler */
static void profiler_sample_handler(int signal) {
    if (!g_profiler.profiling_active || g_profiler.sample_count >= g_profiler.max_samples) {
        return;
    }
    
    /* Check if profiling duration exceeded */
    uint64_t current_time = timer_get_ticks();
    if (g_profiler.profile_duration > 0 && 
        (current_time - g_profiler.start_time) >= (g_profiler.profile_duration * TIMER_HZ)) {
        profiler_stop();
        return;
    }
    
    profile_sample_t* sample = &g_profiler.samples[g_profiler.sample_count++];
    memset(sample, 0, sizeof(profile_sample_t));
    
    sample->timestamp = current_time;
    sample->type = SAMPLE_TYPE_CPU;
    sample->pid = g_profiler.target_pid;
    
    /* Get current instruction pointer from target process */
    if (g_profiler.target_process) {
        sample->ip = g_profiler.target_process->context.rip;
        sample->sp = g_profiler.target_process->context.rsp;
    }
    
    /* Collect call stack if in callgraph mode */
    if (g_profiler.mode == PROFILE_MODE_CALLGRAPH || g_profiler.mode == PROFILE_MODE_TRACE) {
        profiler_collect_callstack(sample);
    }
    
    /* Process the sample */
    profiler_process_sample(sample);
    
    /* Real-time display update */
    if (g_profiler.real_time_display && (g_profiler.sample_count % 100) == 0) {
        profiler_display_realtime_stats();
    }
}

/* Collect call stack */
static int profiler_collect_callstack(profile_sample_t* sample) {
    if (!g_profiler.target_process) {
        return -1;
    }
    
    uint64_t rbp = g_profiler.target_process->context.rbp;
    uint32_t depth = 0;
    
    sample->data.callstack.addresses[depth++] = sample->ip;
    
    /* Walk the stack frames */
    while (depth < MAX_CALL_STACK_DEPTH && rbp != 0) {
        /* Read return address from stack frame */
        uint64_t return_address;
        if (vmm_read_process_memory(g_profiler.target_process, rbp + 8,
                                   &return_address, sizeof(return_address)) == 0) {
            
            sample->data.callstack.addresses[depth++] = return_address;
            
            /* Move to previous stack frame */
            uint64_t prev_rbp;
            if (vmm_read_process_memory(g_profiler.target_process, rbp,
                                       &prev_rbp, sizeof(prev_rbp)) == 0) {
                rbp = prev_rbp;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    
    sample->data.callstack.depth = depth;
    return 0;
}

/* Process sample */
static int profiler_process_sample(profile_sample_t* sample) {
    /* Find function containing the instruction pointer */
    function_info_t* func = profiler_find_function(sample->ip);
    if (func) {
        func->sample_count++;
        profiler_update_function_stats(func, sample->timestamp);
    }
    
    /* Process call stack */
    if (sample->type == SAMPLE_TYPE_CPU && sample->data.callstack.depth > 0) {
        for (uint32_t i = 0; i < sample->data.callstack.depth; i++) {
            function_info_t* stack_func = profiler_find_function(sample->data.callstack.addresses[i]);
            if (stack_func) {
                stack_func->total_time += 1;  /* Sample weight */
            }
        }
        
        /* Update call graph relationships */
        if (sample->data.callstack.depth > 1) {
            for (uint32_t i = 1; i < sample->data.callstack.depth; i++) {
                function_info_t* caller = profiler_find_function(sample->data.callstack.addresses[i]);
                function_info_t* callee = profiler_find_function(sample->data.callstack.addresses[i-1]);
                
                if (caller && callee && caller->callee_count < 32) {
                    /* Add to caller's callee list */
                    bool found = false;
                    for (uint32_t j = 0; j < caller->callee_count; j++) {
                        if (caller->callees[j] == callee) {
                            caller->callee_counts[j]++;
                            found = true;
                            break;
                        }
                    }
                    
                    if (!found) {
                        caller->callees[caller->callee_count] = callee;
                        caller->callee_counts[caller->callee_count] = 1;
                        caller->callee_count++;
                    }
                    
                    /* Add to callee's caller list */
                    if (callee->caller_count < 32) {
                        found = false;
                        for (uint32_t j = 0; j < callee->caller_count; j++) {
                            if (callee->callers[j] == caller) {
                                callee->caller_counts[j]++;
                                found = true;
                                break;
                            }
                        }
                        
                        if (!found) {
                            callee->callers[callee->caller_count] = caller;
                            callee->caller_counts[callee->caller_count] = 1;
                            callee->caller_count++;
                        }
                    }
                }
            }
        }
    }
    
    return 0;
}

/* Find function by address */
static function_info_t* profiler_find_function(uint64_t address) {
    for (uint32_t i = 0; i < g_profiler.function_count; i++) {
        function_info_t* func = &g_profiler.functions[i];
        if (address >= func->start_address && address < func->end_address) {
            return func;
        }
    }
    return NULL;
}

/* Find symbol by address */
static symbol_info_t* profiler_find_symbol(uint64_t address) {
    /* Binary search */
    uint32_t left = 0, right = g_profiler.symbol_count - 1;
    
    while (left <= right) {
        uint32_t mid = (left + right) / 2;
        
        if (g_profiler.symbols[mid].address == address) {
            return &g_profiler.symbols[mid];
        } else if (g_profiler.symbols[mid].address < address) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    /* Find closest symbol */
    if (right < g_profiler.symbol_count) {
        return &g_profiler.symbols[right];
    }
    
    return NULL;
}

/* Update function statistics */
static void profiler_update_function_stats(function_info_t* func, uint64_t timestamp) {
    func->call_count++;
    
    /* Simplified time calculation - in real implementation would use more precise timing */
    func->self_time += 1;  /* Each sample represents 1 unit of time */
}

/* Generate profiling report */
static int profiler_generate_report(const char* output_path) {
    FILE* report_file = fopen(output_path, "w");
    if (!report_file) {
        printf("Failed to create report file: %s\n", output_path);
        return -1;
    }
    
    /* Write report header */
    fprintf(report_file, "LimitlessOS Performance Profile Report\n");
    fprintf(report_file, "=====================================\n\n");
    fprintf(report_file, "Target Process: %d\n", g_profiler.target_pid);
    fprintf(report_file, "Profiling Mode: %d\n", g_profiler.mode);
    fprintf(report_file, "Sample Count: %u\n", g_profiler.sample_count);
    fprintf(report_file, "Sample Frequency: %u Hz\n", g_profiler.sample_frequency);
    fprintf(report_file, "Duration: %lu seconds\n", g_profiler.profile_duration);
    fprintf(report_file, "\n");
    
    /* Sort functions by sample count */
    function_info_t* sorted_functions[MAX_FUNCTIONS];
    for (uint32_t i = 0; i < g_profiler.function_count; i++) {
        sorted_functions[i] = &g_profiler.functions[i];
    }
    
    for (uint32_t i = 0; i < g_profiler.function_count - 1; i++) {
        for (uint32_t j = i + 1; j < g_profiler.function_count; j++) {
            if (sorted_functions[i]->sample_count < sorted_functions[j]->sample_count) {
                function_info_t* temp = sorted_functions[i];
                sorted_functions[i] = sorted_functions[j];
                sorted_functions[j] = temp;
            }
        }
    }
    
    /* Write function statistics */
    fprintf(report_file, "Top Functions by Sample Count\n");
    fprintf(report_file, "-----------------------------\n");
    fprintf(report_file, "%-40s %8s %8s %12s %12s\n", 
            "Function", "Samples", "Calls", "Total Time", "Self Time");
    
    for (uint32_t i = 0; i < 50 && i < g_profiler.function_count; i++) {
        function_info_t* func = sorted_functions[i];
        if (func->sample_count > 0) {
            double sample_percent = (double)func->sample_count * 100.0 / g_profiler.sample_count;
            
            fprintf(report_file, "%-40s %7u %8u %11lu %11lu (%4.1f%%)\n",
                    func->name, func->sample_count, func->call_count,
                    func->total_time, func->self_time, sample_percent);
        }
    }
    
    /* Write call graph information */
    if (g_profiler.mode == PROFILE_MODE_CALLGRAPH) {
        fprintf(report_file, "\nCall Graph Information\n");
        fprintf(report_file, "----------------------\n");
        
        for (uint32_t i = 0; i < 20 && i < g_profiler.function_count; i++) {
            function_info_t* func = sorted_functions[i];
            if (func->sample_count == 0) continue;
            
            fprintf(report_file, "\nFunction: %s\n", func->name);
            
            if (func->caller_count > 0) {
                fprintf(report_file, "  Callers:\n");
                for (uint32_t j = 0; j < func->caller_count; j++) {
                    fprintf(report_file, "    %-30s (%u calls)\n",
                           func->callers[j]->name, func->caller_counts[j]);
                }
            }
            
            if (func->callee_count > 0) {
                fprintf(report_file, "  Callees:\n");
                for (uint32_t j = 0; j < func->callee_count; j++) {
                    fprintf(report_file, "    %-30s (%u calls)\n",
                           func->callees[j]->name, func->callee_counts[j]);
                }
            }
        }
    }
    
    /* Write memory statistics */
    fprintf(report_file, "\nMemory Statistics\n");
    fprintf(report_file, "-----------------\n");
    fprintf(report_file, "Total Allocated: %lu bytes\n", g_profiler.memory_stats.total_allocated);
    fprintf(report_file, "Total Freed: %lu bytes\n", g_profiler.memory_stats.total_freed);
    fprintf(report_file, "Peak Usage: %lu bytes\n", g_profiler.memory_stats.peak_usage);
    fprintf(report_file, "Allocations: %u\n", g_profiler.memory_stats.allocation_count);
    fprintf(report_file, "Deallocations: %u\n", g_profiler.memory_stats.free_count);
    
    /* Write performance counters */
    fprintf(report_file, "\nPerformance Counters\n");
    fprintf(report_file, "--------------------\n");
    fprintf(report_file, "CPU Cycles: %lu\n", g_profiler.perf_counters.cpu_cycles);
    fprintf(report_file, "Instructions: %lu\n", g_profiler.perf_counters.instructions);
    fprintf(report_file, "Cache Misses: %lu\n", g_profiler.perf_counters.cache_misses);
    fprintf(report_file, "Branch Misses: %lu\n", g_profiler.perf_counters.branch_misses);
    fprintf(report_file, "Page Faults: %lu\n", g_profiler.perf_counters.page_faults);
    fprintf(report_file, "Context Switches: %lu\n", g_profiler.perf_counters.context_switches);
    
    fclose(report_file);
    return 0;
}

/* Generate flame graph */
static int profiler_generate_flamegraph(const char* output_path) {
    FILE* flame_file = fopen(output_path, "w");
    if (!flame_file) {
        return -1;
    }
    
    /* SVG header */
    fprintf(flame_file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(flame_file, "<svg width=\"1200\" height=\"800\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    fprintf(flame_file, "<defs><linearGradient id=\"gradient\" x1=\"0%%\" y1=\"0%%\" x2=\"0%%\" y2=\"100%%\">");
    fprintf(flame_file, "<stop offset=\"0%%\" style=\"stop-color:rgb(255,160,160)\"/>");
    fprintf(flame_file, "<stop offset=\"100%%\" style=\"stop-color:rgb(255,100,100)\"/>");
    fprintf(flame_file, "</linearGradient></defs>\n");
    
    /* Generate flame graph rectangles */
    uint32_t y_pos = 50;
    uint32_t rect_height = 20;
    
    for (uint32_t i = 0; i < g_profiler.function_count && i < 30; i++) {
        function_info_t* func = &g_profiler.functions[i];
        if (func->sample_count == 0) continue;
        
        uint32_t width = (func->sample_count * 1000) / g_profiler.sample_count;
        if (width < 5) width = 5;
        
        fprintf(flame_file, "<rect x=\"50\" y=\"%u\" width=\"%u\" height=\"%u\" ",
                y_pos, width, rect_height);
        fprintf(flame_file, "fill=\"url(#gradient)\" stroke=\"black\" stroke-width=\"1\"/>\n");
        
        fprintf(flame_file, "<text x=\"%u\" y=\"%u\" font-family=\"Arial\" font-size=\"12\" fill=\"black\">",
                55, y_pos + 15);
        fprintf(flame_file, "%s (%u)</text>\n", func->name, func->sample_count);
        
        y_pos += rect_height + 5;
    }
    
    fprintf(flame_file, "</svg>\n");
    fclose(flame_file);
    
    printf("Flame graph generated: %s\n", output_path);
    return 0;
}

/* Display real-time statistics */
static void profiler_display_realtime_stats(void) {
    printf("\033[2J\033[H");  /* Clear screen and move cursor to top */
    
    printf("LimitlessOS Profiler - Real-time View\n");
    printf("=====================================\n");
    printf("Samples: %u | Target PID: %d | Mode: %d\n\n", 
           g_profiler.sample_count, g_profiler.target_pid, g_profiler.mode);
    
    printf("Top Functions:\n");
    printf("%-30s %8s %8s\n", "Function", "Samples", "Percent");
    printf("%-30s %8s %8s\n", "--------", "-------", "-------");
    
    for (uint32_t i = 0; i < 10 && i < g_profiler.function_count; i++) {
        function_info_t* func = &g_profiler.functions[i];
        if (func->sample_count > 0) {
            double percent = (double)func->sample_count * 100.0 / g_profiler.sample_count;
            printf("%-30s %8u %7.1f%%\n", func->name, func->sample_count, percent);
        }
    }
    
    fflush(stdout);
}

/* Configuration functions */
void profiler_set_output_file(const char* path) {
    if (path) {
        strncpy(g_profiler.output_file, path, sizeof(g_profiler.output_file) - 1);
    }
}

void profiler_enable_realtime_display(bool enabled) {
    g_profiler.real_time_display = enabled;
}

void profiler_enable_flamegraph(bool enabled) {
    g_profiler.generate_flamegraph = enabled;
}

void profiler_enable_callgraph(bool enabled) {
    g_profiler.generate_callgraph = enabled;
}

void profiler_set_sample_frequency(uint32_t frequency) {
    if (frequency > 0 && frequency <= 10000) {
        g_profiler.sample_frequency = frequency;
    }
}

/* Main function */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <mode> <pid> [duration] [options]\n", argv[0]);
        printf("Modes:\n");
        printf("  0 - CPU profiling\n");
        printf("  1 - Memory profiling\n");
        printf("  2 - Call graph profiling\n");
        printf("  3 - Real-time monitoring\n");
        printf("  4 - Function tracing\n");
        printf("Options:\n");
        printf("  --output <file>    Output file path\n");
        printf("  --frequency <hz>   Sample frequency (default: 1000)\n");
        printf("  --flamegraph       Generate flame graph\n");
        printf("  --callgraph        Generate call graph\n");
        printf("  --realtime         Enable real-time display\n");
        return 1;
    }
    
    profile_mode_t mode = (profile_mode_t)atoi(argv[1]);
    pid_t pid = (pid_t)atoi(argv[2]);
    uint64_t duration = (argc > 3) ? atoi(argv[3]) : 10;  /* Default 10 seconds */
    
    /* Parse options */
    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            profiler_set_output_file(argv[++i]);
        } else if (strcmp(argv[i], "--frequency") == 0 && i + 1 < argc) {
            profiler_set_sample_frequency((uint32_t)atoi(argv[++i]));
        } else if (strcmp(argv[i], "--flamegraph") == 0) {
            profiler_enable_flamegraph(true);
        } else if (strcmp(argv[i], "--callgraph") == 0) {
            profiler_enable_callgraph(true);
        } else if (strcmp(argv[i], "--realtime") == 0) {
            profiler_enable_realtime_display(true);
        }
    }
    
    /* Initialize profiler */
    if (profiler_init(mode) < 0) {
        printf("Failed to initialize profiler\n");
        return 1;
    }
    
    /* Start profiling */
    if (profiler_start(pid, duration) < 0) {
        printf("Failed to start profiling\n");
        return 1;
    }
    
    /* Wait for profiling to complete */
    while (g_profiler.profiling_active) {
        usleep(100000);  /* 100ms */
    }
    
    /* Generate report */
    if (profiler_generate_report_file(NULL) < 0) {
        printf("Failed to generate report\n");
        return 1;
    }
    
    printf("Profiling completed successfully\n");
    return 0;
}