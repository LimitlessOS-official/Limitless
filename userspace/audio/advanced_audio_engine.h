/*
 * LimitlessOS Advanced Audio Engine
 * Professional audio processing, spatial audio, real-time effects, and AI enhancement
 */

#ifndef ADVANCED_AUDIO_ENGINE_H
#define ADVANCED_AUDIO_ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <complex.h>
#include <math.h>
#include <time.h>

/* Audio system constants */
#define MAX_AUDIO_DEVICES           64
#define MAX_AUDIO_CHANNELS          32
#define MAX_AUDIO_STREAMS           256
#define MAX_AUDIO_EFFECTS           128
#define MAX_AUDIO_MIXERS           16
#define MAX_AUDIO_PROCESSORS       32
#define MAX_SPATIAL_OBJECTS        512
#define MAX_REVERB_ZONES          16
#define MAX_EQ_BANDS              31
#define MAX_COMPRESSOR_BANDS      8
#define MAX_SAMPLE_RATES          16
#define MAX_BUFFER_SIZES          8
#define MAX_AUDIO_PLUGINS         128

/* Audio buffer and format constants */
#define AUDIO_BUFFER_SIZE_DEFAULT  1024
#define AUDIO_BUFFER_SIZE_MIN      64
#define AUDIO_BUFFER_SIZE_MAX      8192
#define AUDIO_SAMPLE_RATE_DEFAULT  48000
#define AUDIO_SAMPLE_RATE_MAX      384000
#define AUDIO_BIT_DEPTH_MAX        32
#define AUDIO_CHANNELS_MAX         32

/* DSP processing constants */
#define FFT_SIZE_MAX               8192
#define CONVOLUTION_SIZE_MAX       16384
#define FIR_FILTER_SIZE_MAX        1024
#define IIR_FILTER_STAGES_MAX      16
#define DELAY_LINE_SIZE_MAX        192000  /* 4 seconds at 48kHz */

/* Spatial audio constants */
#define HRTF_SIZE                  512
#define AMBISONIC_ORDER_MAX        7
#define SPEAKER_COUNT_MAX          64
#define ROOM_REFLECTIONS_MAX       32
#define SOUND_SPEED                343.0f  /* m/s */

/* Audio formats and types */
typedef enum {
    AUDIO_FORMAT_UNKNOWN = 0,
    AUDIO_FORMAT_PCM_S16,
    AUDIO_FORMAT_PCM_S24,
    AUDIO_FORMAT_PCM_S32,
    AUDIO_FORMAT_PCM_F32,
    AUDIO_FORMAT_PCM_F64,
    AUDIO_FORMAT_DSD64,
    AUDIO_FORMAT_DSD128,
    AUDIO_FORMAT_DSD256,
    AUDIO_FORMAT_MAX
} audio_format_t;

typedef enum {
    AUDIO_DEVICE_PLAYBACK = 0,
    AUDIO_DEVICE_CAPTURE,
    AUDIO_DEVICE_DUPLEX,
    AUDIO_DEVICE_LOOPBACK,
    AUDIO_DEVICE_MONITOR,
    AUDIO_DEVICE_MAX
} audio_device_type_t;

typedef enum {
    AUDIO_STREAM_PLAYBACK = 0,
    AUDIO_STREAM_CAPTURE,
    AUDIO_STREAM_DUPLEX,
    AUDIO_STREAM_EFFECT_SEND,
    AUDIO_STREAM_EFFECT_RETURN,
    AUDIO_STREAM_MAX
} audio_stream_type_t;

typedef enum {
    EFFECT_NONE = 0,
    EFFECT_REVERB,
    EFFECT_DELAY,
    EFFECT_CHORUS,
    EFFECT_FLANGER,
    EFFECT_PHASER,
    EFFECT_DISTORTION,
    EFFECT_COMPRESSOR,
    EFFECT_LIMITER,
    EFFECT_GATE,
    EFFECT_EQ_PARAMETRIC,
    EFFECT_EQ_GRAPHIC,
    EFFECT_FILTER_LOWPASS,
    EFFECT_FILTER_HIGHPASS,
    EFFECT_FILTER_BANDPASS,
    EFFECT_FILTER_NOTCH,
    EFFECT_PITCH_SHIFT,
    EFFECT_TIME_STRETCH,
    EFFECT_VOCODER,
    EFFECT_BITCRUSHER,
    EFFECT_RING_MODULATOR,
    EFFECT_GRANULAR,
    EFFECT_SPATIAL_REVERB,
    EFFECT_BINAURAL_PANNER,
    EFFECT_AMBISONIC_ENCODER,
    EFFECT_AMBISONIC_DECODER,
    EFFECT_CONVOLUTION_REVERB,
    EFFECT_AI_ENHANCER,
    EFFECT_AI_NOISE_REDUCTION,
    EFFECT_AI_SPATIAL_UPSAMPLER,
    EFFECT_MAX
} audio_effect_type_t;

typedef enum {
    SPATIAL_MODE_STEREO = 0,
    SPATIAL_MODE_SURROUND_5_1,
    SPATIAL_MODE_SURROUND_7_1,
    SPATIAL_MODE_SURROUND_7_1_4, /* Dolby Atmos */
    SPATIAL_MODE_SURROUND_22_2,  /* NHK 22.2 */
    SPATIAL_MODE_BINAURAL,
    SPATIAL_MODE_AMBISONIC_1ST,
    SPATIAL_MODE_AMBISONIC_2ND,
    SPATIAL_MODE_AMBISONIC_3RD,
    SPATIAL_MODE_AMBISONIC_7TH,
    SPATIAL_MODE_OBJECT_BASED,
    SPATIAL_MODE_MAX
} spatial_audio_mode_t;

typedef enum {
    AI_MODEL_NOISE_REDUCTION = 0,
    AI_MODEL_SPEECH_ENHANCEMENT,
    AI_MODEL_MUSIC_SEPARATION,
    AI_MODEL_SPATIAL_UPSAMPLER,
    AI_MODEL_DYNAMIC_RANGE_ENHANCER,
    AI_MODEL_REAL_TIME_MASTERING,
    AI_MODEL_ROOM_CORRECTION,
    AI_MODEL_PERCEPTUAL_ENHANCEMENT,
    AI_MODEL_MAX
} ai_audio_model_t;

/* Audio buffer structure */
typedef struct {
    float **channels;           /* Channel pointers */
    uint32_t frame_count;      /* Number of frames */
    uint32_t channel_count;    /* Number of channels */
    uint32_t sample_rate;      /* Sample rate */
    audio_format_t format;     /* Audio format */
    uint64_t timestamp;        /* Buffer timestamp */
    bool interleaved;          /* Interleaved flag */
    pthread_mutex_t lock;      /* Buffer lock */
} audio_buffer_t;

/* Audio device structure */
typedef struct {
    uint32_t device_id;
    char name[64];
    char manufacturer[32];
    char model[32];
    audio_device_type_t type;
    
    /* Device capabilities */
    uint32_t supported_sample_rates[MAX_SAMPLE_RATES];
    uint32_t sample_rate_count;
    uint32_t supported_buffer_sizes[MAX_BUFFER_SIZES];
    uint32_t buffer_size_count;
    audio_format_t supported_formats[16];
    uint32_t format_count;
    uint32_t max_input_channels;
    uint32_t max_output_channels;
    
    /* Current configuration */
    uint32_t current_sample_rate;
    uint32_t current_buffer_size;
    audio_format_t current_format;
    uint32_t current_input_channels;
    uint32_t current_output_channels;
    
    /* Device state */
    bool active;
    bool exclusive_mode;
    bool hardware_control;
    uint32_t latency_frames;
    double cpu_load;
    
    /* Hardware-specific */
    void *driver_data;
    char driver_name[32];
    uint32_t driver_version;
    
    pthread_mutex_t lock;
} audio_device_t;

/* Audio stream structure */
typedef struct {
    uint32_t stream_id;
    char name[64];
    audio_stream_type_t type;
    uint32_t device_id;
    
    /* Stream configuration */
    uint32_t sample_rate;
    uint32_t buffer_size;
    audio_format_t format;
    uint32_t input_channels;
    uint32_t output_channels;
    
    /* Stream buffers */
    audio_buffer_t *input_buffer;
    audio_buffer_t *output_buffer;
    audio_buffer_t *processing_buffer;
    
    /* Stream state */
    bool active;
    bool running;
    bool paused;
    uint64_t frames_processed;
    uint64_t timestamp;
    double cpu_usage;
    
    /* Real-time priority */
    int rt_priority;
    pthread_t processing_thread;
    
    /* Callback function */
    int (*process_callback)(audio_buffer_t *input, audio_buffer_t *output, void *user_data);
    void *user_data;
    
    pthread_mutex_t lock;
} audio_stream_t;

/* DSP filter structures */
typedef struct {
    double b[IIR_FILTER_STAGES_MAX + 1]; /* Numerator coefficients */
    double a[IIR_FILTER_STAGES_MAX + 1]; /* Denominator coefficients */
    double x[IIR_FILTER_STAGES_MAX + 1]; /* Input delay line */
    double y[IIR_FILTER_STAGES_MAX + 1]; /* Output delay line */
    uint32_t order;                      /* Filter order */
    double gain;                         /* Filter gain */
} iir_filter_t;

typedef struct {
    float h[FIR_FILTER_SIZE_MAX];       /* Impulse response */
    float x[FIR_FILTER_SIZE_MAX];       /* Input delay line */
    uint32_t length;                    /* Filter length */
    uint32_t index;                     /* Current index */
    float gain;                         /* Filter gain */
} fir_filter_t;

typedef struct {
    float *delay_line;                  /* Delay buffer */
    uint32_t size;                      /* Buffer size */
    uint32_t read_index;               /* Read position */
    uint32_t write_index;              /* Write position */
    float feedback;                     /* Feedback amount */
    float wet_level;                    /* Wet signal level */
    float dry_level;                    /* Dry signal level */
} delay_line_t;

/* EQ band structure */
typedef struct {
    float frequency;                    /* Center frequency */
    float gain;                        /* Gain in dB */
    float q_factor;                    /* Q factor */
    iir_filter_t filter;               /* IIR filter */
    bool enabled;                      /* Band enabled */
} eq_band_t;

/* Compressor structure */
typedef struct {
    float threshold;                   /* Threshold in dB */
    float ratio;                      /* Compression ratio */
    float attack_time;                /* Attack time in ms */
    float release_time;               /* Release time in ms */
    float knee_width;                 /* Soft knee width */
    float makeup_gain;                /* Makeup gain in dB */
    
    /* Internal state */
    float envelope;                   /* Envelope follower */
    float gain_reduction;             /* Current gain reduction */
    bool auto_makeup;                 /* Auto makeup gain */
    
    /* Lookahead */
    delay_line_t lookahead_delay;     /* Lookahead buffer */
    uint32_t lookahead_samples;       /* Lookahead time */
} compressor_t;

/* Reverb structure */
typedef struct {
    /* Reverb parameters */
    float room_size;                  /* Room size (0.0 - 1.0) */
    float damping;                    /* Damping factor */
    float early_reflections;          /* Early reflections level */
    float late_reverb;               /* Late reverb level */
    float diffusion;                 /* Diffusion amount */
    float decay_time;                /* RT60 decay time */
    float pre_delay;                 /* Pre-delay time */
    float wet_level;                 /* Wet signal level */
    float dry_level;                 /* Dry signal level */
    
    /* Reverb processing */
    delay_line_t early_delays[8];     /* Early reflection delays */
    delay_line_t late_delays[16];     /* Late reverb delays */
    fir_filter_t allpass_filters[8];  /* Allpass filters */
    iir_filter_t damping_filters[4];  /* Damping filters */
    
    /* Convolution reverb */
    bool convolution_enabled;         /* Use convolution */
    float *impulse_response;          /* IR samples */
    uint32_t ir_length;              /* IR length */
    complex float *fft_buffer;        /* FFT buffer */
    complex float *ir_fft;            /* IR FFT */
} reverb_t;

/* Spatial audio structures */
typedef struct {
    float x, y, z;                    /* Position */
    float vx, vy, vz;                 /* Velocity */
    float orientation[3];             /* Orientation vector */
    float up_vector[3];              /* Up vector */
} spatial_position_t;

typedef struct {
    spatial_position_t position;      /* Object position */
    spatial_position_t velocity;      /* Object velocity */
    float gain;                      /* Object gain */
    float radius;                    /* Sound radius */
    float directivity;               /* Directivity pattern */
    bool distance_attenuation;       /* Distance-based attenuation */
    bool doppler_enabled;            /* Doppler effect */
    
    /* HRTF data */
    float hrtf_left[HRTF_SIZE];      /* Left ear HRTF */
    float hrtf_right[HRTF_SIZE];     /* Right ear HRTF */
    
    /* Ambisonic encoding */
    float ambisonic_gains[64];       /* Ambisonic channel gains */
    uint32_t ambisonic_order;        /* Ambisonic order */
} spatial_audio_object_t;

typedef struct {
    spatial_position_t listener_pos;  /* Listener position */
    spatial_position_t listener_vel;  /* Listener velocity */
    float head_radius;               /* Head radius for HRTF */
    
    /* Room acoustics */
    float room_dimensions[3];        /* Room size */
    float wall_absorption[6];        /* Wall absorption coefficients */
    float air_absorption;            /* Air absorption */
    float temperature;               /* Air temperature */
    float humidity;                  /* Air humidity */
    
    /* Processing mode */
    spatial_audio_mode_t mode;       /* Spatial mode */
    uint32_t speaker_count;          /* Number of speakers */
    spatial_position_t speakers[SPEAKER_COUNT_MAX]; /* Speaker positions */
    
    /* HRTF database */
    float ***hrtf_database;          /* HRTF for all directions */
    uint32_t hrtf_elevations;        /* Number of elevation angles */
    uint32_t hrtf_azimuths;         /* Number of azimuth angles */
} spatial_audio_context_t;

/* AI audio processing */
typedef struct {
    ai_audio_model_t model_type;     /* AI model type */
    void *model_data;                /* Model weights/data */
    uint32_t input_features;         /* Input feature count */
    uint32_t output_features;        /* Output feature count */
    uint32_t hidden_layers;          /* Hidden layer count */
    
    /* Processing buffers */
    float *feature_buffer;           /* Feature extraction buffer */
    float *inference_buffer;         /* Inference buffer */
    float *output_buffer;           /* Output buffer */
    
    /* Model parameters */
    float learning_rate;             /* Adaptive learning rate */
    bool real_time_training;         /* Real-time adaptation */
    uint32_t context_frames;         /* Context window size */
    
    /* Performance metrics */
    double inference_time;           /* Inference time per frame */
    double model_accuracy;           /* Model accuracy */
    uint64_t processed_frames;       /* Total processed frames */
} ai_audio_processor_t;

/* Audio effect structure */
typedef struct {
    uint32_t effect_id;
    char name[64];
    audio_effect_type_t type;
    bool enabled;
    bool bypass;
    
    /* Effect parameters (union for different effect types) */
    union {
        reverb_t reverb;
        compressor_t compressor;
        delay_line_t delay;
        eq_band_t eq_bands[MAX_EQ_BANDS];
        ai_audio_processor_t ai_processor;
        
        /* Generic parameter storage */
        struct {
            float parameters[32];
            int int_parameters[16];
            bool bool_parameters[16];
        } generic;
    } params;
    
    /* Processing function */
    int (*process)(struct audio_effect *effect, audio_buffer_t *input, audio_buffer_t *output);
    
    /* State and metadata */
    uint32_t sample_rate;
    uint32_t channels;
    double cpu_usage;
    uint64_t frames_processed;
    
    pthread_mutex_t lock;
} audio_effect_t;

/* Audio mixer structure */
typedef struct {
    uint32_t mixer_id;
    char name[64];
    
    /* Input channels */
    uint32_t input_count;
    audio_stream_t *inputs[MAX_AUDIO_CHANNELS];
    float input_gains[MAX_AUDIO_CHANNELS];
    float input_pans[MAX_AUDIO_CHANNELS];
    bool input_mutes[MAX_AUDIO_CHANNELS];
    bool input_solos[MAX_AUDIO_CHANNELS];
    
    /* Output configuration */
    uint32_t output_channels;
    float master_gain;
    float output_gains[MAX_AUDIO_CHANNELS];
    
    /* Effects chain */
    uint32_t effect_count;
    audio_effect_t *effects[MAX_AUDIO_EFFECTS];
    
    /* Mixer state */
    bool active;
    double cpu_usage;
    uint64_t frames_mixed;
    
    /* Real-time processing */
    pthread_t mixing_thread;
    bool real_time_priority;
    
    pthread_mutex_t lock;
} audio_mixer_t;

/* Main audio system structure */
typedef struct {
    bool initialized;
    pthread_mutex_t system_lock;
    
    /* Device management */
    audio_device_t devices[MAX_AUDIO_DEVICES];
    uint32_t device_count;
    uint32_t default_playback_device;
    uint32_t default_capture_device;
    
    /* Stream management */
    audio_stream_t streams[MAX_AUDIO_STREAMS];
    uint32_t stream_count;
    uint32_t active_streams;
    
    /* Effect processing */
    audio_effect_t effects[MAX_AUDIO_EFFECTS];
    uint32_t effect_count;
    
    /* Mixer management */
    audio_mixer_t mixers[MAX_AUDIO_MIXERS];
    uint32_t mixer_count;
    
    /* Spatial audio */
    spatial_audio_context_t spatial_context;
    spatial_audio_object_t spatial_objects[MAX_SPATIAL_OBJECTS];
    uint32_t spatial_object_count;
    
    /* AI processing */
    ai_audio_processor_t ai_processors[MAX_AUDIO_PROCESSORS];
    uint32_t ai_processor_count;
    bool ai_enhancement_enabled;
    
    /* System configuration */
    uint32_t default_sample_rate;
    uint32_t default_buffer_size;
    audio_format_t default_format;
    bool exclusive_mode_preferred;
    int real_time_priority;
    
    /* Performance monitoring */
    double system_cpu_usage;
    uint32_t buffer_underruns;
    uint32_t buffer_overruns;
    uint64_t total_frames_processed;
    time_t start_time;
    
    /* Threading */
    pthread_t monitor_thread;
    bool threads_running;
} audio_engine_t;

/* Function declarations */

/* System initialization and cleanup */
int audio_engine_init(void);
int audio_engine_cleanup(void);
int audio_engine_set_config(uint32_t sample_rate, uint32_t buffer_size, audio_format_t format);

/* Device management */
int audio_device_enumerate(void);
int audio_device_get_info(uint32_t device_id, audio_device_t *info);
int audio_device_open(uint32_t device_id, audio_device_type_t type);
int audio_device_close(uint32_t device_id);
int audio_device_set_format(uint32_t device_id, uint32_t sample_rate, audio_format_t format);

/* Stream management */
int audio_stream_create(const char *name, audio_stream_type_t type, uint32_t device_id);
int audio_stream_destroy(uint32_t stream_id);
int audio_stream_start(uint32_t stream_id);
int audio_stream_stop(uint32_t stream_id);
int audio_stream_set_callback(uint32_t stream_id, 
                             int (*callback)(audio_buffer_t *input, audio_buffer_t *output, void *data),
                             void *user_data);

/* Buffer management */
int audio_buffer_create(audio_buffer_t *buffer, uint32_t frames, uint32_t channels, 
                       uint32_t sample_rate, audio_format_t format);
int audio_buffer_destroy(audio_buffer_t *buffer);
int audio_buffer_copy(const audio_buffer_t *src, audio_buffer_t *dst);
int audio_buffer_mix(const audio_buffer_t *src1, const audio_buffer_t *src2, audio_buffer_t *dst, float gain);
int audio_buffer_convert_format(const audio_buffer_t *src, audio_buffer_t *dst, audio_format_t target_format);

/* Effect processing */
int audio_effect_create(const char *name, audio_effect_type_t type);
int audio_effect_destroy(uint32_t effect_id);
int audio_effect_set_parameter(uint32_t effect_id, const char *param_name, float value);
int audio_effect_get_parameter(uint32_t effect_id, const char *param_name, float *value);
int audio_effect_process(uint32_t effect_id, audio_buffer_t *input, audio_buffer_t *output);
int audio_effect_bypass(uint32_t effect_id, bool bypass);

/* Mixer functionality */
int audio_mixer_create(const char *name);
int audio_mixer_destroy(uint32_t mixer_id);
int audio_mixer_add_input(uint32_t mixer_id, uint32_t stream_id);
int audio_mixer_remove_input(uint32_t mixer_id, uint32_t stream_id);
int audio_mixer_set_gain(uint32_t mixer_id, uint32_t channel, float gain);
int audio_mixer_set_pan(uint32_t mixer_id, uint32_t channel, float pan);
int audio_mixer_add_effect(uint32_t mixer_id, uint32_t effect_id);

/* Spatial audio processing */
int spatial_audio_init(spatial_audio_mode_t mode);
int spatial_audio_set_listener_position(float x, float y, float z, float *orientation);
int spatial_audio_create_object(float x, float y, float z);
int spatial_audio_update_object(uint32_t object_id, float x, float y, float z);
int spatial_audio_process(uint32_t object_id, audio_buffer_t *input, audio_buffer_t *output);
int spatial_audio_set_room_acoustics(float *dimensions, float *absorption);

/* AI audio processing */
int ai_audio_init(ai_audio_model_t model_type);
int ai_audio_load_model(const char *model_path, ai_audio_model_t type);
int ai_audio_process_real_time(ai_audio_model_t model_type, audio_buffer_t *input, audio_buffer_t *output);
int ai_audio_enhance_speech(audio_buffer_t *input, audio_buffer_t *output);
int ai_audio_reduce_noise(audio_buffer_t *input, audio_buffer_t *output, float strength);
int ai_audio_spatial_upsample(audio_buffer_t *stereo_input, audio_buffer_t *surround_output);

/* DSP utility functions */
int dsp_fft(const float *input, complex float *output, uint32_t size);
int dsp_ifft(const complex float *input, float *output, uint32_t size);
int dsp_convolution(const float *signal, const float *kernel, float *output, 
                   uint32_t signal_len, uint32_t kernel_len);
int dsp_apply_window(float *buffer, uint32_t size, const char *window_type);
float dsp_rms_level(const float *buffer, uint32_t size);
float dsp_peak_level(const float *buffer, uint32_t size);

/* Format conversion utilities */
int format_pcm_s16_to_f32(const int16_t *input, float *output, uint32_t samples);
int format_pcm_s24_to_f32(const uint8_t *input, float *output, uint32_t samples);
int format_pcm_s32_to_f32(const int32_t *input, float *output, uint32_t samples);
int format_f32_to_pcm_s16(const float *input, int16_t *output, uint32_t samples);
int format_f32_to_pcm_s24(const float *input, uint8_t *output, uint32_t samples);
int format_f32_to_pcm_s32(const float *input, int32_t *output, uint32_t samples);

/* Utility functions */
const char *audio_format_name(audio_format_t format);
const char *audio_effect_name(audio_effect_type_t type);
const char *spatial_mode_name(spatial_audio_mode_t mode);
uint32_t audio_format_bytes_per_sample(audio_format_t format);
uint32_t audio_calculate_buffer_size(uint32_t frames, uint32_t channels, audio_format_t format);
double audio_frames_to_seconds(uint32_t frames, uint32_t sample_rate);
uint32_t audio_seconds_to_frames(double seconds, uint32_t sample_rate);

#endif /* ADVANCED_AUDIO_ENGINE_H */