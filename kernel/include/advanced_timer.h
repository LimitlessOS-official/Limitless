/*
 * LimitlessOS Advanced Timer & Scheduling System Header
 * High-resolution timers, HPET support, tickless kernel, real-time scheduling
 */

#ifndef ADVANCED_TIMER_H
#define ADVANCED_TIMER_H

#include <stdint.h>
#include <stdbool.h>

/* Status codes */
#define STATUS_OK               0
#define STATUS_ERROR           -1
#define STATUS_INVALID         -2
#define STATUS_NOT_SUPPORTED   -3
#define STATUS_NOT_FOUND       -4
#define STATUS_ALREADY_INITIALIZED -5

/* Timer types */
typedef enum {
    TIMER_TYPE_ONESHOT,
    TIMER_TYPE_PERIODIC,
    TIMER_TYPE_WATCHDOG,
    TIMER_TYPE_DEADLINE
} timer_type_t;

/* Real-time scheduling classes */
typedef enum {
    SCHED_CLASS_NORMAL = 0,     /* CFS scheduler */
    SCHED_CLASS_FIFO = 1,       /* FIFO real-time */
    SCHED_CLASS_RR = 2,         /* Round-robin real-time */
    SCHED_CLASS_DEADLINE = 3,   /* Deadline scheduler */
    SCHED_CLASS_BATCH = 4,      /* Batch processing */
    SCHED_CLASS_IDLE = 5        /* Idle tasks */
} sched_class_t;

/* Forward declarations */
typedef struct hrtimer hrtimer_t;
typedef void (*timer_callback_t)(void* data);

/* ============================================================================
 * HIGH-RESOLUTION TIMER API
 * ============================================================================ */

/**
 * Get current system time in nanoseconds
 * @return Current time in nanoseconds since boot
 */
uint64_t hrtimer_get_time_ns(void);

/**
 * Create a new high-resolution timer
 * @param type Timer type (oneshot, periodic, etc.)
 * @param callback Function to call when timer fires
 * @param data Data to pass to callback
 * @return Pointer to timer structure, or NULL on error
 */
hrtimer_t* hrtimer_create(timer_type_t type, timer_callback_t callback, void* data);

/**
 * Start a high-resolution timer
 * @param timer Timer to start
 * @param expires_ns Absolute expiration time in nanoseconds
 * @return 0 on success, negative on error
 */
int hrtimer_start(hrtimer_t* timer, uint64_t expires_ns);

/**
 * Cancel a high-resolution timer
 * @param timer Timer to cancel
 * @return 0 on success, negative on error
 */
int hrtimer_cancel(hrtimer_t* timer);

/**
 * Destroy a high-resolution timer
 * @param timer Timer to destroy
 */
void hrtimer_destroy(hrtimer_t* timer);

/* ============================================================================
 * REAL-TIME SCHEDULING API
 * ============================================================================ */

/**
 * Set process to deadline scheduling class
 * @param pid Process ID
 * @param runtime_ns Execution time budget in nanoseconds
 * @param deadline_ns Relative deadline in nanoseconds
 * @param period_ns Period in nanoseconds
 * @return 0 on success, negative on error
 */
int sched_set_deadline(uint32_t pid, uint64_t runtime_ns, 
                      uint64_t deadline_ns, uint64_t period_ns);

/**
 * Set process to FIFO real-time scheduling
 * @param pid Process ID
 * @param priority Priority (1-99, higher is more important)
 * @return 0 on success, negative on error
 */
int sched_set_fifo(uint32_t pid, int priority);

/**
 * Set process to round-robin real-time scheduling
 * @param pid Process ID
 * @param priority Priority (1-99, higher is more important)
 * @param timeslice_ns Time slice in nanoseconds
 * @return 0 on success, negative on error
 */
int sched_set_rr(uint32_t pid, int priority, uint64_t timeslice_ns);

/**
 * Get real-time scheduling parameters
 * @param pid Process ID
 * @param class Output for scheduling class
 * @param priority Output for priority
 * @return 0 on success, negative on error
 */
int sched_get_params(uint32_t pid, sched_class_t* class, int* priority);

/* ============================================================================
 * TICKLESS KERNEL API
 * ============================================================================ */

/**
 * Enter tickless mode for current CPU
 * @param cpu_id CPU ID to enter tickless mode
 */
void tickless_enter(uint32_t cpu_id);

/**
 * Exit tickless mode for current CPU
 * @param cpu_id CPU ID to exit tickless mode
 */
void tickless_exit(uint32_t cpu_id);

/**
 * Check if CPU is in tickless mode
 * @param cpu_id CPU ID to check
 * @return true if in tickless mode, false otherwise
 */
bool tickless_is_enabled(uint32_t cpu_id);

/**
 * Set tickless parameters
 * @param min_period_ns Minimum tick period in nanoseconds
 * @param max_period_ns Maximum tick period in nanoseconds
 * @return 0 on success, negative on error
 */
int tickless_set_params(uint64_t min_period_ns, uint64_t max_period_ns);

/* ============================================================================
 * TIMER STATISTICS AND MONITORING API
 * ============================================================================ */

/**
 * Print comprehensive timer statistics
 */
void timer_print_statistics(void);

/**
 * Get timer latency statistics
 * @param timer_id Timer ID to get statistics for
 * @param avg_latency_ns Output for average latency
 * @param max_latency_ns Output for maximum latency
 */
void timer_get_latency_stats(uint32_t timer_id, uint64_t* avg_latency_ns, uint64_t* max_latency_ns);

/**
 * Set timer CPU affinity
 * @param timer_id Timer ID
 * @param cpu_id CPU to bind timer to
 * @return 0 on success, negative on error
 */
int timer_set_cpu_affinity(uint32_t timer_id, uint32_t cpu_id);

/**
 * Get system time with nanosecond precision
 * @return Current system time in nanoseconds
 */
uint64_t timer_get_system_time_ns(void);

/**
 * Check if enterprise timer features are enabled
 * @return true if enterprise features are available
 */
bool timer_enterprise_features_enabled(void);

/* ============================================================================
 * HPET (High Precision Event Timer) API
 * ============================================================================ */

/**
 * Check if HPET is available
 * @return true if HPET hardware is present and initialized
 */
bool hpet_is_available(void);

/**
 * Get HPET frequency
 * @return HPET frequency in Hz, 0 if not available
 */
uint64_t hpet_get_frequency(void);

/**
 * Get number of HPET timers
 * @return Number of available HPET timers
 */
uint8_t hpet_get_timer_count(void);

/**
 * Reserve HPET timer for exclusive use
 * @param timer_id HPET timer ID (0-based)
 * @return 0 on success, negative on error
 */
int hpet_reserve_timer(uint8_t timer_id);

/**
 * Release HPET timer
 * @param timer_id HPET timer ID to release
 * @return 0 on success, negative on error
 */
int hpet_release_timer(uint8_t timer_id);

/* ============================================================================
 * POWER MANAGEMENT INTEGRATION
 * ============================================================================ */

/**
 * Get next timer expiry time for power management
 * @param cpu_id CPU ID to check
 * @return Next timer expiry in nanoseconds, 0 if no timers
 */
uint64_t timer_get_next_expiry(uint32_t cpu_id);

/**
 * Set timer power management mode
 * @param aggressive true for aggressive power saving, false for performance
 */
void timer_set_power_mode(bool aggressive);

/**
 * Get estimated sleep duration for CPU
 * @param cpu_id CPU ID to check
 * @return Estimated sleep duration in nanoseconds
 */
uint64_t timer_get_sleep_duration(uint32_t cpu_id);

/* ============================================================================
 * INTERRUPT HANDLING
 * ============================================================================ */

/**
 * Timer interrupt handler (called from low-level interrupt code)
 */
void hrtimer_interrupt_handler(void);

/**
 * Advanced timer interrupt handler with enterprise features
 */
void advanced_timer_interrupt_handler(void);

/**
 * Register timer interrupt handler
 * @param vector Interrupt vector number
 * @param handler Handler function
 * @return 0 on success, negative on error
 */
int timer_register_interrupt_handler(uint8_t vector, void (*handler)(void));

/* ============================================================================
 * SYSTEM INTEGRATION
 * ============================================================================ */

/**
 * Initialize advanced timer system
 * @return STATUS_OK on success, error code on failure
 */
int advanced_timer_init(void);

/**
 * Shutdown timer system
 */
void advanced_timer_shutdown(void);

/**
 * Timer system self-test
 * @return 0 on success, negative on test failure
 */
int timer_self_test(void);

/* ============================================================================
 * COMPATIBILITY LAYER
 * ============================================================================ */

/* Legacy timer functions for compatibility */
uint64_t timer_get_ticks(void);
uint64_t timer_get_freq_hz(void);

#endif /* ADVANCED_TIMER_H */