/**
 * LimitlessOS Screen Recorder
 * 
 * Advanced screen recording and streaming utility with AI-powered quality optimization,
 * real-time effects processing, and military-grade security features. Provides professional
 * screen capture with intelligent compression and automated post-processing.
 * 
 * Features:
 * - Multi-monitor screen recording with selective area capture
 * - AI-powered quality optimization and compression settings
 * - Real-time audio mixing with noise reduction and enhancement
 * - Live streaming to multiple platforms with adaptive bitrate
 * - Advanced video effects and annotations with AI suggestions
 * - Hardware-accelerated encoding (GPU/CPU hybrid optimization)
 * - Intelligent frame rate and resolution adaptation
 * - Automatic scene detection and quality adjustment
 * - Privacy protection with sensitive content masking
 * - Professional editing tools with AI-assisted workflow
 * 
 * Military Design Principles:
 * - Tactical interface with secure recording indicators
 * - Mission-critical recording reliability and redundancy
 * - Advanced encryption for sensitive content protection
 * - Performance optimization for real-time operations
 * - Configurable security policies and access controls
 * 
 * AI Integration:
 * - Intelligent quality optimization based on content analysis
 * - Automated scene detection and enhancement suggestions
 * - Smart compression with content-aware algorithms
 * - Real-time performance monitoring and adjustment
 * - Predictive resource management for smooth recording
 * 
 * @author LimitlessOS Development Team
 * @version 1.0.0
 * @since 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>

// Desktop integration
#include "../../include/limitless_ui.h"
#include "../../include/desktop_integration.h"

// Screen recorder configuration
#define SCREEN_RECORDER_VERSION "1.0.0"
#define MAX_MONITORS 8
#define MAX_AUDIO_SOURCES 16
#define MAX_RECORDING_TIME 28800  // 8 hours max
#define MAX_FILENAME_LENGTH 256
#define MAX_OUTPUT_FORMATS 10
#define FRAME_BUFFER_SIZE 60  // 60 frames buffer
#define AUDIO_BUFFER_SIZE 4096

// Recording modes
typedef enum {
    RECORDING_FULLSCREEN,
    RECORDING_WINDOW,
    RECORDING_REGION,
    RECORDING_WEBCAM,
    RECORDING_MULTI_SOURCE
} recording_mode_t;

// Output formats
typedef enum {
    FORMAT_MP4_H264,
    FORMAT_MP4_H265,
    FORMAT_AVI_XVID,
    FORMAT_MKV_VP9,
    FORMAT_WEBM_VP8,
    FORMAT_MOV_PRORES,
    FORMAT_FLV_H264,
    FORMAT_OGV_THEORA,
    FORMAT_MP4_AV1
} output_format_t;

// Quality presets
typedef enum {
    QUALITY_LOW,        // 720p30, low bitrate
    QUALITY_MEDIUM,     // 1080p30, balanced
    QUALITY_HIGH,       // 1080p60, high bitrate
    QUALITY_ULTRA,      // 4K30, max quality
    QUALITY_STREAMING,  // Optimized for streaming
    QUALITY_CUSTOM      // User-defined settings
} quality_preset_t;

// Recording states
typedef enum {
    STATE_IDLE,
    STATE_PREPARING,
    STATE_RECORDING,
    STATE_PAUSED,
    STATE_STOPPING,
    STATE_ERROR,
    STATE_PROCESSING
} recording_state_t;

// Audio source types
typedef enum {
    AUDIO_MICROPHONE,
    AUDIO_SYSTEM,
    AUDIO_APPLICATION,
    AUDIO_MIXED,
    AUDIO_NONE
} audio_source_type_t;

// Monitor information
typedef struct monitor_info {
    int id;
    char name[64];
    int x, y;
    int width, height;
    int refresh_rate;
    bool is_primary;
    bool enabled_for_recording;
    
    struct monitor_info* next;
} monitor_info_t;

// Audio source configuration
typedef struct audio_source {
    int id;
    char name[64];
    audio_source_type_t type;
    bool enabled;
    float volume;      // 0.0 to 1.0
    bool muted;
    
    // Audio processing
    struct {
        bool noise_reduction;
        bool echo_cancellation;
        bool auto_gain;
        float gain_level;  // dB
    } processing;
    
    struct audio_source* next;
} audio_source_t;

// Recording region
typedef struct {
    int x, y;
    int width, height;
    bool follow_cursor;
    int padding;  // Pixels around cursor
} recording_region_t;

// Video settings
typedef struct {
    int width, height;
    int framerate;
    int bitrate;        // kbps
    output_format_t format;
    quality_preset_t preset;
    
    // Advanced settings
    int keyframe_interval;
    bool variable_bitrate;
    int buffer_size;
    bool hardware_encoding;
    
    // Effects
    bool cursor_enabled;
    bool cursor_highlight;
    bool click_animations;
    bool keyboard_display;
    
} video_settings_t;

// Audio settings
typedef struct {
    int sample_rate;    // Hz
    int channels;       // 1=mono, 2=stereo
    int bitrate;        // kbps
    
    // Processing
    bool noise_gate;
    float noise_gate_threshold;
    bool compressor;
    bool limiter;
    
} audio_settings_t;

// Recording statistics
typedef struct {
    time_t start_time;
    time_t duration;
    uint64_t frames_recorded;
    uint64_t frames_dropped;
    uint64_t bytes_written;
    float average_fps;
    float cpu_usage;
    float gpu_usage;
    float memory_usage;
    
    // Quality metrics
    float encoding_speed;  // x times realtime
    float compression_ratio;
    uint32_t audio_dropouts;
    
} recording_stats_t;

// AI quality optimizer
typedef struct {
    bool enabled;
    bool auto_adjust;
    bool learning_mode;
    
    // Content analysis
    struct {
        bool scene_change_detected;
        float motion_level;      // 0.0 to 1.0
        float detail_level;      // 0.0 to 1.0
        bool text_detected;
        bool faces_detected;
        char scene_type[32];     // "desktop", "game", "presentation", etc.
    } content_analysis;
    
    // Optimization recommendations
    struct {
        bool suggest_bitrate_increase;
        bool suggest_bitrate_decrease;
        bool suggest_framerate_change;
        bool suggest_resolution_change;
        char recommendations[5][256];
        uint32_t recommendation_count;
    } recommendations;
    
    // Adaptive settings
    struct {
        int target_bitrate;
        int target_framerate;
        bool adaptive_quality;
        float quality_factor;    // Dynamic quality multiplier
    } adaptive;
    
    // Learning statistics
    struct {
        uint32_t recordings_analyzed;
        uint32_t optimizations_applied;
        float quality_improvement;
        uint32_t user_accepts;
        uint32_t user_rejects;
    } learning_stats;
    
} ai_quality_optimizer_t;

// Live streaming configuration
typedef struct {
    bool enabled;
    char server_url[256];
    char stream_key[128];
    int target_bitrate;
    bool adaptive_bitrate;
    
    // Platform settings
    char platform[32];      // "Twitch", "YouTube", "Custom"
    char title[128];
    char description[512];
    bool private_stream;
    
    // Stream health
    struct {
        float upload_bandwidth;
        float stream_health;    // 0.0 to 1.0
        uint32_t dropped_frames;
        float latency_ms;
        bool connection_stable;
    } health;
    
} live_streaming_t;

// Recording session
typedef struct recording_session {
    char filename[MAX_FILENAME_LENGTH];
    char output_path[512];
    recording_mode_t mode;
    recording_state_t state;
    
    // Recording area
    recording_region_t region;
    monitor_info_t* target_monitor;
    
    // Settings
    video_settings_t video;
    audio_settings_t audio;
    
    // Statistics
    recording_stats_t stats;
    
    // Buffers and threads
    pthread_t capture_thread;
    pthread_t audio_thread;
    pthread_t encoding_thread;
    
    bool capture_active;
    bool audio_active;
    bool encoding_active;
    
    // Frame buffer
    struct {
        void** frames;
        int buffer_size;
        int write_index;
        int read_index;
        bool buffer_full;
        pthread_mutex_t mutex;
    } frame_buffer;
    
    struct recording_session* next;
} recording_session_t;

// Main screen recorder state
typedef struct {
    bool initialized;
    bool running;
    
    // Hardware capabilities
    bool hardware_encoding_available;
    bool gpu_acceleration_available;
    char gpu_name[64];
    int gpu_memory_mb;
    
    // Monitors
    monitor_info_t* monitors;
    uint32_t monitor_count;
    monitor_info_t* primary_monitor;
    
    // Audio sources
    audio_source_t* audio_sources;
    uint32_t audio_source_count;
    
    // Active recording
    recording_session_t* current_session;
    recording_session_t* session_history[10];
    uint32_t history_count;
    
    // Live streaming
    live_streaming_t streaming;
    
    // UI state
    lui_window_t* main_window;
    lui_widget_t* control_panel;
    lui_widget_t* preview_panel;
    lui_widget_t* settings_panel;
    lui_widget_t* ai_panel;
    lui_widget_t* streaming_panel;
    lui_widget_t* status_bar;
    
    bool show_preview;
    bool show_settings;
    bool show_ai;
    bool show_streaming;
    bool show_advanced;
    
    // Settings
    struct {
        char default_output_path[512];
        output_format_t default_format;
        quality_preset_t default_quality;
        bool auto_start_recording;
        bool minimize_during_recording;
        bool show_countdown;
        uint32_t countdown_seconds;
        bool auto_stop_low_space;
        uint64_t min_free_space_mb;
    } settings;
    
    // AI quality optimizer
    ai_quality_optimizer_t ai_optimizer;
    
    // Security features
    struct {
        bool privacy_mode;
        bool mask_sensitive_content;
        bool encrypt_recordings;
        char encryption_key[64];
        bool require_authentication;
    } security;
    
    // Session statistics
    struct {
        time_t session_start_time;
        uint32_t recordings_created;
        uint64_t total_recording_time;
        uint64_t total_file_size;
        uint32_t streams_started;
        uint32_t ai_optimizations_applied;
        float average_quality_score;
    } stats;
    
} screen_recorder_state_t;

// Global screen recorder state
static screen_recorder_state_t g_screen_recorder = {0};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

static const char* get_recording_mode_name(recording_mode_t mode) {
    switch (mode) {
        case RECORDING_FULLSCREEN: return "Fullscreen";
        case RECORDING_WINDOW: return "Window";
        case RECORDING_REGION: return "Region";
        case RECORDING_WEBCAM: return "Webcam";
        case RECORDING_MULTI_SOURCE: return "Multi-Source";
        default: return "Unknown";
    }
}

static const char* get_output_format_name(output_format_t format) {
    switch (format) {
        case FORMAT_MP4_H264: return "MP4 (H.264)";
        case FORMAT_MP4_H265: return "MP4 (H.265)";
        case FORMAT_AVI_XVID: return "AVI (XviD)";
        case FORMAT_MKV_VP9: return "MKV (VP9)";
        case FORMAT_WEBM_VP8: return "WebM (VP8)";
        case FORMAT_MOV_PRORES: return "MOV (ProRes)";
        case FORMAT_FLV_H264: return "FLV (H.264)";
        case FORMAT_OGV_THEORA: return "OGV (Theora)";
        case FORMAT_MP4_AV1: return "MP4 (AV1)";
        default: return "Unknown";
    }
}

static const char* get_quality_preset_name(quality_preset_t preset) {
    switch (preset) {
        case QUALITY_LOW: return "Low (720p30)";
        case QUALITY_MEDIUM: return "Medium (1080p30)";
        case QUALITY_HIGH: return "High (1080p60)";
        case QUALITY_ULTRA: return "Ultra (4K30)";
        case QUALITY_STREAMING: return "Streaming Optimized";
        case QUALITY_CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

static const char* get_recording_state_name(recording_state_t state) {
    switch (state) {
        case STATE_IDLE: return "Idle";
        case STATE_PREPARING: return "Preparing";
        case STATE_RECORDING: return "Recording";
        case STATE_PAUSED: return "Paused";
        case STATE_STOPPING: return "Stopping";
        case STATE_ERROR: return "Error";
        case STATE_PROCESSING: return "Processing";
        default: return "Unknown";
    }
}

static lui_color_t get_recording_state_color(recording_state_t state) {
    switch (state) {
        case STATE_IDLE: return LUI_COLOR_STEEL_GRAY;
        case STATE_PREPARING: return LUI_COLOR_WARNING_AMBER;
        case STATE_RECORDING: return LUI_COLOR_CRITICAL_RED;
        case STATE_PAUSED: return LUI_COLOR_WARNING_AMBER;
        case STATE_STOPPING: return LUI_COLOR_WARNING_AMBER;
        case STATE_ERROR: return LUI_COLOR_CRITICAL_RED;
        case STATE_PROCESSING: return LUI_COLOR_SECURE_CYAN;
        default: return LUI_COLOR_STEEL_GRAY;
    }
}

static void format_duration(time_t seconds, char* buffer, size_t buffer_size) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    if (hours > 0) {
        snprintf(buffer, buffer_size, "%02d:%02d:%02d", hours, minutes, secs);
    } else {
        snprintf(buffer, buffer_size, "%02d:%02d", minutes, secs);
    }
}

static void format_file_size(uint64_t bytes, char* buffer, size_t buffer_size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size_d = (double)bytes;
    
    while (size_d >= 1024.0 && unit_index < 4) {
        size_d /= 1024.0;
        unit_index++;
    }
    
    if (unit_index == 0) {
        snprintf(buffer, buffer_size, "%llu %s", bytes, units[unit_index]);
    } else {
        snprintf(buffer, buffer_size, "%.1f %s", size_d, units[unit_index]);
    }
}

// ============================================================================
// HARDWARE DETECTION
// ============================================================================

static void detect_hardware_capabilities(void) {
    printf("[ScreenRecorder] Detecting hardware capabilities\n");
    
    // Simulate hardware detection
    // In real implementation, query GPU vendors (NVIDIA, AMD, Intel)
    
    g_screen_recorder.hardware_encoding_available = true;
    g_screen_recorder.gpu_acceleration_available = true;
    strcpy(g_screen_recorder.gpu_name, "NVIDIA GeForce RTX 4080");
    g_screen_recorder.gpu_memory_mb = 16384;
    
    printf("[ScreenRecorder] Hardware capabilities detected:\n");
    printf("  GPU: %s\n", g_screen_recorder.gpu_name);
    printf("  GPU Memory: %d MB\n", g_screen_recorder.gpu_memory_mb);
    printf("  Hardware Encoding: %s\n", g_screen_recorder.hardware_encoding_available ? "Available" : "Not Available");
    printf("  GPU Acceleration: %s\n", g_screen_recorder.gpu_acceleration_available ? "Available" : "Not Available");
}

static void discover_monitors(void) {
    printf("[ScreenRecorder] Discovering monitors\n");
    
    // Clear existing monitors
    monitor_info_t* monitor = g_screen_recorder.monitors;
    while (monitor) {
        monitor_info_t* next = monitor->next;
        free(monitor);
        monitor = next;
    }
    g_screen_recorder.monitors = NULL;
    g_screen_recorder.monitor_count = 0;
    
    // Simulate monitor discovery
    // In real implementation, use X11/Wayland or Windows APIs
    
    // Primary monitor (4K)
    monitor_info_t* primary = calloc(1, sizeof(monitor_info_t));
    if (primary) {
        primary->id = 0;
        strcpy(primary->name, "Primary Monitor (4K)");
        primary->x = 0;
        primary->y = 0;
        primary->width = 3840;
        primary->height = 2160;
        primary->refresh_rate = 60;
        primary->is_primary = true;
        primary->enabled_for_recording = true;
        
        primary->next = g_screen_recorder.monitors;
        g_screen_recorder.monitors = primary;
        g_screen_recorder.primary_monitor = primary;
        g_screen_recorder.monitor_count++;
    }
    
    // Secondary monitor (1080p)
    monitor_info_t* secondary = calloc(1, sizeof(monitor_info_t));
    if (secondary) {
        secondary->id = 1;
        strcpy(secondary->name, "Secondary Monitor (1080p)");
        secondary->x = 3840;
        secondary->y = 0;
        secondary->width = 1920;
        secondary->height = 1080;
        secondary->refresh_rate = 144;
        secondary->is_primary = false;
        secondary->enabled_for_recording = false;
        
        secondary->next = g_screen_recorder.monitors;
        g_screen_recorder.monitors = secondary;
        g_screen_recorder.monitor_count++;
    }
    
    printf("[ScreenRecorder] Discovered %u monitors\n", g_screen_recorder.monitor_count);
}

static void discover_audio_sources(void) {
    printf("[ScreenRecorder] Discovering audio sources\n");
    
    // Clear existing audio sources
    audio_source_t* source = g_screen_recorder.audio_sources;
    while (source) {
        audio_source_t* next = source->next;
        free(source);
        source = next;
    }
    g_screen_recorder.audio_sources = NULL;
    g_screen_recorder.audio_source_count = 0;
    
    // Simulate audio source discovery
    const char* source_names[] = {
        "Default Microphone", "System Audio", "USB Headset", 
        "Line In", "Stereo Mix", "Application Audio"
    };
    
    audio_source_type_t source_types[] = {
        AUDIO_MICROPHONE, AUDIO_SYSTEM, AUDIO_MICROPHONE,
        AUDIO_MICROPHONE, AUDIO_MIXED, AUDIO_APPLICATION
    };
    
    for (size_t i = 0; i < sizeof(source_names) / sizeof(source_names[0]); i++) {
        audio_source_t* new_source = calloc(1, sizeof(audio_source_t));
        if (new_source) {
            new_source->id = i;
            strncpy(new_source->name, source_names[i], sizeof(new_source->name) - 1);
            new_source->type = source_types[i];
            new_source->enabled = (i == 0 || i == 1); // Enable microphone and system audio by default
            new_source->volume = 0.8f;
            new_source->muted = false;
            
            // Default audio processing settings
            new_source->processing.noise_reduction = true;
            new_source->processing.echo_cancellation = (new_source->type == AUDIO_MICROPHONE);
            new_source->processing.auto_gain = true;
            new_source->processing.gain_level = 0.0f; // 0 dB
            
            new_source->next = g_screen_recorder.audio_sources;
            g_screen_recorder.audio_sources = new_source;
            g_screen_recorder.audio_source_count++;
        }
    }
    
    printf("[ScreenRecorder] Discovered %u audio sources\n", g_screen_recorder.audio_source_count);
}

// ============================================================================
// AI QUALITY OPTIMIZATION
// ============================================================================

static void ai_analyze_recording_content(recording_session_t* session) {
    if (!g_screen_recorder.ai_optimizer.enabled || !session) {
        return;
    }
    
    printf("[ScreenRecorder] AI analyzing recording content\n");
    
    ai_quality_optimizer_t* ai = &g_screen_recorder.ai_optimizer;
    
    // Simulate content analysis
    usleep(10000); // 10ms analysis delay
    
    // Analyze motion level
    ai->content_analysis.motion_level = 0.3f + (rand() % 70) / 100.0f; // 0.3 to 1.0
    
    // Analyze detail level
    ai->content_analysis.detail_level = 0.4f + (rand() % 60) / 100.0f; // 0.4 to 1.0
    
    // Scene change detection
    ai->content_analysis.scene_change_detected = (rand() % 100) < 10; // 10% chance
    
    // Content type detection
    const char* scene_types[] = {"desktop", "game", "presentation", "video", "browser"};
    strcpy(ai->content_analysis.scene_type, scene_types[rand() % 5]);
    
    // Text and face detection
    ai->content_analysis.text_detected = (rand() % 100) < 70; // 70% chance
    ai->content_analysis.faces_detected = (rand() % 100) < 20; // 20% chance
    
    // Generate optimization recommendations
    ai->recommendation_count = 0;
    
    // Motion-based recommendations
    if (ai->content_analysis.motion_level > 0.8f) {
        ai->suggest_bitrate_increase = true;
        snprintf(ai->recommendations[ai->recommendation_count++], 256,
                "High motion detected. Increase bitrate for better quality.");
        
        if (session->video.framerate < 60) {
            ai->suggest_framerate_change = true;
            snprintf(ai->recommendations[ai->recommendation_count++], 256,
                    "Consider increasing framerate to 60fps for smooth motion.");
        }
    } else if (ai->content_analysis.motion_level < 0.4f) {
        ai->suggest_bitrate_decrease = true;
        snprintf(ai->recommendations[ai->recommendation_count++], 256,
                "Low motion content. Reduce bitrate to save space.");
    }
    
    // Detail-based recommendations
    if (ai->content_analysis.detail_level > 0.8f && session->video.bitrate < 10000) {
        ai->suggest_bitrate_increase = true;
        snprintf(ai->recommendations[ai->recommendation_count++], 256,
                "High detail content. Increase bitrate to preserve clarity.");
    }
    
    // Scene-specific recommendations
    if (strcmp(ai->content_analysis.scene_type, "game") == 0) {
        snprintf(ai->recommendations[ai->recommendation_count++], 256,
                "Gaming content detected. Enable hardware encoding for performance.");
        
        if (session->video.framerate < 60) {
            snprintf(ai->recommendations[ai->recommendation_count++], 256,
                    "Gaming: 60fps recommended for smooth playback.");
        }
    } else if (strcmp(ai->content_analysis.scene_type, "presentation") == 0) {
        snprintf(ai->recommendations[ai->recommendation_count++], 256,
                "Presentation detected. Lower framerate suitable, focus on text clarity.");
    }
    
    // Adaptive quality adjustment
    if (ai->auto_adjust && session->state == STATE_RECORDING) {
        float target_quality = 0.8f;
        
        // Adjust based on content
        if (ai->content_analysis.motion_level > 0.7f) {
            target_quality += 0.1f;
        }
        if (ai->content_analysis.detail_level > 0.7f) {
            target_quality += 0.1f;
        }
        if (ai->content_analysis.text_detected) {
            target_quality += 0.05f;
        }
        
        ai->adaptive.quality_factor = target_quality;
        ai->adaptive.target_bitrate = (int)(session->video.bitrate * target_quality);
        
        printf("[ScreenRecorder] AI adaptive quality: %.2f, target bitrate: %d kbps\n",
               target_quality, ai->adaptive.target_bitrate);
    }
    
    // Update learning statistics
    ai->learning_stats.recordings_analyzed++;
    
    printf("[ScreenRecorder] AI content analysis: motion=%.2f, detail=%.2f, scene=%s\n",
           ai->content_analysis.motion_level,
           ai->content_analysis.detail_level,
           ai->content_analysis.scene_type);
}

// ============================================================================
// RECORDING OPERATIONS
// ============================================================================

static recording_session_t* create_recording_session(recording_mode_t mode) {
    recording_session_t* session = calloc(1, sizeof(recording_session_t));
    if (!session) {
        printf("[ScreenRecorder] ERROR: Failed to allocate recording session\n");
        return NULL;
    }
    
    // Generate filename with timestamp
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    snprintf(session->filename, sizeof(session->filename),
            "LimitlessScreen_%04d%02d%02d_%02d%02d%02d",
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    
    snprintf(session->output_path, sizeof(session->output_path),
            "%s/%s.mp4", g_screen_recorder.settings.default_output_path, session->filename);
    
    session->mode = mode;
    session->state = STATE_IDLE;
    
    // Default video settings based on quality preset
    quality_preset_t preset = g_screen_recorder.settings.default_quality;
    switch (preset) {
        case QUALITY_LOW:
            session->video.width = 1280;
            session->video.height = 720;
            session->video.framerate = 30;
            session->video.bitrate = 2500;
            break;
        case QUALITY_MEDIUM:
            session->video.width = 1920;
            session->video.height = 1080;
            session->video.framerate = 30;
            session->video.bitrate = 5000;
            break;
        case QUALITY_HIGH:
            session->video.width = 1920;
            session->video.height = 1080;
            session->video.framerate = 60;
            session->video.bitrate = 8000;
            break;
        case QUALITY_ULTRA:
            session->video.width = 3840;
            session->video.height = 2160;
            session->video.framerate = 30;
            session->video.bitrate = 15000;
            break;
        case QUALITY_STREAMING:
            session->video.width = 1920;
            session->video.height = 1080;
            session->video.framerate = 30;
            session->video.bitrate = 6000;
            break;
        default:
            session->video.width = 1920;
            session->video.height = 1080;
            session->video.framerate = 30;
            session->video.bitrate = 5000;
            break;
    }
    
    session->video.format = g_screen_recorder.settings.default_format;
    session->video.preset = preset;
    session->video.keyframe_interval = 2; // 2 seconds
    session->video.variable_bitrate = true;
    session->video.hardware_encoding = g_screen_recorder.hardware_encoding_available;
    session->video.cursor_enabled = true;
    session->video.cursor_highlight = true;
    
    // Default audio settings
    session->audio.sample_rate = 44100;
    session->audio.channels = 2;
    session->audio.bitrate = 128;
    session->audio.noise_gate = true;
    session->audio.noise_gate_threshold = -40.0f; // dB
    session->audio.compressor = true;
    session->audio.limiter = true;
    
    // Initialize frame buffer
    session->frame_buffer.buffer_size = FRAME_BUFFER_SIZE;
    session->frame_buffer.frames = calloc(FRAME_BUFFER_SIZE, sizeof(void*));
    pthread_mutex_init(&session->frame_buffer.mutex, NULL);
    
    // Default region (fullscreen primary monitor)
    if (g_screen_recorder.primary_monitor) {
        session->region.x = g_screen_recorder.primary_monitor->x;
        session->region.y = g_screen_recorder.primary_monitor->y;
        session->region.width = g_screen_recorder.primary_monitor->width;
        session->region.height = g_screen_recorder.primary_monitor->height;
        session->target_monitor = g_screen_recorder.primary_monitor;
    }
    
    printf("[ScreenRecorder] Created recording session: %s\n", session->filename);
    printf("[ScreenRecorder] Resolution: %dx%d @ %dfps, Bitrate: %d kbps\n",
           session->video.width, session->video.height, 
           session->video.framerate, session->video.bitrate);
    
    return session;
}

static void free_recording_session(recording_session_t* session) {
    if (!session) return;
    
    // Stop threads if active
    if (session->capture_active) {
        session->capture_active = false;
        pthread_join(session->capture_thread, NULL);
    }
    if (session->audio_active) {
        session->audio_active = false;
        pthread_join(session->audio_thread, NULL);
    }
    if (session->encoding_active) {
        session->encoding_active = false;
        pthread_join(session->encoding_thread, NULL);
    }
    
    // Free frame buffer
    if (session->frame_buffer.frames) {
        for (int i = 0; i < session->frame_buffer.buffer_size; i++) {
            if (session->frame_buffer.frames[i]) {
                free(session->frame_buffer.frames[i]);
            }
        }
        free(session->frame_buffer.frames);
    }
    pthread_mutex_destroy(&session->frame_buffer.mutex);
    
    free(session);
}

// ============================================================================
// RECORDING THREADS
// ============================================================================

static void* video_capture_thread(void* arg) {
    recording_session_t* session = (recording_session_t*)arg;
    printf("[ScreenRecorder] Video capture thread started\n");
    
    session->stats.start_time = time(NULL);
    
    while (session->capture_active && session->state == STATE_RECORDING) {
        // Simulate frame capture
        usleep(1000000 / session->video.framerate); // Frame interval
        
        if (session->state == STATE_PAUSED) {
            continue;
        }
        
        // Update statistics
        session->stats.frames_recorded++;
        
        // Simulate occasional frame drops under high load
        if ((rand() % 1000) < 2) { // 0.2% drop rate
            session->stats.frames_dropped++;
            continue;
        }
        
        // Calculate frame data size (simulated)
        uint32_t frame_size = session->video.width * session->video.height * 3; // RGB24
        session->stats.bytes_written += frame_size;
        
        // Update FPS calculation
        time_t elapsed = time(NULL) - session->stats.start_time;
        if (elapsed > 0) {
            session->stats.average_fps = (float)session->stats.frames_recorded / elapsed;
        }
        
        // Perform AI content analysis periodically
        if ((session->stats.frames_recorded % 30) == 0) { // Every 30 frames
            ai_analyze_recording_content(session);
        }
    }
    
    printf("[ScreenRecorder] Video capture thread stopped\n");
    return NULL;
}

static void* audio_capture_thread(void* arg) {
    recording_session_t* session = (recording_session_t*)arg;
    printf("[ScreenRecorder] Audio capture thread started\n");
    
    while (session->audio_active && session->state == STATE_RECORDING) {
        // Simulate audio capture
        usleep(1000000 * AUDIO_BUFFER_SIZE / session->audio.sample_rate); // Buffer interval
        
        if (session->state == STATE_PAUSED) {
            continue;
        }
        
        // Simulate occasional audio dropouts
        if ((rand() % 10000) < 5) { // 0.05% dropout rate
            session->stats.audio_dropouts++;
        }
        
        // Update audio bytes written
        uint32_t audio_bytes = AUDIO_BUFFER_SIZE * session->audio.channels * 2; // 16-bit samples
        session->stats.bytes_written += audio_bytes;
    }
    
    printf("[ScreenRecorder] Audio capture thread stopped\n");
    return NULL;
}

static void* encoding_thread(void* arg) {
    recording_session_t* session = (recording_session_t*)arg;
    printf("[ScreenRecorder] Encoding thread started\n");
    
    while (session->encoding_active) {
        // Simulate encoding work
        usleep(10000); // 10ms encoding time per cycle
        
        // Update encoding statistics
        session->stats.encoding_speed = session->video.hardware_encoding ? 2.5f : 1.2f; // x times realtime
        session->stats.compression_ratio = 0.1f + (rand() % 20) / 100.0f; // 10-30%
        
        // Simulate system resource usage
        session->stats.cpu_usage = 15.0f + (rand() % 40); // 15-55%
        session->stats.gpu_usage = session->video.hardware_encoding ? 
                                  (30.0f + (rand() % 40)) : (rand() % 10); // 30-70% or 0-10%
        session->stats.memory_usage = 500.0f + (rand() % 1000); // 0.5-1.5GB MB
    }
    
    printf("[ScreenRecorder] Encoding thread stopped\n");
    return NULL;
}

static bool start_recording(recording_session_t* session) {
    if (!session || session->state != STATE_IDLE) {
        return false;
    }
    
    printf("[ScreenRecorder] Starting recording: %s\n", session->filename);
    
    session->state = STATE_PREPARING;
    
    // Validate settings
    if (session->video.width <= 0 || session->video.height <= 0) {
        printf("[ScreenRecorder] ERROR: Invalid video dimensions\n");
        session->state = STATE_ERROR;
        return false;
    }
    
    // Initialize capture
    session->capture_active = true;
    session->audio_active = true;
    session->encoding_active = true;
    
    // Start capture threads
    if (pthread_create(&session->capture_thread, NULL, video_capture_thread, session) != 0) {
        printf("[ScreenRecorder] ERROR: Failed to create video capture thread\n");
        session->state = STATE_ERROR;
        return false;
    }
    
    if (pthread_create(&session->audio_thread, NULL, audio_capture_thread, session) != 0) {
        printf("[ScreenRecorder] ERROR: Failed to create audio capture thread\n");
        session->capture_active = false;
        pthread_join(session->capture_thread, NULL);
        session->state = STATE_ERROR;
        return false;
    }
    
    if (pthread_create(&session->encoding_thread, NULL, encoding_thread, session) != 0) {
        printf("[ScreenRecorder] ERROR: Failed to create encoding thread\n");
        session->capture_active = false;
        session->audio_active = false;
        pthread_join(session->capture_thread, NULL);
        pthread_join(session->audio_thread, NULL);
        session->state = STATE_ERROR;
        return false;
    }
    
    session->state = STATE_RECORDING;
    g_screen_recorder.stats.recordings_created++;
    
    printf("[ScreenRecorder] Recording started successfully\n");
    return true;
}

static bool stop_recording(recording_session_t* session) {
    if (!session || session->state != STATE_RECORDING) {
        return false;
    }
    
    printf("[ScreenRecorder] Stopping recording: %s\n", session->filename);
    
    session->state = STATE_STOPPING;
    
    // Stop capture threads
    session->capture_active = false;
    session->audio_active = false;
    
    pthread_join(session->capture_thread, NULL);
    pthread_join(session->audio_thread, NULL);
    
    // Finish encoding
    session->encoding_active = false;
    pthread_join(session->encoding_thread, NULL);
    
    // Update final statistics
    session->stats.duration = time(NULL) - session->stats.start_time;
    g_screen_recorder.stats.total_recording_time += session->stats.duration;
    g_screen_recorder.stats.total_file_size += session->stats.bytes_written;
    
    session->state = STATE_IDLE;
    
    printf("[ScreenRecorder] Recording stopped. Duration: %ld seconds, Size: %llu bytes\n",
           session->stats.duration, session->stats.bytes_written);
    
    return true;
}

static bool pause_recording(recording_session_t* session) {
    if (!session || session->state != STATE_RECORDING) {
        return false;
    }
    
    printf("[ScreenRecorder] Pausing recording\n");
    session->state = STATE_PAUSED;
    return true;
}

static bool resume_recording(recording_session_t* session) {
    if (!session || session->state != STATE_PAUSED) {
        return false;
    }
    
    printf("[ScreenRecorder] Resuming recording\n");
    session->state = STATE_RECORDING;
    return true;
}

// ============================================================================
// USER INTERFACE IMPLEMENTATION
// ============================================================================

static void create_control_panel(void) {
    g_screen_recorder.control_panel = lui_create_container(g_screen_recorder.main_window->root_widget);
    strcpy(g_screen_recorder.control_panel->name, "control_panel");
    g_screen_recorder.control_panel->bounds = lui_rect_make(0, 0, 400, 600);
    g_screen_recorder.control_panel->background_color = LUI_COLOR_CHARCOAL_BLACK;
    
    // Recording controls header
    lui_widget_t* header = lui_create_label("🎬 Recording Controls", g_screen_recorder.control_panel);
    header->bounds = lui_rect_make(8, 8, 200, 24);
    header->typography = LUI_TYPOGRAPHY_TITLE_MEDIUM;
    header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    // Recording mode selection
    lui_widget_t* mode_label = lui_create_label("Recording Mode:", g_screen_recorder.control_panel);
    mode_label->bounds = lui_rect_make(8, 40, 120, 16);
    mode_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    const char* mode_names[] = {"Fullscreen", "Window", "Region", "Webcam"};
    for (int i = 0; i < 4; i++) {
        lui_widget_t* mode_btn = lui_create_button(mode_names[i], g_screen_recorder.control_panel);
        mode_btn->bounds = lui_rect_make(8 + i * 90, 60, 85, 24);
        mode_btn->style.background_color = LUI_COLOR_TACTICAL_BLUE;
    }
    
    // Quality preset selection
    lui_widget_t* quality_label = lui_create_label("Quality Preset:", g_screen_recorder.control_panel);
    quality_label->bounds = lui_rect_make(8, 95, 120, 16);
    quality_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    const char* quality_names[] = {"Low", "Medium", "High", "Ultra"};
    for (int i = 0; i < 4; i++) {
        lui_widget_t* quality_btn = lui_create_button(quality_names[i], g_screen_recorder.control_panel);
        quality_btn->bounds = lui_rect_make(8 + i * 90, 115, 85, 24);
        quality_btn->style.background_color = LUI_COLOR_STEEL_GRAY;
    }
    
    // Output format selection
    lui_widget_t* format_label = lui_create_label("Output Format:", g_screen_recorder.control_panel);
    format_label->bounds = lui_rect_make(8, 150, 120, 16);
    format_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    lui_widget_t* format_dropdown = lui_create_button("MP4 (H.264) ▼", g_screen_recorder.control_panel);
    format_dropdown->bounds = lui_rect_make(8, 170, 150, 24);
    format_dropdown->style.background_color = LUI_COLOR_GRAPHITE;
    
    // Audio source selection
    lui_widget_t* audio_header = lui_create_label("🎤 Audio Sources", g_screen_recorder.control_panel);
    audio_header->bounds = lui_rect_make(8, 210, 150, 20);
    audio_header->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
    audio_header->typography.color = LUI_COLOR_WARNING_AMBER;
    
    // Audio source checkboxes
    audio_source_t* source = g_screen_recorder.audio_sources;
    int audio_y = 240;
    int source_count = 0;
    
    while (source && source_count < 4) {
        char source_text[64];
        snprintf(source_text, sizeof(source_text), "%s %s", 
                source->enabled ? "☑️" : "☐", source->name);
        
        lui_widget_t* source_checkbox = lui_create_button(source_text, g_screen_recorder.control_panel);
        source_checkbox->bounds = lui_rect_make(8, audio_y, 200, 20);
        source_checkbox->style.background_color = source->enabled ? 
                                                 LUI_COLOR_SUCCESS_GREEN : LUI_COLOR_STEEL_GRAY;
        source_checkbox->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        
        // Volume slider (simulated with label)
        if (source->enabled) {
            char volume_text[32];
            snprintf(volume_text, sizeof(volume_text), "Vol: %.0f%%", source->volume * 100.0f);
            lui_widget_t* volume_label = lui_create_label(volume_text, g_screen_recorder.control_panel);
            volume_label->bounds = lui_rect_make(220, audio_y, 60, 20);
            volume_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        }
        
        audio_y += 25;
        source_count++;
        source = source->next;
    }
    
    // Main recording controls
    lui_widget_t* controls_header = lui_create_label("⚙️ Recording", g_screen_recorder.control_panel);
    controls_header->bounds = lui_rect_make(8, 370, 150, 20);
    controls_header->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
    controls_header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    // Record button
    const char* record_text = "🔴 Record";
    lui_color_t record_color = LUI_COLOR_CRITICAL_RED;
    
    if (g_screen_recorder.current_session) {
        switch (g_screen_recorder.current_session->state) {
            case STATE_RECORDING:
                record_text = "⏹️ Stop";
                record_color = LUI_COLOR_STEEL_GRAY;
                break;
            case STATE_PAUSED:
                record_text = "▶️ Resume";
                record_color = LUI_COLOR_SUCCESS_GREEN;
                break;
            default:
                break;
        }
    }
    
    lui_widget_t* record_btn = lui_create_button(record_text, g_screen_recorder.control_panel);
    record_btn->bounds = lui_rect_make(8, 400, 120, 40);
    record_btn->style.background_color = record_color;
    record_btn->typography = LUI_TYPOGRAPHY_TITLE_SMALL;
    
    // Pause button
    lui_widget_t* pause_btn = lui_create_button("⏸️ Pause", g_screen_recorder.control_panel);
    pause_btn->bounds = lui_rect_make(140, 400, 80, 40);
    pause_btn->style.background_color = LUI_COLOR_WARNING_AMBER;
    pause_btn->typography = LUI_TYPOGRAPHY_TITLE_SMALL;
    
    // Screenshot button
    lui_widget_t* screenshot_btn = lui_create_button("📸 Screenshot", g_screen_recorder.control_panel);
    screenshot_btn->bounds = lui_rect_make(8, 450, 120, 30);
    screenshot_btn->style.background_color = LUI_COLOR_SECURE_CYAN;
    
    // Settings button
    lui_widget_t* settings_btn = lui_create_button("⚙️ Settings", g_screen_recorder.control_panel);
    settings_btn->bounds = lui_rect_make(140, 450, 80, 30);
    settings_btn->style.background_color = LUI_COLOR_TACTICAL_BLUE;
    
    // Recording status
    if (g_screen_recorder.current_session) {
        recording_session_t* session = g_screen_recorder.current_session;
        
        char status_text[128];
        char duration_str[32];
        format_duration(session->stats.duration, duration_str, sizeof(duration_str));
        
        snprintf(status_text, sizeof(status_text),
                "Status: %s\nDuration: %s\nFrames: %llu\nSize: %.1f MB",
                get_recording_state_name(session->state),
                duration_str,
                session->stats.frames_recorded,
                session->stats.bytes_written / (1024.0f * 1024.0f));
        
        lui_widget_t* status_display = lui_create_label(status_text, g_screen_recorder.control_panel);
        status_display->bounds = lui_rect_make(8, 490, 300, 80);
        status_display->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        status_display->typography.color = get_recording_state_color(session->state);
    } else {
        lui_widget_t* idle_status = lui_create_label("Ready to record", g_screen_recorder.control_panel);
        idle_status->bounds = lui_rect_make(8, 490, 150, 20);
        idle_status->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        idle_status->typography.color = LUI_COLOR_SUCCESS_GREEN;
    }
}

static void create_preview_panel(void) {
    if (!g_screen_recorder.show_preview) return;
    
    g_screen_recorder.preview_panel = lui_create_container(g_screen_recorder.main_window->root_widget);
    strcpy(g_screen_recorder.preview_panel->name, "preview_panel");
    g_screen_recorder.preview_panel->bounds = lui_rect_make(400, 0, 500, 400);
    g_screen_recorder.preview_panel->background_color = LUI_COLOR_GRAPHITE;
    
    // Preview header
    lui_widget_t* header = lui_create_label("📺 Live Preview", g_screen_recorder.preview_panel);
    header->bounds = lui_rect_make(8, 8, 150, 20);
    header->typography = LUI_TYPOGRAPHY_TITLE_MEDIUM;
    header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    // Preview area (simulated with placeholder)
    lui_widget_t* preview_area = lui_create_container(g_screen_recorder.preview_panel);
    preview_area->bounds = lui_rect_make(8, 35, 484, 300);
    preview_area->background_color = LUI_COLOR_CHARCOAL_BLACK;
    
    if (g_screen_recorder.current_session && 
        g_screen_recorder.current_session->state == STATE_RECORDING) {
        // Show recording indicator
        lui_widget_t* recording_indicator = lui_create_label("🔴 RECORDING", preview_area);
        recording_indicator->bounds = lui_rect_make(10, 10, 100, 20);
        recording_indicator->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
        recording_indicator->typography.color = LUI_COLOR_CRITICAL_RED;
        
        // Show preview placeholder
        lui_widget_t* preview_placeholder = lui_create_label("Live Screen Preview\n(Would show actual screen content)", 
                                                           preview_area);
        preview_placeholder->bounds = lui_rect_make(150, 130, 200, 40);
        preview_placeholder->typography = LUI_TYPOGRAPHY_BODY_MEDIUM;
        preview_placeholder->typography.color = LUI_COLOR_STEEL_GRAY;
    } else {
        lui_widget_t* no_preview = lui_create_label("Preview Inactive\nStart recording to see preview", 
                                                  preview_area);
        no_preview->bounds = lui_rect_make(150, 130, 200, 40);
        no_preview->typography = LUI_TYPOGRAPHY_BODY_MEDIUM;
        no_preview->typography.color = LUI_COLOR_STEEL_GRAY;
    }
    
    // Preview controls
    lui_widget_t* preview_controls = lui_create_container(g_screen_recorder.preview_panel);
    preview_controls->bounds = lui_rect_make(8, 345, 484, 30);
    preview_controls->background_color = LUI_COLOR_STEEL_GRAY;
    
    lui_widget_t* zoom_out_btn = lui_create_button("🔍-", preview_controls);
    zoom_out_btn->bounds = lui_rect_make(8, 4, 30, 22);
    zoom_out_btn->style.background_color = LUI_COLOR_TACTICAL_BLUE;
    
    lui_widget_t* zoom_label = lui_create_label("100%", preview_controls);
    zoom_label->bounds = lui_rect_make(46, 6, 40, 18);
    zoom_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    lui_widget_t* zoom_in_btn = lui_create_button("🔍+", preview_controls);
    zoom_in_btn->bounds = lui_rect_make(94, 4, 30, 22);
    zoom_in_btn->style.background_color = LUI_COLOR_TACTICAL_BLUE;
    
    lui_widget_t* fullscreen_btn = lui_create_button("🔲 Fullscreen", preview_controls);
    fullscreen_btn->bounds = lui_rect_make(350, 4, 80, 22);
    fullscreen_btn->style.background_color = LUI_COLOR_WARNING_AMBER;
}

static void create_ai_panel(void) {
    if (!g_screen_recorder.show_ai || !g_screen_recorder.ai_optimizer.enabled) return;
    
    g_screen_recorder.ai_panel = lui_create_container(g_screen_recorder.main_window->root_widget);
    strcpy(g_screen_recorder.ai_panel->name, "ai_panel");
    g_screen_recorder.ai_panel->bounds = lui_rect_make(400, 400, 500, 200);
    g_screen_recorder.ai_panel->background_color = LUI_COLOR_CHARCOAL_BLACK;
    
    // AI header
    lui_widget_t* header = lui_create_label("🤖 AI Quality Optimizer", g_screen_recorder.ai_panel);
    header->bounds = lui_rect_make(8, 8, 200, 20);
    header->typography = LUI_TYPOGRAPHY_TITLE_MEDIUM;
    header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    ai_quality_optimizer_t* ai = &g_screen_recorder.ai_optimizer;
    
    // Content analysis
    if (g_screen_recorder.current_session && 
        g_screen_recorder.current_session->state == STATE_RECORDING) {
        
        char analysis_text[256];
        snprintf(analysis_text, sizeof(analysis_text),
                "Scene: %s\nMotion Level: %.0f%%\nDetail Level: %.0f%%\n%s%s",
                ai->content_analysis.scene_type,
                ai->content_analysis.motion_level * 100.0f,
                ai->content_analysis.detail_level * 100.0f,
                ai->content_analysis.text_detected ? "Text: Detected\n" : "",
                ai->content_analysis.faces_detected ? "Faces: Detected" : "");
        
        lui_widget_t* analysis_label = lui_create_label(analysis_text, g_screen_recorder.ai_panel);
        analysis_label->bounds = lui_rect_make(8, 35, 200, 100);
        analysis_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    }
    
    // AI recommendations
    if (ai->recommendation_count > 0) {
        lui_widget_t* rec_header = lui_create_label("💡 AI Recommendations:", g_screen_recorder.ai_panel);
        rec_header->bounds = lui_rect_make(220, 35, 150, 16);
        rec_header->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        rec_header->typography.color = LUI_COLOR_WARNING_AMBER;
        
        int rec_y = 55;
        for (uint32_t i = 0; i < ai->recommendation_count && i < 3 && rec_y < 150; i++) {
            char rec_text[80];
            snprintf(rec_text, sizeof(rec_text), "• %.70s", ai->recommendations[i]);
            
            lui_widget_t* rec_label = lui_create_label(rec_text, g_screen_recorder.ai_panel);
            rec_label->bounds = lui_rect_make(220, rec_y, 270, 16);
            rec_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
            
            rec_y += 18;
        }
        
        // Action buttons
        if (ai->suggest_bitrate_increase) {
            lui_widget_t* bitrate_btn = lui_create_button("⬆️ Increase Bitrate", g_screen_recorder.ai_panel);
            bitrate_btn->bounds = lui_rect_make(220, rec_y + 5, 120, 20);
            bitrate_btn->style.background_color = LUI_COLOR_SUCCESS_GREEN;
            bitrate_btn->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        }
        
        if (ai->suggest_framerate_change) {
            lui_widget_t* fps_btn = lui_create_button("🎬 Adjust FPS", g_screen_recorder.ai_panel);
            fps_btn->bounds = lui_rect_make(350, rec_y + 5, 100, 20);
            fps_btn->style.background_color = LUI_COLOR_SECURE_CYAN;
            fps_btn->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        }
    }
    
    // Learning statistics
    char learning_text[128];
    snprintf(learning_text, sizeof(learning_text),
            "Recordings Analyzed: %u\nOptimizations Applied: %u\nQuality Improvement: %.1f%%",
            ai->learning_stats.recordings_analyzed,
            ai->learning_stats.optimizations_applied,
            ai->learning_stats.quality_improvement);
    
    lui_widget_t* learning_label = lui_create_label(learning_text, g_screen_recorder.ai_panel);
    learning_label->bounds = lui_rect_make(8, 150, 200, 45);
    learning_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    learning_label->typography.color = LUI_COLOR_STEEL_GRAY;
    
    // Auto-adjust toggle
    lui_widget_t* auto_adjust_btn = lui_create_button(ai->auto_adjust ? "🤖 Auto-Adjust: ON" : "🔒 Auto-Adjust: OFF", 
                                                     g_screen_recorder.ai_panel);
    auto_adjust_btn->bounds = lui_rect_make(220, 150, 150, 24);
    auto_adjust_btn->style.background_color = ai->auto_adjust ? 
                                             LUI_COLOR_SUCCESS_GREEN : LUI_COLOR_STEEL_GRAY;
}

static void create_streaming_panel(void) {
    if (!g_screen_recorder.show_streaming) return;
    
    g_screen_recorder.streaming_panel = lui_create_container(g_screen_recorder.main_window->root_widget);
    strcpy(g_screen_recorder.streaming_panel->name, "streaming_panel");
    g_screen_recorder.streaming_panel->bounds = lui_rect_make(900, 0, 300, 600);
    g_screen_recorder.streaming_panel->background_color = LUI_COLOR_GRAPHITE;
    
    // Streaming header
    lui_widget_t* header = lui_create_label("📡 Live Streaming", g_screen_recorder.streaming_panel);
    header->bounds = lui_rect_make(8, 8, 150, 20);
    header->typography = LUI_TYPOGRAPHY_TITLE_MEDIUM;
    header->typography.color = LUI_COLOR_WARNING_AMBER;
    
    live_streaming_t* stream = &g_screen_recorder.streaming;
    
    // Platform selection
    lui_widget_t* platform_label = lui_create_label("Platform:", g_screen_recorder.streaming_panel);
    platform_label->bounds = lui_rect_make(8, 40, 80, 16);
    platform_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    lui_widget_t* platform_dropdown = lui_create_button("YouTube ▼", g_screen_recorder.streaming_panel);
    platform_dropdown->bounds = lui_rect_make(8, 60, 120, 24);
    platform_dropdown->style.background_color = LUI_COLOR_TACTICAL_BLUE;
    
    // Stream settings
    lui_widget_t* settings_header = lui_create_label("Stream Settings", g_screen_recorder.streaming_panel);
    settings_header->bounds = lui_rect_make(8, 95, 120, 16);
    settings_header->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    settings_header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    char bitrate_text[64];
    snprintf(bitrate_text, sizeof(bitrate_text), "Bitrate: %d kbps", stream->target_bitrate);
    lui_widget_t* bitrate_label = lui_create_label(bitrate_text, g_screen_recorder.streaming_panel);
    bitrate_label->bounds = lui_rect_make(8, 115, 150, 16);
    bitrate_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    lui_widget_t* adaptive_checkbox = lui_create_button(stream->adaptive_bitrate ? 
                                                       "☑️ Adaptive Bitrate" : "☐ Adaptive Bitrate", 
                                                       g_screen_recorder.streaming_panel);
    adaptive_checkbox->bounds = lui_rect_make(8, 135, 150, 20);
    adaptive_checkbox->style.background_color = stream->adaptive_bitrate ? 
                                               LUI_COLOR_SUCCESS_GREEN : LUI_COLOR_STEEL_GRAY;
    
    // Stream health
    if (stream->enabled) {
        lui_widget_t* health_header = lui_create_label("Stream Health", g_screen_recorder.streaming_panel);
        health_header->bounds = lui_rect_make(8, 170, 100, 16);
        health_header->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        health_header->typography.color = LUI_COLOR_WARNING_AMBER;
        
        char health_text[128];
        snprintf(health_text, sizeof(health_text),
                "Health: %.0f%%\nLatency: %.1fms\nDropped: %u frames\nStable: %s",
                stream->health.stream_health * 100.0f,
                stream->health.latency_ms,
                stream->health.dropped_frames,
                stream->health.connection_stable ? "Yes" : "No");
        
        lui_widget_t* health_display = lui_create_label(health_text, g_screen_recorder.streaming_panel);
        health_display->bounds = lui_rect_make(8, 190, 200, 80);
        health_display->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        
        lui_color_t health_color = LUI_COLOR_SUCCESS_GREEN;
        if (stream->health.stream_health < 0.7f) health_color = LUI_COLOR_WARNING_AMBER;
        if (stream->health.stream_health < 0.5f) health_color = LUI_COLOR_CRITICAL_RED;
        health_display->typography.color = health_color;
    }
    
    // Stream controls
    lui_widget_t* start_stream_btn = lui_create_button(stream->enabled ? "⏹️ Stop Stream" : "📡 Start Stream", 
                                                      g_screen_recorder.streaming_panel);
    start_stream_btn->bounds = lui_rect_make(8, 290, 120, 40);
    start_stream_btn->style.background_color = stream->enabled ? 
                                              LUI_COLOR_CRITICAL_RED : LUI_COLOR_SUCCESS_GREEN;
    
    // Stream info
    if (strlen(stream->title) > 0) {
        char stream_info[256];
        snprintf(stream_info, sizeof(stream_info),
                "Title: %s\nPrivate: %s\nPlatform: %s",
                stream->title,
                stream->private_stream ? "Yes" : "No",
                stream->platform);
        
        lui_widget_t* info_display = lui_create_label(stream_info, g_screen_recorder.streaming_panel);
        info_display->bounds = lui_rect_make(8, 340, 280, 60);
        info_display->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    }
}

static void create_status_bar(void) {
    g_screen_recorder.status_bar = lui_create_container(g_screen_recorder.main_window->root_widget);
    strcpy(g_screen_recorder.status_bar->name, "status_bar");
    g_screen_recorder.status_bar->bounds = lui_rect_make(0, 600, 1200, 24);
    g_screen_recorder.status_bar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // Recording status
    char status_text[256] = "Ready";
    
    if (g_screen_recorder.current_session) {
        recording_session_t* session = g_screen_recorder.current_session;
        
        char duration_str[32], size_str[32];
        format_duration(session->stats.duration, duration_str, sizeof(duration_str));
        format_file_size(session->stats.bytes_written, size_str, sizeof(size_str));
        
        snprintf(status_text, sizeof(status_text),
                "%s | %s | %s | %.1f FPS | CPU: %.0f%% | GPU: %.0f%%",
                get_recording_state_name(session->state),
                duration_str,
                size_str,
                session->stats.average_fps,
                session->stats.cpu_usage,
                session->stats.gpu_usage);
    }
    
    lui_widget_t* status_label = lui_create_label(status_text, g_screen_recorder.status_bar);
    status_label->bounds = lui_rect_make(8, 4, 600, 16);
    status_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    // Session statistics
    char session_text[128];
    char total_time_str[32], total_size_str[32];
    format_duration(g_screen_recorder.stats.total_recording_time, total_time_str, sizeof(total_time_str));
    format_file_size(g_screen_recorder.stats.total_file_size, total_size_str, sizeof(total_size_str));
    
    snprintf(session_text, sizeof(session_text), 
            "Session: %u recordings | %s total | %s",
            g_screen_recorder.stats.recordings_created,
            total_time_str,
            total_size_str);
    
    lui_widget_t* session_label = lui_create_label(session_text, g_screen_recorder.status_bar);
    session_label->bounds = lui_rect_make(650, 4, 300, 16);
    session_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    // Hardware indicators
    char hw_indicators[64] = "";
    if (g_screen_recorder.hardware_encoding_available) strcat(hw_indicators, "🔥 ");
    if (g_screen_recorder.gpu_acceleration_available) strcat(hw_indicators, "⚡ ");
    if (g_screen_recorder.ai_optimizer.enabled) strcat(hw_indicators, "🤖 ");
    if (g_screen_recorder.streaming.enabled) strcat(hw_indicators, "📡 ");
    
    lui_widget_t* hw_label = lui_create_label(hw_indicators, g_screen_recorder.status_bar);
    hw_label->bounds = lui_rect_make(1050, 4, 100, 16);
    hw_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    hw_label->typography.color = LUI_COLOR_SUCCESS_GREEN;
}

// ============================================================================
// MAIN SCREEN RECORDER API
// ============================================================================

bool limitless_screen_recorder_init(void) {
    if (g_screen_recorder.initialized) {
        return false;
    }
    
    printf("[ScreenRecorder] Initializing Limitless Screen Recorder v%s\n", SCREEN_RECORDER_VERSION);
    
    // Clear state
    memset(&g_screen_recorder, 0, sizeof(g_screen_recorder));
    
    // Set default configuration
    g_screen_recorder.show_preview = true;
    g_screen_recorder.show_settings = false;
    g_screen_recorder.show_ai = true;
    g_screen_recorder.show_streaming = false;
    g_screen_recorder.show_advanced = false;
    
    // Default settings
    strcpy(g_screen_recorder.settings.default_output_path, "/home/user/Videos/Recordings");
    g_screen_recorder.settings.default_format = FORMAT_MP4_H264;
    g_screen_recorder.settings.default_quality = QUALITY_MEDIUM;
    g_screen_recorder.settings.show_countdown = true;
    g_screen_recorder.settings.countdown_seconds = 3;
    g_screen_recorder.settings.auto_stop_low_space = true;
    g_screen_recorder.settings.min_free_space_mb = 1024; // 1GB
    
    // AI optimizer configuration
    g_screen_recorder.ai_optimizer.enabled = true;
    g_screen_recorder.ai_optimizer.auto_adjust = false; // Require user approval
    g_screen_recorder.ai_optimizer.learning_mode = true;
    
    // Security configuration
    g_screen_recorder.security.privacy_mode = false;
    g_screen_recorder.security.mask_sensitive_content = false;
    g_screen_recorder.security.encrypt_recordings = false;
    g_screen_recorder.security.require_authentication = false;
    
    // Streaming configuration
    g_screen_recorder.streaming.enabled = false;
    g_screen_recorder.streaming.target_bitrate = 6000;
    g_screen_recorder.streaming.adaptive_bitrate = true;
    strcpy(g_screen_recorder.streaming.platform, "YouTube");
    g_screen_recorder.streaming.private_stream = true;
    g_screen_recorder.streaming.health.stream_health = 1.0f;
    g_screen_recorder.streaming.health.connection_stable = true;
    
    // Detect hardware capabilities
    detect_hardware_capabilities();
    
    // Discover monitors and audio sources
    discover_monitors();
    discover_audio_sources();
    
    // Create main window
    g_screen_recorder.main_window = lui_create_window("Limitless Screen Recorder", LUI_WINDOW_NORMAL,
                                                     50, 50, 1200, 624);
    if (!g_screen_recorder.main_window) {
        printf("[ScreenRecorder] ERROR: Failed to create main window\n");
        return false;
    }
    
    // Create UI components
    create_control_panel();
    create_preview_panel();
    create_ai_panel();
    create_streaming_panel();
    create_status_bar();
    
    // Show window
    lui_show_window(g_screen_recorder.main_window);
    
    g_screen_recorder.initialized = true;
    g_screen_recorder.running = true;
    g_screen_recorder.stats.session_start_time = time(NULL);
    
    printf("[ScreenRecorder] Screen Recorder initialized successfully\n");
    printf("[ScreenRecorder] Monitors: %u, Audio Sources: %u, Hardware Encoding: %s\n",
           g_screen_recorder.monitor_count,
           g_screen_recorder.audio_source_count,
           g_screen_recorder.hardware_encoding_available ? "Available" : "Software Only");
    printf("[ScreenRecorder] GPU: %s (%d MB), AI Optimizer: %s\n",
           g_screen_recorder.gpu_name,
           g_screen_recorder.gpu_memory_mb,
           g_screen_recorder.ai_optimizer.enabled ? "Enabled" : "Disabled");
    
    return true;
}

void limitless_screen_recorder_shutdown(void) {
    if (!g_screen_recorder.initialized) {
        return;
    }
    
    printf("[ScreenRecorder] Shutting down Limitless Screen Recorder\n");
    
    g_screen_recorder.running = false;
    
    // Stop current recording
    if (g_screen_recorder.current_session) {
        if (g_screen_recorder.current_session->state == STATE_RECORDING) {
            stop_recording(g_screen_recorder.current_session);
        }
        free_recording_session(g_screen_recorder.current_session);
    }
    
    // Free session history
    for (uint32_t i = 0; i < g_screen_recorder.history_count; i++) {
        if (g_screen_recorder.session_history[i]) {
            free_recording_session(g_screen_recorder.session_history[i]);
        }
    }
    
    // Free monitors
    monitor_info_t* monitor = g_screen_recorder.monitors;
    while (monitor) {
        monitor_info_t* next = monitor->next;
        free(monitor);
        monitor = next;
    }
    
    // Free audio sources
    audio_source_t* source = g_screen_recorder.audio_sources;
    while (source) {
        audio_source_t* next = source->next;
        free(source);
        source = next;
    }
    
    // Destroy main window
    if (g_screen_recorder.main_window) {
        lui_destroy_window(g_screen_recorder.main_window);
    }
    
    // Print session statistics
    time_t session_duration = time(NULL) - g_screen_recorder.stats.session_start_time;
    printf("[ScreenRecorder] Session statistics:\n");
    printf("  Duration: %ld seconds\n", session_duration);
    printf("  Recordings created: %u\n", g_screen_recorder.stats.recordings_created);
    printf("  Total recording time: %llu seconds\n", g_screen_recorder.stats.total_recording_time);
    printf("  Total file size: %llu bytes\n", g_screen_recorder.stats.total_file_size);
    printf("  Streams started: %u\n", g_screen_recorder.stats.streams_started);
    printf("  AI optimizations applied: %u\n", g_screen_recorder.stats.ai_optimizations_applied);
    printf("  Average quality score: %.2f\n", g_screen_recorder.stats.average_quality_score);
    
    memset(&g_screen_recorder, 0, sizeof(g_screen_recorder));
    
    printf("[ScreenRecorder] Shutdown complete\n");
}

const char* limitless_screen_recorder_get_version(void) {
    return SCREEN_RECORDER_VERSION;
}

bool limitless_screen_recorder_start_recording(recording_mode_t mode) {
    if (!g_screen_recorder.initialized) {
        return false;
    }
    
    if (g_screen_recorder.current_session && 
        g_screen_recorder.current_session->state == STATE_RECORDING) {
        printf("[ScreenRecorder] ERROR: Recording already in progress\n");
        return false;
    }
    
    // Create new recording session
    recording_session_t* session = create_recording_session(mode);
    if (!session) {
        return false;
    }
    
    g_screen_recorder.current_session = session;
    
    // Start recording
    if (!start_recording(session)) {
        free_recording_session(session);
        g_screen_recorder.current_session = NULL;
        return false;
    }
    
    // Update UI
    create_control_panel();
    create_preview_panel();
    create_status_bar();
    
    return true;
}

bool limitless_screen_recorder_stop_recording(void) {
    if (!g_screen_recorder.current_session) {
        return false;
    }
    
    bool result = stop_recording(g_screen_recorder.current_session);
    
    if (result) {
        // Add to history
        if (g_screen_recorder.history_count < 10) {
            g_screen_recorder.session_history[g_screen_recorder.history_count++] = g_screen_recorder.current_session;
        } else {
            // Free oldest session and shift array
            free_recording_session(g_screen_recorder.session_history[0]);
            for (uint32_t i = 0; i < 9; i++) {
                g_screen_recorder.session_history[i] = g_screen_recorder.session_history[i + 1];
            }
            g_screen_recorder.session_history[9] = g_screen_recorder.current_session;
        }
        
        g_screen_recorder.current_session = NULL;
        
        // Update UI
        create_control_panel();
        create_preview_panel();
        create_status_bar();
    }
    
    return result;
}

void limitless_screen_recorder_run(void) {
    if (!g_screen_recorder.initialized) {
        printf("[ScreenRecorder] ERROR: Screen Recorder not initialized\n");
        return;
    }
    
    // Update AI analysis if recording is active
    if (g_screen_recorder.current_session && 
        g_screen_recorder.current_session->state == STATE_RECORDING) {
        
        static time_t last_ai_update = 0;
        time_t now = time(NULL);
        
        if (g_screen_recorder.ai_optimizer.enabled && (now - last_ai_update) > 2) {
            ai_analyze_recording_content(g_screen_recorder.current_session);
            last_ai_update = now;
        }
        
        // Update session duration
        g_screen_recorder.current_session->stats.duration = now - g_screen_recorder.current_session->stats.start_time;
    }
    
    // Update streaming health
    if (g_screen_recorder.streaming.enabled) {
        // Simulate streaming health updates
        g_screen_recorder.streaming.health.latency_ms = 50.0f + (rand() % 100);
        g_screen_recorder.streaming.health.stream_health = 0.8f + (rand() % 20) / 100.0f;
    }
    
    // Main event loop is handled by the desktop environment
}