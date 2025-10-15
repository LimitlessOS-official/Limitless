/*
 * LimitlessOS Desktop Environment - Wayland Compositor
 * Production desktop compositor with modern features
 * Features: Multi-monitor, workspaces, animations, accessibility, touch support
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../kernel.h"
#include "../drm/drm_core.h"
#include "../input/input_core.h"

/* Desktop environment constants */
#define MAX_DISPLAYS            16
#define MAX_WINDOWS             1024
#define MAX_WORKSPACES          16
#define MAX_PANELS              8
#define MAX_WIDGETS             128
#define MAX_ANIMATIONS          256
#define MAX_THEMES              32

/* Display modes */
typedef enum {
    DISPLAY_MODE_SINGLE = 0,    /* Single display */
    DISPLAY_MODE_CLONE,         /* Mirror displays */
    DISPLAY_MODE_EXTEND,        /* Extended desktop */
    DISPLAY_MODE_DISCRETE,      /* Discrete displays */
    DISPLAY_MODE_MAX
} display_mode_t;

/* Window types */
typedef enum {
    WINDOW_TYPE_NORMAL = 0,
    WINDOW_TYPE_DESKTOP,
    WINDOW_TYPE_DOCK,
    WINDOW_TYPE_TOOLBAR,
    WINDOW_TYPE_MENU,
    WINDOW_TYPE_UTILITY,
    WINDOW_TYPE_SPLASH,
    WINDOW_TYPE_DIALOG,
    WINDOW_TYPE_DROPDOWN_MENU,
    WINDOW_TYPE_POPUP_MENU,
    WINDOW_TYPE_TOOLTIP,
    WINDOW_TYPE_NOTIFICATION,
    WINDOW_TYPE_COMBO,
    WINDOW_TYPE_DND,
    WINDOW_TYPE_MAX
} window_type_t;

/* Window states */
typedef enum {
    WINDOW_STATE_NORMAL = 0,
    WINDOW_STATE_MINIMIZED,
    WINDOW_STATE_MAXIMIZED,
    WINDOW_STATE_FULLSCREEN,
    WINDOW_STATE_SHADED,
    WINDOW_STATE_STICKY,
    WINDOW_STATE_HIDDEN,
    WINDOW_STATE_MAX
} window_state_t;

/* Animation types */
typedef enum {
    ANIMATION_NONE = 0,
    ANIMATION_FADE_IN,
    ANIMATION_FADE_OUT,
    ANIMATION_SLIDE_LEFT,
    ANIMATION_SLIDE_RIGHT,
    ANIMATION_SLIDE_UP,
    ANIMATION_SLIDE_DOWN,
    ANIMATION_ZOOM_IN,
    ANIMATION_ZOOM_OUT,
    ANIMATION_ROTATE,
    ANIMATION_BOUNCE,
    ANIMATION_ELASTIC,
    ANIMATION_FLIP,
    ANIMATION_CUBE,
    ANIMATION_WOBBLE,
    ANIMATION_MAX
} animation_type_t;

/* Theme types */
typedef enum {
    THEME_LIGHT = 0,
    THEME_DARK,
    THEME_HIGH_CONTRAST,
    THEME_CUSTOM,
    THEME_MAX
} theme_type_t;

/* Display configuration */
typedef struct display_config {
    uint32_t display_id;        /* Display ID */
    char name[64];              /* Display name */
    char manufacturer[32];      /* Display manufacturer */
    char model[32];             /* Display model */
    
    /* Physical properties */
    uint32_t width_mm;          /* Width in millimeters */
    uint32_t height_mm;         /* Height in millimeters */
    float diagonal_inches;      /* Diagonal size in inches */
    
    /* Current mode */
    struct {
        uint32_t width;         /* Resolution width */
        uint32_t height;        /* Resolution height */
        uint32_t refresh_rate;  /* Refresh rate in Hz */
        uint32_t bit_depth;     /* Color bit depth */
    } current_mode;
    
    /* Available modes */
    struct {
        uint32_t width;
        uint32_t height;
        uint32_t refresh_rate;
        uint32_t bit_depth;
    } modes[32];
    uint32_t mode_count;
    
    /* Position and orientation */
    struct {
        int32_t x;              /* X position */
        int32_t y;              /* Y position */
        uint32_t rotation;      /* Rotation (0, 90, 180, 270) */
        bool mirrored;          /* Display is mirrored */
    } geometry;
    
    /* Properties */
    struct {
        bool primary;           /* Primary display */
        bool enabled;           /* Display is enabled */
        bool connected;         /* Display is connected */
        bool touch_enabled;     /* Touch input enabled */
        float scale_factor;     /* UI scale factor */
        uint32_t subpixel_order; /* Subpixel rendering order */
    } properties;
    
    /* Color profile */
    struct {
        float gamma_red;
        float gamma_green;
        float gamma_blue;
        float brightness;
        float contrast;
        float saturation;
        float temperature;      /* Color temperature */
    } color;
    
    /* Power management */
    struct {
        bool dpms_enabled;      /* DPMS support */
        uint32_t idle_timeout_s; /* Idle timeout */
        bool auto_brightness;   /* Automatic brightness */
        uint32_t max_brightness; /* Maximum brightness */
        uint32_t current_brightness; /* Current brightness */
    } power;
    
} display_config_t;

/* Window structure */
typedef struct window {
    uint32_t window_id;         /* Unique window ID */
    char title[256];            /* Window title */
    char app_id[64];            /* Application ID */
    
    window_type_t type;         /* Window type */
    window_state_t state;       /* Window state */
    
    /* Geometry */
    struct {
        int32_t x, y;           /* Window position */
        uint32_t width, height; /* Window size */
        int32_t min_width, min_height; /* Minimum size */
        int32_t max_width, max_height; /* Maximum size */
        bool resizable;         /* Window is resizable */
        bool movable;           /* Window is movable */
    } geometry;
    
    /* Visual properties */
    struct {
        uint32_t opacity;       /* Window opacity (0-255) */
        bool visible;           /* Window is visible */
        bool decorated;         /* Window has decorations */
        bool focused;           /* Window has focus */
        uint32_t layer;         /* Window layer/z-order */
    } visual;
    
    /* Parent/child relationships */
    struct window *parent;      /* Parent window */
    struct window *children[32]; /* Child windows */
    uint32_t child_count;
    
    /* Workspace */
    uint32_t workspace_id;      /* Current workspace */
    bool sticky;                /* Visible on all workspaces */
    
    /* Surface information */
    struct {
        void *buffer;           /* Window buffer */
        uint32_t format;        /* Buffer format */
        uint32_t stride;        /* Buffer stride */
        bool needs_repaint;     /* Needs repainting */
    } surface;
    
    /* Input handling */
    struct {
        bool accepts_input;     /* Accepts input */
        bool keyboard_focus;    /* Has keyboard focus */
        bool mouse_focus;       /* Has mouse focus */
        uint32_t input_region_count;
        struct {
            int32_t x, y;
            uint32_t width, height;
        } input_regions[16];
    } input;
    
    /* Animation state */
    struct {
        bool animating;         /* Currently animating */
        animation_type_t type;  /* Animation type */
        uint64_t start_time;    /* Animation start time */
        uint32_t duration_ms;   /* Animation duration */
        float progress;         /* Animation progress (0.0-1.0) */
    } animation;
    
    /* Associated process */
    struct process *process;
    
    /* Operations */
    const struct window_ops *ops;
    
    struct list_head list;
    
} window_t;

/* Workspace structure */
typedef struct workspace {
    uint32_t workspace_id;      /* Workspace ID */
    char name[64];              /* Workspace name */
    
    /* Windows */
    struct {
        window_t *windows[MAX_WINDOWS];
        uint32_t count;
        window_t *focused_window;
    } windows;
    
    /* Layout */
    enum {
        LAYOUT_FLOATING = 0,
        LAYOUT_TILED,
        LAYOUT_TABBED,
        LAYOUT_STACKED,
        LAYOUT_MAXIMIZED,
        LAYOUT_MAX
    } layout_mode;
    
    /* Properties */
    bool active;                /* Workspace is active */
    bool visible;               /* Workspace is visible */
    uint32_t display_id;        /* Associated display */
    
    /* Background */
    struct {
        char wallpaper_path[256];
        uint32_t background_color;
        enum {
            BG_TILE = 0,
            BG_CENTER,
            BG_STRETCH,
            BG_FIT,
            BG_FILL,
            BG_MAX
        } wallpaper_mode;
    } background;
    
    struct list_head list;
    
} workspace_t;

/* Panel structure */
typedef struct panel {
    uint32_t panel_id;          /* Panel ID */
    char name[64];              /* Panel name */
    
    /* Position and geometry */
    struct {
        enum {
            PANEL_TOP = 0,
            PANEL_BOTTOM,
            PANEL_LEFT,
            PANEL_RIGHT,
            PANEL_MAX
        } position;
        
        int32_t x, y;           /* Panel position */
        uint32_t width, height; /* Panel size */
        uint32_t thickness;     /* Panel thickness */
        bool auto_hide;         /* Auto-hide panel */
        bool reserve_space;     /* Reserve screen space */
    } geometry;
    
    /* Visual properties */
    struct {
        uint32_t background_color;
        uint32_t opacity;
        bool transparent;
        char background_image[256];
    } visual;
    
    /* Widgets */
    struct {
        void *widgets[MAX_WIDGETS];
        uint32_t count;
    } widgets;
    
    /* Associated display */
    uint32_t display_id;
    
    struct list_head list;
    
} panel_t;

/* Animation structure */
typedef struct animation {
    uint32_t animation_id;      /* Animation ID */
    animation_type_t type;      /* Animation type */
    
    /* Target object */
    enum {
        ANIM_TARGET_WINDOW = 0,
        ANIM_TARGET_WORKSPACE,
        ANIM_TARGET_PANEL,
        ANIM_TARGET_CURSOR,
        ANIM_TARGET_MAX
    } target_type;
    void *target;               /* Target object */
    
    /* Timing */
    uint64_t start_time;        /* Start time */
    uint32_t duration_ms;       /* Duration in milliseconds */
    float progress;             /* Current progress (0.0-1.0) */
    
    /* Easing function */
    enum {
        EASING_LINEAR = 0,
        EASING_EASE_IN,
        EASING_EASE_OUT,
        EASING_EASE_IN_OUT,
        EASING_BOUNCE,
        EASING_ELASTIC,
        EASING_MAX
    } easing;
    
    /* Animation parameters */
    struct {
        float start_value;
        float end_value;
        float current_value;
    } params[4];                /* Up to 4 parameters */
    
    /* Callback */
    void (*completion_callback)(struct animation *anim);
    
    /* State */
    bool active;
    bool paused;
    
    struct list_head list;
    
} animation_t;

/* Theme structure */
typedef struct theme {
    char name[64];              /* Theme name */
    char description[256];      /* Theme description */
    theme_type_t type;          /* Theme type */
    
    /* Colors */
    struct {
        uint32_t primary;       /* Primary color */
        uint32_t secondary;     /* Secondary color */
        uint32_t background;    /* Background color */
        uint32_t surface;       /* Surface color */
        uint32_t text_primary;  /* Primary text color */
        uint32_t text_secondary; /* Secondary text color */
        uint32_t accent;        /* Accent color */
        uint32_t error;         /* Error color */
        uint32_t warning;       /* Warning color */
        uint32_t success;       /* Success color */
    } colors;
    
    /* Window decorations */
    struct {
        uint32_t title_bar_height;
        uint32_t border_width;
        uint32_t corner_radius;
        uint32_t title_bar_color;
        uint32_t border_color;
        char font_family[64];
        uint32_t font_size;
    } decorations;
    
    /* Panel styling */
    struct {
        uint32_t background_color;
        uint32_t opacity;
        uint32_t height;
        uint32_t padding;
    } panel;
    
    /* Icons and graphics */
    struct {
        char icon_theme[64];
        char cursor_theme[64];
        uint32_t icon_size;
        uint32_t cursor_size;
    } graphics;
    
} theme_t;

/* Compositor state */
typedef struct compositor {
    bool initialized;
    bool running;
    
    /* Displays */
    struct {
        display_config_t displays[MAX_DISPLAYS];
        uint32_t count;
        uint32_t primary_display;
        display_mode_t mode;
        rwlock_t lock;
    } displays;
    
    /* Windows */
    struct {
        window_t *windows[MAX_WINDOWS];
        uint32_t count;
        window_t *focused_window;
        rwlock_t lock;
    } windows;
    
    /* Workspaces */
    struct {
        workspace_t *workspaces[MAX_WORKSPACES];
        uint32_t count;
        uint32_t current_workspace;
        rwlock_t lock;
    } workspaces;
    
    /* Panels */
    struct {
        panel_t *panels[MAX_PANELS];
        uint32_t count;
        rwlock_t lock;
    } panels;
    
    /* Animations */
    struct {
        animation_t *animations[MAX_ANIMATIONS];
        uint32_t count;
        bool animations_enabled;
        struct workqueue_struct *anim_workqueue;
        spinlock_t lock;
    } animations;
    
    /* Current theme */
    theme_t current_theme;
    
    /* Input handling */
    struct {
        struct input_device *keyboard;
        struct input_device *mouse;
        struct input_device *touchpad;
        bool input_enabled;
    } input;
    
    /* Rendering */
    struct {
        struct drm_device *drm_device;
        void *render_context;
        bool vsync_enabled;
        uint32_t frame_rate_target;
        uint64_t last_frame_time;
    } rendering;
    
    /* Accessibility */
    struct {
        bool screen_reader_enabled;
        bool high_contrast_enabled;
        bool magnifier_enabled;
        float magnification_level;
        bool sticky_keys_enabled;
        bool slow_keys_enabled;
    } accessibility;
    
    /* Performance */
    struct {
        atomic64_t frames_rendered;
        atomic64_t frame_drops;
        atomic64_t input_events_processed;
        float average_frame_time_ms;
        uint32_t active_animations;
    } stats;
    
    /* Configuration */
    struct {
        bool debug_mode;
        bool show_fps;
        bool enable_effects;
        uint32_t animation_speed;
        bool auto_tile_windows;
    } config;
    
} compositor_t;

/* Window operations */
struct window_ops {
    int (*show)(struct window *win);
    int (*hide)(struct window *win);
    int (*move)(struct window *win, int32_t x, int32_t y);
    int (*resize)(struct window *win, uint32_t width, uint32_t height);
    int (*set_title)(struct window *win, const char *title);
    int (*set_state)(struct window *win, window_state_t state);
    int (*focus)(struct window *win);
    int (*unfocus)(struct window *win);
    int (*close)(struct window *win);
};

/* External compositor */
extern compositor_t compositor;

/* Core compositor functions */
int compositor_init(void);
void compositor_exit(void);
int compositor_start(void);
void compositor_stop(void);

/* Display management */
int compositor_add_display(const struct display_config *config);
int compositor_remove_display(uint32_t display_id);
int compositor_configure_display(uint32_t display_id, const struct display_config *config);
int compositor_set_primary_display(uint32_t display_id);
int compositor_set_display_mode(display_mode_t mode);
struct display_config *compositor_get_display(uint32_t display_id);
int compositor_detect_displays(void);

/* Window management */
struct window *compositor_create_window(const char *title, const char *app_id, window_type_t type);
void compositor_destroy_window(struct window *win);
int compositor_show_window(struct window *win);
int compositor_hide_window(struct window *win);
int compositor_move_window(struct window *win, int32_t x, int32_t y);
int compositor_resize_window(struct window *win, uint32_t width, uint32_t height);
int compositor_set_window_state(struct window *win, window_state_t state);
int compositor_focus_window(struct window *win);
struct window *compositor_get_focused_window(void);
struct window *compositor_find_window_at(int32_t x, int32_t y);
int compositor_set_window_workspace(struct window *win, uint32_t workspace_id);

/* Workspace management */
struct workspace *compositor_create_workspace(const char *name);
void compositor_destroy_workspace(struct workspace *ws);
int compositor_switch_workspace(uint32_t workspace_id);
int compositor_move_window_to_workspace(struct window *win, uint32_t workspace_id);
struct workspace *compositor_get_current_workspace(void);
int compositor_set_workspace_layout(uint32_t workspace_id, int layout_mode);
int compositor_set_workspace_wallpaper(uint32_t workspace_id, const char *path);

/* Panel management */
struct panel *compositor_create_panel(const char *name, int position);
void compositor_destroy_panel(struct panel *panel);
int compositor_show_panel(struct panel *panel);
int compositor_hide_panel(struct panel *panel);
int compositor_add_panel_widget(struct panel *panel, void *widget);
int compositor_remove_panel_widget(struct panel *panel, void *widget);

/* Animation system */
struct animation *compositor_create_animation(animation_type_t type, void *target, uint32_t duration_ms);
void compositor_destroy_animation(struct animation *anim);
int compositor_start_animation(struct animation *anim);
int compositor_stop_animation(struct animation *anim);
int compositor_pause_animation(struct animation *anim);
int compositor_resume_animation(struct animation *anim);
void compositor_update_animations(void);
void compositor_enable_animations(bool enable);

/* Theme management */
int compositor_load_theme(const struct theme *theme);
int compositor_set_theme(const char *theme_name);
struct theme *compositor_get_current_theme(void);
int compositor_reload_theme(void);

/* Input handling */
int compositor_handle_key_event(const struct input_event *event);
int compositor_handle_mouse_event(const struct input_event *event);
int compositor_handle_touch_event(const struct input_event *event);
int compositor_handle_gesture_event(const struct input_event *event);

/* Rendering */
int compositor_render_frame(void);
int compositor_set_vsync(bool enabled);
int compositor_set_frame_rate(uint32_t fps);
void compositor_request_repaint(struct window *win);
void compositor_damage_region(int32_t x, int32_t y, uint32_t width, uint32_t height);

/* Effects and compositing */
int compositor_enable_effects(bool enable);
int compositor_set_window_opacity(struct window *win, uint32_t opacity);
int compositor_add_blur_effect(struct window *win, uint32_t radius);
int compositor_add_shadow_effect(struct window *win, uint32_t radius, uint32_t opacity);
int compositor_add_rounded_corners(struct window *win, uint32_t radius);

/* Accessibility */
int compositor_enable_screen_reader(bool enable);
int compositor_enable_high_contrast(bool enable);
int compositor_enable_magnifier(bool enable, float level);
int compositor_enable_sticky_keys(bool enable);
int compositor_enable_slow_keys(bool enable);

/* Touch and gesture support */
int compositor_enable_touch_input(bool enable);
int compositor_handle_multi_touch(const struct multitouch_state *state);
int compositor_recognize_gestures(bool enable);
int compositor_handle_pinch_gesture(float scale, int32_t x, int32_t y);
int compositor_handle_swipe_gesture(int direction, int32_t x, int32_t y);

/* Screen capture and sharing */
int compositor_screenshot(const char *filename);
int compositor_record_screen(const char *filename, uint32_t duration_s);
int compositor_share_screen(const char *target);
int compositor_capture_window(struct window *win, const char *filename);

/* Configuration */
int compositor_load_config(const char *filename);
int compositor_save_config(const char *filename);
int compositor_set_config_value(const char *key, const char *value);
const char *compositor_get_config_value(const char *key);

/* Statistics and monitoring */
void compositor_print_statistics(void);
void compositor_print_display_info(void);
void compositor_print_window_tree(void);
int compositor_get_performance_stats(void *stats, size_t size);

/* Utility functions */
const char *window_type_name(window_type_t type);
const char *window_state_name(window_state_t state);
const char *animation_type_name(animation_type_t type);
const char *theme_type_name(theme_type_t type);
uint32_t compositor_color_blend(uint32_t color1, uint32_t color2, float alpha);
bool compositor_point_in_window(struct window *win, int32_t x, int32_t y);

/* Advanced features */
int compositor_enable_always_on_top(struct window *win, bool enable);
int compositor_set_window_struts(struct window *win, uint32_t left, uint32_t right, uint32_t top, uint32_t bottom);
int compositor_pin_window(struct window *win, bool pin);
int compositor_shade_window(struct window *win, bool shade);
int compositor_minimize_all_windows(uint32_t workspace_id);
int compositor_tile_windows(uint32_t workspace_id, int layout);

/* Multi-monitor support */
int compositor_move_window_to_display(struct window *win, uint32_t display_id);
int compositor_span_window_across_displays(struct window *win);
int compositor_configure_display_arrangement(void);
int compositor_mirror_displays(uint32_t source_id, uint32_t target_id);

/* Hot-plug support */
void compositor_display_connected(uint32_t display_id);
void compositor_display_disconnected(uint32_t display_id);
int compositor_auto_configure_displays(void);