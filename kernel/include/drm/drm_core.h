/*
 * LimitlessOS Advanced Graphics Subsystem (DRM/KMS)
 * Production-quality GPU driver framework with hardware acceleration
 * Features: Mode setting, memory management, command submission, power management
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../kernel.h"

/* DRM/KMS constants */
#define DRM_MAX_MINOR           256
#define DRM_MAX_CRTC            8
#define DRM_MAX_PLANE           32
#define DRM_MAX_ENCODER         8
#define DRM_MAX_CONNECTOR       16
#define DRM_MAX_MODE            64
#define DRM_MAX_FB              256

/* GPU memory types */
typedef enum {
    DRM_MEM_SYSTEM = 0,         /* System RAM */
    DRM_MEM_VRAM,               /* Video RAM */
    DRM_MEM_GTT,                /* Graphics Translation Table */
    DRM_MEM_STOLEN,             /* Stolen system memory */
    DRM_MEM_CMA,                /* Contiguous Memory Allocator */
    DRM_MEM_GART,               /* Graphics Address Remapping Table */
    DRM_MEM_MAX
} drm_memory_type_t;

/* Display connection types */
typedef enum {
    DRM_CONNECTOR_Unknown = 0,
    DRM_CONNECTOR_VGA,
    DRM_CONNECTOR_DVII,
    DRM_CONNECTOR_DVID,
    DRM_CONNECTOR_DVIA,
    DRM_CONNECTOR_Composite,
    DRM_CONNECTOR_SVIDEO,
    DRM_CONNECTOR_LVDS,
    DRM_CONNECTOR_Component,
    DRM_CONNECTOR_9PinDIN,
    DRM_CONNECTOR_DisplayPort,
    DRM_CONNECTOR_HDMIA,
    DRM_CONNECTOR_HDMIB,
    DRM_CONNECTOR_TV,
    DRM_CONNECTOR_eDP,
    DRM_CONNECTOR_VIRTUAL,
    DRM_CONNECTOR_DSI,
    DRM_CONNECTOR_DPI,
    DRM_CONNECTOR_WRITEBACK,
    DRM_CONNECTOR_SPI,
    DRM_CONNECTOR_USB
} drm_connector_type_t;

/* Display mode structure */
typedef struct drm_display_mode {
    char name[32];
    
    /* Timing information */
    int clock;                  /* Pixel clock in kHz */
    int hdisplay;              /* Horizontal display size */
    int hsync_start;           /* Horizontal sync start */
    int hsync_end;             /* Horizontal sync end */
    int htotal;                /* Horizontal total */
    int hskew;                 /* Horizontal skew */
    int vdisplay;              /* Vertical display size */
    int vsync_start;           /* Vertical sync start */
    int vsync_end;             /* Vertical sync end */
    int vtotal;                /* Vertical total */
    int vscan;                 /* Vertical scan */
    
    /* Flags */
    unsigned int flags;
    unsigned int type;
    
    /* Derived information */
    int width_mm, height_mm;   /* Physical dimensions */
    int vrefresh;              /* Refresh rate in Hz */
    
    struct list_head head;
} drm_display_mode_t;

/* Frame buffer structure */
typedef struct drm_framebuffer {
    uint32_t fb_id;
    struct drm_device *dev;
    
    /* Format information */
    uint32_t format;           /* DRM_FORMAT_* */
    uint32_t width, height;
    uint32_t pitches[4];       /* Line stride in bytes */
    uint32_t offsets[4];       /* Plane offsets */
    uint64_t modifier;         /* Format modifier */
    
    /* Memory backing */
    struct drm_gem_object *obj[4];
    
    /* Reference counting */
    struct kref refcount;
    
    /* Callbacks */
    const struct drm_framebuffer_funcs *funcs;
    
    struct list_head head;
} drm_framebuffer_t;

/* GPU memory object */
typedef struct drm_gem_object {
    struct kref refcount;
    unsigned handle_count;
    struct drm_device *dev;
    struct file *filp;
    
    /* Memory information */
    size_t size;
    int name;
    uint32_t read_domains;
    uint32_t write_domain;
    uint32_t pending_read_domains;
    uint32_t pending_write_domain;
    
    /* Memory type and placement */
    drm_memory_type_t memory_type;
    uint64_t gpu_address;
    void *cpu_address;
    
    /* DMA information */
    struct sg_table *sgt;
    dma_addr_t dma_addr;
    
    /* Import/export */
    struct dma_buf *dma_buf;
    struct dma_buf_attachment *import_attach;
    
    /* Synchronization */
    struct dma_resv *resv;
    
    /* Memory management */
    struct {
        bool mappable;
        bool coherent;
        bool cached;
        uint32_t alignment;
        uint64_t offset;
    } mem_info;
    
} drm_gem_object_t;

/* CRTC (display controller) structure */
typedef struct drm_crtc {
    struct drm_device *dev;
    struct device_node *port;
    
    struct list_head head;
    uint32_t crtc_id;
    char *name;
    
    /* State */
    bool enabled;
    struct drm_display_mode mode;
    struct drm_display_mode hwmode;
    int x, y;
    const struct drm_crtc_funcs *funcs;
    uint32_t gamma_size;
    uint16_t *gamma_store;
    
    /* Current framebuffer */
    struct drm_framebuffer *primary;
    struct drm_framebuffer *cursor;
    
    /* Properties */
    struct drm_property_blob *degamma_lut;
    struct drm_property_blob *ctm;
    struct drm_property_blob *gamma_lut;
    
    /* Hardware cursor */
    struct {
        bool enabled;
        int x, y;
        uint32_t width, height;
        uint32_t hot_x, hot_y;
    } cursor_info;
    
    /* Performance monitoring */
    struct {
        uint64_t frame_count;
        uint64_t vblank_count;
        uint64_t underrun_count;
        uint64_t flip_count;
        uint64_t avg_frame_time_ns;
    } stats;
    
} drm_crtc_t;

/* Display encoder structure */
typedef struct drm_encoder {
    struct drm_device *dev;
    struct list_head head;
    
    uint32_t encoder_id;
    uint32_t encoder_type;
    char *name;
    
    uint32_t possible_crtcs;
    uint32_t possible_clones;
    
    struct drm_crtc *crtc;
    const struct drm_encoder_funcs *funcs;
    
    /* Bridge chain */
    struct drm_bridge *bridge;
    
} drm_encoder_t;

/* Display connector structure */
typedef struct drm_connector {
    struct drm_device *dev;
    struct device *kdev;
    struct device_attribute *attr;
    struct list_head head;
    
    uint32_t connector_id;
    uint32_t connector_type;
    uint32_t connector_type_id;
    char *name;
    
    /* Connection state */
    enum drm_connector_status status;
    bool interlace_allowed;
    bool doublescan_allowed;
    bool stereo_allowed;
    
    /* Modes */
    struct list_head modes;
    enum drm_connector_status detect_status;
    
    /* Physical information */
    int width_mm, height_mm;
    
    /* EDID information */
    struct edid *edid;
    struct edid *edid_blob_ptr;
    
    /* Audio capabilities */
    bool has_audio;
    struct drm_audio_info audio_info;
    
    /* HDR capabilities */
    struct {
        bool supported;
        uint32_t eotf;
        uint32_t metadata_type;
        uint16_t max_luminance;
        uint16_t max_frame_avg_luminance;
        uint16_t min_luminance;
    } hdr_info;
    
    /* Power management */
    int dpms;
    
    const struct drm_connector_funcs *funcs;
    
    /* Encoder connection */
    struct drm_encoder *encoder;
    
    /* Backlight control */
    struct backlight_device *backlight;
    
} drm_connector_t;

/* GPU command submission */
typedef struct drm_gpu_command {
    uint32_t cmd_id;
    uint32_t engine_id;
    
    /* Command buffer */
    struct {
        void *data;
        size_t size;
        uint64_t gpu_addr;
    } cmd_buffer;
    
    /* Resource bindings */
    struct {
        struct drm_gem_object **objects;
        uint32_t count;
        uint32_t *read_domains;
        uint32_t *write_domains;
    } resources;
    
    /* Synchronization */
    struct {
        struct dma_fence **wait_fences;
        uint32_t wait_count;
        struct dma_fence *signal_fence;
    } sync;
    
    /* Execution context */
    struct drm_gpu_context *context;
    
    /* Priority and scheduling */
    int priority;
    uint64_t timeout_ns;
    
} drm_gpu_command_t;

/* GPU execution context */
typedef struct drm_gpu_context {
    uint32_t ctx_id;
    struct drm_device *dev;
    
    /* Process information */
    pid_t pid;
    struct mm_struct *mm;
    
    /* Virtual address space */
    struct {
        uint64_t start;
        uint64_t size;
        struct drm_mm mm;
    } vm;
    
    /* Resource tracking */
    struct {
        struct drm_gem_object **objects;
        uint32_t count;
        uint32_t capacity;
    } resources;
    
    /* Statistics */
    struct {
        uint64_t commands_submitted;
        uint64_t gpu_time_ns;
        uint64_t memory_usage;
        uint64_t last_activity;
    } stats;
    
    /* Priority and scheduling */
    int priority;
    bool preemptible;
    
} drm_gpu_context_t;

/* GPU scheduler */
typedef struct drm_gpu_scheduler {
    char name[32];
    uint32_t hw_submission_limit;
    long timeout;
    
    /* Job queue */
    struct {
        struct list_head pending;
        struct list_head running;
        spinlock_t lock;
    } job_queue;
    
    /* Worker thread */
    struct task_struct *thread;
    wait_queue_head_t wake_up_worker;
    wait_queue_head_t job_scheduled;
    
    /* Statistics */
    atomic64_t num_jobs;
    atomic_t hw_rq_count;
    atomic_t score;
    
    /* Callbacks */
    const struct drm_sched_backend_ops *ops;
    
} drm_gpu_scheduler_t;

/* Main DRM device structure */
typedef struct drm_device {
    int if_version;
    
    /* Device information */
    struct device *dev;
    struct pci_dev *pdev;
    char *unique;
    
    /* Driver information */
    const struct drm_driver *driver;
    void *dev_private;
    
    /* Mode setting objects */
    struct {
        struct list_head crtcs;
        struct list_head connectors;
        struct list_head encoders;
        struct list_head planes;
        struct list_head bridges;
        
        uint32_t num_crtc;
        uint32_t num_connector;
        uint32_t num_encoder;
        uint32_t num_total_plane;
        
        struct drm_mode_config_funcs *funcs;
    } mode_config;
    
    /* Memory management */
    struct {
        struct drm_mm vram_mm;
        struct drm_mm gtt_mm;
        
        /* Memory regions */
        struct {
            uint64_t start;
            uint64_t size;
            bool available;
        } mem_regions[DRM_MEM_MAX];
        
        /* Memory statistics */
        atomic64_t vram_usage;
        atomic64_t gtt_usage;
        atomic64_t system_usage;
        
    } memory;
    
    /* GPU engines */
    struct {
        struct drm_gpu_scheduler *schedulers[8];
        uint32_t num_schedulers;
        
        /* Engine capabilities */
        struct {
            bool graphics;
            bool compute;
            bool copy;
            bool video_decode;
            bool video_encode;
        } caps;
        
    } gpu;
    
    /* Power management */
    struct {
        bool runtime_pm;
        int usage_count;
        
        /* Power states */
        enum {
            DRM_POWER_ON = 0,
            DRM_POWER_SUSPEND,
            DRM_POWER_OFF
        } state;
        
        /* DVFS (Dynamic Voltage and Frequency Scaling) */
        struct {
            uint32_t min_freq_mhz;
            uint32_t max_freq_mhz;
            uint32_t current_freq_mhz;
            bool boost_enabled;
        } dvfs;
        
    } power;
    
    /* Performance monitoring */
    struct {
        uint64_t frames_rendered;
        uint64_t commands_processed;
        uint64_t memory_bandwidth_mb_s;
        uint32_t gpu_utilization_percent;
        uint32_t memory_utilization_percent;
    } perf_counters;
    
    /* Security */
    struct {
        bool secure_display;
        bool content_protection;
        uint32_t protection_level;
    } security;
    
    /* File operations */
    struct mutex struct_mutex;
    struct mutex master_mutex;
    
    /* Suspend/resume */
    bool suspend_state;
    
} drm_device_t;

/* DRM driver structure */
struct drm_driver {
    char *name;
    char *desc;
    char *date;
    
    u32 driver_features;
    
    /* Device lifecycle */
    int (*load) (struct drm_device *, unsigned long flags);
    void (*unload) (struct drm_device *);
    
    /* Display management */
    void (*disable_vblank) (struct drm_device *dev, unsigned int pipe);
    int (*enable_vblank) (struct drm_device *dev, unsigned int pipe);
    u32 (*get_vblank_counter) (struct drm_device *dev, unsigned int pipe);
    
    /* Memory management */
    struct drm_gem_object *(*gem_create_object)(struct drm_device *dev, size_t size);
    int (*gem_open_object) (struct drm_gem_object *, struct drm_file *);
    void (*gem_close_object) (struct drm_gem_object *, struct drm_file *);
    
    /* GPU operations */
    int (*gpu_init)(struct drm_device *dev);
    void (*gpu_fini)(struct drm_device *dev);
    int (*submit_command)(struct drm_device *dev, struct drm_gpu_command *cmd);
    
    /* Power management */
    int (*suspend)(struct drm_device *dev, bool state);
    int (*resume)(struct drm_device *dev);
    
    /* Debug and diagnostics */
    void (*debugfs_init)(struct drm_minor *minor);
    void (*debugfs_cleanup)(struct drm_minor *minor);
    
    const struct file_operations *fops;
    
    int major;
    int minor;
    int patchlevel;
};

/* External DRM subsystem */
extern struct drm_device *drm_devices[DRM_MAX_MINOR];
extern int drm_device_count;

/* Core DRM functions */
int drm_init(void);
struct drm_device *drm_dev_alloc(const struct drm_driver *driver, struct device *parent);
void drm_dev_put(struct drm_device *dev);
int drm_dev_register(struct drm_device *dev, unsigned long flags);
void drm_dev_unregister(struct drm_device *dev);

/* Mode setting functions */
int drm_mode_config_init(struct drm_device *dev);
void drm_mode_config_cleanup(struct drm_device *dev);
struct drm_display_mode *drm_mode_create(struct drm_device *dev);
void drm_mode_destroy(struct drm_device *dev, struct drm_display_mode *mode);
int drm_mode_setmode(struct drm_crtc *crtc, struct drm_display_mode *mode,
                     int x, int y, struct drm_framebuffer *fb);

/* CRTC management */
int drm_crtc_init_with_planes(struct drm_device *dev, struct drm_crtc *crtc,
                              struct drm_plane *primary, struct drm_plane *cursor,
                              const struct drm_crtc_funcs *funcs, const char *name);
void drm_crtc_cleanup(struct drm_crtc *crtc);
int drm_crtc_enable_color_mgmt(struct drm_crtc *crtc, uint degamma_lut_size,
                               bool has_ctm, uint gamma_lut_size);

/* Connector management */
int drm_connector_init(struct drm_device *dev, struct drm_connector *connector,
                       const struct drm_connector_funcs *funcs, int connector_type);
void drm_connector_cleanup(struct drm_connector *connector);
enum drm_connector_status drm_connector_detect(struct drm_connector *connector, bool force);
int drm_connector_update_edid_property(struct drm_connector *connector, const struct edid *edid);

/* Framebuffer management */
struct drm_framebuffer *drm_framebuffer_init(struct drm_device *dev,
                                             const struct drm_mode_fb_cmd2 *mode_cmd,
                                             struct drm_gem_object *obj);
void drm_framebuffer_cleanup(struct drm_framebuffer *fb);
void drm_framebuffer_remove(struct drm_framebuffer *fb);
struct drm_framebuffer *drm_framebuffer_lookup(struct drm_device *dev, uint32_t id);

/* GEM memory management */
void drm_gem_object_init(struct drm_device *dev, struct drm_gem_object *obj, size_t size);
void drm_gem_object_release(struct drm_gem_object *obj);
void drm_gem_object_free(struct kref *kref);
struct drm_gem_object *drm_gem_object_lookup(struct drm_file *filp, uint32_t handle);
int drm_gem_handle_create(struct drm_file *file_priv, struct drm_gem_object *obj, uint32_t *handlep);
int drm_gem_handle_delete(struct drm_file *filp, uint32_t handle);

/* GPU command submission */
int drm_gpu_submit_command(struct drm_device *dev, struct drm_gpu_command *cmd);
struct drm_gpu_context *drm_gpu_context_create(struct drm_device *dev);
void drm_gpu_context_destroy(struct drm_gpu_context *ctx);

/* GPU scheduling */
int drm_sched_init(struct drm_gpu_scheduler *sched, const struct drm_sched_backend_ops *ops,
                   uint32_t hw_submission, long timeout, const char *name);
void drm_sched_fini(struct drm_gpu_scheduler *sched);
int drm_sched_job_init(struct drm_sched_job *job, struct drm_gpu_scheduler *sched,
                       struct drm_sched_entity *entity, void *owner);

/* VBLANK handling */
int drm_vblank_init(struct drm_device *dev, unsigned int num_crtcs);
void drm_vblank_cleanup(struct drm_device *dev);
u32 drm_vblank_count(struct drm_device *dev, unsigned int pipe);
int drm_vblank_get(struct drm_device *dev, unsigned int pipe);
void drm_vblank_put(struct drm_device *dev, unsigned int pipe);

/* Power management */
int drm_pm_suspend(struct drm_device *dev);
int drm_pm_resume(struct drm_device *dev);
void drm_pm_set_frequency(struct drm_device *dev, uint32_t freq_mhz);
uint32_t drm_pm_get_frequency(struct drm_device *dev);

/* EDID parsing */
struct edid *drm_get_edid(struct drm_connector *connector, struct i2c_adapter *adapter);
bool drm_edid_is_valid(const struct edid *edid);
int drm_add_edid_modes(struct drm_connector *connector, struct edid *edid);

/* Format helpers */
uint32_t drm_mode_legacy_fb_format(uint32_t bpp, uint32_t depth);
int drm_format_num_planes(uint32_t format);
int drm_format_plane_cpp(uint32_t format, int plane);
int drm_format_horz_chroma_subsampling(uint32_t format);
int drm_format_vert_chroma_subsampling(uint32_t format);

/* Atomic mode setting */
struct drm_atomic_state *drm_atomic_state_alloc(struct drm_device *dev);
void drm_atomic_state_clear(struct drm_atomic_state *state);
void drm_atomic_state_put(struct drm_atomic_state *state);
int drm_atomic_commit(struct drm_atomic_state *state);
int drm_atomic_nonblocking_commit(struct drm_atomic_state *state);

/* Driver feature flags */
#define DRIVER_MODESET          (1 << 0)
#define DRIVER_RENDER           (1 << 1)
#define DRIVER_ATOMIC           (1 << 2)
#define DRIVER_GEM              (1 << 3)
#define DRIVER_PRIME            (1 << 4)
#define DRIVER_SYNCOBJ          (1 << 5)
#define DRIVER_COMPUTE          (1 << 6)

/* Memory domain flags */
#define DRM_GEM_DOMAIN_CPU      (1 << 0)
#define DRM_GEM_DOMAIN_RENDER   (1 << 1)
#define DRM_GEM_DOMAIN_SAMPLER  (1 << 2)
#define DRM_GEM_DOMAIN_COMMAND  (1 << 3)
#define DRM_GEM_DOMAIN_VERTEX   (1 << 4)
#define DRM_GEM_DOMAIN_GTT      (1 << 5)
#define DRM_GEM_DOMAIN_VRAM     (1 << 6)

/* Utility macros */
#define drm_for_each_crtc(crtc, dev) \
    list_for_each_entry(crtc, &(dev)->mode_config.crtc_list, head)

#define drm_for_each_connector_iter(connector, iter) \
    while ((connector = drm_connector_list_iter_next(iter)))

#define to_drm_crtc(x) container_of(x, struct drm_crtc, base)
#define to_drm_connector(x) container_of(x, struct drm_connector, base)
#define to_drm_encoder(x) container_of(x, struct drm_encoder, base)