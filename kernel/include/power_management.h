/**
 * Advanced Power Management Framework for LimitlessOS
 * Enterprise-grade ACPI support with dynamic frequency scaling and thermal management
 */

#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include "kernel.h"

/* Power management states */
typedef enum {
    POWER_STATE_S0 = 0,        /* Working state */
    POWER_STATE_S1,            /* CPU and RAM powered, other devices may power down */
    POWER_STATE_S2,            /* CPU powered off, RAM powered, wake up slower than S1 */
    POWER_STATE_S3,            /* Suspend to RAM, only RAM powered */
    POWER_STATE_S4,            /* Suspend to disk, hibernate */
    POWER_STATE_S5,            /* Soft power off */
    POWER_STATE_G3,            /* Mechanical power off */
    POWER_STATE_COUNT
} power_state_t;

/* CPU power states (C-states) */
typedef enum {
    CPU_STATE_C0 = 0,          /* Active state */
    CPU_STATE_C1,              /* Halt state */
    CPU_STATE_C2,              /* Stop-clock state */
    CPU_STATE_C3,              /* Sleep state */
    CPU_STATE_C6,              /* Deep power down */
    CPU_STATE_C7,              /* Deeper sleep */
    CPU_STATE_C8,              /* Deepest sleep */
    CPU_STATE_COUNT
} cpu_power_state_t;

/* Performance states (P-states) */
typedef enum {
    PERF_STATE_P0 = 0,         /* Maximum performance */
    PERF_STATE_P1,             /* High performance */
    PERF_STATE_P2,             /* Medium performance */
    PERF_STATE_P3,             /* Low performance */
    PERF_STATE_P4,             /* Minimum performance */
    PERF_STATE_COUNT
} performance_state_t;

/* Throttling states (T-states) */
typedef enum {
    THROTTLE_STATE_T0 = 0,     /* No throttling */
    THROTTLE_STATE_T1,         /* 12.5% duty cycle */
    THROTTLE_STATE_T2,         /* 25% duty cycle */
    THROTTLE_STATE_T3,         /* 37.5% duty cycle */
    THROTTLE_STATE_T4,         /* 50% duty cycle */
    THROTTLE_STATE_T5,         /* 62.5% duty cycle */
    THROTTLE_STATE_T6,         /* 75% duty cycle */
    THROTTLE_STATE_T7,         /* 87.5% duty cycle */
    THROTTLE_STATE_COUNT
} throttle_state_t;

/* Power management policies */
typedef enum {
    POWER_POLICY_PERFORMANCE = 0,      /* Maximum performance, ignore power */
    POWER_POLICY_BALANCED,             /* Balance performance and power */
    POWER_POLICY_POWER_SAVER,          /* Minimize power consumption */
    POWER_POLICY_ENTERPRISE,           /* Enterprise-optimized policy */
    POWER_POLICY_REALTIME,             /* Real-time optimized, minimal latency */
    POWER_POLICY_CUSTOM,               /* User-defined policy */
    POWER_POLICY_COUNT
} power_policy_t;

/* Thermal management zones */
typedef enum {
    THERMAL_ZONE_CPU = 0,
    THERMAL_ZONE_GPU,
    THERMAL_ZONE_CHIPSET,
    THERMAL_ZONE_MEMORY,
    THERMAL_ZONE_STORAGE,
    THERMAL_ZONE_SYSTEM,
    THERMAL_ZONE_COUNT
} thermal_zone_t;

/* Thermal trip points */
typedef enum {
    THERMAL_TRIP_ACTIVE = 0,           /* Fan speed increase */
    THERMAL_TRIP_PASSIVE,              /* Throttling begins */
    THERMAL_TRIP_HOT,                  /* Emergency throttling */
    THERMAL_TRIP_CRITICAL,             /* System shutdown */
    THERMAL_TRIP_COUNT
} thermal_trip_type_t;

/* ACPI device power states */
typedef enum {
    DEVICE_STATE_D0 = 0,               /* Fully on */
    DEVICE_STATE_D1,                   /* Intermediate state */
    DEVICE_STATE_D2,                   /* Intermediate state */
    DEVICE_STATE_D3,                   /* Off, but can wake system */
    DEVICE_STATE_COUNT
} device_power_state_t;

/* Power source types */
typedef enum {
    POWER_SOURCE_AC = 0,               /* AC power adapter */
    POWER_SOURCE_BATTERY,              /* Battery power */
    POWER_SOURCE_UPS,                  /* Uninterruptible power supply */
    POWER_SOURCE_UNKNOWN,              /* Unknown power source */
    POWER_SOURCE_COUNT
} power_source_t;

/* Battery information */
typedef struct {
    bool present;
    power_source_t type;
    uint32_t capacity_mwh;             /* Design capacity in mWh */
    uint32_t remaining_mwh;            /* Remaining capacity in mWh */
    uint32_t voltage_mv;               /* Current voltage in mV */
    int32_t current_ma;                /* Current flow in mA (+ charging, - discharging) */
    uint32_t charge_rate_mw;           /* Charging rate in mW */
    uint32_t discharge_rate_mw;        /* Discharge rate in mW */
    uint32_t cycle_count;              /* Battery cycle count */
    uint32_t temperature_celsius;      /* Battery temperature */
    uint32_t time_remaining_minutes;   /* Estimated time remaining */
    bool charging;
    bool critical_low;
    
    /* Battery health */
    uint32_t wear_level_percent;       /* 0-100, 0 = new, 100 = worn out */
    uint32_t health_percent;           /* Overall health percentage */
    
    char manufacturer[32];
    char model[32];
    char serial_number[32];
} battery_info_t;

/* CPU frequency information */
typedef struct {
    uint32_t cpu_id;
    uint32_t current_frequency_mhz;
    uint32_t max_frequency_mhz;
    uint32_t min_frequency_mhz;
    uint32_t base_frequency_mhz;
    
    performance_state_t current_pstate;
    cpu_power_state_t current_cstate;
    throttle_state_t current_tstate;
    
    /* P-state table */
    struct {
        uint32_t frequency_mhz;
        uint32_t voltage_mv;
        uint32_t power_mw;
        uint32_t latency_us;
    } pstates[PERF_STATE_COUNT];
    
    /* C-state table */
    struct {
        uint32_t latency_us;
        uint32_t power_mw;
        uint32_t residency_us;
        bool supported;
    } cstates[CPU_STATE_COUNT];
    
    /* Statistics */
    uint64_t frequency_transitions;
    uint64_t cstate_transitions;
    uint64_t time_in_cstates[CPU_STATE_COUNT];
    uint64_t time_in_pstates[PERF_STATE_COUNT];
    
    spinlock_t lock;
} cpu_frequency_info_t;

/* Thermal sensor information */
typedef struct {
    uint32_t sensor_id;
    thermal_zone_t zone;
    char name[64];
    
    int32_t temperature_celsius;
    int32_t critical_temp_celsius;
    int32_t passive_temp_celsius;
    int32_t active_temp_celsius[4];     /* Multiple active cooling levels */
    
    bool overheated;
    bool critical;
    uint32_t throttling_level;          /* 0-100% throttling */
    
    /* Trip points */
    struct {
        thermal_trip_type_t type;
        int32_t temperature_celsius;
        bool enabled;
        uint32_t action_id;
    } trip_points[THERMAL_TRIP_COUNT];
    
    /* Statistics */
    int32_t max_temperature_celsius;
    int32_t min_temperature_celsius;
    uint64_t overheat_events;
    uint64_t throttle_events;
    uint64_t last_update_ns;
    
    spinlock_t lock;
} thermal_sensor_t;

/* Power device information */
typedef struct {
    uint32_t device_id;
    char name[64];
    device_power_state_t current_state;
    device_power_state_t supported_states[DEVICE_STATE_COUNT];
    
    uint32_t power_consumption_mw[DEVICE_STATE_COUNT];
    uint32_t wakeup_latency_us[DEVICE_STATE_COUNT];
    
    bool can_wake_system;
    bool runtime_pm_enabled;
    uint64_t runtime_suspend_count;
    uint64_t runtime_resume_count;
    uint64_t total_suspend_time_ns;
    
    /* Power management callbacks */
    status_t (*suspend)(void* context);
    status_t (*resume)(void* context);
    void* context;
    
    struct power_device* next;
    spinlock_t lock;
} power_device_t;

/* Power policy configuration */
typedef struct {
    power_policy_t policy;
    
    /* CPU frequency scaling */
    uint32_t min_cpu_frequency_percent;    /* 0-100% */
    uint32_t max_cpu_frequency_percent;    /* 0-100% */
    uint32_t cpu_frequency_step_percent;   /* Frequency change step */
    uint32_t frequency_change_latency_us;  /* Max latency for frequency change */
    
    /* Power state transitions */
    uint32_t idle_threshold_ms;            /* Time before entering idle state */
    uint32_t sleep_threshold_ms;           /* Time before entering sleep */
    uint32_t hibernate_threshold_ms;       /* Time before hibernation */
    
    /* Thermal management */
    uint32_t thermal_polling_interval_ms;
    uint32_t passive_cooling_threshold_celsius;
    uint32_t active_cooling_threshold_celsius;
    uint32_t critical_shutdown_threshold_celsius;
    
    /* Device power management */
    bool runtime_pm_enabled;
    uint32_t device_idle_timeout_ms;
    bool aggressive_link_power_management;
    
    /* Battery management */
    uint32_t low_battery_threshold_percent;
    uint32_t critical_battery_threshold_percent;
    bool battery_charge_optimization;
    uint32_t charge_limit_percent;         /* Limit charging to this level */
    
    /* Enterprise features */
    bool wake_on_lan_enabled;
    bool scheduled_wakeup_enabled;
    bool power_capping_enabled;
    uint32_t power_cap_watts;              /* System power cap */
    
    spinlock_t lock;
} power_policy_config_t;

/* Power management statistics */
typedef struct {
    /* System power states */
    uint64_t time_in_states[POWER_STATE_COUNT];
    uint64_t state_transitions[POWER_STATE_COUNT][POWER_STATE_COUNT];
    uint64_t wakeup_events;
    uint64_t failed_transitions;
    
    /* CPU power management */
    uint64_t frequency_changes;
    uint64_t throttling_events;
    uint64_t thermal_events;
    
    /* Device power management */
    uint64_t device_suspends;
    uint64_t device_resumes;
    uint64_t runtime_pm_events;
    
    /* Power consumption */
    uint32_t current_power_consumption_mw;
    uint32_t average_power_consumption_mw;
    uint32_t peak_power_consumption_mw;
    uint64_t total_energy_consumed_mwh;
    
    /* Thermal statistics */
    uint32_t max_system_temperature_celsius;
    uint64_t cooling_activations;
    uint64_t thermal_shutdowns;
    
    /* Battery statistics */
    uint32_t battery_cycles_completed;
    uint64_t time_on_battery_ns;
    uint64_t time_on_ac_ns;
    
    /* Performance impact */
    uint32_t performance_impact_percent;   /* 0-100% */
    uint64_t power_savings_mwh;           /* Estimated power savings */
} power_management_stats_t;

/* Main power management state */
typedef struct {
    bool initialized;
    bool enabled;
    bool acpi_available;
    
    /* Current system state */
    power_state_t current_power_state;
    power_source_t current_power_source;
    power_policy_t current_policy;
    
    /* ACPI integration */
    bool acpi_s3_supported;                /* Suspend to RAM */
    bool acpi_s4_supported;                /* Hibernate */
    bool acpi_thermal_supported;           /* Thermal management */
    bool acpi_cpufreq_supported;           /* CPU frequency scaling */
    
    /* Hardware components */
    cpu_frequency_info_t cpu_info[MAX_CPUS];
    uint32_t cpu_count;
    
    thermal_sensor_t thermal_sensors[16];
    uint32_t thermal_sensor_count;
    
    power_device_t* device_list;
    uint32_t device_count;
    
    battery_info_t battery_info[4];        /* Support up to 4 batteries */
    uint32_t battery_count;
    
    /* Configuration */
    power_policy_config_t policy_config;
    
    /* Statistics and monitoring */
    power_management_stats_t stats;
    uint64_t monitoring_interval_ns;
    uint64_t last_monitoring_update_ns;
    
    /* Worker threads */
    bool monitoring_thread_running;
    bool thermal_thread_running;
    bool frequency_scaling_thread_running;
    
    /* Events */
    uint32_t power_button_pressed;
    uint32_t sleep_button_pressed;
    uint32_t lid_closed;
    uint32_t ac_adapter_connected;
    uint32_t battery_low;
    uint32_t thermal_alert;
    
    spinlock_t global_lock;
    spinlock_t policy_lock;
    spinlock_t thermal_lock;
    spinlock_t frequency_lock;
} power_management_t;

/* Function prototypes */

/* Initialization and configuration */
status_t power_management_init(void);
status_t power_management_enable(bool enable);
status_t power_management_set_policy(power_policy_t policy);
status_t power_management_configure_policy(const power_policy_config_t* config);

/* System power state management */
status_t power_management_suspend_to_ram(void);
status_t power_management_hibernate(void);
status_t power_management_shutdown(void);
status_t power_management_reboot(void);
status_t power_management_set_power_state(power_state_t state);
status_t power_management_get_power_state(power_state_t* state);

/* CPU frequency and power state management */
status_t power_management_set_cpu_frequency(uint32_t cpu_id, uint32_t frequency_mhz);
status_t power_management_set_cpu_pstate(uint32_t cpu_id, performance_state_t pstate);
status_t power_management_set_cpu_cstate(uint32_t cpu_id, cpu_power_state_t cstate);
status_t power_management_enable_cpu_boost(uint32_t cpu_id, bool enable);
status_t power_management_get_cpu_info(uint32_t cpu_id, cpu_frequency_info_t* info);

/* Thermal management */
status_t power_management_register_thermal_sensor(thermal_zone_t zone, const char* name, uint32_t* sensor_id);
status_t power_management_update_temperature(uint32_t sensor_id, int32_t temperature_celsius);
status_t power_management_set_thermal_trip_point(uint32_t sensor_id, thermal_trip_type_t type, 
                                               int32_t temperature_celsius);
status_t power_management_get_thermal_info(uint32_t sensor_id, thermal_sensor_t* info);
status_t power_management_activate_cooling(thermal_zone_t zone, uint32_t level);

/* Device power management */
status_t power_management_register_device(const char* name, power_device_t* device, uint32_t* device_id);
status_t power_management_set_device_state(uint32_t device_id, device_power_state_t state);
status_t power_management_enable_runtime_pm(uint32_t device_id, bool enable);
status_t power_management_runtime_suspend_device(uint32_t device_id);
status_t power_management_runtime_resume_device(uint32_t device_id);

/* Battery management */
status_t power_management_update_battery_info(uint32_t battery_id, const battery_info_t* info);
status_t power_management_get_battery_info(uint32_t battery_id, battery_info_t* info);
status_t power_management_set_charge_limit(uint32_t battery_id, uint32_t limit_percent);
status_t power_management_calibrate_battery(uint32_t battery_id);

/* Power monitoring and statistics */
status_t power_management_get_stats(power_management_stats_t* stats);
status_t power_management_reset_stats(void);
status_t power_management_get_power_consumption(uint32_t* consumption_mw);
status_t power_management_estimate_battery_life(uint32_t* minutes_remaining);

/* ACPI integration */
status_t power_management_acpi_init(void);
status_t power_management_acpi_evaluate_method(const char* method, uint32_t* result);
status_t power_management_acpi_set_wakeup_device(const char* device, bool enable);
status_t power_management_handle_acpi_event(uint32_t event_type, uint32_t event_data);

/* Advanced features */
status_t power_management_enable_power_capping(uint32_t power_limit_watts);
status_t power_management_schedule_wakeup(uint64_t wakeup_time_ns);
status_t power_management_set_performance_boost(bool enable);
status_t power_management_optimize_for_workload(const char* workload_type);

/* Utility functions */
const char* power_management_state_name(power_state_t state);
const char* power_management_policy_name(power_policy_t policy);
const char* power_management_thermal_zone_name(thermal_zone_t zone);
uint32_t power_management_calculate_power_savings(void);

/* Configuration constants */
#define MAX_CPUS                        256
#define MAX_THERMAL_SENSORS             32
#define MAX_POWER_DEVICES               1024
#define MAX_BATTERIES                   4
#define POWER_MONITORING_INTERVAL_MS    1000
#define THERMAL_POLLING_INTERVAL_MS     2000
#define FREQUENCY_SCALING_INTERVAL_MS   500
#define DEFAULT_IDLE_THRESHOLD_MS       5000
#define DEFAULT_SLEEP_THRESHOLD_MS      30000
#define DEFAULT_HIBERNATE_THRESHOLD_MS  1800000  /* 30 minutes */

/* Default temperature thresholds */
#define DEFAULT_PASSIVE_TEMP_CELSIUS    75
#define DEFAULT_ACTIVE_TEMP_CELSIUS     65
#define DEFAULT_CRITICAL_TEMP_CELSIUS   100
#define DEFAULT_HOT_TEMP_CELSIUS        95

/* Default power limits */
#define DEFAULT_LOW_BATTERY_PERCENT     15
#define DEFAULT_CRITICAL_BATTERY_PERCENT 5
#define DEFAULT_CHARGE_LIMIT_PERCENT    100

/* Power management events */
#define POWER_EVENT_BUTTON_PRESSED      0x01
#define POWER_EVENT_SLEEP_BUTTON        0x02
#define POWER_EVENT_LID_CLOSED          0x04
#define POWER_EVENT_AC_CONNECTED        0x08
#define POWER_EVENT_AC_DISCONNECTED     0x10
#define POWER_EVENT_BATTERY_LOW         0x20
#define POWER_EVENT_BATTERY_CRITICAL    0x40
#define POWER_EVENT_THERMAL_ALERT       0x80

/* Frequency scaling governors */
#define FREQ_GOVERNOR_PERFORMANCE       "performance"
#define FREQ_GOVERNOR_POWERSAVE         "powersave"
#define FREQ_GOVERNOR_ONDEMAND          "ondemand"
#define FREQ_GOVERNOR_CONSERVATIVE      "conservative"
#define FREQ_GOVERNOR_SCHEDUTIL         "schedutil"

#endif /* POWER_MANAGEMENT_H */