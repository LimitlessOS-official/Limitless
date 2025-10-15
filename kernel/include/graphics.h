/**
 * LimitlessOS Enterprise Graphics and GPU Subsystem
 * Complete hardware-accelerated graphics framework with multi-vendor support
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "status.h"
#include "../include/list.h"
#include "../include/mutex.h"
#include "../include/workqueue.h"

// Enterprise graphics system calls
#define __NR_gpu_create_context     410
#define __NR_gpu_destroy_context    411
#define __NR_gpu_allocate_memory    412
#define __NR_gpu_free_memory        413
#define __NR_gpu_map_memory         414
#define __NR_gpu_unmap_memory       415
#define __NR_gpu_submit_commands    416
#define __NR_gpu_wait_completion    417
#define __NR_gpu_compile_shader     418
#define __NR_gpu_create_program     419
#define __NR_gpu_get_info           420
#define __NR_gpu_set_display_mode   421
#define __NR_gpu_get_display_info   422

// GPU vendor identification
#define GPU_VENDOR_NVIDIA       0x10DE
#define GPU_VENDOR_AMD          0x1002
#define GPU_VENDOR_INTEL        0x8086
#define GPU_VENDOR_ARM          0x13B5
#define GPU_VENDOR_QUALCOMM     0x17CB

// Graphics API context types
#define GRAPHICS_CONTEXT_OPENGL     0
#define GRAPHICS_CONTEXT_VULKAN     1
#define GRAPHICS_CONTEXT_DIRECTX11  2
#define GRAPHICS_CONTEXT_DIRECTX12  3
#define GRAPHICS_CONTEXT_COMPUTE    4
#define GRAPHICS_CONTEXT_VIDEO      5

// Memory allocation types and flags
#define GPU_MEMORY_VRAM             0
#define GPU_MEMORY_GTT              1
#define GPU_MEMORY_GART             2
#define GPU_MEMORY_SYSTEM           3
#define GPU_MEMORY_UNIFIED          4

#define GPU_MEMORY_FLAG_READ_ONLY       0x01
#define GPU_MEMORY_FLAG_WRITE_ONLY      0x02
#define GPU_MEMORY_FLAG_READ_WRITE      0x03
#define GPU_MEMORY_FLAG_COHERENT        0x04
#define GPU_MEMORY_FLAG_CACHED          0x08
#define GPU_MEMORY_FLAG_PERSISTENT      0x10
#define GPU_MEMORY_FLAG_MAP_COHERENT    0x20

// Enterprise GPU information structure
typedef struct {
    uint32_t device_id;             // Device ID
    uint32_t vendor_id;             // Vendor ID (NVIDIA/AMD/Intel)
    uint32_t device_pci_id;         // PCI device ID
    uint32_t architecture;          // GPU architecture
    char device_name[64];           // Device name
    char driver_name[32];           // Driver name
    
    // Memory information
    uint64_t vram_size;             // VRAM size (bytes)
    uint64_t vram_used;             // VRAM used (bytes)
    uint64_t vram_available;        // VRAM available (bytes)
    uint32_t memory_bandwidth_gbps; // Memory bandwidth (GB/s)
    
    // Performance information
    uint32_t base_clock_mhz;        // Base clock frequency (MHz)
    uint32_t boost_clock_mhz;       // Boost clock frequency (MHz)
    uint32_t memory_clock_mhz;      // Memory clock frequency (MHz)
    uint32_t shader_units;          // Number of shader units
    uint32_t compute_units;         // Number of compute units
    float theoretical_gflops;       // Theoretical GFLOPS
    
    // Thermal and power
    uint32_t current_power_watts;   // Current power consumption (W)
    uint32_t max_power_watts;       // Maximum power consumption (W)
    uint32_t current_temperature;   // Current temperature (°C)
    uint32_t max_temperature;       // Maximum safe temperature (°C)
    
    // Display information
    uint32_t connector_count;       // Number of display connectors
    uint32_t max_displays;          // Maximum simultaneous displays
    
    // Enterprise capability flags
    bool ray_tracing_support;       // Ray tracing support
    bool mesh_shader_support;       // Mesh shader support
    bool variable_rate_shading;     // Variable rate shading support
    bool compute_shader_support;    // Compute shader support
    bool tessellation_support;      // Tessellation support
    bool geometry_shader_support;   // Geometry shader support
    bool fp16_support;              // 16-bit floating point support
    bool fp64_support;              // 64-bit floating point support
    bool async_compute_support;     // Asynchronous compute support
    bool bindless_resources;        // Bindless resource support
    
    // API support
    bool opengl_support;            // OpenGL support
    bool vulkan_support;            // Vulkan support
    bool directx_support;           // DirectX support
    bool opencl_support;            // OpenCL support
    bool cuda_support;              // CUDA support (NVIDIA)
    bool rocm_support;              // ROCm support (AMD)
    
    uint32_t opengl_version_major;  // OpenGL version
    uint32_t opengl_version_minor;
    uint32_t vulkan_version_major;  // Vulkan version
    uint32_t vulkan_version_minor;
    uint32_t vulkan_patch_version;
} gpu_info_t;

// Enterprise graphics subsystem API
int graphics_subsystem_init(void);
void graphics_subsystem_shutdown(void);
uint32_t gpu_get_device_count(void);
int gpu_get_device_info(uint32_t device_id, gpu_info_t *info);
uint64_t gpu_allocate_memory(uint32_t device_id, size_t size, uint32_t type, uint32_t flags);
int gpu_free_memory(uint32_t device_id, uint64_t gpu_address);

// System calls for enterprise GPU access
asmlinkage long sys_gpu_create_context(uint32_t gpu_id, uint32_t api_type);
asmlinkage long sys_gpu_allocate_memory(uint32_t gpu_id, size_t size, uint32_t type);
asmlinkage long sys_gpu_get_info(uint32_t gpu_id, gpu_info_t __user *info);

/* Graphics formats */
#define GFX_FORMAT_RGBA8888  0
#define GFX_FORMAT_RGB888    1
#define GFX_FORMAT_RGB565    2

/* Window flags */
#define WINDOW_FLAG_VISIBLE     0x01
#define WINDOW_FLAG_RESIZABLE   0x02
#define WINDOW_FLAG_DECORATED   0x04

/* Color helper macro */
#define RGBA(r, g, b, a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define RGB(r, g, b) RGBA(r, g, b, 255)

/* Common colors */
#define COLOR_BLACK     RGB(0, 0, 0)
#define COLOR_WHITE     RGB(255, 255, 255)
#define COLOR_RED       RGB(255, 0, 0)
#define COLOR_GREEN     RGB(0, 255, 0)
#define COLOR_BLUE      RGB(0, 0, 255)
#define COLOR_YELLOW    RGB(255, 255, 0)
#define COLOR_CYAN      RGB(0, 255, 255)
#define COLOR_MAGENTA   RGB(255, 0, 255)
#define COLOR_GRAY      RGB(128, 128, 128)
#define COLOR_DARKGRAY  RGB(64, 64, 64)
#define COLOR_LIGHTGRAY RGB(192, 192, 192)

/* Graphics API */
status_t graphics_init(void);
status_t graphics_clear_screen(uint32_t color);

/* Buffer management */
uint32_t graphics_create_buffer(uint32_t width, uint32_t height, uint32_t format);
status_t graphics_destroy_buffer(uint32_t buffer_id);

/* Window management */
uint32_t graphics_create_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t flags);
status_t graphics_destroy_window(uint32_t window_id);

/* Advanced drawing operations */
status_t graphics_fill_rect(uint32_t buffer_id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
status_t graphics_draw_line(uint32_t buffer_id, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color, uint32_t thickness);
status_t graphics_draw_circle(uint32_t buffer_id, uint32_t cx, uint32_t cy, uint32_t radius, uint32_t color, bool filled);
status_t graphics_draw_ellipse(uint32_t buffer_id, uint32_t cx, uint32_t cy, uint32_t rx, uint32_t ry, uint32_t color, bool filled);
status_t graphics_draw_triangle(uint32_t buffer_id, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, uint32_t color, bool filled);
status_t graphics_draw_polygon(uint32_t buffer_id, uint32_t* points, uint32_t num_points, uint32_t color, bool filled);
status_t graphics_draw_bezier_curve(uint32_t buffer_id, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, uint32_t x4, uint32_t y4, uint32_t color);
status_t graphics_blit(uint32_t src_buffer_id, uint32_t dst_buffer_id, 
                      uint32_t src_x, uint32_t src_y, uint32_t dst_x, uint32_t dst_y, 
                      uint32_t width, uint32_t height);
status_t graphics_blit_scaled(uint32_t src_buffer_id, uint32_t dst_buffer_id,
                             uint32_t src_x, uint32_t src_y, uint32_t src_w, uint32_t src_h,
                             uint32_t dst_x, uint32_t dst_y, uint32_t dst_w, uint32_t dst_h);
status_t graphics_blit_rotated(uint32_t src_buffer_id, uint32_t dst_buffer_id,
                              uint32_t src_x, uint32_t src_y, uint32_t dst_x, uint32_t dst_y,
                              uint32_t width, uint32_t height, float angle);

/* Text rendering */
status_t graphics_draw_text(uint32_t buffer_id, const char* text, uint32_t x, uint32_t y, uint32_t font_id, uint32_t color);
status_t graphics_measure_text(const char* text, uint32_t font_id, uint32_t* width, uint32_t* height);
uint32_t graphics_load_font(const char* font_path, uint32_t size);
status_t graphics_unload_font(uint32_t font_id);

/* Bitmap operations */
uint32_t graphics_load_bitmap(const char* image_path);
status_t graphics_draw_bitmap(uint32_t buffer_id, uint32_t bitmap_id, uint32_t x, uint32_t y);
status_t graphics_draw_bitmap_scaled(uint32_t buffer_id, uint32_t bitmap_id, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
status_t graphics_unload_bitmap(uint32_t bitmap_id);

/* Presentation */
status_t graphics_present(void);

/* 3D Graphics Support */
typedef struct matrix4x4 {
    float m[4][4];
} matrix4x4_t;

typedef struct vertex {
    float x, y, z;
    float u, v;  /* texture coordinates */
    uint32_t color;
} vertex_t;

typedef struct mesh {
    vertex_t* vertices;
    uint32_t* indices;
    uint32_t vertex_count;
    uint32_t index_count;
    uint32_t texture_id;
} mesh_t;

/* 3D rendering */
status_t graphics_3d_init(void);
uint32_t graphics_create_mesh(vertex_t* vertices, uint32_t vertex_count, uint32_t* indices, uint32_t index_count);
status_t graphics_destroy_mesh(uint32_t mesh_id);
status_t graphics_render_mesh(uint32_t mesh_id, matrix4x4_t* model_matrix, matrix4x4_t* view_matrix, matrix4x4_t* projection_matrix);
status_t graphics_set_camera(float x, float y, float z, float target_x, float target_y, float target_z);
status_t graphics_set_perspective(float fov, float aspect, float near_plane, float far_plane);

/* GPU acceleration */
status_t graphics_gpu_init(void);
status_t graphics_gpu_create_shader(const char* vertex_shader, const char* fragment_shader, uint32_t* shader_id);
status_t graphics_gpu_destroy_shader(uint32_t shader_id);
status_t graphics_gpu_use_shader(uint32_t shader_id);
status_t graphics_gpu_set_uniform_float(uint32_t shader_id, const char* name, float value);
status_t graphics_gpu_set_uniform_matrix(uint32_t shader_id, const char* name, matrix4x4_t* matrix);

/* Texture management */
uint32_t graphics_create_texture(uint32_t width, uint32_t height, uint32_t format, const void* data);
status_t graphics_destroy_texture(uint32_t texture_id);
status_t graphics_bind_texture(uint32_t texture_id, uint32_t slot);
status_t graphics_update_texture(uint32_t texture_id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data);

/* Information */
status_t graphics_get_screen_info(uint32_t* width, uint32_t* height, uint32_t* bpp);
status_t graphics_get_gpu_info(char* gpu_name, uint32_t* memory_size, uint32_t* compute_units);
status_t graphics_get_performance_stats(uint32_t* frames_per_second, uint32_t* draw_calls, uint32_t* triangles_rendered);