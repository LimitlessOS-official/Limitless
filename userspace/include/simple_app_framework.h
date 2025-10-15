/**
 * LimitlessOS Simple Application Framework
 * Lightweight framework for system applications
 */

#ifndef SIMPLE_APP_FRAMEWORK_H
#define SIMPLE_APP_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Application metadata */
typedef struct {
    const char* name;
    const char* version;
    const char* description;
    const char* author;
} app_info_t;

/* Simple application interface */
typedef struct {
    app_info_t info;
    int (*init)(void);
    int (*run)(void);
    void (*cleanup)(void);
} simple_app_t;

/* Utility functions */
void app_print_header(const app_info_t* info);
void app_print_separator(void);
void app_wait_for_input(void);
int app_get_choice(int min, int max);

/* Common app initialization */
#define SIMPLE_APP_INIT(app_name, app_version, app_desc) \
    static app_info_t app_info = { \
        .name = app_name, \
        .version = app_version, \
        .description = app_desc, \
        .author = "LimitlessOS Team" \
    }; \
    \
    int main(void) { \
        app_print_header(&app_info); \
        int result = app_main(); \
        app_wait_for_input(); \
        return result; \
    }

#endif /* SIMPLE_APP_FRAMEWORK_H */