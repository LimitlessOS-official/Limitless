/*
 * LimitlessOS Phase 6 - Comprehensive Testing Framework
 * Enterprise-grade testing infrastructure with QA automation
 */

#ifndef TESTING_H
#define TESTING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Status codes */
typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_INVALID_PARAMETER = 2,
    STATUS_OUT_OF_MEMORY = 3,
    STATUS_TIMEOUT = 4
} status_t;

/* Testing framework constants */
#define MAX_TESTS 512
#define MAX_TEST_SUITES 32
#define MAX_TEST_NAME 64
#define MAX_ERROR_MSG 256

/* Test types */
typedef enum {
    TEST_TYPE_UNIT = 1,
    TEST_TYPE_INTEGRATION = 2,
    TEST_TYPE_SYSTEM = 3,
    TEST_TYPE_STRESS = 4,
    TEST_TYPE_SECURITY = 5,
    TEST_TYPE_PERFORMANCE = 6
} test_type_t;

/* Test case structure */
typedef struct {
    uint32_t id;
    char name[MAX_TEST_NAME];
    test_type_t type;
    bool (*test_function)(void);
    uint32_t timeout_ms;
    bool enabled;
    uint32_t priority;
} test_case_t;

/* Test suite structure */
typedef struct {
    uint32_t id;
    char name[MAX_TEST_NAME];
    test_type_t type;
    uint32_t test_count;
    uint32_t passed_count;
    uint32_t failed_count;
    uint64_t execution_time_ms;
    bool enabled;
} test_suite_t;

/* Test result structure */
struct test_result {
    char name[MAX_TEST_NAME];
    bool passed;
    uint64_t duration_us;
    char error_msg[MAX_ERROR_MSG];
};

/* Global testing state */
struct testing_framework {
    struct test_result results[MAX_TESTS];
    uint32_t test_count;
    uint32_t passed_count;
    uint32_t failed_count;
    uint64_t total_duration_us;
    bool verbose;
};

/* Testing macros */
#define TEST_START(name) do { \
    uint64_t _start_time = timer_get_ticks(); \
    console_printf("Testing: %s... ", (name)); \
    bool _test_passed = true; \
    char _error_buffer[MAX_ERROR_MSG] = {0};

#define TEST_END(name) \
    uint64_t _end_time = timer_get_ticks(); \
    uint64_t _duration = _end_time - _start_time; \
    record_test_result((name), _test_passed, _duration, _error_buffer); \
    if (_test_passed) { \
        console_printf("PASSED (%llu us)\n", _duration); \
    } else { \
        console_printf("FAILED (%llu us): %s\n", _duration, _error_buffer); \
    } \
} while(0)

#define TEST_ASSERT(condition, msg) do { \
    if (!(condition)) { \
        _test_passed = false; \
        snprintf(_error_buffer, MAX_ERROR_MSG, "Assertion failed: %s", (msg)); \
    } \
} while(0)

#define TEST_ASSERT_EQ(expected, actual, msg) do { \
    if ((expected) != (actual)) { \
        _test_passed = false; \
        snprintf(_error_buffer, MAX_ERROR_MSG, "%s: expected %d, got %d", (msg), (int)(expected), (int)(actual)); \
    } \
} while(0)

#define TEST_ASSERT_NOT_NULL(ptr, msg) do { \
    if ((ptr) == NULL) { \
        _test_passed = false; \
        snprintf(_error_buffer, MAX_ERROR_MSG, "%s: pointer is NULL", (msg)); \
    } \
} while(0)

#define TEST_ASSERT_NULL(ptr, msg) do { \
    if ((ptr) != NULL) { \
        _test_passed = false; \
        snprintf(_error_buffer, MAX_ERROR_MSG, "%s: pointer is not NULL", (msg)); \
    } \
} while(0)

/* Original testing framework functions */
extern void testing_framework_init(void);
extern void record_test_result(const char* name, bool passed, uint64_t duration, const char* error_msg);
extern void print_test_summary(void);
extern void set_verbose_testing(bool verbose);
extern uint32_t get_failed_test_count(void);

/* Phase 6 Enhanced Testing Framework Functions */
extern status_t test_init_comprehensive_framework(void);
extern status_t test_run_all_suites(void);
extern status_t test_run_suite_by_type(test_type_t type);
extern status_t test_create_suite(const char* name, test_type_t type);
extern status_t test_add_case(uint32_t suite_id, const char* name, bool (*test_func)(void));
extern status_t test_run_single_case(uint32_t suite_id, uint32_t case_id);

/* Performance Optimization Functions */
extern status_t perf_init_optimization_system(void);
extern status_t perf_run_optimization_suite(void);
extern status_t perf_optimize_boot_time(void);
extern status_t perf_optimize_memory_usage(void);
extern status_t perf_optimize_cpu_scheduling(void);
extern status_t perf_optimize_power_management(void);
extern status_t perf_collect_system_metrics(void);
extern status_t perf_generate_optimization_report(void);

/* Hardware Compatibility Testing Functions */
extern status_t hw_init_compatibility_testing(void);
extern status_t hw_run_compatibility_suite(void);
extern status_t hw_test_laptop_compatibility(void);
extern status_t hw_test_device_detection(void);
extern status_t hw_test_driver_loading(void);
extern status_t hw_generate_compatibility_report(void);

/* Security Testing Functions */
extern status_t test_run_security_audit(void);
extern status_t test_buffer_overflow_protection(void);
extern status_t test_privilege_escalation_protection(void);
extern status_t test_memory_corruption_detection(void);

/* QA Automation Functions */
extern status_t qa_init_automation_system(void);
extern status_t qa_run_full_automation(void);
extern status_t qa_run_ci_pipeline(void);
extern status_t qa_run_regression_testing(void);

/* Test utility functions */
extern bool test_detect_hardware(void);
extern bool test_memory_allocator(void);
extern bool test_filesystem_operations(void);
extern bool test_network_connectivity(void);
extern bool test_security_features(void);

/* String utility */
extern size_t k_strlcpy(char* dst, const char* src, size_t size);

#endif /* TESTING_H */