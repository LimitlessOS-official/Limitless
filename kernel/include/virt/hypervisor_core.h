/*
 * LimitlessOS Hypervisor Core
 * Type-1 hypervisor implementation with hardware virtualization support
 */

#ifndef HYPERVISOR_CORE_H
#define HYPERVISOR_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

/* Hardware virtualization features */
typedef enum {
    VIRT_FEATURE_NONE           = 0,
    VIRT_FEATURE_VT_X          = (1 << 0),  /* Intel VT-x */
    VIRT_FEATURE_AMD_V         = (1 << 1),  /* AMD-V */
    VIRT_FEATURE_EPT           = (1 << 2),  /* Extended Page Tables */
    VIRT_FEATURE_NPT           = (1 << 3),  /* Nested Page Tables */
    VIRT_FEATURE_IOMMU         = (1 << 4),  /* I/O Memory Management Unit */
    VIRT_FEATURE_SR_IOV        = (1 << 5),  /* Single Root I/O Virtualization */
    VIRT_FEATURE_GPU_PV        = (1 << 6),  /* GPU Paravirtualization */
    VIRT_FEATURE_NESTED_VIRT   = (1 << 7),  /* Nested Virtualization */
    VIRT_FEATURE_SECURE_BOOT   = (1 << 8),  /* Secure Boot for VMs */
    VIRT_FEATURE_ENCRYPTED_MEM = (1 << 9)   /* Memory Encryption (SEV/TXT) */
} virtualization_feature_t;

/* VM states */
typedef enum {
    VM_STATE_STOPPED,
    VM_STATE_STARTING,
    VM_STATE_RUNNING,
    VM_STATE_PAUSED,
    VM_STATE_SUSPENDED,
    VM_STATE_STOPPING,
    VM_STATE_CRASHED,
    VM_STATE_MIGRATING,
    VM_STATE_SNAPSHOTTING,
    VM_STATE_RESTORING
} vm_state_t;

/* VM types */
typedef enum {
    VM_TYPE_HVM,        /* Hardware Virtual Machine */
    VM_TYPE_CONTAINER,  /* Container (LXC/Docker) */
    VM_TYPE_UNIKERNEL,  /* Unikernel */
    VM_TYPE_MICROVM,    /* Lightweight VM (Firecracker-style) */
    VM_TYPE_GPU_VM,     /* GPU-accelerated VM */
    VM_TYPE_REALTIME    /* Real-time VM */
} vm_type_t;

/* CPU virtualization */
typedef struct {
    uint32_t vcpu_count;
    uint32_t cores_per_socket;
    uint32_t threads_per_core;
    uint64_t cpu_features;      /* CPUID feature mask */
    bool nested_virtualization;
    bool cpu_hotplug;
    uint32_t cpu_affinity[64];  /* CPU pinning */
    
    /* Performance monitoring */
    bool pmu_enabled;
    bool perfctr_enabled;
    
    /* Security */
    bool smep;              /* Supervisor Mode Execution Prevention */
    bool smap;              /* Supervisor Mode Access Prevention */
    bool cet;               /* Control-flow Enforcement Technology */
    bool shadow_stack;
} vm_cpu_config_t;

/* Memory virtualization */
typedef struct {
    uint64_t memory_size;       /* Total memory in bytes */
    uint64_t max_memory_size;   /* Maximum memory (for ballooning) */
    bool memory_hotplug;
    bool memory_encryption;     /* AMD SEV/Intel TXT */
    bool memory_compression;
    
    /* NUMA configuration */
    uint32_t numa_nodes;
    uint64_t numa_distances[8][8];
    
    /* Memory backing */
    enum {
        MEMORY_BACKING_ANONYMOUS,
        MEMORY_BACKING_HUGETLB,
        MEMORY_BACKING_FILE,
        MEMORY_BACKING_MEMFD,
        MEMORY_BACKING_NVDIMM
    } memory_backing;
    
    char *backing_file;
    bool prealloc;
    bool shared;
    
    /* Memory ballooning */
    bool balloon_enabled;
    uint64_t balloon_size;
    uint64_t balloon_target;
} vm_memory_config_t;

/* Storage virtualization */
typedef struct {
    char device_path[256];
    char format[32];            /* raw, qcow2, vmdk, vhd, etc. */
    bool readonly;
    bool snapshot;
    enum {
        STORAGE_INTERFACE_IDE,
        STORAGE_INTERFACE_SCSI,
        STORAGE_INTERFACE_VIRTIO_BLK,
        STORAGE_INTERFACE_NVME,
        STORAGE_INTERFACE_USB
    } interface;
    
    /* Performance */
    uint64_t iops_limit;
    uint64_t bandwidth_limit;
    bool aio_native;
    bool direct_io;
    
    /* Encryption */
    bool encrypted;
    char encryption_key[64];
    
    /* Backup/Snapshot */
    bool backup_enabled;
    char backup_path[256];
    uint32_t snapshot_count;
} vm_storage_device_t;

/* Network virtualization */
typedef struct {
    char interface_name[32];
    char mac_address[18];
    enum {
        NET_MODEL_E1000,
        NET_MODEL_VIRTIO_NET,
        NET_MODEL_RTL8139,
        NET_MODEL_VMXNET3,
        NET_MODEL_SR_IOV
    } model;
    
    /* Network backend */
    enum {
        NET_BACKEND_TAP,
        NET_BACKEND_BRIDGE,
        NET_BACKEND_USER,
        NET_BACKEND_VHOST_USER,
        NET_BACKEND_SR_IOV_VF
    } backend;
    
    char backend_name[64];
    
    /* Performance */
    bool multiqueue;
    uint32_t queue_count;
    bool vhost_acceleration;
    
    /* Security */
    bool port_security;
    char allowed_addresses[16][46]; /* Up to 16 allowed IP addresses */
    uint32_t allowed_address_count;
    
    /* QoS */
    uint64_t bandwidth_limit;
    uint32_t priority;
} vm_network_device_t;

/* GPU virtualization */
typedef struct {
    enum {
        GPU_VIRT_NONE,
        GPU_VIRT_PASSTHROUGH,   /* GPU passthrough */
        GPU_VIRT_SR_IOV,        /* SR-IOV virtual functions */
        GPU_VIRT_MEDIATED,      /* Intel GVT-g, NVIDIA vGPU */
        GPU_VIRT_SOFTWARE       /* Software rendering */
    } type;
    
    char gpu_device[64];        /* PCI device identifier */
    uint32_t memory_size;       /* GPU memory in MB */
    uint32_t virtual_outputs;   /* Number of virtual outputs */
    
    /* Performance */
    bool hardware_acceleration;
    bool opencl_support;
    bool cuda_support;
    bool vulkan_support;
    
    /* Display */
    uint32_t max_resolution_x;
    uint32_t max_resolution_y;
    uint32_t refresh_rate;
    bool multi_head;
} vm_gpu_config_t;

/* VM configuration */
typedef struct {
    char name[64];
    char uuid[37];              /* UUID string */
    vm_type_t type;
    vm_state_t state;
    
    /* Hardware configuration */
    vm_cpu_config_t cpu;
    vm_memory_config_t memory;
    vm_gpu_config_t gpu;
    
    /* Storage devices */
    vm_storage_device_t storage_devices[16];
    uint32_t storage_device_count;
    
    /* Network devices */
    vm_network_device_t network_devices[8];
    uint32_t network_device_count;
    
    /* Boot configuration */
    enum {
        BOOT_ORDER_DISK,
        BOOT_ORDER_NETWORK,
        BOOT_ORDER_CDROM,
        BOOT_ORDER_USB
    } boot_order[4];
    uint32_t boot_order_count;
    
    char kernel_path[256];      /* Direct kernel boot */
    char initrd_path[256];
    char kernel_cmdline[512];
    
    /* Advanced features */
    bool secure_boot;
    bool tpm_enabled;
    char tpm_version[8];        /* 1.2 or 2.0 */
    
    /* Migration */
    bool migration_enabled;
    char migration_uri[256];
    bool live_migration;
    
    /* Monitoring */
    bool qmp_enabled;           /* QEMU Machine Protocol */
    uint16_t qmp_port;
    bool vnc_enabled;
    uint16_t vnc_port;
    bool spice_enabled;
    uint16_t spice_port;
    
    /* Resource limits */
    uint32_t cpu_shares;        /* CPU scheduling weight */
    uint64_t memory_limit;      /* Memory limit in bytes */
    uint64_t swap_limit;        /* Swap limit in bytes */
    
    /* Security */
    bool apparmor_profile[256];
    bool selinux_context[256];
    uint32_t uid_map[32];       /* User namespace mapping */
    uint32_t gid_map[32];       /* Group namespace mapping */
} vm_config_t;

/* VM instance */
typedef struct {
    vm_config_t config;
    pid_t process_id;           /* VM process ID */
    time_t start_time;
    time_t last_heartbeat;
    
    /* Runtime state */
    uint64_t cpu_time;          /* CPU time used in nanoseconds */
    uint64_t memory_used;       /* Current memory usage */
    uint64_t network_rx_bytes;
    uint64_t network_tx_bytes;
    uint64_t disk_read_bytes;
    uint64_t disk_write_bytes;
    
    /* Control sockets */
    int monitor_socket;         /* Monitor/control socket */
    int qmp_socket;            /* QMP socket */
    
    /* Threading */
    pthread_t monitor_thread;
    pthread_mutex_t state_mutex;
    
    /* Event callbacks */
    void (*state_change_callback)(const char *vm_name, vm_state_t old_state, vm_state_t new_state);
    void (*error_callback)(const char *vm_name, const char *error_message);
} vm_instance_t;

/* Hypervisor system */
typedef struct {
    bool initialized;
    uint32_t virt_features;     /* Available virtualization features */
    
    /* VM management */
    vm_instance_t vms[256];     /* Up to 256 VMs */
    uint32_t vm_count;
    pthread_mutex_t vm_mutex;
    
    /* Resource management */
    uint64_t total_memory;
    uint64_t available_memory;
    uint32_t total_cpus;
    uint32_t available_cpus;
    
    /* Container integration */
    bool container_support;
    char container_runtime[64]; /* docker, podman, containerd */
    
    /* Networking */
    char default_bridge[32];
    bool nat_enabled;
    bool bridge_enabled;
    
    /* Storage pools */
    struct {
        char name[64];
        char path[256];
        char format[32];        /* dir, lvm, zfs, etc. */
        uint64_t capacity;
        uint64_t available;
    } storage_pools[16];
    uint32_t storage_pool_count;
    
    /* Migration */
    bool migration_enabled;
    uint16_t migration_port;
    char migration_interface[32];
    
    /* Security */
    bool secure_boot_required;
    bool tpm_required;
    bool encryption_required;
    
    /* Performance monitoring */
    struct {
        uint64_t total_vms_created;
        uint64_t total_vms_destroyed;
        uint64_t total_migrations;
        uint64_t failed_migrations;
        uint64_t total_cpu_time;
        uint64_t total_memory_allocated;
        uint64_t total_network_traffic;
        uint64_t total_disk_io;
        
        /* Current metrics */
        double cpu_utilization;
        double memory_utilization;
        double network_utilization;
        double disk_utilization;
    } metrics;
} hypervisor_system_t;

/* Function prototypes */

/* Hypervisor initialization and management */
int hypervisor_init(void);
void hypervisor_exit(void);
int hypervisor_detect_features(void);
bool hypervisor_feature_available(virtualization_feature_t feature);

/* VM lifecycle management */
int vm_create(const vm_config_t *config);
int vm_start(const char *vm_name);
int vm_stop(const char *vm_name, bool force);
int vm_pause(const char *vm_name);
int vm_resume(const char *vm_name);
int vm_restart(const char *vm_name);
int vm_destroy(const char *vm_name);

/* VM configuration */
int vm_get_config(const char *vm_name, vm_config_t *config);
int vm_set_config(const char *vm_name, const vm_config_t *config);
int vm_clone(const char *source_vm, const char *dest_vm);

/* VM state management */
vm_state_t vm_get_state(const char *vm_name);
int vm_get_info(const char *vm_name, vm_instance_t *info);
int vm_list(char vm_names[][64], int max_vms);
int vm_count_running(void);

/* Resource management */
int vm_set_cpu_affinity(const char *vm_name, const uint32_t *cpu_mask, uint32_t cpu_count);
int vm_set_memory_limit(const char *vm_name, uint64_t memory_limit);
int vm_balloon_memory(const char *vm_name, uint64_t target_size);
int vm_hotplug_cpu(const char *vm_name, uint32_t cpu_count);
int vm_hotplug_memory(const char *vm_name, uint64_t memory_size);

/* Storage management */
int vm_attach_disk(const char *vm_name, const vm_storage_device_t *device);
int vm_detach_disk(const char *vm_name, const char *device_path);
int vm_create_snapshot(const char *vm_name, const char *snapshot_name);
int vm_restore_snapshot(const char *vm_name, const char *snapshot_name);
int vm_delete_snapshot(const char *vm_name, const char *snapshot_name);

/* Network management */
int vm_attach_network(const char *vm_name, const vm_network_device_t *device);
int vm_detach_network(const char *vm_name, const char *interface_name);
int vm_set_network_qos(const char *vm_name, const char *interface_name, 
                       uint64_t bandwidth_limit, uint32_t priority);

/* GPU virtualization */
int vm_attach_gpu(const char *vm_name, const vm_gpu_config_t *gpu_config);
int vm_detach_gpu(const char *vm_name);
int vm_list_gpus(char gpu_devices[][64], int max_gpus);

/* Migration */
int vm_migrate(const char *vm_name, const char *destination_host, bool live_migration);
int vm_migration_status(const char *vm_name, double *progress, char *status, size_t status_size);
int vm_migration_cancel(const char *vm_name);

/* Container integration */
int container_create_vm(const char *container_name, const vm_config_t *config);
int container_start_vm(const char *container_name);
int container_stop_vm(const char *container_name);
int container_list_vms(char vm_names[][64], int max_vms);

/* Monitoring and metrics */
int hypervisor_get_metrics(hypervisor_system_t *metrics);
int vm_get_performance_stats(const char *vm_name, vm_instance_t *stats);
int hypervisor_set_monitoring_callback(void (*callback)(const char *event, const char *data));

/* Security */
int vm_enable_secure_boot(const char *vm_name, const char *cert_path);
int vm_enable_tpm(const char *vm_name, const char *tpm_version);
int vm_enable_memory_encryption(const char *vm_name);
int vm_set_security_policy(const char *vm_name, const char *policy);

/* Advanced features */
int vm_enable_nested_virtualization(const char *vm_name);
int vm_create_template(const char *template_name, const vm_config_t *config);
int vm_create_from_template(const char *vm_name, const char *template_name);
int vm_export(const char *vm_name, const char *export_path);
int vm_import(const char *import_path, const char *vm_name);

/* Utility functions */
const char *vm_state_name(vm_state_t state);
const char *vm_type_name(vm_type_t type);
const char *virtualization_feature_name(virtualization_feature_t feature);
int vm_config_validate(const vm_config_t *config);
int vm_config_from_file(const char *config_file, vm_config_t *config);
int vm_config_to_file(const vm_config_t *config, const char *config_file);

/* Global hypervisor system */
extern hypervisor_system_t hypervisor_system;

#endif /* HYPERVISOR_CORE_H */