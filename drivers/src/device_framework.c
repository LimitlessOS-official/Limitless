/**
 * Advanced Device Driver Framework Implementation for LimitlessOS
 * 
 * Core implementation of the comprehensive device driver framework including
 * PCI Express, USB 3.0, NVMe, GPU support, and advanced power management.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "device_framework.h"
#include "mm/advanced.h"
#include "smp.h"
#include "kernel.h"
#include <string.h>

/* Global device framework state */
struct {
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

/* Built-in bus types */
static bus_type_t pci_bus_type;
static bus_type_t usb_bus_type;

/* Built-in device classes */
static device_class_t storage_class;
static device_class_t network_class;
static device_class_t gpu_class;
static device_class_t audio_class;

/**
 * Initialize device framework
 */
int device_framework_init(void) {
    kprintf("[DEV] Initializing device framework...\n");
    
    /* Clear global state */
    memset(&device_framework, 0, sizeof(device_framework));
    
    /* Initialize locks */
    spinlock_init(&device_framework.device_lock);
    spinlock_init(&device_framework.bus_lock);
    spinlock_init(&device_framework.class_lock);
    spinlock_init(&device_framework.driver_lock);
    
    /* Initialize statistics */
    atomic_long_set(&device_framework.stats.devices_total, 0);
    atomic_long_set(&device_framework.stats.devices_active, 0);
    atomic_long_set(&device_framework.stats.drivers_loaded, 0);
    atomic_long_set(&device_framework.stats.probe_success, 0);
    atomic_long_set(&device_framework.stats.probe_failed, 0);
    
    device_framework.next_device_id = 1;
    
    /* Initialize built-in bus types */
    if (pci_bus_init() != 0) {
        kprintf("[DEV] PCI bus initialization failed\n");
        return -1;
    }
    
    if (usb_bus_init() != 0) {
        kprintf("[DEV] USB bus initialization failed\n");
        return -1;
    }
    
    /* Initialize built-in device classes */
    storage_class_init();
    network_class_init();
    gpu_class_init();
    audio_class_init();
    
    /* Initialize subsystem drivers */
    if (nvme_driver_init() != 0) {
        kprintf("[DEV] NVMe driver initialization failed\n");
        return -1;
    }
    
    if (gpu_driver_init() != 0) {
        kprintf("[DEV] GPU driver initialization failed\n");
        return -1;
    }
    
    kprintf("[DEV] Device framework initialized\n");
    return 0;
}

/**
 * Initialize PCI bus
 */
int pci_bus_init(void) {
    memset(&pci_bus_type, 0, sizeof(pci_bus_type));
    strcpy(pci_bus_type.name, "pci");
    
    pci_bus_type.match = pci_bus_match;
    pci_bus_type.probe = pci_bus_probe;
    pci_bus_type.remove = pci_bus_remove;
    pci_bus_type.enumerate = pci_bus_enumerate;
    pci_bus_type.add_device = pci_bus_add_device;
    pci_bus_type.remove_device = pci_bus_remove_device;
    
    spinlock_init(&pci_bus_type.lock);
    
    /* Register PCI bus */
    if (bus_register(&pci_bus_type) != 0) {
        return -1;
    }
    
    kprintf("[DEV] PCI bus initialized\n");
    return 0;
}

/**
 * Initialize USB bus
 */
int usb_bus_init(void) {
    memset(&usb_bus_type, 0, sizeof(usb_bus_type));
    strcpy(usb_bus_type.name, "usb");
    
    usb_bus_type.match = usb_bus_match;
    usb_bus_type.probe = usb_bus_probe;
    usb_bus_type.remove = usb_bus_remove;
    usb_bus_type.enumerate = usb_bus_enumerate;
    usb_bus_type.add_device = usb_bus_add_device;
    usb_bus_type.remove_device = usb_bus_remove_device;
    
    spinlock_init(&usb_bus_type.lock);
    
    /* Register USB bus */
    if (bus_register(&usb_bus_type) != 0) {
        return -1;
    }
    
    kprintf("[DEV] USB bus initialized\n");
    return 0;
}

/**
 * Create new device
 */
device_t *device_create(device_type_t type, const char *name) {
    device_t *dev = (device_t*)kzalloc(sizeof(device_t), GFP_KERNEL);
    if (!dev) {
        return NULL;
    }
    
    /* Initialize device structure */
    dev->type = type;
    dev->state = DEVICE_STATE_UNKNOWN;
    strncpy(dev->name, name, sizeof(dev->name) - 1);
    
    /* Assign unique device ID */
    spin_lock(&device_framework.device_lock);
    dev->device_id = device_framework.next_device_id++;
    spin_unlock(&device_framework.device_lock);
    
    /* Initialize locks */
    spinlock_init(&dev->lock);
    spinlock_init(&dev->power.lock);
    
    /* Initialize reference counting */
    atomic_set(&dev->ref_count, 1);
    
    /* Initialize power management */
    dev->power.current_state = POWER_STATE_D3_COLD;
    dev->power.target_state = POWER_STATE_D0;
    dev->power.can_wakeup = false;
    dev->power.runtime_pm = false;
    dev->power.usage_count = 0;
    
    /* Initialize DMA settings */
    dev->dma_info.dma_mask = 0xFFFFFFFFULL;           /* 32-bit by default */
    dev->dma_info.coherent_dma_mask = 0xFFFFFFFFULL;  /* 32-bit coherent */
    dev->dma_info.dma_coherent = true;
    
    /* Set as present */
    dev->present = true;
    dev->removable = false;
    
    kprintf("[DEV] Created device '%s' (ID: %u, Type: %d)\n", 
            dev->name, dev->device_id, dev->type);
    
    atomic_long_inc(&device_framework.stats.devices_total);
    
    return dev;
}

/**
 * Register device with framework
 */
int device_register(device_t *dev) {
    if (!dev) return -EINVAL;
    
    /* Add to global device list */
    spin_lock(&device_framework.device_lock);
    dev->next_global = device_framework.device_list;
    device_framework.device_list = dev;
    spin_unlock(&device_framework.device_lock);
    
    /* Add to bus if specified */
    if (dev->bus) {
        bus_add_device(dev->bus, dev);
    }
    
    /* Add to class if specified */
    if (dev->class) {
        class_add_device(dev->class, dev);
    }
    
    /* Try to find matching driver */
    device_t *matched_dev = dev;
    device_driver_t *drv = device_framework.driver_list;
    
    spin_lock(&device_framework.driver_lock);
    while (drv) {
        if (drv->match && drv->match(dev, drv)) {
            /* Found matching driver */
            dev->driver = drv;
            spin_unlock(&device_framework.driver_lock);
            
            /* Probe device */
            if (driver_probe_device(dev) == 0) {
                dev->state = DEVICE_STATE_ACTIVE;
                atomic_long_inc(&device_framework.stats.devices_active);
                atomic_long_inc(&device_framework.stats.probe_success);
                kprintf("[DEV] Device '%s' bound to driver '%s'\n", 
                        dev->name, drv->name);
            } else {
                dev->driver = NULL;
                atomic_long_inc(&device_framework.stats.probe_failed);
                kprintf("[DEV] Device '%s' probe failed\n", dev->name);
            }
            break;
        }
        drv = drv->next;
    }
    
    if (!dev->driver) {
        spin_unlock(&device_framework.driver_lock);
        kprintf("[DEV] No driver found for device '%s'\n", dev->name);
    }
    
    kprintf("[DEV] Registered device '%s'\n", dev->name);
    return 0;
}

/**
 * Register device driver
 */
int driver_register(device_driver_t *drv) {
    if (!drv) return -EINVAL;
    
    /* Add to driver list */
    spin_lock(&device_framework.driver_lock);
    drv->next = device_framework.driver_list;
    device_framework.driver_list = drv;
    spin_unlock(&device_framework.driver_lock);
    
    atomic_long_inc(&device_framework.stats.drivers_loaded);
    
    /* Try to match with existing unbound devices */
    device_t *dev = device_framework.device_list;
    
    spin_lock(&device_framework.device_lock);
    while (dev) {
        if (!dev->driver && drv->match && drv->match(dev, drv)) {
            dev->driver = drv;
            spin_unlock(&device_framework.device_lock);
            
            /* Probe device */
            if (driver_probe_device(dev) == 0) {
                dev->state = DEVICE_STATE_ACTIVE;
                atomic_long_inc(&device_framework.stats.devices_active);
                atomic_long_inc(&device_framework.stats.probe_success);
                kprintf("[DEV] Device '%s' bound to new driver '%s'\n", 
                        dev->name, drv->name);
            } else {
                dev->driver = NULL;
                atomic_long_inc(&device_framework.stats.probe_failed);
            }
            
            spin_lock(&device_framework.device_lock);
        }
        dev = dev->next_global;
    }
    spin_unlock(&device_framework.device_lock);
    
    kprintf("[DEV] Registered driver '%s'\n", drv->name);
    return 0;
}

/**
 * Probe device with driver
 */
int driver_probe_device(device_t *dev) {
    if (!dev || !dev->driver) return -EINVAL;
    
    device_driver_t *drv = dev->driver;
    
    kprintf("[DEV] Probing device '%s' with driver '%s'\n", 
            dev->name, drv->name);
    
    dev->state = DEVICE_STATE_INITIALIZING;
    
    /* Set device power state to active */
    if (device_set_power_state(dev, POWER_STATE_D0) != 0) {
        kprintf("[DEV] Failed to power on device '%s'\n", dev->name);
        return -EIO;
    }
    
    /* Call driver probe function */
    int result = 0;
    if (drv->probe) {
        result = drv->probe(dev);
    }
    
    if (result == 0) {
        dev->state = DEVICE_STATE_ACTIVE;
        kprintf("[DEV] Successfully probed device '%s'\n", dev->name);
    } else {
        dev->state = DEVICE_STATE_ERROR;
        kprintf("[DEV] Failed to probe device '%s' (error: %d)\n", 
                dev->name, result);
    }
    
    return result;
}

/**
 * Create PCI Express device
 */
pcie_device_t *pcie_device_create(uint8_t bus, uint8_t slot, uint8_t function) {
    pcie_device_t *pdev = (pcie_device_t*)kzalloc(sizeof(pcie_device_t), GFP_KERNEL);
    if (!pdev) {
        return NULL;
    }
    
    /* Initialize base device */
    device_t *dev = &pdev->device;
    dev->type = DEVICE_TYPE_PCI;
    dev->bus = &pci_bus_type;
    
    snprintf(dev->name, sizeof(dev->name), "pci:%02x:%02x.%x", bus, slot, function);
    
    /* Set PCI location */
    pdev->bus = bus;
    pdev->slot = slot;
    pdev->function = function;
    
    /* Read PCI configuration space */
    pci_read_config_space(pdev);
    
    /* Extract device information */
    uint32_t *config = pdev->config_space;
    dev->vendor_id = config[0] & 0xFFFF;
    dev->product_id = (config[0] >> 16) & 0xFFFF;
    dev->class_code = (config[2] >> 24) & 0xFF;
    dev->subclass_code = (config[2] >> 16) & 0xFF;
    dev->interface_code = (config[2] >> 8) & 0xFF;
    dev->revision = config[2] & 0xFF;
    
    /* Read base addresses */
    for (int i = 0; i < 6; i++) {
        uint32_t bar = config[4 + i];  /* BAR0-5 */
        if (bar != 0) {
            if (bar & 1) {
                /* I/O BAR */
                dev->resources.base_addr[i] = bar & ~3;
                dev->resources.flags[i] = 1;  /* I/O flag */
            } else {
                /* Memory BAR */
                dev->resources.base_addr[i] = bar & ~15;
                dev->resources.flags[i] = 0;  /* Memory flag */
                
                /* Handle 64-bit BAR */
                if ((bar & 6) == 4 && i < 5) {
                    dev->resources.base_addr[i] |= ((uint64_t)config[5 + i]) << 32;
                    i++;  /* Skip next BAR */
                }
            }
        }
    }
    
    /* Read interrupt line */
    dev->resources.irq = (config[15] >> 8) & 0xFF;
    
    /* Initialize PCIe capabilities */
    pcie_init_capabilities(pdev);
    
    /* Assign device ID */
    spin_lock(&device_framework.device_lock);
    dev->device_id = device_framework.next_device_id++;
    spin_unlock(&device_framework.device_lock);
    
    /* Initialize locks and reference counting */
    spinlock_init(&dev->lock);
    spinlock_init(&dev->power.lock);
    atomic_set(&dev->ref_count, 1);
    
    kprintf("[DEV] Created PCIe device %02x:%02x.%x (Vendor: 0x%04X, Device: 0x%04X)\n",
            bus, slot, function, dev->vendor_id, dev->product_id);
    
    return pdev;
}

/**
 * Create NVMe device
 */
nvme_device_t *nvme_device_create(pcie_device_t *pdev) {
    nvme_device_t *nvme = (nvme_device_t*)kzalloc(sizeof(nvme_device_t), GFP_KERNEL);
    if (!nvme) {
        return NULL;
    }
    
    /* Initialize base device */
    device_t *dev = &nvme->device;
    dev->type = DEVICE_TYPE_NVME;
    dev->parent = &pdev->device;
    
    snprintf(dev->name, sizeof(dev->name), "nvme%d", 0);  /* TODO: proper numbering */
    
    /* Copy device information from PCIe device */
    dev->vendor_id = pdev->device.vendor_id;
    dev->product_id = pdev->device.product_id;
    dev->class_code = pdev->device.class_code;
    
    /* Map NVMe controller registers (BAR0) */
    nvme->ctrl.bar0 = (void*)pdev->device.resources.base_addr[0];
    if (!nvme->ctrl.bar0) {
        kfree(nvme);
        return NULL;
    }
    
    /* Read controller capabilities */
    nvme->ctrl.capabilities = *(volatile uint32_t*)nvme->ctrl.bar0;
    nvme->ctrl.version = *(volatile uint32_t*)((uintptr_t)nvme->ctrl.bar0 + 8);
    
    /* Calculate queue parameters */
    nvme->ctrl.max_queues = ((nvme->ctrl.capabilities >> 16) & 0xFFFF) + 1;
    nvme->ctrl.queue_depth = (nvme->ctrl.capabilities & 0xFFFF) + 1;
    nvme->ctrl.page_size = 1 << (12 + ((nvme->ctrl.capabilities >> 48) & 0xF));
    
    /* Initialize admin queue */
    nvme->admin_queue.depth = 32;  /* Standard admin queue depth */
    spinlock_init(&nvme->admin_queue.lock);
    
    /* Initialize I/O queues */
    nvme->num_io_queues = min(nvme->ctrl.max_queues - 1, (uint16_t)64);
    for (int i = 0; i < nvme->num_io_queues; i++) {
        nvme->io_queues[i].depth = nvme->ctrl.queue_depth;
        nvme->io_queues[i].vector = i + 1;  /* MSI-X vector */
        spinlock_init(&nvme->io_queues[i].lock);
    }
    
    /* Initialize device */
    atomic_set(&dev->ref_count, 1);
    spinlock_init(&dev->lock);
    dev->state = DEVICE_STATE_DETECTED;
    
    kprintf("[DEV] Created NVMe device '%s' (Version: %08X, Max Queues: %u)\n",
            dev->name, nvme->ctrl.version, nvme->ctrl.max_queues);
    
    return nvme;
}

/**
 * Create GPU device
 */
gpu_device_t *gpu_device_create(pcie_device_t *pdev) {
    gpu_device_t *gpu = (gpu_device_t*)kzalloc(sizeof(gpu_device_t), GFP_KERNEL);
    if (!gpu) {
        return NULL;
    }
    
    /* Initialize base device */
    device_t *dev = &gpu->device;
    dev->type = DEVICE_TYPE_GPU;
    dev->parent = &pdev->device;
    dev->class = &gpu_class;
    
    snprintf(dev->name, sizeof(dev->name), "gpu%d", 0);  /* TODO: proper numbering */
    
    /* Copy device information from PCIe device */
    dev->vendor_id = pdev->device.vendor_id;
    dev->product_id = pdev->device.product_id;
    
    /* Determine GPU type */
    if (pdev->device.resources.base_addr[0] != 0) {
        gpu->gpu_type = GPU_TYPE_DISCRETE;
    } else {
        gpu->gpu_type = GPU_TYPE_INTEGRATED;
    }
    
    /* Initialize memory information based on vendor */
    switch (dev->vendor_id) {
        case 0x10DE:  /* NVIDIA */
            gpu_init_nvidia(gpu, pdev);
            break;
        case 0x1002:  /* AMD */
            gpu_init_amd(gpu, pdev);
            break;
        case 0x8086:  /* Intel */
            gpu_init_intel(gpu, pdev);
            break;
        default:
            /* Generic GPU */
            gpu->memory.vram_size = 256 * 1024 * 1024;  /* 256 MB default */
            gpu->compute.compute_units = 16;
            break;
    }
    
    /* Set display capabilities */
    gpu->display.max_displays = 4;
    gpu->display.max_resolution_x = 3840;
    gpu->display.max_resolution_y = 2160;
    gpu->display.max_refresh_rate = 120;
    gpu->display.hdmi_support = true;
    gpu->display.displayport_support = true;
    
    /* Set compute capabilities */
    gpu->compute.opencl_support = true;
    gpu->compute.vulkan_support = true;
    
    /* Initialize power management */
    gpu->power.base_power = 50;    /* 50W base */
    gpu->power.max_power = 250;    /* 250W max */
    gpu->power.dynamic_clocking = true;
    gpu->power.power_gating = true;
    
    /* Initialize device */
    atomic_set(&dev->ref_count, 1);
    spinlock_init(&dev->lock);
    dev->state = DEVICE_STATE_DETECTED;
    
    kprintf("[DEV] Created GPU device '%s' (VRAM: %lu MB, Compute Units: %u)\n",
            dev->name, gpu->memory.vram_size / (1024 * 1024), gpu->compute.compute_units);
    
    return gpu;
}

/**
 * Set device power state
 */
int device_set_power_state(device_t *dev, power_state_t state) {
    if (!dev) return -EINVAL;
    
    spin_lock(&dev->power.lock);
    
    if (dev->power.current_state == state) {
        spin_unlock(&dev->power.lock);
        return 0;  /* Already in desired state */
    }
    
    power_state_t old_state = dev->power.current_state;
    dev->power.target_state = state;
    
    /* Call driver power management if available */
    int result = 0;
    if (dev->driver) {
        if (state > old_state && dev->driver->suspend) {
            result = dev->driver->suspend(dev, state);
        } else if (state < old_state && dev->driver->resume) {
            result = dev->driver->resume(dev);
        }
    }
    
    if (result == 0) {
        dev->power.current_state = state;
        kprintf("[DEV] Device '%s' power state changed: %d -> %d\n", 
                dev->name, old_state, state);
    } else {
        dev->power.target_state = old_state;
        kprintf("[DEV] Failed to change power state for device '%s'\n", dev->name);
    }
    
    spin_unlock(&dev->power.lock);
    return result;
}

/**
 * Device enumeration functions
 */
int enumerate_pci_devices(void) {
    kprintf("[DEV] Enumerating PCI devices...\n");
    
    int device_count = 0;
    
    /* Scan all PCI buses */
    for (int bus = 0; bus < 256; bus++) {
        for (int slot = 0; slot < 32; slot++) {
            for (int function = 0; function < 8; function++) {
                /* Read vendor ID to check if device exists */
                uint16_t vendor_id = pci_read_config_word(bus, slot, function, 0);
                if (vendor_id == 0xFFFF) {
                    continue;  /* No device */
                }
                
                /* Create PCIe device */
                pcie_device_t *pdev = pcie_device_create(bus, slot, function);
                if (pdev) {
                    /* Register device */
                    device_register(&pdev->device);
                    device_count++;
                    
                    /* Check for specific device types */
                    uint8_t class_code = pdev->device.class_code;
                    
                    if (class_code == 0x01 && pdev->device.subclass_code == 0x08) {
                        /* NVMe controller */
                        nvme_device_t *nvme = nvme_device_create(pdev);
                        if (nvme) {
                            device_register(&nvme->device);
                            device_count++;
                        }
                    } else if (class_code == 0x03) {
                        /* Display controller (GPU) */
                        gpu_device_t *gpu = gpu_device_create(pdev);
                        if (gpu) {
                            device_register(&gpu->device);
                            device_count++;
                        }
                    }
                }
                
                /* Single function device */
                if (function == 0 && !(pci_read_config_byte(bus, slot, 0, 14) & 0x80)) {
                    break;
                }
            }
        }
    }
    
    kprintf("[DEV] Found %d PCI devices\n", device_count);
    return device_count;
}

/**
 * Show device framework statistics
 */
void device_framework_show_stats(void) {
    kprintf("[DEV] Device Framework Statistics:\n");
    kprintf("  Total devices: %lu\n", 
            atomic_long_read(&device_framework.stats.devices_total));
    kprintf("  Active devices: %lu\n", 
            atomic_long_read(&device_framework.stats.devices_active));
    kprintf("  Loaded drivers: %lu\n", 
            atomic_long_read(&device_framework.stats.drivers_loaded));
    kprintf("  Successful probes: %lu\n", 
            atomic_long_read(&device_framework.stats.probe_success));
    kprintf("  Failed probes: %lu\n", 
            atomic_long_read(&device_framework.stats.probe_failed));
}

/**
 * Dump device tree
 */
void device_tree_dump(void) {
    kprintf("[DEV] Device Tree:\n");
    
    device_t *dev = device_framework.device_list;
    
    spin_lock(&device_framework.device_lock);
    while (dev) {
        device_dump_info(dev);
        dev = dev->next_global;
    }
    spin_unlock(&device_framework.device_lock);
}

/**
 * Dump device information
 */
void device_dump_info(device_t *dev) {
    if (!dev) return;
    
    kprintf("  Device: %s (ID: %u)\n", dev->name, dev->device_id);
    kprintf("    Type: %d, State: %d\n", dev->type, dev->state);
    kprintf("    Vendor: 0x%04X, Product: 0x%04X\n", dev->vendor_id, dev->product_id);
    kprintf("    Class: 0x%02X, Subclass: 0x%02X, Interface: 0x%02X\n",
            dev->class_code, dev->subclass_code, dev->interface_code);
    kprintf("    Driver: %s\n", dev->driver ? dev->driver->name : "None");
    kprintf("    Power State: %d\n", dev->power.current_state);
    
    for (int i = 0; i < 6; i++) {
        if (dev->resources.base_addr[i] != 0) {
            kprintf("    BAR%d: 0x%016lX (Size: %zu)\n", 
                    i, dev->resources.base_addr[i], dev->resources.size[i]);
        }
    }
    
    if (dev->resources.irq != 0) {
        kprintf("    IRQ: %d\n", dev->resources.irq);
    }
}

/**
 * Helper functions for device class initialization
 */
void storage_class_init(void) {
    memset(&storage_class, 0, sizeof(storage_class));
    strcpy(storage_class.name, "storage");
    spinlock_init(&storage_class.lock);
    class_register(&storage_class);
}

void network_class_init(void) {
    memset(&network_class, 0, sizeof(network_class));
    strcpy(network_class.name, "network");
    spinlock_init(&network_class.lock);
    class_register(&network_class);
}

void gpu_class_init(void) {
    memset(&gpu_class, 0, sizeof(gpu_class));
    strcpy(gpu_class.name, "gpu");
    spinlock_init(&gpu_class.lock);
    class_register(&gpu_class);
}

void audio_class_init(void) {
    memset(&audio_class, 0, sizeof(audio_class));
    strcpy(audio_class.name, "audio");
    spinlock_init(&audio_class.lock);
    class_register(&audio_class);
}

/**
 * PCI configuration space access
 */
uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    uint32_t address = (1UL << 31) | (bus << 16) | (slot << 11) | (function << 8) | (offset & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

uint16_t pci_read_config_word(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    uint32_t dword = pci_read_config_dword(bus, slot, function, offset);
    return (uint16_t)((dword >> ((offset & 2) * 8)) & 0xFFFF);
}

uint8_t pci_read_config_byte(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    uint32_t dword = pci_read_config_dword(bus, slot, function, offset);
    return (uint8_t)((dword >> ((offset & 3) * 8)) & 0xFF);
}

void pci_read_config_space(pcie_device_t *pdev) {
    for (int i = 0; i < 64; i++) {
        pdev->config_space[i] = pci_read_config_dword(pdev->bus, pdev->slot, pdev->function, i * 4);
    }
}

/**
 * Placeholder implementations for driver-specific functions
 */
int nvme_driver_init(void) {
    kprintf("[DEV] NVMe driver initialized\n");
    return 0;
}

int gpu_driver_init(void) {
    kprintf("[DEV] GPU driver initialized\n");
    return 0;
}

/* Bus match/probe functions - simplified implementations */
int pci_bus_match(device_t *dev, device_driver_t *drv) { return 1; }
int pci_bus_probe(device_t *dev) { return 0; }
int pci_bus_remove(device_t *dev) { return 0; }
int pci_bus_enumerate(bus_type_t *bus) { return enumerate_pci_devices(); }
int pci_bus_add_device(bus_type_t *bus, device_t *dev) { return 0; }
void pci_bus_remove_device(device_t *dev) {}

int usb_bus_match(device_t *dev, device_driver_t *drv) { return 1; }
int usb_bus_probe(device_t *dev) { return 0; }
int usb_bus_remove(device_t *dev) { return 0; }
int usb_bus_enumerate(bus_type_t *bus) { return 0; }
int usb_bus_add_device(bus_type_t *bus, device_t *dev) { return 0; }
void usb_bus_remove_device(device_t *dev) {}

void gpu_init_nvidia(gpu_device_t *gpu, pcie_device_t *pdev) {
    gpu->memory.vram_size = 8ULL * 1024 * 1024 * 1024;  /* 8 GB */
    gpu->compute.compute_units = 128;
    gpu->compute.cuda_support = true;
}

void gpu_init_amd(gpu_device_t *gpu, pcie_device_t *pdev) {
    gpu->memory.vram_size = 16ULL * 1024 * 1024 * 1024;  /* 16 GB */
    gpu->compute.compute_units = 64;
    gpu->compute.opencl_support = true;
}

void gpu_init_intel(gpu_device_t *gpu, pcie_device_t *pdev) {
    gpu->memory.vram_size = 0;  /* Shared memory */
    gpu->memory.unified_memory = true;
    gpu->compute.compute_units = 32;
}

void pcie_init_capabilities(pcie_device_t *pdev) {
    /* TODO: Initialize PCIe capabilities */
}

int bus_register(bus_type_t *bus) {
    /* TODO: Implement bus registration */
    return 0;
}

int class_register(device_class_t *class) {
    /* TODO: Implement class registration */
    return 0;
}

int bus_add_device(bus_type_t *bus, device_t *dev) {
    /* TODO: Implement bus device addition */
    return 0;
}

int class_add_device(device_class_t *class, device_t *dev) {
    /* TODO: Implement class device addition */
    return 0;
}