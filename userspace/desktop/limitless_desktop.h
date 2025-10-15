/**
 * LimitlessOS Desktop Environment - Military-Grade Desktop Shell
 * 
 * The command center of LimitlessOS. Precision-engineered for professional workflows
 * with enterprise-grade security, AI-enhanced productivity, and elegant minimalism.
 * 
 * Core Features:
 * - Adaptive window management with intelligent tiling
 * - Military-grade security indicators and controls  
 * - AI-powered workspace optimization
 * - Zero-distraction interface design
 * - Universal application launcher with predictive search
 * - Real-time system monitoring and control
 */

#ifndef LIMITLESS_DESKTOP_H
#define LIMITLESS_DESKTOP_H

#include "../ui/limitlessui.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// DESKTOP ARCHITECTURE CONSTANTS
// ============================================================================

#define DESKTOP_VERSION_MAJOR           1
#define DESKTOP_VERSION_MINOR           0  
#define DESKTOP_VERSION_PATCH           0
#define DESKTOP_CODENAME               "Tactical Command"

// Layout Constants (Golden Ratio Based)
#define TASKBAR_HEIGHT                  48    // 3 * 16px baseline
#define TITLEBAR_HEIGHT                 32    // 2 * 16px baseline  
#define SIDEBAR_WIDTH                   320   // Golden ratio of 1920/6
#define NOTIFICATION_WIDTH              400   // Optimal for readability
#define LAUNCHER_MAX_RESULTS            12    // Cognitive load limit
#define WORKSPACE_GRID_SIZE             8     // Snap grid for precision

// Timing Constants (Military Precision)
#define DESKTOP_ANIMATION_FAST          100   // Quick feedback
#define DESKTOP_ANIMATION_STANDARD      200   // Standard interactions
#define DESKTOP_ANIMATION_SMOOTH        300   // Complex transitions
#define DESKTOP_REFRESH_RATE           144    // Target 144Hz for fluid response

// Security & Privacy Levels
#define SECURITY_LEVEL_OPEN             0     // Standard desktop mode
#define SECURITY_LEVEL_PROTECTED        1     // Enhanced privacy
#define SECURITY_LEVEL_CLASSIFIED       2     // High security mode  
#define SECURITY_LEVEL_TOP_SECRET       3     // Maximum security

// ============================================================================
// CORE DATA STRUCTURES
// ============================================================================

// Desktop themes (extending UI themes with desktop-specific elements)
typedef enum {
    DESKTOP_THEME_TACTICAL_DARK = 0,      // Default military dark
    DESKTOP_THEME_TACTICAL_LIGHT,         // Light variant for bright environments
    DESKTOP_THEME_MISSION_BLUE,           // Blue accent theme
    DESKTOP_THEME_STEALTH_BLACK,          // Pure black for OLED displays
    DESKTOP_THEME_HIGH_CONTRAST,          // Accessibility theme
    DESKTOP_THEME_CUSTOM                  // User-defined custom theme
} desktop_theme_t;

// Window management modes
typedef enum {
    WINDOW_MODE_FLOATING = 0,             // Traditional floating windows
    WINDOW_MODE_TILED,                    // Automatic tiling
    WINDOW_MODE_TABBED,                   // Tabbed interface  
    WINDOW_MODE_STACKED,                  // Stacked window management
    WINDOW_MODE_ADAPTIVE                  // AI-powered adaptive layout
} window_management_mode_t;

// Workspace types for different workflows
typedef enum {
    WORKSPACE_TYPE_GENERAL = 0,           // General productivity
    WORKSPACE_TYPE_DEVELOPMENT,           // Software development
    WORKSPACE_TYPE_DESIGN,                // Creative design work
    WORKSPACE_TYPE_COMMUNICATION,         // Messaging and collaboration
    WORKSPACE_TYPE_MEDIA,                 // Media consumption/creation
    WORKSPACE_TYPE_ANALYSIS,              // Data analysis and research
    WORKSPACE_TYPE_SECURITY,              // Security and monitoring
    WORKSPACE_TYPE_CUSTOM                 // User-defined workflow
} workspace_type_t;

// Application categories for intelligent organization
typedef enum {
    APP_CATEGORY_SYSTEM = 0,              // System utilities
    APP_CATEGORY_PRODUCTIVITY,            // Office, documents, planning
    APP_CATEGORY_DEVELOPMENT,             // IDEs, editors, tools
    APP_CATEGORY_MEDIA,                   // Audio, video, graphics
    APP_CATEGORY_COMMUNICATION,           // Email, chat, video calls
    APP_CATEGORY_WEB,                     // Browsers, web apps
    APP_CATEGORY_GAMES,                   // Gaming applications
    APP_CATEGORY_EDUCATION,               // Learning and reference
    APP_CATEGORY_SECURITY,                // Security tools
    APP_CATEGORY_UNKNOWN                  // Uncategorized
} app_category_t;

// Security indicator states
typedef enum {
    SECURITY_INDICATOR_SECURE = 0,        // All systems secure
    SECURITY_INDICATOR_WARNING,           // Minor security concern
    SECURITY_INDICATOR_ALERT,             // Security attention needed
    SECURITY_INDICATOR_BREACH,            // Active security threat
    SECURITY_INDICATOR_LOCKDOWN          // System in lockdown mode
} security_indicator_t;

// AI assistance modes
typedef enum {
    AI_MODE_DISABLED = 0,                 // No AI assistance
    AI_MODE_MINIMAL,                      // Basic predictions only
    AI_MODE_STANDARD,                     // Standard AI features
    AI_MODE_ENHANCED,                     // Advanced AI integration
    AI_MODE_MAXIMUM                       // Full AI augmentation
} ai_assistance_mode_t;

// ============================================================================
// DESKTOP COMPONENTS
// ============================================================================

// Desktop workspace definition
typedef struct desktop_workspace {
    uint32_t id;
    char name[64];
    char description[256];
    workspace_type_t type;
    
    // Visual representation
    uint32_t icon_id;
    lui_color_t accent_color;
    
    // Window management
    window_management_mode_t window_mode;
    lui_window_t** windows;
    uint32_t window_count;
    uint32_t max_windows;
    
    // AI optimization
    bool ai_layout_enabled;
    float productivity_score;
    uint64_t focus_time_ms;
    uint64_t last_activity_time;
    
    // Security context
    uint8_t security_level;
    bool screen_lock_enabled;
    bool network_isolation;
    
    struct desktop_workspace* next;
    struct desktop_workspace* prev;
} desktop_workspace_t;

// Application entry for launcher and management
typedef struct desktop_application {
    char name[128];
    char description[256];
    char executable_path[512];
    char icon_path[512];
    char keywords[512];                   // Space-separated search keywords
    
    app_category_t category;
    uint8_t security_level_required;      // Minimum security level to run
    
    // Usage statistics for AI
    uint32_t launch_count;
    uint64_t total_usage_time_ms;
    uint64_t last_launched_time;
    float user_rating;                    // 0.0 - 1.0 user satisfaction
    
    // System integration
    bool system_integration;              // Can integrate with desktop
    bool autostart_enabled;
    bool background_allowed;
    
    struct desktop_application* next;
} desktop_application_t;

// Notification system
typedef struct desktop_notification {
    uint32_t id;
    char title[128];
    char message[512];
    char app_name[64];
    
    // Visual properties
    uint32_t icon_id;
    lui_color_t accent_color;
    
    // Behavior
    uint32_t priority;                    // 0 = low, 1 = normal, 2 = high, 3 = urgent
    uint32_t timeout_ms;                  // 0 = persistent
    bool requires_action;
    bool sound_enabled;
    
    // Timing
    uint64_t created_time;
    uint64_t display_time;
    uint64_t expire_time;
    
    // Actions
    struct {
        char label[32];
        char action_id[64];
    } actions[4];
    uint8_t action_count;
    
    struct desktop_notification* next;
    struct desktop_notification* prev;
} desktop_notification_t;

// System monitoring data
typedef struct desktop_system_info {
    // Performance metrics
    float cpu_usage_percent;
    float memory_usage_percent;  
    float disk_usage_percent;
    float network_upload_kbps;
    float network_download_kbps;
    float gpu_usage_percent;
    uint32_t battery_percent;
    
    // Security status
    security_indicator_t security_status;
    uint32_t active_connections;
    uint32_t blocked_threats;
    bool firewall_enabled;
    bool antivirus_enabled;
    
    // System health
    float temperature_cpu;
    float temperature_gpu;
    uint32_t uptime_seconds;
    uint32_t active_processes;
    uint64_t memory_total_mb;
    uint64_t memory_available_mb;
    
    // Update timestamp
    uint64_t last_updated;
} desktop_system_info_t;

// Main desktop state
typedef struct desktop_environment {
    bool initialized;
    bool running;
    
    // Core UI components
    lui_window_t* desktop_window;         // Root desktop window
    lui_widget_t* desktop_root;           // Root widget container
    lui_widget_t* taskbar;               // Main taskbar
    lui_widget_t* sidebar;               // Optional sidebar
    lui_widget_t* launcher;              // Application launcher
    lui_widget_t* notification_area;     // Notification display area
    
    // Window management
    lui_window_t** managed_windows;
    uint32_t window_count;
    uint32_t max_windows;
    lui_window_t* active_window;
    lui_window_t* focused_window;
    
    // Workspace system
    desktop_workspace_t* workspaces;
    desktop_workspace_t* current_workspace;
    uint32_t workspace_count;
    
    // Application management
    desktop_application_t* applications;
    uint32_t application_count;
    
    // Notification system
    desktop_notification_t* notifications;
    uint32_t notification_count;
    uint32_t max_notifications;
    
    // Theme and appearance
    desktop_theme_t current_theme;
    float ui_scale;
    bool dark_mode;
    bool animations_enabled;
    bool transparency_enabled;
    
    // Behavior settings
    window_management_mode_t default_window_mode;
    ai_assistance_mode_t ai_mode;
    uint8_t security_level;
    bool auto_arrange_enabled;
    bool smart_notifications_enabled;
    
    // System monitoring
    desktop_system_info_t system_info;
    uint32_t system_refresh_interval_ms;
    
    // AI and productivity
    struct {
        bool enabled;
        float productivity_score;
        uint32_t focus_sessions_today;
        uint64_t productive_time_today_ms;
        char current_activity[128];
        float stress_level;               // 0.0 = calm, 1.0 = high stress
    } ai_insights;
    
    // Performance tracking
    struct {
        uint64_t frames_rendered;
        uint32_t current_fps;
        uint32_t average_frame_time_us;
        uint32_t window_switches_count;
        uint32_t launcher_activations;
    } performance_stats;
    
} desktop_environment_t;

// ============================================================================
// DESKTOP API
// ============================================================================

// Core desktop lifecycle
bool desktop_init(void);
void desktop_shutdown(void);
void desktop_run(void);
void desktop_stop(void);
desktop_environment_t* desktop_get_instance(void);

// Theme and appearance
void desktop_set_theme(desktop_theme_t theme);
desktop_theme_t desktop_get_theme(void);
void desktop_set_dark_mode(bool enabled);
void desktop_set_ui_scale(float scale);
void desktop_set_animations_enabled(bool enabled);
void desktop_set_transparency_enabled(bool enabled);

// Workspace management  
desktop_workspace_t* desktop_create_workspace(const char* name, workspace_type_t type);
void desktop_destroy_workspace(desktop_workspace_t* workspace);
void desktop_switch_workspace(desktop_workspace_t* workspace);
desktop_workspace_t* desktop_get_current_workspace(void);
desktop_workspace_t* desktop_get_workspace_by_name(const char* name);
void desktop_set_workspace_layout(desktop_workspace_t* workspace, window_management_mode_t mode);

// Window management
void desktop_manage_window(lui_window_t* window);
void desktop_unmanage_window(lui_window_t* window);
void desktop_focus_window(lui_window_t* window);
void desktop_minimize_window(lui_window_t* window);
void desktop_maximize_window(lui_window_t* window);
void desktop_close_window(lui_window_t* window);
void desktop_tile_windows(void);
void desktop_cascade_windows(void);

// Application management
bool desktop_register_application(const desktop_application_t* app);
bool desktop_unregister_application(const char* name);
desktop_application_t* desktop_find_application(const char* name);
desktop_application_t** desktop_search_applications(const char* query, uint32_t* result_count);
bool desktop_launch_application(const char* name, const char* arguments);
void desktop_update_application_stats(const char* name, uint64_t usage_time_ms);

// Notification system
uint32_t desktop_show_notification(const char* title, const char* message, 
                                  const char* app_name, uint32_t priority);
void desktop_hide_notification(uint32_t notification_id);
void desktop_clear_notifications(const char* app_name);
void desktop_set_notification_sound_enabled(bool enabled);

// System monitoring and AI
void desktop_update_system_info(void);
desktop_system_info_t* desktop_get_system_info(void);
void desktop_set_ai_mode(ai_assistance_mode_t mode);
ai_assistance_mode_t desktop_get_ai_mode(void);
void desktop_update_ai_insights(void);
float desktop_get_productivity_score(void);

// Launcher and search
void desktop_show_launcher(void);
void desktop_hide_launcher(void);
bool desktop_is_launcher_visible(void);
void desktop_launcher_set_query(const char* query);
desktop_application_t** desktop_launcher_get_results(uint32_t* count);

// Security and privacy
void desktop_set_security_level(uint8_t level);
uint8_t desktop_get_security_level(void);
security_indicator_t desktop_get_security_status(void);
void desktop_enable_screen_lock(bool enabled);
void desktop_trigger_security_alert(const char* message);

// Configuration and persistence
bool desktop_save_configuration(void);
bool desktop_load_configuration(void);
void desktop_reset_to_defaults(void);
void desktop_set_config_string(const char* key, const char* value);
void desktop_set_config_bool(const char* key, bool value);
void desktop_set_config_float(const char* key, float value);
const char* desktop_get_config_string(const char* key, const char* default_value);
bool desktop_get_config_bool(const char* key, bool default_value);
float desktop_get_config_float(const char* key, float default_value);

// Event callbacks (for external integration)
typedef void (*desktop_window_event_cb)(lui_window_t* window, const char* event, void* user_data);
typedef void (*desktop_workspace_event_cb)(desktop_workspace_t* workspace, const char* event, void* user_data);
typedef void (*desktop_app_event_cb)(const char* app_name, const char* event, void* user_data);

void desktop_set_window_event_callback(desktop_window_event_cb callback, void* user_data);
void desktop_set_workspace_event_callback(desktop_workspace_event_cb callback, void* user_data);
void desktop_set_app_event_callback(desktop_app_event_cb callback, void* user_data);

// Performance and debugging
void desktop_get_performance_stats(uint32_t* fps, uint32_t* frame_time_us, 
                                  uint32_t* window_count, uint32_t* memory_usage_mb);
void desktop_enable_debug_overlay(bool enabled);
void desktop_print_workspace_info(void);
void desktop_print_window_hierarchy(void);

// Utility functions
const char* desktop_get_version_string(void);
uint32_t desktop_get_uptime_seconds(void);
void desktop_schedule_task(const char* task_name, uint32_t delay_ms, 
                          void (*callback)(void* user_data), void* user_data);

#endif // LIMITLESS_DESKTOP_H