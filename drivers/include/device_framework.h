/**
 * Advanced Device Driver Framework for LimitlessOS
 * 
 * Implements comprehensive device driver support including PCI Express, USB 3.0,
 * SATA/NVMe storage, modern GPUs, wireless networking, audio subsystems, and
 * advanced power management with proper device enumeration and hot-plug support.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#ifndef __DEVICE_FRAMEWORK_H__
#define __DEVICE_FRAMEWORK_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "smp.h"

/* Device types */
typedef enum {
    DEVICE_TYPE_UNKNOWN,
    DEVICE_TYPE_PCI,
    DEVICE_TYPE_USB,
    DEVICE_TYPE_SATA,
    DEVICE_TYPE_NVME,
    DEVICE_TYPE_GPU,
    DEVICE_TYPE_NETWORK,
    DEVICE_TYPE_AUDIO,
    DEVICE_TYPE_INPUT,
    DEVICE_TYPE_DISPLAY,
    DEVICE_TYPE_STORAGE,
    DEVICE_TYPE_POWER,
    DEVICE_TYPE_THERMAL,
    DEVICE_TYPE_SENSOR,
    DEVICE_TYPE_SECURITY,
    DEVICE_TYPE_WIRELESS,
    DEVICE_TYPE_BLUETOOTH,
    DEVICE_TYPE_CAMERA,
    DEVICE_TYPE_MAX
} device_type_t;

/* Device states */
typedef enum {
    DEVICE_STATE_UNKNOWN,
    DEVICE_STATE_DETECTED,
    DEVICE_STATE_INITIALIZING,
    DEVICE_STATE_ACTIVE,
    DEVICE_STATE_SUSPENDED,
    DEVICE_STATE_ERROR,
    DEVICE_STATE_REMOVED
} device_state_t;

/* Power states */
typedef enum {
    POWER_STATE_D0,     /* Fully on */
    POWER_STATE_D1,     /* Low power */
    POWER_STATE_D2,     /* Lower power */
    POWER_STATE_D3_HOT, /* Off, but hot-pluggable */
    POWER_STATE_D3_COLD /* Off */
} power_state_t;

/* Forward declarations */
typedef struct device device_t;
typedef struct device_driver device_driver_t;
typedef struct bus_type bus_type_t;
typedef struct device_class device_class_t;

/**
 * Device structure
 */
struct device {
    /* Basic device information */
    char name[64];                      /* Device name */
    device_type_t type;                 /* Device type */
    device_state_t state;               /* Current state */
    uint32_t device_id;                 /* Unique device ID */
    uint16_t vendor_id;                 /* Vendor ID */
    uint16_t product_id;                /* Product ID */
    uint8_t class_code;                 /* Device class */
    uint8_t subclass_code;              /* Device subclass */
    uint8_t interface_code;             /* Interface code */
    uint8_t revision;                   /* Revision */
    
    /* Device hierarchy */
    device_t *parent;                   /* Parent device */
    device_t *children;                 /* First child */
    device_t *sibling;                  /* Next sibling */
    
    /* Driver binding */
    device_driver_t *driver;            /* Bound driver */
    void *driver_data;                  /* Driver private data */
    
    /* Bus information */
    bus_type_t *bus;                    /* Bus type */
    void *bus_data;                     /* Bus-specific data */
    
    /* Device class */
    device_class_t *class;              /* Device class */
    
    /* Resources */
    struct {
        uintptr_t base_addr[6];         /* Base addresses */
        size_t size[6];                 /* Region sizes */
        uint32_t flags[6];              /* Region flags */
        int irq;                        /* Interrupt number */
        int dma_channels[4];            /* DMA channels */
    } resources;
    
    /* Power management */
    struct {
        power_state_t current_state;    /* Current power state */
        power_state_t target_state;     /* Target power state */
        bool can_wakeup;                /* Can generate wakeup events */
        bool runtime_pm;                /* Runtime power management */
        uint32_t usage_count;           /* Usage reference count */
        spinlock_t lock;                /* Power management lock */
    } power;
    
    /* DMA information */
    struct {
        uint64_t dma_mask;              /* DMA address mask */
        uint64_t coherent_dma_mask;     /* Coherent DMA mask */
        bool dma_coherent;              /* DMA coherent */
        struct dma_pools *dma_pools;    /* DMA pools */
    } dma_info;
    
    /* Sysfs attributes */
    struct {
        void *kobj;                     /* Kernel object */
        struct attribute_group **groups; /* Attribute groups */
    } sysfs;
    
    /* Device locks */
    spinlock_t lock;                    /* Device lock */
    struct mutex mutex;                 /* Device mutex */
    
    /* Reference counting */
    atomic_t ref_count;                 /* Reference count */
    
    /* Device flags */
    unsigned long flags;                /* Device flags */
    
    /* Hot-plug support */
    bool removable;                     /* Device is removable */
    bool present;                       /* Device is present */
    
    /* List linkage */
    device_t *next_global;              /* Global device list */
    device_t *next_bus;                 /* Bus device list */
    device_t *next_class;               /* Class device list */
};

/**
 * Device driver structure
 */
struct device_driver {
    char name[64];                      /* Driver name */
    const char *version;                /* Driver version */
    const char *author;                 /* Driver author */
    const char *description;            /* Driver description */
    const char *license;                /* Driver license */
    
    /* Device matching */
    const struct device_id *id_table;  /* Device ID table */
    int (*match)(device_t *dev, device_driver_t *drv);
    
    /* Driver operations */
    int (*probe)(device_t *dev);        /* Device probe */
    int (*remove)(device_t *dev);       /* Device remove */
    void (*shutdown)(device_t *dev);    /* Device shutdown */
    
    /* Power management */
    int (*suspend)(device_t *dev, power_state_t state);
    int (*resume)(device_t *dev);
    int (*runtime_suspend)(device_t *dev);
    int (*runtime_resume)(device_t *dev);
    
    /* Error handling */
    int (*error_detected)(device_t *dev, int error);
    int (*mmio_enabled)(device_t *dev);
    int (*slot_reset)(device_t *dev);
    void (*resume_normal)(device_t *dev);
    
    /* Bus type */
    bus_type_t *bus;                    /* Bus type */
    
    /* Driver flags */
    unsigned long flags;                /* Driver flags */
    
    /* Module information */
    struct module *owner;               /* Owning module */
    
    /* List linkage */
    device_driver_t *next;              /* Driver list */
};

/**
 * Bus type structure
 */
struct bus_type {
    char name[32];                      /* Bus name */
    
    /* Bus operations */
    int (*match)(device_t *dev, device_driver_t *drv);
    int (*probe)(device_t *dev);
    int (*remove)(device_t *dev);
    void (*shutdown)(device_t *dev);
    
    /* Power management */
    int (*suspend)(device_t *dev, power_state_t state);
    int (*resume)(device_t *dev);
    
    /* Device enumeration */
    int (*enumerate)(bus_type_t *bus);
    
    /* Hot-plug support */
    int (*add_device)(device_t *dev);
    void (*remove_device)(device_t *dev);
    
    /* Bus-specific data */
    void *private_data;
    
    /* Device lists */
    device_t *devices;                  /* Devices on this bus */
    device_driver_t *drivers;           /* Drivers for this bus */
    
    /* Bus lock */
    spinlock_t lock;
    
    /* List linkage */
    bus_type_t *next;
};

/**
 * Device class structure
 */
struct device_class {
    char name[32];                      /* Class name */
    
    /* Class operations */
    int (*add_device)(device_t *dev);
    void (*remove_device)(device_t *dev);
    
    /* Attribute operations */
    ssize_t (*show_attribute)(device_t *dev, char *buf);
    ssize_t (*store_attribute)(device_t *dev, const char *buf, size_t count);
    
    /* Device list */
    device_t *devices;                  /* Devices in this class */
    
    /* Class lock */
    spinlock_t lock;
    
    /* List linkage */
    device_class_t *next;
};

/**
 * Device ID structure
 */
struct device_id {
    uint16_t vendor_id;                 /* Vendor ID */
    uint16_t product_id;                /* Product ID */
    uint16_t subvendor_id;              /* Subsystem vendor ID */
    uint16_t subdevice_id;              /* Subsystem device ID */
    uint32_t class_mask;                /* Device class mask */
    uintptr_t driver_data;              /* Driver data */
};

/**
 * DMA pool structure
 */
typedef struct dma_pool {
    char name[32];                      /* Pool name */
    size_t size;                        /* Block size */
    size_t align;                       /* Alignment */
    size_t boundary;                    /* Boundary */
    device_t *dev;                      /* Associated device */
    
    void *vaddr;                        /* Virtual address */
    dma_addr_t dma_addr;                /* DMA address */
    
    struct {
        void *free_list;                /* Free block list */
        size_t free_count;              /* Free block count */
        size_t total_count;             /* Total block count */
    } blocks;
    
    spinlock_t lock;                    /* Pool lock */
    struct dma_pool *next;              /* Next pool */
} dma_pool_t;

/**
 * PCI Express support structures
 */
typedef struct pcie_device {
    device_t device;                    /* Base device */
    
    uint8_t bus;                        /* PCI bus number */
    uint8_t slot;                       /* PCI slot number */
    uint8_t function;                   /* PCI function number */
    
    /* PCI configuration space */
    uint32_t config_space[64];          /* 256 bytes */
    
    /* PCIe capabilities */
    struct {
        uint16_t express_cap;           /* PCIe capability offset */
        uint16_t device_cap;            /* Device capabilities */
        uint16_t device_control;        /* Device control */
        uint16_t device_status;         /* Device status */
        uint32_t link_cap;              /* Link capabilities */
        uint16_t link_control;          /* Link control */
        uint16_t link_status;           /* Link status */
        uint8_t max_payload_size;       /* Max payload size */
        uint8_t max_read_request;       /* Max read request size */
    } pcie;
    
    /* MSI/MSI-X support */
    struct {
        bool msi_enabled;               /* MSI enabled */
        bool msix_enabled;              /* MSI-X enabled */
        uint8_t msi_vectors;            /* Number of MSI vectors */
        uint16_t msix_vectors;          /* Number of MSI-X vectors */
        void *msix_table;               /* MSI-X table */
    } msi;
    
    /* Power management */
    struct {
        uint16_t pm_cap;                /* PM capability offset */
        uint16_t pm_control;            /* PM control/status */
        bool d1_support;                /* D1 support */
        bool d2_support;                /* D2 support */
        bool pme_support;               /* PME support */
    } pm;
    
    /* Advanced Error Reporting */
    struct {
        uint16_t aer_cap;               /* AER capability offset */
        uint32_t uncorrectable_status;  /* Uncorrectable error status */
        uint32_t uncorrectable_mask;    /* Uncorrectable error mask */
        uint32_t correctable_status;    /* Correctable error status */
        uint32_t correctable_mask;      /* Correctable error mask */
    } aer;
} pcie_device_t;

/**
 * USB 3.0 support structures
 */
typedef struct usb_device {
    device_t device;                    /* Base device */
    
    uint8_t address;                    /* USB device address */
    uint8_t speed;                      /* USB speed */
    uint8_t port;                       /* Port number */
    uint8_t tier;                       /* Tier in USB tree */
    
    /* USB descriptors */
    struct {
        uint16_t bcdUSB;                /* USB specification release */
        uint8_t bDeviceClass;           /* Device class */
        uint8_t bDeviceSubClass;        /* Device subclass */
        uint8_t bDeviceProtocol;        /* Device protocol */
        uint8_t bMaxPacketSize0;        /* Max packet size for endpoint 0 */
        uint16_t idVendor;              /* Vendor ID */
        uint16_t idProduct;             /* Product ID */
        uint16_t bcdDevice;             /* Device release number */
        uint8_t iManufacturer;          /* Manufacturer string index */
        uint8_t iProduct;               /* Product string index */
        uint8_t iSerialNumber;          /* Serial number string index */
        uint8_t bNumConfigurations;     /* Number of configurations */
    } desc;
    
    /* USB 3.0 specific */
    struct {
        bool superspeed;                /* SuperSpeed capable */
        bool superspeed_plus;           /* SuperSpeed+ capable */
        uint8_t u1_timeout;             /* U1 timeout */
        uint8_t u2_timeout;             /* U2 timeout */
        bool lpm_capable;               /* Link Power Management */
    } usb3;
    
    /* Endpoints */
    struct {
        uint16_t wMaxPacketSize;        /* Max packet size */
        uint8_t bInterval;              /* Polling interval */
        uint8_t bmAttributes;           /* Attributes */
        bool in_use;                    /* Endpoint in use */
    } endpoints[32];
    
    /* Power management */
    struct {
        bool remote_wakeup;             /* Remote wakeup capable */
        bool self_powered;              /* Self-powered device */
        uint16_t max_power;             /* Max power consumption (mA) */
    } power;
} usb_device_t;

/**
 * NVMe support structures
 */
typedef struct nvme_device {
    device_t device;                    /* Base device */
    
    /* NVMe controller */
    struct {
        void *bar0;                     /* Controller registers */
        uint32_t capabilities;          /* Controller capabilities */
        uint32_t version;               /* NVMe version */
        uint16_t max_queues;            /* Maximum queues */
        uint16_t queue_depth;           /* Queue depth */
        uint32_t page_size;             /* Page size */
        bool volatile_cache;            /* Volatile write cache */
    } ctrl;
    
    /* Admin queue */
    struct {
        void *sq_base;                  /* Submission queue base */
        void *cq_base;                  /* Completion queue base */
        uint16_t sq_tail;               /* SQ tail pointer */
        uint16_t cq_head;               /* CQ head pointer */
        uint16_t depth;                 /* Queue depth */
        spinlock_t lock;                /* Queue lock */
    } admin_queue;
    
    /* I/O queues */
    struct {
        void *sq_base;                  /* Submission queue base */
        void *cq_base;                  /* Completion queue base */
        uint16_t sq_tail;               /* SQ tail pointer */
        uint16_t cq_head;               /* CQ head pointer */
        uint16_t depth;                 /* Queue depth */
        uint16_t vector;                /* Interrupt vector */
        spinlock_t lock;                /* Queue lock */
    } io_queues[64];
    
    uint16_t num_io_queues;             /* Number of I/O queues */
    
    /* Namespaces */
    struct {
        uint32_t namespace_id;          /* Namespace ID */
        uint64_t size;                  /* Size in blocks */
        uint32_t block_size;            /* Block size */
        uint32_t metadata_size;         /* Metadata size */
        bool active;                    /* Namespace active */
    } namespaces[256];
    
    uint32_t num_namespaces;            /* Number of namespaces */
} nvme_device_t;

/**
 * Modern GPU support structures
 */
typedef struct gpu_device {
    device_t device;                    /* Base device */
    
    /* GPU type */
    enum {
        GPU_TYPE_INTEGRATED,
        GPU_TYPE_DISCRETE,
        GPU_TYPE_VIRTUAL
    } gpu_type;
    
    /* Memory information */
    struct {
        uint64_t vram_size;             /* Video memory size */
        uint64_t vram_used;             /* Video memory used */
        void *vram_base;                /* Video memory base */
        uint64_t system_memory;         /* System memory available */
        bool unified_memory;            /* Unified memory architecture */
    } memory;
    
    /* Display capabilities */
    struct {
        uint32_t max_displays;          /* Maximum displays */
        uint32_t max_resolution_x;      /* Maximum X resolution */
        uint32_t max_resolution_y;      /* Maximum Y resolution */
        uint32_t max_refresh_rate;      /* Maximum refresh rate */
        bool hdmi_support;              /* HDMI support */
        bool displayport_support;       /* DisplayPort support */
        bool hdcp_support;              /* HDCP support */
    } display;
    
    /* Compute capabilities */
    struct {
        uint32_t compute_units;         /* Number of compute units */
        uint32_t shader_units;          /* Number of shader units */
        uint32_t max_threads;           /* Maximum threads */
        bool opencl_support;            /* OpenCL support */
        bool cuda_support;              /* CUDA support */
        bool vulkan_support;            /* Vulkan support */
    } compute;
    
    /* Power management */
    struct {
        uint32_t base_power;            /* Base power consumption */
        uint32_t max_power;             /* Maximum power consumption */
        bool dynamic_clocking;          /* Dynamic clock adjustment */
        bool power_gating;              /* Power gating support */
    } power;
} gpu_device_t;

/* Global device framework state */
extern struct {
    device_t *device_list;              /* Global device list */
    bus_type_t *bus_list;               /* Bus type list */
    device_class_t *class_list;         /* Device class list */
    device_driver_t *driver_list;       /* Driver list */
    
    uint32_t next_device_id;            /* Next device ID */
    
    spinlock_t device_lock;             /* Device list lock */
    spinlock_t bus_lock;                /* Bus list lock */
    spinlock_t class_lock;              /* Class list lock */
    spinlock_t driver_lock;             /* Driver list lock */
    
    struct {
        atomic_long_t devices_total;    /* Total devices */
        atomic_long_t devices_active;   /* Active devices */
        atomic_long_t drivers_loaded;   /* Loaded drivers */
        atomic_long_t probe_success;    /* Successful probes */
        atomic_long_t probe_failed;     /* Failed probes */
    } stats;
} device_framework;

/* Function declarations */

/* Framework initialization */
int device_framework_init(void);
int pci_bus_init(void);
int usb_bus_init(void);
int nvme_driver_init(void);
int gpu_driver_init(void);

/* Device management */
device_t *device_create(device_type_t type, const char *name);
void device_destroy(device_t *dev);
int device_register(device_t *dev);
void device_unregister(device_t *dev);
device_t *device_find_by_name(const char *name);
device_t *device_find_by_id(uint32_t device_id);

/* Driver management */
int driver_register(device_driver_t *drv);
void driver_unregister(device_driver_t *drv);
int driver_probe_device(device_t *dev);
void driver_remove_device(device_t *dev);

/* Bus management */
int bus_register(bus_type_t *bus);
void bus_unregister(bus_type_t *bus);
int bus_add_device(bus_type_t *bus, device_t *dev);
void bus_remove_device(device_t *dev);
int bus_enumerate_devices(bus_type_t *bus);

/* Device class management */
int class_register(device_class_t *class);
void class_unregister(device_class_t *class);
int class_add_device(device_class_t *class, device_t *dev);
void class_remove_device(device_t *dev);

/* Power management */
int device_set_power_state(device_t *dev, power_state_t state);
power_state_t device_get_power_state(device_t *dev);
int device_enable_runtime_pm(device_t *dev);
void device_disable_runtime_pm(device_t *dev);
int device_runtime_suspend(device_t *dev);
int device_runtime_resume(device_t *dev);

/* DMA management */
dma_pool_t *dma_pool_create(const char *name, device_t *dev, 
                           size_t size, size_t align, size_t boundary);
void dma_pool_destroy(dma_pool_t *pool);
void *dma_pool_alloc(dma_pool_t *pool, gfp_t gfp_flags, dma_addr_t *dma_handle);
void dma_pool_free(dma_pool_t *pool, void *vaddr, dma_addr_t dma_handle);

/* PCI Express functions */
pcie_device_t *pcie_device_create(uint8_t bus, uint8_t slot, uint8_t function);
int pcie_enable_device(pcie_device_t *pdev);
void pcie_disable_device(pcie_device_t *pdev);
int pcie_request_regions(pcie_device_t *pdev, const char *name);
void pcie_release_regions(pcie_device_t *pdev);
int pcie_enable_msi(pcie_device_t *pdev);
void pcie_disable_msi(pcie_device_t *pdev);
int pcie_enable_msix(pcie_device_t *pdev, int nvec);
void pcie_disable_msix(pcie_device_t *pdev);

/* USB 3.0 functions */
usb_device_t *usb_device_create(uint8_t address, uint8_t speed);
int usb_control_transfer(usb_device_t *udev, uint8_t request_type, 
                        uint8_t request, uint16_t value, uint16_t index,
                        void *data, uint16_t length);
int usb_bulk_transfer(usb_device_t *udev, uint8_t endpoint,
                     void *data, size_t length, size_t *actual_length);
int usb_interrupt_transfer(usb_device_t *udev, uint8_t endpoint,
                          void *data, size_t length, size_t *actual_length);

/* NVMe functions */
nvme_device_t *nvme_device_create(pcie_device_t *pdev);
int nvme_init_controller(nvme_device_t *nvme);
int nvme_create_queues(nvme_device_t *nvme, uint16_t num_queues);
int nvme_submit_command(nvme_device_t *nvme, uint16_t queue_id, void *cmd);
int nvme_read_blocks(nvme_device_t *nvme, uint32_t namespace_id,
                    uint64_t lba, uint16_t block_count, void *buffer);
int nvme_write_blocks(nvme_device_t *nvme, uint32_t namespace_id,
                     uint64_t lba, uint16_t block_count, const void *buffer);

/* GPU functions */
gpu_device_t *gpu_device_create(pcie_device_t *pdev);
int gpu_init_device(gpu_device_t *gpu);
int gpu_allocate_memory(gpu_device_t *gpu, size_t size, void **vaddr, uint64_t *gpu_addr);
void gpu_free_memory(gpu_device_t *gpu, void *vaddr);
int gpu_submit_command_buffer(gpu_device_t *gpu, void *commands, size_t size);

/* Hot-plug support */
int device_hotplug_add(device_t *dev);
void device_hotplug_remove(device_t *dev);
int device_hotplug_register_notifier(void (*callback)(device_t *dev, bool added));

/* Device enumeration and discovery */
int enumerate_pci_devices(void);
int enumerate_usb_devices(void);
int discover_nvme_devices(void);
int discover_gpu_devices(void);

/* Debugging and diagnostics */
void device_framework_show_stats(void);
void device_dump_info(device_t *dev);
void device_tree_dump(void);
int device_framework_selftest(void);

/* Inline helper functions */
static inline void device_get(device_t *dev) {
    if (dev) {
        atomic_inc(&dev->ref_count);
    }
}

static inline void device_put(device_t *dev) {
    if (dev && atomic_dec_and_test(&dev->ref_count)) {
        device_destroy(dev);
    }
}

static inline bool device_is_pci(device_t *dev) {
    return dev->type == DEVICE_TYPE_PCI;
}

static inline bool device_is_usb(device_t *dev) {
    return dev->type == DEVICE_TYPE_USB;
}

static inline bool device_is_nvme(device_t *dev) {
    return dev->type == DEVICE_TYPE_NVME;
}

static inline bool device_is_gpu(device_t *dev) {
    return dev->type == DEVICE_TYPE_GPU;
}

#endif /* __DEVICE_FRAMEWORK_H__ */