/*
 * Device Driver Framework
 * 
 * Defines the device driver model with:
 * - Device registration and management
 * - Driver registration and matching
 * - Device classes (block, character, network)
 * - I/O operations interface
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>
#include <stddef.h>

// Type aliases for kernel use
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Device types
#define DEV_TYPE_CHAR    1  // Character device (keyboard, serial, etc.)
#define DEV_TYPE_BLOCK   2  // Block device (disk, etc.)
#define DEV_TYPE_NETWORK 3  // Network device
#define DEV_TYPE_OTHER   4  // Other device types

// Device flags
#define DEV_FLAG_READY   0x01  // Device is ready for I/O
#define DEV_FLAG_BUSY    0x02  // Device is busy
#define DEV_FLAG_ERROR   0x04  // Device has error
#define DEV_FLAG_HOTPLUG 0x08  // Device supports hotplug

// Forward declarations
typedef struct device device_t;
typedef struct driver driver_t;
typedef struct device_class device_class_t;

// Device operations
typedef struct device_ops {
    int (*open)(device_t* dev);
    int (*close)(device_t* dev);
    long (*read)(device_t* dev, u64 offset, void* buf, size_t len);
    long (*write)(device_t* dev, u64 offset, const void* buf, size_t len);
    int (*ioctl)(device_t* dev, u32 cmd, void* arg);
    int (*poll)(device_t* dev, u32 events);
} device_ops_t;

// Device structure
struct device {
    char name[64];              // Device name (e.g., "sda", "tty0")
    u32 type;                   // Device type (DEV_TYPE_*)
    u32 flags;                  // Device flags
    u32 major;                  // Major device number
    u32 minor;                  // Minor device number
    
    driver_t* driver;           // Associated driver
    device_class_t* class;      // Device class
    void* private_data;         // Driver-specific data
    
    device_ops_t* ops;          // Device operations
    
    // Link in device list
    device_t* next;
};

// Driver operations
typedef struct driver_ops {
    int (*probe)(device_t* dev);     // Probe device
    int (*remove)(device_t* dev);    // Remove device
    int (*suspend)(device_t* dev);   // Suspend device
    int (*resume)(device_t* dev);    // Resume device
} driver_ops_t;

// Driver structure
struct driver {
    char name[64];              // Driver name
    u32 type;                   // Device type this driver supports
    
    driver_ops_t* ops;          // Driver operations
    
    // Link in driver list
    driver_t* next;
};

// Device class (for grouping similar devices)
struct device_class {
    char name[32];              // Class name (e.g., "block", "input")
    device_t* devices;          // List of devices in this class
    device_class_t* next;       // Link in class list
};

// Device registration and management
int device_register(device_t* dev);
int device_unregister(device_t* dev);
device_t* device_find_by_name(const char* name);
device_t* device_find_by_number(u32 major, u32 minor);

// Driver registration
int driver_register(driver_t* drv);
int driver_unregister(driver_t* drv);

// Device class management
device_class_t* device_class_create(const char* name);
int device_class_add_device(device_class_t* class, device_t* dev);

// Device operations
int device_open(device_t* dev);
int device_close(device_t* dev);
long device_read(device_t* dev, u64 offset, void* buf, size_t len);
long device_write(device_t* dev, u64 offset, const void* buf, size_t len);
int device_ioctl(device_t* dev, u32 cmd, void* arg);

// Initialize device subsystem
void device_init(void);

// Character device helpers
device_t* char_device_create(const char* name, u32 major, u32 minor);

// Block device helpers
device_t* block_device_create(const char* name, u32 major, u32 minor, u64 size);

#endif // DEVICE_H
