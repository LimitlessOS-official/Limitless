/*
 * LimitlessOS Advanced Interrupt Management
 * Production-quality interrupt controller with AI balancing
 * Features: APIC/x2APIC, MSI/MSI-X, threaded IRQs, storm detection
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../kernel.h"

/* Interrupt configuration constants */
#define MAX_IRQS                    256
#define MAX_CPUS                    256
#define MAX_MSI_VECTORS             2048
#define IRQ_STACK_SIZE              16384
#define MAX_NESTED_IRQS             8

/* Interrupt types */
typedef enum {
    IRQ_TYPE_EDGE_RISING = 0,
    IRQ_TYPE_EDGE_FALLING,
    IRQ_TYPE_EDGE_BOTH,
    IRQ_TYPE_LEVEL_HIGH,
    IRQ_TYPE_LEVEL_LOW,
    IRQ_TYPE_NONE
} irq_flow_type_t;

/* Interrupt priorities */
typedef enum {
    IRQ_PRIO_CRITICAL = 0,      /* NMI, MCE, critical errors */
    IRQ_PRIO_HIGH,              /* Timer, IPI, urgent devices */
    IRQ_PRIO_NORMAL,            /* Network, storage, keyboard */
    IRQ_PRIO_LOW,               /* Background devices */
    IRQ_PRIO_IDLE,              /* Housekeeping */
    IRQ_PRIO_MAX
} irq_priority_t;

/* APIC types */
typedef enum {
    APIC_TYPE_NONE = 0,
    APIC_TYPE_LOCAL_APIC,
    APIC_TYPE_IO_APIC,
    APIC_TYPE_X2APIC,
    APIC_TYPE_HYPERV_APIC,
    APIC_TYPE_MAX
} apic_type_t;

/* MSI/MSI-X information */
typedef struct msi_info {
    bool enabled;
    bool is_msix;
    uint32_t nvec_used;
    uint32_t nvec_max;
    struct {
        uint64_t address;
        uint32_t data;
        uint32_t vector;
        uint32_t cpu;
    } vectors[MAX_MSI_VECTORS];
} msi_info_t;

/* AI interrupt balancer */
typedef struct ai_interrupt_balancer {
    bool enabled;
    
    /* Neural network for load prediction */
    struct {
        float cpu_weights[MAX_CPUS][64];
        float device_weights[64][32];
        float balance_weights[32][16];
        float output_weights[16][MAX_CPUS];
    } neural_network;
    
    /* Per-CPU interrupt load tracking */
    struct {
        uint64_t irq_count;
        uint64_t processing_time_ns;
        uint64_t avg_latency_ns;
        float load_factor;
        bool overloaded;
        uint64_t last_balance_time;
    } cpu_load[MAX_CPUS];
    
    /* Per-IRQ statistics */
    struct {
        uint64_t count;
        uint64_t processing_time_ns;
        uint64_t avg_rate_per_sec;
        uint32_t preferred_cpu;
        float cpu_affinity_scores[MAX_CPUS];
    } irq_stats[MAX_IRQS];
    
    /* Balancing parameters */
    struct {
        uint32_t balance_interval_ms;
        float load_threshold;
        uint32_t migration_cost_ns;
        bool numa_aware;
    } config;
    
    /* Statistics */
    uint64_t total_migrations;
    uint64_t successful_predictions;
    uint64_t total_predictions;
    float prediction_accuracy;
    
} ai_interrupt_balancer_t;

/* Interrupt storm detection */
typedef struct irq_storm_detector {
    bool enabled;
    uint32_t threshold_per_sec;
    uint32_t burst_threshold;
    uint32_t quiet_time_ms;
    
    /* Per-IRQ storm tracking */
    struct {
        uint64_t count_last_sec;
        uint64_t burst_count;
        uint64_t last_timestamp;
        bool storm_detected;
        bool throttled;
        uint32_t throttle_level;
    } irq_tracking[MAX_IRQS];
    
    /* Global storm statistics */
    uint64_t total_storms_detected;
    uint64_t total_irqs_throttled;
    uint64_t storm_recovery_time_ms;
    
} irq_storm_detector_t;

/* Threaded IRQ management */
typedef struct threaded_irq {
    struct task_struct *thread;
    irq_handler_t handler;
    irq_handler_t thread_fn;
    void *dev_id;
    char name[32];
    
    /* Threading control */
    bool force_threading;
    bool oneshot;
    bool masked;
    
    /* Performance tracking */
    uint64_t handler_time_ns;
    uint64_t thread_time_ns;
    uint64_t context_switches;
    uint64_t wakeup_latency_ns;
    
} threaded_irq_t;

/* IRQ descriptor - core interrupt management structure */
typedef struct irq_desc {
    uint32_t irq;
    uint32_t hwirq;
    
    /* IRQ configuration */
    irq_flow_type_t irq_type;
    irq_priority_t priority;
    uint32_t flags;
    
    /* Handler chain */
    struct irqaction *action;
    
    /* IRQ chip (hardware abstraction) */
    struct irq_chip *chip;
    void *chip_data;
    
    /* Threading */
    threaded_irq_t *threaded;
    
    /* CPU affinity */
    cpumask_t affinity;
    cpumask_t effective_affinity;
    uint32_t target_cpu;
    
    /* MSI information */
    msi_info_t msi_info;
    
    /* Statistics */
    struct {
        uint64_t count;
        uint64_t spurious_count;
        uint64_t unhandled_count;
        uint64_t total_time_ns;
        uint64_t max_time_ns;
        uint64_t min_time_ns;
        uint64_t avg_time_ns;
        uint64_t last_timestamp;
    } stats;
    
    /* AI prediction data */
    struct {
        float predicted_rate;
        uint32_t predicted_cpu;
        float confidence_level;
        uint64_t last_prediction_time;
    } ai_prediction;
    
    /* Storm detection */
    struct {
        bool storm_detected;
        uint32_t storm_level;
        uint64_t storm_start_time;
        uint32_t throttle_count;
    } storm_info;
    
    /* Locking */
    raw_spinlock_t lock;
    struct mutex request_mutex;
    
    /* Status flags */
    unsigned int status_use_accessors;
    unsigned int core_internal_state__do_not_mess_with_it;
    unsigned int depth;
    unsigned int wake_depth;
    unsigned int irq_count;
    unsigned long last_unhandled;
    unsigned int irqs_unhandled;
    atomic_t threads_handled;
    int threads_handled_last;
    
    /* Debugging */
    const char *name;
    struct dentry *debugfs_dir;
    
} irq_desc_t;

/* IRQ chip operations */
struct irq_chip {
    const char *name;
    
    unsigned int (*irq_startup)(struct irq_data *data);
    void (*irq_shutdown)(struct irq_data *data);
    void (*irq_enable)(struct irq_data *data);
    void (*irq_disable)(struct irq_data *data);
    
    void (*irq_ack)(struct irq_data *data);
    void (*irq_mask)(struct irq_data *data);
    void (*irq_mask_ack)(struct irq_data *data);
    void (*irq_unmask)(struct irq_data *data);
    void (*irq_eoi)(struct irq_data *data);
    
    int (*irq_set_affinity)(struct irq_data *data, const struct cpumask *dest, bool force);
    int (*irq_retrigger)(struct irq_data *data);
    int (*irq_set_type)(struct irq_data *data, unsigned int flow_type);
    int (*irq_set_wake)(struct irq_data *data, unsigned int on);
    
    void (*irq_bus_lock)(struct irq_data *data);
    void (*irq_bus_sync_unlock)(struct irq_data *data);
    
    void (*irq_cpu_online)(struct irq_data *data);
    void (*irq_cpu_offline)(struct irq_data *data);
    
    void (*irq_suspend)(struct irq_data *data);
    void (*irq_resume)(struct irq_data *data);
    void (*irq_pm_shutdown)(struct irq_data *data);
    
    void (*irq_calc_mask)(struct irq_data *data);
    
    void (*irq_print_chip)(struct irq_data *data, struct seq_file *p);
    int (*irq_request_resources)(struct irq_data *data);
    void (*irq_release_resources)(struct irq_data *data);
    
    void (*irq_compose_msi_msg)(struct irq_data *data, struct msi_msg *msg);
    void (*irq_write_msi_msg)(struct irq_data *data, struct msi_msg *msg);
    
    int (*irq_get_irqchip_state)(struct irq_data *data, enum irqchip_irq_state which, bool *state);
    int (*irq_set_irqchip_state)(struct irq_data *data, enum irqchip_irq_state which, bool state);
    
    int (*irq_set_vcpu_affinity)(struct irq_data *data, void *vcpu_info);
    
    void (*ipi_send_single)(struct irq_data *data, unsigned int cpu);
    void (*ipi_send_mask)(struct irq_data *data, const struct cpumask *dest);
    
    unsigned long flags;
};

/* APIC controller structure */
typedef struct apic_controller {
    apic_type_t type;
    bool enabled;
    bool x2apic_mode;
    
    /* APIC addresses */
    struct {
        uint64_t local_apic_base;
        uint64_t io_apic_base[16];
        uint32_t io_apic_count;
    } addresses;
    
    /* Local APIC info */
    struct {
        uint32_t id;
        uint32_t version;
        uint32_t max_lvt;
        bool apic_id_valid;
        uint32_t timer_divisor;
        uint64_t timer_frequency;
    } local_apic;
    
    /* I/O APIC info */
    struct {
        struct {
            uint32_t id;
            uint32_t version;
            uint32_t max_redir_entries;
            uint64_t base_address;
        } io_apics[16];
        uint32_t total_pins;
        uint32_t gsi_base[16];
    } io_apic;
    
    /* Inter-processor interrupts */
    struct {
        uint32_t ipi_vector_base;
        uint64_t ipi_count[MAX_CPUS];
        uint64_t ipi_latency_ns[MAX_CPUS];
    } ipi_info;
    
    /* Performance monitoring */
    struct {
        uint64_t interrupts_delivered;
        uint64_t interrupts_received;
        uint64_t spurious_interrupts;
        uint64_t delivery_errors;
        uint64_t avg_delivery_latency_ns;
    } performance;
    
} apic_controller_t;

/* Global interrupt controller state */
typedef struct interrupt_controller {
    bool initialized;
    bool smp_enabled;
    
    /* IRQ descriptors */
    irq_desc_t *irq_descs[MAX_IRQS];
    uint32_t nr_irqs;
    
    /* APIC controller */
    apic_controller_t apic;
    
    /* AI interrupt balancer */
    ai_interrupt_balancer_t ai_optimizer;
    
    /* Storm detector */
    irq_storm_detector_t storm_detector;
    
    /* Per-CPU interrupt stacks */
    struct {
        void *irq_stack;
        uint32_t stack_size;
        uint32_t nesting_level;
        struct irq_desc *current_irq;
    } cpu_stacks[MAX_CPUS];
    
    /* Global statistics */
    struct {
        uint64_t total_interrupts;
        uint64_t handled_interrupts;
        uint64_t spurious_interrupts;
        uint64_t nested_interrupts;
        uint64_t threaded_interrupts;
        uint64_t storm_detections;
        uint64_t load_balances;
    } global_stats;
    
    /* Performance monitoring */
    struct {
        uint64_t avg_irq_latency_ns;
        uint64_t max_irq_latency_ns;
        uint64_t total_irq_time_ns;
        uint32_t irq_load_percent[MAX_CPUS];
    } performance;
    
    /* Configuration */
    struct {
        bool threaded_irqs_default;
        bool storm_detection_enabled;
        bool ai_balancing_enabled;
        uint32_t balance_interval_ms;
        uint32_t storm_threshold;
    } config;
    
    /* Debugging */
    uint32_t debug_level;
    bool tracing_enabled;
    
} interrupt_controller_t;

/* External interrupt controller state */
extern interrupt_controller_t irq_ctrl;

/* Core interrupt functions */
void irq_init(void);
void irq_init_smp(void);

/* IRQ management */
int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
               const char *name, void *dev);
int request_threaded_irq(unsigned int irq, irq_handler_t handler,
                        irq_handler_t thread_fn, unsigned long flags,
                        const char *name, void *dev);
void free_irq(unsigned int irq, void *dev_id);
void enable_irq(unsigned int irq);
void disable_irq(unsigned int irq);
void disable_irq_nosync(unsigned int irq);
void synchronize_irq(unsigned int irq);

/* IRQ descriptor management */
struct irq_desc *irq_to_desc(unsigned int irq);
unsigned int irq_get_irq_data(unsigned int irq);
int irq_alloc_descs(int irq, unsigned int from, unsigned int cnt, int node);
void irq_free_descs(unsigned int from, unsigned int cnt);

/* IRQ affinity and balancing */
int irq_set_affinity(unsigned int irq, const struct cpumask *cpumask);
int irq_set_affinity_hint(unsigned int irq, const struct cpumask *m);
int irq_force_affinity(unsigned int irq, const struct cpumask *cpumask);
void irq_migrate_all_off_this_cpu(void);

/* APIC management */
void apic_init(void);
void apic_init_uniprocessor(void);
void x2apic_enable(void);
void send_ipi(int cpu, int vector);
void send_ipi_mask(const struct cpumask *mask, int vector);
void send_ipi_all(int vector);
void send_ipi_allbutself(int vector);

/* MSI/MSI-X support */
int pci_enable_msi(struct pci_dev *dev);
int pci_enable_msix(struct pci_dev *dev, struct msix_entry *entries, int nvec);
void pci_disable_msi(struct pci_dev *dev);
void pci_disable_msix(struct pci_dev *dev);
int msi_alloc_irq(struct device *dev, int nvec);
void msi_free_irq(struct device *dev, unsigned int irq);

/* AI interrupt optimization */
void ai_optimize_irq_placement(void);
void ai_predict_irq_load(unsigned int irq);
void ai_update_irq_prediction(unsigned int irq, uint64_t processing_time);
uint32_t ai_get_optimal_cpu_for_irq(unsigned int irq);
void ai_train_interrupt_model(void);

/* Storm detection and mitigation */
void irq_storm_check(unsigned int irq);
void irq_storm_throttle(unsigned int irq);
void irq_storm_unthrottle(unsigned int irq);
bool irq_is_storm_detected(unsigned int irq);

/* Threaded IRQ support */
int setup_irq_thread(struct irqaction *new, unsigned int irq, bool secondary);
void irq_wake_thread(unsigned int irq, void *dev_id);
void irq_thread_check_affinity(struct irq_desc *desc, struct irqaction *action);

/* IRQ statistics and debugging */
void show_interrupts(struct seq_file *p, void *v);
void irq_print_stats(void);
void irq_debug_show_chip(unsigned int irq);
void irq_debug_show_data(unsigned int irq);

/* Generic IRQ handling */
irqreturn_t handle_irq_event_percpu(struct irq_desc *desc, struct irqaction *action);
irqreturn_t handle_irq_event(struct irq_desc *desc);
void generic_handle_irq(unsigned int irq);
int generic_handle_domain_irq(struct irq_domain *domain, unsigned int hwirq);

/* IRQ flow handlers */
void handle_level_irq(struct irq_desc *desc);
void handle_edge_irq(struct irq_desc *desc);
void handle_fasteoi_irq(struct irq_desc *desc);
void handle_simple_irq(struct irq_desc *desc);
void handle_percpu_irq(struct irq_desc *desc);
void handle_percpu_devid_irq(struct irq_desc *desc);
void handle_bad_irq(struct irq_desc *desc);

/* IRQ flags */
#define IRQF_TRIGGER_NONE      0x00000000
#define IRQF_TRIGGER_RISING    0x00000001
#define IRQF_TRIGGER_FALLING   0x00000002
#define IRQF_TRIGGER_HIGH      0x00000004
#define IRQF_TRIGGER_LOW       0x00000008
#define IRQF_TRIGGER_MASK      (IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW | \
                               IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING)
#define IRQF_TRIGGER_PROBE     0x00000010

#define IRQF_SHARED            0x00000080
#define IRQF_PROBE_SHARED      0x00000100
#define IRQF_TIMER             0x00000200
#define IRQF_PERCPU            0x00000400
#define IRQF_NOBALANCING       0x00000800
#define IRQF_IRQPOLL           0x00001000
#define IRQF_ONESHOT           0x00002000
#define IRQF_NO_SUSPEND        0x00004000
#define IRQF_FORCE_RESUME      0x00008000
#define IRQF_NO_THREAD         0x00010000
#define IRQF_EARLY_RESUME      0x00020000
#define IRQF_COND_SUSPEND      0x00040000

/* IRQ return values */
#define IRQ_NONE               (0 << 0)
#define IRQ_HANDLED            (1 << 0)
#define IRQ_WAKE_THREAD        (1 << 1)

typedef int irqreturn_t;
typedef irqreturn_t (*irq_handler_t)(int, void *);

/* Utility macros */
#define local_irq_enable()      __asm__ __volatile__("sti": : :"memory")
#define local_irq_disable()     __asm__ __volatile__("cli": : :"memory")

#define local_irq_save(flags) \
    do { \
        __asm__ __volatile__("pushf ; pop %0" : "=g" (flags) : : "memory"); \
        local_irq_disable(); \
    } while (0)

#define local_irq_restore(flags) \
    __asm__ __volatile__("push %0 ; popf" : : "g" (flags) : "memory", "cc")

#define irqs_disabled() \
    ({ \
        unsigned long flags; \
        __asm__ __volatile__("pushf ; pop %0" : "=g" (flags) : : "memory"); \
        !(flags & 0x200); \
    })

/* Hardware abstraction */
static inline void apic_write(u32 reg, u32 val)
{
    *(volatile u32 *)(APIC_BASE + reg) = val;
}

static inline u32 apic_read(u32 reg)
{
    return *(volatile u32 *)(APIC_BASE + reg);
}

static inline void x2apic_wrmsr(u32 reg, u64 val)
{
    u32 low = val & 0xFFFFFFFF;
    u32 high = val >> 32;
    __asm__ __volatile__("wrmsr" : : "c" (reg), "a" (low), "d" (high));
}

static inline u64 x2apic_rdmsr(u32 reg)
{
    u32 low, high;
    __asm__ __volatile__("rdmsr" : "=a" (low), "=d" (high) : "c" (reg));
    return ((u64)high << 32) | low;
}