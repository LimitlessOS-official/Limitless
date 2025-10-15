/* Global I/O port outb for kernel linkage */
#ifdef __x86_64__
extern void outb(uint16_t port, uint8_t value);
#endif
/*
 * LimitlessOS Hardware Abstraction Layer
 * Unified interface for CPU architectures, peripherals, and device management
 */

#pragma once


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* CPU Architecture Types */
typedef enum {
    ARCH_UNKNOWN = 0,
    ARCH_X86_64,
    ARCH_ARM64,
    ARCH_RISCV64,
    ARCH_MAX
} cpu_architecture_t;

/* CPU Vendor Types */
typedef enum {
    CPU_VENDOR_UNKNOWN = 0,
    CPU_VENDOR_INTEL,
    CPU_VENDOR_AMD,
    CPU_VENDOR_ARM,
    CPU_VENDOR_QUALCOMM,
    CPU_VENDOR_APPLE,
    CPU_VENDOR_NVIDIA,
    CPU_VENDOR_RISCV,
    CPU_VENDOR_MAX
} cpu_vendor_t;

/* CPU Features */
typedef struct cpu_features {
    /* x86_64 specific features */
    struct {
        bool sse;           /* Streaming SIMD Extensions */
        bool sse2;          /* SSE2 */
        bool sse3;          /* SSE3 */
        bool ssse3;         /* Supplemental SSE3 */
        bool sse4_1;        /* SSE4.1 */
        bool sse4_2;        /* SSE4.2 */
        bool avx;           /* Advanced Vector Extensions */
        bool avx2;          /* AVX2 */
        bool avx512;        /* AVX-512 */
        bool aes;           /* AES instruction set */
        bool fma;           /* Fused Multiply-Add */
        bool rdrand;        /* Random number generator */
        bool rdseed;        /* Random seed */
        bool tsx;           /* Transactional Synchronization Extensions */
        bool mpx;           /* Memory Protection Extensions */
        bool cet;           /* Control-flow Enforcement Technology */
        bool pku;           /* Protection Keys for Userspace */
    } x86;
    
    /* ARM64 specific features */
    struct {
        bool neon;          /* Advanced SIMD (NEON) */
        bool sve;           /* Scalable Vector Extension */
        bool sve2;          /* SVE2 */
        bool crypto;        /* Cryptographic extensions */
        bool crc32;         /* CRC32 instructions */
        bool atomics;       /* Atomic instructions */
        bool fp16;          /* Half-precision floating point */
        bool dotprod;       /* Dot product instructions */
        bool pointer_auth;  /* Pointer authentication */
        bool mte;           /* Memory Tagging Extension */
        bool bti;           /* Branch Target Identification */
    } arm;
    
    /* RISC-V specific features */
    struct {
        bool compressed;    /* C extension */
        bool atomic;        /* A extension */
        bool multiply;      /* M extension */
        bool single_float;  /* F extension */
        bool double_float;  /* D extension */
        bool vector;        /* V extension */
        bool bit_manip;     /* B extension */
        bool crypto;        /* K extension */
    } riscv;
    
    /* Common features */
    struct {
        bool virtualization;
        bool hardware_breakpoints;
        bool performance_counters;
        bool temperature_sensor;
        bool frequency_scaling;
        bool power_management;
        bool memory_encryption;
        bool secure_boot;
    } common;
    
} cpu_features_t;

/* CPU Topology */
typedef struct cpu_topology {
    int physical_packages;      /* Number of physical CPU packages */
    int cores_per_package;      /* Cores per package */
    int threads_per_core;       /* Threads per core (SMT/HT) */
    int total_logical_cores;    /* Total logical processors */
    
    struct {
        int package_id;         /* Physical package ID */
        int core_id;            /* Physical core ID */
        int thread_id;          /* Hardware thread ID */
        int logical_id;         /* Logical processor ID */
        bool online;            /* CPU is online */
        
        /* Cache information */
        struct {
            int l1i_size_kb;    /* L1 instruction cache */
            int l1d_size_kb;    /* L1 data cache */
            int l2_size_kb;     /* L2 cache */
            int l3_size_kb;     /* L3 cache */
        } cache;
        
        /* NUMA information */
        int numa_node;          /* NUMA node ID */
        
    } cpus[256];               /* Per-CPU information */
    
} cpu_topology_t;

/* Memory Information */
typedef struct memory_info {
    uint64_t total_physical;    /* Total physical memory */
    uint64_t available_physical; /* Available physical memory */
    uint64_t total_virtual;     /* Total virtual address space */
    uint64_t page_size;         /* Memory page size */
    uint64_t large_page_size;   /* Large page size */
    
    /* NUMA topology */
    struct {
        int node_count;         /* Number of NUMA nodes */
        struct {
            int node_id;        /* NUMA node ID */
            uint64_t memory_size; /* Memory in this node */
            uint64_t free_memory; /* Free memory in this node */
            int cpu_count;      /* CPUs in this node */
            int cpu_list[64];   /* CPU IDs in this node */
        } nodes[16];
    } numa;
    
    /* Memory features */
    struct {
        bool ecc;               /* Error-correcting code */
        bool compression;       /* Hardware compression */
        bool encryption;        /* Hardware encryption */
        bool deduplication;     /* Hardware deduplication */
        bool memory_tagging;    /* Memory tagging (ARM MTE) */
    } features;
    
} memory_info_t;

/* Device Types */
typedef enum {
    DEVICE_UNKNOWN = 0,
    DEVICE_CPU,
    DEVICE_MEMORY,
    DEVICE_PCI,
    DEVICE_USB,
    DEVICE_STORAGE,
    DEVICE_NETWORK,
    DEVICE_GRAPHICS,
    DEVICE_AUDIO,
    DEVICE_INPUT,
    DEVICE_SENSOR,
    DEVICE_CAMERA,
    DEVICE_BLUETOOTH,
    DEVICE_WIFI,
    DEVICE_CELLULAR,
    DEVICE_MAX
} device_type_t;

/* Device Information */
typedef struct device_info {
    char name[128];             /* Device name */
    char manufacturer[64];      /* Manufacturer */
    char model[64];             /* Model number */
    char serial[64];            /* Serial number */
    char firmware_version[32];  /* Firmware version */
    char driver_name[64];       /* Driver name */
    device_type_t type;         /* Device type */
    
    /* Device identification */
    union {
        struct {
            uint16_t vendor_id;
            uint16_t device_id;
            uint16_t subsystem_vendor_id;
            uint16_t subsystem_device_id;
            uint8_t class_code;
            uint8_t subclass_code;
            uint8_t prog_if;
            uint8_t revision;
        } pci;
        
        struct {
            uint16_t vendor_id;
            uint16_t product_id;
            uint16_t device_version;
            uint8_t class_code;
            uint8_t subclass_code;
            uint8_t protocol;
        } usb;
        
        struct {
            uint64_t unique_id;
        } generic;
    } id;
    
    /* Device capabilities */
    struct {
        bool hotplug;           /* Hot-plug capable */
        bool removable;         /* Removable device */
        bool power_management;  /* Power management */
        bool dma_capable;       /* DMA capable */
        bool interrupt_capable; /* Interrupt capable */
        bool secure;            /* Secure device */
    } capabilities;
    
    /* Device state */
    struct {
        bool present;           /* Device is present */
        bool enabled;           /* Device is enabled */
        bool configured;        /* Device is configured */
        bool driver_loaded;     /* Driver is loaded */
        bool functioning;       /* Device is functioning */
        int error_count;        /* Error count */
    } state;
    
    /* Resource allocation */
    struct {
        struct {
            uint64_t base;      /* Base address */
            uint64_t size;      /* Size */
            bool prefetchable;  /* Prefetchable */
        } memory_regions[8];
        int memory_region_count;
        
        struct {
            uint16_t base;      /* Base port */
            uint16_t size;      /* Port range size */
        } io_regions[8];
        int io_region_count;
        
        struct {
            int irq_number;     /* IRQ number */
            int irq_type;       /* IRQ type (level/edge) */
        } interrupts[4];
        int interrupt_count;
        
        struct {
            int dma_channel;    /* DMA channel */
        } dma_channels[4];
        int dma_channel_count;
        
    } resources;
    
} device_info_t;

/* Platform Information */
typedef struct platform_info {
    char system_vendor[64];     /* System vendor */
    char system_product[64];    /* System product name */
    char system_version[32];    /* System version */
    char system_serial[64];     /* System serial number */
    char system_uuid[37];       /* System UUID */
    
    char bios_vendor[64];       /* BIOS vendor */
    char bios_version[32];      /* BIOS version */
    char bios_date[16];         /* BIOS date */
    
    char motherboard_vendor[64]; /* Motherboard vendor */
    char motherboard_product[64]; /* Motherboard product */
    char motherboard_version[32]; /* Motherboard version */
    
    /* Platform features */
    struct {
        bool acpi;              /* ACPI support */
        bool uefi;              /* UEFI firmware */
        bool secure_boot;       /* Secure boot */
        bool tpm;               /* TPM support */
        bool virtualization;    /* Hardware virtualization */
        bool iommu;             /* IOMMU support */
        bool wake_on_lan;       /* Wake-on-LAN */
        bool remote_management; /* Remote management (IPMI, AMT) */
    } features;
    
    /* Power management */
    struct {
        bool battery_present;   /* Battery present */
        int battery_percentage; /* Battery percentage */
        bool ac_connected;      /* AC adapter connected */
        int power_states;       /* Supported power states */
        bool cpu_frequency_scaling; /* CPU frequency scaling */
        bool gpu_power_management; /* GPU power management */
    } power;
    
    /* Thermal management */
    struct {
        int sensor_count;       /* Number of thermal sensors */
        struct {
            char name[32];      /* Sensor name */
            int temperature_c;  /* Temperature in Celsius */
            int critical_temp;  /* Critical temperature */
            int warning_temp;   /* Warning temperature */
        } sensors[16];
        
        int fan_count;          /* Number of fans */
        struct {
            char name[32];      /* Fan name */
            int rpm;            /* Current RPM */
            int max_rpm;        /* Maximum RPM */
            bool auto_control;  /* Automatic control */
        } fans[8];
        
    } thermal;
    
} platform_info_t;

/* Hardware Abstraction Layer */
typedef struct hal {
    bool initialized;
    
    /* Architecture information */
    cpu_architecture_t architecture;
    cpu_vendor_t cpu_vendor;
    cpu_features_t cpu_features;
    cpu_topology_t cpu_topology;
    memory_info_t memory_info;
    platform_info_t platform_info;
    
    /* Device management */
    struct {
        device_info_t devices[512];
        int device_count;
        int next_device_id;
    } devices;
    
    /* Driver management */
    struct {
        struct {
            char name[64];      /* Driver name */
            char version[32];   /* Driver version */
            device_type_t supported_types[16]; /* Supported device types */
            int type_count;     /* Number of supported types */
            
            /* Driver callbacks */
            int (*probe)(struct device_info *device);
            int (*remove)(struct device_info *device);
            int (*suspend)(struct device_info *device);
            int (*resume)(struct device_info *device);
            
            bool loaded;        /* Driver is loaded */
            int reference_count; /* Number of devices using this driver */
            
        } drivers[128];
        int driver_count;
    } drivers;
    
    /* Power management */
    struct {
        bool enabled;
        int current_state;      /* Current power state */
        int supported_states;   /* Supported power states bitmask */
        
        struct {
            int (*suspend)(void);
            int (*resume)(void);
            int (*hibernate)(void);
            int (*shutdown)(void);
            int (*reboot)(void);
        } callbacks;
        
        /* CPU frequency scaling */
        struct {
            bool enabled;
            int current_frequency;
            int min_frequency;
            int max_frequency;
            char governor[32];  /* Frequency governor */
        } cpu_scaling;
        
        /* Device power management */
        struct {
            int devices_suspended;
            int devices_total;
        } device_pm;
        
    } power_management;
    
    /* Interrupt management */
    struct {
        int total_interrupts;
        struct {
            int irq_number;
            char device_name[64];
            uint64_t count;
            bool shared;
            void (*handler)(int irq, void *data);
            void *data;
        } interrupts[256];
        int interrupt_count;
    } interrupts;
    
    /* DMA management */
    struct {
        struct {
            int channel;
            char device_name[64];
            bool in_use;
            uint64_t transfer_count;
        } channels[32];
        int channel_count;
    } dma;
    
} hal_t;

/* Global HAL instance */
extern hal_t system_hal;

/* HAL initialization and cleanup */
int hal_init(void);
void hal_exit(void);

/* Architecture detection */
cpu_architecture_t hal_detect_architecture(void);
cpu_vendor_t hal_detect_cpu_vendor(void);
int hal_detect_cpu_features(cpu_features_t *features);
int hal_detect_cpu_topology(cpu_topology_t *topology);
int hal_detect_memory_info(memory_info_t *memory);
int hal_detect_platform_info(platform_info_t *platform);

/* Device management */
int hal_enumerate_devices(void);
int hal_register_device(device_info_t *device);
int hal_unregister_device(int device_id);
device_info_t *hal_find_device(device_type_t type, const char *name);
device_info_t *hal_find_device_by_id(int device_id);
int hal_get_device_list(device_type_t type, device_info_t *devices, int max_devices);
int hal_enable_device(int device_id);
int hal_disable_device(int device_id);
int hal_configure_device(int device_id, void *config);
int hal_get_device_status(int device_id);

/* Driver management */
int hal_register_driver(const char *name, const char *version, 
                       device_type_t *supported_types, int type_count,
                       int (*probe)(struct device_info *),
                       int (*remove)(struct device_info *),
                       int (*suspend)(struct device_info *),
                       int (*resume)(struct device_info *));
int hal_unregister_driver(const char *name);
int hal_load_driver(const char *name);
int hal_unload_driver(const char *name);
int hal_bind_device_driver(int device_id, const char *driver_name);
int hal_unbind_device_driver(int device_id);
const char *hal_get_device_driver(int device_id);

/* Power management */
int hal_power_init(void);
int hal_power_suspend(void);
int hal_power_resume(void);
int hal_power_hibernate(void);
int hal_power_shutdown(void);
int hal_power_reboot(void);
int hal_power_set_state(int state);
int hal_power_get_state(void);
int hal_power_set_cpu_frequency(int frequency);
int hal_power_get_cpu_frequency(void);
int hal_power_set_cpu_governor(const char *governor);
const char *hal_power_get_cpu_governor(void);

/* Interrupt management */
int hal_request_irq(int irq, void (*handler)(int, void *), void *data, const char *name);
int hal_free_irq(int irq);
int hal_enable_irq(int irq);
int hal_disable_irq(int irq);
int hal_get_irq_count(int irq);
bool hal_is_irq_shared(int irq);

/* DMA management */
int hal_request_dma_channel(const char *device_name);
int hal_release_dma_channel(int channel);
int hal_dma_transfer(int channel, uint64_t src, uint64_t dst, size_t size);
int hal_dma_get_status(int channel);
uint64_t hal_dma_get_transfer_count(int channel);

/* Memory management */
void *hal_alloc_coherent_memory(size_t size, uint64_t *phys_addr);
void hal_free_coherent_memory(void *virt_addr, size_t size, uint64_t phys_addr);
uint64_t hal_virt_to_phys(void *virt_addr);
void *hal_phys_to_virt(uint64_t phys_addr);
int hal_map_device_memory(uint64_t phys_addr, size_t size, void **virt_addr);
int hal_unmap_device_memory(void *virt_addr, size_t size);

/* I/O port access (x86 specific) */
uint8_t hal_inb(uint16_t port);
uint16_t hal_inw(uint16_t port);
uint32_t hal_inl(uint16_t port);
void hal_outb(uint16_t port, uint8_t value);
void hal_outw(uint16_t port, uint16_t value);
void hal_outl(uint16_t port, uint32_t value);

/* Memory-mapped I/O */
uint8_t hal_readb(volatile void *addr);
uint16_t hal_readw(volatile void *addr);
uint32_t hal_readl(volatile void *addr);
uint64_t hal_readq(volatile void *addr);
void hal_writeb(volatile void *addr, uint8_t value);
void hal_writew(volatile void *addr, uint16_t value);
void hal_writel(volatile void *addr, uint32_t value);
void hal_writeq(volatile void *addr, uint64_t value);

/* CPU operations */
void hal_cpu_pause(void);
void hal_cpu_relax(void);
void hal_cpu_barrier(void);
void hal_cpu_flush_cache(void);
void hal_cpu_invalidate_cache(void);
void hal_cpu_flush_tlb(void);
void hal_cpu_enable_interrupts(void);
void hal_cpu_disable_interrupts(void);
bool hal_cpu_interrupts_enabled(void);
uint64_t hal_cpu_get_timestamp(void);
uint64_t hal_cpu_get_frequency(void);

void hal_arch_switch_aspace(void *new_aspace);

/* NUMA support */
int hal_numa_get_node_count(void);
int hal_numa_get_current_node(void);
int hal_numa_get_node_cpus(int node, int *cpus, int max_cpus);
uint64_t hal_numa_get_node_memory(int node);
int hal_numa_set_policy(int policy);
int hal_numa_bind_to_node(int node);

/* Thermal management */
int hal_thermal_get_sensor_count(void);
int hal_thermal_get_temperature(int sensor);
int hal_thermal_get_critical_temp(int sensor);
int hal_thermal_set_threshold(int sensor, int temp);
int hal_thermal_get_fan_count(void);
int hal_thermal_get_fan_speed(int fan);
int hal_thermal_set_fan_speed(int fan, int rpm);
bool hal_thermal_is_auto_control(int fan);
int hal_thermal_set_auto_control(int fan, bool enable);

/* Performance monitoring */
int hal_perf_init(void);
int hal_perf_start_counter(int counter, int event);
int hal_perf_stop_counter(int counter);
uint64_t hal_perf_read_counter(int counter);
int hal_perf_get_counter_count(void);

/* Hardware debugging */
int hal_debug_set_breakpoint(uint64_t addr);
int hal_debug_clear_breakpoint(int bp_id);
int hal_debug_set_watchpoint(uint64_t addr, size_t size, int type);
int hal_debug_clear_watchpoint(int wp_id);
int hal_debug_single_step(void);
int hal_debug_continue(void);

/* Platform-specific functions */
#ifdef __x86_64__
/* x86_64 specific functions */
void hal_x86_cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);
void hal_x86_wrmsr(uint32_t msr, uint64_t value);
uint64_t hal_x86_rdmsr(uint32_t msr);
void hal_x86_write_cr0(uint64_t value);
uint64_t hal_x86_read_cr0(void);
void hal_x86_write_cr3(uint64_t value);
uint64_t hal_x86_read_cr3(void);
void hal_x86_write_cr4(uint64_t value);
uint64_t hal_x86_read_cr4(void);
#endif

#ifdef __aarch64__
/* ARM64 specific functions */
uint64_t hal_arm_read_sctlr(void);
void hal_arm_write_sctlr(uint64_t value);
uint64_t hal_arm_read_ttbr0(void);
void hal_arm_write_ttbr0(uint64_t value);
uint64_t hal_arm_read_ttbr1(void);
void hal_arm_write_ttbr1(uint64_t value);
void hal_arm_dsb(void);
void hal_arm_dmb(void);
void hal_arm_isb(void);
#endif

/* Utility functions */
const char *hal_architecture_name(cpu_architecture_t arch);
const char *hal_cpu_vendor_name(cpu_vendor_t vendor);
const char *hal_device_type_name(device_type_t type);
bool hal_cpu_has_feature(const cpu_features_t *features, const char *feature_name);
int hal_format_device_info(const device_info_t *device, char *buffer, size_t size);
int hal_format_cpu_info(char *buffer, size_t size);
int hal_format_memory_info(char *buffer, size_t size);
int hal_format_platform_info(char *buffer, size_t size);