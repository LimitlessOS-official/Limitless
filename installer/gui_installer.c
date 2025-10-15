/**
 * LimitlessOS Graphical Installer Interface
 * Professional GUI installer with step-by-step wizard
 * Integrated with enterprise graphics system
 */

#include "gui_installer.h"
#include "../userspace/include/graphics_enterprise.h"
#include "complete_installer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* GUI Constants */
#define WINDOW_WIDTH        1024
#define WINDOW_HEIGHT       768
#define SIDEBAR_WIDTH       200
#define HEADER_HEIGHT       80
#define FOOTER_HEIGHT       60
#define BUTTON_WIDTH        120
#define BUTTON_HEIGHT       35
#define PROGRESS_BAR_HEIGHT 20

/* Colors (RGB) */
#define COLOR_PRIMARY       0x2E86DE
#define COLOR_SECONDARY     0x54A0FF
#define COLOR_SUCCESS       0x5F27CD
#define COLOR_WARNING       0xFF9F43
#define COLOR_ERROR         0xEE5A24
#define COLOR_BACKGROUND    0xF1F2F6
#define COLOR_SURFACE       0xFFFFFF
#define COLOR_TEXT          0x2F3640
#define COLOR_TEXT_LIGHT    0x57606F

/* GUI State */
typedef struct {
    bool initialized;
    gui_window_t* main_window;
    gui_context_t* context;
    installer_page_t current_page;
    uint32_t animation_frame;
    float progress_animation;
    bool installation_running;
    installation_config_t* config;
    hardware_info_t* hardware;
    gui_theme_t theme;
} gui_installer_state_t;

static gui_installer_state_t g_gui_state = {0};

/* Page definitions */
typedef struct {
    installer_page_t page_id;
    const char* title;
    const char* subtitle;
    bool (*render_func)(gui_context_t* ctx, gui_rect_t content_area);
    bool (*handle_input_func)(gui_input_event_t* event);
    bool show_navigation;
    bool show_progress;
} page_definition_t;

/* Forward declarations */
static bool render_welcome_page(gui_context_t* ctx, gui_rect_t content_area);
static bool render_license_page(gui_context_t* ctx, gui_rect_t content_area);
static bool render_privacy_page(gui_context_t* ctx, gui_rect_t content_area);
static bool render_hardware_page(gui_context_t* ctx, gui_rect_t content_area);
static bool render_disk_page(gui_context_t* ctx, gui_rect_t content_area);
static bool render_partitioning_page(gui_context_t* ctx, gui_rect_t content_area);
static bool render_security_page(gui_context_t* ctx, gui_rect_t content_area);
static bool render_user_page(gui_context_t* ctx, gui_rect_t content_area);
static bool render_features_page(gui_context_t* ctx, gui_rect_t content_area);
static bool render_summary_page(gui_context_t* ctx, gui_rect_t content_area);
static bool render_installation_page(gui_context_t* ctx, gui_rect_t content_area);
static bool render_complete_page(gui_context_t* ctx, gui_rect_t content_area);

static bool handle_welcome_input(gui_input_event_t* event);
static bool handle_license_input(gui_input_event_t* event);
static bool handle_privacy_input(gui_input_event_t* event);
static bool handle_hardware_input(gui_input_event_t* event);
static bool handle_disk_input(gui_input_event_t* event);
static bool handle_partitioning_input(gui_input_event_t* event);
static bool handle_security_input(gui_input_event_t* event);
static bool handle_user_input(gui_input_event_t* event);
static bool handle_features_input(gui_input_event_t* event);
static bool handle_summary_input(gui_input_event_t* event);
static bool handle_installation_input(gui_input_event_t* event);
static bool handle_complete_input(gui_input_event_t* event);

/* Page definitions array */
static page_definition_t installer_pages[] = {
    {PAGE_WELCOME, "Welcome to LimitlessOS", "The superior operating system for the future", render_welcome_page, handle_welcome_input, true, false},
    {PAGE_LICENSE, "License Agreement", "Please read and accept the LimitlessOS license", render_license_page, handle_license_input, true, false},
    {PAGE_PRIVACY, "Privacy Settings", "Configure AI features and data privacy", render_privacy_page, handle_privacy_input, true, false},
    {PAGE_HARDWARE, "Hardware Detection", "Analyzing your system configuration", render_hardware_page, handle_hardware_input, true, true},
    {PAGE_DISK_SELECTION, "Disk Selection", "Choose installation target", render_disk_page, handle_disk_input, true, false},
    {PAGE_PARTITIONING, "Disk Partitioning", "Configure disk layout", render_partitioning_page, handle_partitioning_input, true, false},
    {PAGE_SECURITY, "Security Options", "Configure encryption and security features", render_security_page, handle_security_input, true, false},
    {PAGE_USER_ACCOUNT, "User Account", "Create your user account", render_user_page, handle_user_input, true, false},
    {PAGE_FEATURES, "Feature Selection", "Choose which features to install", render_features_page, handle_features_input, true, false},
    {PAGE_SUMMARY, "Installation Summary", "Review your installation settings", render_summary_page, handle_summary_input, true, false},
    {PAGE_INSTALLATION, "Installing LimitlessOS", "Please wait while LimitlessOS is installed", render_installation_page, handle_installation_input, false, true},
    {PAGE_COMPLETE, "Installation Complete", "LimitlessOS has been installed successfully", render_complete_page, handle_complete_input, false, false}
};

#define NUM_PAGES (sizeof(installer_pages) / sizeof(page_definition_t))

/**
 * Initialize GUI installer
 */
status_t gui_installer_init(void) {
    if (g_gui_state.initialized) {
        return STATUS_SUCCESS;
    }
    
    /* Initialize graphics system */
    if (graphics_enterprise_init() != STATUS_SUCCESS) {
        printf("[ERROR] Failed to initialize graphics system\n");
        return STATUS_ERROR;
    }
    
    /* Create GUI context */
    g_gui_state.context = gui_create_context(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!g_gui_state.context) {
        printf("[ERROR] Failed to create GUI context\n");
        return STATUS_ERROR;
    }
    
    /* Create main window */
    g_gui_state.main_window = gui_create_window(g_gui_state.context, 
                                                "LimitlessOS Installer",
                                                WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!g_gui_state.main_window) {
        printf("[ERROR] Failed to create main window\n");
        return STATUS_ERROR;
    }
    
    /* Initialize theme */
    g_gui_state.theme.primary_color = COLOR_PRIMARY;
    g_gui_state.theme.secondary_color = COLOR_SECONDARY;
    g_gui_state.theme.background_color = COLOR_BACKGROUND;
    g_gui_state.theme.surface_color = COLOR_SURFACE;
    g_gui_state.theme.text_color = COLOR_TEXT;
    g_gui_state.theme.text_light_color = COLOR_TEXT_LIGHT;
    
    /* Set initial state */
    g_gui_state.current_page = PAGE_WELCOME;
    g_gui_state.animation_frame = 0;
    g_gui_state.progress_animation = 0.0f;
    g_gui_state.installation_running = false;
    
    /* Allocate configuration structures */
    g_gui_state.config = calloc(1, sizeof(installation_config_t));
    g_gui_state.hardware = calloc(1, sizeof(hardware_info_t));
    
    if (!g_gui_state.config || !g_gui_state.hardware) {
        printf("[ERROR] Failed to allocate memory for installer state\n");
        return STATUS_ERROR;
    }
    
    /* Set default configuration */
    g_gui_state.config->installation_type = INSTALL_TYPE_FULL;
    g_gui_state.config->enable_encryption = true;
    g_gui_state.config->enable_ai_features = false; /* Privacy first */
    g_gui_state.config->enable_secure_boot = true;
    g_gui_state.config->filesystem_type = FS_TYPE_LIMITLESS_FS;
    
    printf("[GUI] LimitlessOS GUI installer initialized\n");
    g_gui_state.initialized = true;
    
    return STATUS_SUCCESS;
}

/**
 * Run GUI installer main loop
 */
status_t gui_installer_run(void) {
    if (!g_gui_state.initialized) {
        return STATUS_ERROR;
    }
    
    printf("[GUI] Starting GUI installer main loop\n");
    
    bool running = true;
    gui_input_event_t input_event;
    
    while (running) {
        /* Handle input events */
        while (gui_poll_input_event(g_gui_state.context, &input_event)) {
            /* Handle global events */
            if (input_event.type == GUI_EVENT_QUIT) {
                running = false;
                break;
            }
            
            /* Handle page-specific input */
            page_definition_t* current_page_def = &installer_pages[g_gui_state.current_page];
            if (current_page_def->handle_input_func) {
                current_page_def->handle_input_func(&input_event);
            }
        }
        
        if (!running) break;
        
        /* Render current page */
        gui_begin_frame(g_gui_state.context);
        gui_clear_background(g_gui_state.context, g_gui_state.theme.background_color);
        
        render_main_layout();
        
        gui_end_frame(g_gui_state.context);
        gui_present_frame(g_gui_state.context);
        
        /* Animation update */
        g_gui_state.animation_frame++;
        g_gui_state.progress_animation += 0.02f;
        if (g_gui_state.progress_animation > 1.0f) {
            g_gui_state.progress_animation = 0.0f;
        }
        
        /* Small delay for smooth animation */
        usleep(16666); /* ~60 FPS */
    }
    
    return STATUS_SUCCESS;
}

/**
 * Render main layout
 */
static void render_main_layout(void) {
    gui_context_t* ctx = g_gui_state.context;
    
    /* Calculate layout areas */
    gui_rect_t header_area = {0, 0, WINDOW_WIDTH, HEADER_HEIGHT};
    gui_rect_t sidebar_area = {0, HEADER_HEIGHT, SIDEBAR_WIDTH, WINDOW_HEIGHT - HEADER_HEIGHT - FOOTER_HEIGHT};
    gui_rect_t content_area = {SIDEBAR_WIDTH, HEADER_HEIGHT, 
                              WINDOW_WIDTH - SIDEBAR_WIDTH, 
                              WINDOW_HEIGHT - HEADER_HEIGHT - FOOTER_HEIGHT};
    gui_rect_t footer_area = {0, WINDOW_HEIGHT - FOOTER_HEIGHT, WINDOW_WIDTH, FOOTER_HEIGHT};
    
    /* Render header */
    render_header(ctx, header_area);
    
    /* Render sidebar */
    render_sidebar(ctx, sidebar_area);
    
    /* Render current page content */
    page_definition_t* current_page_def = &installer_pages[g_gui_state.current_page];
    if (current_page_def->render_func) {
        current_page_def->render_func(ctx, content_area);
    }
    
    /* Render footer */
    render_footer(ctx, footer_area);
}

/**
 * Render header
 */
static void render_header(gui_context_t* ctx, gui_rect_t area) {
    /* Header background */
    gui_fill_rect(ctx, area, g_gui_state.theme.primary_color);
    
    /* LimitlessOS logo and title */
    gui_rect_t logo_area = {20, 15, 50, 50};
    gui_draw_logo(ctx, logo_area);
    
    gui_rect_t title_area = {80, 20, 400, 40};
    gui_set_text_color(ctx, 0xFFFFFF);
    gui_set_font_size(ctx, 24);
    gui_draw_text(ctx, title_area, "LimitlessOS Enterprise Installer", GUI_TEXT_ALIGN_LEFT);
    
    /* Current page indicator */
    page_definition_t* current_page_def = &installer_pages[g_gui_state.current_page];
    gui_rect_t page_area = {WINDOW_WIDTH - 300, 25, 280, 30};
    gui_set_font_size(ctx, 16);
    char page_text[128];
    snprintf(page_text, sizeof(page_text), "Step %d of %d", 
             (int)g_gui_state.current_page + 1, NUM_PAGES);
    gui_draw_text(ctx, page_area, page_text, GUI_TEXT_ALIGN_RIGHT);
}

/**
 * Render sidebar with navigation steps
 */
static void render_sidebar(gui_context_t* ctx, gui_rect_t area) {
    /* Sidebar background */
    gui_fill_rect(ctx, area, g_gui_state.theme.surface_color);
    
    /* Draw border */
    gui_draw_rect_outline(ctx, area, g_gui_state.theme.text_light_color, 1);
    
    /* Navigation steps */
    int step_height = 45;
    int y_offset = 20;
    
    for (int i = 0; i < NUM_PAGES; i++) {
        gui_rect_t step_area = {10, y_offset, SIDEBAR_WIDTH - 20, step_height - 5};
        
        /* Step state colors */
        uint32_t bg_color = g_gui_state.theme.surface_color;
        uint32_t text_color = g_gui_state.theme.text_light_color;
        
        if (i < g_gui_state.current_page) {
            /* Completed step */
            bg_color = g_gui_state.theme.success_color;
            text_color = 0xFFFFFF;
        } else if (i == g_gui_state.current_page) {
            /* Current step */
            bg_color = g_gui_state.theme.primary_color;
            text_color = 0xFFFFFF;
        }
        
        /* Draw step background */
        gui_fill_rounded_rect(ctx, step_area, bg_color, 5);
        
        /* Draw step number */
        gui_rect_t number_area = {15, y_offset + 5, 25, 25};
        gui_fill_circle(ctx, number_area.x + 12, number_area.y + 12, 12, text_color);
        
        char number_text[8];
        snprintf(number_text, sizeof(number_text), "%d", i + 1);
        gui_set_text_color(ctx, bg_color);
        gui_set_font_size(ctx, 12);
        gui_draw_text_centered(ctx, number_area, number_text);
        
        /* Draw step name */
        gui_rect_t name_area = {45, y_offset + 8, SIDEBAR_WIDTH - 60, 20};
        gui_set_text_color(ctx, text_color);
        gui_set_font_size(ctx, 11);
        gui_draw_text(ctx, name_area, installer_pages[i].title, GUI_TEXT_ALIGN_LEFT);
        
        y_offset += step_height;
    }
}

/**
 * Render footer with navigation buttons
 */
static void render_footer(gui_context_t* ctx, gui_rect_t area) {
    /* Footer background */
    gui_fill_rect(ctx, area, g_gui_state.theme.surface_color);
    gui_draw_line(ctx, area.x, area.y, area.x + area.width, area.y, 
                  g_gui_state.theme.text_light_color, 1);
    
    page_definition_t* current_page_def = &installer_pages[g_gui_state.current_page];
    
    if (current_page_def->show_navigation) {
        /* Back button */
        if (g_gui_state.current_page > PAGE_WELCOME) {
            gui_rect_t back_button = {20, area.y + 15, BUTTON_WIDTH, BUTTON_HEIGHT};
            render_button(ctx, back_button, "Back", false, false);
        }
        
        /* Next button */
        gui_rect_t next_button = {WINDOW_WIDTH - BUTTON_WIDTH - 20, 
                                  area.y + 15, BUTTON_WIDTH, BUTTON_HEIGHT};
        const char* button_text = "Next";
        if (g_gui_state.current_page == PAGE_SUMMARY) {
            button_text = "Install";
        }
        render_button(ctx, next_button, button_text, true, false);
    }
    
    /* Progress indicator for installation page */
    if (current_page_def->show_progress && g_gui_state.installation_running) {
        gui_rect_t progress_area = {SIDEBAR_WIDTH + 20, area.y + 20, 
                                    WINDOW_WIDTH - SIDEBAR_WIDTH - 40, PROGRESS_BAR_HEIGHT};
        render_progress_bar(ctx, progress_area, g_gui_state.progress_animation);
    }
}

/**
 * Render a button
 */
static void render_button(gui_context_t* ctx, gui_rect_t area, const char* text, 
                         bool primary, bool disabled) {
    uint32_t bg_color = primary ? g_gui_state.theme.primary_color : g_gui_state.theme.surface_color;
    uint32_t text_color = primary ? 0xFFFFFF : g_gui_state.theme.text_color;
    uint32_t border_color = g_gui_state.theme.text_light_color;
    
    if (disabled) {
        bg_color = g_gui_state.theme.text_light_color;
        text_color = 0xFFFFFF;
    }
    
    /* Button background */
    gui_fill_rounded_rect(ctx, area, bg_color, 5);
    
    /* Button border */
    if (!primary) {
        gui_draw_rounded_rect_outline(ctx, area, border_color, 1, 5);
    }
    
    /* Button text */
    gui_set_text_color(ctx, text_color);
    gui_set_font_size(ctx, 14);
    gui_draw_text_centered(ctx, area, text);
}

/**
 * Render progress bar
 */
static void render_progress_bar(gui_context_t* ctx, gui_rect_t area, float progress) {
    /* Progress bar background */
    gui_fill_rounded_rect(ctx, area, g_gui_state.theme.text_light_color, 3);
    
    /* Progress bar fill */
    gui_rect_t fill_area = {area.x, area.y, 
                           (int)(area.width * progress), area.height};
    gui_fill_rounded_rect(ctx, fill_area, g_gui_state.theme.primary_color, 3);
    
    /* Progress text */
    char progress_text[32];
    snprintf(progress_text, sizeof(progress_text), "%.0f%%", progress * 100);
    gui_set_text_color(ctx, 0xFFFFFF);
    gui_set_font_size(ctx, 12);
    gui_draw_text_centered(ctx, area, progress_text);
}

/**
 * Page render functions
 */

static bool render_welcome_page(gui_context_t* ctx, gui_rect_t content_area) {
    /* Welcome content area */
    gui_rect_t welcome_area = {content_area.x + 40, content_area.y + 60, 
                              content_area.width - 80, content_area.height - 120};
    
    /* Welcome title */
    gui_set_text_color(ctx, g_gui_state.theme.text_color);
    gui_set_font_size(ctx, 36);
    gui_rect_t title_area = {welcome_area.x, welcome_area.y, welcome_area.width, 50};
    gui_draw_text_centered(ctx, title_area, "Welcome to LimitlessOS");
    
    /* Subtitle */
    gui_set_font_size(ctx, 18);
    gui_set_text_color(ctx, g_gui_state.theme.text_light_color);
    gui_rect_t subtitle_area = {welcome_area.x, welcome_area.y + 70, welcome_area.width, 30};
    gui_draw_text_centered(ctx, subtitle_area, "The Superior Operating System for the Future");
    
    /* Feature highlights */
    int feature_y = welcome_area.y + 140;
    const char* features[] = {
        "Enterprise-grade security and encryption",
        "Advanced graphics and desktop environment", 
        "Privacy-preserving AI framework with user control",
        "Application sandboxing for maximum security",
        "Complete development environment included",
        "Military-grade cryptographic protocols"
    };
    
    gui_set_font_size(ctx, 16);
    for (int i = 0; i < 6; i++) {
        gui_rect_t feature_area = {welcome_area.x + 50, feature_y, welcome_area.width - 100, 25};
        
        /* Checkmark icon */
        gui_fill_circle(ctx, welcome_area.x + 20, feature_y + 12, 8, g_gui_state.theme.success_color);
        gui_set_text_color(ctx, 0xFFFFFF);
        gui_draw_text_centered(ctx, (gui_rect_t){welcome_area.x + 12, feature_y + 4, 16, 16}, "✓");
        
        /* Feature text */
        gui_set_text_color(ctx, g_gui_state.theme.text_color);
        gui_draw_text(ctx, feature_area, features[i], GUI_TEXT_ALIGN_LEFT);
        
        feature_y += 35;
    }
    
    /* System requirements */
    gui_rect_t req_area = {welcome_area.x, feature_y + 30, welcome_area.width, 100};
    gui_set_font_size(ctx, 14);
    gui_set_text_color(ctx, g_gui_state.theme.text_light_color);
    gui_draw_text(ctx, req_area, 
        "System Requirements:\n"
        "• 64-bit processor with virtualization support\n"
        "• Minimum 4GB RAM (8GB recommended)\n" 
        "• 20GB available disk space\n"
        "• UEFI firmware (recommended)",
        GUI_TEXT_ALIGN_LEFT);
    
    return true;
}

static bool render_license_page(gui_context_t* ctx, gui_rect_t content_area) {
    /* License text area with scrolling */
    gui_rect_t license_area = {content_area.x + 20, content_area.y + 20, 
                              content_area.width - 40, content_area.height - 120};
    
    /* License title */
    gui_set_text_color(ctx, g_gui_state.theme.text_color);
    gui_set_font_size(ctx, 24);
    gui_rect_t title_area = {license_area.x, license_area.y, license_area.width, 30};
    gui_draw_text(ctx, title_area, "LimitlessOS Enterprise License Agreement", GUI_TEXT_ALIGN_CENTER);
    
    /* License text box */
    gui_rect_t text_box = {license_area.x, license_area.y + 50, 
                          license_area.width, license_area.height - 100};
    gui_fill_rect(ctx, text_box, 0xFAFAFA);
    gui_draw_rect_outline(ctx, text_box, g_gui_state.theme.text_light_color, 1);
    
    /* License text content */
    gui_set_font_size(ctx, 12);
    gui_set_text_color(ctx, g_gui_state.theme.text_color);
    gui_rect_t text_area = {text_box.x + 15, text_box.y + 15, 
                           text_box.width - 30, text_box.height - 30};
    
    const char* license_text = 
        "LimitlessOS Enterprise License Agreement\n\n"
        "This software is provided under the LimitlessOS Enterprise License.\n\n"
        "PRIVACY COMMITMENT:\n"
        "LimitlessOS is designed with privacy as a fundamental principle. All AI features "
        "are disabled by default and require explicit user consent. No telemetry or user "
        "data is collected without your explicit permission.\n\n"
        "ENTERPRISE FEATURES:\n"
        "This installation includes enterprise-grade security, advanced cryptography, "
        "application sandboxing, and complete development tools.\n\n"
        "By proceeding with this installation, you acknowledge that you have read and "
        "agree to the terms of this license agreement.\n\n"
        "For the complete license terms, visit: https://limitlessos.org/license";
    
    gui_draw_text(ctx, text_area, license_text, GUI_TEXT_ALIGN_LEFT);
    
    /* Acceptance checkbox */
    gui_rect_t checkbox_area = {license_area.x, license_area.y + license_area.height - 40, 
                               license_area.width, 30};
    render_checkbox(ctx, (gui_rect_t){checkbox_area.x, checkbox_area.y, 20, 20}, 
                    g_gui_state.config->license_accepted);
    
    gui_set_font_size(ctx, 14);
    gui_rect_t accept_text = {checkbox_area.x + 30, checkbox_area.y, 400, 20};
    gui_draw_text(ctx, accept_text, "I accept the LimitlessOS Enterprise License Agreement", 
                  GUI_TEXT_ALIGN_LEFT);
    
    return true;
}

static bool render_privacy_page(gui_context_t* ctx, gui_rect_t content_area) {
    gui_rect_t privacy_area = {content_area.x + 40, content_area.y + 30, 
                              content_area.width - 80, content_area.height - 60};
    
    /* Privacy title */
    gui_set_text_color(ctx, g_gui_state.theme.text_color);
    gui_set_font_size(ctx, 28);
    gui_rect_t title_area = {privacy_area.x, privacy_area.y, privacy_area.width, 40};
    gui_draw_text_centered(ctx, title_area, "Privacy & AI Settings");
    
    /* Privacy description */
    gui_set_font_size(ctx, 16);
    gui_set_text_color(ctx, g_gui_state.theme.text_light_color);
    gui_rect_t desc_area = {privacy_area.x, privacy_area.y + 60, privacy_area.width, 60};
    gui_draw_text(ctx, desc_area,
        "LimitlessOS prioritizes your privacy. All AI features are disabled by default. "
        "You can choose to enable specific AI capabilities with full local processing.",
        GUI_TEXT_ALIGN_CENTER);
    
    /* AI Feature Options */
    int option_y = privacy_area.y + 150;
    
    struct {
        const char* name;
        const char* description;
        bool* setting;
    } ai_options[] = {
        {"Hardware Detection AI", "Use AI to optimize hardware driver selection", &g_gui_state.config->ai_hardware_detection},
        {"Partition Optimization AI", "AI-powered disk partitioning recommendations", &g_gui_state.config->ai_partition_optimization},
        {"Security Analysis AI", "AI-enhanced security threat detection", &g_gui_state.config->ai_security_analysis},
        {"Performance Optimization AI", "AI-driven system performance tuning", &g_gui_state.config->ai_performance_optimization}
    };
    
    for (int i = 0; i < 4; i++) {
        gui_rect_t option_area = {privacy_area.x, option_y, privacy_area.width, 50};
        
        /* Option checkbox */
        render_checkbox(ctx, (gui_rect_t){option_area.x, option_area.y, 20, 20}, 
                       *ai_options[i].setting);
        
        /* Option name */
        gui_set_font_size(ctx, 16);
        gui_set_text_color(ctx, g_gui_state.theme.text_color);
        gui_rect_t name_area = {option_area.x + 30, option_area.y, 300, 20};
        gui_draw_text(ctx, name_area, ai_options[i].name, GUI_TEXT_ALIGN_LEFT);
        
        /* Option description */
        gui_set_font_size(ctx, 14);
        gui_set_text_color(ctx, g_gui_state.theme.text_light_color);
        gui_rect_t desc_area = {option_area.x + 30, option_area.y + 25, 500, 20};
        gui_draw_text(ctx, desc_area, ai_options[i].description, GUI_TEXT_ALIGN_LEFT);
        
        option_y += 60;
    }
    
    /* Privacy guarantee */
    gui_rect_t guarantee_area = {privacy_area.x, option_y + 20, privacy_area.width, 80};
    gui_fill_rounded_rect(ctx, guarantee_area, 0xE8F4FD, 8);
    gui_draw_rounded_rect_outline(ctx, guarantee_area, g_gui_state.theme.primary_color, 1, 8);
    
    gui_set_font_size(ctx, 14);
    gui_set_text_color(ctx, g_gui_state.theme.text_color);
    gui_rect_t guarantee_text = {guarantee_area.x + 20, guarantee_area.y + 15, 
                                guarantee_area.width - 40, guarantee_area.height - 30};
    gui_draw_text(ctx, guarantee_text,
        "🔒 Privacy Guarantee: All AI processing happens locally on your device. "
        "No data is sent to external servers. You maintain complete control over your data. "
        "AI features can be disabled at any time after installation.",
        GUI_TEXT_ALIGN_LEFT);
    
    return true;
}

/**
 * Navigation functions
 */
status_t gui_installer_next_page(void) {
    if (g_gui_state.current_page < PAGE_COMPLETE) {
        g_gui_state.current_page++;
        
        /* Special handling for hardware detection page */
        if (g_gui_state.current_page == PAGE_HARDWARE) {
            /* Start hardware detection in background */
            // TODO: Implement async hardware detection
        }
        
        return STATUS_SUCCESS;
    }
    return STATUS_ERROR;
}

status_t gui_installer_previous_page(void) {
    if (g_gui_state.current_page > PAGE_WELCOME) {
        g_gui_state.current_page--;
        return STATUS_SUCCESS;
    }
    return STATUS_ERROR;
}

/**
 * Helper function to render checkbox
 */
static void render_checkbox(gui_context_t* ctx, gui_rect_t area, bool checked) {
    /* Checkbox background */
    gui_fill_rounded_rect(ctx, area, 0xFFFFFF, 3);
    gui_draw_rounded_rect_outline(ctx, area, g_gui_state.theme.text_light_color, 1, 3);
    
    /* Checkmark if checked */
    if (checked) {
        gui_fill_rounded_rect(ctx, area, g_gui_state.theme.primary_color, 3);
        gui_set_text_color(ctx, 0xFFFFFF);
        gui_set_font_size(ctx, 12);
        gui_draw_text_centered(ctx, area, "✓");
    }
}

/**
 * Input handler stubs (implement based on specific needs)
 */
static bool handle_welcome_input(gui_input_event_t* event) {
    if (event->type == GUI_EVENT_BUTTON_CLICK && event->button_id == BUTTON_NEXT) {
        return gui_installer_next_page() == STATUS_SUCCESS;
    }
    return false;
}

static bool handle_license_input(gui_input_event_t* event) {
    if (event->type == GUI_EVENT_BUTTON_CLICK) {
        if (event->button_id == BUTTON_NEXT && g_gui_state.config->license_accepted) {
            return gui_installer_next_page() == STATUS_SUCCESS;
        } else if (event->button_id == BUTTON_BACK) {
            return gui_installer_previous_page() == STATUS_SUCCESS;
        }
    } else if (event->type == GUI_EVENT_CHECKBOX_CLICK) {
        g_gui_state.config->license_accepted = !g_gui_state.config->license_accepted;
    }
    return false;
}

/* Additional input handlers would be implemented similarly... */

/**
 * Cleanup GUI installer
 */
void gui_installer_cleanup(void) {
    if (g_gui_state.initialized) {
        if (g_gui_state.config) {
            free(g_gui_state.config);
            g_gui_state.config = NULL;
        }
        
        if (g_gui_state.hardware) {
            free(g_gui_state.hardware);
            g_gui_state.hardware = NULL;
        }
        
        if (g_gui_state.main_window) {
            gui_destroy_window(g_gui_state.main_window);
            g_gui_state.main_window = NULL;
        }
        
        if (g_gui_state.context) {
            gui_destroy_context(g_gui_state.context);
            g_gui_state.context = NULL;
        }
        
        graphics_enterprise_shutdown();
        
        g_gui_state.initialized = false;
        printf("[GUI] GUI installer cleaned up\n");
    }
}