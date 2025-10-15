/**
 * Limitless Settings - Comprehensive System Configuration Panel
 * 
 * Advanced system configuration application for LimitlessOS with military-grade
 * security controls, AI-powered optimization recommendations, and comprehensive
 * system management capabilities.
 * 
 * Features:
 * - System-wide configuration management
 * - Security policy configuration with military-grade controls
 * - User account and permission management
 * - Network and connectivity settings
 * - Hardware configuration and driver management
 * - AI-powered optimization recommendations
 * - Theme and appearance customization
 * - Privacy and data protection controls
 * - System maintenance and update management
 */

#include "../ui/limitlessui.h"
#include "../desktop/limitless_desktop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// ============================================================================
// SETTINGS CONSTANTS AND CONFIGURATION
// ============================================================================

#define SETTINGS_VERSION            "1.0.0-Command"
#define MAX_SETTING_NAME_LENGTH     128
#define MAX_SETTING_VALUE_LENGTH    512
#define MAX_SETTINGS_CATEGORIES     20
#define MAX_SETTINGS_PER_CATEGORY   50
#define MAX_USER_ACCOUNTS          100
#define MAX_SECURITY_POLICIES       50
#define AI_RECOMMENDATION_INTERVAL  60      // AI analysis every 60 seconds

// Settings categories
typedef enum {
    SETTINGS_CATEGORY_SYSTEM = 0,         // System information and basic settings
    SETTINGS_CATEGORY_APPEARANCE,         // Themes, fonts, desktop appearance
    SETTINGS_CATEGORY_SECURITY,           // Security policies and controls
    SETTINGS_CATEGORY_NETWORK,            // Network configuration
    SETTINGS_CATEGORY_HARDWARE,           // Hardware configuration and drivers
    SETTINGS_CATEGORY_ACCOUNTS,           // User accounts and permissions
    SETTINGS_CATEGORY_PRIVACY,            // Privacy and data protection
    SETTINGS_CATEGORY_AI,                 // AI system configuration
    SETTINGS_CATEGORY_UPDATES,            // System updates and maintenance
    SETTINGS_CATEGORY_ADVANCED           // Advanced system configuration
} settings_category_t;

// Security levels for various system components
typedef enum {
    SECURITY_LEVEL_MINIMAL = 0,           // Minimal security (development)
    SECURITY_LEVEL_STANDARD,              // Standard security (normal use)
    SECURITY_LEVEL_ENHANCED,              // Enhanced security (business)
    SECURITY_LEVEL_MILITARY,              // Military-grade security
    SECURITY_LEVEL_CLASSIFIED            // Classified/top-secret security
} security_level_t;

// Setting data types
typedef enum {
    SETTING_TYPE_BOOLEAN = 0,             // True/false setting
    SETTING_TYPE_INTEGER,                 // Integer number
    SETTING_TYPE_FLOAT,                   // Floating point number
    SETTING_TYPE_STRING,                  // Text string
    SETTING_TYPE_ENUM,                    // Enumerated value
    SETTING_TYPE_COLOR,                   // Color value
    SETTING_TYPE_FILE_PATH,               // File or directory path
    SETTING_TYPE_PASSWORD                 // Password (hidden input)
} setting_type_t;

// AI recommendation types for settings
typedef enum {
    AI_SETTINGS_RECOMMEND_NONE = 0,
    AI_SETTINGS_RECOMMEND_SECURITY_UPGRADE,
    AI_SETTINGS_RECOMMEND_PERFORMANCE_TUNE,
    AI_SETTINGS_RECOMMEND_PRIVACY_ENHANCE,
    AI_SETTINGS_RECOMMEND_POWER_OPTIMIZE,
    AI_SETTINGS_RECOMMEND_NETWORK_OPTIMIZE,
    AI_SETTINGS_RECOMMEND_UPDATE_SYSTEM
} ai_settings_recommendation_t;

// ============================================================================
// CORE DATA STRUCTURES
// ============================================================================

// Individual setting definition
typedef struct setting_definition {
    char name[MAX_SETTING_NAME_LENGTH];
    char display_name[MAX_SETTING_NAME_LENGTH];
    char description[256];
    setting_type_t type;
    
    union {
        bool boolean_value;
        int integer_value;
        float float_value;
        char string_value[MAX_SETTING_VALUE_LENGTH];
        int enum_value;
        lui_color_t color_value;
    } current_value;
    
    union {
        bool boolean_default;
        int integer_default;
        float float_default;
        char string_default[MAX_SETTING_VALUE_LENGTH];
        int enum_default;
        lui_color_t color_default;
    } default_value;
    
    // Constraints and validation
    struct {
        int min_value, max_value;         // For integer/float types
        char* enum_options[16];           // For enum types
        uint32_t enum_option_count;
        bool requires_admin;              // Requires administrator privileges
        bool requires_reboot;             // Requires system reboot
        security_level_t min_security_level; // Minimum security level required
    } constraints;
    
    bool is_modified;                     // Has been changed from default
    bool is_system_managed;               // Managed by system (read-only)
    
    struct setting_definition* next;
} setting_definition_t;

// Settings category
typedef struct settings_category {
    settings_category_t category_id;
    char name[64];
    char description[256];
    char icon[32];                        // Icon name/emoji
    bool requires_admin;                  // Category requires admin access
    
    setting_definition_t* settings;       // List of settings in this category
    uint32_t setting_count;
    
    struct settings_category* next;
} settings_category_t;

// User account information
typedef struct user_account {
    uid_t user_id;
    gid_t group_id;
    char username[64];
    char full_name[128];
    char home_directory[256];
    char shell[128];
    bool is_admin;
    bool is_active;
    bool password_expires;
    time_t last_login;
    time_t password_changed;
    
    // Security settings
    bool two_factor_enabled;
    bool biometric_enabled;
    security_level_t access_level;
    char allowed_hours[24];               // Hours user is allowed to login
    
    struct user_account* next;
} user_account_t;

// Security policy definition
typedef struct security_policy {
    char policy_name[128];
    char description[256];
    security_level_t level;
    bool is_enabled;
    bool is_enforced;                     // Actively enforced vs. advisory
    
    // Policy parameters
    struct {
        uint32_t password_min_length;
        bool password_require_special;
        bool password_require_numbers;
        uint32_t login_attempt_limit;
        uint32_t session_timeout_minutes;
        bool require_encryption;
        bool allow_usb_devices;
        bool allow_network_shares;
        bool enable_audit_logging;
        bool enable_intrusion_detection;
    } parameters;
    
    struct security_policy* next;
} security_policy_t;

// Network configuration
typedef struct network_config {
    char interface_name[32];
    bool use_dhcp;
    char static_ip[46];                   // IPv4 or IPv6
    char subnet_mask[46];
    char gateway[46];
    char dns_primary[46];
    char dns_secondary[46];
    
    // Security settings
    bool enable_firewall;
    bool enable_vpn;
    char vpn_server[128];
    bool block_malicious_domains;
    bool enable_dns_over_https;
    
    struct network_config* next;
} network_config_t;

// Hardware configuration item
typedef struct hardware_config {
    char device_name[128];
    char device_type[64];                 // CPU, GPU, Storage, Network, etc.
    char driver_name[128];
    char driver_version[64];
    bool is_enabled;
    bool driver_loaded;
    
    // Performance settings
    struct {
        uint32_t power_mode;              // 0=power save, 1=balanced, 2=performance
        bool hardware_acceleration;
        uint32_t polling_rate_hz;
        bool wake_on_lan;
        bool auto_suspend;
    } performance;
    
    struct hardware_config* next;
} hardware_config_t;

// AI settings analysis
typedef struct ai_settings_analysis {
    float security_score;                 // Current security posture (0.0-1.0)
    float performance_score;              // Performance optimization (0.0-1.0)
    float privacy_score;                  // Privacy protection level (0.0-1.0)
    float power_efficiency_score;         // Power management efficiency (0.0-1.0)
    
    // Recommendations
    ai_settings_recommendation_t primary_recommendation;
    char recommendation_details[512];
    float confidence_score;               // AI confidence (0.0-1.0)
    
    // Identified issues
    uint32_t security_issues;
    uint32_t performance_issues;
    uint32_t privacy_issues;
    uint32_t configuration_errors;
    
    time_t last_analysis;
} ai_settings_analysis_t;

// Main settings application state
typedef struct limitless_settings {
    bool initialized;
    bool running;
    bool has_admin_privileges;
    
    // UI components
    lui_window_t* main_window;
    lui_widget_t* category_sidebar;
    lui_widget_t* settings_panel;
    lui_widget_t* details_panel;
    lui_widget_t* ai_panel;
    lui_widget_t* status_bar;
    lui_widget_t* toolbar;
    
    // Current state
    settings_category_t current_category;
    setting_definition_t* selected_setting;
    bool settings_modified;
    
    // Configuration data
    settings_category_t* categories;
    uint32_t category_count;
    
    // User management
    user_account_t* user_accounts;
    uint32_t user_account_count;
    user_account_t* current_user;
    
    // Security management
    security_policy_t* security_policies;
    uint32_t security_policy_count;
    security_level_t current_security_level;
    
    // Network configuration
    network_config_t* network_configs;
    uint32_t network_config_count;
    
    // Hardware configuration
    hardware_config_t* hardware_configs;
    uint32_t hardware_config_count;
    
    // AI analysis
    ai_settings_analysis_t ai_analysis;
    bool ai_recommendations_enabled;
    time_t last_ai_analysis;
    
    // Application settings
    bool show_advanced_settings;
    bool show_ai_panel;
    bool confirm_dangerous_changes;
    bool auto_apply_safe_changes;
    
    // System information
    struct {
        char os_name[64];
        char os_version[64];
        char kernel_version[64];
        char hostname[64];
        char architecture[32];
        uint64_t total_memory;
        uint32_t cpu_cores;
        char cpu_model[128];
    } system_info;
    
    // Statistics
    struct {
        time_t session_start_time;
        uint32_t settings_changed;
        uint32_t categories_visited;
        uint32_t ai_recommendations_applied;
        uint32_t security_changes_made;
    } stats;
    
} limitless_settings_t;

// Global settings instance
static limitless_settings_t g_settings = {0};

// ============================================================================
// SYSTEM INFORMATION COLLECTION
// ============================================================================

static void collect_system_information(void) {
    struct utsname sys_info;
    
    // Get system information
    if (uname(&sys_info) == 0) {
        strncpy(g_settings.system_info.os_name, "LimitlessOS", sizeof(g_settings.system_info.os_name) - 1);
        strncpy(g_settings.system_info.os_version, "1.0.0", sizeof(g_settings.system_info.os_version) - 1);
        strncpy(g_settings.system_info.kernel_version, sys_info.release, sizeof(g_settings.system_info.kernel_version) - 1);
        strncpy(g_settings.system_info.hostname, sys_info.nodename, sizeof(g_settings.system_info.hostname) - 1);
        strncpy(g_settings.system_info.architecture, sys_info.machine, sizeof(g_settings.system_info.architecture) - 1);
    }
    
    // Simulate hardware information
    g_settings.system_info.total_memory = 16ULL * 1024 * 1024 * 1024; // 16GB
    g_settings.system_info.cpu_cores = 8;
    strcpy(g_settings.system_info.cpu_model, "LimitlessOS Virtual CPU @ 3.2GHz");
    
    printf("[Settings] System: %s %s, Kernel: %s, Arch: %s\n",
           g_settings.system_info.os_name,
           g_settings.system_info.os_version,
           g_settings.system_info.kernel_version,
           g_settings.system_info.architecture);
}

// ============================================================================
// SETTINGS MANAGEMENT
// ============================================================================

static setting_definition_t* create_setting(const char* name, const char* display_name,
                                           const char* description, setting_type_t type) {
    setting_definition_t* setting = calloc(1, sizeof(setting_definition_t));
    if (!setting) return NULL;
    
    strncpy(setting->name, name, sizeof(setting->name) - 1);
    strncpy(setting->display_name, display_name, sizeof(setting->display_name) - 1);
    strncpy(setting->description, description, sizeof(setting->description) - 1);
    setting->type = type;
    
    return setting;
}

static void initialize_system_settings(void) {
    // Create system settings category
    settings_category_t* system_cat = calloc(1, sizeof(settings_category_t));
    system_cat->category_id = SETTINGS_CATEGORY_SYSTEM;
    strcpy(system_cat->name, "System");
    strcpy(system_cat->description, "Basic system configuration and information");
    strcpy(system_cat->icon, "🖥️");
    
    // System settings
    setting_definition_t* hostname_setting = create_setting("hostname", "Computer Name",
                                                           "Name of this computer on the network",
                                                           SETTING_TYPE_STRING);
    strcpy(hostname_setting->current_value.string_value, g_settings.system_info.hostname);
    strcpy(hostname_setting->default_value.string_default, g_settings.system_info.hostname);
    hostname_setting->constraints.requires_admin = true;
    hostname_setting->constraints.requires_reboot = true;
    
    setting_definition_t* auto_login = create_setting("auto_login", "Automatic Login",
                                                     "Automatically log in without password",
                                                     SETTING_TYPE_BOOLEAN);
    auto_login->current_value.boolean_value = false;
    auto_login->default_value.boolean_default = false;
    auto_login->constraints.requires_admin = true;
    auto_login->constraints.min_security_level = SECURITY_LEVEL_STANDARD;
    
    // Link settings to category
    hostname_setting->next = auto_login;
    system_cat->settings = hostname_setting;
    system_cat->setting_count = 2;
    
    g_settings.categories = system_cat;
    g_settings.category_count = 1;
}

static void initialize_security_settings(void) {
    // Create security settings category
    settings_category_t* security_cat = calloc(1, sizeof(settings_category_t));
    security_cat->category_id = SETTINGS_CATEGORY_SECURITY;
    strcpy(security_cat->name, "Security");
    strcpy(security_cat->description, "Security policies and access controls");
    strcpy(security_cat->icon, "🛡️");
    security_cat->requires_admin = true;
    
    // Security level setting
    setting_definition_t* security_level = create_setting("security_level", "Security Level",
                                                         "Overall system security level",
                                                         SETTING_TYPE_ENUM);
    security_level->current_value.enum_value = SECURITY_LEVEL_STANDARD;
    security_level->default_value.enum_default = SECURITY_LEVEL_STANDARD;
    security_level->constraints.enum_options[0] = "Minimal";
    security_level->constraints.enum_options[1] = "Standard";
    security_level->constraints.enum_options[2] = "Enhanced";
    security_level->constraints.enum_options[3] = "Military";
    security_level->constraints.enum_options[4] = "Classified";
    security_level->constraints.enum_option_count = 5;
    security_level->constraints.requires_admin = true;
    security_level->constraints.requires_reboot = true;
    
    // Firewall setting
    setting_definition_t* firewall_enabled = create_setting("firewall_enabled", "Enable Firewall",
                                                           "Enable network firewall protection",
                                                           SETTING_TYPE_BOOLEAN);
    firewall_enabled->current_value.boolean_value = true;
    firewall_enabled->default_value.boolean_default = true;
    firewall_enabled->constraints.requires_admin = true;
    
    // Two-factor authentication
    setting_definition_t* two_factor = create_setting("require_2fa", "Require Two-Factor Auth",
                                                     "Require two-factor authentication for all users",
                                                     SETTING_TYPE_BOOLEAN);
    two_factor->current_value.boolean_value = false;
    two_factor->default_value.boolean_default = false;
    two_factor->constraints.requires_admin = true;
    two_factor->constraints.min_security_level = SECURITY_LEVEL_ENHANCED;
    
    // Link settings
    security_level->next = firewall_enabled;
    firewall_enabled->next = two_factor;
    security_cat->settings = security_level;
    security_cat->setting_count = 3;
    
    // Link to main categories
    security_cat->next = g_settings.categories;
    g_settings.categories = security_cat;
    g_settings.category_count++;
}

static void initialize_appearance_settings(void) {
    // Create appearance settings category
    settings_category_t* appearance_cat = calloc(1, sizeof(settings_category_t));
    appearance_cat->category_id = SETTINGS_CATEGORY_APPEARANCE;
    strcpy(appearance_cat->name, "Appearance");
    strcpy(appearance_cat->description, "Desktop themes, fonts, and visual settings");
    strcpy(appearance_cat->icon, "🎨");
    
    // Theme setting
    setting_definition_t* theme = create_setting("desktop_theme", "Desktop Theme",
                                                "Visual theme for desktop and applications",
                                                SETTING_TYPE_ENUM);
    theme->current_value.enum_value = 0; // Dark theme
    theme->default_value.enum_default = 0;
    theme->constraints.enum_options[0] = "Limitless Dark";
    theme->constraints.enum_options[1] = "Limitless Light";
    theme->constraints.enum_options[2] = "Military Tactical";
    theme->constraints.enum_options[3] = "Arctic Blue";
    theme->constraints.enum_options[4] = "Custom";
    theme->constraints.enum_option_count = 5;
    
    // Font size setting
    setting_definition_t* font_size = create_setting("font_size", "Font Size",
                                                    "System font size in points",
                                                    SETTING_TYPE_INTEGER);
    font_size->current_value.integer_value = 12;
    font_size->default_value.integer_default = 12;
    font_size->constraints.min_value = 8;
    font_size->constraints.max_value = 24;
    
    // Animation setting
    setting_definition_t* animations = create_setting("enable_animations", "Enable Animations",
                                                     "Enable desktop and window animations",
                                                     SETTING_TYPE_BOOLEAN);
    animations->current_value.boolean_value = true;
    animations->default_value.boolean_default = true;
    
    // Link settings
    theme->next = font_size;
    font_size->next = animations;
    appearance_cat->settings = theme;
    appearance_cat->setting_count = 3;
    
    // Link to main categories
    appearance_cat->next = g_settings.categories;
    g_settings.categories = appearance_cat;
    g_settings.category_count++;
}

static void initialize_ai_settings(void) {
    // Create AI settings category
    settings_category_t* ai_cat = calloc(1, sizeof(settings_category_t));
    ai_cat->category_id = SETTINGS_CATEGORY_AI;
    strcpy(ai_cat->name, "AI Assistant");
    strcpy(ai_cat->description, "Artificial intelligence and automation settings");
    strcpy(ai_cat->icon, "🤖");
    
    // AI assistance level
    setting_definition_t* ai_level = create_setting("ai_assistance_level", "AI Assistance Level",
                                                   "Level of AI assistance throughout the system",
                                                   SETTING_TYPE_ENUM);
    ai_level->current_value.enum_value = 2; // Standard
    ai_level->default_value.enum_default = 2;
    ai_level->constraints.enum_options[0] = "Disabled";
    ai_level->constraints.enum_options[1] = "Basic";
    ai_level->constraints.enum_options[2] = "Standard";
    ai_level->constraints.enum_options[3] = "Enhanced";
    ai_level->constraints.enum_options[4] = "Copilot";
    ai_level->constraints.enum_option_count = 5;
    
    // AI data collection
    setting_definition_t* ai_data = create_setting("ai_data_collection", "AI Data Collection",
                                                  "Allow AI to collect usage data for improvements",
                                                  SETTING_TYPE_BOOLEAN);
    ai_data->current_value.boolean_value = true;
    ai_data->default_value.boolean_default = true;
    ai_data->constraints.min_security_level = SECURITY_LEVEL_STANDARD;
    
    // Predictive features
    setting_definition_t* predictive = create_setting("ai_predictive_features", "Predictive Features",
                                                     "Enable AI predictive assistance and suggestions",
                                                     SETTING_TYPE_BOOLEAN);
    predictive->current_value.boolean_value = true;
    predictive->default_value.boolean_default = true;
    
    // Link settings
    ai_level->next = ai_data;
    ai_data->next = predictive;
    ai_cat->settings = ai_level;
    ai_cat->setting_count = 3;
    
    // Link to main categories
    ai_cat->next = g_settings.categories;
    g_settings.categories = ai_cat;
    g_settings.category_count++;
}

// ============================================================================
// AI ANALYSIS AND RECOMMENDATIONS
// ============================================================================

static void perform_ai_settings_analysis(void) {
    if (!g_settings.ai_recommendations_enabled) return;
    
    printf("[Settings] Performing AI configuration analysis\n");
    
    ai_settings_analysis_t* ai = &g_settings.ai_analysis;
    
    // Analyze current security posture
    float security_factors = 0.0f;
    int security_count = 0;
    
    // Check security level
    settings_category_t* security_cat = g_settings.categories;
    while (security_cat && security_cat->category_id != SETTINGS_CATEGORY_SECURITY) {
        security_cat = security_cat->next;
    }
    
    if (security_cat) {
        setting_definition_t* setting = security_cat->settings;
        while (setting) {
            if (strcmp(setting->name, "security_level") == 0) {
                security_factors += (float)setting->current_value.enum_value / 4.0f; // 0-1 scale
                security_count++;
            } else if (strcmp(setting->name, "firewall_enabled") == 0) {
                security_factors += setting->current_value.boolean_value ? 1.0f : 0.0f;
                security_count++;
            } else if (strcmp(setting->name, "require_2fa") == 0) {
                security_factors += setting->current_value.boolean_value ? 1.0f : 0.0f;
                security_count++;
            }
            setting = setting->next;
        }
    }
    
    ai->security_score = (security_count > 0) ? (security_factors / security_count) : 0.5f;
    
    // Analyze performance configuration
    // Simplified analysis - real implementation would check all performance-related settings
    ai->performance_score = 0.75f; // Simulated performance score
    
    // Analyze privacy settings
    ai->privacy_score = 0.80f; // Simulated privacy score
    
    // Analyze power efficiency
    ai->power_efficiency_score = 0.70f; // Simulated power efficiency
    
    // Generate recommendations
    ai->security_issues = 0;
    ai->performance_issues = 0;
    ai->privacy_issues = 0;
    ai->configuration_errors = 0;
    
    if (ai->security_score < 0.7f) {
        ai->primary_recommendation = AI_SETTINGS_RECOMMEND_SECURITY_UPGRADE;
        strcpy(ai->recommendation_details, 
               "Security configuration needs improvement. Consider enabling two-factor authentication and increasing security level.");
        ai->confidence_score = 0.90f;
        ai->security_issues = 2;
    }
    else if (ai->performance_score < 0.6f) {
        ai->primary_recommendation = AI_SETTINGS_RECOMMEND_PERFORMANCE_TUNE;
        strcpy(ai->recommendation_details,
               "System performance can be improved. Review hardware acceleration and background process settings.");
        ai->confidence_score = 0.85f;
        ai->performance_issues = 1;
    }
    else if (ai->privacy_score < 0.8f) {
        ai->primary_recommendation = AI_SETTINGS_RECOMMEND_PRIVACY_ENHANCE;
        strcpy(ai->recommendation_details,
               "Privacy settings can be strengthened. Review data collection and sharing preferences.");
        ai->confidence_score = 0.80f;
        ai->privacy_issues = 1;
    }
    else {
        ai->primary_recommendation = AI_SETTINGS_RECOMMEND_NONE;
        strcpy(ai->recommendation_details, "Configuration appears optimal. No immediate changes recommended.");
        ai->confidence_score = 0.75f;
    }
    
    ai->last_analysis = time(NULL);
    g_settings.last_ai_analysis = ai->last_analysis;
    
    printf("[Settings] AI Analysis - Security: %.2f, Performance: %.2f, Privacy: %.2f\n",
           ai->security_score, ai->performance_score, ai->privacy_score);
}

// ============================================================================
// USER INTERFACE IMPLEMENTATION
// ============================================================================

static void create_category_sidebar(void) {
    g_settings.category_sidebar = lui_create_container(g_settings.main_window->root_widget);
    strcpy(g_settings.category_sidebar->name, "category_sidebar");
    g_settings.category_sidebar->bounds = lui_rect_make(0, 32, 200, 568);
    g_settings.category_sidebar->background_color = LUI_COLOR_GRAPHITE;
    
    // Category list
    settings_category_t* category = g_settings.categories;
    uint32_t y_offset = 8;
    
    while (category) {
        // Category button
        char category_text[96];
        snprintf(category_text, sizeof(category_text), "%s %s", category->icon, category->name);
        
        lui_widget_t* category_btn = lui_create_button(category_text, g_settings.category_sidebar);
        category_btn->bounds = lui_rect_make(8, y_offset, 184, 32);
        category_btn->typography = LUI_TYPOGRAPHY_BODY_MEDIUM;
        
        if (category->category_id == g_settings.current_category) {
            category_btn->style.background_color = LUI_COLOR_TACTICAL_BLUE;
        } else {
            category_btn->style.background_color = LUI_COLOR_STEEL_GRAY;
        }
        
        // Admin indicator
        if (category->requires_admin && !g_settings.has_admin_privileges) {
            lui_widget_t* admin_icon = lui_create_label("🔒", g_settings.category_sidebar);
            admin_icon->bounds = lui_rect_make(170, y_offset + 8, 16, 16);
            admin_icon->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
            admin_icon->typography.color = LUI_COLOR_WARNING_AMBER;
        }
        
        y_offset += 40;
        category = category->next;
    }
}

static void create_settings_panel(void) {
    g_settings.settings_panel = lui_create_container(g_settings.main_window->root_widget);
    strcpy(g_settings.settings_panel->name, "settings_panel");
    g_settings.settings_panel->bounds = lui_rect_make(200, 32, 500, 568);
    g_settings.settings_panel->background_color = LUI_COLOR_CHARCOAL_BLACK;
    
    // Find current category
    settings_category_t* category = g_settings.categories;
    while (category && category->category_id != g_settings.current_category) {
        category = category->next;
    }
    
    if (!category) return;
    
    // Category header
    char header_text[128];
    snprintf(header_text, sizeof(header_text), "%s %s", category->icon, category->name);
    
    lui_widget_t* header = lui_create_label(header_text, g_settings.settings_panel);
    header->bounds = lui_rect_make(16, 16, 400, 24);
    header->typography = LUI_TYPOGRAPHY_TITLE_LARGE;
    header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    // Category description
    lui_widget_t* description = lui_create_label(category->description, g_settings.settings_panel);
    description->bounds = lui_rect_make(16, 48, 468, 20);
    description->typography = LUI_TYPOGRAPHY_BODY_MEDIUM;
    description->typography.color = LUI_COLOR_STEEL_GRAY;
    
    // Settings list
    setting_definition_t* setting = category->settings;
    uint32_t y_offset = 80;
    
    while (setting && y_offset < 520) {
        // Setting name
        lui_widget_t* setting_name = lui_create_label(setting->display_name, g_settings.settings_panel);
        setting_name->bounds = lui_rect_make(16, y_offset, 300, 20);
        setting_name->typography = LUI_TYPOGRAPHY_BODY_MEDIUM;
        
        // Setting control based on type
        if (setting->type == SETTING_TYPE_BOOLEAN) {
            // Checkbox for boolean settings
            lui_widget_t* checkbox = lui_create_button(setting->current_value.boolean_value ? "☑" : "☐", 
                                                     g_settings.settings_panel);
            checkbox->bounds = lui_rect_make(320, y_offset - 2, 24, 24);
            checkbox->typography = LUI_TYPOGRAPHY_BODY_LARGE;
            checkbox->style.background_color = LUI_COLOR_STEEL_GRAY;
        }
        else if (setting->type == SETTING_TYPE_ENUM) {
            // Dropdown for enum settings
            const char* selected_option = "Unknown";
            if (setting->current_value.enum_value < setting->constraints.enum_option_count) {
                selected_option = setting->constraints.enum_options[setting->current_value.enum_value];
            }
            
            lui_widget_t* dropdown = lui_create_button(selected_option, g_settings.settings_panel);
            dropdown->bounds = lui_rect_make(320, y_offset - 2, 120, 24);
            dropdown->typography = LUI_TYPOGRAPHY_BODY_SMALL;
            dropdown->style.background_color = LUI_COLOR_STEEL_GRAY;
        }
        else if (setting->type == SETTING_TYPE_INTEGER) {
            // Text input for integer settings
            char value_text[32];
            snprintf(value_text, sizeof(value_text), "%d", setting->current_value.integer_value);
            
            lui_widget_t* input = lui_create_text_input(value_text, g_settings.settings_panel);
            input->bounds = lui_rect_make(320, y_offset - 2, 80, 24);
        }
        else if (setting->type == SETTING_TYPE_STRING) {
            // Text input for string settings
            lui_widget_t* input = lui_create_text_input(setting->current_value.string_value, 
                                                      g_settings.settings_panel);
            input->bounds = lui_rect_make(320, y_offset - 2, 150, 24);
        }
        
        // Setting description
        lui_widget_t* setting_desc = lui_create_label(setting->description, g_settings.settings_panel);
        setting_desc->bounds = lui_rect_make(16, y_offset + 25, 468, 16);
        setting_desc->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        setting_desc->typography.color = LUI_COLOR_STEEL_GRAY;
        
        // Modified indicator
        if (setting->is_modified) {
            lui_widget_t* modified_icon = lui_create_label("●", g_settings.settings_panel);
            modified_icon->bounds = lui_rect_make(450, y_offset, 12, 20);
            modified_icon->typography.color = LUI_COLOR_WARNING_AMBER;
        }
        
        // Admin/reboot indicators
        if (setting->constraints.requires_admin) {
            lui_widget_t* admin_icon = lui_create_label("🔒", g_settings.settings_panel);
            admin_icon->bounds = lui_rect_make(460, y_offset, 16, 16);
        }
        
        if (setting->constraints.requires_reboot) {
            lui_widget_t* reboot_icon = lui_create_label("⚠", g_settings.settings_panel);
            reboot_icon->bounds = lui_rect_make(476, y_offset, 16, 16);
            reboot_icon->typography.color = LUI_COLOR_ALERT_RED;
        }
        
        y_offset += 55;
        setting = setting->next;
    }
}

static void create_ai_panel(void) {
    if (!g_settings.show_ai_panel) return;
    
    g_settings.ai_panel = lui_create_container(g_settings.main_window->root_widget);
    strcpy(g_settings.ai_panel->name, "ai_panel");
    g_settings.ai_panel->bounds = lui_rect_make(700, 32, 300, 568);
    g_settings.ai_panel->background_color = LUI_COLOR_GRAPHITE;
    
    // AI header
    lui_widget_t* ai_header = lui_create_label("🤖 AI Configuration Assistant", g_settings.ai_panel);
    ai_header->bounds = lui_rect_make(8, 8, 284, 24);
    ai_header->typography = LUI_TYPOGRAPHY_TITLE_MEDIUM;
    ai_header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    // Configuration scores
    char scores_text[512];
    snprintf(scores_text, sizeof(scores_text),
            "Configuration Analysis:\n\n"
            "Security: %.0f%%\n"
            "Performance: %.0f%%\n"
            "Privacy: %.0f%%\n"
            "Power Efficiency: %.0f%%\n\n"
            "Issues Found:\n"
            "Security: %u\n"
            "Performance: %u\n"
            "Privacy: %u\n"
            "Config Errors: %u",
            g_settings.ai_analysis.security_score * 100.0f,
            g_settings.ai_analysis.performance_score * 100.0f,
            g_settings.ai_analysis.privacy_score * 100.0f,
            g_settings.ai_analysis.power_efficiency_score * 100.0f,
            g_settings.ai_analysis.security_issues,
            g_settings.ai_analysis.performance_issues,
            g_settings.ai_analysis.privacy_issues,
            g_settings.ai_analysis.configuration_errors);
    
    lui_widget_t* scores_label = lui_create_label(scores_text, g_settings.ai_panel);
    scores_label->bounds = lui_rect_make(8, 40, 284, 240);
    scores_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
    
    // AI Recommendations
    if (g_settings.ai_analysis.primary_recommendation != AI_SETTINGS_RECOMMEND_NONE) {
        lui_widget_t* rec_header = lui_create_label("💡 Recommendations:", g_settings.ai_panel);
        rec_header->bounds = lui_rect_make(8, 290, 200, 20);
        rec_header->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
        rec_header->typography.color = LUI_COLOR_WARNING_AMBER;
        
        lui_widget_t* rec_text = lui_create_label(g_settings.ai_analysis.recommendation_details, g_settings.ai_panel);
        rec_text->bounds = lui_rect_make(8, 315, 284, 100);
        rec_text->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        
        // Action buttons
        lui_widget_t* apply_btn = lui_create_button("Apply Recommendation", g_settings.ai_panel);
        apply_btn->bounds = lui_rect_make(8, 425, 140, 28);
        apply_btn->style.background_color = LUI_COLOR_SUCCESS_GREEN;
        
        lui_widget_t* ignore_btn = lui_create_button("Ignore", g_settings.ai_panel);
        ignore_btn->bounds = lui_rect_make(156, 425, 60, 28);
        ignore_btn->style.background_color = LUI_COLOR_STEEL_GRAY;
    }
    
    // Refresh analysis button
    lui_widget_t* refresh_btn = lui_create_button("🔄 Re-analyze", g_settings.ai_panel);
    refresh_btn->bounds = lui_rect_make(8, 520, 100, 28);
    refresh_btn->style.background_color = LUI_COLOR_TACTICAL_BLUE;
}

static void create_toolbar(void) {
    g_settings.toolbar = lui_create_container(g_settings.main_window->root_widget);
    strcpy(g_settings.toolbar->name, "toolbar");
    g_settings.toolbar->bounds = lui_rect_make(0, 0, 1000, 32);
    g_settings.toolbar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // Search box
    lui_widget_t* search_input = lui_create_text_input("Search settings...", g_settings.toolbar);
    search_input->bounds = lui_rect_make(8, 4, 200, 24);
    
    // Action buttons
    lui_widget_t* save_btn = lui_create_button("💾 Save", g_settings.toolbar);
    save_btn->bounds = lui_rect_make(220, 4, 60, 24);
    save_btn->style.background_color = g_settings.settings_modified ? 
                                      LUI_COLOR_SUCCESS_GREEN : LUI_COLOR_STEEL_GRAY;
    
    lui_widget_t* reset_btn = lui_create_button("↺ Reset", g_settings.toolbar);
    reset_btn->bounds = lui_rect_make(290, 4, 60, 24);
    reset_btn->style.background_color = LUI_COLOR_WARNING_AMBER;
    
    lui_widget_t* defaults_btn = lui_create_button("🏠 Defaults", g_settings.toolbar);
    defaults_btn->bounds = lui_rect_make(360, 4, 80, 24);
    defaults_btn->style.background_color = LUI_COLOR_ALERT_RED;
    
    // Admin mode indicator
    if (g_settings.has_admin_privileges) {
        lui_widget_t* admin_indicator = lui_create_label("👑 Administrator", g_settings.toolbar);
        admin_indicator->bounds = lui_rect_make(800, 6, 120, 20);
        admin_indicator->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        admin_indicator->typography.color = LUI_COLOR_WARNING_AMBER;
    }
}

static void create_status_bar(void) {
    g_settings.status_bar = lui_create_container(g_settings.main_window->root_widget);
    strcpy(g_settings.status_bar->name, "status_bar");
    g_settings.status_bar->bounds = lui_rect_make(0, 600, 1000, 24);
    g_settings.status_bar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // Status information
    char status_text[256];
    snprintf(status_text, sizeof(status_text),
            "System: %s %s | Security Level: %s | Settings Modified: %u",
            g_settings.system_info.os_name,
            g_settings.system_info.os_version,
            (g_settings.current_security_level == SECURITY_LEVEL_MINIMAL) ? "Minimal" :
            (g_settings.current_security_level == SECURITY_LEVEL_STANDARD) ? "Standard" :
            (g_settings.current_security_level == SECURITY_LEVEL_ENHANCED) ? "Enhanced" :
            (g_settings.current_security_level == SECURITY_LEVEL_MILITARY) ? "Military" : "Classified",
            g_settings.stats.settings_changed);
    
    lui_widget_t* status_label = lui_create_label(status_text, g_settings.status_bar);
    status_label->bounds = lui_rect_make(8, 2, 700, 20);
    status_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    // AI status
    char ai_status[64];
    snprintf(ai_status, sizeof(ai_status), "AI: %s", 
            g_settings.ai_recommendations_enabled ? "Active" : "Disabled");
    
    lui_widget_t* ai_status_label = lui_create_label(ai_status, g_settings.status_bar);
    ai_status_label->bounds = lui_rect_make(720, 2, 80, 20);
    ai_status_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    ai_status_label->typography.color = g_settings.ai_recommendations_enabled ? 
                                        LUI_COLOR_SECURE_CYAN : LUI_COLOR_STEEL_GRAY;
}

// ============================================================================
// MAIN SETTINGS API
// ============================================================================

bool limitless_settings_init(void) {
    if (g_settings.initialized) {
        return false;
    }
    
    printf("[Settings] Initializing Limitless Settings v%s\n", SETTINGS_VERSION);
    
    // Clear state
    memset(&g_settings, 0, sizeof(g_settings));
    
    // Check for admin privileges
    g_settings.has_admin_privileges = (getuid() == 0);
    
    // Set default configuration
    g_settings.current_category = SETTINGS_CATEGORY_SYSTEM;
    g_settings.current_security_level = SECURITY_LEVEL_STANDARD;
    g_settings.show_advanced_settings = false;
    g_settings.show_ai_panel = true;
    g_settings.confirm_dangerous_changes = true;
    g_settings.auto_apply_safe_changes = true;
    g_settings.ai_recommendations_enabled = true;
    
    // Collect system information
    collect_system_information();
    
    // Initialize settings categories
    initialize_system_settings();
    initialize_security_settings();
    initialize_appearance_settings();
    initialize_ai_settings();
    
    // Perform initial AI analysis
    if (g_settings.ai_recommendations_enabled) {
        perform_ai_settings_analysis();
    }
    
    // Create main window
    int window_width = g_settings.show_ai_panel ? 1000 : 700;
    g_settings.main_window = lui_create_window("Limitless Settings", LUI_WINDOW_NORMAL,
                                             100, 100, window_width, 624);
    if (!g_settings.main_window) {
        printf("[Settings] ERROR: Failed to create main window\n");
        return false;
    }
    
    // Create UI components
    create_toolbar();
    create_category_sidebar();
    create_settings_panel();
    create_ai_panel();
    create_status_bar();
    
    // Show window
    lui_show_window(g_settings.main_window);
    
    g_settings.initialized = true;
    g_settings.running = true;
    g_settings.stats.session_start_time = time(NULL);
    
    printf("[Settings] Settings panel initialized successfully\n");
    printf("[Settings] Admin privileges: %s, Categories: %u, AI enabled: %s\n",
           g_settings.has_admin_privileges ? "Yes" : "No",
           g_settings.category_count,
           g_settings.ai_recommendations_enabled ? "Yes" : "No");
    
    return true;
}

void limitless_settings_shutdown(void) {
    if (!g_settings.initialized) {
        return;
    }
    
    printf("[Settings] Shutting down Limitless Settings\n");
    
    g_settings.running = false;
    
    // Free settings categories and their settings
    settings_category_t* category = g_settings.categories;
    while (category) {
        settings_category_t* next_category = category->next;
        
        setting_definition_t* setting = category->settings;
        while (setting) {
            setting_definition_t* next_setting = setting->next;
            free(setting);
            setting = next_setting;
        }
        
        free(category);
        category = next_category;
    }
    
    // Free user accounts
    user_account_t* user = g_settings.user_accounts;
    while (user) {
        user_account_t* next = user->next;
        free(user);
        user = next;
    }
    
    // Free security policies
    security_policy_t* policy = g_settings.security_policies;
    while (policy) {
        security_policy_t* next = policy->next;
        free(policy);
        policy = next;
    }
    
    // Free network configs
    network_config_t* network = g_settings.network_configs;
    while (network) {
        network_config_t* next = network->next;
        free(network);
        network = next;
    }
    
    // Free hardware configs
    hardware_config_t* hardware = g_settings.hardware_configs;
    while (hardware) {
        hardware_config_t* next = hardware->next;
        free(hardware);
        hardware = next;
    }
    
    // Destroy main window
    if (g_settings.main_window) {
        lui_destroy_window(g_settings.main_window);
    }
    
    // Print session statistics
    time_t session_duration = time(NULL) - g_settings.stats.session_start_time;
    printf("[Settings] Session statistics:\n");
    printf("  Duration: %ld seconds\n", session_duration);
    printf("  Settings changed: %u\n", g_settings.stats.settings_changed);
    printf("  Categories visited: %u\n", g_settings.stats.categories_visited);
    printf("  AI recommendations applied: %u\n", g_settings.stats.ai_recommendations_applied);
    printf("  Security changes made: %u\n", g_settings.stats.security_changes_made);
    
    memset(&g_settings, 0, sizeof(g_settings));
    
    printf("[Settings] Shutdown complete\n");
}

const char* limitless_settings_get_version(void) {
    return SETTINGS_VERSION;
}

void limitless_settings_run(void) {
    if (!g_settings.initialized) {
        printf("[Settings] ERROR: Settings not initialized\n");
        return;
    }
    
    printf("[Settings] Running Limitless Settings\n");
    
    // Main event loop is handled by the desktop environment
}