/**
 * GPU Driver Framework Header
 * Complete GPU driver interface for modern graphics hardware
 */

#pragma once
#include "kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/* GPU Vendor IDs */
#define GPU_VENDOR_INTEL    0x8086
#define GPU_VENDOR_AMD      0x1002
#define GPU_VENDOR_NVIDIA   0x10DE
#define GPU_VENDOR_VMWARE   0x15AD
#define GPU_VENDOR_QEMU     0x1234

/* GPU Types */
typedef enum {
    GPU_TYPE_INTEGRATED,    // Integrated GPU (Intel HD, AMD APU)
    GPU_TYPE_DISCRETE,      // Discrete GPU (dedicated card)
    GPU_TYPE_VIRTUAL,       // Virtual GPU (QEMU, VirtualBox)
    GPU_TYPE_COMPUTE        // Compute-only GPU (no display)
} gpu_type_t;

/* GPU Capabilities */
typedef enum {
    GPU_CAP_2D_ACCEL     = (1 << 0),   // 2D acceleration
    GPU_CAP_3D_ACCEL     = (1 << 1),   // 3D acceleration
    GPU_CAP_COMPUTE      = (1 << 2),   // Compute shaders
    GPU_CAP_VIDEO_DECODE = (1 << 3),   // Video decoding
    GPU_CAP_VIDEO_ENCODE = (1 << 4),   // Video encoding
    GPU_CAP_MULTIHEAD    = (1 << 5),   // Multi-monitor
    GPU_CAP_HOTPLUG      = (1 << 6),   // Hot-plug detection
    GPU_CAP_POWER_MGMT   = (1 << 7),   // Power management
    GPU_CAP_DISPLAYPORT  = (1 << 8),   // DisplayPort support
    GPU_CAP_HDMI         = (1 << 9),   // HDMI support
    GPU_CAP_VGA          = (1 << 10),  // VGA compatibility
} gpu_capabilities_t;

/* GPU Memory Types */
typedef enum {
    GPU_MEM_SYSTEM = 0,     // System RAM (shared)
    GPU_MEM_LOCAL,          // Local VRAM
    GPU_MEM_AGP,            // AGP memory
    GPU_MEM_UNIFIED         // Unified memory architecture
} gpu_memory_type_t;

/* Display Mode */
typedef struct gpu_mode {
    uint32_t width;
    uint32_t height;
    uint32_t refresh_rate;      // Hz
    uint32_t bits_per_pixel;
    uint32_t stride;            // Bytes per scanline
    uint32_t pixel_format;      // Pixel format (RGB, BGR, etc.)
    uint32_t flags;             // Mode flags
} gpu_mode_t;

/* GPU Memory Object */
typedef struct gpu_memory {
    uint64_t handle;            // Memory handle
    uint64_t size;              // Size in bytes
    uint64_t alignment;         // Required alignment
    gpu_memory_type_t type;     // Memory type
    paddr_t physical_address;   // Physical address
    vaddr_t virtual_address;    // Mapped virtual address
    uint32_t flags;             // Memory flags
    bool mapped;                // Is mapped to CPU
    bool coherent;              // Cache coherent
} gpu_memory_t;

/* Framebuffer Info */
typedef struct gpu_framebuffer {
    gpu_memory_t memory;        // Framebuffer memory
    gpu_mode_t mode;            // Current mode
    uint32_t buffer_count;      // Number of buffers (double/triple buffering)
    uint32_t current_buffer;    // Current front buffer
    bool vsync_enabled;         // VSync enabled
} gpu_framebuffer_t;

/* GPU Performance Counters */
typedef struct gpu_performance {
    uint64_t frames_rendered;   // Total frames rendered
    uint64_t triangles_rendered; // Total triangles
    uint64_t memory_bandwidth;  // Memory bandwidth usage
    uint32_t gpu_utilization;   // GPU utilization percentage
    uint32_t memory_utilization; // VRAM utilization percentage
    uint32_t temperature;       // GPU temperature (°C)
    uint32_t power_consumption; // Power consumption (watts)
} gpu_performance_t;

/* Compute Context */
typedef struct gpu_compute_context {
    uint64_t context_id;        // Context ID
    void* command_buffer;       // Command buffer
    uint32_t buffer_size;       // Buffer size
    bool active;                // Is context active
} gpu_compute_context_t;

/* Forward declaration */
struct gpu_device;

/* GPU Device */
typedef struct gpu_device {
    uint32_t id;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t revision_id;
    gpu_type_t type;
    char name[64];
    char driver_name[32];
    
    /* Capabilities */
    gpu_capabilities_t capabilities;
    
    /* Framebuffer */
    gpu_framebuffer_t framebuffer;
    
    /* Supported modes */
    gpu_mode_t* modes;
    uint32_t mode_count;
    gpu_mode_t* preferred_mode;
    
    /* Current state */
    bool enabled;
    bool initialized;
    gpu_mode_t current_mode;
    
    /* Hardware info */
    uint64_t vram_size;         // VRAM size in bytes
    uint64_t vram_used;         // VRAM currently used
    uint32_t pci_bus;
    uint32_t pci_device;
    uint32_t pci_function;
    
    /* Memory management */
    gpu_memory_t* memory_objects;
    uint32_t memory_object_count;
    uint32_t max_memory_objects;
    
    /* Performance monitoring */
    gpu_performance_t performance;
    bool performance_monitoring;
    
    /* Compute contexts */
    gpu_compute_context_t* compute_contexts;
    uint32_t context_count;
    uint32_t max_contexts;
    
    /* Hardware registers */
    vaddr_t mmio_base;          // Memory-mapped I/O base
    uint32_t mmio_size;         // MMIO size
    uint32_t irq_line;          // IRQ line
    
    /* Power management */
    uint32_t power_state;       // Current power state
    bool power_management;      // PM enabled
    
    /* Driver callbacks */
    status_t (*init)(struct gpu_device*);
    status_t (*shutdown)(struct gpu_device*);
    status_t (*suspend)(struct gpu_device*);
    status_t (*resume)(struct gpu_device*);
    
    /* Display operations */
    status_t (*set_mode)(struct gpu_device*, gpu_mode_t*);
    status_t (*get_modes)(struct gpu_device*, gpu_mode_t**, uint32_t*);
    status_t (*enable_output)(struct gpu_device*, bool enable);
    
    /* Memory operations */
    status_t (*alloc_memory)(struct gpu_device*, gpu_memory_t*);
    status_t (*free_memory)(struct gpu_device*, gpu_memory_t*);
    status_t (*map_memory)(struct gpu_device*, gpu_memory_t*);
    status_t (*unmap_memory)(struct gpu_device*, gpu_memory_t*);
    
    /* Rendering operations */
    status_t (*blit)(struct gpu_device*, void* src, uint32_t x, uint32_t y,
                     uint32_t w, uint32_t h);
    status_t (*fill)(struct gpu_device*, uint32_t color, uint32_t x, uint32_t y,
                     uint32_t w, uint32_t h);
    status_t (*copy_buffer)(struct gpu_device*, gpu_memory_t* src, gpu_memory_t* dst);
    
    /* 3D operations */
    status_t (*create_context)(struct gpu_device*, gpu_compute_context_t**);
    status_t (*destroy_context)(struct gpu_device*, gpu_compute_context_t*);
    status_t (*submit_commands)(struct gpu_device*, gpu_compute_context_t*, void* commands, uint32_t size);
    
    /* Interrupt handler */
    void (*irq_handler)(struct gpu_device*);
    
    /* Lock for thread safety */
    spinlock_t lock;
} gpu_device_t;

/* Core GPU API */
status_t gpu_init(void);
status_t gpu_register_device(gpu_device_t* device);

/* Device management */
gpu_device_t* gpu_get_primary(void);
gpu_device_t* gpu_get_device(uint32_t index);
uint32_t gpu_get_count(void);

/* Display operations */
status_t gpu_set_mode(gpu_device_t* device, uint32_t width, uint32_t height, uint32_t refresh_rate);
status_t gpu_get_modes(gpu_device_t* device, gpu_mode_t** modes, uint32_t* count);
status_t gpu_enable_output(gpu_device_t* device, bool enable);

/* Memory management */
status_t gpu_alloc_memory(gpu_device_t* device, uint64_t size, gpu_memory_type_t type, gpu_memory_t** memory);
status_t gpu_free_memory(gpu_device_t* device, gpu_memory_t* memory);

/* Rendering operations */
status_t gpu_blit(gpu_device_t* device, void* src, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
status_t gpu_fill(gpu_device_t* device, uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

/* Performance monitoring */
status_t gpu_get_performance(gpu_device_t* device, gpu_performance_t* perf);

/* Compute operations */
status_t gpu_create_compute_context(gpu_device_t* device, gpu_compute_context_t** context);

/* Vendor-specific driver initialization */
status_t generic_gpu_init(gpu_device_t* device);
status_t intel_gpu_init(gpu_device_t* device);
status_t amd_gpu_init(gpu_device_t* device);
status_t nvidia_gpu_init(gpu_device_t* device);

/* Vendor-specific callbacks (declared but implemented as stubs) */
status_t intel_gpu_set_mode(gpu_device_t* device, gpu_mode_t* mode);
status_t intel_gpu_blit(gpu_device_t* device, void* src, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
status_t intel_gpu_fill(gpu_device_t* device, uint32_t color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
status_t intel_gpu_alloc_memory(gpu_device_t* device, gpu_memory_t* memory);
status_t intel_gpu_free_memory(gpu_device_t* device, gpu_memory_t* memory);
status_t intel_gpu_create_context(gpu_device_t* device, gpu_compute_context_t** context);
status_t intel_gpu_destroy_context(gpu_device_t* device, gpu_compute_context_t* context);
status_t intel_gpu_submit_commands(gpu_device_t* device, gpu_compute_context_t* context, void* commands, uint32_t size);

status_t amd_gpu_set_mode(gpu_device_t* device, gpu_mode_t* mode);
status_t amd_gpu_blit(gpu_device_t* device, void* src, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
status_t amd_gpu_fill(gpu_device_t* device, uint32_t color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
status_t amd_gpu_alloc_memory(gpu_device_t* device, gpu_memory_t* memory);
status_t amd_gpu_free_memory(gpu_device_t* device, gpu_memory_t* memory);
status_t amd_gpu_create_context(gpu_device_t* device, gpu_compute_context_t** context);
status_t amd_gpu_destroy_context(gpu_device_t* device, gpu_compute_context_t* context);
status_t amd_gpu_submit_commands(gpu_device_t* device, gpu_compute_context_t* context, void* commands, uint32_t size);

status_t nvidia_gpu_set_mode(gpu_device_t* device, gpu_mode_t* mode);
status_t nvidia_gpu_blit(gpu_device_t* device, void* src, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
status_t nvidia_gpu_fill(gpu_device_t* device, uint32_t color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
status_t nvidia_gpu_alloc_memory(gpu_device_t* device, gpu_memory_t* memory);
status_t nvidia_gpu_free_memory(gpu_device_t* device, gpu_memory_t* memory);
status_t nvidia_gpu_create_context(gpu_device_t* device, gpu_compute_context_t** context);
status_t nvidia_gpu_destroy_context(gpu_device_t* device, gpu_compute_context_t* context);
status_t nvidia_gpu_submit_commands(gpu_device_t* device, gpu_compute_context_t* context, void* commands, uint32_t size);

#ifdef __cplusplus
}
#endif