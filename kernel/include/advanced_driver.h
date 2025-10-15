/*
 * LimitlessOS Advanced Driver Framework Header
 * Enterprise-grade driver subsystem with comprehensive management capabilities
 */

#ifndef ADVANCED_DRIVER_H
#define ADVANCED_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

/* Driver types */
typedef enum {
    DRIVER_TYPE_BLOCK = 0,      /* Block storage drivers */
    DRIVER_TYPE_NETWORK = 1,    /* Network interface drivers */
    DRIVER_TYPE_GRAPHICS = 2,   /* Graphics and display drivers */
    DRIVER_TYPE_AUDIO = 3,      /* Audio and sound drivers */
    DRIVER_TYPE_INPUT = 4,      /* Input device drivers */
    DRIVER_TYPE_USB = 5,        /* USB device drivers */
    DRIVER_TYPE_PCI = 6,        /* PCI device drivers */
    DRIVER_TYPE_ACPI = 7,       /* ACPI and power management drivers */
    DRIVER_TYPE_CRYPTO = 8,     /* Cryptographic hardware drivers */
    DRIVER_TYPE_SENSOR = 9,     /* Sensor and monitoring drivers */
    DRIVER_TYPE_CONTAINER = 10, /* Container runtime drivers */
    DRIVER_TYPE_VIRTUAL = 11,   /* Virtual device drivers */
    DRIVER_TYPE_MAX = 12
} driver_type_t;

/* Driver states */
typedef enum {
    DRIVER_STATE_UNLOADED = 0,  /* Driver not loaded */
    DRIVER_STATE_LOADING = 1,   /* Driver being loaded */
    DRIVER_STATE_LOADED = 2,    /* Driver loaded but not initialized */
    DRIVER_STATE_INITIALIZING = 3, /* Driver initializing */
    DRIVER_STATE_ACTIVE = 4,    /* Driver active and operational */
    DRIVER_STATE_SUSPENDING = 5, /* Driver suspending */
    DRIVER_STATE_SUSPENDED = 6, /* Driver suspended */
    DRIVER_STATE_RESUMING = 7,  /* Driver resuming */
    DRIVER_STATE_ERROR = 8,     /* Driver in error state */
    DRIVER_STATE_UNLOADING = 9  /* Driver being unloaded */
} driver_state_t;

/* Driver capabilities */
#define DRIVER_CAP_HOTPLUG          0x00000001
#define DRIVER_CAP_POWER_MGMT       0x00000002
#define DRIVER_CAP_DMA              0x00000004
#define DRIVER_CAP_INTERRUPT        0x00000008
#define DRIVER_CAP_MULTIQUEUE       0x00000010
#define DRIVER_CAP_VIRTUALIZATION   0x00000020
#define DRIVER_CAP_CONTAINER        0x00000040
#define DRIVER_CAP_ENTERPRISE       0x00000080
#define DRIVER_CAP_SECURITY         0x00000100
#define DRIVER_CAP_MONITORING       0x00000200

/* Forward declarations */
typedef struct driver driver_t;
typedef struct device device_t;
typedef struct driver_version driver_version_t;
typedef struct driver_dependency driver_dependency_t;
typedef struct driver_device_id driver_device_id_t;
typedef struct driver_ops driver_ops_t;

/* ============================================================================
 * CORE DRIVER API
 * ============================================================================ */

/**
 * Initialize advanced driver framework
 * @return 0 on success, negative on error
 */
int advanced_driver_init(void);

/**
 * Register a driver with the framework
 * @param driver Driver to register
 * @return 0 on success, negative on error
 */
int driver_register(driver_t* driver);

/**
 * Unregister a driver from the framework
 * @param driver Driver to unregister
 * @return 0 on success, negative on error
 */
int driver_unregister(driver_t* driver);

/**
 * Load and initialize a driver
 * @param driver Driver to load
 * @return 0 on success, negative on error
 */
int driver_load(driver_t* driver);

/**
 * Unload a driver
 * @param driver Driver to unload
 * @return 0 on success, negative on error
 */
int driver_unload(driver_t* driver);

/**
 * Find driver by name
 * @param name Driver name to search for
 * @return Driver pointer on success, NULL if not found
 */
driver_t* driver_find_by_name(const char* name);

/**
 * Find drivers by type
 * @param type Driver type to search for
 * @param drivers Output array for driver pointers
 * @param max_drivers Maximum number of drivers to return
 * @return Number of drivers found
 */
int driver_find_by_type(driver_type_t type, driver_t** drivers, uint32_t max_drivers);

/* ============================================================================
 * DEVICE MANAGEMENT API
 * ============================================================================ */

/**
 * Create a new device
 * @param name Device name
 * @param type Device type
 * @return Device pointer on success, NULL on error
 */
device_t* device_create(const char* name, driver_type_t type);

/**
 * Destroy a device
 * @param device Device to destroy
 * @return 0 on success, negative on error
 */
int device_destroy(device_t* device);

/**
 * Attach driver to device
 * @param device Device to attach driver to
 * @param driver Driver to attach
 * @return 0 on success, negative on error
 */
int device_attach_driver(device_t* device, driver_t* driver);

/**
 * Detach driver from device
 * @param device Device to detach driver from
 * @return 0 on success, negative on error
 */
int device_detach_driver(device_t* device);

/**
 * Find device by name
 * @param name Device name to search for
 * @return Device pointer on success, NULL if not found
 */
device_t* device_find_by_name(const char* name);

/**
 * Find device by ID
 * @param device_id Device ID to search for
 * @return Device pointer on success, NULL if not found
 */
device_t* device_find_by_id(uint32_t device_id);

/* ============================================================================
 * HOT-PLUG SUPPORT API
 * ============================================================================ */

/**
 * Handle hot-plug event
 * @param device Device that was plugged/unplugged
 * @param plugged true if device was plugged in, false if unplugged
 */
void driver_handle_hotplug_event(device_t* device, bool plugged);

/**
 * Enable/disable hot-plug support
 * @param enable true to enable, false to disable
 * @return 0 on success, negative on error
 */
int driver_set_hotplug_enabled(bool enable);

/**
 * Enable/disable automatic driver loading
 * @param enable true to enable, false to disable
 * @return 0 on success, negative on error
 */
int driver_set_auto_loading(bool enable);

/**
 * Register hot-plug callback
 * @param callback Callback function for hot-plug events
 * @return 0 on success, negative on error
 */
int driver_register_hotplug_callback(void (*callback)(device_t*, bool));

/* ============================================================================
 * DRIVER VERSIONING API
 * ============================================================================ */

/**
 * Parse version string into version structure
 * @param version_str Version string (e.g., "1.2.3.4-rc1")
 * @param version Output version structure
 * @return 0 on success, negative on error
 */
int driver_parse_version(const char* version_str, driver_version_t* version);

/**
 * Compare two driver versions
 * @param v1 First version
 * @param v2 Second version
 * @return -1 if v1 < v2, 0 if v1 == v2, 1 if v1 > v2
 */
int driver_version_compare(const driver_version_t* v1, const driver_version_t* v2);

/**
 * Check if version satisfies dependency requirements
 * @param version Version to check
 * @param min_version Minimum required version
 * @param max_version Maximum compatible version
 * @return true if version is compatible, false otherwise
 */
bool driver_version_satisfies(const driver_version_t* version, 
                             const driver_version_t* min_version,
                             const driver_version_t* max_version);

/* ============================================================================
 * DRIVER DEPENDENCY API
 * ============================================================================ */

/**
 * Add dependency to driver
 * @param driver Driver to add dependency to
 * @param dependency_name Name of dependency driver
 * @param min_version Minimum required version
 * @param max_version Maximum compatible version
 * @param optional true if dependency is optional
 * @return 0 on success, negative on error
 */
int driver_add_dependency(driver_t* driver, const char* dependency_name,
                         const driver_version_t* min_version,
                         const driver_version_t* max_version, bool optional);

/**
 * Check driver dependencies
 * @param driver Driver to check dependencies for
 * @return true if all dependencies are satisfied, false otherwise
 */
bool driver_check_dependencies(driver_t* driver);

/**
 * Resolve load order for multiple drivers
 * @param drivers Array of drivers to order
 * @param count Number of drivers in array
 * @return 0 on success, negative on error (circular dependency)
 */
int driver_resolve_load_order(driver_t** drivers, uint32_t count);

/* ============================================================================
 * DRIVER SECURITY API
 * ============================================================================ */

/**
 * Set driver security profile
 * @param driver Driver to configure
 * @param profile Security profile name
 * @return 0 on success, negative on error
 */
int driver_set_security_profile(driver_t* driver, const char* profile);

/**
 * Enable/disable driver sandboxing
 * @param driver Driver to configure
 * @param sandboxed true to enable sandboxing, false to disable
 * @return 0 on success, negative on error
 */
int driver_set_sandboxed(driver_t* driver, bool sandboxed);

/**
 * Verify driver signature
 * @param driver Driver to verify
 * @return 0 if signature is valid, negative on error
 */
int driver_verify_signature(driver_t* driver);

/**
 * Set driver trust level
 * @param driver Driver to configure
 * @param trust_level Trust level (0-100)
 * @return 0 on success, negative on error
 */
int driver_set_trust_level(driver_t* driver, uint32_t trust_level);

/* ============================================================================
 * ENTERPRISE FEATURES API
 * ============================================================================ */

/**
 * Enable/disable driver monitoring
 * @param driver Driver to configure
 * @param enable true to enable monitoring, false to disable
 * @return 0 on success, negative on error
 */
int driver_enable_monitoring(driver_t* driver, bool enable);

/**
 * Enable/disable driver logging
 * @param driver Driver to configure
 * @param enable true to enable logging, false to disable
 * @return 0 on success, negative on error
 */
int driver_enable_logging(driver_t* driver, bool enable);

/**
 * Set driver enterprise profile
 * @param driver Driver to configure
 * @param profile Enterprise profile name
 * @return 0 on success, negative on error
 */
int driver_set_enterprise_profile(driver_t* driver, const char* profile);

/**
 * Get driver statistics
 * @param driver Driver to get statistics for
 * @param stats Output statistics structure
 * @return 0 on success, negative on error
 */
int driver_get_statistics(driver_t* driver, void* stats);

/**
 * Update driver statistics
 * @param driver Driver to update statistics for
 */
void driver_update_stats(driver_t* driver);

/* ============================================================================
 * DRIVER SEARCH AND LOADING API
 * ============================================================================ */

/**
 * Add driver search path
 * @param path Directory path to search for drivers
 * @return 0 on success, negative on error
 */
int driver_add_search_path(const char* path);

/**
 * Load driver from file
 * @param driver_path Path to driver file
 * @return Driver pointer on success, NULL on error
 */
driver_t* driver_load_from_file(const char* driver_path);

/**
 * Scan for available drivers
 * @param type Driver type to scan for (DRIVER_TYPE_MAX for all)
 * @return Number of drivers found
 */
int driver_scan_for_drivers(driver_type_t type);

/**
 * Load all drivers of specified type
 * @param type Driver type to load
 * @return Number of drivers loaded successfully
 */
int driver_load_all_by_type(driver_type_t type);

/* ============================================================================
 * POWER MANAGEMENT API
 * ============================================================================ */

/**
 * Suspend device
 * @param device Device to suspend
 * @param state Power state to enter
 * @return 0 on success, negative on error
 */
int device_suspend(device_t* device, uint32_t state);

/**
 * Resume device
 * @param device Device to resume
 * @return 0 on success, negative on error
 */
int device_resume(device_t* device);

/**
 * Reset device
 * @param device Device to reset
 * @return 0 on success, negative on error
 */
int device_reset(device_t* device);

/**
 * Set device power state
 * @param device Device to configure
 * @param power_state Power state (D0-D3)
 * @return 0 on success, negative on error
 */
int device_set_power_state(device_t* device, uint32_t power_state);

/* ============================================================================
 * STATISTICS AND MONITORING API
 * ============================================================================ */

/**
 * Print comprehensive driver framework statistics
 */
void driver_print_statistics(void);

/**
 * Get driver framework statistics
 * @param total_drivers Output for total driver count
 * @param active_drivers Output for active driver count
 * @param total_devices Output for total device count
 * @param hotplug_events Output for hot-plug event count
 * @return 0 on success, negative on error
 */
int driver_get_framework_stats(uint32_t* total_drivers, uint32_t* active_drivers,
                              uint32_t* total_devices, uint64_t* hotplug_events);

/**
 * Driver framework self-test
 * @return 0 on success, negative on test failure
 */
int driver_framework_self_test(void);

/**
 * Shutdown driver framework
 */
void driver_framework_shutdown(void);

/* ============================================================================
 * DRIVER STRUCTURE DEFINITIONS
 * ============================================================================ */

/* Driver version structure */
struct driver_version {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
    uint16_t build;
    char suffix[16];
};

/* Driver device ID structure */
struct driver_device_id {
    uint32_t vendor_id;
    uint32_t device_id;
    uint32_t subsystem_vendor;
    uint32_t subsystem_device;
    uint32_t class_mask;
    uint64_t driver_data;
};

/* Driver operations structure */
struct driver_ops {
    int (*probe)(void* device, const driver_device_id_t* id);
    void (*remove)(void* device);
    int (*suspend)(void* device, int state);
    int (*resume)(void* device);
    int (*reset)(void* device);
    int (*ioctl)(void* device, unsigned int cmd, void* arg);
    void (*shutdown)(void* device);
};

#endif /* ADVANCED_DRIVER_H */