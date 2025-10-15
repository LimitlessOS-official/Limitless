/*
 * LimitlessOS Enterprise Functions Header
 * Clean version without conflicts
 */

#ifndef ENTERPRISE_H
#define ENTERPRISE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Forward declarations
struct list_head;
struct wait_queue_head;
struct task_struct;
typedef struct task_struct task_t;
typedef uint32_t tid_t;

// Enterprise constants
#define IPI_TYPE_RESCHEDULE 1
#define HAL_PLATFORM_UNKNOWN 0
#define RAID_LEVEL_1 1
#define RAID_STATUS_HEALTHY 1
#define AF_INET 2
#define SOCKET_TYPE_STREAM 1
#define SOCKET_TYPE_DGRAM 2
#define IP_PROTOCOL_TCP 6
#define IP_PROTOCOL_UDP 17
#define VPN_TYPE_IPSEC 1
#define GRAPHICS_API_OPENGL 1
#define NET_VIRT_BRIDGE 1
#define SECURITY_LEVEL_CONFIDENTIAL 3
#define CRYPTO_ALG_AES256 1
#define BIOMETRIC_FINGERPRINT 1
#define LOG_LEVEL_INFO 1
#define METRIC_TYPE_GAUGE 1
#define ALERT_SEVERITY_WARNING 2

// Basic types
typedef volatile uint32_t atomic32_t;
typedef uint32_t hal_platform_t;

typedef enum {
    RAID_STATUS_HEALTHY_VAL = 1,
    RAID_STATUS_DEGRADED = 2,
    RAID_STATUS_FAILED = 3
} raid_status_t;

typedef struct {
    uint32_t addr;
} ipv4_addr_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t refresh_rate;
} display_mode_t;

typedef struct {
    bool nested_virtualization;
    bool hardware_acceleration;
    uint32_t max_vcpus;
    uint64_t max_memory;
} hypervisor_capabilities_t;

typedef struct {
    uint32_t active_alerts;
    uint32_t total_metrics;
    uint64_t log_entries;
} mgmt_stats_t;

// Forward type declarations for complex types
typedef struct network_interface network_interface_t;
typedef struct network_socket network_socket_t;
typedef struct graphics_device graphics_device_t;
typedef struct audio_device audio_device_t;

// Core function declarations
extern void console_printf(const char* format, ...);
extern uint64_t timer_get_ticks(void);
extern void timer_msleep(uint32_t ms);

// SMP functions
extern uint32_t smp_get_cpu_count(void);
extern bool numa_is_available(void);
extern uint32_t numa_get_node_count(void);
extern bool smp_hotplug_supported(void);
extern int smp_cpu_offline(uint32_t cpu);
extern int smp_cpu_online(uint32_t cpu);
extern int smp_send_ipi(uint32_t cpu, uint32_t type);
extern uint64_t smp_get_ipi_count(void);

// IOMMU functions
extern bool iommu_is_available(void);
extern uint32_t iommu_create_domain(void);
extern uint32_t iommu_get_device_count(void);
extern int iommu_attach_device(uint32_t domain_id, uint32_t device_id);

// HAL functions
extern hal_platform_t hal_get_platform(void);
extern bool hal_acpi_available(void);
extern int hal_acpi_enumerate_devices(void);
extern bool hal_uefi_available(void);
extern int hal_uefi_get_system_table(void);
extern bool hal_power_management_available(void);
extern uint32_t hal_get_supported_power_states(void);

// Storage functions
extern uint32_t storage_get_block_device_count(void);
extern bool storage_raid_supported(void);
extern uint32_t storage_create_raid_array(uint32_t level, uint32_t* devices, uint32_t count);
extern raid_status_t storage_get_raid_status(uint32_t raid_id);
extern bool storage_lvm_supported(void);
extern uint32_t storage_create_volume_group(const char* name);
extern uint32_t storage_create_logical_volume(uint32_t vg_id, const char* name, uint64_t size);

// Network functions
extern uint32_t network_get_interface_count(void);
extern network_interface_t* network_find_interface_by_name(const char* name);
extern network_socket_t* network_socket_create(uint32_t family, uint32_t type, uint32_t protocol);
extern uint32_t network_create_vpn_tunnel(const char* name, uint32_t type, ipv4_addr_t* local, ipv4_addr_t* remote, uint32_t family);

// Multimedia functions
extern uint32_t multimedia_get_graphics_device_count(void);
extern graphics_device_t* multimedia_get_primary_gpu(void);
extern uint32_t multimedia_create_graphics_context(graphics_device_t* gpu, uint32_t api);
extern uint32_t multimedia_get_audio_device_count(void);
extern audio_device_t* multimedia_get_default_audio_device(void);
extern int multimedia_configure_audio_device(audio_device_t* device, uint32_t channels, uint32_t sample_rate, uint32_t bit_depth);
extern uint32_t multimedia_get_display_count(void);
extern int multimedia_set_display_mode(uint32_t display_id, display_mode_t* mode);

// Virtualization functions
extern uint32_t virtualization_create_vm(const char* name, uint32_t vcpus, uint64_t memory_mb);
extern uint32_t virtualization_create_container(const char* name, const char* image);
extern uint32_t virtualization_create_network(const char* name, uint32_t type, const char* cidr, uint32_t flags);
extern int virtualization_get_capabilities(hypervisor_capabilities_t* caps);

// Security functions
extern uint32_t security_create_context(const char* name, uint32_t level, uint32_t uid, const char* username);
extern uint32_t security_generate_key(uint32_t algorithm, const char* name, bool hardware, void* params);
extern bool security_tpm_available(void);
extern int security_tmp_generate_random(void* buffer, size_t size);
extern int security_tpm_generate_random(void* buffer, size_t size);
extern uint32_t security_enroll_biometric(uint32_t type, const char* username, void* template_data, size_t template_size);
extern uint32_t security_add_identity_provider(const char* name, const char* type, const char* config);

// Management functions
extern uint32_t mgmt_register_metric(const char* name, const char* description, uint32_t type, const char* unit);
extern void mgmt_update_metric(const char* name, double value);
extern void mgmt_log_structured(uint32_t level, const char* source, const char* message, void* fields, uint32_t field_count);
extern uint32_t mgmt_create_alert_rule(const char* name, const char* metric, const char* op, double threshold, uint32_t severity, uint32_t cooldown);
extern void mgmt_get_stats(mgmt_stats_t* stats);

// Enterprise initialization functions
extern int advanced_smp_init(void);
extern int advanced_iommu_init(void);
extern int advanced_hal_init(void);
extern int enterprise_storage_init(void);
extern int enterprise_network_init(void);
extern int enterprise_multimedia_init(void);
extern int enterprise_virtualization_init(void);
extern int enterprise_security_init(void);
extern int enterprise_management_init(void);

#endif /* ENTERPRISE_H */