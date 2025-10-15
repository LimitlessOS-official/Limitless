/*
 * LimitlessOS Multimedia & Gaming Implementation
 * Hardware-accelerated media codecs, gaming support, VR/AR, and streaming
 */

#include "multimedia_gaming.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <alsa/asoundlib.h>
#include <pulse/pulseaudio.h>
#include <dirent.h>
#include <math.h>

/* Global multimedia system */
multimedia_system_t multimedia_system = {0};

/* Threading support */
static pthread_mutex_t multimedia_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t controller_thread;
static pthread_t vr_thread;
static bool multimedia_running = false;

/* Controller input monitoring thread */
static void *controller_monitor_thread(void *arg) {
    while (multimedia_running) {
        for (uint32_t i = 0; i < multimedia_system.gaming.controller_count; i++) {
            gaming_controller_t *controller = &multimedia_system.gaming.controllers[i];
            
            if (!controller->state.connected) continue;
            
            /* Read controller input */
            int fd = open(controller->device_path, O_RDONLY | O_NONBLOCK);
            if (fd >= 0) {
                struct input_event ev;
                while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
                    if (ev.type == EV_KEY) {
                        /* Button event */
                        if (ev.value) {
                            controller->state.buttons |= (1 << ev.code);
                        } else {
                            controller->state.buttons &= ~(1 << ev.code);
                        }
                    } else if (ev.type == EV_ABS) {
                        /* Axis event */
                        if (ev.code < 16) {
                            controller->state.axes[ev.code] = ev.value;
                        }
                    }
                }
                close(fd);
                
                multimedia_system.stats.controller_inputs++;
            }
        }
        
        /* Update performance metrics */
        if (multimedia_system.gaming.game_mode) {
            /* Read GPU utilization from sysfs */
            FILE *gpu_fp = fopen("/sys/class/drm/card0/device/gpu_busy_percent", "r");
            if (gpu_fp) {
                fscanf(gpu_fp, "%lf", &multimedia_system.gaming.performance.gpu_utilization);
                fclose(gpu_fp);
            }
            
            /* Calculate frame rate (simplified) */
            static uint64_t last_frame_count = 0;
            static time_t last_time = 0;
            time_t current_time = time(NULL);
            
            if (current_time != last_time && last_time != 0) {
                multimedia_system.gaming.performance.current_fps = 
                    (multimedia_system.stats.vr_frames_rendered - last_frame_count) / 
                    (current_time - last_time);
            }
            
            last_frame_count = multimedia_system.stats.vr_frames_rendered;
            last_time = current_time;
        }
        
        usleep(16667); /* ~60 FPS polling */
    }
    
    return NULL;
}

/* VR system monitoring thread */
static void *vr_monitor_thread(void *arg) {
    while (multimedia_running && multimedia_system.vr.vr_runtime_active) {
        for (uint32_t i = 0; i < multimedia_system.vr.headset_count; i++) {
            vr_headset_t *hmd = &multimedia_system.vr.headsets[i];
            
            if (!hmd->state.connected) continue;
            
            /* Update head pose (simplified simulation) */
            /* In real implementation, this would read from VR runtime */
            static double rotation = 0.0;
            rotation += 0.01;
            
            hmd->state.head_pose[0] = 0.0; /* X position */
            hmd->state.head_pose[1] = 1.7; /* Y position (standing height) */
            hmd->state.head_pose[2] = 0.0; /* Z position */
            hmd->state.head_pose[3] = cos(rotation / 2); /* Quaternion W */
            hmd->state.head_pose[4] = 0.0; /* Quaternion X */
            hmd->state.head_pose[5] = sin(rotation / 2); /* Quaternion Y */
            hmd->state.head_pose[6] = 0.0; /* Quaternion Z */
            
            /* Update controller poses */
            for (int c = 0; c < 2; c++) {
                if (hmd->state.controllers_connected[c]) {
                    hmd->state.controller_poses[c][0] = (c == 0) ? -0.3 : 0.3; /* Left/Right */
                    hmd->state.controller_poses[c][1] = 1.2; /* Controller height */
                    hmd->state.controller_poses[c][2] = -0.2; /* Forward from body */
                    hmd->state.controller_poses[c][3] = 1.0; /* Identity quaternion */
                    hmd->state.controller_poses[c][4] = 0.0;
                    hmd->state.controller_poses[c][5] = 0.0;
                    hmd->state.controller_poses[c][6] = 0.0;
                }
            }
        }
        
        usleep(11111); /* 90 FPS tracking */
    }
    
    return NULL;
}

/* Initialize multimedia system */
int multimedia_init(void) {
    memset(&multimedia_system, 0, sizeof(multimedia_system));
    
    printf("Initializing Multimedia & Gaming System...\n");
    
    /* Detect hardware capabilities */
    multimedia_detect_hardware();
    
    /* Initialize audio system */
    audio_system_init();
    
    /* Initialize gaming system */
    gaming_init();
    
    /* Initialize VR system */
    vr_system_init();
    
    /* Initialize streaming system */
    streaming_init();
    
    /* Register default codecs */
    media_codec_t h264_codec = {
        .name = "h264",
        .hardware_accelerated = true,
        .accel_type = HWACCEL_VAAPI,
        .encode_supported = true,
        .decode_supported = true,
        .max_encode_width = 4096,
        .max_encode_height = 2160,
        .max_encode_fps = 60,
        .max_encode_bitrate = 100000000, /* 100 Mbps */
        .max_decode_width = 4096,
        .max_decode_height = 2160,
        .max_decode_fps = 60
    };
    codec_register(&h264_codec);
    
    media_codec_t hevc_codec = {
        .name = "hevc",
        .hardware_accelerated = true,
        .accel_type = HWACCEL_VAAPI,
        .encode_supported = true,
        .decode_supported = true,
        .max_encode_width = 7680,
        .max_encode_height = 4320,
        .max_encode_fps = 60,
        .max_encode_bitrate = 200000000, /* 200 Mbps */
        .max_decode_width = 7680,
        .max_decode_height = 4320,
        .max_decode_fps = 60
    };
    codec_register(&hevc_codec);
    
    media_codec_t av1_codec = {
        .name = "av1",
        .hardware_accelerated = true,
        .accel_type = HWACCEL_VAAPI,
        .encode_supported = true,
        .decode_supported = true,
        .max_encode_width = 7680,
        .max_encode_height = 4320,
        .max_encode_fps = 60,
        .max_encode_bitrate = 150000000, /* 150 Mbps */
        .max_decode_width = 7680,
        .max_decode_height = 4320,
        .max_decode_fps = 60
    };
    codec_register(&av1_codec);
    
    /* Start monitoring threads */
    multimedia_running = true;
    pthread_create(&controller_thread, NULL, controller_monitor_thread, NULL);
    pthread_create(&vr_thread, NULL, vr_monitor_thread, NULL);
    
    multimedia_system.initialized = true;
    
    printf("Multimedia system initialized\n");
    printf("Video codecs: %u, Audio codecs: %u\n", 
           multimedia_system.video_codec_count, multimedia_system.audio_codec_count);
    printf("Audio devices: %u, Controllers: %u, VR headsets: %u\n",
           multimedia_system.audio.device_count, 
           multimedia_system.gaming.controller_count,
           multimedia_system.vr.headset_count);
    
    return 0;
}

/* Cleanup multimedia system */
void multimedia_exit(void) {
    multimedia_running = false;
    
    /* Wait for threads to finish */
    pthread_join(controller_thread, NULL);
    pthread_join(vr_thread, NULL);
    
    /* Stop all streams */
    for (uint32_t i = 0; i < multimedia_system.streaming.stream_count; i++) {
        streaming_config_t *stream = &multimedia_system.streaming.streams[i];
        if (stream->status.active) {
            stream_stop(stream->name);
        }
    }
    
    /* Disconnect VR headsets */
    for (uint32_t i = 0; i < multimedia_system.vr.headset_count; i++) {
        vr_headset_t *hmd = &multimedia_system.vr.headsets[i];
        if (hmd->state.connected) {
            vr_headset_disconnect(hmd->serial_number);
        }
    }
    
    /* Close controllers */
    for (uint32_t i = 0; i < multimedia_system.gaming.controller_count; i++) {
        gaming_controller_t *controller = &multimedia_system.gaming.controllers[i];
        if (controller->state.connected) {
            controller_close(controller->device_path);
        }
    }
    
    multimedia_system.initialized = false;
    memset(&multimedia_system, 0, sizeof(multimedia_system));
}

/* Detect hardware capabilities */
int multimedia_detect_hardware(void) {
    /* Detect hardware acceleration support */
    if (access("/dev/dri/renderD128", F_OK) == 0) {
        printf("VAAPI hardware acceleration detected\n");
    }
    
    if (access("/proc/driver/nvidia", F_OK) == 0) {
        printf("NVIDIA hardware acceleration detected\n");
    }
    
    /* Detect AMD hardware */
    DIR *pci_dir = opendir("/sys/bus/pci/devices");
    if (pci_dir) {
        struct dirent *entry;
        while ((entry = readdir(pci_dir)) != NULL) {
            char vendor_path[512];
            snprintf(vendor_path, sizeof(vendor_path), 
                    "/sys/bus/pci/devices/%s/vendor", entry->d_name);
            
            FILE *vendor_fp = fopen(vendor_path, "r");
            if (vendor_fp) {
                char vendor[16];
                if (fgets(vendor, sizeof(vendor), vendor_fp)) {
                    if (strstr(vendor, "0x1002")) { /* AMD vendor ID */
                        printf("AMD hardware acceleration detected\n");
                        break;
                    }
                }
                fclose(vendor_fp);
            }
        }
        closedir(pci_dir);
    }
    
    return 0;
}

/* Register media codec */
int codec_register(const media_codec_t *codec) {
    if (!codec) return -EINVAL;
    
    /* Determine if it's video or audio codec */
    bool is_video_codec = (strstr(codec->name, "h264") || strstr(codec->name, "hevc") || 
                          strstr(codec->name, "av1") || strstr(codec->name, "vp9"));
    
    if (is_video_codec) {
        if (multimedia_system.video_codec_count < VIDEO_FORMAT_MAX) {
            multimedia_system.video_codecs[multimedia_system.video_codec_count] = *codec;
            multimedia_system.video_codec_count++;
            printf("Registered video codec: %s (HW accel: %s)\n", 
                   codec->name, codec->hardware_accelerated ? "Yes" : "No");
            return 0;
        }
    } else {
        if (multimedia_system.audio_codec_count < AUDIO_FORMAT_MAX) {
            multimedia_system.audio_codecs[multimedia_system.audio_codec_count] = *codec;
            multimedia_system.audio_codec_count++;
            printf("Registered audio codec: %s\n", codec->name);
            return 0;
        }
    }
    
    return -ENOMEM;
}

/* Initialize audio system */
int audio_system_init(void) {
    printf("Initializing audio system...\n");
    
    /* Enumerate ALSA devices */
    snd_card_t card = -1;
    while (snd_card_next(&card) >= 0 && card >= 0) {
        char card_name[256];
        if (snd_card_get_name(card, &card_name) >= 0) {
            if (multimedia_system.audio.device_count < 16) {
                audio_device_t *device = &multimedia_system.audio.devices[multimedia_system.audio.device_count];
                
                snprintf(device->name, sizeof(device->name), "hw:%d", card);
                strcpy(device->driver, "alsa");
                device->default_device = (card == 0);
                
                /* Set capabilities */
                device->format_count = 4;
                device->formats[0] = AUDIO_FORMAT_PCM_S16LE;
                device->formats[1] = AUDIO_FORMAT_PCM_S24LE;
                device->formats[2] = AUDIO_FORMAT_PCM_S32LE;
                device->formats[3] = AUDIO_FORMAT_PCM_F32LE;
                
                device->sample_rate_count = 6;
                device->sample_rates[0] = 44100;
                device->sample_rates[1] = 48000;
                device->sample_rates[2] = 88200;
                device->sample_rates[3] = 96000;
                device->sample_rates[4] = 176400;
                device->sample_rates[5] = 192000;
                
                device->channels_min = 1;
                device->channels_max = 8;
                device->spatial_audio = true;
                device->surround_sound = true;
                device->hardware_mixing = true;
                device->low_latency = true;
                device->buffer_size_min = 64;
                device->buffer_size_max = 8192;
                
                /* Set defaults */
                device->current_format = AUDIO_FORMAT_PCM_S16LE;
                device->current_sample_rate = 48000;
                device->current_channels = 2;
                device->current_buffer_size = 1024;
                device->currently_active = false;
                
                multimedia_system.audio.device_count++;
                
                if (device->default_device) {
                    strcpy(multimedia_system.audio.default_output, device->name);
                }
            }
        }
    }
    
    /* Set audio system defaults */
    multimedia_system.audio.spatial_audio_enabled = true;
    multimedia_system.audio.noise_cancellation = false;
    multimedia_system.audio.echo_cancellation = false;
    multimedia_system.audio.master_volume = 0.8;
    multimedia_system.audio.muted = false;
    
    /* Audio effects defaults */
    multimedia_system.audio.effects.equalizer = false;
    multimedia_system.audio.effects.compressor = false;
    multimedia_system.audio.effects.reverb = false;
    multimedia_system.audio.effects.bass_boost = false;
    
    printf("Audio system initialized with %u devices\n", 
           multimedia_system.audio.device_count);
    
    return 0;
}

/* Initialize gaming system */
int gaming_init(void) {
    printf("Initializing gaming system...\n");
    
    /* Enumerate game controllers */
    controller_enumerate();
    
    /* Gaming system defaults */
    multimedia_system.gaming.game_mode = false;
    multimedia_system.gaming.vsync_adaptive = true;
    multimedia_system.gaming.frame_pacing = true;
    multimedia_system.gaming.target_fps = 60;
    
    /* Performance monitoring defaults */
    multimedia_system.gaming.performance.current_fps = 0.0;
    multimedia_system.gaming.performance.avg_fps = 0.0;
    multimedia_system.gaming.performance.frame_time_ms = 16.67; /* 60 FPS */
    multimedia_system.gaming.performance.frame_drops = 0;
    multimedia_system.gaming.performance.gpu_utilization = 0.0;
    multimedia_system.gaming.performance.cpu_utilization = 0.0;
    multimedia_system.gaming.performance.vram_usage = 0;
    
    printf("Gaming system initialized with %u controllers\n", 
           multimedia_system.gaming.controller_count);
    
    return 0;
}

/* Enumerate controllers */
int controller_enumerate(void) {
    DIR *input_dir = opendir("/dev/input");
    if (!input_dir) return -errno;
    
    struct dirent *entry;
    multimedia_system.gaming.controller_count = 0;
    
    while ((entry = readdir(input_dir)) != NULL && multimedia_system.gaming.controller_count < 8) {
        if (strncmp(entry->d_name, "js", 2) == 0) {
            /* Found joystick device */
            char device_path[512];
            snprintf(device_path, sizeof(device_path), "/dev/input/%s", entry->d_name);
            
            int fd = open(device_path, O_RDONLY);
            if (fd >= 0) {
                gaming_controller_t *controller = 
                    &multimedia_system.gaming.controllers[multimedia_system.gaming.controller_count];
                
                memset(controller, 0, sizeof(gaming_controller_t));
                strcpy(controller->device_path, device_path);
                
                /* Get controller name */
                char name_buffer[256] = {0};
                if (ioctl(fd, JSIOCGNAME(sizeof(name_buffer)), name_buffer) >= 0) {
                    strncpy(controller->name, name_buffer, sizeof(controller->name) - 1);
                } else {
                    snprintf(controller->name, sizeof(controller->name), "Controller %u", 
                            multimedia_system.gaming.controller_count);
                }
                
                /* Get button and axis counts */
                uint8_t buttons, axes;
                if (ioctl(fd, JSIOCGBUTTONS, &buttons) >= 0) {
                    controller->capabilities.button_count = buttons;
                }
                if (ioctl(fd, JSIOCGAXES, &axes) >= 0) {
                    controller->capabilities.axis_count = axes;
                }
                
                /* Determine controller type based on name */
                if (strstr(controller->name, "Xbox") || strstr(controller->name, "xbox")) {
                    if (strstr(controller->name, "360")) {
                        controller->type = CONTROLLER_XBOX_360;
                    } else if (strstr(controller->name, "One")) {
                        controller->type = CONTROLLER_XBOX_ONE;
                    } else {
                        controller->type = CONTROLLER_XBOX_SERIES;
                    }
                } else if (strstr(controller->name, "PlayStation") || 
                          strstr(controller->name, "DualShock") ||
                          strstr(controller->name, "DualSense")) {
                    if (strstr(controller->name, "3")) {
                        controller->type = CONTROLLER_PS3;
                    } else if (strstr(controller->name, "4")) {
                        controller->type = CONTROLLER_PS4;
                    } else {
                        controller->type = CONTROLLER_PS5_DUALSENSE;
                    }
                } else if (strstr(controller->name, "Nintendo") || 
                          strstr(controller->name, "Pro Controller")) {
                    controller->type = CONTROLLER_NINTENDO_SWITCH_PRO;
                } else {
                    controller->type = CONTROLLER_GENERIC_HID;
                }
                
                /* Set capabilities based on type */
                switch (controller->type) {
                    case CONTROLLER_PS5_DUALSENSE:
                        controller->capabilities.has_touchpad = true;
                        controller->capabilities.has_motion = true;
                        controller->capabilities.has_haptic = true;
                        controller->capabilities.has_adaptive_triggers = true;
                        break;
                    case CONTROLLER_PS4:
                        controller->capabilities.has_touchpad = true;
                        controller->capabilities.has_motion = true;
                        controller->capabilities.has_haptic = true;
                        break;
                    case CONTROLLER_NINTENDO_SWITCH_PRO:
                        controller->capabilities.has_motion = true;
                        controller->capabilities.has_haptic = true;
                        break;
                    default:
                        break;
                }
                
                controller->capabilities.has_dpad = true;
                controller->capabilities.has_triggers = true;
                controller->capabilities.wireless = false; /* Assume wired for now */
                
                /* Configuration defaults */
                controller->config.deadzone = 0.1;
                controller->config.sensitivity = 1.0;
                controller->config.invert_y = false;
                
                /* State */
                controller->state.connected = true;
                controller->state.battery_level = 100;
                controller->state.charging = false;
                
                close(fd);
                
                multimedia_system.gaming.controller_count++;
                
                printf("Found controller: %s (%s)\n", 
                       controller->name, controller_type_name(controller->type));
            }
        }
    }
    
    closedir(input_dir);
    return multimedia_system.gaming.controller_count;
}

/* Initialize VR system */
int vr_system_init(void) {
    printf("Initializing VR/AR system...\n");
    
    multimedia_system.vr.vr_runtime_active = false;
    multimedia_system.vr.room_scale = false;
    multimedia_system.vr.hand_tracking = false;
    multimedia_system.vr.eye_tracking = false;
    multimedia_system.vr.foveated_rendering = false;
    multimedia_system.vr.reprojection = true;
    multimedia_system.vr.supersampling = 1;
    
    /* Try to detect VR headsets */
    /* This would normally interface with OpenVR, OpenXR, or vendor SDKs */
    
    return 0;
}

/* Initialize streaming system */
int streaming_init(void) {
    printf("Initializing streaming system...\n");
    
    multimedia_system.streaming.hardware_encoding = true;
    multimedia_system.streaming.servers.rtmp_server = false;
    multimedia_system.streaming.servers.webrtc_server = false;
    multimedia_system.streaming.servers.ndi_server = false;
    multimedia_system.streaming.servers.rtmp_port = 1935;
    multimedia_system.streaming.servers.webrtc_port = 8080;
    
    return 0;
}

/* Enable game mode */
int game_mode_enable(void) {
    if (multimedia_system.gaming.game_mode) return 0; /* Already enabled */
    
    printf("Enabling game mode...\n");
    
    /* Set CPU governor to performance */
    system("echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor");
    
    /* Disable CPU frequency scaling */
    system("echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo 2>/dev/null");
    
    /* Set I/O scheduler to deadline for better gaming performance */
    system("echo deadline | sudo tee /sys/block/*/queue/scheduler");
    
    /* Increase network buffer sizes */
    system("echo 16777216 | sudo tee /proc/sys/net/core/rmem_max");
    system("echo 16777216 | sudo tee /proc/sys/net/core/wmem_max");
    
    multimedia_system.gaming.game_mode = true;
    
    return 0;
}

/* Disable game mode */
int game_mode_disable(void) {
    if (!multimedia_system.gaming.game_mode) return 0; /* Already disabled */
    
    printf("Disabling game mode...\n");
    
    /* Restore CPU governor */
    system("echo ondemand | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor");
    
    /* Re-enable CPU frequency scaling */
    system("echo 0 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo 2>/dev/null");
    
    /* Restore I/O scheduler */
    system("echo mq-deadline | sudo tee /sys/block/*/queue/scheduler");
    
    multimedia_system.gaming.game_mode = false;
    
    return 0;
}

/* Create streaming configuration */
int stream_create(const streaming_config_t *config) {
    if (!config || multimedia_system.streaming.stream_count >= 16) return -EINVAL;
    
    streaming_config_t *stream = &multimedia_system.streaming.streams[multimedia_system.streaming.stream_count];
    *stream = *config;
    
    /* Initialize status */
    stream->status.active = false;
    stream->status.bytes_sent = 0;
    stream->status.frames_sent = 0;
    stream->status.current_fps = 0.0;
    stream->status.dropped_frames = 0;
    stream->status.network_errors = 0;
    
    multimedia_system.streaming.stream_count++;
    
    printf("Created stream: %s (%s)\n", config->name, stream_protocol_name(config->protocol));
    
    return 0;
}

/* Utility functions */
const char *audio_format_name(audio_format_t format) {
    static const char *names[] = {
        "PCM S16LE", "PCM S24LE", "PCM S32LE", "PCM F32LE",
        "MP3", "AAC", "FLAC", "Ogg Vorbis", "Opus", 
        "DTS", "Dolby Atmos"
    };
    
    if (format >= 0 && format < AUDIO_FORMAT_MAX) {
        return names[format];
    }
    return "Unknown";
}

const char *video_format_name(video_format_t format) {
    static const char *names[] = {
        "H.264", "H.265/HEVC", "AV1", "VP9", "VP8", 
        "MPEG-2", "MPEG-4", "Xvid", "Theora", "MJPEG", "Raw"
    };
    
    if (format >= 0 && format < VIDEO_FORMAT_MAX) {
        return names[format];
    }
    return "Unknown";
}

const char *controller_type_name(controller_type_t type) {
    static const char *names[] = {
        "Unknown", "Xbox 360", "Xbox One", "Xbox Series", 
        "PlayStation 3", "PlayStation 4", "PlayStation 5 DualSense",
        "Nintendo Switch Pro", "Steam Controller", "Generic HID"
    };
    
    if (type >= 0 && type < CONTROLLER_MAX) {
        return names[type];
    }
    return "Unknown";
}

const char *hmd_type_name(hmd_type_t type) {
    static const char *names[] = {
        "Unknown", "Oculus Rift", "Oculus Quest", "HTC Vive", 
        "Valve Index", "Pico", "Varjo", "Microsoft HoloLens",
        "Magic Leap", "Apple Vision Pro"
    };
    
    if (type >= 0 && type < HMD_MAX) {
        return names[type];
    }
    return "Unknown";
}

const char *stream_protocol_name(stream_protocol_t protocol) {
    static const char *names[] = {
        "RTMP", "RTSP", "HLS", "DASH", "WebRTC", "SRT", "NDI"
    };
    
    if (protocol >= 0 && protocol < STREAM_PROTOCOL_MAX) {
        return names[protocol];
    }
    return "Unknown";
}

const char *hwaccel_type_name(hwaccel_type_t type) {
    static const char *names[] = {
        "None", "VAAPI", "NVENC/NVDEC", "AMD AMF", "Intel Quick Sync",
        "Apple VideoToolbox", "OpenCL", "Vulkan"
    };
    
    if (type >= 0 && type < HWACCEL_MAX) {
        return names[type];
    }
    return "Unknown";
}