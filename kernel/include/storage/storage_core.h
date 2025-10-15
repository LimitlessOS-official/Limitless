/*
 * LimitlessOS Advanced Storage Subsystem
 * Production NVMe, SATA, and advanced block device support
 * Features: Multi-queue, async I/O, SMART, encryption, compression
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../kernel.h"

/* Storage subsystem constants */
#define MAX_STORAGE_DEVICES     256
#define MAX_QUEUE_DEPTH         1024
#define MAX_IO_ENGINES          32
#define SECTOR_SIZE             512
#define MAX_SECTORS_PER_IO      2048
#define MAX_NAMESPACES          256

/* Storage device types */
typedef enum {
    STORAGE_TYPE_UNKNOWN = 0,
    STORAGE_TYPE_HDD,           /* Traditional hard disk */
    STORAGE_TYPE_SSD,           /* SATA SSD */
    STORAGE_TYPE_NVME,          /* NVMe SSD */
    STORAGE_TYPE_EMMC,          /* eMMC storage */
    STORAGE_TYPE_SDCARD,        /* SD card */
    STORAGE_TYPE_USB,           /* USB storage */
    STORAGE_TYPE_OPTICAL,       /* CD/DVD/Blu-ray */
    STORAGE_TYPE_TAPE,          /* Tape storage */
    STORAGE_TYPE_VIRTUAL,       /* Virtual/RAM disk */
    STORAGE_TYPE_MAX
} storage_device_type_t;

/* Storage interface types */
typedef enum {
    STORAGE_INTERFACE_UNKNOWN = 0,
    STORAGE_INTERFACE_SATA,
    STORAGE_INTERFACE_NVME,
    STORAGE_INTERFACE_SCSI,
    STORAGE_INTERFACE_USB,
    STORAGE_INTERFACE_MMC,
    STORAGE_INTERFACE_VIRTUAL,
    STORAGE_INTERFACE_MAX
} storage_interface_t;

/* I/O operation types */
typedef enum {
    IO_OP_READ = 0,
    IO_OP_WRITE,
    IO_OP_FLUSH,
    IO_OP_DISCARD,
    IO_OP_WRITE_ZEROES,
    IO_OP_ZONE_OPEN,
    IO_OP_ZONE_CLOSE,
    IO_OP_ZONE_RESET,
    IO_OP_SECURE_ERASE,
    IO_OP_MAX
} io_operation_t;

/* I/O priority levels */
typedef enum {
    IO_PRIO_RT_HIGH = 0,        /* Real-time high */
    IO_PRIO_RT_NORMAL,          /* Real-time normal */
    IO_PRIO_HIGH,               /* High priority */
    IO_PRIO_NORMAL,             /* Normal priority */
    IO_PRIO_LOW,                /* Low priority */
    IO_PRIO_IDLE,               /* Idle/background */
    IO_PRIO_MAX
} io_priority_t;

/* NVMe command structure */
typedef struct nvme_command {
    union {
        struct {
            uint8_t opcode;
            uint8_t flags;
            uint16_t command_id;
            uint32_t nsid;
            uint64_t cdw2_3;
            uint64_t metadata;
            uint64_t prp1;
            uint64_t prp2;
            uint32_t cdw10;
            uint32_t cdw11;
            uint32_t cdw12;
            uint32_t cdw13;
            uint32_t cdw14;
            uint32_t cdw15;
        } common;
        
        struct {
            uint8_t opcode;
            uint8_t flags;
            uint16_t command_id;
            uint32_t nsid;
            uint64_t rsvd2;
            uint64_t metadata;
            uint64_t prp1;
            uint64_t prp2;
            uint64_t slba;
            uint16_t length;
            uint16_t control;
            uint32_t dsmgmt;
            uint32_t reftag;
            uint16_t apptag;
            uint16_t appmask;
        } rw;
        
        uint32_t cdw[16];
    };
} __attribute__((packed)) nvme_command_t;

/* NVMe completion structure */
typedef struct nvme_completion {
    uint32_t result;
    uint32_t rsvd;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t command_id;
    uint16_t status;
} __attribute__((packed)) nvme_completion_t;

/* Storage I/O request */
typedef struct storage_io_request {
    uint64_t request_id;
    
    /* Operation details */
    io_operation_t operation;
    io_priority_t priority;
    
    /* Data location */
    uint64_t lba_start;         /* Logical block address */
    uint32_t lba_count;         /* Number of blocks */
    uint32_t block_size;        /* Block size in bytes */
    
    /* Data buffer */
    struct {
        void *virtual_addr;
        uint64_t physical_addr;
        size_t size;
        uint32_t alignment;
        bool is_kernel;
    } buffer;
    
    /* Scatter-gather list */
    struct {
        struct scatterlist *sgl;
        uint32_t nents;
        uint32_t orig_nents;
    } sg_list;
    
    /* Completion callback */
    void (*completion_fn)(struct storage_io_request *req, int result);
    void *completion_data;
    
    /* Error handling */
    uint32_t retry_count;
    uint32_t max_retries;
    uint32_t error_code;
    
    /* Timing information */
    uint64_t submit_time;
    uint64_t start_time;
    uint64_t completion_time;
    
    /* Queue information */
    uint32_t queue_id;
    uint32_t tag;
    
    /* Security */
    struct {
        bool encrypted;
        uint32_t key_id;
        uint8_t iv[16];
    } security;
    
    /* Reference counting */
    atomic_t refcount;
    
    /* List management */
    struct list_head list;
    
} storage_io_request_t;

/* Storage queue structure */
typedef struct storage_queue {
    uint32_t queue_id;
    uint32_t queue_size;
    uint32_t cpu_id;            /* CPU affinity */
    
    /* Queue state */
    bool enabled;
    bool suspended;
    
    /* Submission queue */
    struct {
        void *commands;
        uint32_t head;
        uint32_t tail;
        uint32_t size;
        uint32_t stride;
        dma_addr_t dma_addr;
    } sq;
    
    /* Completion queue */
    struct {
        void *completions;
        uint32_t head;
        uint32_t tail;
        uint32_t size;
        uint32_t stride;
        dma_addr_t dma_addr;
        uint16_t cq_phase;
    } cq;
    
    /* Request tracking */
    struct {
        storage_io_request_t **requests;
        unsigned long *tags;
        uint32_t depth;
        atomic_t active_requests;
    } tags;
    
    /* Interrupt handling */
    uint32_t irq_vector;
    bool irq_enabled;
    
    /* Statistics */
    struct {
        uint64_t submitted;
        uint64_t completed;
        uint64_t errors;
        uint64_t timeouts;
        uint64_t total_latency_ns;
        uint64_t max_latency_ns;
        uint64_t bytes_transferred;
    } stats;
    
    /* Locking */
    spinlock_t sq_lock;
    spinlock_t cq_lock;
    
} storage_queue_t;

/* SMART attribute structure */
typedef struct smart_attribute {
    uint8_t id;
    uint16_t flags;
    uint8_t current_value;
    uint8_t worst_value;
    uint64_t raw_value;
    uint8_t threshold;
    char name[32];
} smart_attribute_t;

/* Storage device health information */
typedef struct storage_health {
    /* Temperature */
    struct {
        int16_t current_celsius;
        int16_t max_celsius;
        int16_t min_celsius;
        bool over_temp;
        bool under_temp;
    } temperature;
    
    /* Wear leveling */
    struct {
        uint32_t wear_level_percent;
        uint64_t total_writes;
        uint64_t total_erases;
        uint32_t bad_blocks;
        uint32_t spare_blocks;
    } wear;
    
    /* Error statistics */
    struct {
        uint64_t correctable_errors;
        uint64_t uncorrectable_errors;
        uint64_t read_errors;
        uint64_t write_errors;
        uint64_t timeout_errors;
    } errors;
    
    /* Performance degradation */
    struct {
        uint32_t performance_percent;
        bool degraded;
        uint64_t reallocated_sectors;
        uint64_t pending_sectors;
    } performance;
    
    /* Power statistics */
    struct {
        uint64_t power_on_hours;
        uint32_t power_cycles;
        uint64_t unsafe_shutdowns;
    } power;
    
    /* Overall health */
    uint8_t health_percent;
    bool critical_warning;
    bool read_only;
    
    /* SMART attributes */
    smart_attribute_t smart_attrs[256];
    uint32_t smart_attr_count;
    
} storage_health_t;

/* Storage device capabilities */
typedef struct storage_capabilities {
    /* Basic capabilities */
    bool supports_flush;
    bool supports_discard;
    bool supports_write_zeroes;
    bool supports_secure_erase;
    bool supports_atomic_writes;
    
    /* Advanced features */
    bool supports_ncq;          /* Native Command Queuing */
    bool supports_tcg;          /* Trusted Computing Group */
    bool supports_encryption;
    bool supports_compression;
    bool supports_dedup;
    
    /* Performance features */
    bool supports_multiqueue;
    bool supports_polling;
    bool supports_streaming;
    uint32_t max_queue_depth;
    uint32_t max_queues;
    
    /* Zoned storage */
    bool supports_zoned;
    uint32_t zone_size;
    uint32_t max_open_zones;
    uint32_t max_active_zones;
    
    /* Data integrity */
    bool supports_t10_dif;      /* T10 Data Integrity Field */
    bool supports_metadata;
    uint32_t metadata_size;
    
} storage_capabilities_t;

/* Main storage device structure */
typedef struct storage_device {
    uint32_t device_id;
    char name[64];
    char model[64];
    char serial[64];
    char firmware[32];
    
    /* Device type and interface */
    storage_device_type_t type;
    storage_interface_t interface;
    
    /* Physical information */
    struct device *dev;
    struct pci_dev *pdev;
    void __iomem *regs;
    
    /* Geometry */
    uint64_t total_sectors;
    uint32_t sector_size;
    uint64_t capacity_bytes;
    uint32_t logical_block_size;
    uint32_t physical_block_size;
    
    /* Multi-queue support */
    struct {
        storage_queue_t *queues;
        uint32_t num_queues;
        uint32_t admin_queue_id;
        bool enabled;
    } mq;
    
    /* Namespaces (NVMe) */
    struct {
        struct storage_namespace *namespaces;
        uint32_t count;
        uint32_t active_count;
    } ns;
    
    /* Capabilities */
    storage_capabilities_t caps;
    
    /* Health monitoring */
    storage_health_t health;
    
    /* Performance statistics */
    struct {
        uint64_t read_operations;
        uint64_t write_operations;
        uint64_t bytes_read;
        uint64_t bytes_written;
        uint64_t total_latency_ns;
        uint64_t queue_depth_sum;
        uint32_t current_queue_depth;
        uint32_t max_queue_depth_used;
    } stats;
    
    /* Power management */
    struct {
        bool runtime_pm;
        uint32_t current_state;
        uint64_t idle_time_ns;
        bool supports_apst;      /* Autonomous Power State Transitions */
        uint32_t apst_states[8];
    } power;
    
    /* Security */
    struct {
        bool locked;
        bool frozen;
        bool supports_opal;
        bool supports_ata_security;
        uint32_t encryption_key_id;
    } security;
    
    /* Error handling */
    struct {
        uint32_t error_recovery_timeout;
        uint32_t max_retries;
        bool supports_error_injection;
    } error_handling;
    
    /* Device operations */
    const struct storage_device_ops *ops;
    
    /* Reference counting */
    struct kref kref;
    
    /* List management */
    struct list_head list;
    
    /* Private driver data */
    void *private_data;
    
} storage_device_t;

/* Storage namespace (NVMe concept) */
typedef struct storage_namespace {
    uint32_t nsid;
    struct storage_device *device;
    
    /* Namespace geometry */
    uint64_t size_sectors;
    uint32_t sector_size;
    uint64_t capacity_bytes;
    
    /* Features */
    bool supports_discard;
    bool supports_write_zeroes;
    bool supports_flush;
    
    /* Statistics per namespace */
    struct {
        uint64_t read_ops;
        uint64_t write_ops;
        uint64_t bytes_read;
        uint64_t bytes_written;
    } stats;
    
} storage_namespace_t;

/* Storage device operations */
struct storage_device_ops {
    /* Basic I/O operations */
    int (*submit_io)(struct storage_device *dev, struct storage_io_request *req);
    int (*cancel_io)(struct storage_device *dev, uint64_t request_id);
    
    /* Device management */
    int (*initialize)(struct storage_device *dev);
    void (*cleanup)(struct storage_device *dev);
    int (*reset)(struct storage_device *dev);
    
    /* Queue management */
    int (*create_queue)(struct storage_device *dev, uint32_t queue_id, uint32_t size, uint32_t cpu);
    int (*delete_queue)(struct storage_device *dev, uint32_t queue_id);
    int (*enable_queue)(struct storage_device *dev, uint32_t queue_id);
    int (*disable_queue)(struct storage_device *dev, uint32_t queue_id);
    
    /* Health monitoring */
    int (*get_health)(struct storage_device *dev, struct storage_health *health);
    int (*get_smart_data)(struct storage_device *dev, void *data, size_t size);
    
    /* Security operations */
    int (*secure_erase)(struct storage_device *dev, bool crypto_erase);
    int (*set_encryption_key)(struct storage_device *dev, const uint8_t *key, size_t key_len);
    
    /* Power management */
    int (*suspend)(struct storage_device *dev);
    int (*resume)(struct storage_device *dev);
    int (*set_power_state)(struct storage_device *dev, uint32_t state);
    
    /* Firmware management */
    int (*update_firmware)(struct storage_device *dev, const void *fw_data, size_t size);
    int (*get_firmware_info)(struct storage_device *dev, char *version, size_t size);
};

/* I/O scheduler types */
typedef enum {
    IO_SCHED_NOOP = 0,          /* No-op scheduler */
    IO_SCHED_DEADLINE,          /* Deadline scheduler */
    IO_SCHED_CFQ,               /* Completely Fair Queuing */
    IO_SCHED_MQ_DEADLINE,       /* Multi-queue deadline */
    IO_SCHED_BFQ,               /* Budget Fair Queuing */
    IO_SCHED_KYBER,             /* Kyber scheduler */
    IO_SCHED_AI_ADAPTIVE,       /* AI-adaptive scheduler */
    IO_SCHED_MAX
} io_scheduler_type_t;

/* I/O scheduler structure */
typedef struct io_scheduler {
    io_scheduler_type_t type;
    char name[32];
    
    /* Scheduler operations */
    struct {
        int (*init)(struct storage_device *dev);
        void (*exit)(struct storage_device *dev);
        int (*schedule)(struct storage_device *dev, struct storage_io_request *req);
        void (*completed)(struct storage_device *dev, struct storage_io_request *req);
        void (*timeout)(struct storage_device *dev, struct storage_io_request *req);
    } ops;
    
    /* Scheduler state */
    void *private_data;
    
    /* Statistics */
    struct {
        uint64_t scheduled_requests;
        uint64_t merged_requests;
        uint64_t avg_latency_ns;
        uint64_t throughput_mb_s;
    } stats;
    
    /* AI enhancement */
    struct {
        bool enabled;
        float workload_prediction[4];  /* Sequential, random, mixed, batch */
        uint32_t optimal_queue_depth;
        uint32_t predicted_latency_ns;
    } ai;
    
} io_scheduler_t;

/* Global storage subsystem state */
typedef struct storage_subsystem {
    bool initialized;
    
    /* Device registry */
    struct {
        storage_device_t *devices[MAX_STORAGE_DEVICES];
        uint32_t count;
        spinlock_t lock;
    } devices;
    
    /* I/O engine */
    struct {
        struct workqueue_struct *io_workqueue;
        struct task_struct *io_threads[MAX_IO_ENGINES];
        uint32_t num_threads;
        bool polling_enabled;
    } io_engine;
    
    /* Global statistics */
    struct {
        uint64_t total_read_ops;
        uint64_t total_write_ops;
        uint64_t total_bytes_read;
        uint64_t total_bytes_written;
        uint64_t total_errors;
        uint64_t avg_latency_ns;
        uint32_t current_queue_depth;
    } stats;
    
    /* Health monitoring */
    struct {
        bool enabled;
        uint32_t check_interval_ms;
        struct timer_list health_timer;
        uint32_t critical_device_count;
    } health_monitor;
    
    /* Default I/O scheduler */
    io_scheduler_t default_scheduler;
    
} storage_subsystem_t;

/* External storage subsystem */
extern storage_subsystem_t storage_subsystem;

/* Core storage functions */
int storage_init(void);
void storage_exit(void);

/* Device management */
int storage_device_register(struct storage_device *dev);
void storage_device_unregister(struct storage_device *dev);
struct storage_device *storage_device_find_by_name(const char *name);
struct storage_device *storage_device_get_by_id(uint32_t device_id);

/* I/O submission */
int storage_submit_io(struct storage_device *dev, struct storage_io_request *req);
int storage_submit_io_sync(struct storage_device *dev, struct storage_io_request *req);
struct storage_io_request *storage_alloc_request(gfp_t gfp_flags);
void storage_free_request(struct storage_io_request *req);

/* Multi-queue support */
int storage_init_multiqueue(struct storage_device *dev, uint32_t num_queues);
void storage_cleanup_multiqueue(struct storage_device *dev);
int storage_queue_request(struct storage_device *dev, uint32_t queue_id, 
                         struct storage_io_request *req);

/* NVMe specific functions */
int nvme_init_controller(struct storage_device *dev);
int nvme_submit_command(struct storage_device *dev, struct nvme_command *cmd);
int nvme_create_io_queues(struct storage_device *dev, uint32_t num_queues);
int nvme_identify_controller(struct storage_device *dev, void *data);
int nvme_identify_namespace(struct storage_device *dev, uint32_t nsid, void *data);

/* SATA/AHCI specific functions */
int ahci_init_controller(struct storage_device *dev);
int ahci_submit_command(struct storage_device *dev, struct ata_taskfile *tf);
int sata_identify_device(struct storage_device *dev, void *data);

/* Health monitoring */
int storage_get_device_health(struct storage_device *dev, struct storage_health *health);
int storage_monitor_health_start(struct storage_device *dev);
void storage_monitor_health_stop(struct storage_device *dev);
bool storage_is_device_healthy(struct storage_device *dev);

/* SMART support */
int storage_get_smart_data(struct storage_device *dev, void *data, size_t size);
int storage_parse_smart_attributes(struct storage_device *dev, const void *data, 
                                  struct smart_attribute *attrs, uint32_t *count);

/* Security functions */
int storage_secure_erase(struct storage_device *dev, bool crypto_erase);
int storage_set_encryption_key(struct storage_device *dev, const uint8_t *key, size_t key_len);
int storage_lock_device(struct storage_device *dev, const char *password);
int storage_unlock_device(struct storage_device *dev, const char *password);

/* Performance optimization */
int storage_optimize_queue_depth(struct storage_device *dev);
int storage_tune_scheduler_params(struct storage_device *dev);
void storage_update_performance_counters(struct storage_device *dev);

/* Error handling */
int storage_handle_io_error(struct storage_device *dev, struct storage_io_request *req, int error);
void storage_reset_device(struct storage_device *dev);
int storage_inject_error(struct storage_device *dev, uint32_t error_type);

/* Power management */
int storage_suspend_device(struct storage_device *dev);
int storage_resume_device(struct storage_device *dev);
int storage_set_power_state(struct storage_device *dev, uint32_t state);
void storage_runtime_pm_enable(struct storage_device *dev);

/* Utility functions */
const char *storage_device_type_name(storage_device_type_t type);
const char *storage_interface_name(storage_interface_t interface);
void storage_print_device_info(struct storage_device *dev);
void storage_print_statistics(void);

/* DMA helpers */
dma_addr_t storage_map_buffer(struct device *dev, void *buffer, size_t size, 
                             enum dma_data_direction direction);
void storage_unmap_buffer(struct device *dev, dma_addr_t addr, size_t size,
                         enum dma_data_direction direction);

/* Async I/O support */
struct storage_aio_context;
struct storage_aio_context *storage_aio_create_context(uint32_t max_events);
void storage_aio_destroy_context(struct storage_aio_context *ctx);
int storage_aio_submit(struct storage_aio_context *ctx, struct storage_io_request **reqs, uint32_t count);
int storage_aio_wait_events(struct storage_aio_context *ctx, uint32_t min_events, uint32_t max_events,
                           struct storage_io_request **completed, uint32_t timeout_ms);