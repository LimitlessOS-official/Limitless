/*
 * trace_analyzer.c - LimitlessOS Trace Analysis Tool
 * 
 * Advanced trace analysis with flame graphs, performance bottlenecks,
 * and real-time monitoring dashboard.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_TRACE_EVENTS    1000000
#define MAX_FUNCTIONS       10000
#define MAX_STACK_DEPTH     64
#define MAX_FLAME_NODES     50000

/* Trace event structure (matches kernel) */
typedef struct trace_event {
    uint64_t timestamp;
    uint32_t type;
    uint32_t pid;
    uint32_t tid;
    uint64_t data1;
    uint64_t data2;
    uint64_t data3;
    char description[128];
} trace_event_t;

/* Function call info */
typedef struct function_info {
    char name[128];
    uint64_t total_time;
    uint64_t call_count;
    uint64_t min_time;
    uint64_t max_time;
    uint64_t self_time;
} function_info_t;

/* Call stack entry */
typedef struct call_stack_entry {
    char function[128];
    uint64_t start_time;
    uint64_t self_time_start;
} call_stack_entry_t;

/* Flame graph node */
typedef struct flame_node {
    char name[128];
    uint64_t total_time;
    uint64_t self_time;
    struct flame_node* parent;
    struct flame_node* children;
    struct flame_node* sibling;
    uint32_t call_count;
} flame_node_t;

/* Performance hotspot */
typedef struct hotspot {
    char function[128];
    char file[256];
    uint32_t line;
    uint64_t time_spent;
    uint32_t call_count;
    double cpu_percent;
} hotspot_t;

/* Global analysis state */
static struct {
    trace_event_t* events;
    uint32_t event_count;
    
    function_info_t functions[MAX_FUNCTIONS];
    uint32_t function_count;
    
    flame_node_t flame_nodes[MAX_FLAME_NODES];
    uint32_t flame_node_count;
    flame_node_t* flame_root;
    
    hotspot_t hotspots[1000];
    uint32_t hotspot_count;
    
    uint64_t total_trace_time;
    uint64_t analysis_start_time;
    uint64_t analysis_end_time;
} analyzer;

/* Function prototypes */
static int load_trace_file(const char* filename);
static void analyze_function_calls(void);
static void build_flame_graph(void);
static void find_performance_hotspots(void);
static void generate_flame_graph_svg(const char* filename);
static void generate_report(const char* filename);
static function_info_t* find_or_create_function(const char* name);
static flame_node_t* find_or_create_flame_node(flame_node_t* parent, const char* name);
static void print_flame_node_svg(FILE* fp, flame_node_t* node, int x, int y, int width, int depth);

/* Main analysis function */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <trace_file> [output_dir]\n", argv[0]);
        return 1;
    }
    
    const char* trace_file = argv[1];
    const char* output_dir = argc > 2 ? argv[2] : ".";
    
    printf("LimitlessOS Trace Analyzer\n");
    printf("Loading trace file: %s\n", trace_file);
    
    /* Load trace data */
    if (load_trace_file(trace_file) < 0) {
        fprintf(stderr, "Failed to load trace file\n");
        return 1;
    }
    
    printf("Loaded %u trace events\n", analyzer.event_count);
    
    /* Perform analysis */
    printf("Analyzing function calls...\n");
    analyze_function_calls();
    
    printf("Building flame graph...\n");
    build_flame_graph();
    
    printf("Finding performance hotspots...\n");
    find_performance_hotspots();
    
    /* Generate outputs */
    char output_path[512];
    
    snprintf(output_path, sizeof(output_path), "%s/flamegraph.svg", output_dir);
    printf("Generating flame graph: %s\n", output_path);
    generate_flame_graph_svg(output_path);
    
    snprintf(output_path, sizeof(output_path), "%s/analysis_report.txt", output_dir);
    printf("Generating analysis report: %s\n", output_path);
    generate_report(output_path);
    
    printf("Analysis complete!\n");
    return 0;
}

/* Load trace file */
static int load_trace_file(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }
    
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return -1;
    }
    
    /* Map trace file into memory */
    void* mapped = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }
    
    close(fd);
    
    /* Parse trace events */
    analyzer.events = (trace_event_t*)mapped;
    analyzer.event_count = st.st_size / sizeof(trace_event_t);
    
    if (analyzer.event_count > 0) {
        analyzer.analysis_start_time = analyzer.events[0].timestamp;
        analyzer.analysis_end_time = analyzer.events[analyzer.event_count - 1].timestamp;
        analyzer.total_trace_time = analyzer.analysis_end_time - analyzer.analysis_start_time;
    }
    
    return 0;
}

/* Analyze function calls */
static void analyze_function_calls(void) {
    call_stack_entry_t call_stack[MAX_STACK_DEPTH];
    int stack_depth = 0;
    
    for (uint32_t i = 0; i < analyzer.event_count; i++) {
        trace_event_t* event = &analyzer.events[i];
        
        if (event->type == 0) {  /* TRACE_EVENT_FUNCTION_ENTER */
            if (stack_depth < MAX_STACK_DEPTH) {
                call_stack_entry_t* entry = &call_stack[stack_depth++];
                strncpy(entry->function, event->description, sizeof(entry->function) - 1);
                entry->start_time = event->timestamp;
                entry->self_time_start = event->timestamp;
            }
        } else if (event->type == 1) {  /* TRACE_EVENT_FUNCTION_EXIT */
            if (stack_depth > 0) {
                call_stack_entry_t* entry = &call_stack[--stack_depth];
                
                if (strcmp(entry->function, event->description) == 0) {
                    uint64_t total_time = event->timestamp - entry->start_time;
                    uint64_t self_time = event->timestamp - entry->self_time_start;
                    
                    function_info_t* func = find_or_create_function(entry->function);
                    if (func) {
                        func->call_count++;
                        func->total_time += total_time;
                        func->self_time += self_time;
                        
                        if (func->call_count == 1) {
                            func->min_time = total_time;
                            func->max_time = total_time;
                        } else {
                            if (total_time < func->min_time) func->min_time = total_time;
                            if (total_time > func->max_time) func->max_time = total_time;
                        }
                    }
                    
                    /* Subtract time from parent's self time */
                    if (stack_depth > 0) {
                        call_stack[stack_depth - 1].self_time_start = event->timestamp;
                    }
                }
            }
        }
    }
}

/* Build flame graph */
static void build_flame_graph(void) {
    call_stack_entry_t call_stack[MAX_STACK_DEPTH];
    int stack_depth = 0;
    
    /* Create root node */
    analyzer.flame_root = &analyzer.flame_nodes[analyzer.flame_node_count++];
    memset(analyzer.flame_root, 0, sizeof(flame_node_t));
    strcpy(analyzer.flame_root->name, "root");
    
    for (uint32_t i = 0; i < analyzer.event_count; i++) {
        trace_event_t* event = &analyzer.events[i];
        
        if (event->type == 0) {  /* TRACE_EVENT_FUNCTION_ENTER */
            if (stack_depth < MAX_STACK_DEPTH) {
                call_stack_entry_t* entry = &call_stack[stack_depth++];
                strncpy(entry->function, event->description, sizeof(entry->function) - 1);
                entry->start_time = event->timestamp;
            }
        } else if (event->type == 1) {  /* TRACE_EVENT_FUNCTION_EXIT */
            if (stack_depth > 0) {
                call_stack_entry_t* entry = &call_stack[--stack_depth];
                
                if (strcmp(entry->function, event->description) == 0) {
                    uint64_t duration = event->timestamp - entry->start_time;
                    
                    /* Find or create flame graph nodes for call stack */
                    flame_node_t* current = analyzer.flame_root;
                    
                    for (int j = 0; j < stack_depth + 1; j++) {
                        const char* func_name = (j < stack_depth) ? 
                                              call_stack[j].function : entry->function;
                        current = find_or_create_flame_node(current, func_name);
                    }
                    
                    if (current) {
                        current->total_time += duration;
                        current->call_count++;
                    }
                }
            }
        }
    }
}

/* Find performance hotspots */
static void find_performance_hotspots(void) {
    /* Sort functions by total time spent */
    for (uint32_t i = 0; i < analyzer.function_count; i++) {
        function_info_t* func = &analyzer.functions[i];
        
        if (analyzer.hotspot_count < 1000) {
            hotspot_t* hotspot = &analyzer.hotspots[analyzer.hotspot_count++];
            strcpy(hotspot->function, func->name);
            hotspot->time_spent = func->total_time;
            hotspot->call_count = func->call_count;
            hotspot->cpu_percent = (double)func->total_time * 100.0 / analyzer.total_trace_time;
        }
    }
    
    /* Sort hotspots by time spent (bubble sort for simplicity) */
    for (uint32_t i = 0; i < analyzer.hotspot_count - 1; i++) {
        for (uint32_t j = 0; j < analyzer.hotspot_count - 1 - i; j++) {
            if (analyzer.hotspots[j].time_spent < analyzer.hotspots[j + 1].time_spent) {
                hotspot_t temp = analyzer.hotspots[j];
                analyzer.hotspots[j] = analyzer.hotspots[j + 1];
                analyzer.hotspots[j + 1] = temp;
            }
        }
    }
}

/* Generate flame graph SVG */
static void generate_flame_graph_svg(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return;
    }
    
    const int width = 1200;
    const int height = 800;
    const int frame_height = 16;
    
    /* SVG header */
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp, "<svg width=\"%d\" height=\"%d\" xmlns=\"http://www.w3.org/2000/svg\">\n", 
            width, height);
    fprintf(fp, "<defs><linearGradient id=\"background\" x1=\"0%%\" y1=\"0%%\" x2=\"0%%\" y2=\"100%%\">");
    fprintf(fp, "<stop offset=\"0%%\" style=\"stop-color:rgb(255,255,255);stop-opacity:1\" />");
    fprintf(fp, "<stop offset=\"100%%\" style=\"stop-color:rgb(240,240,240);stop-opacity:1\" />");
    fprintf(fp, "</linearGradient></defs>\n");
    fprintf(fp, "<rect width=\"100%%\" height=\"100%%\" fill=\"url(#background)\"/>\n");
    
    /* Title */
    fprintf(fp, "<text x=\"%d\" y=\"24\" text-anchor=\"middle\" font-family=\"Arial\" font-size=\"16\" font-weight=\"bold\">", 
            width / 2);
    fprintf(fp, "LimitlessOS Flame Graph</text>\n");
    
    /* Draw flame graph recursively */
    if (analyzer.flame_root && analyzer.flame_root->children) {
        print_flame_node_svg(fp, analyzer.flame_root->children, 0, 50, width, 0);
    }
    
    fprintf(fp, "</svg>\n");
    fclose(fp);
}

/* Print flame node to SVG */
static void print_flame_node_svg(FILE* fp, flame_node_t* node, int x, int y, int width, int depth) {
    const int frame_height = 16;
    const int max_depth = 40;
    
    if (depth > max_depth || !node) {
        return;
    }
    
    /* Calculate width based on time percentage */
    double time_ratio = (double)node->total_time / analyzer.flame_root->total_time;
    int node_width = (int)(width * time_ratio);
    
    if (node_width < 1) {
        return;
    }
    
    /* Generate color based on function name hash */
    uint32_t hash = 0;
    for (const char* p = node->name; *p; p++) {
        hash = hash * 31 + *p;
    }
    
    uint8_t r = 200 + (hash % 55);
    uint8_t g = 100 + ((hash >> 8) % 155);
    uint8_t b = 100 + ((hash >> 16) % 155);
    
    /* Draw rectangle */
    fprintf(fp, "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" ", 
            x, y, node_width, frame_height);
    fprintf(fp, "fill=\"rgb(%d,%d,%d)\" stroke=\"black\" stroke-width=\"1\"/>\n", r, g, b);
    
    /* Add text if wide enough */
    if (node_width > 50) {
        fprintf(fp, "<text x=\"%d\" y=\"%d\" font-family=\"Arial\" font-size=\"12\" fill=\"black\">", 
                x + 4, y + 13);
        
        /* Truncate long function names */
        char display_name[64];
        strncpy(display_name, node->name, sizeof(display_name) - 1);
        display_name[sizeof(display_name) - 1] = '\0';
        
        if (strlen(display_name) * 7 > node_width - 8) {
            int max_chars = (node_width - 8) / 7;
            if (max_chars > 3) {
                display_name[max_chars - 3] = '.';
                display_name[max_chars - 2] = '.';
                display_name[max_chars - 1] = '.';
                display_name[max_chars] = '\0';
            }
        }
        
        fprintf(fp, "%s</text>\n", display_name);
    }
    
    /* Draw children */
    int child_x = x;
    for (flame_node_t* child = node->children; child; child = child->sibling) {
        double child_ratio = (double)child->total_time / node->total_time;
        int child_width = (int)(node_width * child_ratio);
        
        print_flame_node_svg(fp, child, child_x, y + frame_height + 2, child_width, depth + 1);
        child_x += child_width;
    }
}

/* Generate analysis report */
static void generate_report(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return;
    }
    
    fprintf(fp, "LimitlessOS Trace Analysis Report\n");
    fprintf(fp, "=================================\n\n");
    
    fprintf(fp, "Analysis Summary:\n");
    fprintf(fp, "- Total trace time: %.2f ms\n", analyzer.total_trace_time / 1000.0);
    fprintf(fp, "- Total events: %u\n", analyzer.event_count);
    fprintf(fp, "- Unique functions: %u\n", analyzer.function_count);
    fprintf(fp, "- Analysis period: %lu - %lu us\n\n", 
            analyzer.analysis_start_time, analyzer.analysis_end_time);
    
    fprintf(fp, "Top Performance Hotspots:\n");
    fprintf(fp, "-------------------------\n");
    fprintf(fp, "%-40s %12s %10s %8s\n", "Function", "Time (us)", "Calls", "CPU %%");
    fprintf(fp, "%-40s %12s %10s %8s\n", "--------", "--------", "-----", "-----");
    
    for (uint32_t i = 0; i < analyzer.hotspot_count && i < 20; i++) {
        hotspot_t* hotspot = &analyzer.hotspots[i];
        fprintf(fp, "%-40s %12lu %10u %7.2f%%\n",
                hotspot->function, hotspot->time_spent, hotspot->call_count, hotspot->cpu_percent);
    }
    
    fprintf(fp, "\nDetailed Function Statistics:\n");
    fprintf(fp, "----------------------------\n");
    fprintf(fp, "%-40s %12s %10s %12s %12s %12s\n", 
            "Function", "Total (us)", "Calls", "Avg (us)", "Min (us)", "Max (us)");
    fprintf(fp, "%-40s %12s %10s %12s %12s %12s\n", 
            "--------", "---------", "-----", "--------", "--------", "--------");
    
    for (uint32_t i = 0; i < analyzer.function_count; i++) {
        function_info_t* func = &analyzer.functions[i];
        uint64_t avg_time = func->call_count > 0 ? func->total_time / func->call_count : 0;
        
        fprintf(fp, "%-40s %12lu %10lu %12lu %12lu %12lu\n",
                func->name, func->total_time, func->call_count, 
                avg_time, func->min_time, func->max_time);
    }
    
    fclose(fp);
}

/* Find or create function info */
static function_info_t* find_or_create_function(const char* name) {
    /* Search existing functions */
    for (uint32_t i = 0; i < analyzer.function_count; i++) {
        if (strcmp(analyzer.functions[i].name, name) == 0) {
            return &analyzer.functions[i];
        }
    }
    
    /* Create new function */
    if (analyzer.function_count < MAX_FUNCTIONS) {
        function_info_t* func = &analyzer.functions[analyzer.function_count++];
        memset(func, 0, sizeof(function_info_t));
        strncpy(func->name, name, sizeof(func->name) - 1);
        return func;
    }
    
    return NULL;
}

/* Find or create flame graph node */
static flame_node_t* find_or_create_flame_node(flame_node_t* parent, const char* name) {
    /* Search existing children */
    for (flame_node_t* child = parent->children; child; child = child->sibling) {
        if (strcmp(child->name, name) == 0) {
            return child;
        }
    }
    
    /* Create new child */
    if (analyzer.flame_node_count < MAX_FLAME_NODES) {
        flame_node_t* node = &analyzer.flame_nodes[analyzer.flame_node_count++];
        memset(node, 0, sizeof(flame_node_t));
        strncpy(node->name, name, sizeof(node->name) - 1);
        node->parent = parent;
        
        /* Add to parent's children list */
        node->sibling = parent->children;
        parent->children = node;
        
        return node;
    }
    
    return NULL;
}