/**
 * LimitlessOS Enterprise Graphics Subsystem
 * Military-grade graphics with hardware acceleration, advanced compositing,
 * and enterprise-level GPU management
 */

#ifndef LIMITLESS_GRAPHICS_ENTERPRISE_H
#define LIMITLESS_GRAPHICS_ENTERPRISE_H

#include <stdint.h>
#include <stdbool.h>
#include "hal.h"
#include "vmm.h"

/* Graphics API Version */
#define LIMITLESS_GRAPHICS_API_VERSION_MAJOR 2
#define LIMITLESS_GRAPHICS_API_VERSION_MINOR 0

/* Maximum limits */
#define MAX_GPU_ADAPTERS            8
#define MAX_DISPLAYS_PER_GPU        16
#define MAX_RENDER_TARGETS          256
#define MAX_TEXTURES                4096
#define MAX_SHADERS                 1024
#define MAX_VERTEX_BUFFERS          2048
#define MAX_COMMAND_BUFFERS         64
#define MAX_FENCE_OBJECTS           512
#define MAX_GPU_MEMORY_POOLS        32

/* GPU Vendor IDs */
#define GPU_VENDOR_NVIDIA           0x10DE
#define GPU_VENDOR_AMD              0x1002
#define GPU_VENDOR_INTEL            0x8086
#define GPU_VENDOR_ARM              0x13B5
#define GPU_VENDOR_QUALCOMM         0x17CB
#define GPU_VENDOR_LIMITLESS        0x1337  /* Our own GPU IP */

/* GPU Architecture Types */
#define GPU_ARCH_UNKNOWN            0
#define GPU_ARCH_NVIDIA_TURING      1
#define GPU_ARCH_NVIDIA_AMPERE      2
#define GPU_ARCH_NVIDIA_ADA         3
#define GPU_ARCH_NVIDIA_HOPPER      4
#define GPU_ARCH_AMD_RDNA2          5
#define GPU_ARCH_AMD_RDNA3          6
#define GPU_ARCH_INTEL_ARC          7
#define GPU_ARCH_ARM_MALI           8
#define GPU_ARCH_LIMITLESS_NEURAL   9   /* Our AI-accelerated GPU */

/* GPU Memory Types */
typedef enum {
    GPU_MEMORY_DEVICE_LOCAL     = 0x01,  /* On-GPU VRAM */
    GPU_MEMORY_HOST_VISIBLE     = 0x02,  /* CPU-accessible */
    GPU_MEMORY_HOST_COHERENT    = 0x04,  /* CPU-GPU coherent */
    GPU_MEMORY_HOST_CACHED      = 0x08,  /* CPU cached */
    GPU_MEMORY_PROTECTED        = 0x10,  /* DRM protected */
    GPU_MEMORY_DEVICE_COHERENT  = 0x20,  /* GPU-coherent */
    GPU_MEMORY_DEVICE_UNCACHED  = 0x40   /* Uncached device memory */
} gpu_memory_property_flags_t;

/* Render Pipeline States */
typedef enum {
    RENDER_STATE_DEPTH_TEST_ENABLE,
    RENDER_STATE_DEPTH_WRITE_ENABLE,
    RENDER_STATE_STENCIL_TEST_ENABLE,
    RENDER_STATE_ALPHA_BLEND_ENABLE,
    RENDER_STATE_CULLING_ENABLE,
    RENDER_STATE_WIREFRAME_ENABLE,
    RENDER_STATE_MULTISAMPLE_ENABLE,
    RENDER_STATE_CONSERVATIVE_RASTER
} render_state_t;

/* Texture Formats */
typedef enum {
    TEXTURE_FORMAT_R8_UNORM,
    TEXTURE_FORMAT_R8G8_UNORM,
    TEXTURE_FORMAT_R8G8B8_UNORM,
    TEXTURE_FORMAT_R8G8B8A8_UNORM,
    TEXTURE_FORMAT_R8G8B8A8_SRGB,
    TEXTURE_FORMAT_R16_FLOAT,
    TEXTURE_FORMAT_R16G16_FLOAT,
    TEXTURE_FORMAT_R16G16B16A16_FLOAT,
    TEXTURE_FORMAT_R32_FLOAT,
    TEXTURE_FORMAT_R32G32_FLOAT,
    TEXTURE_FORMAT_R32G32B32_FLOAT,
    TEXTURE_FORMAT_R32G32B32A32_FLOAT,
    TEXTURE_FORMAT_BC1_RGB_UNORM,       /* S3TC/DXT1 */
    TEXTURE_FORMAT_BC1_RGBA_UNORM,
    TEXTURE_FORMAT_BC3_RGBA_UNORM,      /* S3TC/DXT5 */
    TEXTURE_FORMAT_BC7_RGBA_UNORM,      /* BPTC */
    TEXTURE_FORMAT_ASTC_4x4_UNORM,      /* ARM ASTC */
    TEXTURE_FORMAT_ETC2_RGB8_UNORM,     /* Ericsson ETC2 */
    TEXTURE_FORMAT_D16_UNORM,           /* Depth formats */
    TEXTURE_FORMAT_D24_UNORM_S8_UINT,
    TEXTURE_FORMAT_D32_FLOAT,
    TEXTURE_FORMAT_D32_FLOAT_S8X24_UINT
} texture_format_t;

/* Shader Types */
typedef enum {
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_TESSELLATION_CONTROL,
    SHADER_TYPE_TESSELLATION_EVALUATION,
    SHADER_TYPE_GEOMETRY,
    SHADER_TYPE_FRAGMENT,
    SHADER_TYPE_COMPUTE,
    SHADER_TYPE_MESH,               /* Nvidia mesh shaders */
    SHADER_TYPE_TASK,               /* Nvidia task shaders */
    SHADER_TYPE_RAY_GEN,            /* Ray tracing shaders */
    SHADER_TYPE_RAY_MISS,
    SHADER_TYPE_RAY_CLOSEST_HIT,
    SHADER_TYPE_RAY_ANY_HIT,
    SHADER_TYPE_RAY_INTERSECTION,
    SHADER_TYPE_CALLABLE
} shader_type_t;

/* Command Buffer Types */
typedef enum {
    COMMAND_BUFFER_PRIMARY,
    COMMAND_BUFFER_SECONDARY,
    COMMAND_BUFFER_COMPUTE,
    COMMAND_BUFFER_COPY,
    COMMAND_BUFFER_PRESENT,
    COMMAND_BUFFER_RAY_TRACING
} command_buffer_type_t;

/* GPU Memory Allocation */
typedef struct gpu_memory_allocation {
    uint64_t            handle;         /* Allocation handle */
    uint64_t            size;           /* Allocation size */
    uint64_t            offset;         /* Offset in memory pool */
    uint32_t            memory_type;    /* Memory type index */
    uint32_t            properties;     /* Memory properties */
    void*               mapped_ptr;     /* CPU mapping (if applicable) */
    bool                dedicated;      /* Dedicated allocation */
} gpu_memory_allocation_t;

/* GPU Buffer */
typedef struct gpu_buffer {
    uint64_t            handle;         /* Buffer handle */
    uint64_t            size;           /* Buffer size */
    uint32_t            usage;          /* Usage flags */
    uint32_t            sharing_mode;   /* Sharing mode */
    gpu_memory_allocation_t* memory;    /* Memory allocation */
    void*               mapped_data;    /* Mapped CPU data */
} gpu_buffer_t;

/* GPU Image/Texture */
typedef struct gpu_image {
    uint64_t            handle;         /* Image handle */
    uint32_t            width;          /* Width in pixels */
    uint32_t            height;         /* Height in pixels */
    uint32_t            depth;          /* Depth (for 3D textures) */
    uint32_t            mip_levels;     /* Number of mip levels */
    uint32_t            array_layers;   /* Array layers */
    texture_format_t    format;         /* Pixel format */
    uint32_t            usage;          /* Usage flags */
    uint32_t            samples;        /* MSAA sample count */
    gpu_memory_allocation_t* memory;    /* Memory allocation */
} gpu_image_t;

/* Render Pass */
typedef struct render_pass_attachment {
    texture_format_t    format;         /* Attachment format */
    uint32_t            samples;        /* Sample count */
    uint32_t            load_op;        /* Load operation */
    uint32_t            store_op;       /* Store operation */
    uint32_t            stencil_load_op; /* Stencil load op */
    uint32_t            stencil_store_op; /* Stencil store op */
    uint32_t            initial_layout; /* Initial layout */
    uint32_t            final_layout;   /* Final layout */
} render_pass_attachment_t;

typedef struct render_pass {
    uint64_t            handle;         /* Render pass handle */
    uint32_t            attachment_count; /* Number of attachments */
    render_pass_attachment_t* attachments; /* Attachments */
    uint32_t            subpass_count;  /* Number of subpasses */
    void*               subpasses;      /* Subpass descriptions */
} render_pass_t;

/* Framebuffer */
typedef struct gpu_framebuffer {
    uint64_t            handle;         /* Framebuffer handle */
    render_pass_t*      render_pass;    /* Associated render pass */
    uint32_t            attachment_count; /* Number of attachments */
    gpu_image_t**       attachments;    /* Image attachments */
    uint32_t            width;          /* Framebuffer width */
    uint32_t            height;         /* Framebuffer height */
    uint32_t            layers;         /* Number of layers */
} gpu_framebuffer_t;

/* Shader Module */
typedef struct shader_module {
    uint64_t            handle;         /* Shader handle */
    shader_type_t       type;           /* Shader type */
    size_t              code_size;      /* Code size in bytes */
    void*               code;           /* Shader bytecode */
    char                entry_point[64]; /* Entry point name */
} shader_module_t;

/* Graphics Pipeline */
typedef struct graphics_pipeline {
    uint64_t            handle;         /* Pipeline handle */
    shader_module_t*    vertex_shader;
    shader_module_t*    fragment_shader;
    shader_module_t*    geometry_shader;
    shader_module_t*    tess_control_shader;
    shader_module_t*    tess_eval_shader;
    render_pass_t*      render_pass;
    uint32_t            subpass;
    /* Vertex input state */
    /* Input assembly state */
    /* Viewport state */
    /* Rasterization state */
    /* Multisample state */
    /* Depth/stencil state */
    /* Color blend state */
    /* Dynamic state */
} graphics_pipeline_t;

/* Command Buffer */
typedef struct gpu_command_buffer {
    uint64_t            handle;         /* Command buffer handle */
    command_buffer_type_t type;         /* Command buffer type */
    uint32_t            level;          /* Primary/Secondary */
    bool                recording;      /* Currently recording */
    uint32_t            command_count;  /* Number of commands */
} gpu_command_buffer_t;

/* Fence Object */
typedef struct gpu_fence {
    uint64_t            handle;         /* Fence handle */
    bool                signaled;       /* Fence state */
    uint64_t            timeline_value; /* Timeline value */
} gpu_fence_t;

/* Semaphore */
typedef struct gpu_semaphore {
    uint64_t            handle;         /* Semaphore handle */
    uint32_t            type;           /* Binary/Timeline */
    uint64_t            value;          /* Current value */
} gpu_semaphore_t;

/* GPU Queue */
typedef struct gpu_queue {
    uint64_t            handle;         /* Queue handle */
    uint32_t            family_index;   /* Queue family index */
    uint32_t            queue_index;    /* Queue index in family */
    uint32_t            flags;          /* Queue capabilities */
    float               priority;       /* Queue priority */
} gpu_queue_t;

/* Display Mode */
typedef struct display_mode_info {
    uint32_t            width;          /* Display width */
    uint32_t            height;         /* Display height */
    uint32_t            refresh_rate;   /* Refresh rate (mHz) */
    uint32_t            bit_depth;      /* Bits per pixel */
    texture_format_t    format;         /* Pixel format */
    uint32_t            flags;          /* Mode flags */
} display_mode_info_t;

/* Display Output */
typedef struct display_output {
    uint32_t            id;             /* Display ID */
    char                name[128];      /* Display name */
    char                manufacturer[64]; /* Manufacturer */
    char                model[64];      /* Model */
    uint32_t            interface_type; /* HDMI, DP, etc. */
    bool                connected;      /* Connection status */
    bool                primary;        /* Primary display */
    
    /* Physical properties */
    uint32_t            width_mm;       /* Physical width (mm) */
    uint32_t            height_mm;      /* Physical height (mm) */
    uint32_t            dpi_x;          /* Horizontal DPI */
    uint32_t            dpi_y;          /* Vertical DPI */
    
    /* Current mode */
    display_mode_info_t current_mode;   /* Current display mode */
    
    /* Supported modes */
    uint32_t            mode_count;     /* Number of supported modes */
    display_mode_info_t* supported_modes; /* Supported display modes */
    
    /* Capabilities */
    bool                supports_hdr;   /* HDR support */
    bool                supports_vrr;   /* Variable refresh rate */
    bool                supports_hdcp;  /* HDCP support */
    uint32_t            max_luminance;  /* Max luminance (nits) */
    uint32_t            min_luminance;  /* Min luminance (0.0001 nits) */
    
    /* Color space */
    float               color_primaries[8]; /* RGB + white point */
    float               gamma;          /* Display gamma */
    
    struct display_output* next;        /* Next in list */
} display_output_t;

/* GPU Adapter */
typedef struct gpu_adapter {
    uint32_t            id;             /* Adapter ID */
    uint32_t            vendor_id;      /* PCI vendor ID */
    uint32_t            device_id;      /* PCI device ID */
    uint32_t            subsystem_id;   /* PCI subsystem ID */
    uint32_t            revision;       /* Revision ID */
    uint32_t            architecture;   /* GPU architecture type */
    
    char                name[128];      /* GPU name */
    char                driver_version[64]; /* Driver version */
    
    /* Memory information */
    uint64_t            total_memory;   /* Total GPU memory */
    uint64_t            available_memory; /* Available memory */
    uint32_t            memory_type_count; /* Memory type count */
    gpu_memory_allocation_t* memory_pools; /* Memory pools */
    
    /* Capabilities */
    struct {
        bool            unified_memory;     /* Unified memory architecture */
        bool            discrete_gpu;       /* Discrete GPU */
        bool            ray_tracing;        /* Ray tracing support */
        bool            mesh_shaders;       /* Mesh shader support */
        bool            variable_rate_shading; /* VRS support */
        bool            int8_support;       /* INT8 precision */
        bool            int16_support;      /* INT16 precision */
        bool            fp16_support;       /* FP16 precision */
        bool            fp64_support;       /* FP64 precision */
        bool            atomic_int64;       /* 64-bit atomics */
        bool            multiview;          /* Multiview rendering */
        bool            geometry_shader;    /* Geometry shader support */
        bool            tessellation;       /* Tessellation support */
        bool            compute_shader;     /* Compute shader support */
        bool            sparse_binding;     /* Sparse resource binding */
        bool            sparse_residency;   /* Sparse residency */
        bool            protected_memory;   /* Protected memory */
    } capabilities;
    
    /* Limits */
    struct {
        uint32_t        max_texture_size_1d;
        uint32_t        max_texture_size_2d;
        uint32_t        max_texture_size_3d;
        uint32_t        max_texture_array_layers;
        uint32_t        max_framebuffer_width;
        uint32_t        max_framebuffer_height;
        uint32_t        max_framebuffer_layers;
        uint32_t        max_color_attachments;
        uint32_t        max_vertex_input_bindings;
        uint32_t        max_vertex_input_attributes;
        uint32_t        max_compute_workgroup_size[3];
        uint32_t        max_compute_workgroup_invocations;
    } limits;
    
    /* Queues */
    uint32_t            queue_family_count; /* Queue family count */
    struct {
        uint32_t        queue_count;        /* Queues in family */
        uint32_t        queue_flags;        /* Queue capabilities */
        uint32_t        timestamp_valid_bits; /* Timestamp precision */
        uint32_t        min_image_transfer_granularity[3];
    }* queue_families;
    
    /* Connected displays */
    uint32_t            display_count;  /* Number of displays */
    display_output_t*   displays;       /* Connected displays */
    
    /* Performance counters */
    uint64_t            frames_rendered;
    uint64_t            triangles_processed;
    uint64_t            pixels_shaded;
    uint32_t            temperature;    /* GPU temperature (°C) */
    uint32_t            power_usage;    /* Power usage (watts) */
    uint32_t            clock_core;     /* Core clock (MHz) */
    uint32_t            clock_memory;   /* Memory clock (MHz) */
    uint32_t            utilization;    /* GPU utilization (%) */
    
    struct gpu_adapter* next;           /* Next adapter */
} gpu_adapter_t;

/* Graphics Context */
typedef struct graphics_context {
    gpu_adapter_t*      adapter;        /* Associated GPU adapter */
    uint64_t            handle;         /* Context handle */
    
    /* Command submission */
    gpu_queue_t*        graphics_queue; /* Graphics queue */
    gpu_queue_t*        compute_queue;  /* Compute queue */
    gpu_queue_t*        transfer_queue; /* Transfer queue */
    gpu_queue_t*        present_queue;  /* Present queue */
    
    /* Swapchain */
    struct {
        uint64_t        handle;         /* Swapchain handle */
        uint32_t        image_count;    /* Number of images */
        gpu_image_t**   images;         /* Swapchain images */
        texture_format_t format;        /* Image format */
        uint32_t        width;          /* Swapchain width */
        uint32_t        height;         /* Swapchain height */
        uint32_t        current_image;  /* Current image index */
        bool            vsync_enabled;  /* VSync enabled */
    } swapchain;
    
    /* Resource pools */
    gpu_buffer_t*       buffers[MAX_VERTEX_BUFFERS];
    gpu_image_t*        images[MAX_TEXTURES];
    shader_module_t*    shaders[MAX_SHADERS];
    render_pass_t*      render_passes[256];
    graphics_pipeline_t* pipelines[512];
    gpu_command_buffer_t* command_buffers[MAX_COMMAND_BUFFERS];
    
    /* Synchronization */
    gpu_fence_t*        fences[MAX_FENCE_OBJECTS];
    gpu_semaphore_t*    semaphores[256];
    
} graphics_context_t;

/* Function Pointers for GPU Operations */
typedef struct gpu_interface {
    /* Context management */
    status_t (*create_context)(gpu_adapter_t* adapter, graphics_context_t** context);
    void (*destroy_context)(graphics_context_t* context);
    
    /* Memory management */
    status_t (*allocate_memory)(graphics_context_t* ctx, uint64_t size, uint32_t memory_type, gpu_memory_allocation_t** allocation);
    void (*free_memory)(graphics_context_t* ctx, gpu_memory_allocation_t* allocation);
    status_t (*map_memory)(graphics_context_t* ctx, gpu_memory_allocation_t* allocation, void** ptr);
    void (*unmap_memory)(graphics_context_t* ctx, gpu_memory_allocation_t* allocation);
    
    /* Buffer management */
    status_t (*create_buffer)(graphics_context_t* ctx, uint64_t size, uint32_t usage, gpu_buffer_t** buffer);
    void (*destroy_buffer)(graphics_context_t* ctx, gpu_buffer_t* buffer);
    
    /* Image/Texture management */
    status_t (*create_image)(graphics_context_t* ctx, uint32_t width, uint32_t height, uint32_t depth, uint32_t mip_levels, texture_format_t format, uint32_t usage, gpu_image_t** image);
    void (*destroy_image)(graphics_context_t* ctx, gpu_image_t* image);
    
    /* Shader management */
    status_t (*create_shader)(graphics_context_t* ctx, shader_type_t type, const void* code, size_t code_size, const char* entry_point, shader_module_t** shader);
    void (*destroy_shader)(graphics_context_t* ctx, shader_module_t* shader);
    
    /* Pipeline management */
    status_t (*create_graphics_pipeline)(graphics_context_t* ctx, const void* pipeline_desc, graphics_pipeline_t** pipeline);
    void (*destroy_graphics_pipeline)(graphics_context_t* ctx, graphics_pipeline_t* pipeline);
    
    /* Command buffer management */
    status_t (*create_command_buffer)(graphics_context_t* ctx, command_buffer_type_t type, gpu_command_buffer_t** cmd_buffer);
    void (*destroy_command_buffer)(graphics_context_t* ctx, gpu_command_buffer_t* cmd_buffer);
    status_t (*begin_command_buffer)(graphics_context_t* ctx, gpu_command_buffer_t* cmd_buffer);
    status_t (*end_command_buffer)(graphics_context_t* ctx, gpu_command_buffer_t* cmd_buffer);
    
    /* Rendering commands */
    void (*cmd_begin_render_pass)(gpu_command_buffer_t* cmd_buffer, render_pass_t* render_pass, gpu_framebuffer_t* framebuffer);
    void (*cmd_end_render_pass)(gpu_command_buffer_t* cmd_buffer);
    void (*cmd_bind_pipeline)(gpu_command_buffer_t* cmd_buffer, graphics_pipeline_t* pipeline);
    void (*cmd_bind_vertex_buffer)(gpu_command_buffer_t* cmd_buffer, uint32_t binding, gpu_buffer_t* buffer, uint64_t offset);
    void (*cmd_bind_index_buffer)(gpu_command_buffer_t* cmd_buffer, gpu_buffer_t* buffer, uint64_t offset, uint32_t index_type);
    void (*cmd_draw)(gpu_command_buffer_t* cmd_buffer, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
    void (*cmd_draw_indexed)(gpu_command_buffer_t* cmd_buffer, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);
    
    /* Command submission */
    status_t (*submit_command_buffer)(graphics_context_t* ctx, gpu_queue_t* queue, gpu_command_buffer_t* cmd_buffer, gpu_fence_t* fence);
    status_t (*queue_present)(graphics_context_t* ctx, gpu_queue_t* queue, uint32_t image_index);
    
    /* Synchronization */
    status_t (*create_fence)(graphics_context_t* ctx, bool signaled, gpu_fence_t** fence);
    void (*destroy_fence)(graphics_context_t* ctx, gpu_fence_t* fence);
    status_t (*wait_for_fence)(graphics_context_t* ctx, gpu_fence_t* fence, uint64_t timeout);
    status_t (*reset_fence)(graphics_context_t* ctx, gpu_fence_t* fence);
    
    /* Debug and profiling */
    void (*debug_marker_begin)(gpu_command_buffer_t* cmd_buffer, const char* name, float color[4]);
    void (*debug_marker_end)(gpu_command_buffer_t* cmd_buffer);
    void (*debug_marker_insert)(gpu_command_buffer_t* cmd_buffer, const char* name, float color[4]);
    
} gpu_interface_t;

/* Graphics subsystem global state */
extern struct graphics_enterprise_subsystem {
    bool                initialized;
    uint32_t            api_version;
    
    /* GPU adapters */
    uint32_t            adapter_count;
    gpu_adapter_t*      adapters;
    gpu_adapter_t*      primary_adapter;
    
    /* Display outputs */
    uint32_t            display_count;
    display_output_t*   displays;
    display_output_t*   primary_display;
    
    /* Graphics contexts */
    uint32_t            context_count;
    graphics_context_t* contexts[16];
    graphics_context_t* primary_context;
    
    /* GPU interfaces */
    gpu_interface_t*    gpu_interface;
    
    /* Performance statistics */
    struct {
        uint64_t        frames_rendered;
        uint64_t        draw_calls;
        uint64_t        triangles_processed;
        uint64_t        pixels_shaded;
        uint64_t        memory_allocated;
        uint64_t        memory_used;
        uint32_t        active_contexts;
        uint32_t        active_pipelines;
        uint32_t        active_textures;
    } stats;
    
} graphics_enterprise;

/* Core Graphics API */
status_t graphics_enterprise_init(void);
void graphics_enterprise_shutdown(void);

/* GPU Adapter Management */
status_t graphics_enumerate_adapters(void);
gpu_adapter_t* graphics_get_primary_adapter(void);
gpu_adapter_t* graphics_get_adapter_by_id(uint32_t id);
status_t graphics_set_primary_adapter(gpu_adapter_t* adapter);

/* Display Management */
status_t graphics_enumerate_displays(gpu_adapter_t* adapter);
display_output_t* graphics_get_primary_display(void);
display_output_t* graphics_get_display_by_id(uint32_t id);
status_t graphics_set_display_mode(display_output_t* display, display_mode_info_t* mode);
status_t graphics_enable_display(display_output_t* display, bool enable);

/* Graphics Context Management */
status_t graphics_create_context(gpu_adapter_t* adapter, graphics_context_t** context);
void graphics_destroy_context(graphics_context_t* context);
status_t graphics_make_context_current(graphics_context_t* context);
graphics_context_t* graphics_get_current_context(void);

/* Swapchain Management */
status_t graphics_create_swapchain(graphics_context_t* ctx, display_output_t* display, uint32_t width, uint32_t height, texture_format_t format, bool vsync);
status_t graphics_destroy_swapchain(graphics_context_t* ctx);
status_t graphics_acquire_next_image(graphics_context_t* ctx, uint32_t* image_index);
status_t graphics_present_image(graphics_context_t* ctx, uint32_t image_index);

/* Debug and Profiling */
void graphics_print_adapter_info(gpu_adapter_t* adapter);
void graphics_print_display_info(display_output_t* display);
void graphics_print_performance_stats(void);
status_t graphics_enable_debug_layer(bool enable);
status_t graphics_capture_frame(const char* filename);

/* Power Management */
status_t graphics_set_power_state(gpu_adapter_t* adapter, uint32_t power_state);
uint32_t graphics_get_power_usage(gpu_adapter_t* adapter);
status_t graphics_enable_power_management(gpu_adapter_t* adapter, bool enable);

#endif /* LIMITLESS_GRAPHICS_ENTERPRISE_H */