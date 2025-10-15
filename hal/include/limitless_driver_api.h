// LimitlessOS Kernel Driver API (Core Header)
// Advanced, extensible, and secure driver interface for all hardware classes
// Copyright (c) LimitlessOS Project

#ifndef LIMITLESS_DRIVER_API_H
#define LIMITLESS_DRIVER_API_H

#include <stdint.h>
#include <stddef.h>
#include "hal_core.h" // Core kernel hardware abstraction

// Driver Manifest: Metadata for registration and security
struct limitless_driver_manifest {
    const char *name;
    const char *vendor;
    uint32_t version_major;
    uint32_t version_minor;
    uint32_t supported_device_class;
    const char *signature; // cryptographic signature
};

// Driver Lifecycle States
typedef enum {
    DRIVER_STATE_UNREGISTERED = 0,
    DRIVER_STATE_REGISTERED,
    DRIVER_STATE_INITIALIZED,
    DRIVER_STATE_ACTIVE,
    DRIVER_STATE_ERROR,
    DRIVER_STATE_UNLOADED
} limitless_driver_state_t;

// Device Event Types
typedef enum {
    DEVICE_EVENT_NONE = 0,
    DEVICE_EVENT_INIT,
    DEVICE_EVENT_SHUTDOWN,
    DEVICE_EVENT_HOTPLUG,
    DEVICE_EVENT_UNPLUG,
    DEVICE_EVENT_ERROR,
    DEVICE_EVENT_POWER_CHANGE,
    DEVICE_EVENT_CUSTOM
} limitless_device_event_t;

// Driver Core Interface
struct limitless_driver_ops {
    int (*probe)(void *device_info);
    int (*init)(void *device_info);
    int (*shutdown)(void *device_info);
    int (*handle_event)(void *device_info, limitless_device_event_t event, void *event_data);
    int (*error_report)(void *device_info, int error_code, const char *msg);
    int (*resource_request)(void *device_info, size_t bytes, int resource_type);
    int (*power_manage)(void *device_info, int power_state);
    // Extensible for future hardware classes
};

// Driver Registration
int limitless_register_driver(const struct limitless_driver_manifest *manifest, const struct limitless_driver_ops *ops);
int limitless_unregister_driver(const char *name);

// Driver State Query
limitless_driver_state_t limitless_query_driver_state(const char *name);

// Event Propagation
int limitless_send_device_event(const char *driver_name, limitless_device_event_t event, void *event_data);

// Resource Management
int limitless_allocate_driver_resource(const char *driver_name, size_t bytes, int resource_type);
int limitless_release_driver_resource(const char *driver_name, int resource_id);

// Error Reporting
int limitless_report_driver_error(const char *driver_name, int error_code, const char *msg);

// Power Management
int limitless_set_driver_power_state(const char *driver_name, int power_state);

// Event Bus Integration Structures
#define MAX_DRIVER_NAME_LEN 128
#define MAX_EVENT_DETAILS_LEN 256

// Structured event data for user-space event bus
struct driver_event_entry {
    char driver_name[MAX_DRIVER_NAME_LEN];
    limitless_device_event_t event;
    char details[MAX_EVENT_DETAILS_LEN];
};

// User-space event bus API
int subscribe_driver_events(const char *driver_name);
int publish_driver_event(const struct driver_event_entry *event);
int query_driver_event_history(const char *driver_name);
void remove_subscription(const char *driver_name, int fd);
void cleanup_event_bus();

#endif // LIMITLESS_DRIVER_API_H
