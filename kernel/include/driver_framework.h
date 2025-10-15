/**
 * Advanced Driver Framework for LimitlessOS
 * Comprehensive driver architecture with hot-plugging, power management, and sandboxing
 */

#ifndef DRIVER_FRAMEWORK_H
#define DRIVER_FRAMEWORK_H

#include "kernel.h"
#include "process.h"
#include "vmm.h"

/* Driver framework constants */
#define MAX_DRIVERS                 256
#define MAX_DEVICES                 1024
#define MAX_DRIVER_NAME_LENGTH      64
#define MAX_DEVICE_NAME_LENGTH      64
#define MAX_DRIVER_DEPENDENCIES     16
#define MAX_DEVICE_RESOURCES        32
#define MAX_DRIVER_CALLBACKS        16
#define DRIVER_STACK_SIZE           (64 * 1024)  /* 64KB stack per driver */

/* Driver types */
typedef enum {
    DRIVER_TYPE_UNKNOWN = 0,
    DRIVER_TYPE_BLOCK,
    DRIVER_TYPE_CHARACTER,
    DRIVER_TYPE_NETWORK,
    DRIVER_TYPE_DISPLAY,
    DRIVER_TYPE_AUDIO,
    DRIVER_TYPE_INPUT,
    DRIVER_TYPE_USB,
    DRIVER_TYPE_PCI,
    DRIVER_TYPE_FILESYSTEM,
    DRIVER_TYPE_PROTOCOL,
    DRIVER_TYPE_VIRTUAL,
    DRIVER_TYPE_SYSTEM,
    DRIVER_TYPE_MAX
} driver_type_t;

/* Device types */
typedef enum {
    DEVICE_TYPE_UNKNOWN = 0,
    DEVICE_TYPE_STORAGE,
    DEVICE_TYPE_NETWORK_ADAPTER,
    DEVICE_TYPE_DISPLAY_ADAPTER,
    DEVICE_TYPE_AUDIO_DEVICE,
    DEVICE_TYPE_INPUT_DEVICE,
    DEVICE_TYPE_USB_CONTROLLER,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_PCI_DEVICE,
    DEVICE_TYPE_ACPI_DEVICE,
    DEVICE_TYPE_PLATFORM_DEVICE,
    DEVICE_TYPE_VIRTUAL_DEVICE,
    DEVICE_TYPE_MAX
} device_type_t;

/* Driver states */
typedef enum {
    DRIVER_STATE_UNLOADED = 0,
    DRIVER_STATE_LOADING,
    DRIVER_STATE_LOADED,
    DRIVER_STATE_INITIALIZING,
    DRIVER_STATE_ACTIVE,
    DRIVER_STATE_SUSPENDING,
    DRIVER_STATE_SUSPENDED,
    DRIVER_STATE_RESUMING,
    DRIVER_STATE_STOPPING,
    DRIVER_STATE_ERROR,
    DRIVER_STATE_UNLOADING
} driver_state_t;

/* Device states */
typedef enum {
    DEVICE_STATE_UNKNOWN = 0,
    DEVICE_STATE_DETECTED,
    DEVICE_STATE_CONFIGURING,
    DEVICE_STATE_ACTIVE,
    DEVICE_STATE_SUSPENDING,
    DEVICE_STATE_SUSPENDED,
    DEVICE_STATE_RESUMING,
    DEVICE_STATE_STOPPING,
    DEVICE_STATE_REMOVED,
    DEVICE_STATE_ERROR
} device_state_t;

/* Power states */
typedef enum {
    POWER_STATE_D0 = 0,  /* Fully on */
    POWER_STATE_D1,      /* Low power */
    POWER_STATE_D2,      /* Lower power */
    POWER_STATE_D3_HOT,  /* Off, but quick recovery */
    POWER_STATE_D3_COLD, /* Off, slow recovery */
    POWER_STATE_UNKNOWN
} power_state_t;

/* Bus types */
typedef enum {
    BUS_TYPE_UNKNOWN = 0,
    BUS_TYPE_PCI,
    BUS_TYPE_USB,
    BUS_TYPE_ACPI,
    BUS_TYPE_PLATFORM,
    BUS_TYPE_I2C,
    BUS_TYPE_SPI,
    BUS_TYPE_VIRTUAL
} bus_type_t;

/* Resource types */
typedef enum {
    RESOURCE_TYPE_NONE = 0,
    RESOURCE_TYPE_MEMORY,
    RESOURCE_TYPE_IO_PORT,
    RESOURCE_TYPE_IRQ,
    RESOURCE_TYPE_DMA,
    RESOURCE_TYPE_BUS_NUMBER,
    RESOURCE_TYPE_CUSTOM
} resource_type_t;

/* Driver capabilities */
typedef enum {
    DRIVER_CAP_NONE          = 0x00000000,
    DRIVER_CAP_HOT_PLUG      = 0x00000001,
    DRIVER_CAP_POWER_MGMT    = 0x00000002,
    DRIVER_CAP_DMA           = 0x00000004,
    DRIVER_CAP_INTERRUPT     = 0x00000008,
    DRIVER_CAP_MEMORY_MAP    = 0x00000010,
    DRIVER_CAP_MULTITHREAD   = 0x00000020,
    DRIVER_CAP_SANDBOXED     = 0x00000040,
    DRIVER_CAP_SECURE        = 0x00000080,
    DRIVER_CAP_VIRTUALIZED   = 0x00000100,
    DRIVER_CAP_REAL_TIME     = 0x00000200,
    DRIVER_CAP_64BIT         = 0x00000400
} driver_capabilities_t;

/* Forward declarations */
typedef struct driver_s driver_t;
typedef struct device_s device_t;
typedef struct driver_registry_s driver_registry_t;
typedef struct device_registry_s device_registry_t;

/* Device resource */
typedef struct {
    resource_type_t type;
    uint64_t start;
    uint64_t size;
    uint32_t flags;
    char name[32];
} device_resource_t;

/* Device identification */
typedef struct {
    uint32_t vendor_id;
    uint32_t device_id;
    uint32_t subsystem_vendor_id;
    uint32_t subsystem_device_id;
    uint32_t class_code;
    uint32_t revision;
    bus_type_t bus_type;
    char manufacturer[64];
    char product[64];
    char serial_number[64];
} device_id_t;

/* Driver callbacks */
typedef struct {
    status_t (*probe)(device_t* device);
    status_t (*remove)(device_t* device);
    status_t (*suspend)(device_t* device, power_state_t state);
    status_t (*resume)(device_t* device);
    status_t (*shutdown)(device_t* device);
    status_t (*reset)(device_t* device);
    status_t (*ioctl)(device_t* device, uint32_t cmd, void* arg);
    status_t (*read)(device_t* device, void* buffer, size_t size, uint64_t offset);
    status_t (*write)(device_t* device, const void* buffer, size_t size, uint64_t offset);
    status_t (*mmap)(device_t* device, vm_area_t* vma);
    status_t (*interrupt_handler)(device_t* device, uint32_t irq);
    status_t (*dma_complete)(device_t* device, void* context);
    status_t (*hotplug_event)(device_t* device, uint32_t event);
    status_t (*power_event)(device_t* device, power_state_t new_state);
    status_t (*error_handler)(device_t* device, uint32_t error_code);
    status_t (*statistics)(device_t* device, void* stats_buffer);
} driver_callbacks_t;

/* Driver security context */
typedef struct {
    uint32_t sandbox_id;
    uint32_t permissions;
    uint32_t memory_limit;
    uint32_t cpu_quota;
    bool isolated;
    bool trusted;
    uint8_t security_level;
    char security_label[32];
} driver_security_t;

/* Driver performance metrics */
typedef struct {
    uint64_t load_time;
    uint64_t init_time;
    uint64_t total_runtime;
    uint64_t interrupt_count;
    uint64_t dma_transfers;
    uint64_t io_operations;
    uint64_t memory_usage;
    uint64_t cpu_usage;
    uint64_t error_count;
    uint64_t last_activity;
    uint32_t active_devices;
    uint32_t peak_devices;
} driver_performance_t;

/* Driver dependency */
typedef struct {
    char driver_name[MAX_DRIVER_NAME_LENGTH];
    uint32_t min_version;
    uint32_t max_version;
    bool optional;
} driver_dependency_t;

/* Driver structure */
struct driver_s {
    uint32_t driver_id;
    char name[MAX_DRIVER_NAME_LENGTH];
    char description[128];
    char vendor[64];
    uint32_t version;
    uint32_t build;
    
    driver_type_t type;
    driver_state_t state;
    driver_capabilities_t capabilities;
    
    /* Driver binary info */
    void* module_base;
    size_t module_size;
    void* entry_point;
    
    /* Callbacks */
    driver_callbacks_t callbacks;
    
    /* Dependencies */
    driver_dependency_t dependencies[MAX_DRIVER_DEPENDENCIES];
    uint32_t dependency_count;
    
    /* Security context */
    driver_security_t security;
    
    /* Performance metrics */
    driver_performance_t performance;
    
    /* Driver process context */
    process_t* driver_process;
    thread_t* driver_thread;
    
    /* Device list */
    device_t* devices[MAX_DEVICES];
    uint32_t device_count;
    
    /* Power management */
    power_state_t power_state;
    bool supports_runtime_pm;
    
    /* Reference counting */
    atomic_t ref_count;
    
    /* Synchronization */
    spinlock_t lock;
    
    /* Linked list */
    struct driver_s* next;
    struct driver_s* prev;
};

/* Device structure */
struct device_s {
    uint32_t device_id;
    char name[MAX_DEVICE_NAME_LENGTH];
    char path[256];  /* Device path in sysfs-like hierarchy */
    
    device_type_t type;
    device_state_t state;
    device_id_t id_info;
    
    /* Parent-child relationships */
    device_t* parent;
    device_t* children[MAX_DEVICES];
    uint32_t child_count;
    
    /* Bus information */
    bus_type_t bus_type;
    uint32_t bus_number;
    uint32_t slot_number;
    
    /* Resources */
    device_resource_t resources[MAX_DEVICE_RESOURCES];
    uint32_t resource_count;
    
    /* Driver binding */
    driver_t* driver;
    void* driver_data;
    
    /* Power management */
    power_state_t power_state;
    power_state_t target_power_state;
    bool can_wakeup;
    
    /* Hot-plug support */
    bool hot_pluggable;
    bool surprise_removal_ok;
    
    /* Device capabilities */
    uint32_t capabilities;
    
    /* Statistics */
    uint64_t uptime;
    uint64_t total_operations;
    uint64_t error_count;
    uint64_t last_error_time;
    
    /* Synchronization */
    spinlock_t lock;
    
    /* Reference counting */
    atomic_t ref_count;
    
    /* Linked list */
    struct device_s* next;
    struct device_s* prev;
};

/* Driver registry */
struct driver_registry_s {
    driver_t* drivers[MAX_DRIVERS];
    uint32_t driver_count;
    uint32_t next_driver_id;
    
    /* Index by name for fast lookup */
    struct {
        char name[MAX_DRIVER_NAME_LENGTH];
        driver_t* driver;
    } name_index[MAX_DRIVERS];
    
    spinlock_t lock;
};

/* Device registry */
struct device_registry_s {
    device_t* devices[MAX_DEVICES];
    uint32_t device_count;
    uint32_t next_device_id;
    
    /* Index by type */
    device_t* type_index[DEVICE_TYPE_MAX][MAX_DEVICES];
    uint32_t type_count[DEVICE_TYPE_MAX];
    
    /* Bus enumeration state */
    bool enumeration_complete[BUS_TYPE_VIRTUAL + 1];
    
    spinlock_t lock;
};

/* Hot-plug event */
typedef struct {
    uint32_t event_type;
    device_t* device;
    uint64_t timestamp;
    char description[128];
} hotplug_event_t;

/* Driver framework statistics */
typedef struct {
    uint32_t total_drivers;
    uint32_t active_drivers;
    uint32_t failed_drivers;
    uint32_t total_devices;
    uint32_t active_devices;
    uint32_t hotplug_events;
    uint64_t total_memory_usage;
    uint64_t framework_uptime;
} driver_framework_stats_t;

/* Driver sandbox statistics */
typedef struct {
    uint32_t total_sandboxes;
    uint64_t memory_pool_size;
    uint64_t memory_pool_used;
    uint64_t peak_memory_usage;
    uint64_t total_cpu_time;
    uint64_t total_syscalls;
    uint64_t total_io_operations;
} driver_sandbox_stats_t;

/* Kernel driver framework status */
typedef struct {
    bool initialized;
    bool framework_active;
    bool enumeration_active;
    bool sandbox_active;
    bool hotplug_active;
    uint32_t total_drivers;
    uint32_t active_drivers;
    uint32_t total_devices;
    uint32_t active_devices;
    uint32_t sandboxed_drivers;
    uint32_t hotplug_events;
    uint64_t sandbox_memory_used;
    uint64_t init_time;
} kernel_driver_framework_status_t;

/* Hot-plug event types */
#define HOTPLUG_EVENT_DEVICE_ADDED      1
#define HOTPLUG_EVENT_DEVICE_REMOVED    2
#define HOTPLUG_EVENT_DEVICE_CHANGED    3
#define HOTPLUG_EVENT_DRIVER_LOADED     4
#define HOTPLUG_EVENT_DRIVER_UNLOADED   5

/* Driver framework API */

/* Initialization */
status_t driver_framework_init(void);
status_t driver_framework_shutdown(void);

/* Driver management */
status_t driver_register(const char* name, driver_type_t type, 
                        const driver_callbacks_t* callbacks, driver_t** driver);
status_t driver_unregister(driver_t* driver);
status_t driver_load(const char* name, const void* module_data, size_t module_size);
status_t driver_unload(const char* name);
status_t driver_start(driver_t* driver);
status_t driver_stop(driver_t* driver);
status_t driver_suspend(driver_t* driver, power_state_t state);
status_t driver_resume(driver_t* driver);

/* Device management */
status_t device_register(const char* name, device_type_t type, 
                        bus_type_t bus_type, device_t** device);
status_t device_unregister(device_t* device);
status_t device_bind_driver(device_t* device, driver_t* driver);
status_t device_unbind_driver(device_t* device);
status_t device_add_resource(device_t* device, resource_type_t type,
                           uint64_t start, uint64_t size, uint32_t flags);

/* Device enumeration */
status_t device_enumerate_bus(bus_type_t bus_type);
status_t device_enumerate_all(void);
device_t* device_find_by_name(const char* name);
device_t* device_find_by_id(device_id_t* id);
uint32_t device_get_by_type(device_type_t type, device_t** devices, uint32_t max_devices);

/* Driver lookup */
driver_t* driver_find_by_name(const char* name);
driver_t* driver_find_for_device(device_t* device);
uint32_t driver_get_by_type(driver_type_t type, driver_t** drivers, uint32_t max_drivers);

/* Hot-plug support */
status_t hotplug_enable(void);
status_t hotplug_disable(void);
status_t hotplug_notify_event(uint32_t event_type, device_t* device, const char* description);
status_t hotplug_register_callback(status_t (*callback)(hotplug_event_t* event));

/* Power management */
status_t power_set_device_state(device_t* device, power_state_t state);
power_state_t power_get_device_state(device_t* device);
status_t power_enable_runtime_pm(device_t* device);
status_t power_disable_runtime_pm(device_t* device);

/* Driver sandboxing */
status_t driver_create_sandbox(driver_t* driver, uint32_t memory_limit, uint32_t cpu_quota);
status_t driver_destroy_sandbox(driver_t* driver);
status_t driver_set_permissions(driver_t* driver, uint32_t permissions);

/* Statistics and monitoring */
status_t driver_framework_get_stats(driver_framework_stats_t* stats);
status_t driver_get_performance(driver_t* driver, driver_performance_t* perf);
status_t device_get_statistics(device_t* device, void* stats_buffer, size_t buffer_size);

/* Utility functions */
const char* driver_state_to_string(driver_state_t state);
const char* device_state_to_string(device_state_t state);
const char* power_state_to_string(power_state_t state);

/* Reference counting */
driver_t* driver_get(driver_t* driver);
void driver_put(driver_t* driver);
device_t* device_get(device_t* device);
void device_put(device_t* device);

/* Driver framework integration */
status_t kernel_driver_framework_init(void);
status_t kernel_driver_framework_get_status(kernel_driver_framework_status_t* status);
status_t kernel_driver_framework_diagnostics(void);
status_t kernel_driver_framework_shutdown(void);

/* Device enumeration */
status_t device_enumeration_init(void);
status_t hotplug_start_monitoring(void);

/* Driver sandbox system */
status_t driver_sandbox_init(void);
status_t driver_sandbox_get_stats(driver_sandbox_stats_t* stats);
status_t driver_sandbox_shutdown(void);

#endif /* DRIVER_FRAMEWORK_H */