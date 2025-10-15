/*
 * LimitlessOS Enhanced Graphics and Boot Transition
 * 
 * Production-grade framebuffer graphics implementation with Ubuntu-level
 * visual boot experience, splash screen, and smooth transitions.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Multiboot2 framebuffer constants */
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB 1

/* Multiboot2 framebuffer structure */
struct multiboot_tag_framebuffer_common {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint8_t reserved;
};

/* Graphics and framebuffer constants */
#define FRAMEBUFFER_TYPE_RGB        1
#define FRAMEBUFFER_TYPE_EGA_TEXT   2

/* Color definitions for 32-bit RGBA */
#define COLOR_BLACK         0xFF000000
#define COLOR_WHITE         0xFFFFFFFF
#define COLOR_BLUE          0xFF0000FF
#define COLOR_GREEN         0xFF00FF00
#define COLOR_RED           0xFFFF0000
#define COLOR_CYAN          0xFF00FFFF
#define COLOR_MAGENTA       0xFFFF00FF
#define COLOR_YELLOW        0xFFFFFF00
#define COLOR_LIGHT_GRAY    0xFFC0C0C0
#define COLOR_DARK_GRAY     0xFF808080

/* LimitlessOS brand colors */
#define LIMITLESS_PRIMARY   0xFF2E86C1    /* Professional blue */
#define LIMITLESS_SECONDARY 0xFF28B463    /* Success green */
#define LIMITLESS_ACCENT    0xFFF39C12    /* Warning amber */
#define LIMITLESS_BACKGROUND 0xFF1B2631   /* Dark background */

/* Graphics state structure */
typedef struct {
    uint32_t* framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
    uint8_t type;
    bool initialized;
    bool text_mode_fallback;
} graphics_context_t;

/* Boot animation state */
typedef struct {
    uint32_t frame;
    uint32_t total_frames;
    bool active;
    uint32_t progress;
} boot_animation_t;

/* Global graphics context */
static graphics_context_t g_graphics = {0};

/* Function prototypes */
static bool graphics_initialize(struct multiboot_tag_framebuffer_common* fb_info);
static void graphics_clear_screen(uint32_t color);
static void graphics_draw_pixel(uint32_t x, uint32_t y, uint32_t color);
static void graphics_draw_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
static void graphics_draw_filled_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
static void __attribute__((unused)) graphics_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color);
static void graphics_draw_circle(uint32_t cx, uint32_t cy, uint32_t radius, uint32_t color);
static void graphics_draw_text_simple(const char* text, uint32_t x, uint32_t y, uint32_t color);
static void graphics_show_splash_screen(void);
static void graphics_show_boot_progress(uint32_t progress);
static void graphics_animate_boot_sequence(void);
static void graphics_show_system_info(void);
static uint32_t graphics_blend_colors(uint32_t color1, uint32_t color2, uint8_t alpha);

/* Simple 8x8 bitmap font for basic text rendering */
static const uint8_t simple_font[128][8] = {
    /* Space (32) */
    [32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* ! (33) */
    [33] = {0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00},
    /* A (65) */
    [65] = {0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    /* B (66) */
    [66] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    /* C (67) */
    [67] = {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00},
    /* D (68) */
    [68] = {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00},
    /* E (69) */
    [69] = {0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7E, 0x00},
    /* F (70) */
    [70] = {0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00},
    /* G (71) */
    [71] = {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00},
    /* H (72) */
    [72] = {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    /* I (73) */
    [73] = {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
    /* L (76) */
    [76] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00},
    /* M (77) */
    [77] = {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00},
    /* N (78) */
    [78] = {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00},
    /* O (79) */
    [79] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    /* R (82) */
    [82] = {0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x00},
    /* S (83) */
    [83] = {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00},
    /* T (84) */
    [84] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    /* U (85) */
    [85] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    /* Numbers */
    [48] = {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00}, /* 0 */
    [49] = {0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7E, 0x00}, /* 1 */
    [50] = {0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E, 0x00}, /* 2 */
    [51] = {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00}, /* 3 */
    [52] = {0x06, 0x0E, 0x1E, 0x66, 0x7F, 0x06, 0x06, 0x00}, /* 4 */
    [53] = {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00}, /* 5 */
    /* Dot */
    [46] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, /* . */
};

/* Graphics initialization */
static bool graphics_initialize(struct multiboot_tag_framebuffer_common* fb_info) {
    if (!fb_info) {
        g_graphics.text_mode_fallback = true;
        return false;
    }
    
    /* Check if we have a valid framebuffer */
    if (fb_info->framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB) {
        g_graphics.text_mode_fallback = true;
        return false;
    }
    
    /* Initialize graphics context */
    g_graphics.framebuffer = (uint32_t*)(uintptr_t)fb_info->framebuffer_addr;
    g_graphics.width = fb_info->framebuffer_width;
    g_graphics.height = fb_info->framebuffer_height;
    g_graphics.pitch = fb_info->framebuffer_pitch;
    g_graphics.bpp = fb_info->framebuffer_bpp;
    g_graphics.type = fb_info->framebuffer_type;
    g_graphics.initialized = true;
    g_graphics.text_mode_fallback = false;
    
    /* Verify reasonable resolution */
    if (g_graphics.width < 640 || g_graphics.height < 480) {
        g_graphics.text_mode_fallback = true;
        return false;
    }
    
    /* Clear screen to black */
    graphics_clear_screen(COLOR_BLACK);
    
    return true;
}

/* Clear entire screen with specified color */
static void graphics_clear_screen(uint32_t color) {
    if (!g_graphics.initialized || g_graphics.text_mode_fallback) {
        return;
    }
    
    for (uint32_t y = 0; y < g_graphics.height; y++) {
        for (uint32_t x = 0; x < g_graphics.width; x++) {
            graphics_draw_pixel(x, y, color);
        }
    }
}

/* Draw a single pixel */
static void graphics_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!g_graphics.initialized || g_graphics.text_mode_fallback) {
        return;
    }
    
    if (x >= g_graphics.width || y >= g_graphics.height) {
        return;
    }
    
    uint32_t pixel_offset = y * (g_graphics.pitch / 4) + x;
    g_graphics.framebuffer[pixel_offset] = color;
}

/* Draw a rectangle outline */
static void graphics_draw_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    /* Top and bottom edges */
    for (uint32_t i = 0; i < w; i++) {
        graphics_draw_pixel(x + i, y, color);
        graphics_draw_pixel(x + i, y + h - 1, color);
    }
    
    /* Left and right edges */
    for (uint32_t i = 0; i < h; i++) {
        graphics_draw_pixel(x, y + i, color);
        graphics_draw_pixel(x + w - 1, y + i, color);
    }
}

/* Draw a filled rectangle */
static void graphics_draw_filled_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t row = 0; row < h; row++) {
        for (uint32_t col = 0; col < w; col++) {
            graphics_draw_pixel(x + col, y + row, color);
        }
    }
}

/* Simple abs function for freestanding environment */
static int simple_abs(int x) {
    return (x < 0) ? -x : x;
}

/* Draw a line using Bresenham's algorithm */
static void __attribute__((unused)) graphics_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color) {
    int dx = simple_abs((int)x2 - (int)x1);
    int dy = simple_abs((int)y2 - (int)y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        graphics_draw_pixel(x1, y1, color);
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

/* Draw a circle outline */
static void graphics_draw_circle(uint32_t cx, uint32_t cy, uint32_t radius, uint32_t color) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;
    
    while (y >= x) {
        /* Draw 8 octants */
        graphics_draw_pixel(cx + x, cy + y, color);
        graphics_draw_pixel(cx - x, cy + y, color);
        graphics_draw_pixel(cx + x, cy - y, color);
        graphics_draw_pixel(cx - x, cy - y, color);
        graphics_draw_pixel(cx + y, cy + x, color);
        graphics_draw_pixel(cx - y, cy + x, color);
        graphics_draw_pixel(cx + y, cy - x, color);
        graphics_draw_pixel(cx - y, cy - x, color);
        
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

/* Draw simple text using bitmap font */
static void graphics_draw_text_simple(const char* text, uint32_t x, uint32_t y, uint32_t color) {
    uint32_t start_x = x;
    
    while (*text) {
        char c = *text;
        
        /* Handle newlines */
        if (c == '\n') {
            x = start_x;
            y += 10;
            text++;
            continue;
        }
        
        /* Draw character if we have a bitmap for it */
        if ((unsigned char)c < 128) {
            for (int row = 0; row < 8; row++) {
                uint8_t line = simple_font[(int)c][row];
                for (int col = 0; col < 8; col++) {
                    if (line & (0x80 >> col)) {
                        graphics_draw_pixel(x + col, y + row, color);
                    }
                }
            }
        }
        
        x += 9;  /* 8 pixels + 1 for spacing */
        text++;
    }
}

/* Blend two colors with alpha */
static uint32_t graphics_blend_colors(uint32_t color1, uint32_t color2, uint8_t alpha) {
    uint8_t r1 = (color1 >> 16) & 0xFF;
    uint8_t g1 = (color1 >> 8) & 0xFF;
    uint8_t b1 = color1 & 0xFF;
    
    uint8_t r2 = (color2 >> 16) & 0xFF;
    uint8_t g2 = (color2 >> 8) & 0xFF;
    uint8_t b2 = color2 & 0xFF;
    
    uint8_t r = ((r1 * alpha) + (r2 * (255 - alpha))) / 255;
    uint8_t g = ((g1 * alpha) + (g2 * (255 - alpha))) / 255;
    uint8_t b = ((b1 * alpha) + (b2 * (255 - alpha))) / 255;
    
    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

/* Show LimitlessOS splash screen */
static void graphics_show_splash_screen(void) {
    if (!g_graphics.initialized || g_graphics.text_mode_fallback) {
        return;
    }
    
    /* Clear screen with dark background */
    graphics_clear_screen(LIMITLESS_BACKGROUND);
    
    uint32_t center_x = g_graphics.width / 2;
    uint32_t center_y = g_graphics.height / 2;
    
    /* Draw LimitlessOS logo (simplified geometric design) */
    
    /* Draw outer circle (represents limitless possibilities) */
    graphics_draw_circle(center_x, center_y - 50, 80, LIMITLESS_PRIMARY);
    graphics_draw_circle(center_x, center_y - 50, 78, LIMITLESS_PRIMARY);
    
    /* Draw inner elements */
    graphics_draw_filled_rectangle(center_x - 40, center_y - 70, 80, 8, LIMITLESS_SECONDARY);
    graphics_draw_filled_rectangle(center_x - 30, center_y - 50, 60, 8, LIMITLESS_SECONDARY);
    graphics_draw_filled_rectangle(center_x - 20, center_y - 30, 40, 8, LIMITLESS_SECONDARY);
    
    /* Draw company name */
    graphics_draw_text_simple("LimitlessOS", center_x - 45, center_y + 50, COLOR_WHITE);
    graphics_draw_text_simple("Enterprise v1.0.0", center_x - 70, center_y + 70, LIMITLESS_PRIMARY);
    
    /* Draw tagline */
    graphics_draw_text_simple("The Future of Computing", center_x - 90, center_y + 100, LIMITLESS_ACCENT);
    
    /* Draw progress bar background */
    graphics_draw_filled_rectangle(center_x - 150, center_y + 140, 300, 20, COLOR_DARK_GRAY);
    graphics_draw_rectangle(center_x - 150, center_y + 140, 300, 20, COLOR_WHITE);
}

/* Show boot progress */
static void graphics_show_boot_progress(uint32_t progress) {
    if (!g_graphics.initialized || g_graphics.text_mode_fallback) {
        return;
    }
    
    uint32_t center_x = g_graphics.width / 2;
    uint32_t center_y = g_graphics.height / 2;
    
    /* Update progress bar */
    uint32_t bar_width = (300 * progress) / 100;
    
    /* Clear previous progress */
    graphics_draw_filled_rectangle(center_x - 148, center_y + 142, 296, 16, COLOR_DARK_GRAY);
    
    /* Draw new progress */
    if (bar_width > 0) {
        graphics_draw_filled_rectangle(center_x - 148, center_y + 142, bar_width, 16, LIMITLESS_SECONDARY);
    }
    
    /* Show loading text */
    char progress_text[32];
    /* Simple integer to string conversion */
    progress_text[0] = 'L'; progress_text[1] = 'o'; progress_text[2] = 'a';
    progress_text[3] = 'd'; progress_text[4] = 'i'; progress_text[5] = 'n';
    progress_text[6] = 'g'; progress_text[7] = '.'; progress_text[8] = '.';
    progress_text[9] = '.'; progress_text[10] = ' ';
    
    /* Add percentage */
    if (progress >= 100) {
        progress_text[11] = '1'; progress_text[12] = '0'; progress_text[13] = '0';
        progress_text[14] = '%'; progress_text[15] = '\0';
    } else {
        progress_text[11] = '0' + (progress / 10);
        progress_text[12] = '0' + (progress % 10);
        progress_text[13] = '%';
        progress_text[14] = '\0';
    }
    
    graphics_draw_text_simple(progress_text, center_x - 60, center_y + 170, COLOR_WHITE);
}

/* Animate boot sequence */
static void graphics_animate_boot_sequence(void) {
    if (!g_graphics.initialized || g_graphics.text_mode_fallback) {
        return;
    }
    
    /* Show splash screen */
    graphics_show_splash_screen();
    
    /* Simulate boot progress */
    const char* boot_stages[] = {
        "Initializing hardware...",
        "Loading drivers...",
        "Starting services...",
        "Preparing desktop...",
        "Ready!"
    };
    
    uint32_t center_x = g_graphics.width / 2;
    uint32_t center_y = g_graphics.height / 2;
    
    for (int stage = 0; stage < 5; stage++) {
        /* Clear status area */
        graphics_draw_filled_rectangle(center_x - 150, center_y + 200, 300, 20, LIMITLESS_BACKGROUND);
        
        /* Show current stage */
        graphics_draw_text_simple(boot_stages[stage], center_x - 80, center_y + 200, LIMITLESS_ACCENT);
        
        /* Animate progress */
        for (int progress = stage * 20; progress <= (stage + 1) * 20; progress++) {
            graphics_show_boot_progress(progress);
            
            /* Simple delay (not accurate timing, but visual effect) */
            for (volatile int i = 0; i < 1000000; i++);
        }
    }
    
    /* Final completion message */
    graphics_draw_filled_rectangle(center_x - 150, center_y + 200, 300, 20, LIMITLESS_BACKGROUND);
    graphics_draw_text_simple("Boot complete! Starting LimitlessOS...", center_x - 140, center_y + 200, LIMITLESS_SECONDARY);
}

/* Show system information overlay */
static void graphics_show_system_info(void) {
    if (!g_graphics.initialized || g_graphics.text_mode_fallback) {
        return;
    }
    
    uint32_t info_x = 20;
    uint32_t info_y = 20;
    
    /* Draw semi-transparent background */
    graphics_draw_filled_rectangle(info_x - 10, info_y - 10, 300, 150, 
                                 graphics_blend_colors(COLOR_BLACK, LIMITLESS_BACKGROUND, 180));
    graphics_draw_rectangle(info_x - 10, info_y - 10, 300, 150, LIMITLESS_PRIMARY);
    
    /* Display system information */
    graphics_draw_text_simple("System Information:", info_x, info_y, COLOR_WHITE);
    graphics_draw_text_simple("Resolution: 1024x768x32", info_x, info_y + 20, LIMITLESS_ACCENT);
    graphics_draw_text_simple("Memory: Available", info_x, info_y + 40, LIMITLESS_ACCENT);
    graphics_draw_text_simple("Graphics: Framebuffer", info_x, info_y + 60, LIMITLESS_ACCENT);
    graphics_draw_text_simple("Status: Ready", info_x, info_y + 80, LIMITLESS_SECONDARY);
    
    char resolution[64];
    /* Simple formatting */
    resolution[0] = 'R'; resolution[1] = 'e'; resolution[2] = 's'; resolution[3] = ':';
    resolution[4] = ' '; resolution[5] = '0' + (g_graphics.width / 1000);
    resolution[6] = '0' + ((g_graphics.width / 100) % 10);
    resolution[7] = '0' + ((g_graphics.width / 10) % 10);
    resolution[8] = '0' + (g_graphics.width % 10);
    resolution[9] = 'x';
    resolution[10] = '0' + (g_graphics.height / 1000);
    resolution[11] = '0' + ((g_graphics.height / 100) % 10);
    resolution[12] = '0' + ((g_graphics.height / 10) % 10);
    resolution[13] = '0' + (g_graphics.height % 10);
    resolution[14] = '\0';
    
    graphics_draw_text_simple(resolution, info_x, info_y + 100, COLOR_WHITE);
}

/* Public interface functions */
bool initialize_graphics_system(struct multiboot_tag_framebuffer_common* fb_info) {
    return graphics_initialize(fb_info);
}

void show_boot_splash(void) {
    graphics_animate_boot_sequence();
}

void display_system_info(void) {
    graphics_show_system_info();
}

bool is_graphics_available(void) {
    return g_graphics.initialized && !g_graphics.text_mode_fallback;
}

void graphics_cleanup(void) {
    if (g_graphics.initialized) {
        graphics_clear_screen(COLOR_BLACK);
        g_graphics.initialized = false;
    }
}