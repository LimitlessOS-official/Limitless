/*
 * LimitlessOS Advanced Power Management Header
 * Enterprise-grade power management with CPU frequency scaling,
 * device power states, thermal management, and enterprise policies
 */

#ifndef ADVANCED_POWER_H
#define ADVANCED_POWER_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * POWER MANAGEMENT CONSTANTS
 * ============================================================================ */

#define POWER_MAX_CPU_CORES         256     /* Maximum CPU cores supported */
#define POWER_MAX_DEVICES           512     /* Maximum power-managed devices */
#define POWER_MAX_POLICIES          64      /* Maximum power policies */
#define POWER_MAX_THERMAL_ZONES     32      /* Maximum thermal zones */
#define POWER_MAX_GOVERNORS         16      /* Maximum CPU governors */
#define POWER_MAX_FREQ_DOMAINS      32      /* Maximum frequency domains */
#define POWER_MAX_VOLTAGE_DOMAINS   16      /* Maximum voltage domains */

/* Power states */
#define POWER_STATE_S0              0       /* Working state */
#define POWER_STATE_S1              1       /* Sleep with CPU context retained */
#define POWER_STATE_S2              2       /* Sleep with CPU powered off */
#define POWER_STATE_S3              3       /* Suspend to RAM */
#define POWER_STATE_S4              4       /* Suspend to disk (hibernation) */
#define POWER_STATE_S5              5       /* Soft power off */

/* Device power states (D-states) */
#define DEVICE_POWER_D0             0       /* Fully on */
#define DEVICE_POWER_D1             1       /* Intermediate sleep state */
#define DEVICE_POWER_D2             2       /* Deeper sleep state */
#define DEVICE_POWER_D3_HOT         3       /* Sleep state, can wake system */
#define DEVICE_POWER_D3_COLD        4       /* Off state, requires full reinit */

/* CPU power states (C-states) */
#define CPU_CSTATE_C0               0       /* Operating state */
#define CPU_CSTATE_C1               1       /* Halt */
#define CPU_CSTATE_C1E              2       /* Enhanced halt */
#define CPU_CSTATE_C2               3       /* Stop grant */
#define CPU_CSTATE_C3               4       /* Sleep */
#define CPU_CSTATE_C6               6       /* Deep sleep */
#define CPU_CSTATE_C7               7       /* Deeper sleep */
#define CPU_CSTATE_C8               8       /* Deepest sleep */

/* Performance states (P-states) */
#define CPU_PSTATE_P0               0       /* Maximum performance */
#define CPU_PSTATE_P1               1       /* High performance */
#define CPU_PSTATE_P2               2       /* Medium performance */
#define CPU_PSTATE_P3               3       /* Low performance */
#define CPU_PSTATE_PN               15      /* Minimum performance */

/* Thermal trip points */
#define THERMAL_TRIP_ACTIVE         0       /* Active cooling */
#define THERMAL_TRIP_PASSIVE        1       /* Passive cooling */
#define THERMAL_TRIP_HOT            2       /* Hot threshold */
#define THERMAL_TRIP_CRITICAL       3       /* Critical shutdown */

/* Power policy types */
#define POLICY_TYPE_PERFORMANCE     0       /* Maximum performance */
#define POLICY_TYPE_BALANCED        1       /* Balanced performance/power */
#define POLICY_TYPE_POWER_SAVER     2       /* Maximum power savings */
#define POLICY_TYPE_ENTERPRISE      3       /* Enterprise-specific policy */
#define POLICY_TYPE_CUSTOM          4       /* Custom user-defined policy */

/* ============================================================================
 * POWER MANAGEMENT STRUCTURES
 * ============================================================================ */

/* CPU frequency information */
typedef struct cpu_freq_info {
    uint32_t cpu_id;                /* CPU identifier */
    uint32_t domain_id;             /* Frequency domain ID */
    
    /* Current state */
    uint64_t current_freq;          /* Current frequency in Hz */
    uint32_t current_pstate;        /* Current P-state */
    uint32_t current_cstate;        /* Current C-state */
    uint64_t current_voltage;       /* Current voltage in µV */
    
    /* Supported frequencies */
    uint64_t* supported_freqs;      /* Array of supported frequencies */
    uint32_t freq_count;            /* Number of supported frequencies */
    uint64_t min_freq;              /* Minimum frequency */
    uint64_t max_freq;              /* Maximum frequency */
    
    /* Supported P-states */
    uint32_t* supported_pstates;    /* Array of supported P-states */
    uint32_t pstate_count;          /* Number of P-states */
    
    /* Governor settings */
    char governor[32];              /* Current governor name */
    uint32_t target_load;           /* Target CPU load (%) */
    uint64_t sampling_rate;         /* Sampling rate in µs */
    uint64_t up_threshold;          /* Up scaling threshold */
    uint64_t down_threshold;        /* Down scaling threshold */
    
    /* Statistics */
    uint64_t freq_transitions;      /* Number of frequency transitions */
    uint64_t total_time_ms;         /* Total time in all states */
    uint64_t* time_in_state;        /* Time spent in each frequency state */
    uint64_t last_transition_time;  /* Last transition timestamp */
} cpu_freq_info_t;

/* Device power information */
typedef struct device_power_info {
    uint32_t device_id;             /* Device identifier */
    char device_name[64];           /* Device name */
    uint32_t device_class;          /* Device class */
    
    /* Current state */
    uint32_t current_dstate;        /* Current D-state */
    uint64_t current_power_mw;      /* Current power consumption in mW */
    bool wake_enabled;              /* Wake-up enabled */
    bool runtime_pm_enabled;        /* Runtime power management enabled */
    
    /* Supported states */
    uint32_t supported_dstates;     /* Bitmask of supported D-states */
    uint64_t* state_power_mw;       /* Power consumption per D-state */
    uint64_t* transition_time_us;   /* Transition time per D-state */
    
    /* Wake capabilities */
    uint32_t wake_events;           /* Supported wake events bitmask */
    bool system_wake_capable;       /* Can wake system from sleep */
    
    /* Power constraints */
    uint64_t max_power_budget_mw;   /* Maximum power budget */
    uint64_t thermal_budget_mw;     /* Thermal power budget */
    
    /* Statistics */
    uint64_t state_transitions;     /* Number of state transitions */
    uint64_t* time_in_dstate;       /* Time spent in each D-state */
    uint64_t total_energy_consumed; /* Total energy consumed (µWh) */
    uint64_t runtime_active_time;   /* Runtime PM active time */
    uint64_t runtime_suspended_time; /* Runtime PM suspended time */
    
    /* Device-specific operations */
    int (*set_power_state)(struct device_power_info* dev, uint32_t dstate);
    int (*get_power_consumption)(struct device_power_info* dev, uint64_t* power_mw);
    int (*configure_wake)(struct device_power_info* dev, bool enable);
    
    struct device_power_info* next;
} device_power_info_t;

/* Thermal zone information */
typedef struct thermal_zone {
    uint32_t zone_id;               /* Thermal zone identifier */
    char zone_name[32];             /* Zone name */
    char sensor_type[32];           /* Sensor type */
    
    /* Current state */
    int32_t current_temp;           /* Current temperature in milli-Celsius */
    uint32_t current_trip;          /* Current trip point state */
    bool throttling_active;         /* Throttling currently active */
    
    /* Trip points */
    int32_t* trip_temps;            /* Trip point temperatures */
    uint32_t* trip_types;           /* Trip point types */
    uint32_t trip_count;            /* Number of trip points */
    
    /* Cooling devices */
    uint32_t* cooling_devices;      /* Associated cooling device IDs */
    uint32_t cooling_device_count;  /* Number of cooling devices */
    
    /* Thresholds and limits */
    int32_t critical_temp;          /* Critical temperature */
    int32_t hot_temp;               /* Hot temperature */
    int32_t passive_temp;           /* Passive cooling temperature */
    uint32_t polling_delay_ms;      /* Polling delay */
    
    /* Statistics */
    int32_t min_temp;               /* Minimum recorded temperature */
    int32_t max_temp;               /* Maximum recorded temperature */
    uint64_t trip_violations;       /* Number of trip point violations */
    uint64_t throttling_events;     /* Number of throttling events */
    uint64_t total_throttling_time; /* Total time spent throttling */
    
    /* Operations */
    int (*read_temperature)(struct thermal_zone* zone, int32_t* temp);
    int (*set_trip_point)(struct thermal_zone* zone, uint32_t trip, int32_t temp);
    int (*enable_trip)(struct thermal_zone* zone, uint32_t trip, bool enable);
    
    struct thermal_zone* next;
} thermal_zone_t;

/* Power policy */
typedef struct power_policy {
    uint32_t policy_id;             /* Policy identifier */
    char policy_name[64];           /* Policy name */
    uint32_t policy_type;           /* Policy type */
    
    /* CPU settings */
    char cpu_governor[32];          /* CPU frequency governor */
    uint32_t cpu_min_freq_percent;  /* Minimum CPU frequency (% of max) */
    uint32_t cpu_max_freq_percent;  /* Maximum CPU frequency (% of max) */
    uint32_t cpu_target_load;       /* Target CPU load */
    
    /* Device settings */
    uint32_t disk_timeout_ms;       /* Disk idle timeout */
    uint32_t display_timeout_ms;    /* Display timeout */
    uint32_t usb_autosuspend_ms;    /* USB autosuspend timeout */
    bool pci_aspm_enabled;          /* PCIe ASPM enabled */
    
    /* System settings */
    uint32_t sleep_timeout_ms;      /* System sleep timeout */
    bool hybrid_sleep_enabled;      /* Hybrid sleep enabled */
    bool fast_startup_enabled;      /* Fast startup enabled */
    
    /* Thermal settings */
    int32_t thermal_throttle_temp;  /* Temperature to start throttling */
    uint32_t thermal_throttle_percent; /* Throttling percentage */
    bool aggressive_cooling;        /* Aggressive cooling enabled */
    
    /* Enterprise settings */
    uint32_t tenant_id;             /* Tenant identifier */
    uint32_t priority;              /* Policy priority */
    bool override_user_settings;    /* Override user power settings */
    bool audit_events;              /* Audit power events */
    
    /* Power budgets */
    uint64_t total_power_budget_mw; /* Total system power budget */
    uint64_t cpu_power_budget_mw;   /* CPU power budget */
    uint64_t gpu_power_budget_mw;   /* GPU power budget */
    uint64_t memory_power_budget_mw; /* Memory power budget */
    
    struct power_policy* next;
} power_policy_t;

/* CPU governor */
typedef struct cpu_governor {
    char name[32];                  /* Governor name */
    char description[128];          /* Governor description */
    
    /* Governor parameters */
    uint64_t sampling_rate_us;      /* Sampling rate in microseconds */
    uint32_t up_threshold;          /* Up scaling threshold (%) */
    uint32_t down_threshold;        /* Down scaling threshold (%) */
    bool ignore_nice;               /* Ignore nice processes */
    
    /* Governor operations */
    int (*init)(struct cpu_governor* gov);
    int (*start)(struct cpu_governor* gov, uint32_t cpu_id);
    int (*stop)(struct cpu_governor* gov, uint32_t cpu_id);
    int (*update)(struct cpu_governor* gov, uint32_t cpu_id, uint32_t load);
    void (*cleanup)(struct cpu_governor* gov);
    
    /* Statistics */
    uint64_t decisions_made;        /* Number of scaling decisions */
    uint64_t frequency_changes;     /* Number of frequency changes */
    uint64_t average_load;          /* Average CPU load */
    
    struct cpu_governor* next;
} cpu_governor_t;

/* Power event */
typedef struct power_event {
    uint64_t timestamp;             /* Event timestamp */
    uint32_t event_type;            /* Event type */
    uint32_t source_id;             /* Source device/CPU ID */
    char description[128];          /* Event description */
    
    /* Event data */
    union {
        struct {
            uint32_t old_state;     /* Old power state */
            uint32_t new_state;     /* New power state */
        } state_change;
        
        struct {
            uint64_t old_freq;      /* Old frequency */
            uint64_t new_freq;      /* New frequency */
        } freq_change;
        
        struct {
            int32_t temperature;    /* Temperature in milli-Celsius */
            uint32_t trip_point;    /* Trip point triggered */
        } thermal;
        
        struct {
            uint64_t power_mw;      /* Power consumption in mW */
            uint32_t budget_exceeded; /* Budget exceeded flag */
        } power;
    } data;
    
    /* Enterprise audit */
    uint32_t tenant_id;             /* Tenant ID */
    uint32_t user_id;               /* User ID */
    bool security_relevant;         /* Security-relevant event */
    
    struct power_event* next;
} power_event_t;

/* ============================================================================
 * POWER MANAGEMENT API
 * ============================================================================ */

/* Initialization and configuration */
int advanced_power_init(void);
void power_enable_enterprise_mode(bool enable);
void power_print_statistics(void);
int power_self_test(void);
void power_shutdown(void);

/* CPU frequency management */
int power_cpu_set_governor(uint32_t cpu_id, const char* governor_name);
int power_cpu_set_frequency(uint32_t cpu_id, uint64_t frequency);
int power_cpu_set_pstate(uint32_t cpu_id, uint32_t pstate);
int power_cpu_get_frequency(uint32_t cpu_id, uint64_t* frequency);
int power_cpu_get_load(uint32_t cpu_id, uint32_t* load_percent);

/* Device power management */
int power_device_register(device_power_info_t* device);
int power_device_unregister(uint32_t device_id);
int power_device_set_state(uint32_t device_id, uint32_t dstate);
int power_device_get_consumption(uint32_t device_id, uint64_t* power_mw);
int power_device_configure_wake(uint32_t device_id, bool enable);

/* Thermal management */
int power_thermal_register_zone(thermal_zone_t* zone);
int power_thermal_get_temperature(uint32_t zone_id, int32_t* temperature);
int power_thermal_set_trip_point(uint32_t zone_id, uint32_t trip, int32_t temp);
int power_thermal_check_violations(void);

/* Power policy management */
int power_policy_create(const char* name, uint32_t type, power_policy_t** policy);
int power_policy_apply(uint32_t policy_id);
int power_policy_set_cpu_settings(uint32_t policy_id, const char* governor,
                                 uint32_t min_freq_percent, uint32_t max_freq_percent);
int power_policy_set_thermal_settings(uint32_t policy_id, int32_t throttle_temp,
                                      uint32_t throttle_percent);

/* System power management */
int power_system_suspend(uint32_t state);
int power_system_hibernate(void);
int power_system_shutdown(void);
int power_system_reboot(void);

/* Enterprise features */
int power_set_tenant_policy(uint32_t tenant_id, uint32_t policy_id);
int power_audit_events(bool enable);
int power_get_tenant_consumption(uint32_t tenant_id, uint64_t* power_mw);

/* Statistics and monitoring */
int power_get_system_consumption(uint64_t* total_power_mw);
int power_get_cpu_statistics(uint32_t cpu_id, cpu_freq_info_t* stats);
int power_get_device_statistics(uint32_t device_id, device_power_info_t* stats);
int power_get_thermal_statistics(uint32_t zone_id, thermal_zone_t* stats);

#endif /* ADVANCED_POWER_H */