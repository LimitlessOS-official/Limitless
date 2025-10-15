/**
 * LimitlessOS Window Manager and Compositor
 * 
 * Advanced window management system with compositing, animations, effects,
 * and modern desktop features comparable to Windows DWM, macOS Quartz,
 * and Linux compositors like Mutter/KWin.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "graphics_gui.h"
#include "real_hardware.h"
#include "smp.h"
#include <math.h>

/* Window Manager Configuration */
#define WM_MAX_WORKSPACES       10
#define WM_ANIMATION_DURATION   250   /* milliseconds */
#define WM_SHADOW_SIZE          8
#define WM_BLUR_RADIUS          4

/* Window States */
typedef enum {
    WIN_STATE_NORMAL,
    WIN_STATE_MINIMIZED,
    WIN_STATE_MAXIMIZED,
    WIN_STATE_FULLSCREEN,
    WIN_STATE_TILED_LEFT,
    WIN_STATE_TILED_RIGHT,
    WIN_STATE_ANIMATING
} window_state_t;

/* Animation Types */
typedef enum {
    ANIM_NONE,
    ANIM_FADE_IN,
    ANIM_FADE_OUT,
    ANIM_SLIDE_IN,
    ANIM_SLIDE_OUT,
    ANIM_SCALE_IN,
    ANIM_SCALE_OUT,
    ANIM_FLIP,
    ANIM_WOBBLE
} animation_type_t;

/* Window Animation */
typedef struct window_animation {
    animation_type_t type;          /* Animation type */
    uint64_t start_time;            /* Start timestamp */
    uint32_t duration;              /* Duration in milliseconds */
    float progress;                 /* Animation progress (0.0 - 1.0) */
    
    /* Animation parameters */
    struct {
        float start_x, start_y;     /* Starting position */
        float end_x, end_y;         /* Ending position */
        float start_scale;          /* Starting scale */
        float end_scale;            /* Ending scale */
        float start_alpha;          /* Starting alpha */
        float end_alpha;            /* Ending alpha */
        float start_rotation;       /* Starting rotation */
        float end_rotation;         /* Ending rotation */
    } params;
    
    void (*on_complete)(struct window *win);  /* Completion callback */
} window_animation_t;

/* Workspace */
typedef struct workspace {
    uint32_t id;                    /* Workspace ID */
    char name[32];                  /* Workspace name */
    window_t *windows;              /* Windows in workspace */
    window_t *focused_window;       /* Focused window */
    uint32_t window_count;          /* Number of windows */
    
    /* Layout */
    enum {
        LAYOUT_FLOATING,
        LAYOUT_TILED,
        LAYOUT_MONOCLE,
        LAYOUT_GRID
    } layout_mode;
    
    uint32_t background_color;      /* Workspace background */
    struct workspace *next;         /* Next workspace */
} workspace_t;

/* Window Manager Context */
typedef struct window_manager {
    workspace_t *workspaces;        /* Workspace list */
    workspace_t *current_workspace; /* Current workspace */
    uint32_t workspace_count;       /* Number of workspaces */
    
    /* Global window list */
    window_t *all_windows;          /* All windows across workspaces */
    uint32_t total_windows;         /* Total window count */
    
    /* Focus management */
    window_t *focused_window;       /* Globally focused window */
    window_t **focus_stack;         /* Focus history stack */
    uint32_t focus_stack_size;      /* Stack size */
    
    /* Window effects */
    bool shadows_enabled;           /* Drop shadows */
    bool transparency_enabled;      /* Window transparency */
    bool blur_enabled;              /* Background blur */
    bool animations_enabled;        /* Window animations */
    
    /* Compositor state */
    bool vsync_enabled;             /* V-Sync */
    uint32_t frame_rate;            /* Target frame rate */
    uint64_t last_frame_time;       /* Last frame timestamp */
    uint32_t frame_count;           /* Frame counter */
    
    /* Hot corners and edges */
    struct {
        bool top_left;              /* Show overview */
        bool top_right;             /* Show desktop */
        bool bottom_left;           /* Application launcher */
        bool bottom_right;          /* System settings */
    } hot_corners;
    
    spinlock_t wm_lock;             /* Window manager lock */
} window_manager_t;

/* Global window manager */
static window_manager_t g_wm = {0};

/* Animation easing functions */
static float ease_in_out_cubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

static float ease_out_back(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
}

static float ease_out_elastic(float t) {
    const float c4 = (2.0f * M_PI) / 3.0f;
    return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : 
           powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
}

/**
 * Initialize window manager
 */
int window_manager_init(void) {
    memset(&g_wm, 0, sizeof(window_manager_t));
    
    g_wm.shadows_enabled = true;
    g_wm.transparency_enabled = true;
    g_wm.blur_enabled = true;
    g_wm.animations_enabled = true;
    g_wm.vsync_enabled = true;
    g_wm.frame_rate = 60;
    
    // Enable all hot corners
    g_wm.hot_corners.top_left = true;
    g_wm.hot_corners.top_right = true;
    g_wm.hot_corners.bottom_left = true;
    g_wm.hot_corners.bottom_right = true;
    
    spin_lock_init(&g_wm.wm_lock);
    
    // Create default workspace
    workspace_t *default_ws = create_workspace("Desktop");
    if (default_ws) {
        g_wm.current_workspace = default_ws;
    }
    
    printk("Window manager initialized\n");
    return 0;
}

/**
 * Create workspace
 */
workspace_t *create_workspace(const char *name) {
    workspace_t *workspace = kmalloc(sizeof(workspace_t));
    if (!workspace) return NULL;
    
    memset(workspace, 0, sizeof(workspace_t));
    
    static uint32_t workspace_id = 0;
    workspace->id = workspace_id++;
    strncpy(workspace->name, name, sizeof(workspace->name) - 1);
    workspace->layout_mode = LAYOUT_FLOATING;
    workspace->background_color = DESKTOP_WALLPAPER_COLOR;
    
    spin_lock(&g_wm.wm_lock);
    
    workspace->next = g_wm.workspaces;
    g_wm.workspaces = workspace;
    g_wm.workspace_count++;
    
    spin_unlock(&g_wm.wm_lock);
    
    printk("Created workspace: %s (ID: %u)\n", name, workspace->id);
    return workspace;
}

/**
 * Switch to workspace
 */
void switch_workspace(uint32_t workspace_id) {
    spin_lock(&g_wm.wm_lock);
    
    workspace_t *workspace = g_wm.workspaces;
    while (workspace) {
        if (workspace->id == workspace_id) {
            g_wm.current_workspace = workspace;
            g_wm.focused_window = workspace->focused_window;
            break;
        }
        workspace = workspace->next;
    }
    
    spin_unlock(&g_wm.wm_lock);
    
    if (workspace) {
        printk("Switched to workspace: %s\n", workspace->name);
        
        // Trigger workspace switch animation
        animate_workspace_switch();
        
        // Request compositor update
        request_compositor_update();
    }
}

/**
 * Add window to workspace
 */
void add_window_to_workspace(window_t *window, workspace_t *workspace) {
    if (!window || !workspace) return;
    
    spin_lock(&g_wm.wm_lock);
    
    // Remove from current workspace if any
    workspace_t *ws = g_wm.workspaces;
    while (ws) {
        window_t **current = &ws->windows;
        while (*current) {
            if (*current == window) {
                *current = window->next;
                ws->window_count--;
                break;
            }
            current = &(*current)->next;
        }
        ws = ws->next;
    }
    
    // Add to new workspace
    window->next = workspace->windows;
    workspace->windows = window;
    workspace->window_count++;
    
    // Focus new window
    workspace->focused_window = window;
    if (workspace == g_wm.current_workspace) {
        g_wm.focused_window = window;
    }
    
    spin_unlock(&g_wm.wm_lock);
    
    // Animate window appearance
    if (g_wm.animations_enabled) {
        animate_window_in(window);
    }
}

/**
 * Focus window
 */
void focus_window(window_t *window) {
    if (!window) return;
    
    spin_lock(&g_wm.wm_lock);
    
    // Update global focus
    g_wm.focused_window = window;
    
    // Update workspace focus
    workspace_t *workspace = g_wm.workspaces;
    while (workspace) {
        window_t *win = workspace->windows;
        while (win) {
            if (win == window) {
                workspace->focused_window = window;
                win->focused = true;
            } else {
                win->focused = false;
            }
            win = win->next;
        }
        workspace = workspace->next;
    }
    
    // Update focus stack
    // TODO: Implement focus history
    
    spin_unlock(&g_wm.wm_lock);
    
    printk("Focused window: %s (ID: %u)\n", window->title, window->id);
}

/**
 * Minimize window
 */
void minimize_window(window_t *window) {
    if (!window || window->minimized) return;
    
    window->minimized = true;
    window->visible = false;
    
    if (g_wm.animations_enabled) {
        animate_window_minimize(window);
    }
    
    // Find next window to focus
    workspace_t *workspace = g_wm.current_workspace;
    if (workspace && workspace->focused_window == window) {
        window_t *next_window = workspace->windows;
        while (next_window && (next_window == window || next_window->minimized)) {
            next_window = next_window->next;
        }
        
        if (next_window) {
            focus_window(next_window);
        }
    }
    
    printk("Minimized window: %s\n", window->title);
}

/**
 * Maximize window
 */
void maximize_window(window_t *window) {
    if (!window || window->maximized) return;
    
    // Store original size and position
    window->restore_x = window->x;
    window->restore_y = window->y;
    window->restore_width = window->width;
    window->restore_height = window->height;
    
    // Set to screen size
    display_t *display = g_gui.primary_display;
    if (display) {
        window->x = 0;
        window->y = 0;
        window->width = display->framebuffer.width;
        window->height = display->framebuffer.height - 32;  // Account for taskbar
        
        window->maximized = true;
        
        if (g_wm.animations_enabled) {
            animate_window_maximize(window);
        }
        
        printk("Maximized window: %s\n", window->title);
    }
}

/**
 * Restore window
 */
void restore_window(window_t *window) {
    if (!window) return;
    
    if (window->minimized) {
        window->minimized = false;
        window->visible = true;
        
        if (g_wm.animations_enabled) {
            animate_window_restore(window);
        }
    }
    
    if (window->maximized) {
        window->maximized = false;
        
        // Restore original size and position
        window->x = window->restore_x;
        window->y = window->restore_y;
        window->width = window->restore_width;
        window->height = window->restore_height;
        
        if (g_wm.animations_enabled) {
            animate_window_unmaximize(window);
        }
    }
    
    printk("Restored window: %s\n", window->title);
}

/**
 * Close window
 */
void close_window(window_t *window) {
    if (!window) return;
    
    // Call window close handler
    if (window->on_close) {
        window->on_close(window);
    }
    
    if (g_wm.animations_enabled) {
        animate_window_close(window);
    } else {
        destroy_window(window);
    }
}

/**
 * Tile windows
 */
void tile_windows_horizontally(void) {
    workspace_t *workspace = g_wm.current_workspace;
    if (!workspace || workspace->window_count == 0) return;
    
    display_t *display = g_gui.primary_display;
    if (!display) return;
    
    uint32_t available_width = display->framebuffer.width;
    uint32_t available_height = display->framebuffer.height - 32;  // Taskbar
    uint32_t window_width = available_width / workspace->window_count;
    
    uint32_t x = 0;
    window_t *window = workspace->windows;
    while (window) {
        if (!window->minimized) {
            window->x = x;
            window->y = 0;
            window->width = window_width;
            window->height = available_height;
            
            x += window_width;
        }
        window = window->next;
    }
    
    workspace->layout_mode = LAYOUT_TILED;
    printk("Tiled windows horizontally\n");
}

/**
 * Show window overview (Expose/Mission Control style)
 */
void show_window_overview(void) {
    if (!g_wm.current_workspace || g_wm.current_workspace->window_count == 0) {
        return;
    }
    
    display_t *display = g_gui.primary_display;
    if (!display) return;
    
    // Calculate grid layout for overview
    uint32_t window_count = g_wm.current_workspace->window_count;
    uint32_t cols = (uint32_t)ceil(sqrt(window_count));
    uint32_t rows = (window_count + cols - 1) / cols;
    
    uint32_t thumb_width = display->framebuffer.width / cols;
    uint32_t thumb_height = display->framebuffer.height / rows;
    
    uint32_t index = 0;
    window_t *window = g_wm.current_workspace->windows;
    while (window) {
        if (!window->minimized) {
            uint32_t col = index % cols;
            uint32_t row = index / cols;
            
            // Store original position
            window->overview_restore_x = window->x;
            window->overview_restore_y = window->y;
            window->overview_restore_width = window->width;
            window->overview_restore_height = window->height;
            
            // Set overview position
            window->x = col * thumb_width + 20;
            window->y = row * thumb_height + 20;
            window->width = thumb_width - 40;
            window->height = thumb_height - 40;
            
            // Animate to overview position
            if (g_wm.animations_enabled) {
                animate_window_to_overview(window);
            }
            
            index++;
        }
        window = window->next;
    }
    
    printk("Showing window overview (%u windows)\n", window_count);
}

/**
 * Animate window in
 */
void animate_window_in(window_t *window) {
    window_animation_t *anim = &window->animation;
    
    anim->type = ANIM_SCALE_IN;
    anim->start_time = get_system_time();
    anim->duration = WM_ANIMATION_DURATION;
    anim->progress = 0.0f;
    
    anim->params.start_scale = 0.0f;
    anim->params.end_scale = 1.0f;
    anim->params.start_alpha = 0.0f;
    anim->params.end_alpha = 1.0f;
    
    window->state = WIN_STATE_ANIMATING;
}

/**
 * Animate window minimize
 */
void animate_window_minimize(window_t *window) {
    window_animation_t *anim = &window->animation;
    
    anim->type = ANIM_SCALE_OUT;
    anim->start_time = get_system_time();
    anim->duration = WM_ANIMATION_DURATION;
    anim->progress = 0.0f;
    
    anim->params.start_scale = 1.0f;
    anim->params.end_scale = 0.1f;
    anim->params.start_alpha = 1.0f;
    anim->params.end_alpha = 0.0f;
    
    // Animate to taskbar position
    display_t *display = g_gui.primary_display;
    if (display) {
        anim->params.start_x = window->x;
        anim->params.start_y = window->y;
        anim->params.end_x = 100;  // Taskbar position
        anim->params.end_y = display->framebuffer.height - 16;
    }
    
    window->state = WIN_STATE_ANIMATING;
    
    // Set completion callback
    anim->on_complete = minimize_animation_complete;
}

/**
 * Update window animations
 */
void update_window_animations(void) {
    uint64_t current_time = get_system_time();
    
    window_t *window = g_wm.all_windows;
    while (window) {
        if (window->state == WIN_STATE_ANIMATING) {
            window_animation_t *anim = &window->animation;
            
            // Calculate progress
            uint64_t elapsed = current_time - anim->start_time;
            anim->progress = (float)elapsed / (float)anim->duration;
            
            if (anim->progress >= 1.0f) {
                anim->progress = 1.0f;
                window->state = WIN_STATE_NORMAL;
                
                // Call completion callback
                if (anim->on_complete) {
                    anim->on_complete(window);
                }
            }
            
            // Apply animation based on type
            apply_window_animation(window, anim);
        }
        window = window->next;
    }
}

/**
 * Apply window animation
 */
void apply_window_animation(window_t *window, window_animation_t *anim) {
    float t = anim->progress;
    
    switch (anim->type) {
        case ANIM_SCALE_IN:
        case ANIM_SCALE_OUT: {
            float eased_t = ease_out_back(t);
            float scale = anim->params.start_scale + 
                         (anim->params.end_scale - anim->params.start_scale) * eased_t;
            
            // Apply scale to window (would need proper implementation)
            window->scale = scale;
            break;
        }
        
        case ANIM_FADE_IN:
        case ANIM_FADE_OUT: {
            float eased_t = ease_in_out_cubic(t);
            window->alpha = anim->params.start_alpha + 
                           (anim->params.end_alpha - anim->params.start_alpha) * eased_t;
            break;
        }
        
        case ANIM_SLIDE_IN:
        case ANIM_SLIDE_OUT: {
            float eased_t = ease_out_elastic(t);
            window->x = anim->params.start_x + 
                       (anim->params.end_x - anim->params.start_x) * eased_t;
            window->y = anim->params.start_y + 
                       (anim->params.end_y - anim->params.start_y) * eased_t;
            break;
        }
        
        default:
            break;
    }
}

/**
 * Compositor main loop
 */
void compositor_main_loop(void) {
    uint64_t target_frame_time = 1000000 / g_wm.frame_rate;  // microseconds
    uint64_t last_frame = get_system_time();
    
    while (true) {
        uint64_t current_time = get_system_time();
        uint64_t frame_delta = current_time - last_frame;
        
        if (frame_delta >= target_frame_time) {
            // Update animations
            update_window_animations();
            
            // Composite frame
            composite_frame();
            
            // Update frame tracking
            g_wm.frame_count++;
            g_wm.last_frame_time = current_time;
            last_frame = current_time;
            
            // VSync delay if enabled
            if (g_wm.vsync_enabled) {
                // Wait for VBlank (hardware-specific implementation needed)
                wait_for_vblank();
            }
        } else {
            // Sleep for remaining frame time
            uint64_t sleep_time = target_frame_time - frame_delta;
            usleep(sleep_time);
        }
    }
}

/**
 * Handle hot corner activation
 */
void handle_hot_corner(uint32_t corner) {
    switch (corner) {
        case 0: // Top-left: Show overview
            if (g_wm.hot_corners.top_left) {
                show_window_overview();
            }
            break;
            
        case 1: // Top-right: Show desktop
            if (g_wm.hot_corners.top_right) {
                show_desktop();
            }
            break;
            
        case 2: // Bottom-left: Application launcher
            if (g_wm.hot_corners.bottom_left) {
                show_application_launcher();
            }
            break;
            
        case 3: // Bottom-right: System settings
            if (g_wm.hot_corners.bottom_right) {
                show_system_settings();
            }
            break;
    }
}

/**
 * Window manager status
 */
void window_manager_status(void) {
    printk("Window Manager Status:\n");
    printk("=====================\n");
    
    printk("Workspaces: %u\n", g_wm.workspace_count);
    if (g_wm.current_workspace) {
        printk("Current Workspace: %s (%u windows)\n", 
               g_wm.current_workspace->name, g_wm.current_workspace->window_count);
    }
    
    printk("Total Windows: %u\n", g_wm.total_windows);
    printk("Frame Rate: %u FPS (Target: %u)\n", 
           calculate_fps(), g_wm.frame_rate);
    printk("VSync: %s\n", g_wm.vsync_enabled ? "Enabled" : "Disabled");
    
    printk("Effects:\n");
    printk("  Shadows: %s\n", g_wm.shadows_enabled ? "On" : "Off");
    printk("  Transparency: %s\n", g_wm.transparency_enabled ? "On" : "Off");
    printk("  Blur: %s\n", g_wm.blur_enabled ? "On" : "Off");
    printk("  Animations: %s\n", g_wm.animations_enabled ? "On" : "Off");
}

/**
 * Test window manager
 */
void test_window_manager(void) {
    printk("Testing window manager and compositor...\n");
    
    // Create multiple workspaces
    create_workspace("Development");
    create_workspace("Web Browsing");
    create_workspace("Media");
    
    // Create test windows
    window_t *terminal = create_window("Terminal", 100, 100, 600, 400);
    window_t *editor = create_window("Code Editor", 200, 150, 800, 600);
    window_t *browser = create_window("Web Browser", 300, 200, 900, 700);
    
    if (terminal && editor && browser) {
        // Add windows to workspaces
        add_window_to_workspace(terminal, g_wm.current_workspace);
        add_window_to_workspace(editor, g_wm.current_workspace);
        
        workspace_t *web_ws = g_wm.workspaces->next;  // Second workspace
        if (web_ws) {
            add_window_to_workspace(browser, web_ws);
        }
        
        // Test window operations
        focus_window(editor);
        maximize_window(editor);
        
        sleep(1000);  // 1 second
        
        restore_window(editor);
        minimize_window(terminal);
        
        // Test tiling
        tile_windows_horizontally();
        
        // Test workspace switching
        switch_workspace(web_ws->id);
        
        // Show overview
        show_window_overview();
    }
    
    window_manager_status();
    
    printk("Window manager test completed\n");
}

/* Export window manager functions */
EXPORT_SYMBOL(window_manager_init);
EXPORT_SYMBOL(create_workspace);
EXPORT_SYMBOL(switch_workspace);
EXPORT_SYMBOL(focus_window);
EXPORT_SYMBOL(minimize_window);
EXPORT_SYMBOL(maximize_window);
EXPORT_SYMBOL(restore_window);
EXPORT_SYMBOL(close_window);
EXPORT_SYMBOL(show_window_overview);
EXPORT_SYMBOL(compositor_main_loop);