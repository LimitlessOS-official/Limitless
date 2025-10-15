/*
 * Device Driver Framework Implementation
 * 
 * Implements device and driver management:
 * - Device registration/unregistration
 * - Driver registration and auto-probing
 * - Device lookup and enumeration
 * - I/O operation dispatch
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "kernel.h"
#include "device.h"
#include <mm/mm.h>
#include <string.h>

// Forward declarations
extern void* kmalloc(size_t size);
extern void kfree(void* ptr);

// Global device and driver lists
static device_t* device_list = NULL;
static driver_t* driver_list = NULL;
static device_class_t* class_list = NULL;

// Device number allocation
static u32 next_major = 1;

// Initialize device subsystem
void device_init(void) {
    device_list = NULL;
    driver_list = NULL;
    class_list = NULL;
    
    kprintf("[DEVICE] Device subsystem initialized\n");
}

// Register a device
int device_register(device_t* dev) {
    if (!dev) return -1;
    
    // Add to device list
    dev->next = device_list;
    device_list = dev;
    
    kprintf("[DEVICE] Registered device: %s (type=%u, major=%u, minor=%u)\n",
            dev->name, dev->type, dev->major, dev->minor);
    
    // Try to match with a driver
    driver_t* drv = driver_list;
    while (drv) {
        if (drv->type == dev->type || drv->type == 0) {
            // Type matches, try to probe
            if (drv->ops && drv->ops->probe) {
                if (drv->ops->probe(dev) == 0) {
                    dev->driver = drv;
                    kprintf("[DEVICE] Driver '%s' bound to device '%s'\n", 
                            drv->name, dev->name);
                    break;
                }
            }
        }
        drv = drv->next;
    }
    
    return 0;
}

// Unregister a device
int device_unregister(device_t* dev) {
    if (!dev) return -1;
    
    // Remove driver binding
    if (dev->driver && dev->driver->ops && dev->driver->ops->remove) {
        dev->driver->ops->remove(dev);
    }
    
    // Remove from device list
    device_t** ptr = &device_list;
    while (*ptr) {
        if (*ptr == dev) {
            *ptr = dev->next;
            kprintf("[DEVICE] Unregistered device: %s\n", dev->name);
            return 0;
        }
        ptr = &(*ptr)->next;
    }
    
    return -1;
}

// Find device by name
device_t* device_find_by_name(const char* name) {
    device_t* dev = device_list;
    while (dev) {
        if (strcmp(dev->name, name) == 0) {
            return dev;
        }
        dev = dev->next;
    }
    return NULL;
}

// Find device by major/minor number
device_t* device_find_by_number(u32 major, u32 minor) {
    device_t* dev = device_list;
    while (dev) {
        if (dev->major == major && dev->minor == minor) {
            return dev;
        }
        dev = dev->next;
    }
    return NULL;
}

// Register a driver
int driver_register(driver_t* drv) {
    if (!drv) return -1;
    
    // Add to driver list
    drv->next = driver_list;
    driver_list = drv;
    
    kprintf("[DEVICE] Registered driver: %s (type=%u)\n", drv->name, drv->type);
    
    // Probe existing devices
    device_t* dev = device_list;
    while (dev) {
        if (!dev->driver && (drv->type == dev->type || drv->type == 0)) {
            if (drv->ops && drv->ops->probe) {
                if (drv->ops->probe(dev) == 0) {
                    dev->driver = drv;
                    kprintf("[DEVICE] Driver '%s' bound to device '%s'\n",
                            drv->name, dev->name);
                }
            }
        }
        dev = dev->next;
    }
    
    return 0;
}

// Unregister a driver
int driver_unregister(driver_t* drv) {
    if (!drv) return -1;
    
    // Unbind from devices
    device_t* dev = device_list;
    while (dev) {
        if (dev->driver == drv) {
            if (drv->ops && drv->ops->remove) {
                drv->ops->remove(dev);
            }
            dev->driver = NULL;
        }
        dev = dev->next;
    }
    
    // Remove from driver list
    driver_t** ptr = &driver_list;
    while (*ptr) {
        if (*ptr == drv) {
            *ptr = drv->next;
            kprintf("[DEVICE] Unregistered driver: %s\n", drv->name);
            return 0;
        }
        ptr = &(*ptr)->next;
    }
    
    return -1;
}

// Create a device class
device_class_t* device_class_create(const char* name) {
    device_class_t* class = kmalloc(sizeof(device_class_t));
    if (!class) return NULL;
    
    memset(class, 0, sizeof(device_class_t));
    strncpy(class->name, name, 31);
    class->name[31] = '\0';
    
    // Add to class list
    class->next = class_list;
    class_list = class;
    
    kprintf("[DEVICE] Created device class: %s\n", name);
    return class;
}

// Add device to class
int device_class_add_device(device_class_t* class, device_t* dev) {
    if (!class || !dev) return -1;
    
    dev->class = class;
    dev->next = class->devices;
    class->devices = dev;
    
    return 0;
}

// Device I/O operations
int device_open(device_t* dev) {
    if (!dev || !dev->ops || !dev->ops->open) return -1;
    return dev->ops->open(dev);
}

int device_close(device_t* dev) {
    if (!dev || !dev->ops || !dev->ops->close) return -1;
    return dev->ops->close(dev);
}

long device_read(device_t* dev, u64 offset, void* buf, size_t len) {
    if (!dev || !dev->ops || !dev->ops->read) return -1;
    return dev->ops->read(dev, offset, buf, len);
}

long device_write(device_t* dev, u64 offset, const void* buf, size_t len) {
    if (!dev || !dev->ops || !dev->ops->write) return -1;
    return dev->ops->write(dev, offset, buf, len);
}

int device_ioctl(device_t* dev, u32 cmd, void* arg) {
    if (!dev || !dev->ops || !dev->ops->ioctl) return -1;
    return dev->ops->ioctl(dev, cmd, arg);
}

// Character device helper
device_t* char_device_create(const char* name, u32 major, u32 minor) {
    device_t* dev = kmalloc(sizeof(device_t));
    if (!dev) return NULL;
    
    memset(dev, 0, sizeof(device_t));
    strncpy(dev->name, name, 63);
    dev->name[63] = '\0';
    dev->type = DEV_TYPE_CHAR;
    dev->major = major ? major : next_major++;
    dev->minor = minor;
    dev->flags = DEV_FLAG_READY;
    
    return dev;
}

// Block device helper
device_t* block_device_create(const char* name, u32 major, u32 minor, u64 size) {
    device_t* dev = kmalloc(sizeof(device_t));
    if (!dev) return NULL;
    
    memset(dev, 0, sizeof(device_t));
    strncpy(dev->name, name, 63);
    dev->name[63] = '\0';
    dev->type = DEV_TYPE_BLOCK;
    dev->major = major ? major : next_major++;
    dev->minor = minor;
    dev->flags = DEV_FLAG_READY;
    
    // Store size in private_data for now
    dev->private_data = (void*)size;
    
    return dev;
}
