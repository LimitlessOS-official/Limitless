/*
 * LimitlessOS Advanced Audio/Sound Subsystem  
 * Production audio system with professional features
 * Features: Multi-channel, spatial audio, DSP, low-latency, professional I/O
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../kernel.h"

/* Audio subsystem constants */
#define MAX_AUDIO_DEVICES       64
#define MAX_AUDIO_STREAMS       256
#define MAX_AUDIO_MIXERS        32
#define MAX_AUDIO_EFFECTS       128
#define MAX_CHANNELS            32
#define MAX_SAMPLE_RATE         192000
#define MIN_SAMPLE_RATE         8000
#define MAX_BUFFER_SIZE         8192
#define MIN_BUFFER_SIZE         64

/* Sample formats */
typedef enum {
    AUDIO_FORMAT_UNKNOWN = 0,
    AUDIO_FORMAT_PCM_S8,        /* Signed 8-bit PCM */
    AUDIO_FORMAT_PCM_U8,        /* Unsigned 8-bit PCM */
    AUDIO_FORMAT_PCM_S16_LE,    /* Signed 16-bit PCM Little Endian */
    AUDIO_FORMAT_PCM_S16_BE,    /* Signed 16-bit PCM Big Endian */
    AUDIO_FORMAT_PCM_S24_LE,    /* Signed 24-bit PCM Little Endian */
    AUDIO_FORMAT_PCM_S24_BE,    /* Signed 24-bit PCM Big Endian */
    AUDIO_FORMAT_PCM_S32_LE,    /* Signed 32-bit PCM Little Endian */
    AUDIO_FORMAT_PCM_S32_BE,    /* Signed 32-bit PCM Big Endian */
    AUDIO_FORMAT_FLOAT32_LE,    /* 32-bit Float Little Endian */
    AUDIO_FORMAT_FLOAT32_BE,    /* 32-bit Float Big Endian */
    AUDIO_FORMAT_FLOAT64_LE,    /* 64-bit Float Little Endian */
    AUDIO_FORMAT_FLOAT64_BE,    /* 64-bit Float Big Endian */
    AUDIO_FORMAT_DSD64,         /* Direct Stream Digital 64fs */
    AUDIO_FORMAT_DSD128,        /* Direct Stream Digital 128fs */
    AUDIO_FORMAT_MAX
} audio_format_t;

/* Audio device types */
typedef enum {
    AUDIO_DEVICE_UNKNOWN = 0,
    AUDIO_DEVICE_PLAYBACK,      /* Speakers, headphones */
    AUDIO_DEVICE_CAPTURE,       /* Microphones */
    AUDIO_DEVICE_DUPLEX,        /* Full duplex (playback + capture) */
    AUDIO_DEVICE_MIDI_IN,       /* MIDI input */
    AUDIO_DEVICE_MIDI_OUT,      /* MIDI output */
    AUDIO_DEVICE_DIGITAL_IN,    /* S/PDIF, ADAT input */
    AUDIO_DEVICE_DIGITAL_OUT,   /* S/PDIF, ADAT output */
    AUDIO_DEVICE_BLUETOOTH,     /* Bluetooth audio */
    AUDIO_DEVICE_USB,          /* USB audio */
    AUDIO_DEVICE_HDMI,         /* HDMI audio */
    AUDIO_DEVICE_THUNDERBOLT,  /* Thunderbolt audio */
    AUDIO_DEVICE_MAX
} audio_device_type_t;

/* Audio stream states */
typedef enum {
    AUDIO_STREAM_CLOSED = 0,
    AUDIO_STREAM_OPEN,
    AUDIO_STREAM_PREPARED,
    AUDIO_STREAM_RUNNING,
    AUDIO_STREAM_PAUSED,
    AUDIO_STREAM_DRAINING,
    AUDIO_STREAM_XRUN,         /* Underrun/overrun */
    AUDIO_STREAM_SUSPENDED,
    AUDIO_STREAM_ERROR,
    AUDIO_STREAM_MAX
} audio_stream_state_t;

/* Channel positions for spatial audio */
typedef enum {
    AUDIO_CHANNEL_FRONT_LEFT = 0,
    AUDIO_CHANNEL_FRONT_RIGHT,
    AUDIO_CHANNEL_FRONT_CENTER,
    AUDIO_CHANNEL_LOW_FREQUENCY,
    AUDIO_CHANNEL_BACK_LEFT,
    AUDIO_CHANNEL_BACK_RIGHT,
    AUDIO_CHANNEL_FRONT_LEFT_CENTER,
    AUDIO_CHANNEL_FRONT_RIGHT_CENTER,
    AUDIO_CHANNEL_BACK_CENTER,
    AUDIO_CHANNEL_SIDE_LEFT,
    AUDIO_CHANNEL_SIDE_RIGHT,
    AUDIO_CHANNEL_TOP_CENTER,
    AUDIO_CHANNEL_TOP_FRONT_LEFT,
    AUDIO_CHANNEL_TOP_FRONT_CENTER,
    AUDIO_CHANNEL_TOP_FRONT_RIGHT,
    AUDIO_CHANNEL_TOP_BACK_LEFT,
    AUDIO_CHANNEL_TOP_BACK_CENTER,
    AUDIO_CHANNEL_TOP_BACK_RIGHT,
    AUDIO_CHANNEL_MAX
} audio_channel_t;

/* Audio buffer descriptor */
typedef struct audio_buffer {
    void *data;                 /* Buffer data */
    size_t size;               /* Buffer size in bytes */
    size_t frames;             /* Number of audio frames */
    uint64_t timestamp;        /* Buffer timestamp */
    uint32_t sequence;         /* Sequence number */
    
    /* Buffer state */
    bool filled;               /* Buffer contains valid data */
    bool processed;            /* Buffer has been processed */
    uint32_t flags;           /* Buffer flags */
    
    /* Memory management */
    dma_addr_t dma_addr;      /* DMA address */
    bool is_dma_coherent;     /* DMA coherency */
    
    struct list_head list;
    
} audio_buffer_t;

/* Audio format description */
typedef struct audio_format_desc {
    audio_format_t format;     /* Sample format */
    uint32_t sample_rate;      /* Sample rate in Hz */
    uint8_t channels;          /* Number of channels */
    uint8_t bits_per_sample;   /* Bits per sample */
    uint8_t bytes_per_sample;  /* Bytes per sample */
    uint8_t bytes_per_frame;   /* Bytes per frame */
    uint32_t buffer_time_us;   /* Buffer time in microseconds */
    uint32_t period_time_us;   /* Period time in microseconds */
    uint16_t buffer_size;      /* Buffer size in frames */
    uint16_t period_size;      /* Period size in frames */
    uint8_t periods;           /* Number of periods */
    
    /* Channel mapping */
    audio_channel_t channel_map[MAX_CHANNELS];
    
} audio_format_desc_t;

/* Audio device capabilities */
typedef struct audio_device_caps {
    /* Supported formats */
    uint32_t formats;          /* Bitmask of supported formats */
    uint32_t min_sample_rate;  /* Minimum sample rate */
    uint32_t max_sample_rate;  /* Maximum sample rate */
    uint8_t min_channels;      /* Minimum channels */
    uint8_t max_channels;      /* Maximum channels */
    
    /* Buffer capabilities */
    uint32_t min_buffer_size;  /* Minimum buffer size */
    uint32_t max_buffer_size;  /* Maximum buffer size */
    uint32_t min_period_size;  /* Minimum period size */
    uint32_t max_period_size;  /* Maximum period size */
    uint8_t min_periods;       /* Minimum periods */
    uint8_t max_periods;       /* Maximum periods */
    
    /* Hardware features */
    bool hardware_mixing;      /* Hardware mixer support */
    bool hardware_effects;     /* Hardware effects support */
    bool sample_rate_conversion; /* Hardware SRC */
    bool bit_depth_conversion; /* Hardware bit depth conversion */
    bool channel_conversion;   /* Hardware channel conversion */
    bool synchronization;      /* Hardware sync support */
    bool low_latency;         /* Low latency capability */
    bool professional_io;     /* Professional I/O features */
    
    /* Volume control */
    bool volume_control;       /* Hardware volume control */
    bool mute_control;        /* Hardware mute control */
    uint32_t volume_steps;    /* Number of volume steps */
    
    /* Spatial audio */
    bool spatial_audio;       /* Spatial audio support */
    bool surround_sound;      /* Surround sound support */
    bool binaural_rendering;  /* Binaural audio rendering */
    bool room_correction;     /* Room correction support */
    
} audio_device_caps_t;

/* DSP effect types */
typedef enum {
    AUDIO_EFFECT_NONE = 0,
    AUDIO_EFFECT_REVERB,
    AUDIO_EFFECT_ECHO,
    AUDIO_EFFECT_CHORUS,
    AUDIO_EFFECT_FLANGER,
    AUDIO_EFFECT_PHASER,
    AUDIO_EFFECT_DISTORTION,
    AUDIO_EFFECT_COMPRESSOR,
    AUDIO_EFFECT_LIMITER,
    AUDIO_EFFECT_EXPANDER,
    AUDIO_EFFECT_GATE,
    AUDIO_EFFECT_EQUALIZER,
    AUDIO_EFFECT_FILTER_LOWPASS,
    AUDIO_EFFECT_FILTER_HIGHPASS,
    AUDIO_EFFECT_FILTER_BANDPASS,
    AUDIO_EFFECT_FILTER_NOTCH,
    AUDIO_EFFECT_PITCH_SHIFT,
    AUDIO_EFFECT_TIME_STRETCH,
    AUDIO_EFFECT_STEREO_WIDENER,
    AUDIO_EFFECT_3D_SPATIALIZER,
    AUDIO_EFFECT_NOISE_REDUCTION,
    AUDIO_EFFECT_AUTO_GAIN,
    AUDIO_EFFECT_MAX
} audio_effect_type_t;

/* DSP effect parameters */
typedef struct audio_effect {
    audio_effect_type_t type;
    bool enabled;
    uint32_t effect_id;
    
    /* Effect parameters */
    union {
        /* Reverb */
        struct {
            float room_size;
            float damping;
            float wet_level;
            float dry_level;
            float pre_delay_ms;
        } reverb;
        
        /* Echo */
        struct {
            float delay_ms;
            float feedback;
            float mix;
        } echo;
        
        /* Equalizer */
        struct {
            uint8_t band_count;
            struct {
                float frequency;
                float gain_db;
                float q_factor;
            } bands[32];
        } equalizer;
        
        /* Compressor */
        struct {
            float threshold_db;
            float ratio;
            float attack_ms;
            float release_ms;
            float knee_width;
            float makeup_gain_db;
        } compressor;
        
        /* 3D Spatializer */
        struct {
            float position_x;
            float position_y;
            float position_z;
            float velocity_x;
            float velocity_y;
            float velocity_z;
            float distance_attenuation;
            float doppler_factor;
        } spatializer;
        
        /* Generic parameters */
        float params[16];
        
    } params;
    
    /* Processing state */
    void *private_state;
    size_t state_size;
    
} audio_effect_t;

/* Audio mixer control */
typedef struct audio_mixer_control {
    uint32_t control_id;
    char name[64];
    
    enum {
        MIXER_CONTROL_VOLUME = 0,
        MIXER_CONTROL_MUTE,
        MIXER_CONTROL_SWITCH,
        MIXER_CONTROL_ENUM,
        MIXER_CONTROL_EQ_BAND,
        MIXER_CONTROL_MAX
    } type;
    
    /* Value ranges */
    int32_t min_value;
    int32_t max_value;
    int32_t step;
    int32_t current_value;
    
    /* Control flags */
    bool readable;
    bool writable;
    bool volatile_value;
    bool inactive;
    
    /* Enumerated values (for enum controls) */
    char enum_names[16][32];
    uint8_t enum_count;
    
} audio_mixer_control_t;

/* Audio stream configuration */
typedef struct audio_stream_config {
    audio_format_desc_t format;
    
    /* Stream properties */
    uint32_t stream_flags;
    uint32_t latency_us;       /* Desired latency in microseconds */
    
    /* Buffer configuration */
    uint32_t buffer_count;
    size_t buffer_size;
    
    /* Effects chain */
    audio_effect_t effects[MAX_AUDIO_EFFECTS];
    uint8_t effect_count;
    
    /* Routing */
    uint32_t input_device_id;  /* For capture streams */
    uint32_t output_device_id; /* For playback streams */
    
} audio_stream_config_t;

/* Audio stream */
typedef struct audio_stream {
    uint32_t stream_id;
    char name[64];
    audio_device_type_t direction;
    audio_stream_state_t state;
    
    /* Configuration */
    audio_stream_config_t config;
    
    /* Associated device */
    struct audio_device *device;
    
    /* Buffer management */
    struct {
        audio_buffer_t *buffers;
        uint32_t count;
        uint32_t current_buffer;
        spinlock_t lock;
        
        /* Ring buffer for streaming */
        void *ring_buffer;
        size_t ring_size;
        volatile uint32_t write_ptr;
        volatile uint32_t read_ptr;
        
    } buffers;
    
    /* Timing and synchronization */
    struct {
        uint64_t start_time;
        uint64_t frames_processed;
        uint32_t sample_rate;
        uint32_t xruns;           /* Underrun/overrun count */
        uint64_t last_interrupt_time;
        
        /* Synchronization objects */
        wait_queue_head_t wait_queue;
        struct completion completion;
        
    } timing;
    
    /* Performance monitoring */
    struct {
        uint64_t bytes_transferred;
        uint32_t interrupts;
        uint32_t buffer_fills;
        uint32_t buffer_drains;
        float cpu_usage_percent;
        uint64_t processing_time_ns;
    } stats;
    
    /* Stream operations */
    const struct audio_stream_ops *ops;
    
    /* Private driver data */
    void *private_data;
    
    struct list_head list;
    
} audio_stream_t;

/* Audio device structure */
typedef struct audio_device {
    uint32_t device_id;
    char name[64];
    char manufacturer[32];
    char driver_name[32];
    
    audio_device_type_t type;
    audio_device_caps_t caps;
    
    /* Hardware information */
    struct device *dev;
    uint16_t vendor_id;
    uint16_t product_id;
    
    /* Device state */
    bool enabled;
    bool suspended;
    uint32_t reference_count;
    
    /* Streams */
    struct {
        audio_stream_t *playback_streams[MAX_AUDIO_STREAMS];
        audio_stream_t *capture_streams[MAX_AUDIO_STREAMS];
        uint32_t playback_count;
        uint32_t capture_count;
        rwlock_t lock;
    } streams;
    
    /* Mixer controls */
    struct {
        audio_mixer_control_t controls[64];
        uint32_t count;
        rwlock_t lock;
    } mixer;
    
    /* Clock and timing */
    struct {
        uint32_t master_clock_rate;
        bool external_clock;
        bool sync_source;
        uint64_t clock_offset_ns;
    } clock;
    
    /* Power management */
    struct {
        uint32_t idle_timeout_ms;
        bool runtime_pm_enabled;
        uint32_t current_power_state;
    } power;
    
    /* Device operations */
    const struct audio_device_ops *ops;
    
    /* Statistics */
    struct {
        uint64_t streams_opened;
        uint64_t streams_closed;
        uint64_t bytes_processed;
        uint32_t errors;
        uint64_t uptime_ms;
    } stats;
    
    /* Private driver data */
    void *private_data;
    
    struct list_head list;
    
} audio_device_t;

/* Audio device operations */
struct audio_device_ops {
    /* Device lifecycle */
    int (*probe)(struct audio_device *dev);
    void (*remove)(struct audio_device *dev);
    
    /* Stream management */
    int (*open_stream)(struct audio_device *dev, struct audio_stream *stream);
    int (*close_stream)(struct audio_device *dev, struct audio_stream *stream);
    int (*prepare_stream)(struct audio_device *dev, struct audio_stream *stream);
    
    /* Stream control */
    int (*start_stream)(struct audio_device *dev, struct audio_stream *stream);
    int (*stop_stream)(struct audio_device *dev, struct audio_stream *stream);
    int (*pause_stream)(struct audio_device *dev, struct audio_stream *stream);
    int (*resume_stream)(struct audio_device *dev, struct audio_stream *stream);
    
    /* Buffer management */
    int (*get_buffer)(struct audio_device *dev, struct audio_stream *stream, struct audio_buffer **buffer);
    int (*put_buffer)(struct audio_device *dev, struct audio_stream *stream, struct audio_buffer *buffer);
    
    /* Mixer control */
    int (*get_mixer_control)(struct audio_device *dev, uint32_t control_id, struct audio_mixer_control *control);
    int (*set_mixer_control)(struct audio_device *dev, uint32_t control_id, const struct audio_mixer_control *control);
    
    /* Clock and timing */
    uint64_t (*get_timestamp)(struct audio_device *dev);
    int (*set_sample_rate)(struct audio_device *dev, uint32_t sample_rate);
    
    /* Power management */
    int (*suspend)(struct audio_device *dev);
    int (*resume)(struct audio_device *dev);
    
    /* Effects and DSP */
    int (*apply_effect)(struct audio_device *dev, struct audio_stream *stream, const struct audio_effect *effect);
    int (*remove_effect)(struct audio_device *dev, struct audio_stream *stream, uint32_t effect_id);
    
};

/* Audio stream operations */
struct audio_stream_ops {
    /* Stream lifecycle */
    int (*configure)(struct audio_stream *stream, const struct audio_stream_config *config);
    int (*prepare)(struct audio_stream *stream);
    void (*cleanup)(struct audio_stream *stream);
    
    /* Data transfer */
    ssize_t (*read)(struct audio_stream *stream, void *buffer, size_t size);
    ssize_t (*write)(struct audio_stream *stream, const void *buffer, size_t size);
    int (*mmap)(struct audio_stream *stream, struct vm_area_struct *vma);
    
    /* Position and timing */
    uint64_t (*get_position)(struct audio_stream *stream);
    int (*get_delay)(struct audio_stream *stream, uint32_t *delay_frames);
    
    /* Synchronization */
    int (*sync)(struct audio_stream *stream);
    int (*drop)(struct audio_stream *stream);
    
};

/* Global audio subsystem */
typedef struct audio_subsystem {
    bool initialized;
    
    /* Device registry */
    struct {
        audio_device_t *devices[MAX_AUDIO_DEVICES];
        uint32_t count;
        uint32_t next_device_id;
        rwlock_t lock;
    } devices;
    
    /* Stream management */
    struct {
        audio_stream_t *streams[MAX_AUDIO_STREAMS];
        uint32_t count;
        uint32_t next_stream_id;
        rwlock_t lock;
    } streams;
    
    /* Global mixer */
    struct {
        float master_volume;
        bool master_mute;
        audio_mixer_control_t controls[128];
        uint32_t control_count;
    } mixer;
    
    /* DSP engine */
    struct {
        bool enabled;
        struct workqueue_struct *dsp_workqueue;
        audio_effect_t global_effects[MAX_AUDIO_EFFECTS];
        uint32_t effect_count;
    } dsp;
    
    /* Global configuration */
    struct {
        uint32_t default_sample_rate;
        audio_format_t default_format;
        uint32_t default_buffer_size;
        bool low_latency_mode;
        bool professional_mode;
        bool spatial_audio_enabled;
    } config;
    
    /* Statistics */
    struct {
        uint64_t total_streams;
        uint64_t active_streams;
        uint64_t bytes_processed;
        float cpu_usage_percent;
        uint32_t xrun_count;
    } stats;
    
} audio_subsystem_t;

/* External audio subsystem */
extern audio_subsystem_t audio_subsystem;

/* Core audio functions */
int audio_init(void);
void audio_exit(void);

/* Device management */
int audio_register_device(struct audio_device *dev);
void audio_unregister_device(struct audio_device *dev);
struct audio_device *audio_get_device_by_id(uint32_t device_id);
struct audio_device *audio_find_device_by_name(const char *name);
struct audio_device *audio_get_default_device(audio_device_type_t type);

/* Stream management */
struct audio_stream *audio_create_stream(const struct audio_stream_config *config);
void audio_destroy_stream(struct audio_stream *stream);
int audio_open_stream(struct audio_stream *stream);
int audio_close_stream(struct audio_stream *stream);
int audio_start_stream(struct audio_stream *stream);
int audio_stop_stream(struct audio_stream *stream);
int audio_pause_stream(struct audio_stream *stream);
int audio_resume_stream(struct audio_stream *stream);

/* Buffer management */
struct audio_buffer *audio_get_buffer(struct audio_stream *stream);
void audio_put_buffer(struct audio_stream *stream, struct audio_buffer *buffer);
int audio_queue_buffer(struct audio_stream *stream, const void *data, size_t size);
int audio_dequeue_buffer(struct audio_stream *stream, void *data, size_t size);

/* Data transfer */
ssize_t audio_read(struct audio_stream *stream, void *buffer, size_t size);
ssize_t audio_write(struct audio_stream *stream, const void *buffer, size_t size);
ssize_t audio_read_nonblocking(struct audio_stream *stream, void *buffer, size_t size);
ssize_t audio_write_nonblocking(struct audio_stream *stream, const void *buffer, size_t size);

/* Mixer control */
int audio_get_volume(struct audio_device *dev, uint32_t channel, float *volume);
int audio_set_volume(struct audio_device *dev, uint32_t channel, float volume);
int audio_get_mute(struct audio_device *dev, uint32_t channel, bool *mute);
int audio_set_mute(struct audio_device *dev, uint32_t channel, bool mute);
int audio_get_mixer_control(struct audio_device *dev, const char *name, struct audio_mixer_control *control);
int audio_set_mixer_control(struct audio_device *dev, const char *name, const struct audio_mixer_control *control);

/* Effects and DSP */
int audio_add_effect(struct audio_stream *stream, const struct audio_effect *effect);
int audio_remove_effect(struct audio_stream *stream, uint32_t effect_id);
int audio_update_effect(struct audio_stream *stream, uint32_t effect_id, const struct audio_effect *effect);
int audio_enable_effect(struct audio_stream *stream, uint32_t effect_id, bool enable);

/* Format conversion */
int audio_convert_format(const void *src_data, audio_format_t src_format,
                        void *dst_data, audio_format_t dst_format,
                        uint32_t frames, uint8_t channels);
int audio_resample(const void *src_data, uint32_t src_rate,
                  void *dst_data, uint32_t dst_rate,
                  uint32_t src_frames, uint32_t *dst_frames,
                  uint8_t channels, audio_format_t format);

/* Spatial audio */
int audio_enable_spatial_audio(struct audio_stream *stream, bool enable);
int audio_set_listener_position(float x, float y, float z);
int audio_set_listener_orientation(float forward_x, float forward_y, float forward_z,
                                  float up_x, float up_y, float up_z);
int audio_set_source_position(struct audio_stream *stream, float x, float y, float z);

/* Timing and synchronization */
uint64_t audio_get_timestamp(void);
uint64_t audio_get_stream_position(struct audio_stream *stream);
int audio_get_latency(struct audio_stream *stream, uint32_t *latency_us);
int audio_sync_streams(struct audio_stream **streams, uint32_t count);

/* Configuration */
int audio_set_global_config(const struct audio_subsystem *config);
int audio_get_global_config(struct audio_subsystem *config);
int audio_set_device_config(struct audio_device *dev, const void *config, size_t size);

/* Power management */
int audio_device_suspend(struct audio_device *dev);
int audio_device_resume(struct audio_device *dev);
void audio_set_power_policy(uint32_t policy);

/* Statistics and monitoring */
void audio_print_statistics(void);
void audio_print_device_info(struct audio_device *dev);
void audio_print_stream_info(struct audio_stream *stream);
int audio_get_performance_stats(struct audio_device *dev, void *stats, size_t size);

/* Utility functions */
const char *audio_format_name(audio_format_t format);
const char *audio_device_type_name(audio_device_type_t type);
const char *audio_stream_state_name(audio_stream_state_t state);
uint32_t audio_format_frame_size(audio_format_t format, uint8_t channels);
uint32_t audio_bytes_to_frames(uint32_t bytes, audio_format_t format, uint8_t channels);
uint32_t audio_frames_to_bytes(uint32_t frames, audio_format_t format, uint8_t channels);
uint64_t audio_frames_to_time_us(uint64_t frames, uint32_t sample_rate);
uint64_t audio_time_us_to_frames(uint64_t time_us, uint32_t sample_rate);

/* Advanced features */
int audio_enable_low_latency_mode(bool enable);
int audio_set_realtime_priority(struct audio_stream *stream, int priority);
int audio_register_notification_callback(void (*callback)(uint32_t event, void *data));
int audio_calibrate_device(struct audio_device *dev);
int audio_test_device(struct audio_device *dev, uint32_t test_type);

/* Hot-plug support */
void audio_device_hotplug_notify(struct audio_device *dev, bool connected);
int audio_scan_for_devices(void);