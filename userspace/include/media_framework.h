/**
 * LimitlessOS Universal Media Framework
 * Enterprise-grade multimedia system with hardware acceleration
 */

#ifndef LIMITLESS_MEDIA_FRAMEWORK_H
#define LIMITLESS_MEDIA_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>
#include "graphics_enterprise.h"

/* Media Framework Version */
#define LIMITLESS_MEDIA_VERSION_MAJOR 2
#define LIMITLESS_MEDIA_VERSION_MINOR 0

/* Maximum limits */
#define MAX_MEDIA_DECODERS          64
#define MAX_MEDIA_ENCODERS          64
#define MAX_AUDIO_DEVICES           32
#define MAX_VIDEO_DEVICES           16
#define MAX_MEDIA_STREAMS           512
#define MAX_AUDIO_CHANNELS          32
#define MAX_CODEC_PROFILES          256
#define MAX_MEDIA_SESSIONS          128

/* Audio sample rates */
#define AUDIO_SAMPLE_RATE_8KHZ      8000
#define AUDIO_SAMPLE_RATE_16KHZ     16000
#define AUDIO_SAMPLE_RATE_22KHZ     22050
#define AUDIO_SAMPLE_RATE_44KHZ     44100
#define AUDIO_SAMPLE_RATE_48KHZ     48000
#define AUDIO_SAMPLE_RATE_88KHZ     88200
#define AUDIO_SAMPLE_RATE_96KHZ     96000
#define AUDIO_SAMPLE_RATE_176KHZ    176400
#define AUDIO_SAMPLE_RATE_192KHZ    192000
#define AUDIO_SAMPLE_RATE_384KHZ    384000

/* Video frame rates (in fps * 1000) */
#define VIDEO_FPS_23976             23976   /* 23.976 fps (film) */
#define VIDEO_FPS_24                24000   /* 24 fps (cinema) */
#define VIDEO_FPS_25                25000   /* 25 fps (PAL) */
#define VIDEO_FPS_29970             29970   /* 29.97 fps (NTSC) */
#define VIDEO_FPS_30                30000   /* 30 fps */
#define VIDEO_FPS_50                50000   /* 50 fps (PAL) */
#define VIDEO_FPS_59940             59940   /* 59.94 fps (NTSC) */
#define VIDEO_FPS_60                60000   /* 60 fps */
#define VIDEO_FPS_120               120000  /* 120 fps */
#define VIDEO_FPS_240               240000  /* 240 fps */

/* Media Types */
typedef enum {
    MEDIA_TYPE_UNKNOWN,
    MEDIA_TYPE_AUDIO,
    MEDIA_TYPE_VIDEO,
    MEDIA_TYPE_SUBTITLE,
    MEDIA_TYPE_DATA,
    MEDIA_TYPE_ATTACHMENT
} media_type_t;

/* Audio Formats */
typedef enum {
    AUDIO_FORMAT_UNKNOWN,
    AUDIO_FORMAT_PCM_S8,        /* 8-bit signed PCM */
    AUDIO_FORMAT_PCM_U8,        /* 8-bit unsigned PCM */
    AUDIO_FORMAT_PCM_S16LE,     /* 16-bit signed little-endian PCM */
    AUDIO_FORMAT_PCM_S16BE,     /* 16-bit signed big-endian PCM */
    AUDIO_FORMAT_PCM_S24LE,     /* 24-bit signed little-endian PCM */
    AUDIO_FORMAT_PCM_S24BE,     /* 24-bit signed big-endian PCM */
    AUDIO_FORMAT_PCM_S32LE,     /* 32-bit signed little-endian PCM */
    AUDIO_FORMAT_PCM_S32BE,     /* 32-bit signed big-endian PCM */
    AUDIO_FORMAT_PCM_F32LE,     /* 32-bit float little-endian */
    AUDIO_FORMAT_PCM_F32BE,     /* 32-bit float big-endian */
    AUDIO_FORMAT_PCM_F64LE,     /* 64-bit float little-endian */
    AUDIO_FORMAT_PCM_F64BE,     /* 64-bit float big-endian */
    AUDIO_FORMAT_MP3,           /* MPEG-1 Layer 3 */
    AUDIO_FORMAT_AAC,           /* Advanced Audio Coding */
    AUDIO_FORMAT_FLAC,          /* Free Lossless Audio Codec */
    AUDIO_FORMAT_VORBIS,        /* Ogg Vorbis */
    AUDIO_FORMAT_OPUS,          /* Opus */
    AUDIO_FORMAT_AC3,           /* Dolby Digital (AC-3) */
    AUDIO_FORMAT_EAC3,          /* Dolby Digital Plus (E-AC-3) */
    AUDIO_FORMAT_DTS,           /* DTS */
    AUDIO_FORMAT_TRUEHD,        /* Dolby TrueHD */
    AUDIO_FORMAT_DTSHD,         /* DTS-HD */
    AUDIO_FORMAT_PCM_BLURAY,    /* Blu-ray PCM */
    AUDIO_FORMAT_ATMOS,         /* Dolby Atmos */
    AUDIO_FORMAT_DTS_X          /* DTS:X */
} audio_format_t;

/* Video Formats */
typedef enum {
    VIDEO_FORMAT_UNKNOWN,
    VIDEO_FORMAT_YUV420P,       /* Planar YUV 4:2:0 */
    VIDEO_FORMAT_YUV422P,       /* Planar YUV 4:2:2 */
    VIDEO_FORMAT_YUV444P,       /* Planar YUV 4:4:4 */
    VIDEO_FORMAT_YUV410P,       /* Planar YUV 4:1:0 */
    VIDEO_FORMAT_YUV411P,       /* Planar YUV 4:1:1 */
    VIDEO_FORMAT_YUYV422,       /* Packed YUV 4:2:2 */
    VIDEO_FORMAT_UYVY422,       /* Packed YUV 4:2:2 */
    VIDEO_FORMAT_RGB24,         /* Packed RGB 8:8:8 */
    VIDEO_FORMAT_BGR24,         /* Packed BGR 8:8:8 */
    VIDEO_FORMAT_RGBA,          /* Packed RGBA 8:8:8:8 */
    VIDEO_FORMAT_BGRA,          /* Packed BGRA 8:8:8:8 */
    VIDEO_FORMAT_ARGB,          /* Packed ARGB 8:8:8:8 */
    VIDEO_FORMAT_ABGR,          /* Packed ABGR 8:8:8:8 */
    VIDEO_FORMAT_RGB565,        /* Packed RGB 5:6:5 */
    VIDEO_FORMAT_BGR565,        /* Packed BGR 5:6:5 */
    VIDEO_FORMAT_YUV420P10LE,   /* 10-bit YUV 4:2:0 */
    VIDEO_FORMAT_YUV422P10LE,   /* 10-bit YUV 4:2:2 */
    VIDEO_FORMAT_YUV444P10LE,   /* 10-bit YUV 4:4:4 */
    VIDEO_FORMAT_YUV420P12LE,   /* 12-bit YUV 4:2:0 */
    VIDEO_FORMAT_YUV422P12LE,   /* 12-bit YUV 4:2:2 */
    VIDEO_FORMAT_YUV444P12LE,   /* 12-bit YUV 4:4:4 */
    VIDEO_FORMAT_NV12,          /* Semi-planar YUV 4:2:0 */
    VIDEO_FORMAT_NV21,          /* Semi-planar YVU 4:2:0 */
} video_format_t;

/* Codec Types */
typedef enum {
    CODEC_TYPE_UNKNOWN,
    /* Audio Codecs */
    CODEC_TYPE_AAC,
    CODEC_TYPE_MP3,
    CODEC_TYPE_FLAC,
    CODEC_TYPE_VORBIS,
    CODEC_TYPE_OPUS,
    CODEC_TYPE_AC3,
    CODEC_TYPE_EAC3,
    CODEC_TYPE_DTS,
    CODEC_TYPE_TRUEHD,
    CODEC_TYPE_DTSHD,
    CODEC_TYPE_ATMOS,
    CODEC_TYPE_DTS_X,
    /* Video Codecs */
    CODEC_TYPE_H264,            /* AVC/H.264 */
    CODEC_TYPE_H265,            /* HEVC/H.265 */
    CODEC_TYPE_H266,            /* VVC/H.266 */
    CODEC_TYPE_VP8,             /* VP8 */
    CODEC_TYPE_VP9,             /* VP9 */
    CODEC_TYPE_AV1,             /* AV1 */
    CODEC_TYPE_MPEG2,           /* MPEG-2 */
    CODEC_TYPE_MPEG4,           /* MPEG-4 Part 2 */
    CODEC_TYPE_THEORA,          /* Theora */
    CODEC_TYPE_MJPEG,           /* Motion JPEG */
    CODEC_TYPE_PRORES,          /* Apple ProRes */
    CODEC_TYPE_DNXHD,           /* Avid DNxHD */
    CODEC_TYPE_CINEFORM,        /* GoPro CineForm */
    /* Image Codecs */
    CODEC_TYPE_JPEG,
    CODEC_TYPE_PNG,
    CODEC_TYPE_WEBP,
    CODEC_TYPE_AVIF,
    CODEC_TYPE_HEIF
} codec_type_t;

/* Hardware Acceleration Types */
typedef enum {
    HW_ACCEL_NONE,
    HW_ACCEL_DXVA2,             /* DirectX Video Acceleration 2 */
    HW_ACCEL_D3D11VA,           /* Direct3D 11 Video API */
    HW_ACCEL_VAAPI,             /* Video Acceleration API (Linux) */
    HW_ACCEL_VDPAU,             /* Video Decode and Presentation API */
    HW_ACCEL_NVENC,             /* NVIDIA NVENC */
    HW_ACCEL_NVDEC,             /* NVIDIA NVDEC */
    HW_ACCEL_QSV,               /* Intel Quick Sync Video */
    HW_ACCEL_AMF,               /* AMD Media Framework */
    HW_ACCEL_VIDEOTOOLBOX,      /* Apple VideoToolbox */
    HW_ACCEL_CUDA,              /* CUDA */
    HW_ACCEL_OPENCL,            /* OpenCL */
    HW_ACCEL_LIMITLESS_NPU      /* LimitlessOS Neural Processing Unit */
} hw_accel_type_t;

/* Media Stream Information */
typedef struct media_stream_info {
    uint32_t index;
    media_type_t type;
    codec_type_t codec;
    
    union {
        struct {
            audio_format_t format;
            uint32_t sample_rate;
            uint32_t channels;
            uint32_t bit_depth;
            uint64_t channel_layout;
            uint32_t bitrate;
            uint32_t frame_size;
        } audio;
        
        struct {
            video_format_t format;
            uint32_t width;
            uint32_t height;
            uint32_t fps;           /* fps * 1000 */
            uint32_t bitrate;
            float aspect_ratio;
            bool interlaced;
            uint32_t color_space;
            uint32_t color_range;
            uint32_t bit_depth;
            uint32_t profile;
            uint32_t level;
        } video;
        
        struct {
            char language[8];
            char encoding[32];
            bool forced;
            bool default_track;
        } subtitle;
    } params;
    
    /* Metadata */
    char title[128];
    char language[8];
    char codec_name[64];
    uint64_t duration;          /* Duration in microseconds */
    uint64_t start_time;        /* Start time in microseconds */
    uint32_t disposition;       /* Track disposition flags */
    
} media_stream_info_t;

/* Media Container Information */
typedef struct media_container_info {
    char format_name[64];       /* Container format name */
    char format_long_name[256]; /* Long format name */
    uint64_t duration;          /* Total duration in microseconds */
    uint64_t start_time;        /* Start time in microseconds */
    uint64_t bitrate;           /* Total bitrate */
    uint32_t nb_streams;        /* Number of streams */
    media_stream_info_t* streams; /* Stream information array */
    
    /* Metadata */
    struct {
        char title[256];
        char artist[256];
        char album[256];
        char genre[64];
        char year[8];
        char comment[512];
        char track[8];
        char disc[8];
        char encoder[128];
        char creation_time[32];
    } metadata;
} media_container_info_t;

/* Audio Frame */
typedef struct audio_frame {
    audio_format_t format;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t samples;           /* Number of samples per channel */
    uint64_t pts;               /* Presentation timestamp */
    uint64_t duration;          /* Frame duration */
    
    /* Audio data */
    void* data[MAX_AUDIO_CHANNELS]; /* Planar audio data pointers */
    uint32_t linesize[MAX_AUDIO_CHANNELS]; /* Data size per channel */
    size_t total_size;          /* Total frame size */
    
    /* Hardware acceleration */
    hw_accel_type_t hw_accel;
    void* hw_data;              /* Hardware-specific data */
    
} audio_frame_t;

/* Video Frame */
typedef struct video_frame {
    video_format_t format;
    uint32_t width;
    uint32_t height;
    uint64_t pts;               /* Presentation timestamp */
    uint64_t duration;          /* Frame duration */
    
    /* Video data */
    void* data[4];              /* Video plane pointers */
    uint32_t linesize[4];       /* Line size for each plane */
    size_t total_size;          /* Total frame size */
    
    /* Color information */
    uint32_t color_space;
    uint32_t color_range;
    uint32_t color_primaries;
    uint32_t color_trc;         /* Transfer characteristics */
    
    /* Hardware acceleration */
    hw_accel_type_t hw_accel;
    gpu_image_t* gpu_texture;   /* GPU texture for HW frames */
    void* hw_data;              /* Hardware-specific data */
    
    /* Frame properties */
    bool key_frame;
    bool interlaced;
    uint32_t picture_type;      /* I, P, B frame */
    
} video_frame_t;

/* Codec Context */
typedef struct codec_context {
    codec_type_t codec_type;
    bool is_encoder;            /* true for encoder, false for decoder */
    
    union {
        struct {
            audio_format_t format;
            uint32_t sample_rate;
            uint32_t channels;
            uint64_t channel_layout;
            uint32_t bit_rate;
            uint32_t frame_size;
            uint32_t compression_level;
        } audio;
        
        struct {
            video_format_t format;
            uint32_t width;
            uint32_t height;
            uint32_t fps;
            uint32_t bit_rate;
            uint32_t gop_size;      /* Group of Pictures size */
            uint32_t max_b_frames;  /* Maximum B-frames */
            uint32_t profile;
            uint32_t level;
            float crf;              /* Constant Rate Factor */
            char preset[32];        /* Encoding preset */
            char tune[32];          /* Tuning option */
        } video;
    } params;
    
    /* Hardware acceleration */
    hw_accel_type_t hw_accel;
    void* hw_device_ctx;        /* Hardware device context */
    
    /* Codec-specific data */
    void* codec_data;           /* Codec private data */
    size_t codec_data_size;
    
    /* Threading */
    uint32_t thread_count;
    uint32_t thread_type;
    
    /* Quality settings */
    uint32_t quality;           /* Codec quality (0-100) */
    bool lossless;              /* Lossless encoding */
    
} codec_context_t;

/* Media Decoder */
typedef struct media_decoder {
    uint32_t id;
    codec_context_t* context;
    
    /* Input stream */
    media_stream_info_t stream_info;
    
    /* Decoding state */
    bool initialized;
    bool flushing;
    uint64_t decoded_frames;
    uint64_t dropped_frames;
    
    /* Buffer management */
    void* input_buffer;
    size_t input_buffer_size;
    void* output_buffer;
    size_t output_buffer_size;
    
    /* Hardware acceleration */
    hw_accel_type_t hw_accel;
    void* hw_context;
    
    /* Performance metrics */
    float decode_fps;
    uint64_t decode_time_total;
    uint64_t decode_time_avg;
    
} media_decoder_t;

/* Media Encoder */
typedef struct media_encoder {
    uint32_t id;
    codec_context_t* context;
    
    /* Output stream */
    media_stream_info_t stream_info;
    
    /* Encoding state */
    bool initialized;
    bool flushing;
    uint64_t encoded_frames;
    
    /* Buffer management */
    void* input_buffer;
    size_t input_buffer_size;
    void* output_buffer;
    size_t output_buffer_size;
    
    /* Hardware acceleration */
    hw_accel_type_t hw_accel;
    void* hw_context;
    
    /* Rate control */
    uint32_t bitrate_target;
    uint32_t bitrate_max;
    uint32_t bitrate_min;
    
    /* Performance metrics */
    float encode_fps;
    uint64_t encode_time_total;
    uint64_t encode_time_avg;
    
} media_encoder_t;

/* Audio Device */
typedef struct audio_device {
    uint32_t id;
    char name[128];
    char driver[64];
    
    /* Device capabilities */
    bool is_input;              /* Capture device */
    bool is_output;             /* Playback device */
    bool is_default;            /* Default device */
    
    /* Supported formats */
    uint32_t format_count;
    audio_format_t* supported_formats;
    
    /* Supported sample rates */
    uint32_t sample_rate_count;
    uint32_t* supported_sample_rates;
    
    /* Channel configuration */
    uint32_t min_channels;
    uint32_t max_channels;
    uint64_t channel_layouts;   /* Supported channel layouts bitmask */
    
    /* Latency */
    uint32_t min_latency_us;    /* Minimum latency in microseconds */
    uint32_t max_latency_us;    /* Maximum latency in microseconds */
    
    /* Hardware info */
    char manufacturer[64];
    char model[64];
    uint32_t vendor_id;
    uint32_t product_id;
    
    struct audio_device* next;
} audio_device_t;

/* Video Device (Camera) */
typedef struct video_device {
    uint32_t id;
    char name[128];
    char driver[64];
    
    /* Device capabilities */
    bool is_input;              /* Capture device */
    bool is_output;             /* Display device */
    
    /* Supported formats */
    uint32_t format_count;
    video_format_t* supported_formats;
    
    /* Supported resolutions */
    uint32_t resolution_count;
    struct {
        uint32_t width;
        uint32_t height;
        uint32_t fps_count;
        uint32_t* fps_values;
    }* resolutions;
    
    /* Controls */
    struct {
        bool auto_exposure;
        bool auto_white_balance;
        bool auto_focus;
        uint32_t brightness;
        uint32_t contrast;
        uint32_t saturation;
        uint32_t hue;
        uint32_t gamma;
        uint32_t gain;
        uint32_t exposure;
        uint32_t white_balance;
        uint32_t focus;
        uint32_t zoom;
    } controls;
    
    /* Hardware info */
    char manufacturer[64];
    char model[64];
    uint32_t vendor_id;
    uint32_t product_id;
    
    struct video_device* next;
} video_device_t;

/* Media Session */
typedef struct media_session {
    uint32_t id;
    char name[128];
    
    /* Input/Output */
    char input_url[512];        /* Input media URL/file */
    char output_url[512];       /* Output media URL/file */
    
    /* Container info */
    media_container_info_t container_info;
    
    /* Decoders/Encoders */
    media_decoder_t* decoders[MAX_MEDIA_STREAMS];
    media_encoder_t* encoders[MAX_MEDIA_STREAMS];
    uint32_t decoder_count;
    uint32_t encoder_count;
    
    /* Playback state */
    enum {
        MEDIA_STATE_STOPPED,
        MEDIA_STATE_PLAYING,
        MEDIA_STATE_PAUSED,
        MEDIA_STATE_SEEKING
    } state;
    
    uint64_t position;          /* Current position in microseconds */
    uint64_t duration;          /* Total duration in microseconds */
    float playback_rate;        /* Playback speed (1.0 = normal) */
    
    /* Audio/Video sync */
    uint64_t audio_pts;         /* Audio presentation timestamp */
    uint64_t video_pts;         /* Video presentation timestamp */
    int64_t av_sync_diff;       /* A/V sync difference */
    
    /* Hardware acceleration */
    hw_accel_type_t hw_accel;
    void* hw_device_ctx;
    
    /* Performance */
    struct {
        float fps;              /* Current playback FPS */
        uint32_t dropped_frames;
        uint32_t decoded_frames;
        float cpu_usage;
        float memory_usage;
    } performance;
    
} media_session_t;

/* Media Framework */
typedef struct media_framework {
    bool initialized;
    uint32_t version;
    
    /* Hardware acceleration */
    hw_accel_type_t available_hw_accel[16];
    uint32_t hw_accel_count;
    
    /* Devices */
    audio_device_t* audio_devices;
    video_device_t* video_devices;
    uint32_t audio_device_count;
    uint32_t video_device_count;
    
    /* Sessions */
    media_session_t* sessions[MAX_MEDIA_SESSIONS];
    uint32_t session_count;
    
    /* Codec support */
    bool codec_support[256];    /* Supported codecs bitmask */
    
    /* Performance settings */
    uint32_t thread_count;
    uint32_t max_decode_threads;
    uint32_t max_encode_threads;
    bool hardware_accel_enabled;
    uint32_t buffer_size;
    
    /* Statistics */
    struct {
        uint64_t sessions_created;
        uint64_t frames_decoded;
        uint64_t frames_encoded;
        uint64_t bytes_processed;
        float avg_decode_fps;
        float avg_encode_fps;
    } stats;
    
} media_framework_t;

/* Global media framework */
extern media_framework_t media_framework;

/* Core Media API */
status_t media_framework_init(void);
void media_framework_shutdown(void);
status_t media_detect_hardware_acceleration(void);

/* Device Management */
status_t media_enumerate_audio_devices(void);
status_t media_enumerate_video_devices(void);
audio_device_t* media_get_audio_device_by_id(uint32_t id);
video_device_t* media_get_video_device_by_id(uint32_t id);
audio_device_t* media_get_default_audio_input(void);
audio_device_t* media_get_default_audio_output(void);

/* Media Session Management */
media_session_t* media_create_session(const char* name);
void media_destroy_session(media_session_t* session);
status_t media_open_input(media_session_t* session, const char* url);
status_t media_open_output(media_session_t* session, const char* url);
status_t media_close_session(media_session_t* session);

/* Playback Control */
status_t media_play(media_session_t* session);
status_t media_pause(media_session_t* session);
status_t media_stop(media_session_t* session);
status_t media_seek(media_session_t* session, uint64_t position);
status_t media_set_playback_rate(media_session_t* session, float rate);

/* Frame Processing */
status_t media_decode_audio_frame(media_decoder_t* decoder, audio_frame_t** frame);
status_t media_decode_video_frame(media_decoder_t* decoder, video_frame_t** frame);
status_t media_encode_audio_frame(media_encoder_t* encoder, audio_frame_t* frame);
status_t media_encode_video_frame(media_encoder_t* encoder, video_frame_t* frame);

/* Frame Management */
audio_frame_t* media_create_audio_frame(audio_format_t format, uint32_t sample_rate, uint32_t channels, uint32_t samples);
video_frame_t* media_create_video_frame(video_format_t format, uint32_t width, uint32_t height);
void media_free_audio_frame(audio_frame_t* frame);
void media_free_video_frame(video_frame_t* frame);

/* Codec Management */
status_t media_create_decoder(codec_type_t codec, media_stream_info_t* stream_info, media_decoder_t** decoder);
status_t media_create_encoder(codec_type_t codec, codec_context_t* context, media_encoder_t** encoder);
void media_destroy_decoder(media_decoder_t* decoder);
void media_destroy_encoder(media_encoder_t* encoder);

/* Hardware Acceleration */
status_t media_init_hardware_acceleration(hw_accel_type_t type);
void media_shutdown_hardware_acceleration(hw_accel_type_t type);
bool media_is_hardware_accel_available(hw_accel_type_t type);
status_t media_create_hw_decoder(codec_type_t codec, hw_accel_type_t hw_accel, media_decoder_t** decoder);
status_t media_create_hw_encoder(codec_type_t codec, hw_accel_type_t hw_accel, media_encoder_t** encoder);

/* Audio Processing */
status_t media_audio_resample(audio_frame_t* src, audio_frame_t* dst, uint32_t target_sample_rate, uint32_t target_channels);
status_t media_audio_mix(audio_frame_t** inputs, uint32_t input_count, audio_frame_t* output);
status_t media_audio_apply_effects(audio_frame_t* frame, void* effects_chain);

/* Video Processing */
status_t media_video_scale(video_frame_t* src, video_frame_t* dst, uint32_t target_width, uint32_t target_height);
status_t media_video_convert_format(video_frame_t* src, video_frame_t* dst, video_format_t target_format);
status_t media_video_apply_filters(video_frame_t* frame, void* filter_chain);

/* Streaming */
status_t media_start_rtmp_stream(media_session_t* session, const char* url, const char* stream_key);
status_t media_start_webrtc_stream(media_session_t* session, void* webrtc_config);
status_t media_stop_stream(media_session_t* session);

/* Utilities */
const char* media_get_codec_name(codec_type_t codec);
const char* media_get_format_name(audio_format_t format);
const char* media_get_format_name_video(video_format_t format);
uint32_t media_get_audio_frame_size(audio_format_t format, uint32_t channels, uint32_t samples);
uint32_t media_get_video_frame_size(video_format_t format, uint32_t width, uint32_t height);

/* Performance and Debug */
void media_print_device_info(void);
void media_print_session_info(media_session_t* session);
void media_print_performance_stats(void);
status_t media_enable_debug_logging(bool enable);

#endif /* LIMITLESS_MEDIA_FRAMEWORK_H */