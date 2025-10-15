/*
 * LimitlessOS App Store Developer SDK Implementation
 * Provides onboarding, registration, and callback hooks for third-party developers
 */

#include "app_store.h"
#include <stdio.h>

#define MAX_SDKS 32
static struct {
    char name[APP_NAME_MAX];
    sdk_callback_t cb;
} g_sdks[MAX_SDKS];
static int g_sdk_count = 0;

int app_store_sdk_register(const char* sdk_name, sdk_callback_t cb) {
    if (!sdk_name || !cb || g_sdk_count >= MAX_SDKS) return -1;
    snprintf(g_sdks[g_sdk_count].name, APP_NAME_MAX, "%s", sdk_name);
    g_sdks[g_sdk_count].cb = cb;
    g_sdk_count++;
    return 0;
}

int app_store_sdk_invoke(const char* sdk_name, const char* app_name) {
    for (int i = 0; i < g_sdk_count; i++) {
        if (strcmp(g_sdks[i].name, sdk_name) == 0 && g_sdks[i].cb) {
            return g_sdks[i].cb(app_name);
        }
    }
    return -1;
}
