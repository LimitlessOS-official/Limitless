/*
 * LimitlessOS Multimedia & Gaming Framework
 * Hardware-accelerated media codecs, gaming support, VR/AR, and streaming
 */

#ifndef MULTIMEDIA_GAMING_H
#define MULTIMEDIA_GAMING_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

/* Audio formats */
typedef enum {
    AUDIO_FORMAT_PCM_S16LE,     /* 16-bit signed little-endian PCM */
    AUDIO_FORMAT_PCM_S24LE,     /* 24-bit signed little-endian PCM */
    AUDIO_FORMAT_PCM_S32LE,     /* 32-bit signed little-endian PCM */
    AUDIO_FORMAT_PCM_F32LE,     /* 32-bit float little-endian PCM */
    AUDIO_FORMAT_MP3,           /* MPEG Audio Layer 3 */
    AUDIO_FORMAT_AAC,           /* Advanced Audio Coding */
    AUDIO_FORMAT_FLAC,          /* Free Lossless Audio Codec */
    AUDIO_FORMAT_OGG_VORBIS,    /* Ogg Vorbis */
    AUDIO_FORMAT_OPUS,          /* Opus */
    AUDIO_FORMAT_DTS,           /* DTS */
    AUDIO_FORMAT_DOLBY_ATMOS,   /* Dolby Atmos */
    AUDIO_FORMAT_MAX
} audio_format_t;

/* Video formats */
typedef enum {
    VIDEO_FORMAT_H264,          /* H.264/AVC */
    VIDEO_FORMAT_H265_HEVC,     /* H.265/HEVC */
    VIDEO_FORMAT_AV1,           /* AV1 */
    VIDEO_FORMAT_VP9,           /* VP9 */
    VIDEO_FORMAT_VP8,           /* VP8 */
    VIDEO_FORMAT_MPEG2,         /* MPEG-2 */
    VIDEO_FORMAT_MPEG4,         /* MPEG-4 */
    VIDEO_FORMAT_XVID,          /* Xvid */
    VIDEO_FORMAT_THEORA,        /* Theora */
    VIDEO_FORMAT_MJPEG,         /* Motion JPEG */
    VIDEO_FORMAT_RAW,           /* Raw/Uncompressed */
    VIDEO_FORMAT_MAX
} video_format_t;

/* Container formats */
typedef enum {
    CONTAINER_MP4,              /* MP4 */
    CONTAINER_MKV,              /* Matroska */
    CONTAINER_AVI,              /* Audio Video Interleave */
    CONTAINER_MOV,              /* QuickTime */
    CONTAINER_WEBM,             /* WebM */
    CONTAINER_FLV,              /* Flash Video */
    CONTAINER_TS,               /* Transport Stream */
    CONTAINER_M3U8,             /* HTTP Live Streaming */
    CONTAINER_DASH,             /* Dynamic Adaptive Streaming */
    CONTAINER_OGG,              /* Ogg */
    CONTAINER_MAX
} container_format_t;

/* Hardware acceleration types */
typedef enum {
    HWACCEL_NONE,               /* Software only */
    HWACCEL_VAAPI,              /* Video Acceleration API (Intel) */
    HWACCEL_NVENC_NVDEC,        /* NVIDIA Video Codec SDK */
    HWACCEL_AMF,                /* AMD Advanced Media Framework */
    HWACCEL_QUICKSYNC,          /* Intel Quick Sync Video */
    HWACCEL_VIDEOTOOLBOX,       /* Apple VideoToolbox */
    HWACCEL_OPENCL,             /* OpenCL acceleration */
    HWACCEL_VULKAN,             /* Vulkan acceleration */
    HWACCEL_MAX
} hwaccel_type_t;

/* Gaming controller types */
typedef enum {
    CONTROLLER_UNKNOWN,
    CONTROLLER_XBOX_360,
    CONTROLLER_XBOX_ONE,
    CONTROLLER_XBOX_SERIES,
    CONTROLLER_PS3,
    CONTROLLER_PS4,
    CONTROLLER_PS5_DUALSENSE,
    CONTROLLER_NINTENDO_SWITCH_PRO,
    CONTROLLER_STEAM_CONTROLLER,
    CONTROLLER_GENERIC_HID,
    CONTROLLER_MAX
} controller_type_t;

/* VR/AR headset types */
typedef enum {
    HMD_UNKNOWN,
    HMD_OCULUS_RIFT,
    HMD_OCULUS_QUEST,
    HMD_HTC_VIVE,
    HMD_VALVE_INDEX,
    HMD_PICO,
    HMD_VARJO,
    HMD_MICROSOFT_HOLOLENS,
    HMD_MAGIC_LEAP,
    HMD_APPLE_VISION_PRO,
    HMD_MAX
} hmd_type_t;

/* Streaming protocols */
typedef enum {
    STREAM_PROTOCOL_RTMP,       /* Real-Time Messaging Protocol */
    STREAM_PROTOCOL_RTSP,       /* Real Time Streaming Protocol */
    STREAM_PROTOCOL_HLS,        /* HTTP Live Streaming */
    STREAM_PROTOCOL_DASH,       /* Dynamic Adaptive Streaming */
    STREAM_PROTOCOL_WEBRTC,     /* Web Real-Time Communication */
    STREAM_PROTOCOL_SRT,        /* Secure Reliable Transport */
    STREAM_PROTOCOL_NDI,        /* Network Device Interface */
    STREAM_PROTOCOL_MAX
} stream_protocol_t;

/* Media codec configuration */
typedef struct {
    char name[64];              /* Codec name */
    bool hardware_accelerated;  /* Hardware acceleration available */
    hwaccel_type_t accel_type;  /* Acceleration type */
    
    /* Encoding capabilities */
    bool encode_supported;
    uint32_t max_encode_width;
    uint32_t max_encode_height;
    uint32_t max_encode_fps;
    uint64_t max_encode_bitrate;
    
    /* Decoding capabilities */
    bool decode_supported;
    uint32_t max_decode_width;
    uint32_t max_decode_height;
    uint32_t max_decode_fps;
    
    /* Profile support */
    char profiles[16][32];      /* Supported profiles */
    uint32_t profile_count;
    
    /* Quality settings */
    struct {
        uint32_t min_quality;   /* Minimum quality level */
        uint32_t max_quality;   /* Maximum quality level */
        uint32_t default_quality; /* Default quality level */
        bool variable_bitrate;  /* Variable bitrate support */
        bool constant_quality;  /* Constant quality mode */
    } quality;
} media_codec_t;

/* Audio device configuration */
typedef struct {
    char name[64];              /* Device name */
    char driver[32];            /* Audio driver */
    bool default_device;        /* Is default device */
    
    /* Capabilities */
    audio_format_t formats[AUDIO_FORMAT_MAX]; /* Supported formats */
    uint32_t format_count;
    uint32_t sample_rates[16];  /* Supported sample rates */
    uint32_t sample_rate_count;
    uint32_t channels_min;      /* Minimum channels */
    uint32_t channels_max;      /* Maximum channels */
    
    /* Spatial audio */
    bool spatial_audio;         /* Spatial audio support */
    bool surround_sound;        /* Surround sound support */
    bool binaural_rendering;    /* Binaural rendering */
    
    /* Hardware features */
    bool hardware_mixing;       /* Hardware audio mixing */
    bool dsp_effects;          /* DSP effects processing */
    bool low_latency;          /* Low-latency mode */
    uint32_t buffer_size_min;   /* Minimum buffer size */
    uint32_t buffer_size_max;   /* Maximum buffer size */
    
    /* Current configuration */
    audio_format_t current_format;
    uint32_t current_sample_rate;
    uint32_t current_channels;
    uint32_t current_buffer_size;
    bool currently_active;
} audio_device_t;

/* Gaming controller configuration */
typedef struct {
    char name[64];              /* Controller name */
    controller_type_t type;     /* Controller type */
    char device_path[256];      /* Device path */
    uint16_t vendor_id;         /* USB vendor ID */
    uint16_t product_id;        /* USB product ID */
    
    /* Capabilities */
    struct {
        uint32_t button_count;   /* Number of buttons */
        uint32_t axis_count;     /* Number of analog axes */
        bool has_dpad;          /* D-pad support */
        bool has_triggers;      /* Analog triggers */
        bool has_touchpad;      /* Touchpad support */
        bool has_motion;        /* Motion sensors (gyro/accel) */
        bool has_haptic;        /* Haptic feedback */
        bool has_adaptive_triggers; /* Adaptive triggers (PS5) */
        bool wireless;          /* Wireless connection */
        bool bluetooth;         /* Bluetooth support */
    } capabilities;
    
    /* Configuration */
    struct {
        double deadzone;        /* Analog stick deadzone */
        double sensitivity;     /* Input sensitivity */
        bool invert_y;         /* Invert Y-axis */
        uint32_t button_map[32]; /* Button mapping */
        double axis_map[16];    /* Axis mapping */
    } config;
    
    /* Current state */
    struct {
        bool connected;
        uint32_t buttons;       /* Button state bitfield */
        int16_t axes[16];       /* Axis values */
        uint8_t battery_level;  /* Battery level (0-100) */
        bool charging;          /* Charging status */
    } state;
} gaming_controller_t;

/* VR/AR headset configuration */
typedef struct {
    char name[64];              /* Headset name */
    hmd_type_t type;           /* Headset type */
    char serial_number[64];     /* Serial number */
    
    /* Display specifications */
    struct {
        uint32_t resolution_x;   /* Per-eye horizontal resolution */
        uint32_t resolution_y;   /* Per-eye vertical resolution */
        uint32_t refresh_rate;   /* Display refresh rate */
        double fov_horizontal;   /* Horizontal field of view */
        double fov_vertical;     /* Vertical field of view */
        double ipd_min;         /* Minimum IPD (mm) */
        double ipd_max;         /* Maximum IPD (mm) */
    } display;
    
    /* Tracking capabilities */
    struct {
        bool head_tracking;     /* 6DOF head tracking */
        bool controller_tracking; /* Controller tracking */
        bool hand_tracking;     /* Hand/finger tracking */
        bool eye_tracking;      /* Eye tracking */
        bool inside_out;        /* Inside-out tracking */
        bool outside_in;        /* Outside-in tracking */
        uint32_t tracking_cameras; /* Number of tracking cameras */
    } tracking;
    
    /* Features */
    struct {
        bool passthrough;       /* AR passthrough */
        bool adjustment_lenses; /* IPD adjustment */
        bool audio_builtin;     /* Built-in audio */
        bool microphone;        /* Built-in microphone */
        bool haptic_feedback;   /* Haptic feedback */
    } features;
    
    /* Current state */
    struct {
        bool connected;
        bool displaying;        /* Currently displaying content */
        double head_pose[7];    /* Position (3) + Quaternion (4) */
        double controller_poses[2][7]; /* Left/Right controller poses */
        bool controllers_connected[2]; /* Controller connection status */
    } state;
} vr_headset_t;

/* Streaming configuration */
typedef struct {
    char name[64];              /* Stream name */
    stream_protocol_t protocol; /* Streaming protocol */
    char url[512];              /* Stream URL/endpoint */
    
    /* Video settings */
    struct {
        video_format_t codec;   /* Video codec */
        uint32_t width;         /* Video width */
        uint32_t height;        /* Video height */
        uint32_t fps;           /* Frames per second */
        uint64_t bitrate;       /* Video bitrate */
        uint32_t keyframe_interval; /* Keyframe interval */
        bool hardware_encode;   /* Hardware encoding */
    } video;
    
    /* Audio settings */
    struct {
        audio_format_t codec;   /* Audio codec */
        uint32_t sample_rate;   /* Sample rate */
        uint32_t channels;      /* Channel count */
        uint64_t bitrate;       /* Audio bitrate */
    } audio;
    
    /* Network settings */
    struct {
        uint32_t buffer_size;   /* Stream buffer size */
        uint32_t max_bitrate;   /* Maximum bitrate */
        bool adaptive_bitrate;  /* Adaptive bitrate streaming */
        uint32_t segment_duration; /* Segment duration (seconds) */
    } network;
    
    /* Status */
    struct {
        bool active;            /* Currently streaming */
        uint64_t bytes_sent;    /* Total bytes sent */
        uint64_t frames_sent;   /* Total frames sent */
        double current_fps;     /* Current FPS */
        uint32_t dropped_frames; /* Dropped frames */
        uint32_t network_errors; /* Network errors */
    } status;
} streaming_config_t;

/* Multimedia system */
typedef struct {
    bool initialized;
    
    /* Media codecs */
    media_codec_t video_codecs[VIDEO_FORMAT_MAX];
    media_codec_t audio_codecs[AUDIO_FORMAT_MAX];
    uint32_t video_codec_count;
    uint32_t audio_codec_count;
    
    /* Audio system */
    struct {
        audio_device_t devices[16]; /* Up to 16 audio devices */
        uint32_t device_count;
        char default_output[64];    /* Default output device */
        char default_input[64];     /* Default input device */
        
        /* Audio processing */
        bool spatial_audio_enabled; /* Global spatial audio */
        bool noise_cancellation;    /* Noise cancellation */
        bool echo_cancellation;     /* Echo cancellation */
        double master_volume;       /* Master volume (0.0-1.0) */
        bool muted;                /* Master mute */
        
        /* Audio effects */
        struct {
            bool equalizer;         /* Graphic equalizer */
            bool compressor;        /* Dynamic range compressor */
            bool reverb;           /* Reverb effect */
            bool bass_boost;       /* Bass enhancement */
            double effects_params[16]; /* Effect parameters */
        } effects;
    } audio;
    
    /* Gaming system */
    struct {
        gaming_controller_t controllers[8]; /* Up to 8 controllers */
        uint32_t controller_count;
        
        /* Gaming features */
        bool game_mode;            /* Gaming optimization mode */
        bool vsync_adaptive;       /* Adaptive VSync */
        bool frame_pacing;         /* Frame pacing */
        uint32_t target_fps;       /* Target FPS */
        
        /* Performance monitoring */
        struct {
            double current_fps;     /* Current FPS */
            double avg_fps;         /* Average FPS */
            double frame_time_ms;   /* Frame time in milliseconds */
            uint32_t frame_drops;   /* Dropped frames */
            double gpu_utilization; /* GPU utilization */
            double cpu_utilization; /* CPU utilization */
            uint64_t vram_usage;    /* VRAM usage in bytes */
        } performance;
    } gaming;
    
    /* VR/AR system */
    struct {
        vr_headset_t headsets[4];   /* Up to 4 headsets */
        uint32_t headset_count;
        bool vr_runtime_active;     /* VR runtime active */
        
        /* VR features */
        bool room_scale;           /* Room-scale tracking */
        bool hand_tracking;        /* Hand tracking enabled */
        bool eye_tracking;         /* Eye tracking enabled */
        double play_area[4][2];    /* Play area boundaries */
        
        /* Performance settings */
        bool foveated_rendering;   /* Foveated rendering */
        bool reprojection;         /* Asynchronous reprojection */
        uint32_t supersampling;    /* Supersampling multiplier */
    } vr;
    
    /* Streaming system */
    struct {
        streaming_config_t streams[16]; /* Up to 16 streams */
        uint32_t stream_count;
        bool hardware_encoding;    /* Hardware encoding available */
        
        /* Streaming servers */
        struct {
            bool rtmp_server;      /* RTMP server */
            bool webrtc_server;    /* WebRTC server */
            bool ndi_server;       /* NDI server */
            uint16_t rtmp_port;    /* RTMP port */
            uint16_t webrtc_port;  /* WebRTC port */
        } servers;
    } streaming;
    
    /* Statistics */
    struct {
        uint64_t videos_decoded;
        uint64_t videos_encoded;
        uint64_t audio_samples_processed;
        uint64_t controller_inputs;
        uint64_t vr_frames_rendered;
        uint64_t streams_served;
        double total_encoding_time;
        double total_decoding_time;
    } stats;
} multimedia_system_t;

/* Function prototypes */

/* Multimedia system initialization */
int multimedia_init(void);
void multimedia_exit(void);
int multimedia_detect_hardware(void);

/* Media codec management */
int codec_register(const media_codec_t *codec);
int codec_unregister(const char *name);
media_codec_t *codec_find(const char *name);
int codec_list(char codec_names[][64], int max_codecs);
bool codec_supports_hardware_accel(const char *name);

/* Video processing */
int video_decode_init(const char *codec_name, uint32_t width, uint32_t height);
int video_decode_frame(const uint8_t *input_data, size_t input_size, 
                      uint8_t *output_data, size_t output_size);
int video_encode_init(const char *codec_name, uint32_t width, uint32_t height, 
                     uint32_t fps, uint64_t bitrate);
int video_encode_frame(const uint8_t *input_data, size_t input_size, 
                      uint8_t *output_data, size_t *output_size);

/* Audio system */
int audio_system_init(void);
int audio_device_enumerate(void);
int audio_device_set_default(const char *device_name, bool output);
int audio_play_buffer(const char *device_name, const void *buffer, 
                     size_t size, audio_format_t format);
int audio_record_buffer(const char *device_name, void *buffer, 
                       size_t size, audio_format_t format);
int audio_set_volume(const char *device_name, double volume);
int audio_set_mute(const char *device_name, bool muted);

/* Spatial audio */
int spatial_audio_init(void);
int spatial_audio_set_listener_position(double x, double y, double z, 
                                       double orientation[4]);
int spatial_audio_create_source(uint32_t *source_id, double x, double y, double z);
int spatial_audio_update_source(uint32_t source_id, double x, double y, double z);
int spatial_audio_play_source(uint32_t source_id, const void *buffer, 
                             size_t size, audio_format_t format);

/* Gaming system */
int gaming_init(void);
int controller_enumerate(void);
int controller_open(const char *device_path);
int controller_close(const char *device_path);
int controller_get_state(const char *device_path, gaming_controller_t *state);
int controller_set_vibration(const char *device_path, double low_freq, double high_freq);
int controller_set_led(const char *device_path, uint8_t r, uint8_t g, uint8_t b);

/* Game mode optimization */
int game_mode_enable(void);
int game_mode_disable(void);
int game_mode_set_target_fps(uint32_t fps);
int game_mode_get_performance(void *performance_data);

/* VR/AR system */
int vr_system_init(void);
int vr_headset_enumerate(void);
int vr_headset_connect(const char *serial_number);
int vr_headset_disconnect(const char *serial_number);
int vr_headset_get_pose(const char *serial_number, double pose[7]);
int vr_headset_submit_frame(const char *serial_number, const void *left_eye, 
                           const void *right_eye, uint32_t width, uint32_t height);
int vr_set_play_area(double boundaries[4][2]);
int vr_enable_hand_tracking(bool enable);
int vr_enable_eye_tracking(bool enable);

/* Streaming system */
int streaming_init(void);
int stream_create(const streaming_config_t *config);
int stream_start(const char *stream_name);
int stream_stop(const char *stream_name);
int stream_destroy(const char *stream_name);
int stream_get_status(const char *stream_name, streaming_config_t *status);

/* RTMP streaming */
int rtmp_server_start(uint16_t port);
int rtmp_server_stop(void);
int rtmp_stream_publish(const char *stream_key, const streaming_config_t *config);

/* WebRTC streaming */
int webrtc_server_start(uint16_t port);
int webrtc_server_stop(void);
int webrtc_create_session(const char *session_id);
int webrtc_add_stream(const char *session_id, const streaming_config_t *config);

/* Hardware acceleration */
int hwaccel_detect(void);
int hwaccel_enable(hwaccel_type_t type);
int hwaccel_disable(hwaccel_type_t type);
bool hwaccel_is_available(hwaccel_type_t type);

/* Utility functions */
const char *audio_format_name(audio_format_t format);
const char *video_format_name(video_format_t format);
const char *container_format_name(container_format_t format);
const char *controller_type_name(controller_type_t type);
const char *hmd_type_name(hmd_type_t type);
const char *stream_protocol_name(stream_protocol_t protocol);
const char *hwaccel_type_name(hwaccel_type_t type);

/* Global multimedia system */
extern multimedia_system_t multimedia_system;

#endif /* MULTIMEDIA_GAMING_H */