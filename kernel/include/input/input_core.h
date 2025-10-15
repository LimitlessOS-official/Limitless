/*
 * LimitlessOS Advanced Input/HID Subsystem
 * Production input device support with gestures and accessibility
 * Features: Multi-touch, keyboards, mice, game controllers, accessibility
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../kernel.h"

/* Input subsystem constants */
#define MAX_INPUT_DEVICES       256
#define MAX_TOUCH_CONTACTS      10
#define MAX_INPUT_EVENTS        1024
#define MAX_GESTURE_POINTS      32
#define MAX_KEYMAPS             16

/* Input device types */
typedef enum {
    INPUT_TYPE_UNKNOWN = 0,
    INPUT_TYPE_KEYBOARD,
    INPUT_TYPE_MOUSE,
    INPUT_TYPE_TOUCHSCREEN,
    INPUT_TYPE_TOUCHPAD,
    INPUT_TYPE_STYLUS,
    INPUT_TYPE_GAMEPAD,
    INPUT_TYPE_JOYSTICK,
    INPUT_TYPE_ACCELEROMETER,
    INPUT_TYPE_GYROSCOPE,
    INPUT_TYPE_MAGNETOMETER,
    INPUT_TYPE_PROXIMITY,
    INPUT_TYPE_AMBIENT_LIGHT,
    INPUT_TYPE_FINGERPRINT,
    INPUT_TYPE_IRIS_SCANNER,
    INPUT_TYPE_VOICE_COMMAND,
    INPUT_TYPE_MAX
} input_device_type_t;

/* Input event types */
typedef enum {
    INPUT_EVENT_KEY = 0,
    INPUT_EVENT_MOUSE_MOVE,
    INPUT_EVENT_MOUSE_BUTTON,
    INPUT_EVENT_MOUSE_WHEEL,
    INPUT_EVENT_TOUCH_DOWN,
    INPUT_EVENT_TOUCH_MOVE,
    INPUT_EVENT_TOUCH_UP,
    INPUT_EVENT_GESTURE,
    INPUT_EVENT_SENSOR,
    INPUT_EVENT_VOICE,
    INPUT_EVENT_MAX
} input_event_type_t;

/* Gesture types */
typedef enum {
    GESTURE_NONE = 0,
    GESTURE_TAP,
    GESTURE_DOUBLE_TAP,
    GESTURE_LONG_PRESS,
    GESTURE_SWIPE_LEFT,
    GESTURE_SWIPE_RIGHT,
    GESTURE_SWIPE_UP,
    GESTURE_SWIPE_DOWN,
    GESTURE_PINCH_IN,
    GESTURE_PINCH_OUT,
    GESTURE_ROTATE_CW,
    GESTURE_ROTATE_CCW,
    GESTURE_TWO_FINGER_TAP,
    GESTURE_THREE_FINGER_TAP,
    GESTURE_FOUR_FINGER_TAP,
    GESTURE_PALM_DETECTION,
    GESTURE_EDGE_SWIPE,
    GESTURE_MAX
} gesture_type_t;

/* Key codes (standard HID usage codes) */
typedef enum {
    KEY_RESERVED = 0,
    KEY_ESC = 1,
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
    KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_TAB,
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
    KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_ENTER, KEY_LEFTCTRL,
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L,
    KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE, KEY_LEFTSHIFT, KEY_BACKSLASH,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M,
    KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_RIGHTSHIFT,
    KEY_KPASTERISK, KEY_LEFTALT, KEY_SPACE, KEY_CAPSLOCK,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
    KEY_NUMLOCK, KEY_SCROLLLOCK,
    KEY_KP7, KEY_KP8, KEY_KP9, KEY_KPMINUS,
    KEY_KP4, KEY_KP5, KEY_KP6, KEY_KPPLUS,
    KEY_KP1, KEY_KP2, KEY_KP3, KEY_KP0, KEY_KPDOT,
    KEY_F11 = 87, KEY_F12 = 88,
    KEY_KPENTER = 96, KEY_RIGHTCTRL = 97, KEY_KPSLASH = 98, KEY_SYSRQ = 99,
    KEY_RIGHTALT = 100, KEY_HOME = 102, KEY_UP = 103, KEY_PAGEUP = 104,
    KEY_LEFT = 105, KEY_RIGHT = 106, KEY_END = 107, KEY_DOWN = 108,
    KEY_PAGEDOWN = 109, KEY_INSERT = 110, KEY_DELETE = 111,
    KEY_MUTE = 113, KEY_VOLUMEDOWN = 114, KEY_VOLUMEUP = 115,
    KEY_POWER = 116, KEY_PAUSE = 119,
    KEY_LEFTMETA = 125, KEY_RIGHTMETA = 126, KEY_COMPOSE = 127,
    KEY_MAX = 255
} key_code_t;

/* Mouse button codes */
typedef enum {
    MOUSE_BTN_LEFT = 0x001,
    MOUSE_BTN_RIGHT = 0x002,
    MOUSE_BTN_MIDDLE = 0x004,
    MOUSE_BTN_SIDE = 0x008,
    MOUSE_BTN_EXTRA = 0x010,
    MOUSE_BTN_FORWARD = 0x020,
    MOUSE_BTN_BACK = 0x040,
    MOUSE_BTN_TASK = 0x080
} mouse_button_t;

/* Touch contact information */
typedef struct touch_contact {
    uint8_t id;                 /* Contact ID */
    bool active;                /* Contact is active */
    
    /* Position */
    uint32_t x, y;              /* Raw coordinates */
    float normalized_x, normalized_y;  /* 0.0 to 1.0 */
    
    /* Pressure and size */
    uint32_t pressure;          /* Pressure value */
    uint32_t touch_major;       /* Major axis of touch area */
    uint32_t touch_minor;       /* Minor axis of touch area */
    uint32_t width_major;       /* Major axis of approaching tool */
    uint32_t width_minor;       /* Minor axis of approaching tool */
    
    /* Orientation */
    int32_t orientation;        /* Orientation of touch */
    
    /* Tool information */
    uint8_t tool_type;          /* Finger, stylus, palm, etc. */
    
    /* Timing */
    uint64_t down_time;         /* When contact was made */
    uint64_t move_time;         /* Last movement time */
    
} touch_contact_t;

/* Multi-touch state */
typedef struct multitouch_state {
    uint8_t contact_count;      /* Number of active contacts */
    touch_contact_t contacts[MAX_TOUCH_CONTACTS];
    
    /* Global touch state */
    bool touching;
    uint64_t session_start_time;
    
    /* Gesture recognition */
    struct {
        gesture_type_t current_gesture;
        float confidence;
        uint32_t gesture_points[MAX_GESTURE_POINTS][2];
        uint8_t point_count;
        uint64_t gesture_start_time;
    } gesture;
    
    /* Palm rejection */
    struct {
        bool enabled;
        uint32_t palm_threshold_major;
        uint32_t palm_threshold_pressure;
        bool palm_detected;
    } palm_rejection;
    
} multitouch_state_t;

/* Input event structure */
typedef struct input_event {
    uint64_t timestamp;         /* Event timestamp */
    input_event_type_t type;    /* Event type */
    uint32_t device_id;         /* Source device */
    
    union {
        /* Keyboard event */
        struct {
            key_code_t key_code;
            bool pressed;       /* True for press, false for release */
            uint16_t modifiers; /* Ctrl, Alt, Shift, etc. */
            uint32_t unicode;   /* Unicode character */
        } key;
        
        /* Mouse movement */
        struct {
            int32_t delta_x;
            int32_t delta_y;
            uint32_t absolute_x;
            uint32_t absolute_y;
        } mouse_move;
        
        /* Mouse button */
        struct {
            mouse_button_t button;
            bool pressed;
            uint32_t x, y;      /* Position at time of click */
        } mouse_button;
        
        /* Mouse wheel */
        struct {
            int32_t delta_x;    /* Horizontal scroll */
            int32_t delta_y;    /* Vertical scroll */
        } mouse_wheel;
        
        /* Touch event */
        struct {
            uint8_t contact_id;
            uint32_t x, y;
            uint32_t pressure;
            uint32_t touch_major;
            uint32_t touch_minor;
            uint8_t tool_type;
        } touch;
        
        /* Gesture event */
        struct {
            gesture_type_t gesture;
            float confidence;
            uint32_t center_x, center_y;
            float scale;        /* For pinch gestures */
            float rotation;     /* For rotation gestures */
            float velocity_x, velocity_y;  /* For swipe gestures */
        } gesture;
        
        /* Sensor event */
        struct {
            uint8_t sensor_type;
            float values[6];    /* Sensor data (x,y,z + optional) */
            uint8_t accuracy;
        } sensor;
        
        /* Voice command */
        struct {
            char command[64];
            float confidence;
            uint32_t duration_ms;
        } voice;
        
    } data;
    
    /* Event metadata */
    uint32_t flags;
    uint32_t sequence_number;
    
} input_event_t;

/* Input device capabilities */
typedef struct input_capabilities {
    /* Basic capabilities */
    bool supports_key_events;
    bool supports_mouse_events;
    bool supports_touch_events;
    bool supports_gesture_events;
    bool supports_sensor_events;
    
    /* Touch capabilities */
    struct {
        uint8_t max_contacts;
        bool supports_pressure;
        bool supports_size;
        bool supports_orientation;
        uint32_t max_x, max_y;
        uint32_t resolution_x, resolution_y;
        bool supports_palm_rejection;
    } touch;
    
    /* Keyboard capabilities */
    struct {
        bool supports_rollover;
        bool supports_unicode;
        bool has_numpad;
        bool has_function_keys;
        uint8_t num_function_keys;
    } keyboard;
    
    /* Mouse capabilities */
    struct {
        uint8_t button_count;
        bool supports_wheel;
        bool supports_horizontal_wheel;
        uint32_t max_dpi;
        bool supports_acceleration;
    } mouse;
    
    /* Sensor capabilities */
    struct {
        bool supports_accelerometer;
        bool supports_gyroscope;
        bool supports_magnetometer;
        bool supports_proximity;
        bool supports_ambient_light;
        uint32_t max_sample_rate_hz;
    } sensors;
    
} input_capabilities_t;

/* Input device configuration */
typedef struct input_device_config {
    /* General settings */
    bool enabled;
    uint8_t sensitivity;        /* 0-100 */
    
    /* Touch settings */
    struct {
        bool palm_rejection_enabled;
        uint32_t palm_threshold;
        bool gesture_recognition_enabled;
        float tap_timeout_ms;
        float double_tap_timeout_ms;
        uint32_t swipe_threshold_pixels;
        float pinch_threshold;
    } touch;
    
    /* Mouse settings */
    struct {
        uint32_t dpi;
        float acceleration;
        bool acceleration_enabled;
        uint8_t button_mapping[8];
        uint32_t double_click_time_ms;
    } mouse;
    
    /* Keyboard settings */
    struct {
        uint32_t repeat_delay_ms;
        uint32_t repeat_rate_ms;
        bool sticky_keys;
        bool filter_keys;
        uint8_t keymap_id;
    } keyboard;
    
} input_device_config_t;

/* Input device structure */
typedef struct input_device {
    uint32_t device_id;
    char name[64];
    char manufacturer[32];
    char product[32];
    char serial[32];
    
    /* Device type and capabilities */
    input_device_type_t type;
    input_capabilities_t caps;
    input_device_config_t config;
    
    /* Hardware information */
    struct device *dev;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t version;
    
    /* Connection information */
    enum {
        INPUT_BUS_USB = 0,
        INPUT_BUS_BLUETOOTH,
        INPUT_BUS_I2C,
        INPUT_BUS_SPI,
        INPUT_BUS_VIRTUAL,
        INPUT_BUS_MAX
    } bus_type;
    
    /* Current state */
    bool connected;
    bool suspended;
    uint32_t last_activity_time;
    
    /* Multi-touch state (for touch devices) */
    multitouch_state_t *mt_state;
    
    /* Statistics */
    struct {
        uint64_t events_generated;
        uint64_t gestures_recognized;
        uint64_t errors;
        uint64_t last_event_time;
        float average_event_rate;
    } stats;
    
    /* Power management */
    struct {
        bool supports_runtime_pm;
        uint32_t idle_timeout_ms;
        uint32_t current_power_state;
    } power;
    
    /* Device operations */
    const struct input_device_ops *ops;
    
    /* Event handling */
    struct {
        input_event_t event_buffer[MAX_INPUT_EVENTS];
        uint32_t buffer_head;
        uint32_t buffer_tail;
        spinlock_t buffer_lock;
        wait_queue_head_t wait_queue;
    } event_queue;
    
    /* Private driver data */
    void *private_data;
    
    struct list_head list;
    
} input_device_t;

/* Input device operations */
struct input_device_ops {
    /* Device lifecycle */
    int (*probe)(struct input_device *dev);
    void (*remove)(struct input_device *dev);
    
    /* Event handling */
    int (*start)(struct input_device *dev);
    void (*stop)(struct input_device *dev);
    void (*flush)(struct input_device *dev);
    
    /* Configuration */
    int (*set_config)(struct input_device *dev, const struct input_device_config *config);
    int (*get_config)(struct input_device *dev, struct input_device_config *config);
    
    /* Calibration (for touch devices) */
    int (*calibrate)(struct input_device *dev);
    int (*set_calibration_data)(struct input_device *dev, const void *data, size_t size);
    
    /* Power management */
    int (*suspend)(struct input_device *dev);
    int (*resume)(struct input_device *dev);
    
    /* Firmware update */
    int (*update_firmware)(struct input_device *dev, const void *fw_data, size_t size);
    
    /* Debugging */
    void (*debug_info)(struct input_device *dev, char *buffer, size_t size);
};

/* Gesture recognition engine */
typedef struct gesture_recognizer {
    bool enabled;
    
    /* Recognition parameters */
    struct {
        uint32_t min_points;
        uint32_t max_points;
        float min_distance;
        float max_distance;
        uint32_t timeout_ms;
        float confidence_threshold;
    } params;
    
    /* Gesture templates */
    struct {
        gesture_type_t type;
        float template_points[MAX_GESTURE_POINTS][2];
        uint8_t point_count;
        float variance_threshold;
    } templates[GESTURE_MAX];
    
    /* Current recognition state */
    struct {
        bool active;
        gesture_type_t current_gesture;
        float confidence;
        uint64_t start_time;
        uint32_t points[MAX_GESTURE_POINTS][2];
        uint8_t point_count;
    } state;
    
    /* Statistics */
    struct {
        uint64_t gestures_attempted;
        uint64_t gestures_recognized;
        float average_confidence;
        uint64_t false_positives;
    } stats;
    
} gesture_recognizer_t;

/* Accessibility features */
typedef struct accessibility_features {
    /* Keyboard accessibility */
    struct {
        bool sticky_keys_enabled;
        bool filter_keys_enabled;
        bool toggle_keys_enabled;
        bool slow_keys_enabled;
        uint32_t slow_keys_delay_ms;
        bool bounce_keys_enabled;
        uint32_t bounce_keys_delay_ms;
    } keyboard_access;
    
    /* Mouse accessibility */
    struct {
        bool mouse_keys_enabled;
        bool click_assist_enabled;
        bool dwell_click_enabled;
        uint32_t dwell_time_ms;
        bool cursor_enhancement_enabled;
        uint32_t cursor_size;
    } mouse_access;
    
    /* Touch accessibility */
    struct {
        bool assistive_touch_enabled;
        bool touch_accommodations_enabled;
        uint32_t hold_duration_ms;
        bool ignore_repeat_enabled;
        uint32_t ignore_repeat_timeout_ms;
    } touch_access;
    
    /* Visual accessibility */
    struct {
        bool switch_control_enabled;
        bool voice_control_enabled;
        float voice_sensitivity;
    } control_access;
    
} accessibility_features_t;

/* Global input subsystem state */
typedef struct input_subsystem {
    bool initialized;
    
    /* Device registry */
    struct {
        input_device_t *devices[MAX_INPUT_DEVICES];
        uint32_t count;
        uint32_t next_device_id;
        rwlock_t lock;
    } devices;
    
    /* Event processing */
    struct {
        struct workqueue_struct *event_workqueue;
        input_event_t global_event_buffer[MAX_INPUT_EVENTS * 4];
        uint32_t buffer_head;
        uint32_t buffer_tail;
        spinlock_t buffer_lock;
    } event_processing;
    
    /* Gesture recognition */
    gesture_recognizer_t gesture_engine;
    
    /* Accessibility */
    accessibility_features_t accessibility;
    
    /* Global statistics */
    struct {
        uint64_t total_events;
        uint64_t key_events;
        uint64_t mouse_events;
        uint64_t touch_events;
        uint64_t gesture_events;
        uint64_t sensor_events;
        float events_per_second;
    } stats;
    
    /* Configuration */
    struct {
        bool gesture_recognition_enabled;
        bool accessibility_enabled;
        uint32_t event_buffer_size;
        uint32_t max_event_rate;
    } config;
    
} input_subsystem_t;

/* External input subsystem */
extern input_subsystem_t input_subsystem;

/* Core input functions */
int input_init(void);
void input_exit(void);

/* Device management */
int input_register_device(struct input_device *dev);
void input_unregister_device(struct input_device *dev);
struct input_device *input_get_device_by_id(uint32_t device_id);
struct input_device *input_find_device_by_name(const char *name);

/* Event handling */
int input_report_event(struct input_device *dev, const struct input_event *event);
int input_read_events(struct input_event *events, uint32_t max_events, uint32_t timeout_ms);
void input_inject_event(const struct input_event *event);

/* Keyboard functions */
int input_report_key(struct input_device *dev, key_code_t key, bool pressed, uint16_t modifiers);
bool input_is_key_pressed(key_code_t key);
uint16_t input_get_modifier_state(void);

/* Mouse functions */
int input_report_mouse_move(struct input_device *dev, int32_t delta_x, int32_t delta_y);
int input_report_mouse_button(struct input_device *dev, mouse_button_t button, bool pressed, uint32_t x, uint32_t y);
int input_report_mouse_wheel(struct input_device *dev, int32_t delta_x, int32_t delta_y);

/* Touch functions */
int input_report_touch_down(struct input_device *dev, uint8_t contact_id, uint32_t x, uint32_t y, uint32_t pressure);
int input_report_touch_move(struct input_device *dev, uint8_t contact_id, uint32_t x, uint32_t y, uint32_t pressure);
int input_report_touch_up(struct input_device *dev, uint8_t contact_id);
int input_touch_calibrate(struct input_device *dev, uint32_t screen_width, uint32_t screen_height);

/* Gesture recognition */
int input_gesture_init(struct gesture_recognizer *recognizer);
void input_gesture_cleanup(struct gesture_recognizer *recognizer);
gesture_type_t input_gesture_recognize(struct gesture_recognizer *recognizer, 
                                      const uint32_t points[][2], uint8_t point_count);
int input_gesture_add_template(struct gesture_recognizer *recognizer, gesture_type_t type,
                              const uint32_t points[][2], uint8_t point_count);

/* Accessibility functions */
int input_accessibility_enable(void);
void input_accessibility_disable(void);
int input_accessibility_set_config(const struct accessibility_features *config);
int input_accessibility_get_config(struct accessibility_features *config);

/* Configuration */
int input_device_set_config(struct input_device *dev, const struct input_device_config *config);
int input_device_get_config(struct input_device *dev, struct input_device_config *config);
int input_set_global_config(const struct input_subsystem *config);

/* Power management */
int input_device_suspend(struct input_device *dev);
int input_device_resume(struct input_device *dev);
void input_device_set_idle_timeout(struct input_device *dev, uint32_t timeout_ms);

/* Statistics and debugging */
void input_print_statistics(void);
void input_print_device_info(struct input_device *dev);
void input_debug_event(const struct input_event *event);

/* Utility functions */
const char *input_device_type_name(input_device_type_t type);
const char *input_event_type_name(input_event_type_t type);
const char *gesture_type_name(gesture_type_t type);
uint64_t input_get_timestamp(void);

/* Event filtering and transformation */
int input_add_event_filter(bool (*filter_fn)(const struct input_event *event));
void input_remove_event_filter(bool (*filter_fn)(const struct input_event *event));
int input_transform_coordinates(struct input_device *dev, uint32_t *x, uint32_t *y);

/* Hot-plug support */
void input_device_hotplug_notify(struct input_device *dev, bool connected);
int input_scan_for_devices(void);

/* Driver interface helpers */
struct input_device *input_allocate_device(void);
void input_free_device(struct input_device *dev);
int input_set_capability(struct input_device *dev, unsigned int type, unsigned int code);
void input_set_absinfo(struct input_device *dev, unsigned int axis, const struct input_absinfo *absinfo);