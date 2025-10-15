/*
 * Audio Hardware Abstraction Layer
 * Supports AC'97, Intel HDA, and USB Audio devices
 */
#include "hal.h"
#include "pci_cfg.h"

#define MAX_AUDIO_DEVICES 8
#define AUDIO_BUFFER_SIZE 4096

/* Audio device types */
typedef enum {
    AUDIO_TYPE_AC97,
    AUDIO_TYPE_HDA,
    AUDIO_TYPE_USB_AUDIO
} audio_type_t;

/* Audio device structure */
typedef struct {
    bool active;
    audio_type_t type;
    uint16_t vendor_id;
    uint16_t device_id;
    void* mmio_base;
    uint32_t sample_rate;
    uint16_t channels;
    uint16_t bit_depth;
    char name[64];
} hal_audio_device_t;

/* Global audio state */
static hal_audio_device_t audio_devices[MAX_AUDIO_DEVICES];
static uint32_t audio_device_count = 0;
static bool audio_initialized = false;

/* Initialize audio subsystem */
status_t hal_audio_init(void) {
    if (audio_initialized) {
        return STATUS_OK;
    }
    
    hal_log(HAL_LOG_INFO, "AUDIO", "Initializing audio subsystem...");
    
    /* Clear device array */
    for (int i = 0; i < MAX_AUDIO_DEVICES; i++) {
        audio_devices[i].active = false;
    }
    
    /* Scan for audio devices via PCI */
    audio_device_count = 0;
    
    /* Look for Intel HDA controllers (class 0x04, subclass 0x03) */
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            for (uint32_t func = 0; func < 8; func++) {
                uint32_t class_info = pci_cfg_read32(bus, slot, func, 0x08);
                uint8_t class_code = (class_info >> 24) & 0xFF;
                uint8_t subclass = (class_info >> 16) & 0xFF;
                
                if (class_code == 0x04 && subclass == 0x03) { /* Audio controller */
                    if (audio_device_count >= MAX_AUDIO_DEVICES) break;
                    
                    uint32_t vendor_device = pci_cfg_read32(bus, slot, func, 0x00);
                    uint16_t vendor_id = vendor_device & 0xFFFF;
                    uint16_t device_id = (vendor_device >> 16) & 0xFFFF;
                    
                    hal_audio_device_t* dev = &audio_devices[audio_device_count];
                    dev->active = true;
                    dev->type = AUDIO_TYPE_HDA;
                    dev->vendor_id = vendor_id;
                    dev->device_id = device_id;
                    dev->sample_rate = 44100; /* Default CD quality */
                    dev->channels = 2; /* Stereo */
                    dev->bit_depth = 16; /* 16-bit samples */
                    
                    /* Set device name based on vendor */
                    switch (vendor_id) {
                        case 0x8086: strcpy(dev->name, "Intel HDA"); break;
                        case 0x1002: strcpy(dev->name, "AMD HDA"); break;
                        case 0x10DE: strcpy(dev->name, "NVIDIA HDA"); break;
                        case 0x1106: strcpy(dev->name, "VIA HDA"); break;
                        default: strcpy(dev->name, "Generic HDA"); break;
                    }
                    
                    /* Map BAR0 for MMIO access */
                    uint32_t bar0 = pci_cfg_read32(bus, slot, func, 0x10);
                    if (bar0 & 0x1) { /* IO space */
                        dev->mmio_base = (void*)(uintptr_t)(bar0 & ~0x3);
                    } else { /* Memory space */
                        dev->mmio_base = (void*)(uintptr_t)(bar0 & ~0xF);
                    }
                    
                    hal_log(HAL_LOG_INFO, "AUDIO", "Found %s at %02x:%02x.%x", 
                           dev->name, bus, slot, func);
                    
                    audio_device_count++;
                }
            }
        }
    }
    
    audio_initialized = true;
    hal_log(HAL_LOG_INFO, "AUDIO", "Audio subsystem initialized with %u devices", audio_device_count);
    return STATUS_OK;
}

/* Get number of audio devices */
uint32_t hal_audio_get_device_count(void) {
    return audio_device_count;
}

/* Get audio device information */
status_t hal_audio_get_info(audio_device_t device, audio_info_t* info) {
    if (device >= audio_device_count || !info || !audio_devices[device].active) {
        return STATUS_ERROR;
    }
    
    hal_audio_device_t* dev = &audio_devices[device];
    
    strcpy(info->name, dev->name);
    info->sample_rate = dev->sample_rate;
    info->channels = dev->channels;
    info->bit_depth = dev->bit_depth;
    info->buffer_size = AUDIO_BUFFER_SIZE;
    info->can_playback = true;
    info->can_record = true;
    
    return STATUS_OK;
}

/* Play audio data */
status_t hal_audio_play(audio_device_t device, const void* data, size_t size) {
    if (device >= audio_device_count || !data || size == 0 || !audio_devices[device].active) {
        return STATUS_ERROR;
    }
    
    hal_audio_device_t* dev = &audio_devices[device];
    
    /* For HDA devices, simplified playback */
    if (dev->type == AUDIO_TYPE_HDA && dev->mmio_base) {
        /* Write audio data to DMA buffer (simplified) */
        /* In real implementation, would set up DMA and ring buffers */
        hal_log(HAL_LOG_DEBUG, "AUDIO", "Playing %zu bytes on %s", size, dev->name);
        
        /* Simulate playback latency */
        uint32_t samples = size / (dev->channels * (dev->bit_depth / 8));
        uint32_t duration_ms = (samples * 1000) / dev->sample_rate;
        
        /* Would actually configure HDA codec and DMA engine here */
        return STATUS_OK;
    }
    
    return STATUS_NOSUPPORT;
}

/* Record audio data */
status_t hal_audio_record(audio_device_t device, void* buffer, size_t size) {
    if (device >= audio_device_count || !buffer || size == 0 || !audio_devices[device].active) {
        return STATUS_ERROR;
    }
    
    hal_audio_device_t* dev = &audio_devices[device];
    
    /* For HDA devices, simplified recording */
    if (dev->type == AUDIO_TYPE_HDA && dev->mmio_base) {
        /* Read audio data from DMA buffer (simplified) */
        /* In real implementation, would read from ADC DMA buffers */
        hal_log(HAL_LOG_DEBUG, "AUDIO", "Recording %zu bytes from %s", size, dev->name);
        
        /* Clear buffer for now (silence) */
        memset(buffer, 0, size);
        
        /* Would actually read from HDA codec input here */
        return STATUS_OK;
    }
    
    return STATUS_NOSUPPORT;
}
