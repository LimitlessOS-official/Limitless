/**
 * LimitlessOS GUI Installer Header
 * Professional graphical installer interface definitions
 */

#ifndef GUI_INSTALLER_H
#define GUI_INSTALLER_H

#include <stdint.h>
#include <stdbool.h>
#include "installer.h"

/* GUI System Integration */
typedef struct gui_context gui_context_t;
typedef struct gui_window gui_window_t;

/* GUI Events */
typedef enum {
    GUI_EVENT_NONE,
    GUI_EVENT_KEY_PRESS,
    GUI_EVENT_KEY_RELEASE,
    GUI_EVENT_MOUSE_MOVE,
    GUI_EVENT_MOUSE_PRESS,
    GUI_EVENT_MOUSE_RELEASE,
    GUI_EVENT_BUTTON_CLICK,
    GUI_EVENT_CHECKBOX_CLICK,
    GUI_EVENT_TEXT_INPUT,
    GUI_EVENT_QUIT
} gui_event_type_t;

typedef struct {
    gui_event_type_t type;
    union {
        struct {
            uint32_t keycode;
            bool shift, ctrl, alt;
        } key;
        struct {
            int32_t x, y;
            int32_t dx, dy;
            uint32_t buttons;
        } mouse;
        struct {
            uint32_t button_id;
        };
        struct {
            char text[32];
        } text;
    };
} gui_input_event_t;

/* GUI Rectangles */
typedef struct {
    int32_t x, y;
    int32_t width, height;
} gui_rect_t;

/* GUI Theme */
typedef struct {
    uint32_t primary_color;
    uint32_t secondary_color;
    uint32_t success_color;
    uint32_t warning_color;
    uint32_t error_color;
    uint32_t background_color;
    uint32_t surface_color;
    uint32_t text_color;
    uint32_t text_light_color;
} gui_theme_t;

/* Text Alignment */
typedef enum {
    GUI_TEXT_ALIGN_LEFT,
    GUI_TEXT_ALIGN_CENTER,
    GUI_TEXT_ALIGN_RIGHT
} gui_text_align_t;

/* Installer Pages */
typedef enum {
    PAGE_WELCOME = 0,
    PAGE_LICENSE,
    PAGE_PRIVACY,
    PAGE_HARDWARE,
    PAGE_DISK_SELECTION,
    PAGE_PARTITIONING,
    PAGE_SECURITY,
    PAGE_USER_ACCOUNT,
    PAGE_FEATURES,
    PAGE_SUMMARY,
    PAGE_INSTALLATION,
    PAGE_COMPLETE
} installer_page_t;

/* GUI Modes */
typedef enum {
    GUI_MODE_GRAPHICAL,
    GUI_MODE_TEXT_ONLY,
    GUI_MODE_ACCESSIBILITY
} gui_mode_t;

typedef enum {
    GUI_THEME_PROFESSIONAL,
    GUI_THEME_DARK,
    GUI_THEME_HIGH_CONTRAST
} gui_theme_mode_t;

/* Installation Configuration Extended */
typedef struct {
    /* Basic installation settings */
    installation_type_t installation_type;
    architecture_t target_architecture;
    filesystem_type_t filesystem_type;
    
    /* Security settings */
    bool enable_encryption;
    bool enable_secure_boot;
    bool enable_tpm;
    encryption_level_t encryption_level;
    
    /* AI and privacy settings */
    bool enable_ai_features;
    bool ai_hardware_detection;
    bool ai_partition_optimization;
    bool ai_security_analysis;
    bool ai_performance_optimization;
    bool telemetry_enabled;
    bool data_collection_enabled;
    
    /* User preferences */
    bool license_accepted;
    char username[64];
    char password[128];
    char full_name[128];
    char hostname[64];
    char timezone[64];
    
    /* Hardware selection */
    uint32_t selected_disk_index;
    bool custom_partitioning;
    
    /* Feature selection */
    bool install_development_tools;
    bool install_multimedia_codecs;
    bool install_enterprise_tools;
    bool install_ai_framework;
    bool install_quantum_computing;
    bool install_blockchain_support;
    
    /* Network settings */
    bool configure_network;
    char wifi_ssid[64];
    char wifi_password[128];
} installation_config_t;

/* Hardware Information */
typedef struct {
    /* CPU Information */
    char cpu_model[128];
    uint32_t cpu_cores;
    uint32_t cpu_threads;
    uint64_t cpu_frequency_mhz;
    bool virtualization_support;
    bool aes_ni_support;
    
    /* Memory Information */
    uint64_t total_memory_mb;
    uint64_t available_memory_mb;
    uint32_t memory_slots;
    uint32_t memory_speed_mhz;
    
    /* Storage Information */
    uint32_t storage_devices;
    uint64_t total_storage_gb;
    
    /* Graphics Information */
    char gpu_model[128];
    uint64_t gpu_memory_mb;
    bool gpu_acceleration;
    
    /* Network Information */
    bool ethernet_available;
    bool wifi_available;
    bool bluetooth_available;
    
    /* Security Features */
    bool tpm_available;
    bool secure_boot_available;
    bool hardware_encryption;
} hardware_info_t;

/* GUI Installer State */
typedef struct {
    bool enabled;
    gui_mode_t mode;
    gui_theme_mode_t theme;
    bool accessibility_enabled;
    bool high_dpi_support;
    uint32_t display_width;
    uint32_t display_height;
    uint32_t scaling_factor;
} gui_installer_t;

/* Progress Tracker */
typedef struct {
    uint32_t total_steps;
    uint32_t current_step;
    uint32_t completed_steps;
    uint32_t overall_progress;
    installation_phase_t current_phase;
    const char* current_step_name;
    const char* current_status;
    bool installation_active;
} progress_tracker_t;

/* Error Handler */
typedef struct {
    uint32_t error_count;
    uint32_t warning_count;
    char last_error[256];
    char last_warning[256];
    bool fatal_error;
} error_handler_t;

/* Deployment Manager */
typedef struct {
    bool automated_mode;
    bool silent_install;
    bool unattended_install;
    char config_file_path[256];
    char deployment_profile[64];
} deployment_manager_t;

/* Button IDs */
typedef enum {
    BUTTON_NONE = 0,
    BUTTON_BACK,
    BUTTON_NEXT,
    BUTTON_INSTALL,
    BUTTON_CANCEL,
    BUTTON_FINISH,
    BUTTON_RESTART,
    BUTTON_SHUTDOWN,
    BUTTON_ADVANCED,
    BUTTON_HELP
} gui_button_id_t;

/* Status codes */
typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR = -1,
    STATUS_ALREADY_INITIALIZED = 1,
    STATUS_NOT_INITIALIZED = 2,
    STATUS_INVALID_PARAMETER = 3,
    STATUS_INSUFFICIENT_MEMORY = 4,
    STATUS_HARDWARE_NOT_SUPPORTED = 5,
    STATUS_USER_CANCELLED = 6
} status_t;

/* Architecture types */
typedef enum {
    ARCH_X86_64,
    ARCH_ARM64,
    ARCH_RISCV64
} architecture_t;

/* Encryption levels */
typedef enum {
    ENCRYPTION_NONE,
    ENCRYPTION_AES_128_XTS,
    ENCRYPTION_AES_256_XTS,
    ENCRYPTION_CHACHA20_POLY1305,
    ENCRYPTION_QUANTUM_RESISTANT
} encryption_level_t;

/* Installation phases */
typedef enum {
    PHASE_INITIALIZATION,
    PHASE_HARDWARE_DETECTION,
    PHASE_DISK_PREPARATION,
    PHASE_PARTITION_CREATION,
    PHASE_FILESYSTEM_SETUP,
    PHASE_SYSTEM_INSTALLATION,
    PHASE_BOOTLOADER_SETUP,
    PHASE_USER_CONFIGURATION,
    PHASE_SECURITY_SETUP,
    PHASE_ENTERPRISE_CONFIG,
    PHASE_AI_INITIALIZATION,
    PHASE_FINALIZATION,
    PHASE_COMPLETE
} installation_phase_t;

/* Function prototypes */

/* GUI Installer Core Functions */
status_t gui_installer_init(void);
status_t gui_installer_run(void);
void gui_installer_cleanup(void);

/* Navigation Functions */
status_t gui_installer_next_page(void);
status_t gui_installer_previous_page(void);
status_t gui_installer_goto_page(installer_page_t page);

/* Configuration Functions */
status_t gui_installer_set_config(const installation_config_t* config);
status_t gui_installer_get_config(installation_config_t* config);
status_t gui_installer_load_config_file(const char* config_file);
status_t gui_installer_save_config_file(const char* config_file);

/* Hardware Detection Functions */
status_t gui_installer_detect_hardware(hardware_info_t* hardware);
status_t gui_installer_get_disk_info(uint32_t disk_index, disk_info_t* disk_info);
status_t gui_installer_validate_hardware_requirements(void);

/* Progress and Status Functions */
status_t gui_installer_update_progress(uint32_t percent, const char* status);
status_t gui_installer_get_progress(uint32_t* percent, char* status, size_t status_size);
status_t gui_installer_report_error(const char* error_message);
status_t gui_installer_report_warning(const char* warning_message);

/* GUI Drawing Functions (to be implemented by graphics system) */
gui_context_t* gui_create_context(uint32_t width, uint32_t height);
void gui_destroy_context(gui_context_t* ctx);
gui_window_t* gui_create_window(gui_context_t* ctx, const char* title, uint32_t width, uint32_t height);
void gui_destroy_window(gui_window_t* window);

void gui_begin_frame(gui_context_t* ctx);
void gui_end_frame(gui_context_t* ctx);
void gui_present_frame(gui_context_t* ctx);
void gui_clear_background(gui_context_t* ctx, uint32_t color);

bool gui_poll_input_event(gui_context_t* ctx, gui_input_event_t* event);

void gui_fill_rect(gui_context_t* ctx, gui_rect_t rect, uint32_t color);
void gui_fill_rounded_rect(gui_context_t* ctx, gui_rect_t rect, uint32_t color, uint32_t radius);
void gui_draw_rect_outline(gui_context_t* ctx, gui_rect_t rect, uint32_t color, uint32_t thickness);
void gui_draw_rounded_rect_outline(gui_context_t* ctx, gui_rect_t rect, uint32_t color, uint32_t thickness, uint32_t radius);
void gui_fill_circle(gui_context_t* ctx, int32_t x, int32_t y, uint32_t radius, uint32_t color);
void gui_draw_line(gui_context_t* ctx, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color, uint32_t thickness);

void gui_set_font_size(gui_context_t* ctx, uint32_t size);
void gui_set_text_color(gui_context_t* ctx, uint32_t color);
void gui_draw_text(gui_context_t* ctx, gui_rect_t area, const char* text, gui_text_align_t align);
void gui_draw_text_centered(gui_context_t* ctx, gui_rect_t area, const char* text);
void gui_draw_logo(gui_context_t* ctx, gui_rect_t area);

/* Internal rendering functions */
static void render_main_layout(void);
static void render_header(gui_context_t* ctx, gui_rect_t area);
static void render_sidebar(gui_context_t* ctx, gui_rect_t area);
static void render_footer(gui_context_t* ctx, gui_rect_t area);
static void render_button(gui_context_t* ctx, gui_rect_t area, const char* text, bool primary, bool disabled);
static void render_progress_bar(gui_context_t* ctx, gui_rect_t area, float progress);
static void render_checkbox(gui_context_t* ctx, gui_rect_t area, bool checked);

#endif /* GUI_INSTALLER_H */