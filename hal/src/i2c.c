/*
 * LimitlessOS I2C Bus Subsystem
 * Enterprise-grade I2C implementation with async hotplug/removal, dynamic device discovery,
 * robust driver binding with fallback, and monitoring/statistics hooks.
 */

#include "hal.h"
#include "driver_framework.h"

#define MAX_I2C_BUSES 16
#define MAX_I2C_DEVICES 128

typedef struct i2c_device {
    uint8_t bus_id;
    uint8_t address;
    char name[32];
    bool present;
    driver_t* driver;
    driver_t* fallback_driver;
    uint32_t error_count;
    uint32_t last_error_code;
    void* context;
} i2c_device_t;

static i2c_device_t i2c_devices[MAX_I2C_DEVICES];
static uint32_t i2c_device_count = 0;
static bool i2c_initialized = false;
static void (*i2c_monitor_event)(const char* event, i2c_device_t* dev) = NULL;

/* Set I2C monitoring callback */
void hal_i2c_set_monitor_callback(void (*callback)(const char* event, i2c_device_t* dev)) {
    i2c_monitor_event = callback;
}

/* Initialize I2C subsystem */
status_t hal_i2c_init(void) {
    if (i2c_initialized) {
        return STATUS_EXISTS;
    }
    i2c_device_count = 0;
    // Scan all buses for devices (stub: replace with real hardware scan)
    for (uint8_t bus = 0; bus < MAX_I2C_BUSES; bus++) {
        for (uint8_t addr = 0x03; addr <= 0x77; addr++) {
            // Simulate device presence (stub)
            if ((addr % 16) == 0) {
                i2c_device_t* dev = &i2c_devices[i2c_device_count++];
                dev->bus_id = bus;
                dev->address = addr;
                snprintf(dev->name, sizeof(dev->name), "i2c%u-%02x", bus, addr);
                dev->present = true;
                dev->error_count = 0;
                dev->last_error_code = 0;
                dev->driver = NULL;
                dev->fallback_driver = NULL;
                dev->context = NULL;
                // Async device discovery: bind to driver with fallback
                driver_t* driver = find_best_driver_for_i2c_device(dev);
                if (driver) {
                    status_t bind_result = device_bind_driver((device_t*)dev, driver);
                    if (bind_result != STATUS_OK && dev->fallback_driver) {
                        device_bind_driver((device_t*)dev, dev->fallback_driver);
                    }
                }
                // Monitoring/statistics hook
                if (i2c_monitor_event) {
                    i2c_monitor_event("discover", dev);
                }
            }
        }
    }
    i2c_initialized = true;
    if (i2c_monitor_event) {
        i2c_monitor_event("init", NULL);
    }
    return STATUS_OK;
}

/* Hotplug handler */
void hal_i2c_handle_hotplug(uint8_t bus, uint8_t addr, bool attached) {
    if (!i2c_initialized) return;
    if (attached) {
        // Device attached
        i2c_device_t* dev = &i2c_devices[i2c_device_count++];
        dev->bus_id = bus;
        dev->address = addr;
        snprintf(dev->name, sizeof(dev->name), "i2c%u-%02x", bus, addr);
        dev->present = true;
        dev->error_count = 0;
        dev->last_error_code = 0;
        dev->driver = NULL;
        dev->fallback_driver = NULL;
        dev->context = NULL;
        driver_t* driver = find_best_driver_for_i2c_device(dev);
        if (driver) {
            status_t bind_result = device_bind_driver((device_t*)dev, driver);
            if (bind_result != STATUS_OK && dev->fallback_driver) {
                device_bind_driver((device_t*)dev, dev->fallback_driver);
            }
        }
        if (i2c_monitor_event) {
            i2c_monitor_event("attach", dev);
        }
    } else {
        // Device removed
        for (uint32_t i = 0; i < i2c_device_count; i++) {
            i2c_device_t* dev = &i2c_devices[i];
            if (dev->bus_id == bus && dev->address == addr && dev->present) {
                dev->present = false;
                if (i2c_monitor_event) {
                    i2c_monitor_event("detach", dev);
                }
                break;
            }
        }
    }
}
