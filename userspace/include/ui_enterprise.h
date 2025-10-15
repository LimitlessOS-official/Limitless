/**
 * LimitlessOS Enterprise UI Toolkit
 * Military-grade user interface with advanced compositing and themes
 */

#ifndef LIMITLESS_UI_ENTERPRISE_H
#define LIMITLESS_UI_ENTERPRISE_H

#include <stdint.h>
#include <stdbool.h>
#include "graphics_enterprise.h"

/* UI Framework Version */
#define LIMITLESS_UI_VERSION_MAJOR 2
#define LIMITLESS_UI_VERSION_MINOR 0

/* Maximum UI limits */
#define MAX_UI_WINDOWS          2048
#define MAX_UI_WIDGETS          16384
#define MAX_UI_TEXTURES         4096
#define MAX_UI_FONTS            256
#define MAX_UI_THEMES           64
#define MAX_UI_ANIMATIONS       1024
#define MAX_UI_LAYOUTS          512

/* Widget Types */
typedef enum {
    WIDGET_TYPE_WINDOW,
    WIDGET_TYPE_PANEL,
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_LABEL,
    WIDGET_TYPE_TEXT_INPUT,
    WIDGET_TYPE_TEXT_AREA,
    WIDGET_TYPE_CHECKBOX,
    WIDGET_TYPE_RADIO_BUTTON,
    WIDGET_TYPE_SLIDER,
    WIDGET_TYPE_PROGRESS_BAR,
    WIDGET_TYPE_LIST_VIEW,
    WIDGET_TYPE_TREE_VIEW,
    WIDGET_TYPE_TABLE_VIEW,
    WIDGET_TYPE_TAB_CONTAINER,
    WIDGET_TYPE_MENU_BAR,
    WIDGET_TYPE_CONTEXT_MENU,
    WIDGET_TYPE_TOOLBAR,
    WIDGET_TYPE_STATUS_BAR,
    WIDGET_TYPE_SPLITTER,
    WIDGET_TYPE_DOCK_AREA,
    WIDGET_TYPE_SCROLL_VIEW,
    WIDGET_TYPE_IMAGE_VIEW,
    WIDGET_TYPE_VIDEO_VIEW,
    WIDGET_TYPE_CANVAS,
    WIDGET_TYPE_CHART,
    WIDGET_TYPE_TERMINAL,
    WIDGET_TYPE_CODE_EDITOR,
    WIDGET_TYPE_WEBVIEW,
    WIDGET_TYPE_CUSTOM
} ui_widget_type_t;

/* Window Types */
typedef enum {
    WINDOW_TYPE_NORMAL,
    WINDOW_TYPE_DIALOG,
    WINDOW_TYPE_POPUP,
    WINDOW_TYPE_TOOLTIP,
    WINDOW_TYPE_SPLASH,
    WINDOW_TYPE_DESKTOP,
    WINDOW_TYPE_PANEL,
    WINDOW_TYPE_DOCK,
    WINDOW_TYPE_NOTIFICATION,
    WINDOW_TYPE_OVERLAY,
    WINDOW_TYPE_FULLSCREEN,
    WINDOW_TYPE_KIOSK
} ui_window_type_t;

/* Window States */
typedef enum {
    WINDOW_STATE_NORMAL,
    WINDOW_STATE_MINIMIZED,
    WINDOW_STATE_MAXIMIZED,
    WINDOW_STATE_FULLSCREEN,
    WINDOW_STATE_HIDDEN,
    WINDOW_STATE_ACTIVE,
    WINDOW_STATE_INACTIVE
} ui_window_state_t;

/* Layout Types */
typedef enum {
    LAYOUT_TYPE_FIXED,
    LAYOUT_TYPE_HORIZONTAL_BOX,
    LAYOUT_TYPE_VERTICAL_BOX,
    LAYOUT_TYPE_GRID,
    LAYOUT_TYPE_FLOW,
    LAYOUT_TYPE_STACK,
    LAYOUT_TYPE_DOCK,
    LAYOUT_TYPE_FORM,
    LAYOUT_TYPE_CONSTRAINT
} ui_layout_type_t;

/* Animation Types */
typedef enum {
    ANIMATION_TYPE_FADE,
    ANIMATION_TYPE_SLIDE,
    ANIMATION_TYPE_SCALE,
    ANIMATION_TYPE_ROTATE,
    ANIMATION_TYPE_MORPH,
    ANIMATION_TYPE_SPRING,
    ANIMATION_TYPE_BOUNCE,
    ANIMATION_TYPE_ELASTIC
} ui_animation_type_t;

/* Theme Types */
typedef enum {
    THEME_TYPE_DARK,
    THEME_TYPE_LIGHT,
    THEME_TYPE_HIGH_CONTRAST,
    THEME_TYPE_BLUE,
    THEME_TYPE_MILITARY,
    THEME_TYPE_NEON,
    THEME_TYPE_GLASS,
    THEME_TYPE_MATERIAL,
    THEME_TYPE_FLUENT,
    THEME_TYPE_CUSTOM
} ui_theme_type_t;

/* Color structure with alpha */
typedef struct ui_color {
    uint8_t r, g, b, a;
} ui_color_t;

/* Rectangle structure */
typedef struct ui_rect {
    int32_t x, y;
    uint32_t width, height;
} ui_rect_t;

/* Point structure */
typedef struct ui_point {
    int32_t x, y;
} ui_point_t;

/* Size structure */
typedef struct ui_size {
    uint32_t width, height;
} ui_size_t;

/* Margin/Padding */
typedef struct ui_insets {
    uint32_t left, top, right, bottom;
} ui_insets_t;

/* Font structure */
typedef struct ui_font {
    uint32_t id;
    char name[128];
    char family[64];
    uint32_t size;
    uint32_t weight;        /* 100-900 */
    bool italic;
    bool bold;
    bool underline;
    bool strikethrough;
    
    /* Font rendering data */
    void* font_data;        /* Font file data */
    size_t data_size;       /* Font data size */
    void* glyph_cache;      /* Cached glyphs */
    uint32_t glyph_count;   /* Number of cached glyphs */
    
    struct ui_font* next;
} ui_font_t;

/* Texture/Image */
typedef struct ui_texture {
    uint32_t id;
    uint32_t width, height;
    texture_format_t format;
    void* pixels;
    size_t size;
    bool premultiplied_alpha;
    uint32_t mip_levels;
    
    /* GPU resources */
    gpu_image_t* gpu_image;
    
    struct ui_texture* next;
} ui_texture_t;

/* Theme Colors */
typedef struct ui_theme_colors {
    ui_color_t background;
    ui_color_t foreground;
    ui_color_t primary;
    ui_color_t secondary;
    ui_color_t accent;
    ui_color_t success;
    ui_color_t warning;
    ui_color_t error;
    ui_color_t info;
    
    /* UI element colors */
    ui_color_t window_background;
    ui_color_t window_border;
    ui_color_t window_title_bg;
    ui_color_t window_title_fg;
    
    ui_color_t button_bg;
    ui_color_t button_fg;
    ui_color_t button_hover_bg;
    ui_color_t button_hover_fg;
    ui_color_t button_pressed_bg;
    ui_color_t button_pressed_fg;
    ui_color_t button_disabled_bg;
    ui_color_t button_disabled_fg;
    
    ui_color_t input_bg;
    ui_color_t input_fg;
    ui_color_t input_border;
    ui_color_t input_focus_border;
    ui_color_t input_selection_bg;
    ui_color_t input_selection_fg;
    
    ui_color_t menu_bg;
    ui_color_t menu_fg;
    ui_color_t menu_hover_bg;
    ui_color_t menu_hover_fg;
    ui_color_t menu_separator;
    
    ui_color_t scrollbar_bg;
    ui_color_t scrollbar_thumb;
    ui_color_t scrollbar_hover;
} ui_theme_colors_t;

/* Theme Metrics */
typedef struct ui_theme_metrics {
    uint32_t window_border_width;
    uint32_t window_title_height;
    uint32_t button_height;
    uint32_t input_height;
    uint32_t menu_height;
    uint32_t scrollbar_width;
    uint32_t splitter_width;
    uint32_t border_radius;
    uint32_t shadow_blur;
    uint32_t shadow_offset;
    float opacity_disabled;
    float opacity_hover;
} ui_theme_metrics_t;

/* UI Theme */
typedef struct ui_theme {
    uint32_t id;
    char name[64];
    ui_theme_type_t type;
    ui_theme_colors_t colors;
    ui_theme_metrics_t metrics;
    ui_font_t* default_font;
    ui_font_t* title_font;
    ui_font_t* monospace_font;
    
    /* Animation settings */
    uint32_t animation_duration_ms;
    float animation_curve[4];   /* Bezier curve */
    
    struct ui_theme* next;
} ui_theme_t;

/* Layout Constraint */
typedef struct ui_constraint {
    enum {
        CONSTRAINT_FIXED,
        CONSTRAINT_RELATIVE,
        CONSTRAINT_CENTER,
        CONSTRAINT_STRETCH,
        CONSTRAINT_ASPECT_RATIO
    } type;
    
    float value;        /* Fixed value or percentage */
    int32_t offset;     /* Offset in pixels */
    uint32_t target_id; /* Target widget ID for relative constraints */
} ui_constraint_t;

/* Widget Layout */
typedef struct ui_layout {
    ui_layout_type_t type;
    ui_insets_t padding;
    ui_insets_t margin;
    uint32_t spacing;
    
    /* Alignment */
    enum {
        ALIGN_START,
        ALIGN_CENTER,
        ALIGN_END,
        ALIGN_STRETCH
    } horizontal_align, vertical_align;
    
    /* Constraints */
    ui_constraint_t x_constraint;
    ui_constraint_t y_constraint;
    ui_constraint_t width_constraint;
    ui_constraint_t height_constraint;
    
    /* Grid layout specific */
    uint32_t grid_columns;
    uint32_t grid_rows;
    uint32_t grid_column_span;
    uint32_t grid_row_span;
} ui_layout_t;

/* Animation */
typedef struct ui_animation {
    uint32_t id;
    ui_widget_type_t target_type;
    uint32_t target_id;
    ui_animation_type_t type;
    
    uint64_t start_time;
    uint32_t duration_ms;
    bool loop;
    bool reverse;
    
    /* Animation values */
    float start_value;
    float end_value;
    float current_value;
    
    /* Easing function */
    float (*easing_func)(float t);
    
    /* Callback */
    void (*on_complete)(struct ui_animation* animation);
    
    bool active;
    struct ui_animation* next;
} ui_animation_t;

/* Forward declaration */
struct ui_widget;

/* Event Types */
typedef enum {
    UI_EVENT_NONE,
    UI_EVENT_PAINT,
    UI_EVENT_MOUSE_MOVE,
    UI_EVENT_MOUSE_DOWN,
    UI_EVENT_MOUSE_UP,
    UI_EVENT_MOUSE_CLICK,
    UI_EVENT_MOUSE_DOUBLE_CLICK,
    UI_EVENT_MOUSE_WHEEL,
    UI_EVENT_KEY_DOWN,
    UI_EVENT_KEY_UP,
    UI_EVENT_KEY_PRESS,
    UI_EVENT_FOCUS_IN,
    UI_EVENT_FOCUS_OUT,
    UI_EVENT_RESIZE,
    UI_EVENT_MOVE,
    UI_EVENT_SHOW,
    UI_EVENT_HIDE,
    UI_EVENT_CLOSE,
    UI_EVENT_TIMER,
    UI_EVENT_ANIMATION,
    UI_EVENT_CUSTOM
} ui_event_type_t;

/* UI Event */
typedef struct ui_event {
    ui_event_type_t type;
    uint64_t timestamp;
    struct ui_widget* target;
    
    union {
        struct {
            ui_rect_t region;
        } paint;
        
        struct {
            ui_point_t position;
            ui_point_t delta;
            uint32_t buttons;
        } mouse;
        
        struct {
            uint32_t keycode;
            uint32_t modifiers;
            char character;
        } key;
        
        struct {
            ui_size_t old_size;
            ui_size_t new_size;
        } resize;
        
        struct {
            ui_point_t old_position;
            ui_point_t new_position;
        } move;
        
        struct {
            uint32_t timer_id;
        } timer;
        
        struct {
            uint32_t animation_id;
        } animation;
        
        struct {
            void* data;
            size_t size;
        } custom;
    } data;
} ui_event_t;

/* Event Handler */
typedef bool (*ui_event_handler_t)(struct ui_widget* widget, ui_event_t* event);

/* Widget State */
typedef struct ui_widget_state {
    bool visible;
    bool enabled;
    bool focused;
    bool hovered;
    bool pressed;
    bool selected;
    bool dirty;         /* Needs redraw */
    bool layout_dirty;  /* Needs layout update */
} ui_widget_state_t;

/* Widget */
typedef struct ui_widget {
    uint32_t id;
    ui_widget_type_t type;
    char name[64];
    
    /* Hierarchy */
    struct ui_widget* parent;
    struct ui_widget* first_child;
    struct ui_widget* last_child;
    struct ui_widget* next_sibling;
    struct ui_widget* prev_sibling;
    uint32_t child_count;
    
    /* Geometry */
    ui_rect_t bounds;           /* Widget bounds */
    ui_rect_t content_bounds;   /* Content area (excluding borders/padding) */
    ui_layout_t layout;
    
    /* Appearance */
    ui_color_t background_color;
    ui_color_t foreground_color;
    ui_color_t border_color;
    uint32_t border_width;
    uint32_t border_radius;
    float opacity;
    
    /* Text */
    char text[512];
    ui_font_t* font;
    uint32_t text_align;        /* Left, center, right */
    uint32_t text_valign;       /* Top, middle, bottom */
    
    /* Images */
    ui_texture_t* background_image;
    ui_texture_t* icon;
    
    /* State */
    ui_widget_state_t state;
    
    /* Events */
    ui_event_handler_t on_paint;
    ui_event_handler_t on_mouse_move;
    ui_event_handler_t on_mouse_down;
    ui_event_handler_t on_mouse_up;
    ui_event_handler_t on_mouse_click;
    ui_event_handler_t on_key_down;
    ui_event_handler_t on_key_up;
    ui_event_handler_t on_focus_in;
    ui_event_handler_t on_focus_out;
    ui_event_handler_t on_resize;
    ui_event_handler_t on_custom;
    
    /* Rendering */
    gpu_framebuffer_t* framebuffer; /* Widget's render target */
    bool needs_redraw;
    
    /* User data */
    void* user_data;
} ui_widget_t;

/* Window */
typedef struct ui_window {
    ui_widget_t widget;     /* Base widget */
    
    ui_window_type_t window_type;
    ui_window_state_t state;
    
    /* Window properties */
    bool resizable;
    bool movable;
    bool closable;
    bool minimizable;
    bool maximizable;
    bool modal;
    bool always_on_top;
    bool show_in_taskbar;
    
    /* Window decorations */
    bool has_title_bar;
    bool has_border;
    bool has_shadow;
    
    /* Window content */
    ui_widget_t* content_widget;
    
    /* Window management */
    struct ui_window* owner;     /* Parent window for modal dialogs */
    
    /* Platform-specific */
    void* platform_handle;      /* Native window handle */
} ui_window_t;

/* Drawing Context */
typedef struct ui_draw_context {
    graphics_context_t* gpu_context;
    gpu_command_buffer_t* command_buffer;
    gpu_framebuffer_t* framebuffer;
    
    ui_rect_t clip_rect;
    ui_color_t fill_color;
    ui_color_t stroke_color;
    float stroke_width;
    ui_font_t* current_font;
    
    /* Transformation matrix (3x3) */
    float transform[9];
    
    /* Anti-aliasing */
    bool anti_alias;
    uint32_t msaa_samples;
} ui_draw_context_t;

/* UI Manager */
typedef struct ui_manager {
    bool initialized;
    uint32_t version;
    
    /* Graphics integration */
    graphics_context_t* graphics_context;
    display_output_t* primary_display;
    
    /* Widget management */
    uint32_t next_widget_id;
    ui_widget_t* widgets[MAX_UI_WIDGETS];
    uint32_t widget_count;
    ui_widget_t* root_widget;
    ui_widget_t* focused_widget;
    ui_widget_t* hovered_widget;
    
    /* Window management */
    ui_window_t* windows[MAX_UI_WINDOWS];
    uint32_t window_count;
    ui_window_t* active_window;
    ui_window_t* desktop_window;
    
    /* Resources */
    ui_font_t* fonts;
    uint32_t font_count;
    ui_texture_t* textures;
    uint32_t texture_count;
    ui_theme_t* themes;
    uint32_t theme_count;
    ui_theme_t* current_theme;
    
    /* Animations */
    ui_animation_t* animations;
    uint32_t animation_count;
    
    /* Input state */
    ui_point_t cursor_position;
    uint32_t mouse_buttons;
    uint32_t keyboard_modifiers;
    
    /* Performance */
    struct {
        uint64_t frames_rendered;
        uint64_t widgets_drawn;
        uint64_t draw_calls;
        uint32_t fps;
        float frame_time_ms;
    } performance;
    
    /* Settings */
    bool enable_animations;
    bool enable_transparency;
    bool enable_anti_aliasing;
    uint32_t animation_fps;
    float ui_scale_factor;
} ui_manager_t;

/* Global UI manager */
extern ui_manager_t ui_manager;

/* Core UI API */
status_t ui_init(graphics_context_t* graphics_context, display_output_t* display);
void ui_shutdown(void);
status_t ui_update(float delta_time);
status_t ui_render(void);

/* Widget Management */
ui_widget_t* ui_create_widget(ui_widget_type_t type, const char* name, ui_widget_t* parent);
void ui_destroy_widget(ui_widget_t* widget);
ui_widget_t* ui_get_widget_by_id(uint32_t id);
ui_widget_t* ui_get_widget_by_name(const char* name);

/* Widget Hierarchy */
void ui_add_child_widget(ui_widget_t* parent, ui_widget_t* child);
void ui_remove_child_widget(ui_widget_t* parent, ui_widget_t* child);
ui_widget_t* ui_get_parent_widget(ui_widget_t* widget);
ui_widget_t* ui_get_first_child_widget(ui_widget_t* widget);
ui_widget_t* ui_get_next_sibling_widget(ui_widget_t* widget);

/* Widget Properties */
void ui_set_widget_bounds(ui_widget_t* widget, ui_rect_t bounds);
ui_rect_t ui_get_widget_bounds(ui_widget_t* widget);
void ui_set_widget_visible(ui_widget_t* widget, bool visible);
bool ui_is_widget_visible(ui_widget_t* widget);
void ui_set_widget_enabled(ui_widget_t* widget, bool enabled);
bool ui_is_widget_enabled(ui_widget_t* widget);

/* Widget Text */
void ui_set_widget_text(ui_widget_t* widget, const char* text);
const char* ui_get_widget_text(ui_widget_t* widget);
void ui_set_widget_font(ui_widget_t* widget, ui_font_t* font);
ui_font_t* ui_get_widget_font(ui_widget_t* widget);

/* Widget Colors */
void ui_set_widget_background_color(ui_widget_t* widget, ui_color_t color);
ui_color_t ui_get_widget_background_color(ui_widget_t* widget);
void ui_set_widget_foreground_color(ui_widget_t* widget, ui_color_t color);
ui_color_t ui_get_widget_foreground_color(ui_widget_t* widget);

/* Window Management */
ui_window_t* ui_create_window(ui_window_type_t type, const char* title, ui_rect_t bounds);
void ui_destroy_window(ui_window_t* window);
void ui_show_window(ui_window_t* window);
void ui_hide_window(ui_window_t* window);
void ui_activate_window(ui_window_t* window);
void ui_minimize_window(ui_window_t* window);
void ui_maximize_window(ui_window_t* window);
void ui_restore_window(ui_window_t* window);

/* Layout Management */
void ui_set_widget_layout(ui_widget_t* widget, ui_layout_t layout);
ui_layout_t ui_get_widget_layout(ui_widget_t* widget);
void ui_update_layout(ui_widget_t* widget);
void ui_invalidate_layout(ui_widget_t* widget);

/* Font Management */
ui_font_t* ui_load_font(const char* name, const char* filename, uint32_t size);
ui_font_t* ui_create_font_from_memory(const char* name, void* data, size_t size, uint32_t font_size);
void ui_destroy_font(ui_font_t* font);
ui_font_t* ui_get_font_by_name(const char* name);

/* Texture Management */
ui_texture_t* ui_load_texture(const char* filename);
ui_texture_t* ui_create_texture_from_memory(void* pixels, uint32_t width, uint32_t height, texture_format_t format);
void ui_destroy_texture(ui_texture_t* texture);
ui_texture_t* ui_get_texture_by_id(uint32_t id);

/* Theme Management */
ui_theme_t* ui_create_theme(const char* name, ui_theme_type_t type);
void ui_destroy_theme(ui_theme_t* theme);
void ui_set_current_theme(ui_theme_t* theme);
ui_theme_t* ui_get_current_theme(void);
ui_theme_t* ui_get_theme_by_name(const char* name);

/* Animation System */
ui_animation_t* ui_create_animation(ui_widget_t* target, ui_animation_type_t type, uint32_t duration_ms);
void ui_destroy_animation(ui_animation_t* animation);
void ui_start_animation(ui_animation_t* animation);
void ui_stop_animation(ui_animation_t* animation);
void ui_update_animations(float delta_time);

/* Drawing API */
ui_draw_context_t* ui_begin_draw(ui_widget_t* widget);
void ui_end_draw(ui_draw_context_t* context);

void ui_draw_rectangle(ui_draw_context_t* ctx, ui_rect_t rect, ui_color_t color);
void ui_draw_rounded_rectangle(ui_draw_context_t* ctx, ui_rect_t rect, uint32_t radius, ui_color_t color);
void ui_draw_circle(ui_draw_context_t* ctx, ui_point_t center, uint32_t radius, ui_color_t color);
void ui_draw_line(ui_draw_context_t* ctx, ui_point_t start, ui_point_t end, ui_color_t color, float width);
void ui_draw_text(ui_draw_context_t* ctx, const char* text, ui_point_t position, ui_font_t* font, ui_color_t color);
void ui_draw_texture(ui_draw_context_t* ctx, ui_texture_t* texture, ui_rect_t dest_rect);
void ui_draw_gradient(ui_draw_context_t* ctx, ui_rect_t rect, ui_color_t start_color, ui_color_t end_color, bool vertical);

/* Event System */
void ui_dispatch_event(ui_event_t* event);
bool ui_handle_mouse_event(ui_point_t position, uint32_t buttons, bool pressed);
bool ui_handle_key_event(uint32_t keycode, uint32_t modifiers, bool pressed);
void ui_set_focus(ui_widget_t* widget);
ui_widget_t* ui_get_focused_widget(void);

/* Utility Functions */
ui_color_t ui_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
ui_color_t ui_color_rgb(uint8_t r, uint8_t g, uint8_t b);
ui_color_t ui_color_from_hex(uint32_t hex);
ui_rect_t ui_rect_make(int32_t x, int32_t y, uint32_t width, uint32_t height);
ui_point_t ui_point_make(int32_t x, int32_t y);
ui_size_t ui_size_make(uint32_t width, uint32_t height);
bool ui_rect_contains_point(ui_rect_t rect, ui_point_t point);
ui_rect_t ui_rect_intersect(ui_rect_t a, ui_rect_t b);

/* Performance and Debug */
void ui_print_performance_stats(void);
void ui_print_widget_hierarchy(ui_widget_t* root, uint32_t depth);
status_t ui_capture_screenshot(const char* filename);
void ui_enable_debug_overlay(bool enable);

#endif /* LIMITLESS_UI_ENTERPRISE_H */