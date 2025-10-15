/*
 * LimitlessOS Device API Implementation (User-space)
 * Persistent device registry, event notification, and state recovery
 */

#include "limitless_device_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static limitless_device_info_t g_device_registry[DEVICE_API_MAX_DEVICES];
static int g_device_count = 0;
static limitless_device_event_t g_event_queue[DEVICE_API_MAX_EVENTS];
static int g_event_count = 0;
static pthread_mutex_t registry_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t event_mutex = PTHREAD_MUTEX_INITIALIZER;

static void (*event_callbacks[32])(const limitless_device_event_t*);
static int event_callback_count = 0;

int limitless_device_registry_init(void) {
    pthread_mutex_lock(&registry_mutex);
    g_device_count = 0;
    memset(g_device_registry, 0, sizeof(g_device_registry));
    pthread_mutex_unlock(&registry_mutex);
    return 0;
}

int limitless_device_registry_reload(void) {
    // TODO: Load registry from persistent storage (e.g., /var/lib/limitless/devices.db)
    return 0;
}

int limitless_device_registry_save(void) {
    // TODO: Save registry to persistent storage
    return 0;
}

int limitless_device_registry_get_count(void) {
    pthread_mutex_lock(&registry_mutex);
    int count = g_device_count;
    pthread_mutex_unlock(&registry_mutex);
    return count;
}

int limitless_device_registry_get_info(uint32_t device_id, limitless_device_info_t* info) {
    pthread_mutex_lock(&registry_mutex);
    for (int i = 0; i < g_device_count; i++) {
        if (g_device_registry[i].device_id == device_id) {
            if (info) *info = g_device_registry[i];
            pthread_mutex_unlock(&registry_mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&registry_mutex);
    return -1;
}

int limitless_device_registry_list(limitless_device_info_t* devices, int max_devices) {
    pthread_mutex_lock(&registry_mutex);
    int count = (g_device_count < max_devices) ? g_device_count : max_devices;
    if (devices) memcpy(devices, g_device_registry, sizeof(limitless_device_info_t) * count);
    pthread_mutex_unlock(&registry_mutex);
    return count;
}

int limitless_device_event_subscribe(void (*callback)(const limitless_device_event_t*)) {
    pthread_mutex_lock(&event_mutex);
    if (event_callback_count < 32) {
        event_callbacks[event_callback_count++] = callback;
        pthread_mutex_unlock(&event_mutex);
        return 0;
    }
    pthread_mutex_unlock(&event_mutex);
    return -1;
}

int limitless_device_event_unsubscribe(void (*callback)(const limitless_device_event_t*)) {
    pthread_mutex_lock(&event_mutex);
    for (int i = 0; i < event_callback_count; i++) {
        if (event_callbacks[i] == callback) {
            for (int j = i; j < event_callback_count - 1; j++) {
                event_callbacks[j] = event_callbacks[j + 1];
            }
            event_callback_count--;
            pthread_mutex_unlock(&event_mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&event_mutex);
    return -1;
}

int limitless_device_event_get_pending(limitless_device_event_t* events, int max_events) {
    pthread_mutex_lock(&event_mutex);
    int count = (g_event_count < max_events) ? g_event_count : max_events;
    if (events) memcpy(events, g_event_queue, sizeof(limitless_device_event_t) * count);
    g_event_count = 0;
    pthread_mutex_unlock(&event_mutex);
    return count;
}

int limitless_device_state_save(uint32_t device_id) {
    // TODO: Save device state to persistent storage
    return 0;
}

int limitless_device_state_restore(uint32_t device_id) {
    // TODO: Restore device state from persistent storage
    return 0;
}

// Internal: Notify all event subscribers
static void notify_event_subscribers(const limitless_device_event_t* event) {
    pthread_mutex_lock(&event_mutex);
    for (int i = 0; i < event_callback_count; i++) {
        if (event_callbacks[i]) event_callbacks[i](event);
    }
    pthread_mutex_unlock(&event_mutex);
}

// Internal: Add event to queue and notify
void limitless_device_event_push(const limitless_device_event_t* event) {
    pthread_mutex_lock(&event_mutex);
    if (g_event_count < DEVICE_API_MAX_EVENTS) {
        g_event_queue[g_event_count++] = *event;
        notify_event_subscribers(event);
    }
    pthread_mutex_unlock(&event_mutex);
}
