/*
 * LimitlessOS Complete Driver Ecosystem - Header Definitions
 * Enterprise Hardware Compatibility Layer
 */

#ifndef LIMITLESSOS_DRIVERS_H
#define LIMITLESSOS_DRIVERS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Forward declarations
struct device;
struct pci_device;
struct list_head;
struct mutex;
struct work_struct;
struct workqueue_struct;

// Basic list implementation
struct list_head {
    struct list_head *next, *prev;
};

// Mutex implementation
struct mutex {
    uint32_t locked;
    uint32_t owner;
};

// Work queue structures
struct work_struct {
    struct list_head entry;
    void (*func)(struct work_struct *work);
};

struct workqueue_struct {
    struct list_head work_list;
    struct mutex work_lock;
    bool running;
};

// Device structure
struct device {
    uint32_t device_id;
    uint32_t vendor_id;
    char name[64];
    struct device *parent;
    void *driver_data;
    struct list_head children;
};

// PCI device structure
struct pci_device {
    struct device device;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t revision;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint32_t irq_line;
    
    struct {
        uint64_t address;
        size_t size;
        bool io_space;
    } bars[6];
};

// Interrupt return type
typedef enum {
    IRQ_NONE = 0,
    IRQ_HANDLED = 1,
    IRQ_WAKE_THREAD = 2
} irqreturn_t;

// IRQ flags
#define IRQF_SHARED     0x80

// GFP flags
#define GFP_KERNEL      0x01
#define GFP_ATOMIC      0x02
#define GFP_NOWAIT      0x04

// Kernel log levels
#define KERN_EMERG      "<0>"
#define KERN_ALERT      "<1>"
#define KERN_CRIT       "<2>"
#define KERN_ERR        "<3>"
#define KERN_WARNING    "<4>"
#define KERN_NOTICE     "<5>"
#define KERN_INFO       "<6>"
#define KERN_DEBUG      "<7>"

// Error codes
#define ENOMEM          12
#define EINVAL          22
#define ENODEV          19
#define EFAULT          14

// File offset type
typedef long long loff_t;

// User space pointer annotation
#define __user

// Driver information structure for user space
struct driver_info {
    char name[64];
    char description[128];
    char version[16];
    char author[64];
    uint32_t driver_type;
    uint32_t device_class;
    uint32_t priority;
    uint32_t state;
    uint32_t device_count;
    bool builtin;
    
    // Statistics
    uint64_t probe_calls;
    uint64_t interrupts_handled;
    uint64_t errors;
    uint64_t bytes_transferred;
    uint32_t uptime_seconds;
};

// Driver statistics structure for user space
struct driver_statistics {
    uint64_t drivers_loaded;
    uint64_t devices_detected;
    uint64_t hotplug_events;
    uint64_t driver_errors;
    uint32_t load_time_ms;
    uint32_t active_devices;
    
    uint64_t total_interrupts;
    uint64_t total_io_operations;
    uint64_t total_bytes_transferred;
    
    uint32_t network_drivers;
    uint32_t storage_drivers;
    uint32_t audio_drivers;
    uint32_t wireless_drivers;
};

// Wi-Fi configuration structures
struct wifi_ax_config {
    bool ofdma_enabled;
    bool mu_mimo_enabled;
    uint32_t channel_width;
    uint32_t spatial_streams;
    bool bss_coloring;
    bool twt_support;
};

struct wifi_ac_config {
    bool beamforming;
    uint32_t channel_width;
    uint32_t spatial_streams;
    bool dfs_support;
};

// Function prototypes
static inline struct pci_device *to_pci_device(struct device *dev)
{
    return (struct pci_device *)dev;
}

// Kernel memory management
void *kzalloc(size_t size, uint32_t flags);
void kfree(void *ptr);

// String operations
int strncpy(char *dest, const char *src, size_t n);

// PCI operations
int pci_enable_device(struct pci_device *dev);
void pci_set_master(struct pci_device *dev);

// Interrupt management
int request_irq(unsigned int irq, irqreturn_t (*handler)(int, void *),
                unsigned long flags, const char *name, void *dev);
void free_irq(unsigned int irq, void *dev_id);

// Work queue operations
struct workqueue_struct *create_workqueue(const char *name);
void destroy_workqueue(struct workqueue_struct *wq);

// List operations
#define INIT_LIST_HEAD(ptr) do { \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

// Work initialization
#define INIT_WORK(work, func) do { \
    (work)->func = (func); \
    INIT_LIST_HEAD(&(work)->entry); \
} while (0)

// Mutex operations
static inline void mutex_init(struct mutex *mutex)
{
    mutex->locked = 0;
    mutex->owner = 0;
}

static inline void mutex_lock(struct mutex *mutex)
{
    while (__sync_lock_test_and_set(&mutex->locked, 1)) {
        // Busy wait
    }
}

static inline void mutex_unlock(struct mutex *mutex)
{
    __sync_lock_release(&mutex->locked);
}

// Kernel print function
int printk(const char *format, ...);

// Copy to/from user space
unsigned long copy_to_user(void __user *to, const void *from, unsigned long n);
unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);

// System call declaration macro
#define asmlinkage

// Driver registration function
struct hardware_driver;
int driver_register(struct hardware_driver *driver);

// Hardware enumeration
int driver_enumerate_hardware(void);

// Hot-plug work handler
void driver_hotplug_work_handler(struct work_struct *work);

// Driver registry functions
int driver_register_device_classes(void);

#endif // LIMITLESSOS_DRIVERS_H