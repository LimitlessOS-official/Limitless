/*
 * LimitlessOS Kernel Hardware Abstraction Layer
 * Core hardware detection and driver implementation
 */

#include <stddef.h>
#include <kernel.h>
#include "../hal_kernel.h"

/* Global device registry */
static hal_device_info_t devices[64];
static int device_count = 0;
static bool hal_initialized = false;

/* Input system state */
static bool input_initialized = false;
static input_event_t event_queue[32];
static int event_head = 0, event_tail = 0;

/* Storage system state */
static storage_info_t storage_devices[8];
static int storage_count = 0;

/* Network system state */
static network_info_t network_devices[4];
static int network_count = 0;

/* PCI system state */
static pci_device_info_t pci_devices[32];
static int pci_count = 0;

/* Timer state */
static uint64_t timer_ticks = 0;
static uint64_t timer_frequency = 1000;  /* 1 KHz */

/* Graphics state */
static void* framebuffer = (void*)0xB8000;  /* VGA text mode buffer */
static int screen_width = 80;
static int screen_height = 25;
static int screen_bpp = 4;  /* 4-bit color in text mode */

/* Initialize HAL subsystem */
int hal_init(void) {
    if (hal_initialized) {
        return STATUS_OK;
    }
    
    device_count = 0;
    
    /* Initialize all subsystems */
    hal_pci_init();
    hal_input_init();
    hal_storage_init();
    hal_network_init();
    hal_timer_init();
    hal_graphics_init();
    hal_audio_init();
    
    hal_initialized = true;
    return STATUS_OK;
}

/* Shutdown HAL subsystem */
void hal_shutdown(void) {
    hal_initialized = false;
    device_count = 0;
}

/* Enumerate all hardware devices */
int hal_enumerate_devices(void) {
    device_count = 0;
    
    /* Add keyboard device */
    if (device_count < 64) {
        hal_device_info_t* dev = &devices[device_count++];
        dev->id = 0;
        dev->type = DEVICE_KEYBOARD;
        dev->present = true;
        dev->initialized = true;
        dev->io_base = 0x60;
        dev->irq = 1;
        for (int i = 0; i < 64 && "PS/2 Keyboard"[i]; i++) {
            dev->name[i] = "PS/2 Keyboard"[i];
        }
    }
    
    /* Add mouse device */
    if (device_count < 64) {
        hal_device_info_t* dev = &devices[device_count++];
        dev->id = 1;
        dev->type = DEVICE_MOUSE;
        dev->present = true;
        dev->initialized = true;
        dev->io_base = 0x60;
        dev->irq = 12;
        for (int i = 0; i < 64 && "PS/2 Mouse"[i]; i++) {
            dev->name[i] = "PS/2 Mouse"[i];
        }
    }
    
    /* Add storage device */
    if (device_count < 64) {
        hal_device_info_t* dev = &devices[device_count++];
        dev->id = 2;
        dev->type = DEVICE_STORAGE;
        dev->present = true;
        dev->initialized = true;
        dev->io_base = 0x1F0;
        dev->irq = 14;
        for (int i = 0; i < 64 && "ATA Primary Master"[i]; i++) {
            dev->name[i] = "ATA Primary Master"[i];
        }
    }
    
    /* Add network device */
    if (device_count < 64) {
        hal_device_info_t* dev = &devices[device_count++];
        dev->id = 3;
        dev->type = DEVICE_NETWORK;
        dev->present = true;
        dev->initialized = true;
        dev->io_base = 0;
        dev->irq = 11;
        for (int i = 0; i < 64 && "Network Controller"[i]; i++) {
            dev->name[i] = "Network Controller"[i];
        }
    }
    
    return device_count;
}

/* Get device count by type */
int hal_get_device_count(hal_device_type_t type) {
    int count = 0;
    for (int i = 0; i < device_count; i++) {
        if (devices[i].type == type && devices[i].present) {
            count++;
        }
    }
    return count;
}

/* Get device info by type and index */
hal_device_info_t* hal_get_device_info(hal_device_type_t type, int index) {
    int count = 0;
    for (int i = 0; i < device_count; i++) {
        if (devices[i].type == type && devices[i].present) {
            if (count == index) {
                return &devices[i];
            }
            count++;
        }
    }
    return NULL;
}

/* Initialize input subsystem */
int hal_input_init(void) {
    if (input_initialized) {
        return STATUS_OK;
    }
    
    event_head = event_tail = 0;
    input_initialized = true;
    return STATUS_OK;
}

/* Get input event from queue */
int hal_input_get_event(input_event_t* event) {
    if (!event || event_head == event_tail) {
    return STATUS_ETIMEDOUT;
    }
    
    *event = event_queue[event_head];
    event_head = (event_head + 1) % 32;
    return STATUS_OK;
}

/* Check for keyboard */
bool hal_input_has_keyboard(void) {
    return hal_get_device_count(DEVICE_KEYBOARD) > 0;
}

/* Check for mouse */
bool hal_input_has_mouse(void) {
    return hal_get_device_count(DEVICE_MOUSE) > 0;
}

/* Initialize storage subsystem */
int hal_storage_init(void) {
    storage_count = 0;
    
    /* Detect ATA/IDE drives */
    if (storage_count < 8) {
        storage_info_t* dev = &storage_devices[storage_count++];
        dev->device_id = 0;
        dev->capacity_bytes = 1024ULL * 1024 * 1024;  /* 1GB */
        dev->sector_size = 512;
        dev->removable = false;
        dev->read_only = false;
        for (int i = 0; i < 64 && "LIMITLESS_DISK_0"[i]; i++) {
            dev->model[i] = "LIMITLESS_DISK_0"[i];
        }
        for (int i = 0; i < 32 && "LOS2024"[i]; i++) {
            dev->serial[i] = "LOS2024"[i];
        }
    }
    
    return STATUS_OK;
}

/* Get storage device count */
int hal_storage_get_device_count(void) {
    return storage_count;
}

/* Get storage device info */
storage_info_t* hal_storage_get_device_info(int device_id) {
    if (device_id < 0 || device_id >= storage_count) {
        return NULL;
    }
    return &storage_devices[device_id];
}

/* Read sectors from storage */
int hal_storage_read_sectors(int device_id, uint64_t lba, uint32_t count, void* buffer) {
    /* Simulate read operation */
    if (!buffer || device_id < 0 || device_id >= storage_count) {
        return STATUS_INVALID;
    }
    
    /* Zero out buffer to simulate read */
    char* buf = (char*)buffer;
    for (uint32_t i = 0; i < count * 512; i++) {
        buf[i] = 0;
    }
    
    return STATUS_OK;
}

/* Write sectors to storage */
int hal_storage_write_sectors(int device_id, uint64_t lba, uint32_t count, const void* buffer) {
    /* Simulate write operation */
    if (!buffer || device_id < 0 || device_id >= storage_count) {
        return STATUS_INVALID;
    }
    return STATUS_OK;
}

/* Initialize network subsystem */
int hal_network_init(void) {
    network_count = 0;
    
    /* Add virtual network device */
    if (network_count < 4) {
        network_info_t* dev = &network_devices[network_count++];
        dev->device_id = 0;
        dev->mac_address[0] = 0x52;
        dev->mac_address[1] = 0x54;
        dev->mac_address[2] = 0x00;
        dev->mac_address[3] = 0x12;
        dev->mac_address[4] = 0x34;
        dev->mac_address[5] = 0x56;
        dev->link_speed_mbps = 1000;
        dev->link_up = true;
        dev->bytes_sent = 0;
        dev->bytes_received = 0;
        for (int i = 0; i < 32 && "eth0"[i]; i++) {
            dev->name[i] = "eth0"[i];
        }
    }
    
    return STATUS_OK;
}

/* Get network device count */
int hal_network_get_device_count(void) {
    return network_count;
}

/* Get network device info */
network_info_t* hal_network_get_device_info(int device_id) {
    if (device_id < 0 || device_id >= network_count) {
        return NULL;
    }
    return &network_devices[device_id];
}

/* Send network packet */
int hal_network_send_packet(int device_id, const void* data, uint32_t size) {
    if (device_id < 0 || device_id >= network_count || !data || size == 0) {
        return STATUS_INVALID;
    }
    
    network_devices[device_id].bytes_sent += size;
    return STATUS_OK;
}

/* Receive network packet */
int hal_network_receive_packet(int device_id, void* buffer, uint32_t max_size) {
    /* No packets to receive in simulation */
    return STATUS_ETIMEDOUT;
}

/* Initialize PCI subsystem */
int hal_pci_init(void) {
    pci_count = 0;
    
    /* Add some virtual PCI devices */
    if (pci_count < 32) {
        pci_device_info_t* dev = &pci_devices[pci_count++];
        dev->bus = 0;
        dev->device = 0;
        dev->function = 0;
        dev->vendor_id = 0x8086;  /* Intel */
        dev->device_id = 0x1237;  /* 440FX chipset */
        dev->class_code = 0x06;   /* Bridge */
        dev->subclass = 0x00;
        dev->prog_if = 0x00;
        dev->interrupt_line = 0;
    }
    
    return STATUS_OK;
}

/* Get PCI device count */
int hal_pci_get_device_count(void) {
    return pci_count;
}

/* Get PCI device info */
pci_device_info_t* hal_pci_get_device_info(int index) {
    if (index < 0 || index >= pci_count) {
        return NULL;
    }
    return &pci_devices[index];
}

/* Find PCI device by vendor/device ID */
pci_device_info_t* hal_pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    for (int i = 0; i < pci_count; i++) {
        if (pci_devices[i].vendor_id == vendor_id && 
            pci_devices[i].device_id == device_id) {
            return &pci_devices[i];
        }
    }
    return NULL;
}

/* Initialize timer subsystem */
int hal_timer_init(void) {
    timer_ticks = 0;
    timer_frequency = 1000;  /* 1 KHz */
    return STATUS_OK;
}

/* Get timer ticks */
uint64_t hal_timer_get_ticks(void) {
    /* Increment ticks (would be done by interrupt) */
    timer_ticks++;
    return timer_ticks;
}

/* Get timer frequency */
uint64_t hal_timer_get_frequency(void) {
    return timer_frequency;
}

/* Delay for milliseconds */
void hal_timer_delay_ms(uint32_t milliseconds) {
    uint64_t start = hal_timer_get_ticks();
    uint64_t target = start + (milliseconds * timer_frequency / 1000);
    while (hal_timer_get_ticks() < target) {
        hal_cpu_pause();
    }
}

/* Initialize graphics subsystem */
int hal_graphics_init(void) {
    /* Using VGA text mode by default */
    framebuffer = (void*)0xB8000;
    screen_width = 80;
    screen_height = 25;
    screen_bpp = 4;
    return STATUS_OK;
}

/* Get graphics mode count */
int hal_graphics_get_mode_count(void) {
    return 1;  /* Only VGA text mode */
}

/* Set graphics mode */
int hal_graphics_set_mode(int width, int height, int bpp) {
    /* Only support VGA text mode for now */
    if (width == 80 && height == 25 && bpp == 4) {
        return STATUS_OK;
    }
    return STATUS_ERROR;
}

/* Get framebuffer pointer */
void* hal_graphics_get_framebuffer(void) {
    return framebuffer;
}

/* Get screen dimensions */
int hal_graphics_get_width(void) { return screen_width; }
int hal_graphics_get_height(void) { return screen_height; }
int hal_graphics_get_bpp(void) { return screen_bpp; }

/* Initialize audio subsystem */
int hal_audio_init(void) {
    return STATUS_OK;  /* No audio devices for now */
}

/* Get audio device count */
int hal_audio_get_device_count(void) {
    return 0;  /* No audio devices */
}

/* CPU operations */
void hal_cpu_pause(void) {
    __asm__ volatile("pause");
}

void hal_cpu_disable_interrupts(void) {
    __asm__ volatile("cli");
}

void hal_cpu_enable_interrupts(void) {
    __asm__ volatile("sti");
}

bool hal_cpu_interrupts_enabled(void) {
    uint64_t flags;
    __asm__ volatile("pushf; pop %0" : "=r"(flags));
    return !!(flags & 0x200);
}

/* Utility functions */
const char* hal_device_type_name(hal_device_type_t type) {
    switch (type) {
        case DEVICE_KEYBOARD: return "Keyboard";
        case DEVICE_MOUSE: return "Mouse";
        case DEVICE_STORAGE: return "Storage";
        case DEVICE_NETWORK: return "Network";
        case DEVICE_GRAPHICS: return "Graphics";
        case DEVICE_AUDIO: return "Audio";
        case DEVICE_TIMER: return "Timer";
        case DEVICE_PCI: return "PCI";
        case DEVICE_USB: return "USB";
        default: return "Unknown";
    }
}

/* Debug print all devices */
void hal_debug_print_devices(void) {
    /* Would print device information - not implemented in kernel context */
}