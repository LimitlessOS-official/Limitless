/**
 * LimitlessUI - Next-Generation GUI Framework for LimitlessOS
 * 
 * A hybrid declarative + immediate-mode GUI framework designed for military-grade precision
 * with modern minimalist aesthetics. Combines structural elegance with ultra-fast performance.
 * 
 * Core Philosophy:
 * - Zero clutter, maximum clarity
 * - Immediate responsiveness with declarative flexibility
 * - AI-augmented interface intelligence
 * - Configurable security and privacy controls
 * - Universal accessibility and internationalization
 */

#ifndef LIMITLESS_UI_H
#define LIMITLESS_UI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Version and Build Information
#define LIMITLESS_UI_VERSION_MAJOR    1
#define LIMITLESS_UI_VERSION_MINOR    0
#define LIMITLESS_UI_VERSION_PATCH    0
#define LIMITLESS_UI_VERSION_STRING   "1.0.0-Military"
#define LIMITLESS_UI_BUILD_DATE       __DATE__ " " __TIME__

// Core System Limits
#define LUI_MAX_WINDOWS              4096
#define LUI_MAX_WIDGETS_PER_WINDOW   8192
#define LUI_MAX_THEMES               64
#define LUI_MAX_FONTS                256
#define LUI_MAX_TEXTURES            2048
#define LUI_MAX_SHADERS             128
#define LUI_MAX_ANIMATIONS          1024
#define LUI_MAX_EVENT_HANDLERS      512

// Military-Grade Design Constants
#define LUI_GOLDEN_RATIO            1.618f
#define LUI_MIN_TOUCH_TARGET        44    // Minimum 44px for accessibility
#define LUI_GRID_BASELINE           8     // 8px baseline grid
#define LUI_ANIMATION_DURATION      200   // Default 200ms animations
#define LUI_DEBOUNCE_THRESHOLD      16    // 16ms input debouncing

// ============================================================================
// COLOR SYSTEM - Military Minimalist Palette
// ============================================================================

typedef struct {
    uint8_t r, g, b, a;
} lui_color_t;

// Primary Military Palette
#define LUI_COLOR_TACTICAL_BLACK    ((lui_color_t){0x0A, 0x0A, 0x0A, 0xFF})
#define LUI_COLOR_GRAPHITE          ((lui_color_t){0x1C, 0x1C, 0x1E, 0xFF})
#define LUI_COLOR_STEEL_GRAY        ((lui_color_t){0x2C, 0x2C, 0x2E, 0xFF})
#define LUI_COLOR_SLATE             ((lui_color_t){0x48, 0x48, 0x4A, 0xFF})
#define LUI_COLOR_TITANIUM          ((lui_color_t){0x8E, 0x8E, 0x93, 0xFF})
#define LUI_COLOR_PLATINUM          ((lui_color_t){0xF2, 0xF2, 0xF7, 0xFF})
#define LUI_COLOR_ARCTIC_WHITE      ((lui_color_t){0xFF, 0xFF, 0xFF, 0xFF})

// Accent Colors - High Contrast, Tactical
#define LUI_COLOR_TACTICAL_BLUE     ((lui_color_t){0x00, 0x7A, 0xFF, 0xFF})
#define LUI_COLOR_MISSION_GREEN     ((lui_color_t){0x32, 0xD0, 0x74, 0xFF})
#define LUI_COLOR_ALERT_AMBER       ((lui_color_t){0xFF, 0xC5, 0x00, 0xFF})
#define LUI_COLOR_CRITICAL_RED      ((lui_color_t){0xFF, 0x45, 0x58, 0xFF})
#define LUI_COLOR_INTEL_PURPLE      ((lui_color_t){0xAF, 0x52, 0xDE, 0xFF})
#define LUI_COLOR_SECURE_CYAN       ((lui_color_t){0x00, 0xC5, 0xF7, 0xFF})

// Transparency Levels
#define LUI_ALPHA_TRANSPARENT       0x00
#define LUI_ALPHA_GHOST             0x19  // 10%
#define LUI_ALPHA_SUBTLE            0x33  // 20%
#define LUI_ALPHA_SOFT              0x66  // 40%
#define LUI_ALPHA_MEDIUM            0x99  // 60%
#define LUI_ALPHA_STRONG            0xCC  // 80%
#define LUI_ALPHA_OPAQUE            0xFF  // 100%

// ============================================================================
// GEOMETRY AND LAYOUT SYSTEM
// ============================================================================

typedef struct {
    float x, y;
} lui_point_t;

typedef struct {
    float x, y, width, height;
} lui_rect_t;

typedef struct {
    float top, right, bottom, left;
} lui_insets_t;

typedef struct {
    float width, height;
} lui_size_t;

typedef enum {
    LUI_ALIGN_START = 0,
    LUI_ALIGN_CENTER,
    LUI_ALIGN_END,
    LUI_ALIGN_STRETCH,
    LUI_ALIGN_BASELINE
} lui_alignment_t;

typedef enum {
    LUI_DIRECTION_ROW = 0,
    LUI_DIRECTION_COLUMN,
    LUI_DIRECTION_ROW_REVERSE,
    LUI_DIRECTION_COLUMN_REVERSE
} lui_direction_t;

typedef enum {
    LUI_WRAP_NONE = 0,
    LUI_WRAP_WRAP,
    LUI_WRAP_REVERSE
} lui_wrap_t;

typedef enum {
    LUI_JUSTIFY_START = 0,
    LUI_JUSTIFY_CENTER,
    LUI_JUSTIFY_END,
    LUI_JUSTIFY_SPACE_BETWEEN,
    LUI_JUSTIFY_SPACE_AROUND,
    LUI_JUSTIFY_SPACE_EVENLY
} lui_justify_t;

// Flexbox-inspired Layout System
typedef struct {
    lui_direction_t direction;
    lui_wrap_t wrap;
    lui_justify_t justify_content;
    lui_alignment_t align_items;
    lui_alignment_t align_content;
    lui_insets_t padding;
    lui_insets_t margin;
    float gap;
} lui_layout_t;

// ============================================================================
// TYPOGRAPHY SYSTEM
// ============================================================================

typedef enum {
    LUI_FONT_WEIGHT_THIN = 100,
    LUI_FONT_WEIGHT_LIGHT = 300,
    LUI_FONT_WEIGHT_REGULAR = 400,
    LUI_FONT_WEIGHT_MEDIUM = 500,
    LUI_FONT_WEIGHT_SEMIBOLD = 600,
    LUI_FONT_WEIGHT_BOLD = 700,
    LUI_FONT_WEIGHT_HEAVY = 900
} lui_font_weight_t;

typedef enum {
    LUI_FONT_STYLE_NORMAL = 0,
    LUI_FONT_STYLE_ITALIC,
    LUI_FONT_STYLE_OBLIQUE
} lui_font_style_t;

typedef enum {
    LUI_TEXT_ALIGN_LEFT = 0,
    LUI_TEXT_ALIGN_CENTER,
    LUI_TEXT_ALIGN_RIGHT,
    LUI_TEXT_ALIGN_JUSTIFY
} lui_text_align_t;

typedef struct {
    char family[64];              // Font family name
    float size;                   // Font size in points
    lui_font_weight_t weight;     // Font weight
    lui_font_style_t style;       // Font style
    float line_height;            // Line height multiplier
    float letter_spacing;         // Letter spacing in pixels
    lui_text_align_t align;       // Text alignment
    lui_color_t color;            // Text color
} lui_typography_t;

// Predefined Typography Scales (Military Design System)
extern const lui_typography_t LUI_TYPOGRAPHY_DISPLAY_LARGE;    // 96px - Hero headers
extern const lui_typography_t LUI_TYPOGRAPHY_DISPLAY_MEDIUM;   // 60px - Large titles
extern const lui_typography_t LUI_TYPOGRAPHY_DISPLAY_SMALL;    // 48px - Section titles
extern const lui_typography_t LUI_TYPOGRAPHY_HEADLINE_LARGE;   // 32px - Page titles
extern const lui_typography_t LUI_TYPOGRAPHY_HEADLINE_MEDIUM;  // 28px - Card titles
extern const lui_typography_t LUI_TYPOGRAPHY_HEADLINE_SMALL;   // 24px - Subsection titles
extern const lui_typography_t LUI_TYPOGRAPHY_TITLE_LARGE;      // 22px - Prominent text
extern const lui_typography_t LUI_TYPOGRAPHY_TITLE_MEDIUM;     // 16px - Medium emphasis
extern const lui_typography_t LUI_TYPOGRAPHY_TITLE_SMALL;      // 14px - Small emphasis
extern const lui_typography_t LUI_TYPOGRAPHY_BODY_LARGE;       // 16px - Body text
extern const lui_typography_t LUI_TYPOGRAPHY_BODY_MEDIUM;      // 14px - Default text
extern const lui_typography_t LUI_TYPOGRAPHY_BODY_SMALL;       // 12px - Caption text
extern const lui_typography_t LUI_TYPOGRAPHY_LABEL_LARGE;      // 14px - Button labels
extern const lui_typography_t LUI_TYPOGRAPHY_LABEL_MEDIUM;     // 12px - Form labels
extern const lui_typography_t LUI_TYPOGRAPHY_LABEL_SMALL;      // 11px - Overline text

// ============================================================================
// ANIMATION AND MOTION SYSTEM
// ============================================================================

typedef enum {
    LUI_EASING_LINEAR = 0,
    LUI_EASING_EASE_IN,
    LUI_EASING_EASE_OUT,
    LUI_EASING_EASE_IN_OUT,
    LUI_EASING_EASE_IN_BACK,
    LUI_EASING_EASE_OUT_BACK,
    LUI_EASING_EASE_IN_OUT_BACK,
    LUI_EASING_EASE_IN_ELASTIC,
    LUI_EASING_EASE_OUT_ELASTIC,
    LUI_EASING_EASE_IN_OUT_ELASTIC,
    LUI_EASING_CUSTOM
} lui_easing_t;

typedef struct {
    float from;
    float to;
    uint32_t duration_ms;
    uint32_t delay_ms;
    lui_easing_t easing;
    void (*on_update)(float value, void* user_data);
    void (*on_complete)(void* user_data);
    void* user_data;
} lui_animation_t;

// Military Motion Design Principles
#define LUI_MOTION_INSTANT          0     // Immediate state changes
#define LUI_MOTION_SNAP             50    // Quick micro-interactions
#define LUI_MOTION_FAST             100   // Fast state transitions
#define LUI_MOTION_NORMAL           200   // Standard interactions
#define LUI_MOTION_SMOOTH           300   // Smooth complex transitions
#define LUI_MOTION_DELIBERATE       500   // Intentional, important changes

// ============================================================================
// INPUT AND INTERACTION SYSTEM
// ============================================================================

typedef enum {
    LUI_INPUT_MOUSE_MOVE = 0,
    LUI_INPUT_MOUSE_DOWN,
    LUI_INPUT_MOUSE_UP,
    LUI_INPUT_MOUSE_WHEEL,
    LUI_INPUT_KEY_DOWN,
    LUI_INPUT_KEY_UP,
    LUI_INPUT_TEXT_INPUT,
    LUI_INPUT_TOUCH_START,
    LUI_INPUT_TOUCH_MOVE,
    LUI_INPUT_TOUCH_END,
    LUI_INPUT_GESTURE_TAP,
    LUI_INPUT_GESTURE_DOUBLE_TAP,
    LUI_INPUT_GESTURE_LONG_PRESS,
    LUI_INPUT_GESTURE_SWIPE,
    LUI_INPUT_GESTURE_PINCH,
    LUI_INPUT_GESTURE_ROTATE
} lui_input_type_t;

typedef enum {
    LUI_MOUSE_LEFT = 0,
    LUI_MOUSE_RIGHT,
    LUI_MOUSE_MIDDLE,
    LUI_MOUSE_X1,
    LUI_MOUSE_X2
} lui_mouse_button_t;

typedef enum {
    LUI_KEY_MOD_NONE = 0x00,
    LUI_KEY_MOD_SHIFT = 0x01,
    LUI_KEY_MOD_CTRL = 0x02,
    LUI_KEY_MOD_ALT = 0x04,
    LUI_KEY_MOD_SUPER = 0x08,
    LUI_KEY_MOD_CAPS = 0x10,
    LUI_KEY_MOD_NUM = 0x20
} lui_key_modifiers_t;

typedef struct {
    lui_input_type_t type;
    uint32_t timestamp;
    
    union {
        struct {
            float x, y;
            lui_mouse_button_t button;
            uint8_t modifiers;
        } mouse;
        
        struct {
            uint32_t keycode;
            uint32_t scancode;
            uint8_t modifiers;
            bool repeat;
        } key;
        
        struct {
            char text[8];  // UTF-8 encoded
            uint32_t length;
        } text;
        
        struct {
            uint32_t id;
            float x, y;
            float pressure;
        } touch;
        
        struct {
            lui_point_t center;
            float scale;
            float rotation;
            float velocity_x, velocity_y;
        } gesture;
    };
} lui_input_event_t;

// ============================================================================
// WIDGET SYSTEM
// ============================================================================

typedef enum {
    LUI_WIDGET_CONTAINER = 0,
    LUI_WIDGET_BUTTON,
    LUI_WIDGET_LABEL,
    LUI_WIDGET_TEXT_INPUT,
    LUI_WIDGET_IMAGE,
    LUI_WIDGET_SLIDER,
    LUI_WIDGET_CHECKBOX,
    LUI_WIDGET_RADIO_BUTTON,
    LUI_WIDGET_DROPDOWN,
    LUI_WIDGET_LIST_VIEW,
    LUI_WIDGET_SCROLL_VIEW,
    LUI_WIDGET_SPLIT_VIEW,
    LUI_WIDGET_TAB_VIEW,
    LUI_WIDGET_TREE_VIEW,
    LUI_WIDGET_TABLE_VIEW,
    LUI_WIDGET_CANVAS,
    LUI_WIDGET_PROGRESS_BAR,
    LUI_WIDGET_SPINNER,
    LUI_WIDGET_MENU,
    LUI_WIDGET_TOOLBAR,
    LUI_WIDGET_STATUS_BAR,
    LUI_WIDGET_CUSTOM
} lui_widget_type_t;

typedef enum {
    LUI_STATE_NORMAL = 0,
    LUI_STATE_HOVERED,
    LUI_STATE_PRESSED,
    LUI_STATE_FOCUSED,
    LUI_STATE_DISABLED,
    LUI_STATE_SELECTED,
    LUI_STATE_ACTIVE
} lui_widget_state_t;

// Forward declarations
typedef struct lui_widget lui_widget_t;
typedef struct lui_window lui_window_t;
typedef struct lui_context lui_context_t;

// Widget Event Handlers
typedef bool (*lui_event_handler_t)(lui_widget_t* widget, const lui_input_event_t* event, void* user_data);
typedef void (*lui_paint_handler_t)(lui_widget_t* widget, lui_context_t* ctx, const lui_rect_t* clip);
typedef lui_size_t (*lui_measure_handler_t)(lui_widget_t* widget, lui_size_t available);
typedef void (*lui_layout_handler_t)(lui_widget_t* widget, const lui_rect_t* bounds);

// Core Widget Structure
struct lui_widget {
    // Identity and hierarchy
    uint32_t id;
    lui_widget_type_t type;
    char name[64];
    lui_widget_t* parent;
    lui_widget_t* first_child;
    lui_widget_t* last_child;
    lui_widget_t* next_sibling;
    lui_widget_t* prev_sibling;
    uint32_t child_count;
    
    // State and properties
    lui_widget_state_t state;
    uint32_t flags;
    bool visible;
    bool enabled;
    bool focusable;
    bool clip_children;
    float opacity;
    
    // Layout and positioning
    lui_rect_t bounds;
    lui_rect_t content_bounds;
    lui_layout_t layout;
    lui_size_t preferred_size;
    lui_size_t min_size;
    lui_size_t max_size;
    
    // Styling
    lui_color_t background_color;
    lui_color_t border_color;
    float border_width;
    float corner_radius;
    lui_typography_t typography;
    
    // Event handling
    lui_event_handler_t on_click;
    lui_event_handler_t on_input;
    lui_event_handler_t on_focus;
    lui_event_handler_t on_key;
    lui_paint_handler_t on_paint;
    lui_measure_handler_t on_measure;
    lui_layout_handler_t on_layout;
    void* user_data;
    
    // Widget-specific data
    void* widget_data;
    size_t widget_data_size;
    
    // Rendering cache
    bool needs_layout;
    bool needs_repaint;
    uint32_t last_frame;
};

// Widget Flags
#define LUI_WIDGET_FLAG_NONE            0x00000000
#define LUI_WIDGET_FLAG_NO_INPUT        0x00000001
#define LUI_WIDGET_FLAG_NO_FOCUS        0x00000002
#define LUI_WIDGET_FLAG_TRANSPARENT     0x00000004
#define LUI_WIDGET_FLAG_ABSOLUTE        0x00000008
#define LUI_WIDGET_FLAG_FIXED           0x00000010
#define LUI_WIDGET_FLAG_DRAGGABLE       0x00000020
#define LUI_WIDGET_FLAG_RESIZABLE       0x00000040
#define LUI_WIDGET_FLAG_SCALABLE        0x00000080
#define LUI_WIDGET_FLAG_ROTATABLE       0x00000100

// ============================================================================
// WINDOW MANAGEMENT
// ============================================================================

typedef enum {
    LUI_WINDOW_NORMAL = 0,
    LUI_WINDOW_DIALOG,
    LUI_WINDOW_POPUP,
    LUI_WINDOW_TOOLTIP,
    LUI_WINDOW_DOCK,
    LUI_WINDOW_DESKTOP,
    LUI_WINDOW_SPLASH,
    LUI_WINDOW_FULLSCREEN
} lui_window_type_t;

typedef enum {
    LUI_WINDOW_STATE_HIDDEN = 0,
    LUI_WINDOW_STATE_NORMAL,
    LUI_WINDOW_STATE_MINIMIZED,
    LUI_WINDOW_STATE_MAXIMIZED,
    LUI_WINDOW_STATE_FULLSCREEN
} lui_window_state_t;

typedef struct {
    bool resizable;
    bool movable;
    bool closable;
    bool minimizable;
    bool maximizable;
    bool always_on_top;
    bool show_in_taskbar;
    bool decorated;
    float min_width, min_height;
    float max_width, max_height;
} lui_window_style_t;

struct lui_window {
    // Identity and hierarchy
    uint32_t id;
    char title[256];
    lui_window_type_t type;
    lui_window_state_t state;
    lui_window_style_t style;
    
    // Geometry
    lui_rect_t frame;          // Includes decorations
    lui_rect_t content_area;   // Excludes decorations
    float dpi_scale;
    
    // Content
    lui_widget_t* root_widget;
    lui_widget_t* focused_widget;
    
    // Rendering
    uint32_t* framebuffer;
    uint32_t fb_width, fb_height;
    bool needs_redraw;
    bool vsync_enabled;
    
    // Event handling
    lui_event_handler_t on_close;
    lui_event_handler_t on_resize;
    lui_event_handler_t on_focus;
    void* user_data;
    
    // Window list management
    lui_window_t* next;
    lui_window_t* prev;
};

// ============================================================================
// RENDERING CONTEXT
// ============================================================================

typedef enum {
    LUI_RENDERER_SOFTWARE = 0,
    LUI_RENDERER_OPENGL,
    LUI_RENDERER_VULKAN,
    LUI_RENDERER_METAL,
    LUI_RENDERER_DIRECTX
} lui_renderer_type_t;

typedef struct {
    lui_point_t start;
    lui_point_t end;
    lui_color_t start_color;
    lui_color_t end_color;
} lui_gradient_t;

typedef struct {
    uint32_t* pixels;
    uint32_t width, height;
    uint32_t stride;
} lui_texture_t;

struct lui_context {
    // Renderer backend
    lui_renderer_type_t renderer_type;
    void* renderer_data;
    
    // Current state
    lui_rect_t clip_rect;
    float opacity;
    lui_color_t fill_color;
    lui_color_t stroke_color;
    float stroke_width;
    
    // Transform matrix (2D)
    float transform[6];  // [a, b, c, d, tx, ty]
    
    // Text rendering
    lui_typography_t current_font;
    void* font_renderer;
    
    // Resource management
    lui_texture_t* textures;
    uint32_t texture_count;
    uint32_t max_textures;
    
    // Performance metrics
    uint32_t draw_calls;
    uint32_t triangles_rendered;
    uint64_t frame_time_us;
};

// ============================================================================
// THEME SYSTEM
// ============================================================================

typedef struct {
    char name[64];
    char description[256];
    
    // Color palette
    lui_color_t primary;
    lui_color_t secondary;
    lui_color_t surface;
    lui_color_t background;
    lui_color_t error;
    lui_color_t warning;
    lui_color_t success;
    lui_color_t info;
    
    // Text colors
    lui_color_t on_primary;
    lui_color_t on_secondary;
    lui_color_t on_surface;
    lui_color_t on_background;
    lui_color_t on_error;
    
    // Typography system
    lui_typography_t typography_scale[16];
    
    // Spacing and sizing
    float spacing_unit;        // Base spacing unit (8px)
    float border_radius_small; // 4px
    float border_radius_medium;// 8px
    float border_radius_large; // 16px
    
    // Animation timing
    uint32_t animation_fast;   // 100ms
    uint32_t animation_normal; // 200ms
    uint32_t animation_slow;   // 300ms
    
    // Shadows and elevation
    struct {
        lui_color_t color;
        float offset_x, offset_y;
        float blur_radius;
        float spread_radius;
    } shadows[8];  // Material elevation levels 0-7
    
} lui_theme_t;

// Predefined Themes
extern const lui_theme_t LUI_THEME_MILITARY_DARK;     // Default dark military theme
extern const lui_theme_t LUI_THEME_MILITARY_LIGHT;    // Light variant
extern const lui_theme_t LUI_THEME_TACTICAL_BLUE;     // Blue accent theme
extern const lui_theme_t LUI_THEME_MISSION_GREEN;     // Green accent theme
extern const lui_theme_t LUI_THEME_HIGH_CONTRAST;     // Accessibility theme

// ============================================================================
// MAIN API FUNCTIONS
// ============================================================================

// System initialization and shutdown
bool lui_init(lui_renderer_type_t renderer);
void lui_shutdown(void);
void lui_set_theme(const lui_theme_t* theme);
const lui_theme_t* lui_get_theme(void);

// Window management
lui_window_t* lui_create_window(const char* title, lui_window_type_t type, 
                               float x, float y, float width, float height);
void lui_destroy_window(lui_window_t* window);
void lui_show_window(lui_window_t* window);
void lui_hide_window(lui_window_t* window);
void lui_set_window_size(lui_window_t* window, float width, float height);
void lui_set_window_position(lui_window_t* window, float x, float y);

// Widget creation and management
lui_widget_t* lui_create_widget(lui_widget_type_t type, lui_widget_t* parent);
void lui_destroy_widget(lui_widget_t* widget);
void lui_add_child(lui_widget_t* parent, lui_widget_t* child);
void lui_remove_child(lui_widget_t* parent, lui_widget_t* child);
lui_widget_t* lui_find_widget_by_id(lui_widget_t* root, uint32_t id);
lui_widget_t* lui_find_widget_by_name(lui_widget_t* root, const char* name);

// Widget properties
void lui_set_widget_bounds(lui_widget_t* widget, const lui_rect_t* bounds);
void lui_set_widget_size(lui_widget_t* widget, float width, float height);
void lui_set_widget_position(lui_widget_t* widget, float x, float y);
void lui_set_widget_text(lui_widget_t* widget, const char* text);
const char* lui_get_widget_text(lui_widget_t* widget);
void lui_set_widget_color(lui_widget_t* widget, lui_color_t color);
void lui_set_widget_font(lui_widget_t* widget, const lui_typography_t* font);
void lui_set_widget_visible(lui_widget_t* widget, bool visible);
void lui_set_widget_enabled(lui_widget_t* widget, bool enabled);

// Event handling
void lui_set_event_handler(lui_widget_t* widget, lui_input_type_t type, 
                          lui_event_handler_t handler, void* user_data);
bool lui_process_events(void);
void lui_inject_input_event(const lui_input_event_t* event);

// Layout system
void lui_set_layout(lui_widget_t* widget, const lui_layout_t* layout);
void lui_invalidate_layout(lui_widget_t* widget);
void lui_update_layout(lui_widget_t* widget);
lui_size_t lui_measure_widget(lui_widget_t* widget, lui_size_t available);

// Rendering
lui_context_t* lui_begin_frame(lui_window_t* window);
void lui_end_frame(lui_context_t* ctx);
void lui_render_widget(lui_widget_t* widget, lui_context_t* ctx);

// Drawing primitives
void lui_draw_rect(lui_context_t* ctx, const lui_rect_t* rect, lui_color_t color);
void lui_draw_rounded_rect(lui_context_t* ctx, const lui_rect_t* rect, 
                          float radius, lui_color_t color);
void lui_draw_circle(lui_context_t* ctx, lui_point_t center, float radius, 
                    lui_color_t color);
void lui_draw_line(lui_context_t* ctx, lui_point_t start, lui_point_t end, 
                  float width, lui_color_t color);
void lui_draw_text(lui_context_t* ctx, const char* text, lui_point_t position, 
                  const lui_typography_t* font, lui_color_t color);
void lui_draw_image(lui_context_t* ctx, const lui_texture_t* texture, 
                   const lui_rect_t* src, const lui_rect_t* dst);

// Animation system
uint32_t lui_animate_float(float* value, float target, uint32_t duration_ms, 
                          lui_easing_t easing);
uint32_t lui_animate_color(lui_color_t* color, lui_color_t target, 
                          uint32_t duration_ms, lui_easing_t easing);
void lui_stop_animation(uint32_t animation_id);
void lui_update_animations(uint32_t delta_ms);

// Utility functions
lui_color_t lui_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
lui_color_t lui_color_lerp(lui_color_t from, lui_color_t to, float t);
lui_rect_t lui_rect_make(float x, float y, float width, float height);
bool lui_rect_contains_point(const lui_rect_t* rect, lui_point_t point);
bool lui_rect_intersects(const lui_rect_t* a, const lui_rect_t* b);
float lui_lerp(float from, float to, float t);
float lui_clamp(float value, float min, float max);

// Configuration and debugging
void lui_set_config_bool(const char* key, bool value);
void lui_set_config_int(const char* key, int value);
void lui_set_config_float(const char* key, float value);
void lui_set_config_string(const char* key, const char* value);
bool lui_get_config_bool(const char* key, bool default_value);
int lui_get_config_int(const char* key, int default_value);
float lui_get_config_float(const char* key, float default_value);
const char* lui_get_config_string(const char* key, const char* default_value);

void lui_enable_debug_mode(bool enabled);
void lui_print_widget_tree(lui_widget_t* root, int indent);
void lui_get_performance_stats(uint32_t* fps, uint32_t* frame_time_us, 
                              uint32_t* draw_calls);

#ifdef __cplusplus
}
#endif

#endif // LIMITLESS_UI_H