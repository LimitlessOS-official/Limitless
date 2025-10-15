/*
 * LimitlessOS Device API (User-space)
 * Enterprise-grade device registry, event notification, and state recovery
 * Provides libudev/IOKit/SetupAPI equivalent APIs for user-space
 */

#ifndef LIMITLESS_DEVICE_API_H
#define LIMITLESS_DEVICE_API_H

#include <stdint.h>
#include <stdbool.h>

#define DEVICE_API_MAX_DEVICES 1024
#define DEVICE_API_MAX_EVENTS  4096

/* Device info structure */
typedef struct {
    uint32_t device_id;
    char name[64];
    uint32_t type;
    char path[256];
    uint32_t state;
    uint32_t power_state;
    uint32_t owner_uid;
    uint32_t group_gid;
    uint32_t permissions;
    bool hot_pluggable;
    bool surprise_removal_ok;
    char driver_name[64];
    char bus_type[32];
} limitless_device_info_t;

/* Device event types */
typedef enum {
    DEVICE_EVENT_ADDED = 1,
    DEVICE_EVENT_REMOVED = 2,
    DEVICE_EVENT_CHANGED = 3,
    DEVICE_EVENT_POWER = 4,
    DEVICE_EVENT_ERROR = 5,
    DEVICE_EVENT_CUSTOM = 100
} limitless_device_event_type_t;

/* Device event structure */
typedef struct {
    uint64_t timestamp;
    limitless_device_event_type_t type;
    uint32_t device_id;
    char device_name[64];
    char description[128];
    uint32_t user_id;
    uint32_t process_id;
} limitless_device_event_t;

/* Persistent device registry API */
int limitless_device_registry_init(void);
int limitless_device_registry_reload(void);
int limitless_device_registry_save(void);
int limitless_device_registry_get_count(void);
int limitless_device_registry_get_info(uint32_t device_id, limitless_device_info_t* info);
int limitless_device_registry_list(limitless_device_info_t* devices, int max_devices);

/* Device event notification API */
int limitless_device_event_subscribe(void (*callback)(const limitless_device_event_t*));
int limitless_device_event_unsubscribe(void (*callback)(const limitless_device_event_t*));
int limitless_device_event_get_pending(limitless_device_event_t* events, int max_events);

/* State recovery API */
int limitless_device_state_save(uint32_t device_id);
int limitless_device_state_restore(uint32_t device_id);

#endif /* LIMITLESS_DEVICE_API_H */
