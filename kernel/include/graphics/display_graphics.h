/*
 * LimitlessOS Advanced Display & Graphics System
 * Next-generation graphics stack with HDR, variable refresh rate, multi-monitor support
 */

#ifndef LIMITLESSOS_GRAPHICS_DISPLAY_H
#define LIMITLESSOS_GRAPHICS_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

/* Display Connection Types */
typedef enum {
    DISPLAY_CONNECTOR_VGA = 0,
    DISPLAY_CONNECTOR_DVI,
    DISPLAY_CONNECTOR_HDMI,
    DISPLAY_CONNECTOR_DISPLAYPORT,
    DISPLAY_CONNECTOR_USB_C,
    DISPLAY_CONNECTOR_THUNDERBOLT,
    DISPLAY_CONNECTOR_WIRELESS,
    DISPLAY_CONNECTOR_EMBEDDED,
    DISPLAY_CONNECTOR_MAX
} display_connector_t;

/* Display Technologies */
typedef enum {
    DISPLAY_TECH_LCD = 0,
    DISPLAY_TECH_OLED,
    DISPLAY_TECH_QLED,
    DISPLAY_TECH_MICROLED,
    DISPLAY_TECH_EINK,
    DISPLAY_TECH_PLASMA,
    DISPLAY_TECH_CRT,
    DISPLAY_TECH_PROJECTOR,
    DISPLAY_TECH_MAX
} display_technology_t;

/* Color Spaces */
typedef enum {
    COLOR_SPACE_SRGB = 0,
    COLOR_SPACE_ADOBE_RGB,
    COLOR_SPACE_DCI_P3,
    COLOR_SPACE_REC2020,
    COLOR_SPACE_REC709,
    COLOR_SPACE_ACES,
    COLOR_SPACE_PROPHOTO_RGB,
    COLOR_SPACE_XYZ,
    COLOR_SPACE_MAX
} color_space_t;

/* HDR Standards */
typedef enum {
    HDR_NONE = 0,
    HDR_HDR10,
    HDR_HDR10_PLUS,
    HDR_DOLBY_VISION,
    HDR_HLG,                      /* Hybrid Log-Gamma */
    HDR_DISPLAYHDR_400,
    HDR_DISPLAYHDR_600,
    HDR_DISPLAYHDR_1000,
    HDR_DISPLAYHDR_1400,
    HDR_MAX
} hdr_standard_t;

/* Variable Refresh Rate Technologies */
typedef enum {
    VRR_NONE = 0,
    VRR_FREESYNC,
    VRR_GSYNC,
    VRR_GSYNC_COMPATIBLE,
    VRR_HDMI_VRR,
    VRR_ADAPTIVE_SYNC,
    VRR_MAX
} vrr_technology_t;

/* Graphics APIs */
typedef enum {
    GRAPHICS_API_OPENGL = 0,
    GRAPHICS_API_VULKAN,
    GRAPHICS_API_DIRECTX,
    GRAPHICS_API_METAL,
    GRAPHICS_API_OPENCL,
    GRAPHICS_API_CUDA,
    GRAPHICS_API_WEBGL,
    GRAPHICS_API_WEBGPU,
    GRAPHICS_API_MAX
} graphics_api_t;

/* GPU Vendors */
typedef enum {
    GPU_VENDOR_NVIDIA = 0,
    GPU_VENDOR_AMD,
    GPU_VENDOR_INTEL,
    GPU_VENDOR_ARM,
    GPU_VENDOR_QUALCOMM,
    GPU_VENDOR_APPLE,
    GPU_VENDOR_IMAGINATION,
    GPU_VENDOR_VIVANTE,
    GPU_VENDOR_MAX
} gpu_vendor_t;

/* Display Mode Structure */
typedef struct {
    uint32_t width;               /* Horizontal resolution */
    uint32_t height;              /* Vertical resolution */
    uint32_t refresh_rate;        /* Refresh rate in Hz */
    uint32_t bit_depth;           /* Color bit depth */
    color_space_t color_space;    /* Color space */
    hdr_standard_t hdr_support;   /* HDR support */
    bool interlaced;              /* Interlaced mode */
    bool reduced_blanking;        /* CVT reduced blanking */
    
    /* Timing information */
    uint32_t pixel_clock;         /* Pixel clock in kHz */
    uint32_t h_sync_start;        /* Horizontal sync start */
    uint32_t h_sync_end;          /* Horizontal sync end */
    uint32_t h_total;             /* Horizontal total */
    uint32_t v_sync_start;        /* Vertical sync start */
    uint32_t v_sync_end;          /* Vertical sync end */
    uint32_t v_total;             /* Vertical total */
    
    /* Sync polarities */
    bool h_sync_positive;         /* Horizontal sync polarity */
    bool v_sync_positive;         /* Vertical sync polarity */
} display_mode_t;

/* Display Capabilities */
typedef struct {
    /* Basic capabilities */
    uint32_t max_width;           /* Maximum horizontal resolution */
    uint32_t max_height;          /* Maximum vertical resolution */
    uint32_t min_refresh_rate;    /* Minimum refresh rate */
    uint32_t max_refresh_rate;    /* Maximum refresh rate */
    uint32_t max_bit_depth;       /* Maximum color bit depth */
    
    /* Advanced features */
    bool hdr_support;             /* HDR capability */
    hdr_standard_t hdr_standards[8]; /* Supported HDR standards */
    uint32_t hdr_standard_count;  /* Number of HDR standards */
    
    bool vrr_support;             /* Variable refresh rate support */
    vrr_technology_t vrr_tech;    /* VRR technology */
    uint32_t vrr_min_rate;        /* Minimum VRR rate */
    uint32_t vrr_max_rate;        /* Maximum VRR rate */
    
    /* Color capabilities */
    color_space_t color_spaces[8]; /* Supported color spaces */
    uint32_t color_space_count;   /* Number of color spaces */
    uint32_t max_luminance;       /* Maximum luminance (nits) */
    uint32_t min_luminance;       /* Minimum luminance (0.01 nits) */
    
    /* Physical properties */
    uint32_t physical_width_mm;   /* Physical width in millimeters */
    uint32_t physical_height_mm;  /* Physical height in millimeters */
    uint32_t dpi_x;               /* Horizontal DPI */
    uint32_t dpi_y;               /* Vertical DPI */
    
    /* Audio capabilities (for HDMI/DP) */
    bool audio_support;           /* Audio over display cable */
    uint32_t max_audio_channels;  /* Maximum audio channels */
    uint32_t audio_sample_rates[8]; /* Supported sample rates */
    uint32_t audio_rate_count;    /* Number of sample rates */
} display_capabilities_t;

/* Display Device Structure */
typedef struct {
    char name[128];               /* Display device name */
    char manufacturer[64];        /* Manufacturer name */
    char model[64];               /* Model name */
    char serial_number[64];       /* Serial number */
    uint32_t display_id;          /* Unique display ID */
    
    /* Connection information */
    display_connector_t connector; /* Connector type */
    display_technology_t technology; /* Display technology */
    bool connected;               /* Currently connected */
    bool enabled;                 /* Currently enabled */
    bool primary;                 /* Primary display */
    
    /* Current configuration */
    display_mode_t current_mode;  /* Current display mode */
    display_capabilities_t caps;  /* Display capabilities */
    
    /* Position and rotation */
    int32_t position_x;           /* X position in virtual space */
    int32_t position_y;           /* Y position in virtual space */
    uint32_t rotation;            /* Rotation in degrees (0, 90, 180, 270) */
    bool mirrored;                /* Mirrored display */
    
    /* Color management */
    color_space_t active_color_space; /* Active color space */
    char icc_profile_path[512];   /* ICC color profile path */
    uint32_t gamma;               /* Gamma correction (x100) */
    uint32_t brightness;          /* Brightness (0-100) */
    uint32_t contrast;            /* Contrast (0-100) */
    uint32_t saturation;          /* Saturation (0-100) */
    
    /* Advanced features */
    bool hdr_enabled;             /* HDR mode enabled */
    hdr_standard_t active_hdr;    /* Active HDR standard */
    bool vrr_enabled;             /* VRR enabled */
    uint32_t vrr_range_min;       /* VRR minimum rate */
    uint32_t vrr_range_max;       /* VRR maximum rate */
    
    /* Multi-monitor setup */
    uint32_t monitor_group_id;    /* Monitor group for spanning */
    bool bezel_compensation;      /* Bezel compensation enabled */
    uint32_t bezel_width_left;    /* Left bezel width */
    uint32_t bezel_width_right;   /* Right bezel width */
    uint32_t bezel_width_top;     /* Top bezel width */
    uint32_t bezel_width_bottom;  /* Bottom bezel width */
    
    /* Performance */
    uint64_t frame_count;         /* Total frames rendered */
    uint64_t dropped_frames;      /* Dropped frames */
    uint32_t current_fps;         /* Current FPS */
    uint32_t target_fps;          /* Target FPS */
    
    /* Power management */
    bool dpms_support;            /* DPMS support */
    uint32_t power_state;         /* Current power state */
    uint32_t backlight_level;     /* Backlight level (0-100) */
    bool auto_brightness;         /* Automatic brightness */
} display_device_t;

/* GPU Device Information */
typedef struct {
    char name[128];               /* GPU device name */
    char vendor_name[64];         /* Vendor name */
    gpu_vendor_t vendor;          /* GPU vendor */
    uint32_t device_id;           /* Device ID */
    uint32_t vendor_id;           /* Vendor ID */
    uint32_t pci_bus;             /* PCI bus number */
    uint32_t pci_device;          /* PCI device number */
    uint32_t pci_function;        /* PCI function number */
    
    /* Memory information */
    uint64_t vram_size;           /* Video RAM size in bytes */
    uint64_t vram_used;           /* Used VRAM */
    uint64_t vram_free;           /* Free VRAM */
    uint32_t memory_bandwidth;    /* Memory bandwidth in GB/s */
    
    /* Performance characteristics */
    uint32_t base_clock;          /* Base clock in MHz */
    uint32_t boost_clock;         /* Boost clock in MHz */
    uint32_t memory_clock;        /* Memory clock in MHz */
    uint32_t shader_units;        /* Number of shader units */
    uint32_t compute_units;       /* Number of compute units */
    uint32_t ray_tracing_units;   /* RT cores/units */
    uint32_t tensor_units;        /* AI/ML tensor units */
    
    /* API support */
    graphics_api_t supported_apis[8]; /* Supported graphics APIs */
    uint32_t api_count;           /* Number of supported APIs */
    
    /* Feature support */
    bool hardware_acceleration;   /* Hardware acceleration */
    bool ray_tracing_support;     /* Hardware ray tracing */
    bool variable_rate_shading;   /* Variable rate shading */
    bool mesh_shaders;            /* Mesh shader support */
    bool ai_acceleration;         /* AI/ML acceleration */
    bool video_encode;            /* Hardware video encoding */
    bool video_decode;            /* Hardware video decoding */
    
    /* Current state */
    uint32_t temperature;         /* GPU temperature in Celsius */
    uint32_t fan_speed;           /* Fan speed percentage */
    uint32_t power_draw;          /* Current power draw in watts */
    uint32_t utilization;         /* GPU utilization percentage */
    uint32_t memory_utilization;  /* VRAM utilization percentage */
    
    /* Performance monitoring */
    uint64_t frames_rendered;     /* Total frames rendered */
    uint64_t triangles_processed; /* Triangles processed */
    uint64_t pixels_shaded;       /* Pixels shaded */
    uint32_t draw_calls;          /* Draw calls per frame */
} gpu_device_t;

/* Graphics Context */
typedef struct {
    uint32_t context_id;          /* Unique context ID */
    graphics_api_t api;           /* Graphics API */
    uint32_t version_major;       /* API version major */
    uint32_t version_minor;       /* API version minor */
    
    /* Rendering target */
    uint32_t width;               /* Render target width */
    uint32_t height;              /* Render target height */
    uint32_t samples;             /* MSAA samples */
    uint32_t bit_depth;           /* Color bit depth */
    
    /* State */
    bool vsync_enabled;           /* V-sync enabled */
    bool triple_buffering;        /* Triple buffering */
    uint32_t swap_interval;       /* Swap interval */
    
    /* Performance */
    uint64_t frame_time_ns;       /* Frame time in nanoseconds */
    uint32_t cpu_time_ms;         /* CPU time per frame */
    uint32_t gpu_time_ms;         /* GPU time per frame */
} graphics_context_t;

/* Display Configuration */
typedef struct {
    /* Multi-monitor setup */
    uint32_t display_count;       /* Number of displays */
    display_device_t displays[16]; /* Display devices */
    uint32_t primary_display;     /* Primary display ID */
    
    /* Virtual desktop */
    uint32_t virtual_width;       /* Virtual desktop width */
    uint32_t virtual_height;      /* Virtual desktop height */
    bool spanning_enabled;        /* Desktop spanning enabled */
    
    /* Global settings */
    bool hardware_acceleration;   /* Global HW acceleration */
    bool compositing_enabled;     /* Desktop compositing */
    uint32_t global_refresh_rate; /* Global refresh rate */
    
    /* Power management */
    uint32_t display_timeout;     /* Display timeout in seconds */
    bool auto_suspend;            /* Auto suspend displays */
    uint32_t brightness_level;    /* Global brightness */
    
    /* Color management */
    bool color_management;        /* Color management enabled */
    char default_icc_profile[512]; /* Default ICC profile */
    uint32_t gamma_correction;    /* Global gamma correction */
} display_configuration_t;

/* Graphics System State */
typedef struct {
    bool initialized;
    pthread_mutex_t lock;
    
    /* GPU devices */
    uint32_t gpu_count;
    gpu_device_t gpus[8];         /* Support up to 8 GPUs */
    uint32_t active_gpu;          /* Active GPU index */
    
    /* Display configuration */
    display_configuration_t config;
    
    /* Graphics contexts */
    uint32_t context_count;
    graphics_context_t contexts[64]; /* Support up to 64 contexts */
    
    /* Performance monitoring */
    struct {
        uint64_t total_frames;
        uint64_t dropped_frames;
        uint64_t vsync_misses;
        uint32_t average_fps;
        uint32_t peak_fps;
        uint64_t gpu_memory_peak;
        uint64_t total_draw_calls;
    } stats;
    
    /* Threading */
    pthread_t monitor_thread;
    pthread_t performance_thread;
    bool threads_running;
} graphics_system_t;

/* Frame Buffer Structure */
typedef struct {
    uint32_t width;               /* Frame buffer width */
    uint32_t height;              /* Frame buffer height */
    uint32_t pitch;               /* Bytes per line */
    uint32_t bpp;                 /* Bits per pixel */
    uint32_t format;              /* Pixel format */
    void *buffer;                 /* Frame buffer memory */
    uint64_t physical_addr;       /* Physical address */
    uint32_t size;                /* Buffer size in bytes */
    bool double_buffered;         /* Double buffering */
    bool hardware_managed;        /* Hardware-managed buffer */
} frame_buffer_t;

/* Function Prototypes */

/* System Initialization */
int graphics_system_init(void);
int graphics_system_cleanup(void);
int graphics_detect_hardware(void);
int graphics_load_drivers(void);

/* GPU Management */
int gpu_enumerate_devices(gpu_device_t *devices, uint32_t max_devices, uint32_t *count);
gpu_device_t *gpu_get_device(uint32_t gpu_id);
int gpu_set_active(uint32_t gpu_id);
int gpu_get_capabilities(uint32_t gpu_id, void *caps);
int gpu_set_power_state(uint32_t gpu_id, uint32_t power_state);
int gpu_monitor_performance(uint32_t gpu_id);

/* Display Management */
int display_enumerate(display_device_t *displays, uint32_t max_displays, uint32_t *count);
display_device_t *display_get_device(uint32_t display_id);
int display_get_modes(uint32_t display_id, display_mode_t *modes, uint32_t max_modes, uint32_t *count);
int display_set_mode(uint32_t display_id, const display_mode_t *mode);
int display_enable(uint32_t display_id, bool enable);
int display_set_primary(uint32_t display_id);

/* Multi-Monitor Support */
int display_configure_layout(uint32_t display_count, const display_device_t *displays);
int display_enable_spanning(bool enable);
int display_set_position(uint32_t display_id, int32_t x, int32_t y);
int display_set_rotation(uint32_t display_id, uint32_t rotation);
int display_mirror(uint32_t source_id, uint32_t target_id);
int display_create_group(const uint32_t *display_ids, uint32_t count);

/* HDR and Color Management */
int display_enable_hdr(uint32_t display_id, hdr_standard_t hdr_standard);
int display_set_color_space(uint32_t display_id, color_space_t color_space);
int display_load_icc_profile(uint32_t display_id, const char *profile_path);
int display_set_brightness(uint32_t display_id, uint32_t brightness);
int display_set_gamma(uint32_t display_id, uint32_t gamma);
int display_calibrate_color(uint32_t display_id);

/* Variable Refresh Rate */
int display_enable_vrr(uint32_t display_id, bool enable);
int display_set_vrr_range(uint32_t display_id, uint32_t min_rate, uint32_t max_rate);
int display_get_vrr_status(uint32_t display_id, bool *enabled, uint32_t *current_rate);

/* Graphics Context Management */
int graphics_create_context(graphics_api_t api, uint32_t width, uint32_t height, graphics_context_t *context);
int graphics_destroy_context(uint32_t context_id);
int graphics_make_current(uint32_t context_id);
int graphics_swap_buffers(uint32_t context_id);
int graphics_set_vsync(uint32_t context_id, bool enabled);

/* Frame Buffer Management */
int framebuffer_create(uint32_t width, uint32_t height, uint32_t format, frame_buffer_t *fb);
int framebuffer_destroy(frame_buffer_t *fb);
int framebuffer_map(frame_buffer_t *fb);
int framebuffer_unmap(frame_buffer_t *fb);
int framebuffer_present(const frame_buffer_t *fb, uint32_t display_id);

/* Hardware Acceleration */
int gpu_compute_init(uint32_t gpu_id);
int gpu_compute_execute(uint32_t gpu_id, const char *kernel, void *data, size_t size);
int gpu_video_encode_init(uint32_t gpu_id, uint32_t codec);
int gpu_video_decode_init(uint32_t gpu_id, uint32_t codec);
int gpu_ray_tracing_init(uint32_t gpu_id);

/* Performance and Monitoring */
int graphics_get_performance_stats(void *stats, size_t size);
int display_measure_latency(uint32_t display_id);
int gpu_benchmark(uint32_t gpu_id);
int graphics_optimize_performance(void);

/* Power Management */
int display_set_dpms_state(uint32_t display_id, uint32_t state);
int gpu_set_clock_speeds(uint32_t gpu_id, uint32_t core_clock, uint32_t memory_clock);
int display_set_backlight(uint32_t display_id, uint32_t level);
int graphics_enable_power_saving(bool enable);

/* Professional Features */
int display_enable_10bit_output(uint32_t display_id, bool enable);
int display_set_custom_resolution(uint32_t display_id, uint32_t width, uint32_t height, uint32_t refresh);
int gpu_enable_ecc_memory(uint32_t gpu_id, bool enable);
int display_setup_color_pipeline(uint32_t display_id, const char *config);

/* Utility Functions */
const char *display_connector_name(display_connector_t connector);
const char *display_technology_name(display_technology_t tech);
const char *color_space_name(color_space_t space);
const char *hdr_standard_name(hdr_standard_t hdr);
const char *graphics_api_name(graphics_api_t api);
const char *gpu_vendor_name(gpu_vendor_t vendor);
bool display_mode_equal(const display_mode_t *mode1, const display_mode_t *mode2);
uint32_t calculate_refresh_rate(const display_mode_t *mode);
uint64_t calculate_pixel_clock(const display_mode_t *mode);

#endif /* LIMITLESSOS_GRAPHICS_DISPLAY_H */