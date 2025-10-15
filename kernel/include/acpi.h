/**
 * ACPI (Advanced Configuration and Power Interface) Support for LimitlessOS
 * 
 * This module provides comprehensive ACPI support including:
 * - ACPI table parsing for hardware detection
 * - Advanced power management (sleep states, CPU P/C states)
 * - Thermal management and battery monitoring
 * - Device power management and wake events
 * - Platform-specific power optimizations
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../include/list.h"
#include "../include/mutex.h"
#include "../include/timer.h"
#include "../include/workqueue.h"

// Maximum number of CPUs supported for power management
#define MAX_CPUS                64

// Power management system calls
#define __NR_acpi_sleep         400
#define __NR_acpi_get_power_info 401
#define __NR_acpi_set_policy    402
#define __NR_acpi_get_thermal   403

/* ACPI Table Signatures */
#define ACPI_SIG_RSDP   "RSD PTR "
#define ACPI_SIG_RSDT   "RSDT"
#define ACPI_SIG_XSDT   "XSDT"
#define ACPI_SIG_FADT   "FACP"
#define ACPI_SIG_MADT   "APIC"
#define ACPI_SIG_DSDT   "DSDT"
#define ACPI_SIG_SSDT   "SSDT"
#define ACPI_SIG_HPET   "HPET"
#define ACPI_SIG_MCFG   "MCFG"

/* RSDP (Root System Description Pointer) */
typedef struct acpi_rsdp {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
    
    /* ACPI 2.0+ fields */
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) acpi_rsdp_t;

/* Generic ACPI Table Header */
typedef struct acpi_table_header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t asl_compiler_id;
    uint32_t asl_compiler_revision;
} __attribute__((packed)) acpi_table_header_t;

/* RSDT (Root System Description Table) */
typedef struct acpi_rsdt {
    acpi_table_header_t header;
    uint32_t entries[];
} __attribute__((packed)) acpi_rsdt_t;

/* XSDT (Extended System Description Table) */
typedef struct acpi_xsdt {
    acpi_table_header_t header;
    uint64_t entries[];
} __attribute__((packed)) acpi_xsdt_t;

/* MADT (Multiple APIC Description Table) */
typedef struct acpi_madt {
    acpi_table_header_t header;
    uint32_t local_apic_address;
    uint32_t flags;
    uint8_t entries[];
} __attribute__((packed)) acpi_madt_t;

/* MADT Entry Types */
#define ACPI_MADT_LAPIC         0   /* Local APIC */
#define ACPI_MADT_IOAPIC        1   /* I/O APIC */
#define ACPI_MADT_INT_SRC       2   /* Interrupt Source Override */
#define ACPI_MADT_NMI_SRC       3   /* NMI Source */
#define ACPI_MADT_LAPIC_NMI     4   /* Local APIC NMI */
#define ACPI_MADT_LAPIC_ADDR    5   /* Local APIC Address Override */
#define ACPI_MADT_IOSAPIC       6   /* I/O SAPIC */
#define ACPI_MADT_LSAPIC        7   /* Local SAPIC */
#define ACPI_MADT_PLAT_INT_SRC  8   /* Platform Interrupt Source */
#define ACPI_MADT_LX2APIC       9   /* Local x2APIC */
#define ACPI_MADT_LX2APIC_NMI   10  /* Local x2APIC NMI */

/* MADT Entry Header */
typedef struct acpi_madt_entry_header {
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) acpi_madt_entry_header_t;

/* Local APIC Entry */
typedef struct acpi_madt_lapic {
    acpi_madt_entry_header_t header;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed)) acpi_madt_lapic_t;

/* I/O APIC Entry */
typedef struct acpi_madt_ioapic {
    acpi_madt_entry_header_t header;
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t address;
    uint32_t global_irq_base;
} __attribute__((packed)) acpi_madt_ioapic_t;

/* Interrupt Source Override Entry */
typedef struct acpi_madt_int_src {
    acpi_madt_entry_header_t header;
    uint8_t bus;
    uint8_t source;
    uint32_t global_irq;
    uint16_t flags;
} __attribute__((packed)) acpi_madt_int_src_t;

/* Local x2APIC Entry */
typedef struct acpi_madt_lx2apic {
    acpi_madt_entry_header_t header;
    uint16_t reserved;
    uint32_t x2apic_id;
    uint32_t flags;
    uint32_t processor_uid;
} __attribute__((packed)) acpi_madt_lx2apic_t;

/* FADT (Fixed ACPI Description Table) */
typedef struct acpi_fadt {
    acpi_table_header_t header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t reserved1;
    uint8_t preferred_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_cnt;
    uint32_t pm1a_evt_blk;
    uint32_t pm1b_evt_blk;
    uint32_t pm1a_cnt_blk;
    uint32_t pm1b_cnt_blk;
    uint32_t pm2_cnt_blk;
    uint32_t pm_tmr_blk;
    uint32_t gpe0_blk;
    uint32_t gpe1_blk;
    uint8_t pm1_evt_len;
    uint8_t pm1_cnt_len;
    uint8_t pm2_cnt_len;
    uint8_t pm_tmr_len;
    uint8_t gpe0_blk_len;
    uint8_t gpe1_blk_len;
    uint8_t gpe1_base;
    uint8_t cst_cnt;
    uint16_t p_lvl2_lat;
    uint16_t p_lvl3_lat;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alrm;
    uint8_t mon_alrm;
    uint8_t century;
    uint16_t iapc_boot_arch;
    uint8_t reserved2;
    uint32_t flags;
    /* ... more fields for ACPI 2.0+ */
} __attribute__((packed)) acpi_fadt_t;

/* CPU Information Structure */
typedef struct acpi_cpu_info {
    uint32_t processor_id;
    uint32_t apic_id;
    uint32_t flags;
    bool enabled;
    bool online_capable;
} acpi_cpu_info_t;

/* I/O APIC Information Structure */
typedef struct acpi_ioapic_info {
    uint8_t ioapic_id;
    uint32_t address;
    uint32_t global_irq_base;
    uint32_t max_redirection_entries;
} acpi_ioapic_info_t;

/* Global ACPI State */
extern acpi_rsdp_t *acpi_rsdp;
extern acpi_rsdt_t *acpi_rsdt;
extern acpi_xsdt_t *acpi_xsdt;
extern acpi_madt_t *acpi_madt;
extern acpi_fadt_t *acpi_fadt;

extern acpi_cpu_info_t acpi_cpus[256];
extern uint32_t acpi_cpu_count;
extern acpi_ioapic_info_t acpi_ioapics[8];
extern uint32_t acpi_ioapic_count;

/* ACPI Initialization */
int acpi_init(void);
int acpi_detect_rsdp(void);
int acpi_parse_tables(void);

/* Table Management */
acpi_table_header_t *acpi_find_table(const char *signature);
bool acpi_checksum_valid(void *table, size_t length);
void acpi_dump_table_header(acpi_table_header_t *header);

/* MADT Parsing */
int acpi_parse_madt(void);
void acpi_parse_madt_entries(void);
uint32_t acpi_get_cpu_count(void);
uint32_t acpi_get_cpu_apic_id(uint32_t cpu_index);
bool acpi_cpu_enabled(uint32_t cpu_index);

/* I/O APIC Information */
uint32_t acpi_get_ioapic_count(void);
acpi_ioapic_info_t *acpi_get_ioapic_info(uint32_t ioapic_index);
uint32_t acpi_get_ioapic_address(uint32_t ioapic_id);
uint32_t acpi_get_ioapic_base_irq(uint32_t ioapic_id);

/* Power Management */
bool acpi_has_power_management(void);
void acpi_enable_power_management(void);
void acpi_disable_power_management(void);

/* Interrupt Routing */
typedef struct acpi_irq_override {
    uint8_t bus;
    uint8_t source_irq;
    uint32_t global_irq;
    uint16_t flags;
} acpi_irq_override_t;

extern acpi_irq_override_t acpi_irq_overrides[16];
extern uint32_t acpi_irq_override_count;

uint32_t acpi_get_irq_override_count(void);
acpi_irq_override_t *acpi_get_irq_override(uint32_t index);
uint32_t acpi_map_irq_to_gsi(uint8_t irq);

/* Memory Mapping */
void *acpi_map_physical(uint64_t phys_addr, size_t size);
void acpi_unmap_physical(void *virt_addr, size_t size);

/* Utility Functions */
void acpi_dump_rsdp(void);
void acpi_dump_rsdt(void);
void acpi_dump_madt(void);
void acpi_dump_cpu_info(void);
void acpi_dump_ioapic_info(void);

/* Error Codes */
#define ACPI_SUCCESS            0
#define ACPI_ERROR_NOT_FOUND    -1
#define ACPI_ERROR_INVALID      -2
#define ACPI_ERROR_CHECKSUM     -3
#define ACPI_ERROR_UNSUPPORTED  -4

// ACPI power management policies
#define ACPI_POLICY_PERFORMANCE 0  // Maximum performance
#define ACPI_POLICY_BALANCED    1  // Balanced performance/power
#define ACPI_POLICY_POWERSAVE   2  // Maximum power saving
#define ACPI_POLICY_CUSTOM      3  // User-defined policy

// CPU frequency scaling governors
#define CPUFREQ_GOV_PERFORMANCE 0  // Always maximum frequency
#define CPUFREQ_GOV_POWERSAVE   1  // Always minimum frequency
#define CPUFREQ_GOV_ONDEMAND    2  // Dynamic scaling based on load
#define CPUFREQ_GOV_CONSERVATIVE 3 // Conservative dynamic scaling
#define CPUFREQ_GOV_SCHEDUTIL   4  // Scheduler-guided scaling

// Device power states
#define ACPI_DEVICE_D0          0  // Fully powered
#define ACPI_DEVICE_D1          1  // Low power state 1
#define ACPI_DEVICE_D2          2  // Low power state 2
#define ACPI_DEVICE_D3HOT       3  // Low power state 3 (hot)
#define ACPI_DEVICE_D3COLD      4  // Low power state 3 (cold/off)

// System power information (for user space)
typedef struct {
    // ACPI version and capabilities
    uint32_t acpi_version_major;     // ACPI version major
    uint32_t acpi_version_minor;     // ACPI version minor
    bool power_management_enabled;   // Power management enabled
    
    // Sleep state support
    bool s0ix_supported;        // Modern standby supported
    bool s1_supported;          // S1 sleep supported
    bool s3_supported;          // S3 sleep (Suspend to RAM) supported
    bool s4_supported;          // S4 sleep (Hibernate) supported
    bool s5_supported;          // S5 sleep (Soft off) supported
    uint32_t current_state;     // Current power state
    
    // Battery status
    bool battery_present;       // Battery is present
    uint32_t battery_percentage; // Battery charge percentage
    bool battery_charging;      // Battery is charging
    uint32_t remaining_time;    // Remaining time (minutes)
    
    // Power adapter status
    bool ac_adapter_online;     // AC adapter connected
    
    // Thermal status
    int32_t system_temperature; // System temperature (°C * 10)
    uint32_t thermal_state;     // Thermal state (0=normal, 1=warning, 2=critical)
    
    // CPU power management
    bool cpu_freq_scaling;      // CPU frequency scaling enabled
    uint32_t num_pstates;       // Number of P-states
    uint32_t num_cstates;       // Number of C-states
    
    // Platform features
    bool intel_speedstep;       // Intel SpeedStep available
    bool amd_powernow;          // AMD PowerNow! available
    bool turbo_boost;           // Turbo Boost available
} acpi_power_info_t;

// Power management statistics
typedef struct {
    // Sleep statistics
    uint64_t sleep_count[6];    // Sleep state entry count (S0-S5)
    uint64_t wake_count[6];     // Wake event count
    uint64_t total_sleep_time;  // Total time in sleep states (seconds)
    
    // Power statistics
    uint64_t total_idle_time;   // Total CPU idle time (nanoseconds)
    uint64_t energy_consumed_mwh; // Total energy consumed (mWh)
    uint32_t avg_power_consumption; // Average power consumption (mW)
    
    // Current status
    uint32_t current_power_state; // Current system power state
    bool on_battery;            // Running on battery power
    uint32_t battery_percentage; // Current battery percentage
    uint32_t remaining_time;    // Battery remaining time (minutes)
    int32_t system_temperature; // Current system temperature (°C * 10)
    
    // CPU statistics
    uint32_t num_pstates;       // Number of P-states
    uint32_t num_cstates;       // Number of C-states
    uint32_t avg_frequency;     // Average CPU frequency (MHz)
    
    // Event counters
    uint64_t power_button_events; // Power button press count
    uint64_t thermal_events;    // Thermal event count
    uint64_t battery_events;    // Battery event count
    uint64_t pstate_transitions; // P-state transition count
    uint64_t cstate_entries;    // C-state entry count
} acpi_power_stats_t;

// Advanced power management API
int acpi_power_init(void);
void acpi_power_shutdown(void);
int acpi_enter_sleep_state(uint32_t state);
int acpi_wake_from_sleep(void);
void acpi_get_power_stats(acpi_power_stats_t *stats);

// System calls for power management
asmlinkage long sys_acpi_sleep(int state);
asmlinkage long sys_acpi_get_power_info(acpi_power_info_t __user *info);

// Inline helper functions
static inline bool acpi_is_on_battery(void)
{
    acpi_power_info_t info;
    if (sys_acpi_get_power_info(&info) == 0) {
        return !info.ac_adapter_online;
    }
    return false;
}

static inline bool acpi_supports_sleep_state(uint32_t state)
{
    acpi_power_info_t info;
    if (sys_acpi_get_power_info(&info) != 0) {
        return false;
    }
    
    switch (state) {
        case 0: return info.s0ix_supported;
        case 1: return info.s1_supported;
        case 3: return info.s3_supported;
        case 4: return info.s4_supported;
        case 5: return info.s5_supported;
        default: return false;
    }
}

/* Legacy compatibility for existing code */
typedef acpi_rsdp_t acpi_rsdp_t;
typedef acpi_table_header_t acpi_sdt_hdr_t;
typedef acpi_madt_t acpi_madt_t;
typedef acpi_fadt_t acpi_fadt_t;

/* Legacy function signatures */
int acpi_init(void);  /* Updated signature */
const acpi_madt_t* acpi_get_madt(void);
const acpi_fadt_t* acpi_get_fadt(void);