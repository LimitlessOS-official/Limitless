#ifndef LIMITLESS_APP_STORE_H
#define LIMITLESS_APP_STORE_H

#include <stdint.h>

#define APP_NAME_MAX 64
#define APP_PUBLISHER_MAX 64
#define APP_MANIFEST_MAX 256

typedef enum {
    APP_TYPE_NATIVE = 0,
    APP_TYPE_WEB,
    APP_TYPE_CONTAINER,
    APP_TYPE_COMPAT,
    APP_TYPE_WINDOWS,
    APP_TYPE_ANDROID
} app_type_t;

#define APP_PERM_DEFAULT 0x0001
#define APP_COMPAT_NONE  0

typedef struct {
    char name[APP_NAME_MAX];
    char publisher[APP_PUBLISHER_MAX];
    app_type_t type;
    char manifest[APP_MANIFEST_MAX];
    int sandboxed;
    int permissions;

// App permission bitmask defines
#define APP_PERM_NETWORK     0x01
#define APP_PERM_FILESYSTEM  0x02
    int compat_layer;
} app_entry_t;

typedef int (*sdk_callback_t)(const char* app_name);

int app_store_register(const char* name, const char* publisher, app_type_t type, const char* manifest);
int app_store_list(app_entry_t* out, uint32_t max);
int app_store_launch(const char* name);
int app_store_remove(const char* name);
int app_store_sdk_register(const char* sdk_name, sdk_callback_t cb);

#endif // LIMITLESS_APP_STORE_H
