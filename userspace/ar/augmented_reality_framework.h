/*
 * LimitlessOS Augmented Reality Framework
 * AR/VR support with 3D spatial computing, gesture recognition, and eye tracking
 */

#ifndef AUGMENTED_REALITY_FRAMEWORK_H
#define AUGMENTED_REALITY_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

/* AR/VR system constants */
#define MAX_AR_DEVICES              16
#define MAX_AR_SESSIONS             64
#define MAX_AR_ANCHORS              1024
#define MAX_AR_OBJECTS              2048
#define MAX_AR_TRACKERS             32
#define MAX_HAND_JOINTS             25
#define MAX_FACE_LANDMARKS          468
#define MAX_EYE_LANDMARKS           32
#define MAX_GESTURE_TEMPLATES       256
#define MAX_AR_CAMERAS              8
#define MAX_DISPLAY_SURFACES        16
#define MAX_RENDER_LAYERS          8
#define MAX_HAPTIC_DEVICES         16

/* Rendering and display constants */
#define AR_FRAME_BUFFER_WIDTH      3840  /* 4K width */
#define AR_FRAME_BUFFER_HEIGHT     2160  /* 4K height */
#define AR_MAX_REFRESH_RATE        120   /* 120Hz */
#define AR_MIN_REFRESH_RATE        60    /* 60Hz */
#define AR_STEREO_SEPARATION       0.064f /* 64mm IPD */
#define AR_NEAR_PLANE             0.01f  /* 1cm */
#define AR_FAR_PLANE              100.0f /* 100m */

/* Tracking and calibration constants */
#define AR_TRACKING_HISTORY_SIZE   1000
#define AR_CALIBRATION_POINTS      50
#define AR_SLAM_MAP_POINTS         10000
#define AR_FEATURE_POINTS          2000
#define AR_MARKER_SIZE_MAX         256
#define AR_QR_CODE_SIZE_MAX        512

/* Spatial computing constants */
#define AR_WORLD_SCALE_MIN         0.001f /* 1mm */
#define AR_WORLD_SCALE_MAX         1000.0f /* 1km */
#define AR_OCCLUSION_LAYERS        16
#define AR_LIGHTING_PROBES         64
#define AR_SHADOW_CASCADES         4

/* AR/VR device types */
typedef enum {
    AR_DEVICE_UNKNOWN = 0,
    AR_DEVICE_HMD,                /* Head-Mounted Display */
    AR_DEVICE_HANDHELD,           /* Smartphone/Tablet */
    AR_DEVICE_PASSTHROUGH,        /* AR Glasses */
    AR_DEVICE_PROJECTION,         /* Spatial AR Projector */
    AR_DEVICE_HOLOGRAPHIC,        /* Holographic Display */
    AR_DEVICE_CONTACT_LENS,       /* AR Contact Lenses */
    AR_DEVICE_RETINAL_DISPLAY,    /* Direct Retinal Projection */
    AR_DEVICE_MAX
} ar_device_type_t;

typedef enum {
    AR_TRACKING_UNKNOWN = 0,
    AR_TRACKING_3DOF,             /* Orientation only */
    AR_TRACKING_6DOF,             /* Position + Orientation */
    AR_TRACKING_WORLD_SCALE,      /* World-scale tracking */
    AR_TRACKING_ROOM_SCALE,       /* Room-scale tracking */
    AR_TRACKING_MARKER_BASED,     /* Marker tracking */
    AR_TRACKING_MARKERLESS,       /* SLAM tracking */
    AR_TRACKING_INSIDE_OUT,       /* Device cameras */
    AR_TRACKING_OUTSIDE_IN,       /* External sensors */
    AR_TRACKING_HYBRID,           /* Multiple systems */
    AR_TRACKING_MAX
} ar_tracking_type_t;

typedef enum {
    AR_DISPLAY_UNKNOWN = 0,
    AR_DISPLAY_STEREO,            /* Stereoscopic 3D */
    AR_DISPLAY_MONO,              /* Monoscopic */
    AR_DISPLAY_LIGHT_FIELD,       /* Light field display */
    AR_DISPLAY_VOLUMETRIC,        /* Volumetric display */
    AR_DISPLAY_RETINAL,           /* Retinal projection */
    AR_DISPLAY_HOLOGRAPHIC,       /* Holographic display */
    AR_DISPLAY_MAX
} ar_display_type_t;

typedef enum {
    AR_RENDER_FORWARD = 0,        /* Forward rendering */
    AR_RENDER_DEFERRED,           /* Deferred rendering */
    AR_RENDER_FOVEATED,           /* Foveated rendering */
    AR_RENDER_MULTI_RES,          /* Multi-resolution */
    AR_RENDER_REPROJECTION,       /* Async reprojection */
    AR_RENDER_NEURAL,             /* Neural rendering */
    AR_RENDER_MAX
} ar_render_mode_t;

typedef enum {
    GESTURE_UNKNOWN = 0,
    GESTURE_POINT,                /* Point gesture */
    GESTURE_GRAB,                 /* Grab/Pinch */
    GESTURE_SWIPE_LEFT,          /* Swipe gestures */
    GESTURE_SWIPE_RIGHT,
    GESTURE_SWIPE_UP,
    GESTURE_SWIPE_DOWN,
    GESTURE_TAP,                 /* Tap gesture */
    GESTURE_DOUBLE_TAP,          /* Double tap */
    GESTURE_LONG_PRESS,          /* Long press */
    GESTURE_ROTATE,              /* Rotation gesture */
    GESTURE_SCALE,               /* Scale/Pinch to zoom */
    GESTURE_WAVE,                /* Wave gesture */
    GESTURE_THUMBS_UP,           /* Thumbs up */
    GESTURE_THUMBS_DOWN,         /* Thumbs down */
    GESTURE_OK_SIGN,             /* OK hand sign */
    GESTURE_PEACE_SIGN,          /* Peace/Victory sign */
    GESTURE_FIST,                /* Closed fist */
    GESTURE_OPEN_PALM,           /* Open palm */
    GESTURE_CUSTOM,              /* Custom gesture */
    GESTURE_MAX
} gesture_type_t;

typedef enum {
    EYE_GAZE_UNKNOWN = 0,
    EYE_GAZE_FOCUSED,            /* Focused gaze */
    EYE_GAZE_SACCADE,            /* Rapid movement */
    EYE_GAZE_FIXATION,           /* Fixed on object */
    EYE_GAZE_PURSUIT,            /* Smooth pursuit */
    EYE_GAZE_BLINK,              /* Eye blink */
    EYE_GAZE_WINK,               /* Wink */
    EYE_GAZE_MAX
} eye_gaze_type_t;

/* 3D math structures */
typedef struct {
    float x, y, z;
} vector3_t;

typedef struct {
    float x, y, z, w;
} quaternion_t;

typedef struct {
    float m[4][4];               /* 4x4 transformation matrix */
} matrix4x4_t;

typedef struct {
    vector3_t position;
    quaternion_t rotation;
    vector3_t scale;
} transform_t;

typedef struct {
    float left, right, top, bottom; /* Frustum bounds */
    float near_plane, far_plane;
} frustum_t;

/* AR anchor and tracking */
typedef struct {
    uint32_t anchor_id;
    char name[64];
    transform_t transform;
    vector3_t world_position;
    quaternion_t world_rotation;
    
    /* Tracking confidence */
    float tracking_confidence;    /* 0.0 - 1.0 */
    bool is_tracked;
    uint64_t last_update_time;
    
    /* Persistence */
    bool persistent;
    uint8_t anchor_data[256];    /* Serialized anchor data */
    uint32_t data_size;
    
    /* Associated objects */
    uint32_t object_count;
    uint32_t object_ids[64];
    
    pthread_mutex_t lock;
} ar_anchor_t;

/* AR object representation */
typedef struct {
    uint32_t object_id;
    char name[64];
    uint32_t anchor_id;          /* Associated anchor */
    
    /* Transformation */
    transform_t local_transform;  /* Relative to anchor */
    transform_t world_transform;  /* World coordinates */
    
    /* Rendering properties */
    uint32_t mesh_id;            /* 3D mesh */
    uint32_t material_id;        /* Material properties */
    uint32_t texture_ids[8];     /* Texture maps */
    float opacity;               /* Transparency */
    bool visible;                /* Visibility flag */
    
    /* Physics properties */
    bool physics_enabled;
    float mass;
    vector3_t velocity;
    vector3_t angular_velocity;
    
    /* Interaction */
    bool interactive;
    float interaction_radius;
    void (*on_select)(uint32_t object_id, void *user_data);
    void (*on_hover)(uint32_t object_id, void *user_data);
    void *user_data;
    
    /* Animation */
    bool animated;
    uint32_t animation_id;
    float animation_time;
    
    pthread_mutex_t lock;
} ar_object_t;

/* Hand tracking data */
typedef struct {
    vector3_t position;
    quaternion_t rotation;
    float confidence;            /* Joint tracking confidence */
    bool tracked;
} hand_joint_t;

typedef struct {
    bool detected;
    float confidence;            /* Hand detection confidence */
    hand_joint_t joints[MAX_HAND_JOINTS];
    
    /* Hand pose classification */
    gesture_type_t current_gesture;
    float gesture_confidence;
    
    /* Hand metrics */
    float hand_size;             /* Estimated hand size */
    bool is_left_hand;           /* Handedness */
    
    /* Velocity tracking */
    vector3_t palm_velocity;
    vector3_t fingertip_velocities[5];
    
    uint64_t timestamp;
} hand_tracking_t;

/* Face tracking data */
typedef struct {
    vector3_t position;
    float confidence;
} face_landmark_t;

typedef struct {
    bool detected;
    float confidence;
    face_landmark_t landmarks[MAX_FACE_LANDMARKS];
    
    /* Face pose */
    vector3_t head_position;
    quaternion_t head_rotation;
    
    /* Expression analysis */
    float expression_weights[52]; /* FACS action units */
    
    /* Face metrics */
    float face_size;
    vector3_t face_center;
    
    uint64_t timestamp;
} face_tracking_t;

/* Eye tracking data */
typedef struct {
    vector3_t gaze_origin;       /* Eye position */
    vector3_t gaze_direction;    /* Gaze direction */
    float pupil_diameter;        /* Pupil size */
    float openness;             /* Eye openness 0-1 */
    bool is_valid;              /* Tracking validity */
} eye_data_t;

typedef struct {
    eye_data_t left_eye;
    eye_data_t right_eye;
    
    /* Combined gaze */
    vector3_t combined_gaze_origin;
    vector3_t combined_gaze_direction;
    
    /* Gaze behavior */
    eye_gaze_type_t gaze_type;
    float fixation_duration;     /* Current fixation time */
    vector3_t fixation_point;    /* World space fixation */
    
    /* Calibration */
    bool calibrated;
    float calibration_quality;   /* 0.0 - 1.0 */
    
    uint64_t timestamp;
} eye_tracking_t;

/* Gesture recognition */
typedef struct {
    gesture_type_t type;
    char name[32];
    float template_points[100][3]; /* Gesture template */
    uint32_t point_count;
    float matching_threshold;
} gesture_template_t;

typedef struct {
    gesture_type_t detected_gesture;
    float confidence;
    vector3_t gesture_center;
    float gesture_scale;
    float gesture_rotation;
    
    /* Gesture timing */
    uint64_t start_time;
    uint64_t duration;
    bool gesture_completed;
    
    /* Associated hand */
    bool left_hand;
    bool right_hand;
    bool two_handed;
} gesture_recognition_t;

/* Camera and sensor data */
typedef struct {
    uint32_t camera_id;
    char name[32];
    
    /* Camera properties */
    uint32_t width, height;
    float fps;
    float focal_length;
    float sensor_width, sensor_height;
    
    /* Intrinsic parameters */
    matrix4x4_t intrinsic_matrix;
    float distortion_coeffs[8];
    
    /* Extrinsic parameters */
    transform_t camera_pose;
    
    /* Frame data */
    uint8_t *frame_buffer;
    uint32_t frame_size;
    uint64_t frame_timestamp;
    uint32_t frame_number;
    
    /* Camera state */
    bool active;
    bool calibrated;
    float calibration_error;
    
    pthread_mutex_t lock;
} ar_camera_t;

/* SLAM (Simultaneous Localization and Mapping) */
typedef struct {
    vector3_t position;
    uint8_t descriptor[32];      /* Feature descriptor */
    float confidence;
    uint32_t observation_count;
    bool is_persistent;
} map_point_t;

typedef struct {
    uint32_t keyframe_id;
    transform_t pose;
    uint32_t feature_count;
    vector3_t feature_points[AR_FEATURE_POINTS];
    uint8_t feature_descriptors[AR_FEATURE_POINTS][32];
    uint64_t timestamp;
} keyframe_t;

typedef struct {
    /* Map data */
    map_point_t map_points[AR_SLAM_MAP_POINTS];
    uint32_t map_point_count;
    
    /* Keyframes */
    keyframe_t keyframes[1000];
    uint32_t keyframe_count;
    
    /* Current tracking state */
    transform_t current_pose;
    float tracking_confidence;
    bool is_lost;
    
    /* Map optimization */
    bool optimization_enabled;
    uint32_t optimization_interval; /* Frames between optimization */
    
    /* Loop closure */
    bool loop_closure_enabled;
    float loop_closure_threshold;
    
    pthread_mutex_t lock;
} slam_system_t;

/* Lighting and environment */
typedef struct {
    vector3_t position;
    vector3_t direction;         /* For directional lights */
    float intensity;
    float color[3];              /* RGB color */
    float attenuation[3];        /* Constant, linear, quadratic */
    bool cast_shadows;
} light_probe_t;

typedef struct {
    /* Environmental lighting */
    light_probe_t light_probes[AR_LIGHTING_PROBES];
    uint32_t light_probe_count;
    
    /* IBL (Image-Based Lighting) */
    uint32_t environment_map;    /* Cubemap texture */
    uint32_t irradiance_map;     /* Pre-computed irradiance */
    uint32_t specular_map;       /* Pre-filtered specular */
    
    /* Ambient lighting */
    float ambient_color[3];
    float ambient_intensity;
    
    /* Shadow mapping */
    bool shadows_enabled;
    uint32_t shadow_map_resolution;
    float shadow_bias;
    
    /* Real-time updates */
    bool auto_update;
    float update_interval;       /* Seconds */
    
    pthread_mutex_t lock;
} lighting_environment_t;

/* Occlusion and depth */
typedef struct {
    /* Depth sensing */
    float *depth_buffer;         /* Per-pixel depth */
    uint32_t depth_width;
    uint32_t depth_height;
    float depth_scale;           /* Meters per unit */
    
    /* Occlusion mesh */
    uint32_t occlusion_mesh_id;
    bool occlusion_enabled;
    
    /* Depth-based occlusion */
    bool depth_occlusion;
    float occlusion_threshold;   /* Depth difference threshold */
    
    /* Temporal stability */
    float temporal_smoothing;    /* 0.0 - 1.0 */
    
    pthread_mutex_t lock;
} occlusion_system_t;

/* AR session management */
typedef struct {
    uint32_t session_id;
    char name[64];
    ar_device_type_t device_type;
    
    /* Session configuration */
    ar_tracking_type_t tracking_type;
    ar_display_type_t display_type;
    ar_render_mode_t render_mode;
    
    /* Display properties */
    uint32_t display_width;
    uint32_t display_height;
    float refresh_rate;
    float field_of_view;         /* Degrees */
    float interpupillary_distance; /* IPD in meters */
    
    /* Tracking systems */
    bool hand_tracking_enabled;
    bool eye_tracking_enabled;
    bool face_tracking_enabled;
    bool gesture_recognition_enabled;
    
    /* Tracking data */
    hand_tracking_t left_hand;
    hand_tracking_t right_hand;
    face_tracking_t face;
    eye_tracking_t eyes;
    gesture_recognition_t gesture;
    
    /* Camera system */
    ar_camera_t cameras[MAX_AR_CAMERAS];
    uint32_t camera_count;
    
    /* SLAM system */
    slam_system_t slam;
    
    /* Lighting and environment */
    lighting_environment_t lighting;
    occlusion_system_t occlusion;
    
    /* Performance metrics */
    float frame_rate;
    float tracking_latency;      /* Milliseconds */
    float render_latency;        /* Milliseconds */
    float motion_to_photon_latency; /* Total latency */
    
    /* Session state */
    bool active;
    bool paused;
    uint64_t start_time;
    uint64_t frame_count;
    
    pthread_mutex_t lock;
} ar_session_t;

/* AR device capabilities */
typedef struct {
    uint32_t device_id;
    char name[64];
    char manufacturer[32];
    char model[32];
    ar_device_type_t type;
    
    /* Display capabilities */
    uint32_t max_resolution_width;
    uint32_t max_resolution_height;
    float max_refresh_rate;
    float max_field_of_view;
    ar_display_type_t display_types[8];
    uint32_t display_type_count;
    
    /* Tracking capabilities */
    ar_tracking_type_t tracking_types[8];
    uint32_t tracking_type_count;
    bool six_dof_tracking;
    bool world_scale_tracking;
    bool marker_tracking;
    bool slam_tracking;
    
    /* Input capabilities */
    bool hand_tracking;
    bool eye_tracking;
    bool face_tracking;
    bool gesture_recognition;
    bool voice_commands;
    
    /* Sensor capabilities */
    uint32_t camera_count;
    bool depth_sensor;
    bool imu_sensor;
    bool magnetometer;
    bool gps_sensor;
    
    /* Rendering capabilities */
    ar_render_mode_t render_modes[8];
    uint32_t render_mode_count;
    bool foveated_rendering;
    bool reprojection;
    bool neural_rendering;
    
    /* Hardware specifications */
    char gpu_name[64];
    uint32_t gpu_memory_mb;
    char cpu_name[64];
    uint32_t cpu_cores;
    uint32_t system_memory_mb;
    
    /* Device state */
    bool connected;
    bool initialized;
    float battery_level;         /* 0.0 - 1.0, -1 if N/A */
    float temperature;           /* Celsius */
    
    pthread_mutex_t lock;
} ar_device_t;

/* Main AR system structure */
typedef struct {
    bool initialized;
    pthread_mutex_t system_lock;
    
    /* Device management */
    ar_device_t devices[MAX_AR_DEVICES];
    uint32_t device_count;
    uint32_t active_device_id;
    
    /* Session management */
    ar_session_t sessions[MAX_AR_SESSIONS];
    uint32_t session_count;
    uint32_t active_session_id;
    
    /* World tracking */
    ar_anchor_t anchors[MAX_AR_ANCHORS];
    uint32_t anchor_count;
    
    /* Virtual objects */
    ar_object_t objects[MAX_AR_OBJECTS];
    uint32_t object_count;
    
    /* Gesture templates */
    gesture_template_t gesture_templates[MAX_GESTURE_TEMPLATES];
    uint32_t template_count;
    
    /* System configuration */
    bool auto_initialization;
    float default_tracking_confidence;
    bool persistent_anchors;
    bool cloud_anchors;
    
    /* Performance settings */
    ar_render_mode_t preferred_render_mode;
    bool adaptive_quality;
    float target_frame_rate;
    bool power_optimization;
    
    /* Threading */
    pthread_t tracking_thread;
    pthread_t rendering_thread;
    pthread_t slam_thread;
    bool threads_running;
    
    /* Statistics */
    uint64_t total_frames_rendered;
    uint64_t total_tracking_updates;
    double average_frame_time;
    double average_tracking_latency;
    time_t start_time;
} ar_system_t;

/* Function declarations */

/* System initialization and management */
int ar_system_init(void);
int ar_system_cleanup(void);
int ar_system_set_config(ar_render_mode_t render_mode, float target_fps, bool power_optimization);

/* Device management */
int ar_device_enumerate(void);
int ar_device_get_capabilities(uint32_t device_id, ar_device_t *capabilities);
int ar_device_connect(uint32_t device_id);
int ar_device_disconnect(uint32_t device_id);
int ar_device_calibrate(uint32_t device_id);

/* Session management */
int ar_session_create(const char *name, ar_device_type_t device_type, ar_tracking_type_t tracking_type);
int ar_session_destroy(uint32_t session_id);
int ar_session_start(uint32_t session_id);
int ar_session_pause(uint32_t session_id);
int ar_session_resume(uint32_t session_id);
int ar_session_stop(uint32_t session_id);
int ar_session_get_info(uint32_t session_id, ar_session_t *session_info);

/* Anchor and object management */
int ar_anchor_create(const char *name, const transform_t *transform);
int ar_anchor_destroy(uint32_t anchor_id);
int ar_anchor_update(uint32_t anchor_id, const transform_t *transform);
int ar_anchor_get_transform(uint32_t anchor_id, transform_t *transform);
int ar_anchor_set_persistent(uint32_t anchor_id, bool persistent);

int ar_object_create(const char *name, uint32_t anchor_id, const transform_t *local_transform);
int ar_object_destroy(uint32_t object_id);
int ar_object_set_mesh(uint32_t object_id, uint32_t mesh_id);
int ar_object_set_material(uint32_t object_id, uint32_t material_id);
int ar_object_set_visibility(uint32_t object_id, bool visible);
int ar_object_set_transform(uint32_t object_id, const transform_t *transform);
int ar_object_set_interaction_callback(uint32_t object_id, 
                                      void (*on_select)(uint32_t, void*),
                                      void (*on_hover)(uint32_t, void*),
                                      void *user_data);

/* Tracking systems */
int ar_tracking_start(uint32_t session_id, ar_tracking_type_t tracking_type);
int ar_tracking_stop(uint32_t session_id);
int ar_tracking_get_pose(uint32_t session_id, transform_t *head_pose);
int ar_tracking_get_confidence(uint32_t session_id, float *confidence);

/* Hand tracking */
int ar_hand_tracking_enable(uint32_t session_id, bool enable);
int ar_hand_tracking_get_data(uint32_t session_id, hand_tracking_t *left_hand, hand_tracking_t *right_hand);
int ar_hand_tracking_calibrate(uint32_t session_id);

/* Eye tracking */
int ar_eye_tracking_enable(uint32_t session_id, bool enable);
int ar_eye_tracking_get_data(uint32_t session_id, eye_tracking_t *eye_data);
int ar_eye_tracking_calibrate(uint32_t session_id);
int ar_eye_tracking_get_gaze_point(uint32_t session_id, vector3_t *gaze_point);

/* Face tracking */
int ar_face_tracking_enable(uint32_t session_id, bool enable);
int ar_face_tracking_get_data(uint32_t session_id, face_tracking_t *face_data);
int ar_face_tracking_get_expression(uint32_t session_id, float expression_weights[52]);

/* Gesture recognition */
int ar_gesture_recognition_enable(uint32_t session_id, bool enable);
int ar_gesture_add_template(const char *name, gesture_type_t type, 
                           const float points[][3], uint32_t point_count);
int ar_gesture_recognize(uint32_t session_id, gesture_recognition_t *gesture_data);
int ar_gesture_train_custom(const char *name, const float training_data[][3], 
                           uint32_t sample_count, uint32_t samples_per_gesture);

/* SLAM and world mapping */
int ar_slam_initialize(uint32_t session_id);
int ar_slam_start_mapping(uint32_t session_id);
int ar_slam_stop_mapping(uint32_t session_id);
int ar_slam_get_map_points(uint32_t session_id, map_point_t **points, uint32_t *count);
int ar_slam_save_map(uint32_t session_id, const char *filename);
int ar_slam_load_map(uint32_t session_id, const char *filename);
int ar_slam_relocalize(uint32_t session_id);

/* Lighting and environment */
int ar_lighting_estimate_environment(uint32_t session_id);
int ar_lighting_add_probe(uint32_t session_id, const vector3_t *position, 
                         float intensity, const float color[3]);
int ar_lighting_update_auto(uint32_t session_id, bool enable);
int ar_lighting_set_ambient(uint32_t session_id, const float color[3], float intensity);

/* Occlusion processing */
int ar_occlusion_enable(uint32_t session_id, bool enable);
int ar_occlusion_update_depth(uint32_t session_id, const float *depth_data, 
                             uint32_t width, uint32_t height);
int ar_occlusion_set_threshold(uint32_t session_id, float threshold);

/* Rendering and display */
int ar_render_frame(uint32_t session_id);
int ar_render_set_mode(uint32_t session_id, ar_render_mode_t mode);
int ar_render_set_foveated(uint32_t session_id, bool enable, const vector3_t *gaze_point);
int ar_render_set_reprojection(uint32_t session_id, bool enable);

/* Camera and sensor access */
int ar_camera_get_frame(uint32_t session_id, uint32_t camera_id, 
                       uint8_t **frame_data, uint32_t *width, uint32_t *height);
int ar_camera_set_exposure(uint32_t session_id, uint32_t camera_id, float exposure);
int ar_camera_set_focus(uint32_t session_id, uint32_t camera_id, float focus_distance);

/* Spatial computing utilities */
int ar_spatial_raycast(uint32_t session_id, const vector3_t *origin, const vector3_t *direction,
                      vector3_t *hit_point, uint32_t *hit_object_id);
int ar_spatial_get_plane_detection(uint32_t session_id, vector3_t **plane_points, 
                                  vector3_t **plane_normals, uint32_t *plane_count);
int ar_spatial_collision_detection(uint32_t session_id, uint32_t object_id, 
                                  uint32_t *collision_objects, uint32_t *collision_count);

/* Math utilities */
vector3_t vec3_add(const vector3_t *a, const vector3_t *b);
vector3_t vec3_subtract(const vector3_t *a, const vector3_t *b);
vector3_t vec3_multiply(const vector3_t *a, float scalar);
float vec3_dot(const vector3_t *a, const vector3_t *b);
vector3_t vec3_cross(const vector3_t *a, const vector3_t *b);
float vec3_length(const vector3_t *v);
vector3_t vec3_normalize(const vector3_t *v);

quaternion_t quat_multiply(const quaternion_t *a, const quaternion_t *b);
quaternion_t quat_from_axis_angle(const vector3_t *axis, float angle);
vector3_t quat_rotate_vector(const quaternion_t *q, const vector3_t *v);
quaternion_t quat_slerp(const quaternion_t *a, const quaternion_t *b, float t);

matrix4x4_t mat4_identity(void);
matrix4x4_t mat4_multiply(const matrix4x4_t *a, const matrix4x4_t *b);
matrix4x4_t mat4_from_transform(const transform_t *transform);
matrix4x4_t mat4_perspective(float fov, float aspect, float near_plane, float far_plane);
matrix4x4_t mat4_look_at(const vector3_t *eye, const vector3_t *center, const vector3_t *up);

transform_t transform_identity(void);
transform_t transform_multiply(const transform_t *a, const transform_t *b);
transform_t transform_inverse(const transform_t *t);
vector3_t transform_point(const transform_t *t, const vector3_t *point);

/* Utility functions */
const char *ar_device_type_name(ar_device_type_t type);
const char *ar_tracking_type_name(ar_tracking_type_t type);
const char *ar_display_type_name(ar_display_type_t type);
const char *gesture_type_name(gesture_type_t type);
uint64_t ar_get_timestamp_microseconds(void);
float ar_calculate_distance(const vector3_t *a, const vector3_t *b);
bool ar_point_in_frustum(const vector3_t *point, const frustum_t *frustum);

#endif /* AUGMENTED_REALITY_FRAMEWORK_H */