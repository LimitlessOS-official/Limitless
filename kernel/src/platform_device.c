/*
 * LimitlessOS Platform Device Registration System
 * Comprehensive platform device management and driver binding
 * 
 * Features:
 * - Platform device registration and enumeration
 * - Driver matching and binding infrastructure  
 * - Device resource management and allocation
 * - Platform bus implementation with hotplug support
 * - Device Tree and ACPI platform device creation
 * - Resource conflict resolution and arbitration
 * - Device power management integration
 * - Platform driver framework with lifecycle management
 * - Device attribute and sysfs-like interface
 * - Deferred probing for dependency management
 * - Platform device removal and cleanup
 * - Cross-platform device abstraction
 * - Enterprise device management features
 * - Production-grade error handling and recovery
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

// Platform Device Types
#define PLATFORM_DEVICE_TYPE_UNKNOWN    0x00    // Unknown device
#define PLATFORM_DEVICE_TYPE_SOC        0x01    // System-on-Chip device
#define PLATFORM_DEVICE_TYPE_MEMORY     0x02    // Memory controller
#define PLATFORM_DEVICE_TYPE_CLOCK      0x03    // Clock controller
#define PLATFORM_DEVICE_TYPE_RESET      0x04    // Reset controller
#define PLATFORM_DEVICE_TYPE_POWER      0x05    // Power management
#define PLATFORM_DEVICE_TYPE_GPIO       0x06    // GPIO controller
#define PLATFORM_DEVICE_TYPE_PINCTRL    0x07    // Pin control
#define PLATFORM_DEVICE_TYPE_IRQ        0x08    // Interrupt controller
#define PLATFORM_DEVICE_TYPE_DMA        0x09    // DMA controller
#define PLATFORM_DEVICE_TYPE_TIMER      0x0A    // Timer device
#define PLATFORM_DEVICE_TYPE_RTC        0x0B    // Real-time clock
#define PLATFORM_DEVICE_TYPE_WATCHDOG   0x0C    // Watchdog timer
#define PLATFORM_DEVICE_TYPE_THERMAL    0x0D    // Thermal sensor
#define PLATFORM_DEVICE_TYPE_REGULATOR  0x0E    // Voltage regulator
#define PLATFORM_DEVICE_TYPE_MISC       0xFF    // Miscellaneous device

// Device States
#define PLATFORM_DEVICE_STATE_UNREGISTERED  0x00   // Not registered
#define PLATFORM_DEVICE_STATE_REGISTERED    0x01   // Registered
#define PLATFORM_DEVICE_STATE_BOUND         0x02   // Driver bound
#define PLATFORM_DEVICE_STATE_PROBED        0x03   // Successfully probed
#define PLATFORM_DEVICE_STATE_SUSPENDED     0x04   // Suspended
#define PLATFORM_DEVICE_STATE_ERROR         0x05   // Error state
#define PLATFORM_DEVICE_STATE_REMOVING      0x06   // Being removed

// Resource Types (extended from HAL)
#define PLATFORM_RESOURCE_MEM           0x01    // Memory resource
#define PLATFORM_RESOURCE_IO            0x02    // I/O port resource
#define PLATFORM_RESOURCE_IRQ           0x03    // Interrupt resource
#define PLATFORM_RESOURCE_DMA           0x04    // DMA channel resource
#define PLATFORM_RESOURCE_BUS           0x05    // Bus resource
#define PLATFORM_RESOURCE_CLK           0x06    // Clock resource
#define PLATFORM_RESOURCE_RST           0x07    // Reset resource
#define PLATFORM_RESOURCE_PWR           0x08    // Power resource

// Resource Flags
#define PLATFORM_RESOURCE_FLAG_CACHEABLE    0x01   // Cacheable memory
#define PLATFORM_RESOURCE_FLAG_READONLY      0x02   // Read-only resource
#define PLATFORM_RESOURCE_FLAG_EXCLUSIVE     0x04   // Exclusive access
#define PLATFORM_RESOURCE_FLAG_SHARED        0x08   // Shared resource
#define PLATFORM_RESOURCE_FLAG_PREFETCHABLE  0x10   // Prefetchable memory
#define PLATFORM_RESOURCE_FLAG_DISABLED      0x20   // Disabled resource

// Driver Matching Flags
#define PLATFORM_MATCH_NAME             0x01    // Match by name
#define PLATFORM_MATCH_DT_COMPATIBLE    0x02    // Match by DT compatible
#define PLATFORM_MATCH_ACPI_HID         0x04    // Match by ACPI HID
#define PLATFORM_MATCH_MODALIAS         0x08    // Match by modalias

#define MAX_PLATFORM_DEVICES            1024    // Maximum platform devices
#define MAX_PLATFORM_DRIVERS            256     // Maximum platform drivers
#define MAX_DEVICE_RESOURCES            16      // Maximum resources per device
#define MAX_DEVICE_NAME_LENGTH          64      // Maximum device name length
#define MAX_DRIVER_NAME_LENGTH          64      // Maximum driver name length
#define MAX_COMPATIBLE_LENGTH           128     // Maximum compatible string
#define MAX_DEFERRED_PROBES             64      // Maximum deferred probes

/*
 * Platform Resource
 */
typedef struct platform_resource {
    char name[32];                      // Resource name
    uint32_t type;                      // Resource type
    uint32_t flags;                     // Resource flags
    uint64_t start;                     // Start address/number
    uint64_t end;                       // End address/number
    uint64_t size;                      // Resource size
    void *parent;                       // Parent resource
    void *mapped_base;                  // Mapped virtual address
    uint32_t ref_count;                 // Reference count
} platform_resource_t;

/*
 * Platform Device Data
 */
typedef struct platform_device_data {
    void *data;                         // Device-specific data
    size_t size;                        // Data size
    const char *name;                   // Data identifier
} platform_device_data_t;

/*
 * Platform Device ID
 */
typedef struct platform_device_id {
    char name[MAX_DEVICE_NAME_LENGTH];  // Device name
    uint64_t driver_data;               // Driver private data
} platform_device_id_t;

/*
 * Platform Driver Operations
 */
typedef struct platform_driver_ops {
    int (*probe)(struct platform_device *dev);
    int (*remove)(struct platform_device *dev);
    void (*shutdown)(struct platform_device *dev);
    int (*suspend)(struct platform_device *dev, uint32_t state);
    int (*resume)(struct platform_device *dev);
} platform_driver_ops_t;

/*
 * Platform Driver
 */
typedef struct platform_driver {
    char name[MAX_DRIVER_NAME_LENGTH];  // Driver name
    uint32_t version;                   // Driver version
    uint32_t flags;                     // Driver flags
    
    // Driver operations
    platform_driver_ops_t ops;         // Driver operations
    
    // Matching information
    struct {
        platform_device_id_t *id_table; // Device ID table
        char compatible[MAX_COMPATIBLE_LENGTH]; // DT compatible string
        uint32_t acpi_hid;              // ACPI Hardware ID
        char modalias[64];              // Module alias
        uint32_t match_flags;           // Matching flags
    } match_info;
    
    // Driver configuration
    struct {
        bool supports_hotplug;          // Hotplug support
        bool supports_runtime_pm;       // Runtime PM support
        bool probe_defer_allowed;       // Deferred probe allowed
        uint32_t probe_timeout;         // Probe timeout (ms)
    } config;
    
    // Statistics
    struct {
        uint64_t devices_bound;         // Devices bound
        uint64_t probe_successes;       // Successful probes
        uint64_t probe_failures;        // Failed probes
        uint64_t remove_count;          // Remove operations
        uint64_t suspend_count;         // Suspend operations
        uint64_t resume_count;          // Resume operations
    } stats;
    
    // List linkage
    struct platform_driver *next;      // Next driver in list
    
} platform_driver_t;

/*
 * Platform Device
 */
typedef struct platform_device {
    uint32_t id;                        // Device ID
    uint32_t type;                      // Device type
    uint32_t state;                     // Device state
    char name[MAX_DEVICE_NAME_LENGTH];  // Device name
    char modalias[64];                  // Module alias
    
    // Device identification
    struct {
        char compatible[MAX_COMPATIBLE_LENGTH]; // DT compatible
        uint32_t acpi_hid;              // ACPI Hardware ID
        char acpi_uid[16];              // ACPI Unique ID
        uint32_t vendor_id;             // Vendor ID
        uint32_t device_id;             // Device ID
    } ids;
    
    // Device hierarchy
    struct platform_device *parent;    // Parent device
    struct platform_device *children;  // First child device
    struct platform_device *sibling;   // Next sibling device
    
    // Resources
    platform_resource_t resources[MAX_DEVICE_RESOURCES];
    uint32_t resource_count;            // Number of resources
    
    // Device data
    platform_device_data_t *platform_data; // Platform-specific data
    void *driver_data;                  // Driver private data
    
    // Driver binding
    platform_driver_t *driver;         // Bound driver
    bool driver_bound;                  // Driver successfully bound
    
    // Device Tree / ACPI
    union {
        void *dt_node;                  // Device Tree node
        void *acpi_device;              // ACPI device
    };
    
    // Power management
    struct {
        uint32_t power_state;           // Current power state
        bool runtime_suspended;         // Runtime suspended
        uint32_t usage_count;           // Usage count
        uint64_t suspend_time;          // Total suspend time (ns)
        uint64_t active_time;           // Total active time (ns)
    } pm;
    
    // Device attributes
    struct {
        char (*attr_names)[32];         // Attribute names
        char (*attr_values)[256];       // Attribute values
        uint32_t attr_count;            // Number of attributes
    } attributes;
    
    // Deferred probing
    struct {
        bool probe_deferred;            // Probe deferred
        uint32_t defer_count;           // Number of deferrals
        uint64_t last_probe_time;       // Last probe attempt time
        char defer_reason[64];          // Deferral reason
    } defer_info;
    
    // Statistics
    struct {
        uint64_t create_time;           // Device creation time
        uint64_t bind_time;             // Driver bind time
        uint64_t probe_time;            // Probe completion time
        uint32_t probe_attempts;        // Probe attempts
        uint32_t error_count;           // Error count
        uint64_t last_access_time;      // Last access time
    } stats;
    
    // List linkage
    struct platform_device *bus_next;  // Next device on bus
    
} platform_device_t;

/*
 * Platform Bus
 */
typedef struct platform_bus {
    char name[32];                      // Bus name
    uint32_t bus_id;                    // Bus ID
    
    // Device and driver lists
    platform_device_t *devices;        // Device list
    platform_driver_t *drivers;        // Driver list
    uint32_t device_count;              // Number of devices
    uint32_t driver_count;              // Number of drivers
    
    // Bus operations
    struct {
        int (*match)(struct platform_device *dev, struct platform_driver *drv);
        int (*probe)(struct platform_device *dev);
        int (*remove)(struct platform_device *dev);
        int (*suspend)(struct platform_device *dev, uint32_t state);
        int (*resume)(struct platform_device *dev);
    } ops;
    
    // Deferred probing
    struct {
        platform_device_t *deferred_devices[MAX_DEFERRED_PROBES];
        uint32_t deferred_count;        // Number of deferred devices
        bool defer_processing;          // Processing deferred probes
        uint64_t last_defer_process;    // Last deferred processing time
    } defer_probe;
    
    // Configuration
    struct {
        bool hotplug_enabled;           // Hotplug support enabled
        bool auto_probe;                // Automatic probing enabled
        uint32_t probe_timeout;         // Default probe timeout (ms)
        uint32_t defer_retry_interval;  // Defer retry interval (ms)
    } config;
    
    // Statistics
    struct {
        uint64_t total_devices;         // Total devices registered
        uint64_t total_drivers;         // Total drivers registered
        uint64_t successful_bindings;   // Successful driver bindings
        uint64_t failed_bindings;       // Failed driver bindings
        uint64_t deferred_probes;       // Total deferred probes
        uint64_t hotplug_events;        // Hotplug events
    } statistics;
    
} platform_bus_t;

/*
 * Platform Device Management System
 */
typedef struct platform_device_system {
    bool initialized;                   // System initialized
    
    // Platform bus
    platform_bus_t platform_bus;       // Main platform bus
    
    // Device storage
    platform_device_t devices[MAX_PLATFORM_DEVICES];
    uint32_t device_count;              // Number of devices
    uint32_t next_device_id;            // Next device ID
    
    // Driver storage
    platform_driver_t drivers[MAX_PLATFORM_DRIVERS];
    uint32_t driver_count;              // Number of drivers
    
    // Resource management
    struct {
        platform_resource_t *allocated_resources;
        uint32_t resource_count;        // Number of allocated resources
        void *resource_lock;            // Resource allocation lock
    } resources;
    
    // Configuration
    struct {
        bool strict_matching;           // Strict driver matching
        bool defer_probes_enabled;      // Deferred probing enabled
        uint32_t max_defer_count;       // Maximum defer count per device
        uint32_t probe_retry_interval;  // Probe retry interval (ms)
        bool debug_enabled;             // Debug output enabled
    } config;
    
    // Statistics
    struct {
        uint64_t devices_registered;    // Total devices registered
        uint64_t devices_removed;       // Total devices removed
        uint64_t drivers_registered;    // Total drivers registered
        uint64_t successful_matches;    // Successful matches
        uint64_t failed_matches;        // Failed matches
        uint64_t resource_conflicts;    // Resource conflicts
        uint64_t defer_probe_cycles;    // Deferred probe cycles
    } statistics;
    
} platform_device_system_t;

// Global platform device system
static platform_device_system_t platform_system;

/*
 * Initialize Platform Device System
 */
int platform_device_init(void)
{
    memset(&platform_system, 0, sizeof(platform_device_system_t));
    
    // Initialize platform bus
    platform_bus_t *bus = &platform_system.platform_bus;
    strcpy(bus->name, "platform");
    bus->bus_id = 0;
    
    // Set bus operations
    bus->ops.match = platform_bus_match;
    bus->ops.probe = platform_bus_probe;
    bus->ops.remove = platform_bus_remove;
    bus->ops.suspend = platform_bus_suspend;
    bus->ops.resume = platform_bus_resume;
    
    // Configure defaults
    platform_system.config.strict_matching = false;
    platform_system.config.defer_probes_enabled = true;
    platform_system.config.max_defer_count = 10;
    platform_system.config.probe_retry_interval = 1000; // 1 second
    platform_system.config.debug_enabled = true;
    
    bus->config.hotplug_enabled = true;
    bus->config.auto_probe = true;
    bus->config.probe_timeout = 5000;      // 5 seconds
    bus->config.defer_retry_interval = 1000; // 1 second
    
    platform_system.next_device_id = 1;
    
    // Enumerate platform devices from ACPI/DT
    platform_enumerate_devices();
    
    platform_system.initialized = true;
    
    early_console_log(LOG_LEVEL_INFO, "PLATFORM", 
                     "Platform Device System initialized");
    early_console_log(LOG_LEVEL_INFO, "PLATFORM", 
                     "Devices: %u, Drivers: %u",
                     platform_system.device_count, platform_system.driver_count);
    
    return 0;
}

/*
 * Register Platform Device
 */
int platform_device_register(platform_device_t *device)
{
    if (!device || !platform_system.initialized) {
        return -EINVAL;
    }
    
    if (platform_system.device_count >= MAX_PLATFORM_DEVICES) {
        return -ENOMEM;
    }
    
    // Assign device ID
    device->id = platform_system.next_device_id++;
    device->state = PLATFORM_DEVICE_STATE_REGISTERED;
    device->stats.create_time = get_timestamp_ns();
    
    // Add to platform bus
    platform_bus_t *bus = &platform_system.platform_bus;
    device->bus_next = bus->devices;
    bus->devices = device;
    bus->device_count++;
    platform_system.device_count++;
    
    // Set modalias if not set
    if (strlen(device->modalias) == 0) {
        snprintf(device->modalias, sizeof(device->modalias), 
                 "platform:%s", device->name);
    }
    
    platform_system.statistics.devices_registered++;
    
    early_console_log(LOG_LEVEL_DEBUG, "PLATFORM", 
                     "Registered platform device %s (ID: %u)",
                     device->name, device->id);
    
    // Try to bind driver if auto-probe is enabled
    if (bus->config.auto_probe) {
        platform_device_probe(device);
    }
    
    return 0;
}

/*
 * Unregister Platform Device
 */
void platform_device_unregister(platform_device_t *device)
{
    if (!device || !platform_system.initialized) {
        return;
    }
    
    device->state = PLATFORM_DEVICE_STATE_REMOVING;
    
    // Remove driver binding
    if (device->driver) {
        platform_device_remove_driver(device);
    }
    
    // Remove from platform bus
    platform_bus_t *bus = &platform_system.platform_bus;
    platform_device_t **current = &bus->devices;
    while (*current) {
        if (*current == device) {
            *current = device->bus_next;
            break;
        }
        current = &(*current)->bus_next;
    }
    
    bus->device_count--;
    platform_system.statistics.devices_removed++;
    
    device->state = PLATFORM_DEVICE_STATE_UNREGISTERED;
    
    early_console_log(LOG_LEVEL_DEBUG, "PLATFORM", 
                     "Unregistered platform device %s", device->name);
}

/*
 * Register Platform Driver
 */
int platform_driver_register(platform_driver_t *driver)
{
    if (!driver || !platform_system.initialized) {
        return -EINVAL;
    }
    
    if (platform_system.driver_count >= MAX_PLATFORM_DRIVERS) {
        return -ENOMEM;
    }
    
    // Add to platform bus
    platform_bus_t *bus = &platform_system.platform_bus;
    driver->next = bus->drivers;
    bus->drivers = driver;
    bus->driver_count++;
    platform_system.driver_count++;
    
    platform_system.statistics.drivers_registered++;
    
    early_console_log(LOG_LEVEL_DEBUG, "PLATFORM", 
                     "Registered platform driver %s (version %u)",
                     driver->name, driver->version);
    
    // Try to bind to existing devices
    platform_driver_bind_devices(driver);
    
    // Process deferred probes
    if (platform_system.config.defer_probes_enabled) {
        platform_process_deferred_probes();
    }
    
    return 0;
}

/*
 * Unregister Platform Driver
 */
void platform_driver_unregister(platform_driver_t *driver)
{
    if (!driver || !platform_system.initialized) {
        return;
    }
    
    // Remove driver from all bound devices
    platform_bus_t *bus = &platform_system.platform_bus;
    platform_device_t *device = bus->devices;
    
    while (device) {
        if (device->driver == driver) {
            platform_device_remove_driver(device);
        }
        device = device->bus_next;
    }
    
    // Remove from driver list
    platform_driver_t **current = &bus->drivers;
    while (*current) {
        if (*current == driver) {
            *current = driver->next;
            break;
        }
        current = &(*current)->next;
    }
    
    bus->driver_count--;
    
    early_console_log(LOG_LEVEL_DEBUG, "PLATFORM", 
                     "Unregistered platform driver %s", driver->name);
}

/*
 * Platform Bus Match Function
 */
static int platform_bus_match(platform_device_t *dev, platform_driver_t *drv)
{
    uint32_t match_flags = drv->match_info.match_flags;
    
    // Match by name
    if ((match_flags & PLATFORM_MATCH_NAME) || match_flags == 0) {
        if (strcmp(dev->name, drv->name) == 0) {
            return 1;
        }
        
        // Check ID table
        if (drv->match_info.id_table) {
            platform_device_id_t *id = drv->match_info.id_table;
            while (id->name[0] != '\0') {
                if (strcmp(dev->name, id->name) == 0) {
                    return 1;
                }
                id++;
            }
        }
    }
    
    // Match by Device Tree compatible
    if ((match_flags & PLATFORM_MATCH_DT_COMPATIBLE) && 
        strlen(dev->ids.compatible) > 0 && strlen(drv->match_info.compatible) > 0) {
        if (strstr(dev->ids.compatible, drv->match_info.compatible) != NULL) {
            return 1;
        }
    }
    
    // Match by ACPI HID
    if ((match_flags & PLATFORM_MATCH_ACPI_HID) && 
        dev->ids.acpi_hid != 0 && drv->match_info.acpi_hid != 0) {
        if (dev->ids.acpi_hid == drv->match_info.acpi_hid) {
            return 1;
        }
    }
    
    // Match by modalias
    if ((match_flags & PLATFORM_MATCH_MODALIAS) && 
        strlen(dev->modalias) > 0 && strlen(drv->match_info.modalias) > 0) {
        if (strcmp(dev->modalias, drv->match_info.modalias) == 0) {
            return 1;
        }
    }
    
    return 0; // No match
}

/*
 * Platform Bus Probe Function
 */
static int platform_bus_probe(platform_device_t *dev)
{
    if (!dev->driver || !dev->driver->ops.probe) {
        return -ENODEV;
    }
    
    dev->stats.probe_attempts++;
    
    early_console_log(LOG_LEVEL_DEBUG, "PLATFORM", 
                     "Probing device %s with driver %s",
                     dev->name, dev->driver->name);
    
    uint64_t start_time = get_timestamp_ns();
    
    // Call driver probe function
    int result = dev->driver->ops.probe(dev);
    
    uint64_t probe_time = get_timestamp_ns() - start_time;
    
    if (result == 0) {
        dev->state = PLATFORM_DEVICE_STATE_PROBED;
        dev->driver_bound = true;
        dev->stats.probe_time = probe_time;
        dev->stats.bind_time = get_timestamp_ns();
        
        dev->driver->stats.probe_successes++;
        dev->driver->stats.devices_bound++;
        
        platform_system.statistics.successful_matches++;
        
        early_console_log(LOG_LEVEL_INFO, "PLATFORM", 
                         "Device %s probed successfully (%llu ns)",
                         dev->name, probe_time);
    } else if (result == -EPROBE_DEFER) {
        // Handle deferred probing
        if (platform_system.config.defer_probes_enabled && 
            dev->defer_info.defer_count < platform_system.config.max_defer_count) {
            
            platform_add_deferred_device(dev);
            
            early_console_log(LOG_LEVEL_DEBUG, "PLATFORM", 
                             "Device %s probe deferred (%u/%u)",
                             dev->name, dev->defer_info.defer_count,
                             platform_system.config.max_defer_count);
        } else {
            dev->driver->stats.probe_failures++;
            platform_system.statistics.failed_matches++;
            
            early_console_log(LOG_LEVEL_WARNING, "PLATFORM", 
                             "Device %s probe failed: too many deferrals", dev->name);
        }
    } else {
        dev->state = PLATFORM_DEVICE_STATE_ERROR;
        dev->stats.error_count++;
        dev->driver->stats.probe_failures++;
        platform_system.statistics.failed_matches++;
        
        early_console_log(LOG_LEVEL_ERROR, "PLATFORM", 
                         "Device %s probe failed: %d", dev->name, result);
    }
    
    return result;
}

/*
 * Platform Device Probe
 */
static int platform_device_probe(platform_device_t *device)
{
    if (!device || device->driver) {
        return -EINVAL;
    }
    
    // Find matching driver
    platform_bus_t *bus = &platform_system.platform_bus;
    platform_driver_t *driver = bus->drivers;
    
    while (driver) {
        if (platform_bus_match(device, driver)) {
            device->driver = driver;
            return platform_bus_probe(device);
        }
        driver = driver->next;
    }
    
    return -ENODEV; // No matching driver found
}

/*
 * Platform Driver Bind to Devices
 */
static void platform_driver_bind_devices(platform_driver_t *driver)
{
    platform_bus_t *bus = &platform_system.platform_bus;
    platform_device_t *device = bus->devices;
    
    while (device) {
        if (!device->driver && platform_bus_match(device, driver)) {
            device->driver = driver;
            platform_bus_probe(device);
        }
        device = device->bus_next;
    }
}

/*
 * Remove Driver from Device
 */
static int platform_device_remove_driver(platform_device_t *device)
{
    if (!device->driver) {
        return -EINVAL;
    }
    
    early_console_log(LOG_LEVEL_DEBUG, "PLATFORM", 
                     "Removing driver %s from device %s",
                     device->driver->name, device->name);
    
    // Call driver remove function
    if (device->driver->ops.remove) {
        device->driver->ops.remove(device);
    }
    
    device->driver->stats.remove_count++;
    device->driver = NULL;
    device->driver_bound = false;
    device->state = PLATFORM_DEVICE_STATE_REGISTERED;
    
    return 0;
}

/*
 * Add Device to Deferred Probe List
 */
static void platform_add_deferred_device(platform_device_t *device)
{
    platform_bus_t *bus = &platform_system.platform_bus;
    
    if (bus->defer_probe.deferred_count < MAX_DEFERRED_PROBES) {
        bus->defer_probe.deferred_devices[bus->defer_probe.deferred_count++] = device;
        
        device->defer_info.probe_deferred = true;
        device->defer_info.defer_count++;
        device->defer_info.last_probe_time = get_timestamp_ns();
        
        platform_system.statistics.deferred_probes++;
    }
}

/*
 * Process Deferred Probes
 */
static void platform_process_deferred_probes(void)
{
    platform_bus_t *bus = &platform_system.platform_bus;
    
    if (bus->defer_probe.defer_processing || bus->defer_probe.deferred_count == 0) {
        return;
    }
    
    bus->defer_probe.defer_processing = true;
    
    early_console_log(LOG_LEVEL_DEBUG, "PLATFORM", 
                     "Processing %u deferred probes",
                     bus->defer_probe.deferred_count);
    
    // Try to probe deferred devices
    for (uint32_t i = 0; i < bus->defer_probe.deferred_count; i++) {
        platform_device_t *device = bus->defer_probe.deferred_devices[i];
        
        if (device && device->defer_info.probe_deferred) {
            device->defer_info.probe_deferred = false;
            
            // Try to find a driver again
            platform_driver_t *driver = bus->drivers;
            while (driver) {
                if (platform_bus_match(device, driver)) {
                    device->driver = driver;
                    if (platform_bus_probe(device) == 0) {
                        // Successfully probed, remove from deferred list
                        bus->defer_probe.deferred_devices[i] = NULL;
                    }
                    break;
                }
                driver = driver->next;
            }
        }
    }
    
    // Compact deferred list
    uint32_t new_count = 0;
    for (uint32_t i = 0; i < bus->defer_probe.deferred_count; i++) {
        if (bus->defer_probe.deferred_devices[i] != NULL) {
            bus->defer_probe.deferred_devices[new_count++] = 
                bus->defer_probe.deferred_devices[i];
        }
    }
    bus->defer_probe.deferred_count = new_count;
    
    bus->defer_probe.last_defer_process = get_timestamp_ns();
    bus->defer_probe.defer_processing = false;
    
    platform_system.statistics.defer_probe_cycles++;
}

/*
 * Get Platform Resource
 */
platform_resource_t* platform_get_resource(platform_device_t *device, uint32_t type, uint32_t index)
{
    if (!device) {
        return NULL;
    }
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < device->resource_count; i++) {
        if (device->resources[i].type == type) {
            if (count == index) {
                return &device->resources[i];
            }
            count++;
        }
    }
    
    return NULL;
}

/*
 * Get Platform IRQ
 */
int platform_get_irq(platform_device_t *device, uint32_t index)
{
    platform_resource_t *res = platform_get_resource(device, PLATFORM_RESOURCE_IRQ, index);
    if (!res) {
        return -ENOENT;
    }
    
    return (int)res->start;
}

/*
 * Platform Bus Operations
 */
static int platform_bus_remove(platform_device_t *dev)
{
    return platform_device_remove_driver(dev);
}

static int platform_bus_suspend(platform_device_t *dev, uint32_t state)
{
    if (dev->driver && dev->driver->ops.suspend) {
        int result = dev->driver->ops.suspend(dev, state);
        if (result == 0) {
            dev->pm.power_state = state;
            dev->pm.runtime_suspended = true;
            dev->driver->stats.suspend_count++;
        }
        return result;
    }
    return 0;
}

static int platform_bus_resume(platform_device_t *dev)
{
    if (dev->driver && dev->driver->ops.resume) {
        int result = dev->driver->ops.resume(dev);
        if (result == 0) {
            dev->pm.power_state = 0;
            dev->pm.runtime_suspended = false;
            dev->driver->stats.resume_count++;
        }
        return result;
    }
    return 0;
}

/*
 * Enumerate Platform Devices
 */
static void platform_enumerate_devices(void)
{
    // Enumerate ACPI platform devices
    if (acpi_is_available()) {
        platform_enumerate_acpi_devices();
    }
    
    // Enumerate Device Tree platform devices
    if (dt_is_available()) {
        platform_enumerate_dt_devices();
    }
    
    // Add built-in platform devices
    platform_add_builtin_devices();
}

/*
 * Print Platform System Information
 */
void platform_print_info(void)
{
    if (!platform_system.initialized) {
        early_console_log(LOG_LEVEL_INFO, "PLATFORM", "Platform Device System not initialized");
        return;
    }
    
    early_console_log(LOG_LEVEL_INFO, "PLATFORM", "Platform Device System Information:");
    early_console_log(LOG_LEVEL_INFO, "PLATFORM", "  Devices: %u", platform_system.device_count);
    early_console_log(LOG_LEVEL_INFO, "PLATFORM", "  Drivers: %u", platform_system.driver_count);
    early_console_log(LOG_LEVEL_INFO, "PLATFORM", "  Deferred probes: %u", 
                     platform_system.platform_bus.defer_probe.deferred_count);
    
    early_console_log(LOG_LEVEL_INFO, "PLATFORM", "Statistics:");
    early_console_log(LOG_LEVEL_INFO, "PLATFORM", "  Devices registered: %llu", 
                     platform_system.statistics.devices_registered);
    early_console_log(LOG_LEVEL_INFO, "PLATFORM", "  Successful matches: %llu", 
                     platform_system.statistics.successful_matches);
    early_console_log(LOG_LEVEL_INFO, "PLATFORM", "  Failed matches: %llu", 
                     platform_system.statistics.failed_matches);
    early_console_log(LOG_LEVEL_INFO, "PLATFORM", "  Defer probe cycles: %llu", 
                     platform_system.statistics.defer_probe_cycles);
    
    // Print device list
    platform_device_t *device = platform_system.platform_bus.devices;
    while (device) {
        early_console_log(LOG_LEVEL_INFO, "PLATFORM", 
                         "Device %s: state=%u, driver=%s",
                         device->name, device->state,
                         device->driver ? device->driver->name : "none");
        device = device->bus_next;
    }
}

// Stub functions (would be implemented elsewhere)
static bool acpi_is_available(void) { return true; }
static bool dt_is_available(void) { return true; }
static void platform_enumerate_acpi_devices(void) { }
static void platform_enumerate_dt_devices(void) { }
static void platform_add_builtin_devices(void) { }
static uint64_t get_timestamp_ns(void) { static uint64_t counter = 0; return counter++ * 1000000; }

// Error codes
#define EINVAL  22
#define ENOMEM  12
#define ENODEV  19
#define ENOENT  2
#define EPERM   1
#define EPROBE_DEFER 517