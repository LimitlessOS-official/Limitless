#pragma once
#include "kernel.h"

/*
 * Unified device model (Phase 2)
 * - Simple registry and hotplug callbacks
 * - Sysfs-like enumeration placeholder
 */

typedef enum {
    DEV_CLASS_UNKNOWN = 0,
    DEV_CLASS_NET,
    DEV_CLASS_BLOCK,
    DEV_CLASS_USB,
    DEV_CLASS_AUDIO,
} dev_class_t;

typedef struct device {
    dev_class_t cls;
    char name[32];
    void* drv;
} device_t;

typedef void (*dev_hotplug_cb)(device_t* dev, int added);

int  dev_register(device_t* d);
void dev_enumerate(void (*cb)(device_t*, void*), void* user);
void dev_set_hotplug(dev_hotplug_cb cb);