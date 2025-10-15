/*
 * LimitlessOS UI Theme Implementation
 * Provides customizable themes, layouts, and high-contrast support
 */

#include "ui_theme.h"
#include <stdio.h>
#include <string.h>

#define MAX_THEMES 16
static char g_themes[MAX_THEMES][32];
static int g_theme_count = 0;
static int g_current_theme = 0;

int ui_theme_add(const char* theme_name) {
    if (!theme_name || g_theme_count >= MAX_THEMES) return -1;
    strncpy(g_themes[g_theme_count], theme_name, 31);
    g_themes[g_theme_count][31] = 0;
    g_theme_count++;
    return 0;
}

int ui_theme_set(int theme_id) {
    if (theme_id < 0 || theme_id >= g_theme_count) return -1;
    g_current_theme = theme_id;
    printf("UI theme set to '%s'.\n", g_themes[theme_id]);
    // Apply theme system-wide
    limitlessui_apply_theme(g_themes[theme_id]);
    return 0;
}

void ui_theme_set_high_contrast(void) {
    // Set high-contrast theme
    int hc_id = -1;
    for (int i = 0; i < g_theme_count; i++) {
        if (strcmp(g_themes[i], "HighContrast") == 0) hc_id = i;
    }
    if (hc_id >= 0) {
        ui_theme_set(hc_id);
    } else {
        ui_theme_add("HighContrast");
        ui_theme_set(g_theme_count-1);
    }
}

const char* ui_theme_get(void) {
    return g_themes[g_current_theme];
}
