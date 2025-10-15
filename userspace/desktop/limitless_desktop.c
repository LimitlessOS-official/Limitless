/**
 * LimitlessOS Desktop Environment Implementation - Command Center Core
 * 
 * The neural center of LimitlessOS user experience. Every interaction is calculated,
 * every pixel serves a purpose, every animation conveys meaning.
 */

#include "limitless_desktop.h"
#include "../ui/limitlessui.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// ============================================================================
// GLOBAL DESKTOP STATE
// ============================================================================

static desktop_environment_t g_desktop = {0};

// Default applications that ship with LimitlessOS
static const desktop_application_t g_default_applications[] = {
    {
        .name = "Limitless Terminal",
        .description = "Advanced terminal with AI assistance and system integration",
        .executable_path = "/usr/bin/limitless-terminal",
        .icon_path = "/usr/share/icons/limitless/terminal.svg",
        .keywords = "terminal console command shell cli bash zsh",
        .category = APP_CATEGORY_SYSTEM,
        .security_level_required = SECURITY_LEVEL_OPEN,
        .system_integration = true,
        .autostart_enabled = false,
        .background_allowed = true
    },
    {
        .name = "Limitless Files", 
        .description = "Intelligent file manager with AI-powered organization",
        .executable_path = "/usr/bin/limitless-files",
        .icon_path = "/usr/share/icons/limitless/files.svg",
        .keywords = "files folders explorer manager browse directory",
        .category = APP_CATEGORY_SYSTEM,
        .security_level_required = SECURITY_LEVEL_OPEN,
        .system_integration = true,
        .autostart_enabled = false,
        .background_allowed = false
    },
    {
        .name = "Limitless Browser",
        .description = "Next-generation browser with AI integration and privacy focus",
        .executable_path = "/usr/bin/limitless-browser", 
        .icon_path = "/usr/share/icons/limitless/browser.svg",
        .keywords = "browser web internet http https surf navigate",
        .category = APP_CATEGORY_WEB,
        .security_level_required = SECURITY_LEVEL_OPEN,
        .system_integration = true,
        .autostart_enabled = false,
        .background_allowed = true
    },
    {
        .name = "Limitless Editor",
        .description = "Professional text editor with AI-powered coding assistance", 
        .executable_path = "/usr/bin/limitless-editor",
        .icon_path = "/usr/share/icons/limitless/editor.svg",
        .keywords = "editor text code programming development ide write",
        .category = APP_CATEGORY_DEVELOPMENT,
        .security_level_required = SECURITY_LEVEL_OPEN,
        .system_integration = true,
        .autostart_enabled = false,
        .background_allowed = false
    },
    {
        .name = "System Monitor",
        .description = "Real-time system performance and security monitoring",
        .executable_path = "/usr/bin/limitless-monitor",
        .icon_path = "/usr/share/icons/limitless/monitor.svg", 
        .keywords = "monitor system performance cpu memory disk network security",
        .category = APP_CATEGORY_SYSTEM,
        .security_level_required = SECURITY_LEVEL_OPEN,
        .system_integration = true,
        .autostart_enabled = true,
        .background_allowed = true
    },
    {
        .name = "Settings",
        .description = "System configuration and personalization center",
        .executable_path = "/usr/bin/limitless-settings",
        .icon_path = "/usr/share/icons/limitless/settings.svg",
        .keywords = "settings preferences configuration control panel system",
        .category = APP_CATEGORY_SYSTEM,
        .security_level_required = SECURITY_LEVEL_OPEN,
        .system_integration = true,
        .autostart_enabled = false,
        .background_allowed = false
    },
    {
        .name = "Calculator",
        .description = "Advanced calculator with scientific and programming modes",
        .executable_path = "/usr/bin/limitless-calculator",
        .icon_path = "/usr/share/icons/limitless/calculator.svg",
        .keywords = "calculator math arithmetic scientific programming hex binary",
        .category = APP_CATEGORY_PRODUCTIVITY,
        .security_level_required = SECURITY_LEVEL_OPEN,
        .system_integration = false,
        .autostart_enabled = false,
        .background_allowed = false
    }
};

static const uint32_t g_default_applications_count = 
    sizeof(g_default_applications) / sizeof(g_default_applications[0]);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

static uint64_t desktop_get_timestamp_ms(void) {
    // TODO: Integrate with LimitlessOS high-precision timer
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static float desktop_calculate_productivity_score(void) {
    // AI-powered productivity calculation based on various metrics
    uint64_t current_time = desktop_get_timestamp_ms();
    uint64_t session_duration = current_time - (current_time % (8 * 60 * 60 * 1000)); // Start of work day
    
    float base_score = 0.5f; // Neutral baseline
    
    // Factor in focus time
    float focus_ratio = (float)g_desktop.ai_insights.productive_time_today_ms / (session_duration + 1);
    base_score += focus_ratio * 0.3f;
    
    // Factor in window switch frequency (lower is better for focus)
    float switch_penalty = fminf(g_desktop.performance_stats.window_switches_count / 100.0f, 0.2f);
    base_score -= switch_penalty;
    
    // Factor in current stress level  
    base_score -= g_desktop.ai_insights.stress_level * 0.2f;
    
    return fmaxf(0.0f, fminf(1.0f, base_score));
}

static void desktop_update_ai_insights_internal(void) {
    if (!g_desktop.ai_insights.enabled) return;
    
    // Update productivity score
    g_desktop.ai_insights.productivity_score = desktop_calculate_productivity_score();
    
    // Detect stress level based on interaction patterns
    uint64_t current_time = desktop_get_timestamp_ms();
    
    // High window switching indicates potential stress/distraction
    if (g_desktop.performance_stats.window_switches_count > 50) {
        g_desktop.ai_insights.stress_level = fminf(1.0f, g_desktop.ai_insights.stress_level + 0.1f);
    } else {
        g_desktop.ai_insights.stress_level = fmaxf(0.0f, g_desktop.ai_insights.stress_level - 0.05f);
    }
    
    // Update current activity based on focused window
    if (g_desktop.focused_window && strlen(g_desktop.focused_window->title) > 0) {
        snprintf(g_desktop.ai_insights.current_activity, 
                sizeof(g_desktop.ai_insights.current_activity),
                "Working on: %s", g_desktop.focused_window->title);
    }
}

// ============================================================================
// CORE DESKTOP LIFECYCLE
// ============================================================================

bool desktop_init(void) {
    if (g_desktop.initialized) {
        printf("[Desktop] Already initialized\n");
        return false;
    }
    
    printf("[Desktop] Initializing LimitlessOS Desktop Environment v%s\n", desktop_get_version_string());
    
    // Initialize LimitlessUI if not already done
    if (!lui_init(LUI_RENDERER_OPENGL)) {
        printf("[Desktop] ERROR: Failed to initialize LimitlessUI\n");
        return false;
    }
    
    // Clear desktop state
    memset(&g_desktop, 0, sizeof(g_desktop));
    
    // Set default configuration
    g_desktop.current_theme = DESKTOP_THEME_TACTICAL_DARK;
    g_desktop.ui_scale = 1.0f;
    g_desktop.dark_mode = true;
    g_desktop.animations_enabled = true;
    g_desktop.transparency_enabled = true;
    g_desktop.default_window_mode = WINDOW_MODE_FLOATING;
    g_desktop.ai_mode = AI_MODE_STANDARD;
    g_desktop.security_level = SECURITY_LEVEL_OPEN;
    g_desktop.auto_arrange_enabled = true;
    g_desktop.smart_notifications_enabled = true;
    g_desktop.max_windows = 256;
    g_desktop.max_notifications = 50;
    g_desktop.system_refresh_interval_ms = 1000; // 1 second
    
    // Allocate window management arrays
    g_desktop.managed_windows = calloc(g_desktop.max_windows, sizeof(lui_window_t*));
    if (!g_desktop.managed_windows) {
        printf("[Desktop] ERROR: Failed to allocate window management memory\n");
        return false;
    }
    
    // Create desktop window (full screen background)
    g_desktop.desktop_window = lui_create_window("LimitlessOS Desktop", LUI_WINDOW_DESKTOP, 
                                               0, 0, 1920, 1080);
    if (!g_desktop.desktop_window) {
        printf("[Desktop] ERROR: Failed to create desktop window\n");
        free(g_desktop.managed_windows);
        return false;
    }
    
    // Configure desktop window
    g_desktop.desktop_root = g_desktop.desktop_window->root_widget;
    
    // Set desktop background based on theme
    if (g_desktop.dark_mode) {
        g_desktop.desktop_root->background_color = LUI_COLOR_TACTICAL_BLACK;
    } else {
        g_desktop.desktop_root->background_color = LUI_COLOR_ARCTIC_WHITE;
    }
    
    // Create taskbar
    g_desktop.taskbar = lui_create_container(g_desktop.desktop_root);
    if (!g_desktop.taskbar) {
        printf("[Desktop] ERROR: Failed to create taskbar\n");
        desktop_shutdown();
        return false;
    }
    
    // Configure taskbar
    strcpy(g_desktop.taskbar->name, "taskbar");
    g_desktop.taskbar->bounds = lui_rect_make(0, 1080 - TASKBAR_HEIGHT, 1920, TASKBAR_HEIGHT);
    g_desktop.taskbar->background_color = LUI_COLOR_GRAPHITE;
    g_desktop.taskbar->corner_radius = 0;
    
    // Create default workspace
    desktop_workspace_t* default_workspace = desktop_create_workspace("Main", WORKSPACE_TYPE_GENERAL);
    if (!default_workspace) {
        printf("[Desktop] ERROR: Failed to create default workspace\n");
        desktop_shutdown();
        return false;
    }
    
    g_desktop.current_workspace = default_workspace;
    
    // Register default applications
    for (uint32_t i = 0; i < g_default_applications_count; i++) {
        desktop_register_application(&g_default_applications[i]);
    }
    
    // Initialize AI insights
    g_desktop.ai_insights.enabled = (g_desktop.ai_mode > AI_MODE_DISABLED);
    g_desktop.ai_insights.productivity_score = 0.5f;
    
    // Show desktop window
    lui_show_window(g_desktop.desktop_window);
    
    g_desktop.initialized = true;
    g_desktop.running = true;
    
    printf("[Desktop] Desktop environment initialized successfully\n");
    printf("[Desktop] Theme: %s, Scale: %.1fx, Windows: %u, Security: Level %u\n", 
           (g_desktop.current_theme == DESKTOP_THEME_TACTICAL_DARK) ? "Tactical Dark" : "Other",
           g_desktop.ui_scale, g_desktop.max_windows, g_desktop.security_level);
    
    return true;
}

void desktop_shutdown(void) {
    if (!g_desktop.initialized) {
        return;
    }
    
    printf("[Desktop] Shutting down desktop environment\n");
    
    g_desktop.running = false;
    
    // Destroy all managed windows
    for (uint32_t i = 0; i < g_desktop.window_count; i++) {
        if (g_desktop.managed_windows[i]) {
            lui_destroy_window(g_desktop.managed_windows[i]);
        }
    }
    
    // Destroy workspaces
    desktop_workspace_t* workspace = g_desktop.workspaces;
    while (workspace) {
        desktop_workspace_t* next = workspace->next;
        desktop_destroy_workspace(workspace);
        workspace = next;
    }
    
    // Free application list
    desktop_application_t* app = g_desktop.applications;
    while (app) {
        desktop_application_t* next = app->next;
        free(app);
        app = next;
    }
    
    // Clear notifications
    desktop_notification_t* notif = g_desktop.notifications;
    while (notif) {
        desktop_notification_t* next = notif->next;
        free(notif);
        notif = next;
    }
    
    // Destroy desktop window
    if (g_desktop.desktop_window) {
        lui_destroy_window(g_desktop.desktop_window);
    }
    
    // Free window management memory
    if (g_desktop.managed_windows) {
        free(g_desktop.managed_windows);
    }
    
    // Clear state
    memset(&g_desktop, 0, sizeof(g_desktop));
    
    printf("[Desktop] Shutdown complete\n");
}

void desktop_run(void) {
    if (!g_desktop.initialized || !g_desktop.running) {
        return;
    }
    
    printf("[Desktop] Starting desktop main loop\n");
    
    uint64_t last_system_update = 0;
    uint64_t last_ai_update = 0;
    uint64_t frame_count = 0;
    
    while (g_desktop.running) {
        uint64_t frame_start = desktop_get_timestamp_ms();
        
        // Process UI events
        if (!lui_process_events()) {
            // No more events, can exit
            break;
        }
        
        // Update system info periodically
        if (frame_start - last_system_update >= g_desktop.system_refresh_interval_ms) {
            desktop_update_system_info();
            last_system_update = frame_start;
        }
        
        // Update AI insights periodically
        if (frame_start - last_ai_update >= 5000) { // Every 5 seconds
            desktop_update_ai_insights_internal();
            last_ai_update = frame_start;
        }
        
        // Render desktop
        lui_context_t* ctx = lui_begin_frame(g_desktop.desktop_window);
        if (ctx) {
            // Render desktop components
            lui_render_widget(g_desktop.desktop_root, ctx);
            
            // Add performance overlay in debug mode
            if (lui_get_config_bool("debug_mode", false)) {
                char perf_text[256];
                snprintf(perf_text, sizeof(perf_text), 
                        "FPS: %u | Frame: %llu | Windows: %u | AI: %.1f%%", 
                        g_desktop.performance_stats.current_fps, 
                        frame_count,
                        g_desktop.window_count,
                        g_desktop.ai_insights.productivity_score * 100.0f);
                
                lui_draw_text(ctx, perf_text, (lui_point_t){10, 10}, 
                             &LUI_TYPOGRAPHY_LABEL_MEDIUM, LUI_COLOR_MISSION_GREEN);
            }
            
            lui_end_frame(ctx);
        }
        
        // Update performance stats
        uint64_t frame_end = desktop_get_timestamp_ms();
        g_desktop.performance_stats.frames_rendered++;
        g_desktop.performance_stats.average_frame_time_us = 
            (uint32_t)((frame_end - frame_start) * 1000);
        
        frame_count++;
        
        // Simple FPS calculation (update every second)
        static uint64_t last_fps_update = 0;
        static uint64_t frames_in_second = 0;
        
        frames_in_second++;
        if (frame_end - last_fps_update >= 1000) {
            g_desktop.performance_stats.current_fps = (uint32_t)frames_in_second;
            frames_in_second = 0;
            last_fps_update = frame_end;
        }
        
        // Maintain target frame rate
        uint32_t target_frame_time = 1000 / DESKTOP_REFRESH_RATE;
        uint32_t actual_frame_time = (uint32_t)(frame_end - frame_start);
        if (actual_frame_time < target_frame_time) {
            // Sleep for remaining time (simplified - real implementation would use proper timing)
            // usleep((target_frame_time - actual_frame_time) * 1000);
        }
    }
    
    printf("[Desktop] Desktop main loop ended\n");
}

void desktop_stop(void) {
    g_desktop.running = false;
    printf("[Desktop] Desktop stop requested\n");
}

desktop_environment_t* desktop_get_instance(void) {
    return g_desktop.initialized ? &g_desktop : NULL;
}

// ============================================================================
// WORKSPACE MANAGEMENT  
// ============================================================================

desktop_workspace_t* desktop_create_workspace(const char* name, workspace_type_t type) {
    if (!g_desktop.initialized) {
        return NULL;
    }
    
    desktop_workspace_t* workspace = calloc(1, sizeof(desktop_workspace_t));
    if (!workspace) {
        printf("[Desktop] ERROR: Failed to allocate workspace\n");
        return NULL;
    }
    
    // Initialize workspace
    workspace->id = g_desktop.workspace_count + 1;
    strncpy(workspace->name, name ? name : "Unnamed", sizeof(workspace->name) - 1);
    workspace->type = type;
    workspace->window_mode = g_desktop.default_window_mode;
    workspace->max_windows = 64; // Per-workspace window limit
    workspace->ai_layout_enabled = (g_desktop.ai_mode >= AI_MODE_STANDARD);
    workspace->security_level = g_desktop.security_level;
    workspace->screen_lock_enabled = false;
    workspace->network_isolation = false;
    
    // Allocate window array
    workspace->windows = calloc(workspace->max_windows, sizeof(lui_window_t*));
    if (!workspace->windows) {
        free(workspace);
        return NULL;
    }
    
    // Set workspace-specific properties based on type
    switch (type) {
        case WORKSPACE_TYPE_DEVELOPMENT:
            strcpy(workspace->description, "Optimized for software development");
            workspace->accent_color = LUI_COLOR_MISSION_GREEN;
            workspace->window_mode = WINDOW_MODE_TILED;
            break;
        case WORKSPACE_TYPE_DESIGN:
            strcpy(workspace->description, "Creative workspace for design work");
            workspace->accent_color = LUI_COLOR_INTEL_PURPLE;
            workspace->window_mode = WINDOW_MODE_FLOATING;
            break;
        case WORKSPACE_TYPE_COMMUNICATION:
            strcpy(workspace->description, "Communication and collaboration hub");
            workspace->accent_color = LUI_COLOR_SECURE_CYAN;
            workspace->window_mode = WINDOW_MODE_TABBED;
            break;
        case WORKSPACE_TYPE_SECURITY:
            strcpy(workspace->description, "High-security monitoring workspace");
            workspace->accent_color = LUI_COLOR_CRITICAL_RED;
            workspace->security_level = SECURITY_LEVEL_PROTECTED;
            workspace->network_isolation = true;
            break;
        default:
            strcpy(workspace->description, "General purpose workspace");
            workspace->accent_color = LUI_COLOR_TACTICAL_BLUE;
            break;
    }
    
    // Add to workspace list
    if (g_desktop.workspaces) {
        g_desktop.workspaces->prev = workspace;
    }
    workspace->next = g_desktop.workspaces;
    g_desktop.workspaces = workspace;
    g_desktop.workspace_count++;
    
    printf("[Desktop] Created workspace '%s' (ID: %u, Type: %d)\n", 
           workspace->name, workspace->id, workspace->type);
    
    return workspace;
}

void desktop_destroy_workspace(desktop_workspace_t* workspace) {
    if (!workspace) return;
    
    printf("[Desktop] Destroying workspace '%s' (ID: %u)\n", workspace->name, workspace->id);
    
    // Move all windows to another workspace if this is not the last one
    if (g_desktop.workspace_count > 1 && workspace->window_count > 0) {
        desktop_workspace_t* target_workspace = g_desktop.workspaces;
        if (target_workspace == workspace) {
            target_workspace = workspace->next;
        }
        
        if (target_workspace) {
            for (uint32_t i = 0; i < workspace->window_count; i++) {
                if (workspace->windows[i]) {
                    // Move window to target workspace
                    // TODO: Implement window migration between workspaces
                }
            }
        }
    }
    
    // Remove from workspace list
    if (workspace->prev) {
        workspace->prev->next = workspace->next;
    } else {
        g_desktop.workspaces = workspace->next;
    }
    if (workspace->next) {
        workspace->next->prev = workspace->prev;
    }
    
    // Update current workspace if needed
    if (g_desktop.current_workspace == workspace) {
        g_desktop.current_workspace = g_desktop.workspaces;
    }
    
    // Free resources
    if (workspace->windows) {
        free(workspace->windows);
    }
    
    free(workspace);
    g_desktop.workspace_count--;
}

void desktop_switch_workspace(desktop_workspace_t* workspace) {
    if (!workspace || workspace == g_desktop.current_workspace) {
        return;
    }
    
    printf("[Desktop] Switching to workspace '%s'\n", workspace->name);
    
    // Hide windows from current workspace
    if (g_desktop.current_workspace) {
        for (uint32_t i = 0; i < g_desktop.current_workspace->window_count; i++) {
            if (g_desktop.current_workspace->windows[i]) {
                lui_hide_window(g_desktop.current_workspace->windows[i]);
            }
        }
    }
    
    // Show windows from new workspace
    for (uint32_t i = 0; i < workspace->window_count; i++) {
        if (workspace->windows[i]) {
            lui_show_window(workspace->windows[i]);
        }
    }
    
    // Update current workspace
    desktop_workspace_t* prev_workspace = g_desktop.current_workspace;
    g_desktop.current_workspace = workspace;
    
    // Update desktop theme accent color
    g_desktop.desktop_root->border_color = workspace->accent_color;
    
    // Trigger workspace change animation
    if (g_desktop.animations_enabled) {
        // TODO: Implement smooth workspace transition animation
    }
    
    // Update AI insights
    if (prev_workspace) {
        prev_workspace->last_activity_time = desktop_get_timestamp_ms();
    }
    
    printf("[Desktop] Workspace switch completed\n");
}

desktop_workspace_t* desktop_get_current_workspace(void) {
    return g_desktop.current_workspace;
}

// ============================================================================
// APPLICATION MANAGEMENT
// ============================================================================

bool desktop_register_application(const desktop_application_t* app) {
    if (!app || !app->name[0]) {
        return false;
    }
    
    // Check if application already exists
    if (desktop_find_application(app->name)) {
        printf("[Desktop] Application '%s' already registered\n", app->name);
        return false;
    }
    
    // Allocate new application entry
    desktop_application_t* new_app = malloc(sizeof(desktop_application_t));
    if (!new_app) {
        printf("[Desktop] ERROR: Failed to allocate application entry\n");
        return false;
    }
    
    // Copy application data
    memcpy(new_app, app, sizeof(desktop_application_t));
    new_app->next = NULL;
    
    // Initialize usage statistics
    new_app->launch_count = 0;
    new_app->total_usage_time_ms = 0;
    new_app->last_launched_time = 0;
    new_app->user_rating = 0.5f; // Neutral rating
    
    // Add to application list
    new_app->next = g_desktop.applications;
    g_desktop.applications = new_app;
    g_desktop.application_count++;
    
    printf("[Desktop] Registered application '%s' (Category: %d)\n", 
           new_app->name, new_app->category);
    
    return true;
}

desktop_application_t* desktop_find_application(const char* name) {
    if (!name) return NULL;
    
    desktop_application_t* app = g_desktop.applications;
    while (app) {
        if (strcmp(app->name, name) == 0) {
            return app;
        }
        app = app->next;
    }
    
    return NULL;
}

bool desktop_launch_application(const char* name, const char* arguments) {
    desktop_application_t* app = desktop_find_application(name);
    if (!app) {
        printf("[Desktop] Application '%s' not found\n", name ? name : "NULL");
        return false;
    }
    
    // Check security level requirements
    if (app->security_level_required > g_desktop.security_level) {
        printf("[Desktop] Application '%s' requires security level %u (current: %u)\n",
               app->name, app->security_level_required, g_desktop.security_level);
        return false;
    }
    
    printf("[Desktop] Launching application '%s'\n", app->name);
    
    // TODO: Implement actual application launching
    // For now, simulate launch
    
    // Update usage statistics
    app->launch_count++;
    app->last_launched_time = desktop_get_timestamp_ms();
    
    // Update AI insights
    if (g_desktop.ai_insights.enabled) {
        // Track application usage patterns for AI optimization
        printf("[Desktop] AI: Recording application launch pattern for '%s'\n", app->name);
    }
    
    return true;
}

// ============================================================================
// UTILITY IMPLEMENTATION
// ============================================================================

const char* desktop_get_version_string(void) {
    static char version_str[64];
    snprintf(version_str, sizeof(version_str), "%d.%d.%d-%s", 
             DESKTOP_VERSION_MAJOR, DESKTOP_VERSION_MINOR, 
             DESKTOP_VERSION_PATCH, DESKTOP_CODENAME);
    return version_str;
}

void desktop_update_system_info(void) {
    // TODO: Integrate with actual LimitlessOS system monitoring APIs
    // For now, simulate system information
    
    static float cpu_trend = 0.0f;
    static float mem_trend = 0.0f;
    
    // Simulate some variation in system metrics
    cpu_trend += (rand() / (float)RAND_MAX - 0.5f) * 0.1f;
    cpu_trend = fmaxf(0.0f, fminf(1.0f, cpu_trend));
    
    mem_trend += (rand() / (float)RAND_MAX - 0.5f) * 0.05f; 
    mem_trend = fmaxf(0.0f, fminf(1.0f, mem_trend));
    
    g_desktop.system_info.cpu_usage_percent = 15.0f + cpu_trend * 30.0f;
    g_desktop.system_info.memory_usage_percent = 45.0f + mem_trend * 25.0f;
    g_desktop.system_info.disk_usage_percent = 67.3f;
    g_desktop.system_info.network_upload_kbps = 12.5f;
    g_desktop.system_info.network_download_kbps = 156.8f;
    g_desktop.system_info.gpu_usage_percent = 8.2f;
    g_desktop.system_info.battery_percent = 87;
    
    // Security status
    g_desktop.system_info.security_status = SECURITY_INDICATOR_SECURE;
    g_desktop.system_info.active_connections = 23;
    g_desktop.system_info.blocked_threats = 0;
    g_desktop.system_info.firewall_enabled = true;
    g_desktop.system_info.antivirus_enabled = true;
    
    // System health
    g_desktop.system_info.temperature_cpu = 42.0f + (rand() / (float)RAND_MAX) * 8.0f;
    g_desktop.system_info.temperature_gpu = 38.0f + (rand() / (float)RAND_MAX) * 6.0f;
    g_desktop.system_info.uptime_seconds = (uint32_t)(desktop_get_timestamp_ms() / 1000);
    g_desktop.system_info.active_processes = 156 + (rand() % 20);
    g_desktop.system_info.memory_total_mb = 16384; // 16GB
    g_desktop.system_info.memory_available_mb = 
        g_desktop.system_info.memory_total_mb * (1.0f - g_desktop.system_info.memory_usage_percent / 100.0f);
    
    g_desktop.system_info.last_updated = desktop_get_timestamp_ms();
}

desktop_system_info_t* desktop_get_system_info(void) {
    return &g_desktop.system_info;
}

void desktop_get_performance_stats(uint32_t* fps, uint32_t* frame_time_us, 
                                  uint32_t* window_count, uint32_t* memory_usage_mb) {
    if (fps) *fps = g_desktop.performance_stats.current_fps;
    if (frame_time_us) *frame_time_us = g_desktop.performance_stats.average_frame_time_us;
    if (window_count) *window_count = g_desktop.window_count;
    if (memory_usage_mb) *memory_usage_mb = (uint32_t)(g_desktop.system_info.memory_total_mb - 
                                                       g_desktop.system_info.memory_available_mb);
}