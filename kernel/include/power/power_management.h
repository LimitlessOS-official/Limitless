/*
 * LimitlessOS Advanced Power Management System
 * Comprehensive power states, thermal management, and battery optimization
 */

#ifndef LIMITLESSOS_POWER_MANAGEMENT_H
#define LIMITLESSOS_POWER_MANAGEMENT_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

/* Power States */
typedef enum {
    POWER_STATE_S0_ACTIVE = 0,      /* Fully active */
    POWER_STATE_S0ix_STANDBY,       /* Modern standby */
    POWER_STATE_S1_STANDBY,         /* CPU stopped, RAM powered */
    POWER_STATE_S2_STANDBY,         /* CPU off, dirty cache flushed */
    POWER_STATE_S3_SUSPEND,         /* Suspend to RAM */
    POWER_STATE_S4_HIBERNATE,       /* Suspend to disk */
    POWER_STATE_S5_SHUTDOWN,        /* Soft power off */
    POWER_STATE_G3_MECHANICAL_OFF,  /* Complete power off */
    POWER_STATE_MAX
} power_state_t;

/* CPU Power States (P-States and C-States) */
typedef enum {
    CPU_PSTATE_P0 = 0,     /* Maximum performance */
    CPU_PSTATE_P1,         /* Reduced performance */
    CPU_PSTATE_P2,         /* Further reduced performance */
    CPU_PSTATE_P3,         /* Minimum performance */
    CPU_PSTATE_MAX
} cpu_pstate_t;

typedef enum {
    CPU_CSTATE_C0 = 0,     /* Active */
    CPU_CSTATE_C1,         /* Halt */
    CPU_CSTATE_C2,         /* Stop clock */
    CPU_CSTATE_C3,         /* Sleep */
    CPU_CSTATE_C6,         /* Deep sleep */
    CPU_CSTATE_C7,         /* Deeper sleep */
    CPU_CSTATE_C8,         /* Deepest sleep */
    CPU_CSTATE_MAX
} cpu_cstate_t;

/* Device Power States */
typedef enum {
    DEVICE_POWER_D0 = 0,   /* Fully on */
    DEVICE_POWER_D1,       /* Intermediate state */
    DEVICE_POWER_D2,       /* Intermediate state */
    DEVICE_POWER_D3_HOT,   /* Off but powered */
    DEVICE_POWER_D3_COLD,  /* Completely off */
    DEVICE_POWER_MAX
} device_power_state_t;

/* Thermal Management */
typedef enum {
    THERMAL_ZONE_CPU = 0,
    THERMAL_ZONE_GPU,
    THERMAL_ZONE_CHIPSET,
    THERMAL_ZONE_BATTERY,
    THERMAL_ZONE_AMBIENT,
    THERMAL_ZONE_STORAGE,
    THERMAL_ZONE_MAX
} thermal_zone_t;

typedef struct {
    thermal_zone_t zone;
    char name[32];
    uint32_t temperature;    /* Current temperature (milli-celsius) */
    uint32_t critical_temp;  /* Critical temperature threshold */
    uint32_t hot_temp;      /* Hot temperature threshold */
    uint32_t warm_temp;     /* Warm temperature threshold */
    uint32_t cool_temp;     /* Cool temperature threshold */
    bool cooling_active;    /* Cooling system active */
    uint32_t fan_speed;     /* Current fan speed (RPM) */
    uint32_t throttle_level; /* Current throttling level */
} thermal_sensor_t;

/* Power Policy */
typedef enum {
    POWER_POLICY_PERFORMANCE = 0,  /* Maximum performance */
    POWER_POLICY_BALANCED,         /* Balance performance and power */
    POWER_POLICY_POWER_SAVER,      /* Maximum power savings */
    POWER_POLICY_CUSTOM,           /* Custom user-defined policy */
    POWER_POLICY_MAX
} power_policy_t;

/* Battery Management */
typedef struct {
    uint32_t capacity_mah;          /* Battery capacity in mAh */
    uint32_t current_charge_mah;    /* Current charge in mAh */
    uint32_t voltage_mv;            /* Current voltage in mV */
    int32_t current_ma;             /* Current flow in mA (+ charging, - discharging) */
    uint32_t temperature;           /* Battery temperature (milli-celsius) */
    uint32_t cycle_count;           /* Number of charge cycles */
    uint32_t health_percent;        /* Battery health percentage */
    uint32_t time_to_empty;         /* Time to empty (minutes) */
    uint32_t time_to_full;          /* Time to full charge (minutes) */
    bool is_charging;               /* Charging status */
    bool is_present;                /* Battery present */
    bool is_ac_connected;           /* AC adapter connected */
    char manufacturer[32];          /* Battery manufacturer */
    char model[32];                 /* Battery model */
    char serial_number[32];         /* Battery serial number */
} battery_info_t;

/* Power Management Configuration */
typedef struct {
    power_policy_t active_policy;
    
    /* CPU power settings */
    uint32_t cpu_min_freq_mhz;      /* Minimum CPU frequency */
    uint32_t cpu_max_freq_mhz;      /* Maximum CPU frequency */
    uint32_t cpu_target_freq_mhz;   /* Target CPU frequency */
    uint32_t cpu_governor_type;     /* CPU frequency governor */
    bool cpu_turbo_enabled;         /* Turbo boost enabled */
    
    /* Display settings */
    uint32_t display_brightness;    /* Display brightness (0-100) */
    uint32_t display_timeout_ms;    /* Display timeout in milliseconds */
    bool display_auto_brightness;   /* Automatic brightness adjustment */
    
    /* Suspend/hibernate settings */
    uint32_t suspend_timeout_ms;    /* Suspend timeout */
    uint32_t hibernate_timeout_ms;  /* Hibernate timeout */
    bool wake_on_lan;              /* Wake on LAN enabled */
    bool wake_on_usb;              /* Wake on USB enabled */
    
    /* Battery settings */
    uint32_t low_battery_warning;   /* Low battery warning threshold */
    uint32_t critical_battery;      /* Critical battery threshold */
    bool battery_saver_mode;        /* Battery saver mode enabled */
    uint32_t charge_limit_percent;  /* Maximum charge limit */
    
    /* Thermal settings */
    bool active_cooling;            /* Active cooling enabled */
    uint32_t fan_curve[10];        /* Fan speed curve */
    uint32_t thermal_throttle_temp; /* Temperature threshold for throttling */
} power_config_t;

/* Power Event Types */
typedef enum {
    POWER_EVENT_SUSPEND = 0,
    POWER_EVENT_RESUME,
    POWER_EVENT_HIBERNATE,
    POWER_EVENT_BATTERY_LOW,
    POWER_EVENT_BATTERY_CRITICAL,
    POWER_EVENT_AC_CONNECTED,
    POWER_EVENT_AC_DISCONNECTED,
    POWER_EVENT_THERMAL_WARNING,
    POWER_EVENT_THERMAL_CRITICAL,
    POWER_EVENT_MAX
} power_event_t;

/* Power Event Handler */
typedef int (*power_event_handler_t)(power_event_t event, void *data);

/* Device Power Management */
typedef struct device_pm {
    char device_name[64];
    device_power_state_t current_state;
    device_power_state_t target_state;
    bool can_wakeup;
    bool runtime_pm_enabled;
    uint64_t suspend_time;
    uint64_t resume_time;
    uint32_t suspend_count;
    uint32_t resume_count;
    
    /* Device-specific power operations */
    int (*suspend)(struct device_pm *dev);
    int (*resume)(struct device_pm *dev);
    int (*runtime_suspend)(struct device_pm *dev);
    int (*runtime_resume)(struct device_pm *dev);
    
    struct device_pm *next;
} device_pm_t;

/* Power Management System */
typedef struct {
    bool initialized;
    pthread_mutex_t lock;
    
    /* Current system state */
    power_state_t current_state;
    power_state_t target_state;
    power_policy_t current_policy;
    
    /* Configuration */
    power_config_t config;
    
    /* Battery management */
    battery_info_t battery;
    bool battery_present;
    
    /* Thermal management */
    thermal_sensor_t thermal_zones[THERMAL_ZONE_MAX];
    uint32_t active_thermal_zones;
    
    /* CPU power management */
    cpu_pstate_t current_pstate;
    cpu_cstate_t current_cstate;
    uint32_t cpu_freq_mhz;
    bool cpu_scaling_enabled;
    
    /* Device power management */
    device_pm_t *device_list;
    uint32_t device_count;
    
    /* Power events */
    power_event_handler_t event_handlers[POWER_EVENT_MAX];
    
    /* Statistics */
    struct {
        uint64_t suspend_count;
        uint64_t resume_count;
        uint64_t hibernate_count;
        uint64_t total_suspend_time;
        uint64_t total_resume_time;
        uint64_t power_consumed_mwh;
        uint64_t battery_cycles;
    } stats;
    
    /* Threading */
    pthread_t thermal_thread;
    pthread_t battery_thread;
    pthread_t governor_thread;
    bool threads_running;
} power_management_t;

/* Function Prototypes */

/* Initialization */
int power_management_init(void);
int power_management_cleanup(void);

/* System Power States */
int power_set_state(power_state_t state);
power_state_t power_get_state(void);
int power_suspend_to_ram(void);
int power_hibernate_to_disk(void);
int power_shutdown_system(void);
int power_reboot_system(void);

/* CPU Power Management */
int cpu_set_pstate(uint32_t cpu_id, cpu_pstate_t pstate);
cpu_pstate_t cpu_get_pstate(uint32_t cpu_id);
int cpu_set_cstate(uint32_t cpu_id, cpu_cstate_t cstate);
cpu_cstate_t cpu_get_cstate(uint32_t cpu_id);
int cpu_set_frequency(uint32_t cpu_id, uint32_t freq_mhz);
uint32_t cpu_get_frequency(uint32_t cpu_id);
int cpu_enable_turbo(uint32_t cpu_id, bool enable);

/* Device Power Management */
int device_pm_register(device_pm_t *device);
int device_pm_unregister(device_pm_t *device);
int device_pm_suspend(const char *device_name);
int device_pm_resume(const char *device_name);
int device_pm_runtime_suspend(const char *device_name);
int device_pm_runtime_resume(const char *device_name);

/* Battery Management */
int battery_update_info(void);
battery_info_t *battery_get_info(void);
int battery_set_charge_limit(uint32_t percent);
bool battery_is_charging(void);
bool battery_is_ac_connected(void);
uint32_t battery_get_capacity_percent(void);

/* Thermal Management */
int thermal_init(void);
int thermal_add_zone(thermal_zone_t zone, const char *name);
int thermal_update_temperature(thermal_zone_t zone, uint32_t temp);
uint32_t thermal_get_temperature(thermal_zone_t zone);
int thermal_set_cooling_policy(thermal_zone_t zone, bool active_cooling);
int thermal_throttle_cpu(uint32_t level);

/* Power Policy */
int power_set_policy(power_policy_t policy);
power_policy_t power_get_policy(void);
int power_create_custom_policy(power_config_t *config);
int power_load_policy_from_file(const char *filename);
int power_save_policy_to_file(const char *filename);

/* Configuration */
int power_set_config(const power_config_t *config);
power_config_t *power_get_config(void);
int power_set_display_brightness(uint32_t brightness);
uint32_t power_get_display_brightness(void);
int power_set_suspend_timeout(uint32_t timeout_ms);
int power_set_hibernate_timeout(uint32_t timeout_ms);

/* Event Management */
int power_register_event_handler(power_event_t event, power_event_handler_t handler);
int power_unregister_event_handler(power_event_t event);
int power_notify_event(power_event_t event, void *data);

/* Wake Sources */
int power_enable_wake_source(const char *device_name);
int power_disable_wake_source(const char *device_name);
bool power_is_wake_source_enabled(const char *device_name);

/* Power Statistics */
int power_get_statistics(void *stats_buffer, size_t buffer_size);
uint64_t power_get_uptime_ms(void);
uint64_t power_get_suspend_time_ms(void);
double power_get_average_consumption_mw(void);

/* Hardware Abstraction */
int power_acpi_init(void);
int power_read_acpi_battery(battery_info_t *battery);
int power_read_acpi_thermal(thermal_zone_t zone, uint32_t *temperature);
int power_set_acpi_state(power_state_t state);

/* Modern Standby (S0ix) */
int power_enter_modern_standby(void);
int power_exit_modern_standby(void);
bool power_is_modern_standby_supported(void);
int power_configure_s0ix_constraints(void);

/* Dynamic Frequency Scaling */
int dfs_init(void);
int dfs_set_governor(const char *governor);
const char *dfs_get_governor(void);
int dfs_set_frequency_range(uint32_t min_mhz, uint32_t max_mhz);
int dfs_update_load(uint32_t cpu_id, uint32_t load_percent);

/* Utility Functions */
const char *power_state_name(power_state_t state);
const char *power_policy_name(power_policy_t policy);
const char *thermal_zone_name(thermal_zone_t zone);
bool power_is_on_battery(void);
bool power_is_low_battery(void);
uint32_t power_estimate_battery_time(void);

#endif /* LIMITLESSOS_POWER_MANAGEMENT_H */