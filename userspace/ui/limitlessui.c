/**
 * LimitlessUI Core Implementation - Military-Grade GUI Framework
 * 
 * The heart of the LimitlessOS user experience. Built for precision, performance,
 * and elegance. Every pixel matters, every interaction is deliberate.
 */

#include "limitlessui.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

// ============================================================================
// INTERNAL STRUCTURES AND GLOBALS
// ============================================================================

// Global UI state
static struct {
    bool initialized;
    lui_renderer_type_t renderer_type;
    const lui_theme_t* current_theme;
    
    // Window management
    lui_window_t* windows;
    lui_window_t* active_window;
    lui_window_t* focused_window;
    uint32_t window_count;
    uint32_t next_window_id;
    
    // Widget management
    uint32_t next_widget_id;
    
    // Input state
    lui_point_t cursor_position;
    uint8_t mouse_buttons;
    uint8_t key_modifiers;
    lui_widget_t* hovered_widget;
    lui_widget_t* pressed_widget;
    lui_widget_t* focused_widget;
    
    // Animation system
    lui_animation_t* animations;
    uint32_t animation_count;
    uint32_t max_animations;
    uint32_t next_animation_id;
    
    // Performance tracking
    uint64_t frame_count;
    uint64_t last_frame_time;
    uint32_t current_fps;
    uint32_t draw_calls_per_frame;
    
    // Configuration
    bool debug_mode;
    bool vsync_enabled;
    bool animations_enabled;
    float ui_scale;
    
} g_lui_state = {0};

// ============================================================================
// PREDEFINED TYPOGRAPHY SCALES
// ============================================================================

const lui_typography_t LUI_TYPOGRAPHY_DISPLAY_LARGE = {
    .family = "Inter",
    .size = 96.0f,
    .weight = LUI_FONT_WEIGHT_THIN,
    .style = LUI_FONT_STYLE_NORMAL,
    .line_height = 1.1f,
    .letter_spacing = -1.5f,
    .align = LUI_TEXT_ALIGN_LEFT,
    .color = LUI_COLOR_PLATINUM
};

const lui_typography_t LUI_TYPOGRAPHY_DISPLAY_MEDIUM = {
    .family = "Inter",
    .size = 60.0f,
    .weight = LUI_FONT_WEIGHT_LIGHT,
    .style = LUI_FONT_STYLE_NORMAL,
    .line_height = 1.15f,
    .letter_spacing = -0.5f,
    .align = LUI_TEXT_ALIGN_LEFT,
    .color = LUI_COLOR_PLATINUM
};

const lui_typography_t LUI_TYPOGRAPHY_DISPLAY_SMALL = {
    .family = "Inter",
    .size = 48.0f,
    .weight = LUI_FONT_WEIGHT_REGULAR,
    .style = LUI_FONT_STYLE_NORMAL,
    .line_height = 1.2f,
    .letter_spacing = 0.0f,
    .align = LUI_TEXT_ALIGN_LEFT,
    .color = LUI_COLOR_PLATINUM
};

const lui_typography_t LUI_TYPOGRAPHY_HEADLINE_LARGE = {
    .family = "Inter",
    .size = 32.0f,
    .weight = LUI_FONT_WEIGHT_REGULAR,
    .style = LUI_FONT_STYLE_NORMAL,
    .line_height = 1.25f,
    .letter_spacing = 0.25f,
    .align = LUI_TEXT_ALIGN_LEFT,
    .color = LUI_COLOR_PLATINUM
};

const lui_typography_t LUI_TYPOGRAPHY_BODY_MEDIUM = {
    .family = "Inter",
    .size = 14.0f,
    .weight = LUI_FONT_WEIGHT_REGULAR,
    .style = LUI_FONT_STYLE_NORMAL,
    .line_height = 1.4f,
    .letter_spacing = 0.25f,
    .align = LUI_TEXT_ALIGN_LEFT,
    .color = LUI_COLOR_TITANIUM
};

const lui_typography_t LUI_TYPOGRAPHY_LABEL_MEDIUM = {
    .family = "Inter",
    .size = 12.0f,
    .weight = LUI_FONT_WEIGHT_MEDIUM,
    .style = LUI_FONT_STYLE_NORMAL,
    .line_height = 1.3f,
    .letter_spacing = 0.5f,
    .align = LUI_TEXT_ALIGN_LEFT,
    .color = LUI_COLOR_TITANIUM
};

// ============================================================================
// PREDEFINED THEMES - MILITARY DESIGN SYSTEM
// ============================================================================

const lui_theme_t LUI_THEME_MILITARY_DARK = {
    .name = "Military Dark",
    .description = "Default dark military theme with tactical precision",
    
    // Color palette
    .primary = LUI_COLOR_TACTICAL_BLUE,
    .secondary = LUI_COLOR_STEEL_GRAY,
    .surface = LUI_COLOR_GRAPHITE,
    .background = LUI_COLOR_TACTICAL_BLACK,
    .error = LUI_COLOR_CRITICAL_RED,
    .warning = LUI_COLOR_ALERT_AMBER,
    .success = LUI_COLOR_MISSION_GREEN,
    .info = LUI_COLOR_SECURE_CYAN,
    
    // Text colors
    .on_primary = LUI_COLOR_ARCTIC_WHITE,
    .on_secondary = LUI_COLOR_PLATINUM,
    .on_surface = LUI_COLOR_PLATINUM,
    .on_background = LUI_COLOR_PLATINUM,
    .on_error = LUI_COLOR_ARCTIC_WHITE,
    
    // Spacing and sizing
    .spacing_unit = 8.0f,
    .border_radius_small = 4.0f,
    .border_radius_medium = 8.0f,
    .border_radius_large = 16.0f,
    
    // Animation timing
    .animation_fast = 100,
    .animation_normal = 200,
    .animation_slow = 300,
    
    // Shadows (material design elevation)
    .shadows = {
        {{0x00, 0x00, 0x00, 0x00}, 0, 0, 0, 0},     // Level 0
        {{0x00, 0x00, 0x00, 0x33}, 0, 1, 3, 0},     // Level 1
        {{0x00, 0x00, 0x00, 0x40}, 0, 2, 6, 0},     // Level 2
        {{0x00, 0x00, 0x00, 0x4D}, 0, 4, 12, 0},    // Level 3
        {{0x00, 0x00, 0x00, 0x59}, 0, 6, 18, 0},    // Level 4
        {{0x00, 0x00, 0x00, 0x66}, 0, 8, 24, 0},    // Level 5
        {{0x00, 0x00, 0x00, 0x73}, 0, 12, 36, 0},   // Level 6
        {{0x00, 0x00, 0x00, 0x80}, 0, 16, 48, 0}    // Level 7
    }
};

const lui_theme_t LUI_THEME_MILITARY_LIGHT = {
    .name = "Military Light",
    .description = "Light military theme for bright environments",
    
    // Color palette (inverted for light theme)
    .primary = LUI_COLOR_TACTICAL_BLUE,
    .secondary = LUI_COLOR_TITANIUM,
    .surface = LUI_COLOR_PLATINUM,
    .background = LUI_COLOR_ARCTIC_WHITE,
    .error = LUI_COLOR_CRITICAL_RED,
    .warning = LUI_COLOR_ALERT_AMBER,
    .success = LUI_COLOR_MISSION_GREEN,
    .info = LUI_COLOR_SECURE_CYAN,
    
    // Text colors
    .on_primary = LUI_COLOR_ARCTIC_WHITE,
    .on_secondary = LUI_COLOR_TACTICAL_BLACK,
    .on_surface = LUI_COLOR_TACTICAL_BLACK,
    .on_background = LUI_COLOR_TACTICAL_BLACK,
    .on_error = LUI_COLOR_ARCTIC_WHITE,
    
    // Same spacing and timing as dark theme
    .spacing_unit = 8.0f,
    .border_radius_small = 4.0f,
    .border_radius_medium = 8.0f,
    .border_radius_large = 16.0f,
    .animation_fast = 100,
    .animation_normal = 200,
    .animation_slow = 300,
    
    // Lighter shadows for light theme
    .shadows = {
        {{0x00, 0x00, 0x00, 0x00}, 0, 0, 0, 0},     // Level 0
        {{0x00, 0x00, 0x00, 0x1A}, 0, 1, 3, 0},     // Level 1
        {{0x00, 0x00, 0x00, 0x26}, 0, 2, 6, 0},     // Level 2
        {{0x00, 0x00, 0x00, 0x33}, 0, 4, 12, 0},    // Level 3
        {{0x00, 0x00, 0x00, 0x40}, 0, 6, 18, 0},    // Level 4
        {{0x00, 0x00, 0x00, 0x4D}, 0, 8, 24, 0},    // Level 5
        {{0x00, 0x00, 0x00, 0x59}, 0, 12, 36, 0},   // Level 6
        {{0x00, 0x00, 0x00, 0x66}, 0, 16, 48, 0}    // Level 7
    }
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

static uint64_t lui_get_time_us(void) {
    // TODO: Integrate with LimitlessOS high-precision timer
    return g_lui_state.frame_count * 16667; // Assume 60 FPS for now
}

static float lui_ease_function(float t, lui_easing_t easing) {
    switch (easing) {
        case LUI_EASING_LINEAR:
            return t;
        case LUI_EASING_EASE_IN:
            return t * t;
        case LUI_EASING_EASE_OUT:
            return 1.0f - (1.0f - t) * (1.0f - t);
        case LUI_EASING_EASE_IN_OUT:
            return t < 0.5f ? 2.0f * t * t : 1.0f - 2.0f * (1.0f - t) * (1.0f - t);
        case LUI_EASING_EASE_IN_BACK: {
            const float c1 = 1.70158f;
            const float c3 = c1 + 1.0f;
            return c3 * t * t * t - c1 * t * t;
        }
        case LUI_EASING_EASE_OUT_BACK: {
            const float c1 = 1.70158f;
            const float c3 = c1 + 1.0f;
            return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
        }
        case LUI_EASING_EASE_IN_OUT_BACK: {
            const float c1 = 1.70158f;
            const float c2 = c1 * 1.525f;
            return t < 0.5f
                ? (powf(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f
                : (powf(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
        }
        default:
            return t;
    }
}

// ============================================================================
// CORE API IMPLEMENTATION
// ============================================================================

bool lui_init(lui_renderer_type_t renderer) {
    if (g_lui_state.initialized) {
        return false;
    }
    
    printf("[LimitlessUI] Initializing military-grade GUI framework v%s\n", 
           LIMITLESS_UI_VERSION_STRING);
    
    // Initialize global state
    memset(&g_lui_state, 0, sizeof(g_lui_state));
    g_lui_state.initialized = true;
    g_lui_state.renderer_type = renderer;
    g_lui_state.current_theme = &LUI_THEME_MILITARY_DARK;
    g_lui_state.next_window_id = 1;
    g_lui_state.next_widget_id = 1;
    g_lui_state.next_animation_id = 1;
    g_lui_state.ui_scale = 1.0f;
    g_lui_state.vsync_enabled = true;
    g_lui_state.animations_enabled = true;
    
    // Allocate animation system
    g_lui_state.max_animations = LUI_MAX_ANIMATIONS;
    g_lui_state.animations = calloc(g_lui_state.max_animations, sizeof(lui_animation_t));
    if (!g_lui_state.animations) {
        printf("[LimitlessUI] ERROR: Failed to allocate animation system\n");
        g_lui_state.initialized = false;
        return false;
    }
    
    // Initialize renderer backend
    switch (renderer) {
        case LUI_RENDERER_SOFTWARE:
            printf("[LimitlessUI] Using software renderer (fallback)\n");
            break;
        case LUI_RENDERER_OPENGL:
            printf("[LimitlessUI] Using OpenGL hardware acceleration\n");
            break;
        case LUI_RENDERER_VULKAN:
            printf("[LimitlessUI] Using Vulkan high-performance rendering\n");
            break;
        default:
            printf("[LimitlessUI] WARNING: Unknown renderer, falling back to software\n");
            g_lui_state.renderer_type = LUI_RENDERER_SOFTWARE;
            break;
    }
    
    printf("[LimitlessUI] Framework initialized successfully\n");
    printf("[LimitlessUI] Theme: %s\n", g_lui_state.current_theme->name);
    printf("[LimitlessUI] Max windows: %d, Max widgets per window: %d\n", 
           LUI_MAX_WINDOWS, LUI_MAX_WIDGETS_PER_WINDOW);
    
    return true;
}

void lui_shutdown(void) {
    if (!g_lui_state.initialized) {
        return;
    }
    
    printf("[LimitlessUI] Shutting down GUI framework\n");
    
    // Destroy all windows
    lui_window_t* window = g_lui_state.windows;
    while (window) {
        lui_window_t* next = window->next;
        lui_destroy_window(window);
        window = next;
    }
    
    // Free animation system
    if (g_lui_state.animations) {
        free(g_lui_state.animations);
        g_lui_state.animations = NULL;
    }
    
    // Clear state
    memset(&g_lui_state, 0, sizeof(g_lui_state));
    
    printf("[LimitlessUI] Shutdown complete\n");
}

void lui_set_theme(const lui_theme_t* theme) {
    if (!g_lui_state.initialized || !theme) {
        return;
    }
    
    g_lui_state.current_theme = theme;
    
    // Trigger repaint of all windows
    lui_window_t* window = g_lui_state.windows;
    while (window) {
        window->needs_redraw = true;
        window = window->next;
    }
    
    printf("[LimitlessUI] Theme changed to: %s\n", theme->name);
}

const lui_theme_t* lui_get_theme(void) {
    return g_lui_state.current_theme;
}

// ============================================================================
// WINDOW MANAGEMENT
// ============================================================================

lui_window_t* lui_create_window(const char* title, lui_window_type_t type, 
                               float x, float y, float width, float height) {
    if (!g_lui_state.initialized) {
        printf("[LimitlessUI] ERROR: Framework not initialized\n");
        return NULL;
    }
    
    if (g_lui_state.window_count >= LUI_MAX_WINDOWS) {
        printf("[LimitlessUI] ERROR: Maximum windows exceeded\n");
        return NULL;
    }
    
    // Allocate window structure
    lui_window_t* window = calloc(1, sizeof(lui_window_t));
    if (!window) {
        printf("[LimitlessUI] ERROR: Failed to allocate window\n");
        return NULL;
    }
    
    // Initialize window properties
    window->id = g_lui_state.next_window_id++;
    strncpy(window->title, title ? title : "Untitled", sizeof(window->title) - 1);
    window->type = type;
    window->state = LUI_WINDOW_STATE_HIDDEN;
    
    // Set default window style based on type
    switch (type) {
        case LUI_WINDOW_NORMAL:
            window->style.resizable = true;
            window->style.movable = true;
            window->style.closable = true;
            window->style.minimizable = true;
            window->style.maximizable = true;
            window->style.decorated = true;
            window->style.show_in_taskbar = true;
            break;
        case LUI_WINDOW_DIALOG:
            window->style.resizable = false;
            window->style.movable = true;
            window->style.closable = true;
            window->style.decorated = true;
            window->style.always_on_top = true;
            break;
        case LUI_WINDOW_POPUP:
            window->style.decorated = false;
            window->style.always_on_top = true;
            break;
        case LUI_WINDOW_DESKTOP:
            window->style.decorated = false;
            window->style.resizable = false;
            window->style.movable = false;
            window->style.closable = false;
            break;
        default:
            window->style.decorated = true;
            break;
    }
    
    // Set geometry
    window->frame = lui_rect_make(x, y, width, height);
    window->content_area = window->frame;
    if (window->style.decorated && type != LUI_WINDOW_DESKTOP) {
        // Account for title bar and borders
        window->content_area.y += 32;  // Title bar height
        window->content_area.height -= 32;
        window->content_area.x += 1;
        window->content_area.y += 1;
        window->content_area.width -= 2;
        window->content_area.height -= 1;
    }
    
    window->dpi_scale = g_lui_state.ui_scale;
    window->needs_redraw = true;
    window->vsync_enabled = g_lui_state.vsync_enabled;
    
    // Allocate framebuffer
    window->fb_width = (uint32_t)(width * window->dpi_scale);
    window->fb_height = (uint32_t)(height * window->dpi_scale);
    window->framebuffer = calloc(window->fb_width * window->fb_height, sizeof(uint32_t));
    if (!window->framebuffer) {
        printf("[LimitlessUI] ERROR: Failed to allocate framebuffer\n");
        free(window);
        return NULL;
    }
    
    // Create root widget
    window->root_widget = lui_create_widget(LUI_WIDGET_CONTAINER, NULL);
    if (!window->root_widget) {
        printf("[LimitlessUI] ERROR: Failed to create root widget\n");
        free(window->framebuffer);
        free(window);
        return NULL;
    }
    
    // Configure root widget
    window->root_widget->bounds = window->content_area;
    window->root_widget->background_color = g_lui_state.current_theme->background;
    strcpy(window->root_widget->name, "root");
    
    // Add to window list
    if (g_lui_state.windows) {
        g_lui_state.windows->prev = window;
    }
    window->next = g_lui_state.windows;
    g_lui_state.windows = window;
    g_lui_state.window_count++;
    
    printf("[LimitlessUI] Created window '%s' (ID: %u) - %.0fx%.0f at (%.0f,%.0f)\n", 
           window->title, window->id, width, height, x, y);
    
    return window;
}

void lui_destroy_window(lui_window_t* window) {
    if (!window) return;
    
    printf("[LimitlessUI] Destroying window '%s' (ID: %u)\n", window->title, window->id);
    
    // Remove from window list
    if (window->prev) {
        window->prev->next = window->next;
    } else {
        g_lui_state.windows = window->next;
    }
    if (window->next) {
        window->next->prev = window->prev;
    }
    
    // Update global state
    if (g_lui_state.active_window == window) {
        g_lui_state.active_window = g_lui_state.windows;
    }
    if (g_lui_state.focused_window == window) {
        g_lui_state.focused_window = g_lui_state.windows;
    }
    
    // Destroy root widget (which will recursively destroy children)
    if (window->root_widget) {
        lui_destroy_widget(window->root_widget);
    }
    
    // Free framebuffer
    if (window->framebuffer) {
        free(window->framebuffer);
    }
    
    // Free window structure
    free(window);
    g_lui_state.window_count--;
}

void lui_show_window(lui_window_t* window) {
    if (!window) return;
    
    window->state = LUI_WINDOW_STATE_NORMAL;
    window->needs_redraw = true;
    g_lui_state.active_window = window;
    
    printf("[LimitlessUI] Showing window '%s'\n", window->title);
}

void lui_hide_window(lui_window_t* window) {
    if (!window) return;
    
    window->state = LUI_WINDOW_STATE_HIDDEN;
    
    if (g_lui_state.active_window == window) {
        g_lui_state.active_window = NULL;
    }
    
    printf("[LimitlessUI] Hiding window '%s'\n", window->title);
}

// ============================================================================
// WIDGET MANAGEMENT
// ============================================================================

lui_widget_t* lui_create_widget(lui_widget_type_t type, lui_widget_t* parent) {
    if (!g_lui_state.initialized) {
        return NULL;
    }
    
    // Allocate widget structure
    lui_widget_t* widget = calloc(1, sizeof(lui_widget_t));
    if (!widget) {
        printf("[LimitlessUI] ERROR: Failed to allocate widget\n");
        return NULL;
    }
    
    // Initialize widget properties
    widget->id = g_lui_state.next_widget_id++;
    widget->type = type;
    widget->visible = true;
    widget->enabled = true;
    widget->opacity = 1.0f;
    widget->needs_layout = true;
    widget->needs_repaint = true;
    
    // Set default colors from theme
    const lui_theme_t* theme = g_lui_state.current_theme;
    widget->background_color = theme->surface;
    widget->border_color = theme->secondary;
    widget->typography = LUI_TYPOGRAPHY_BODY_MEDIUM;
    
    // Set type-specific defaults
    switch (type) {
        case LUI_WIDGET_BUTTON:
            widget->background_color = theme->primary;
            widget->typography.color = theme->on_primary;
            widget->corner_radius = theme->border_radius_medium;
            widget->focusable = true;
            break;
        case LUI_WIDGET_TEXT_INPUT:
            widget->background_color = theme->surface;
            widget->border_color = theme->secondary;
            widget->border_width = 1.0f;
            widget->corner_radius = theme->border_radius_small;
            widget->focusable = true;
            break;
        case LUI_WIDGET_LABEL:
            widget->background_color = (lui_color_t){0, 0, 0, 0}; // Transparent
            widget->typography.color = theme->on_surface;
            break;
        default:
            break;
    }
    
    // Add to parent if specified
    if (parent) {
        lui_add_child(parent, widget);
    }
    
    return widget;
}

void lui_destroy_widget(lui_widget_t* widget) {
    if (!widget) return;
    
    // Recursively destroy children
    lui_widget_t* child = widget->first_child;
    while (child) {
        lui_widget_t* next = child->next_sibling;
        lui_destroy_widget(child);
        child = next;
    }
    
    // Remove from parent
    if (widget->parent) {
        lui_remove_child(widget->parent, widget);
    }
    
    // Free widget-specific data
    if (widget->widget_data) {
        free(widget->widget_data);
    }
    
    // Clear global references
    if (g_lui_state.hovered_widget == widget) {
        g_lui_state.hovered_widget = NULL;
    }
    if (g_lui_state.pressed_widget == widget) {
        g_lui_state.pressed_widget = NULL;
    }
    if (g_lui_state.focused_widget == widget) {
        g_lui_state.focused_widget = NULL;
    }
    
    free(widget);
}

void lui_add_child(lui_widget_t* parent, lui_widget_t* child) {
    if (!parent || !child || child->parent == parent) {
        return;
    }
    
    // Remove from current parent
    if (child->parent) {
        lui_remove_child(child->parent, child);
    }
    
    // Set parent relationship
    child->parent = parent;
    child->next_sibling = NULL;
    child->prev_sibling = parent->last_child;
    
    if (parent->last_child) {
        parent->last_child->next_sibling = child;
    } else {
        parent->first_child = child;
    }
    parent->last_child = child;
    parent->child_count++;
    
    // Invalidate layout
    parent->needs_layout = true;
}

void lui_remove_child(lui_widget_t* parent, lui_widget_t* child) {
    if (!parent || !child || child->parent != parent) {
        return;
    }
    
    // Update sibling links
    if (child->prev_sibling) {
        child->prev_sibling->next_sibling = child->next_sibling;
    } else {
        parent->first_child = child->next_sibling;
    }
    
    if (child->next_sibling) {
        child->next_sibling->prev_sibling = child->prev_sibling;
    } else {
        parent->last_child = child->prev_sibling;
    }
    
    // Clear relationships
    child->parent = NULL;
    child->next_sibling = NULL;
    child->prev_sibling = NULL;
    parent->child_count--;
    
    // Invalidate layout
    parent->needs_layout = true;
}

// ============================================================================
// UTILITY FUNCTIONS IMPLEMENTATION
// ============================================================================

lui_color_t lui_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (lui_color_t){r, g, b, a};
}

lui_color_t lui_color_lerp(lui_color_t from, lui_color_t to, float t) {
    t = lui_clamp(t, 0.0f, 1.0f);
    return (lui_color_t){
        (uint8_t)(from.r + (to.r - from.r) * t),
        (uint8_t)(from.g + (to.g - from.g) * t),
        (uint8_t)(from.b + (to.b - from.b) * t),
        (uint8_t)(from.a + (to.a - from.a) * t)
    };
}

lui_rect_t lui_rect_make(float x, float y, float width, float height) {
    return (lui_rect_t){x, y, width, height};
}

bool lui_rect_contains_point(const lui_rect_t* rect, lui_point_t point) {
    return point.x >= rect->x && point.x < rect->x + rect->width &&
           point.y >= rect->y && point.y < rect->y + rect->height;
}

bool lui_rect_intersects(const lui_rect_t* a, const lui_rect_t* b) {
    return !(a->x >= b->x + b->width || b->x >= a->x + a->width ||
             a->y >= b->y + b->height || b->y >= a->y + a->height);
}

float lui_lerp(float from, float to, float t) {
    return from + (to - from) * t;
}

float lui_clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// ============================================================================
// CONFIGURATION SYSTEM
// ============================================================================

void lui_set_config_bool(const char* key, bool value) {
    if (!key) return;
    
    if (strcmp(key, "debug_mode") == 0) {
        g_lui_state.debug_mode = value;
    } else if (strcmp(key, "vsync_enabled") == 0) {
        g_lui_state.vsync_enabled = value;
    } else if (strcmp(key, "animations_enabled") == 0) {
        g_lui_state.animations_enabled = value;
    }
}

void lui_set_config_float(const char* key, float value) {
    if (!key) return;
    
    if (strcmp(key, "ui_scale") == 0) {
        g_lui_state.ui_scale = lui_clamp(value, 0.5f, 4.0f);
    }
}

bool lui_get_config_bool(const char* key, bool default_value) {
    if (!key) return default_value;
    
    if (strcmp(key, "debug_mode") == 0) {
        return g_lui_state.debug_mode;
    } else if (strcmp(key, "vsync_enabled") == 0) {
        return g_lui_state.vsync_enabled;
    } else if (strcmp(key, "animations_enabled") == 0) {
        return g_lui_state.animations_enabled;
    }
    
    return default_value;
}

float lui_get_config_float(const char* key, float default_value) {
    if (!key) return default_value;
    
    if (strcmp(key, "ui_scale") == 0) {
        return g_lui_state.ui_scale;
    }
    
    return default_value;
}

void lui_enable_debug_mode(bool enabled) {
    g_lui_state.debug_mode = enabled;
    printf("[LimitlessUI] Debug mode %s\n", enabled ? "enabled" : "disabled");
}

void lui_print_widget_tree(lui_widget_t* root, int indent) {
    if (!root) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    printf("├─ %s (ID: %u, Type: %d, Children: %u) [%.1f,%.1f %.1fx%.1f]\n",
           root->name[0] ? root->name : "unnamed",
           root->id, root->type, root->child_count,
           root->bounds.x, root->bounds.y, root->bounds.width, root->bounds.height);
    
    lui_widget_t* child = root->first_child;
    while (child) {
        lui_print_widget_tree(child, indent + 1);
        child = child->next_sibling;
    }
}

void lui_get_performance_stats(uint32_t* fps, uint32_t* frame_time_us, 
                              uint32_t* draw_calls) {
    if (fps) *fps = g_lui_state.current_fps;
    if (frame_time_us) *frame_time_us = (uint32_t)(g_lui_state.last_frame_time);
    if (draw_calls) *draw_calls = g_lui_state.draw_calls_per_frame;
}