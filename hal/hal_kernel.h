/* Provide global outb for kernel linkage */
extern void outb(uint16_t port, uint8_t value);
/*
 * LimitlessOS Kernel HAL Interface
 * Simplified HAL definitions for kernel integration
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "hal_core.h"

/* Status codes */

// Use device_type_t from hal_core.h

/* Device information */
typedef struct {
    uint32_t id;
    device_type_t type;
    char name[64];
    bool present;
    bool initialized;
    uint32_t io_base;
    uint32_t irq;
    uint64_t memory_base;
    uint32_t memory_size;
} hal_device_info_t;

/* Input event types */
typedef enum {
    INPUT_EVENT_KEY_PRESS,
    INPUT_EVENT_KEY_RELEASE,
    INPUT_EVENT_MOUSE_MOVE,
    INPUT_EVENT_MOUSE_BUTTON
} input_event_type_t;

/* Input event structure */
typedef struct {
    input_event_type_t type;
    uint64_t timestamp;
    union {
        struct {
            uint32_t keycode;
            uint32_t modifiers;
        } key;
        struct {
            int32_t dx, dy;
            uint32_t x, y;
        } mouse_move;
        struct {
            uint32_t button;
            bool pressed;
            uint32_t x, y;
        } mouse_button;
    };
} input_event_t;

/* Storage device information */
typedef struct {
    uint32_t device_id;
    uint64_t capacity_bytes;
    uint32_t sector_size;
    char model[64];
    char serial[32];
    bool removable;
    bool read_only;
} storage_info_t;

/* Network device information */
typedef struct {
    uint32_t device_id;
    uint8_t mac_address[6];
    uint32_t link_speed_mbps;
    bool link_up;
    char name[32];
    uint64_t bytes_sent;
    uint64_t bytes_received;
} network_info_t;

/* PCI device information */
typedef struct {
    uint32_t bus;
    uint32_t device;
    uint32_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint32_t base_addresses[6];
    uint8_t interrupt_line;
} pci_device_info_t;

/* HAL function prototypes */
int hal_init(void);
void hal_shutdown(void);

/* Architecture-specific address space switch (stub for kernel linkage) */
void hal_arch_switch_aspace(void* new_aspace);

/* Device enumeration */
int hal_enumerate_devices(void);
int hal_get_device_count(device_type_t type);
hal_device_info_t* hal_get_device_info(device_type_t type, int index);

/* Input subsystem */
int hal_input_init(void);
int hal_input_get_event(input_event_t* event);
bool hal_input_has_keyboard(void);
bool hal_input_has_mouse(void);

/* Storage subsystem */
int hal_storage_init(void);
int hal_storage_get_device_count(void);
storage_info_t* hal_storage_get_device_info(int device_id);
int hal_storage_read_sectors(int device_id, uint64_t lba, uint32_t count, void* buffer);
int hal_storage_write_sectors(int device_id, uint64_t lba, uint32_t count, const void* buffer);

/* Network subsystem */
int hal_network_init(void);
int hal_network_get_device_count(void);
network_info_t* hal_network_get_device_info(int device_id);
int hal_network_send_packet(int device_id, const void* data, uint32_t size);
int hal_network_receive_packet(int device_id, void* buffer, uint32_t max_size);

/* PCI subsystem */
int hal_pci_init(void);
int hal_pci_get_device_count(void);
pci_device_info_t* hal_pci_get_device_info(int index);
pci_device_info_t* hal_pci_find_device(uint16_t vendor_id, uint16_t device_id);
uint32_t hal_pci_read_config(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset);
void hal_pci_write_config(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t value);

/* Timer subsystem */
int hal_timer_init(void);
uint64_t hal_timer_get_ticks(void);
uint64_t hal_timer_get_frequency(void);
void hal_timer_delay_ms(uint32_t milliseconds);

/* Graphics subsystem */
int hal_graphics_init(void);
int hal_graphics_get_mode_count(void);
int hal_graphics_set_mode(int width, int height, int bpp);
void* hal_graphics_get_framebuffer(void);
int hal_graphics_get_width(void);
int hal_graphics_get_height(void);
int hal_graphics_get_bpp(void);

/* Audio subsystem */
int hal_audio_init(void);
int hal_audio_get_device_count(void);
int hal_audio_play_buffer(int device_id, const void* buffer, uint32_t size);
int hal_audio_set_volume(int device_id, uint32_t volume);

/* CPU operations */
void hal_cpu_pause(void);
void hal_cpu_disable_interrupts(void);
void hal_cpu_enable_interrupts(void);
bool hal_cpu_interrupts_enabled(void);

/* Utility functions */
// Use hal_device_type_name from hal_core.h
void hal_debug_print_devices(void);