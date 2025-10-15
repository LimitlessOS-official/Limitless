/*
 * LimitlessOS Advanced Power Management Subsystem
 * Production power management with AI optimization and enterprise features
 * Features: ACPI, runtime PM, CPU governors, thermal management, battery optimization
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../kernel.h"

/* Power management constants */
#define MAX_POWER_DOMAINS       64
#define MAX_CPU_FREQ_LEVELS     32
#define MAX_THERMAL_ZONES       16
#define MAX_COOLING_DEVICES     32
#define MAX_POWER_SUPPLIES      8
#define MAX_WAKE_SOURCES        128

/* Power states (ACPI-compatible) */
typedef enum {
    POWER_STATE_S0 = 0,        /* Working state */
    POWER_STATE_S1,            /* Standby */
    POWER_STATE_S2,            /* Suspend to RAM (shallow) */
    POWER_STATE_S3,            /* Suspend to RAM (deep sleep) */
    POWER_STATE_S4,            /* Suspend to disk (hibernate) */
    POWER_STATE_S5,            /* Soft off */
    POWER_STATE_G3,            /* Mechanical off */
    POWER_STATE_MAX
} system_power_state_t;

/* Device power states (ACPI D-states) */
typedef enum {
    DEVICE_POWER_D0 = 0,       /* Fully on */
    DEVICE_POWER_D1,           /* Low power on */
    DEVICE_POWER_D2,           /* Standby */
    DEVICE_POWER_D3_HOT,       /* Sleep with context retained */
    DEVICE_POWER_D3_COLD,      /* Off */
    DEVICE_POWER_MAX
} device_power_state_t;

/* CPU power states (C-states) */
typedef enum {
    CPU_STATE_C0 = 0,          /* Active */
    CPU_STATE_C1,              /* Halt */
    CPU_STATE_C1E,             /* Enhanced halt */
    CPU_STATE_C2,              /* Stop clock */
    CPU_STATE_C3,              /* Sleep */
    CPU_STATE_C6,              /* Deep sleep */
    CPU_STATE_C7,              /* Deeper sleep */
    CPU_STATE_C8,              /* Deepest sleep */
    CPU_STATE_C9,              /* Ultra low power */
    CPU_STATE_C10,             /* Package C-state */
    CPU_STATE_MAX
} cpu_power_state_t;

/* CPU frequency scaling governors */
typedef enum {
    CPU_GOVERNOR_PERFORMANCE = 0,   /* Maximum performance */
    CPU_GOVERNOR_POWERSAVE,         /* Minimum power */
    CPU_GOVERNOR_ONDEMAND,          /* Dynamic scaling */
    CPU_GOVERNOR_CONSERVATIVE,      /* Gradual scaling */
    CPU_GOVERNOR_USERSPACE,         /* User controlled */
    CPU_GOVERNOR_SCHEDUTIL,         /* Scheduler driven */
    CPU_GOVERNOR_AI_ADAPTIVE,       /* AI-driven optimization */
    CPU_GOVERNOR_MAX
} cpu_governor_t;

/* Power supply types */
typedef enum {
    POWER_SUPPLY_UNKNOWN = 0,
    POWER_SUPPLY_BATTERY,
    POWER_SUPPLY_AC_ADAPTER,
    POWER_SUPPLY_USB,
    POWER_SUPPLY_WIRELESS,
    POWER_SUPPLY_SOLAR,
    POWER_SUPPLY_MAX
} power_supply_type_t;

/* Battery health states */
typedef enum {
    BATTERY_HEALTH_UNKNOWN = 0,
    BATTERY_HEALTH_GOOD,
    BATTERY_HEALTH_OVERHEAT,
    BATTERY_HEALTH_DEAD,
    BATTERY_HEALTH_OVER_VOLTAGE,
    BATTERY_HEALTH_UNSPEC_FAILURE,
    BATTERY_HEALTH_COLD,
    BATTERY_HEALTH_WATCHDOG_TIMER_EXPIRE,
    BATTERY_HEALTH_SAFETY_TIMER_EXPIRE,
    BATTERY_HEALTH_WARM,
    BATTERY_HEALTH_COOL,
    BATTERY_HEALTH_HOT,
    BATTERY_HEALTH_MAX
} battery_health_t;

/* Thermal trip point types */
typedef enum {
    THERMAL_TRIP_ACTIVE = 0,    /* Cooling device activation */
    THERMAL_TRIP_PASSIVE,       /* Passive cooling */
    THERMAL_TRIP_HOT,          /* Hot trip point */
    THERMAL_TRIP_CRITICAL,     /* Critical shutdown */
    THERMAL_TRIP_MAX
} thermal_trip_type_t;

/* AI power prediction neural network */
typedef struct power_ai_network {
    bool enabled;
    
    /* Network architecture */
    struct {
        uint32_t input_size;
        uint32_t hidden_layers;
        uint32_t hidden_size;
        uint32_t output_size;
    } architecture;
    
    /* Network weights and biases */
    struct {
        float *input_weights;
        float *hidden_weights;
        float *output_weights;
        float *input_bias;
        float *hidden_bias;
        float *output_bias;
    } parameters;
    
    /* Training data */
    struct {
        float *input_data;
        float *target_data;
        uint32_t sample_count;
        uint32_t max_samples;
    } training;
    
    /* Performance metrics */
    struct {
        float accuracy;
        float power_savings_percent;
        uint64_t predictions_made;
        uint64_t correct_predictions;
        uint64_t training_iterations;
    } metrics;
    
} power_ai_network_t;

/* CPU frequency table entry */
typedef struct cpu_freq_level {
    uint32_t frequency_khz;     /* Frequency in kHz */
    uint32_t voltage_uv;        /* Voltage in microvolts */
    uint32_t power_mw;          /* Power consumption in mW */
    uint32_t transition_latency_us; /* Transition latency */
    bool available;             /* Level is available */
    
} cpu_freq_level_t;

/* CPU power domain */
typedef struct cpu_power_domain {
    uint32_t domain_id;
    char name[32];
    
    /* CPU mask for this domain */
    cpumask_t cpu_mask;
    
    /* Frequency scaling */
    struct {
        cpu_governor_t current_governor;
        uint32_t current_freq_khz;
        uint32_t min_freq_khz;
        uint32_t max_freq_khz;
        
        /* Frequency table */
        cpu_freq_level_t freq_table[MAX_CPU_FREQ_LEVELS];
        uint32_t freq_count;
        
        /* Scaling parameters */
        uint32_t up_threshold;      /* Load threshold to scale up */
        uint32_t down_threshold;    /* Load threshold to scale down */
        uint32_t sampling_rate_ms;  /* Governor sampling rate */
        
    } freq_scaling;
    
    /* Idle states */
    struct {
        cpu_power_state_t current_state;
        cpu_power_state_t deepest_state;
        
        /* C-state table */
        struct {
            cpu_power_state_t state;
            uint32_t exit_latency_us;
            uint32_t target_residency_us;
            uint32_t power_usage_mw;
            bool available;
            uint64_t usage_count;
            uint64_t time_ns;
        } states[CPU_STATE_MAX];
        
    } idle;
    
    /* Performance counters */
    struct {
        uint64_t frequency_changes;
        uint64_t idle_entries;
        uint64_t idle_time_ns;
        uint64_t active_time_ns;
        float average_load;
        float power_efficiency;
    } stats;
    
    /* AI optimization */
    power_ai_network_t ai_predictor;
    
} cpu_power_domain_t;

/* Power supply information */
typedef struct power_supply {
    uint32_t supply_id;
    char name[64];
    char manufacturer[32];
    char model[32];
    char serial[32];
    
    power_supply_type_t type;
    
    /* Current status */
    struct {
        bool online;
        bool charging;
        bool discharging;
        bool full;
        uint32_t capacity_percent;      /* 0-100 */
        uint32_t voltage_uv;           /* Microvolts */
        int32_t current_ua;            /* Microamps (+ charging, - discharging) */
        int32_t power_uw;              /* Microwatts */
        int32_t temperature_decidegrees; /* Temperature in 0.1°C */
        uint32_t time_to_empty_min;    /* Minutes to empty */
        uint32_t time_to_full_min;     /* Minutes to full */
    } status;
    
    /* Battery information (for battery supplies) */
    struct {
        battery_health_t health;
        uint32_t design_capacity_mah;   /* Design capacity */
        uint32_t full_capacity_mah;     /* Full charge capacity */
        uint32_t remaining_capacity_mah; /* Remaining capacity */
        uint32_t cycle_count;          /* Charge cycles */
        uint32_t design_voltage_uv;    /* Design voltage */
        uint32_t min_voltage_uv;       /* Minimum voltage */
        uint32_t max_voltage_uv;       /* Maximum voltage */
        
        /* Chemistry information */
        char chemistry[16];            /* Li-ion, NiMH, etc. */
        uint32_t manufacture_date;     /* Manufacturing date */
        
        /* Smart battery features */
        bool smart_battery;
        uint32_t remaining_time_alarm; /* Low battery alarm */
        uint32_t critical_capacity;    /* Critical capacity level */
        
    } battery;
    
    /* Power supply operations */
    const struct power_supply_ops *ops;
    
    /* Statistics */
    struct {
        uint64_t charge_cycles;
        uint64_t total_energy_charged_mwh;
        uint64_t total_energy_discharged_mwh;
        float charge_efficiency_percent;
        uint32_t max_temperature_reached;
        uint32_t min_voltage_reached;
    } stats;
    
    /* Private driver data */
    void *private_data;
    
    struct list_head list;
    
} power_supply_t;

/* Thermal zone */
typedef struct thermal_zone {
    uint32_t zone_id;
    char name[64];
    char type[32];
    
    /* Temperature information */
    struct {
        int32_t temperature_millicelsius; /* Current temperature */
        int32_t critical_temp;            /* Critical temperature */
        int32_t hot_temp;                 /* Hot temperature */
        int32_t passive_temp;             /* Passive cooling temp */
        
        /* Temperature history for trend analysis */
        int32_t temp_history[16];
        uint8_t history_index;
        
    } thermal;
    
    /* Trip points */
    struct {
        thermal_trip_type_t type;
        int32_t temperature;
        uint32_t hysteresis;
        bool enabled;
        
        /* Associated cooling device */
        uint32_t cooling_device_id;
        
    } trip_points[8];
    uint8_t trip_count;
    
    /* Thermal governor */
    enum {
        THERMAL_GOV_STEP_WISE = 0,
        THERMAL_GOV_FAIR_SHARE,
        THERMAL_GOV_BANG_BANG,
        THERMAL_GOV_USER_SPACE,
        THERMAL_GOV_AI_ADAPTIVE,
        THERMAL_GOV_MAX
    } governor;
    
    /* Zone operations */
    const struct thermal_zone_ops *ops;
    
    /* Statistics */
    struct {
        int32_t max_temp_recorded;
        int32_t min_temp_recorded;
        uint32_t over_temp_events;
        uint32_t cooling_activations;
        uint64_t throttling_time_ms;
    } stats;
    
    /* Private driver data */
    void *private_data;
    
    struct list_head list;
    
} thermal_zone_t;

/* Cooling device */
typedef struct cooling_device {
    uint32_t device_id;
    char name[64];
    char type[32];
    
    /* Cooling capabilities */
    struct {
        uint32_t max_state;        /* Maximum cooling state */
        uint32_t current_state;    /* Current cooling state */
        bool supports_stats;       /* Statistics support */
    } caps;
    
    /* Performance curve */
    struct {
        uint32_t state;
        uint32_t power_mw;         /* Power at this state */
        uint32_t performance;      /* Performance percentage */
    } curve[16];
    uint8_t curve_points;
    
    /* Device operations */
    const struct cooling_device_ops *ops;
    
    /* Statistics */
    struct {
        uint64_t state_changes;
        uint64_t total_cooling_time_ms;
        uint32_t max_state_reached;
        float average_state;
        uint32_t power_saved_mw;
    } stats;
    
    /* Private driver data */
    void *private_data;
    
    struct list_head list;
    
} cooling_device_t;

/* Wake source */
typedef struct wake_source {
    uint32_t source_id;
    char name[64];
    
    /* Wake source properties */
    bool enabled;
    bool active;
    bool system_wake_capable;
    
    /* Wake statistics */
    struct {
        uint64_t wakeup_count;
        uint64_t active_count;
        uint64_t total_time_ms;
        uint64_t max_time_ms;
        uint64_t last_time_ms;
        bool prevent_suspend_time_valid;
        uint64_t prevent_suspend_time_ms;
    } stats;
    
    /* Associated device */
    struct device *dev;
    
    struct list_head list;
    
} wake_source_t;

/* Power management policy */
typedef struct power_policy {
    char name[64];
    
    /* CPU policy */
    struct {
        cpu_governor_t governor;
        uint32_t max_freq_khz;
        uint32_t min_freq_khz;
        bool turbo_enabled;
        uint32_t idle_timeout_ms;
    } cpu;
    
    /* Display policy */
    struct {
        uint32_t brightness_percent;
        uint32_t dim_timeout_s;
        uint32_t off_timeout_s;
        bool adaptive_brightness;
    } display;
    
    /* Storage policy */
    struct {
        uint32_t spindown_timeout_s;
        bool write_cache_enabled;
        bool power_management_enabled;
    } storage;
    
    /* Network policy */
    struct {
        bool wake_on_lan_enabled;
        bool power_save_mode;
        uint32_t scan_interval_s;
    } network;
    
    /* USB policy */
    struct {
        bool autosuspend_enabled;
        uint32_t autosuspend_delay_ms;
    } usb;
    
    /* Audio policy */
    struct {
        uint32_t idle_timeout_ms;
        bool dynamic_power_control;
    } audio;
    
    /* System policy */
    struct {
        bool hibernate_enabled;
        bool hybrid_sleep_enabled;
        uint32_t suspend_timeout_s;
        uint32_t hibernate_timeout_s;
    } system;
    
} power_policy_t;

/* Global power management subsystem */
typedef struct power_management {
    bool initialized;
    
    /* Current system state */
    system_power_state_t current_state;
    system_power_state_t target_state;
    
    /* CPU power domains */
    struct {
        cpu_power_domain_t domains[MAX_POWER_DOMAINS];
        uint32_t count;
        rwlock_t lock;
    } cpu_domains;
    
    /* Power supplies */
    struct {
        power_supply_t *supplies[MAX_POWER_SUPPLIES];
        uint32_t count;
        rwlock_t lock;
    } supplies;
    
    /* Thermal management */
    struct {
        thermal_zone_t *zones[MAX_THERMAL_ZONES];
        cooling_device_t *cooling_devices[MAX_COOLING_DEVICES];
        uint32_t zone_count;
        uint32_t cooling_count;
        rwlock_t lock;
        
        /* Global thermal state */
        bool thermal_throttling_active;
        int32_t highest_temperature;
        uint32_t active_cooling_devices;
        
    } thermal;
    
    /* Wake sources */
    struct {
        wake_source_t *sources[MAX_WAKE_SOURCES];
        uint32_t count;
        uint32_t active_sources;
        rwlock_t lock;
    } wake_sources;
    
    /* Current power policy */
    power_policy_t current_policy;
    
    /* AI power optimization */
    struct {
        bool enabled;
        power_ai_network_t system_predictor;
        
        /* Learning parameters */
        float learning_rate;
        uint32_t prediction_window_ms;
        uint32_t adaptation_period_ms;
        
        /* Performance metrics */
        float power_reduction_percent;
        float performance_impact_percent;
        uint64_t successful_predictions;
        uint64_t total_predictions;
        
    } ai_optimization;
    
    /* Runtime power management */
    struct {
        bool enabled;
        uint32_t autosuspend_delay_ms;
        uint32_t active_devices;
        struct workqueue_struct *pm_workqueue;
    } runtime_pm;
    
    /* Statistics */
    struct {
        uint64_t suspend_count;
        uint64_t resume_count;
        uint64_t hibernate_count;
        uint64_t total_suspend_time_ms;
        uint64_t total_hibernate_time_ms;
        uint32_t failed_suspends;
        uint32_t failed_resumes;
        float average_power_mw;
        uint64_t total_energy_consumed_mwh;
    } stats;
    
    /* Configuration */
    struct {
        bool debug_enabled;
        uint32_t pm_trace_level;
        bool wakeup_irq_check;
        bool console_suspend_enabled;
    } config;
    
} power_management_t;

/* Power device operations */
struct power_supply_ops {
    int (*get_property)(struct power_supply *psy, int property, void *value);
    int (*set_property)(struct power_supply *psy, int property, const void *value);
    int (*external_power_changed)(struct power_supply *psy);
};

struct thermal_zone_ops {
    int (*get_temp)(struct thermal_zone *tz, int *temp);
    int (*get_mode)(struct thermal_zone *tz, int *mode);
    int (*set_mode)(struct thermal_zone *tz, int mode);
    int (*get_trip_type)(struct thermal_zone *tz, int trip, int *type);
    int (*get_trip_temp)(struct thermal_zone *tz, int trip, int *temp);
    int (*set_trip_temp)(struct thermal_zone *tz, int trip, int temp);
    int (*get_crit_temp)(struct thermal_zone *tz, int *temp);
    int (*notify)(struct thermal_zone *tz, int trip, int type);
};

struct cooling_device_ops {
    int (*get_max_state)(struct cooling_device *cdev, unsigned long *state);
    int (*get_cur_state)(struct cooling_device *cdev, unsigned long *state);
    int (*set_cur_state)(struct cooling_device *cdev, unsigned long state);
    int (*get_requested_power)(struct cooling_device *cdev, struct thermal_zone *tz, u32 *power);
    int (*state2power)(struct cooling_device *cdev, struct thermal_zone *tz, unsigned long state, u32 *power);
    int (*power2state)(struct cooling_device *cdev, struct thermal_zone *tz, u32 power, unsigned long *state);
};

/* External power management */
extern power_management_t power_mgmt;

/* Core power management functions */
int power_init(void);
void power_exit(void);

/* System power state management */
int power_suspend_system(system_power_state_t target_state);
int power_resume_system(void);
int power_hibernate_system(void);
int power_shutdown_system(void);
int power_reboot_system(void);
bool power_can_suspend(void);
bool power_can_hibernate(void);

/* CPU power management */
int power_register_cpu_domain(struct cpu_power_domain *domain);
void power_unregister_cpu_domain(struct cpu_power_domain *domain);
int power_set_cpu_governor(uint32_t domain_id, cpu_governor_t governor);
int power_set_cpu_frequency(uint32_t domain_id, uint32_t freq_khz);
int power_get_cpu_frequency(uint32_t domain_id, uint32_t *freq_khz);
int power_cpu_idle_enter(uint32_t cpu_id, cpu_power_state_t state);
void power_cpu_idle_exit(uint32_t cpu_id);

/* Power supply management */
int power_register_supply(struct power_supply *supply);
void power_unregister_supply(struct power_supply *supply);
struct power_supply *power_get_main_battery(void);
struct power_supply *power_get_ac_adapter(void);
int power_get_battery_capacity(uint32_t *capacity_percent);
int power_get_battery_time_remaining(uint32_t *minutes);
bool power_is_charging(void);
bool power_is_on_battery(void);

/* Thermal management */
int power_register_thermal_zone(struct thermal_zone *zone);
void power_unregister_thermal_zone(struct thermal_zone *zone);
int power_register_cooling_device(struct cooling_device *cdev);
void power_unregister_cooling_device(struct cooling_device *cdev);
int power_get_thermal_temperature(uint32_t zone_id, int32_t *temp_millicelsius);
int power_set_cooling_state(uint32_t device_id, uint32_t state);
bool power_is_thermal_throttling(void);

/* Wake source management */
int power_register_wake_source(struct wake_source *ws);
void power_unregister_wake_source(struct wake_source *ws);
void power_wake_source_activate(struct wake_source *ws);
void power_wake_source_deactivate(struct wake_source *ws);
bool power_wake_source_is_active(struct wake_source *ws);
void power_enable_wake_source(struct wake_source *ws, bool enable);

/* Power policy management */
int power_set_policy(const struct power_policy *policy);
int power_get_policy(struct power_policy *policy);
int power_load_policy_from_file(const char *filename);
int power_save_policy_to_file(const char *filename);
void power_apply_battery_policy(void);
void power_apply_ac_policy(void);

/* Runtime power management */
int power_pm_runtime_enable(struct device *dev);
void power_pm_runtime_disable(struct device *dev);
int power_pm_runtime_get(struct device *dev);
void power_pm_runtime_put(struct device *dev);
int power_pm_runtime_suspend(struct device *dev);
int power_pm_runtime_resume(struct device *dev);
void power_pm_runtime_set_autosuspend_delay(struct device *dev, int delay_ms);

/* AI power optimization */
int power_ai_init(void);
void power_ai_cleanup(void);
int power_ai_enable(bool enable);
int power_ai_train_model(const float *input_data, const float *target_data, uint32_t samples);
float power_ai_predict_consumption(const float *system_state);
int power_ai_optimize_policy(struct power_policy *policy);
void power_ai_update_feedback(float actual_power, float predicted_power);

/* Configuration and tuning */
int power_set_global_config(const struct power_management *config);
int power_get_global_config(struct power_management *config);
void power_enable_debug(bool enable);
void power_set_trace_level(uint32_t level);

/* Statistics and monitoring */
void power_print_statistics(void);
void power_print_cpu_domain_info(uint32_t domain_id);
void power_print_thermal_info(void);
void power_print_battery_info(void);
int power_get_system_power_consumption(uint32_t *power_mw);
int power_get_performance_stats(void *stats, size_t size);

/* Utility functions */
const char *power_state_name(system_power_state_t state);
const char *device_power_state_name(device_power_state_t state);
const char *cpu_power_state_name(cpu_power_state_t state);
const char *cpu_governor_name(cpu_governor_t governor);
const char *power_supply_type_name(power_supply_type_t type);
const char *battery_health_name(battery_health_t health);
uint64_t power_get_uptime_ms(void);
uint32_t power_calculate_remaining_time(uint32_t capacity_percent, int32_t current_ua);

/* Device power management helpers */
int power_device_suspend(struct device *dev, device_power_state_t state);
int power_device_resume(struct device *dev);
bool power_device_may_wakeup(struct device *dev);
void power_device_set_wakeup_capable(struct device *dev, bool capable);
void power_device_wakeup_enable(struct device *dev, bool enable);

/* Advanced features */
int power_enable_fast_suspend(bool enable);
int power_enable_hybrid_sleep(bool enable);
int power_configure_suspend_blockers(bool enable);
int power_register_suspend_notifier(struct notifier_block *nb);
int power_unregister_suspend_notifier(struct notifier_block *nb);