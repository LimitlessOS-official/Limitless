/*
 * LimitlessOS Device Monitoring API (User-space)
 * Real-time telemetry, predictive analytics, and enterprise integration
 */

#ifndef LIMITLESS_DEVICE_MONITORING_H
#define LIMITLESS_DEVICE_MONITORING_H

#include <stdint.h>
#include <stdbool.h>

#define DEVICE_MONITOR_MAX_DEVICES 1024
#define DEVICE_MONITOR_MAX_STATS   4096

/* Telemetry data structure */
typedef struct {
    uint32_t device_id;
    char name[64];
    uint32_t type;
    uint64_t timestamp;
    uint32_t power_state;
    uint32_t error_count;
    uint32_t temperature_c;
    uint32_t battery_level;
    uint32_t throughput_mbps;
    uint32_t latency_us;
    uint32_t health_score;
    char status[64];
} limitless_device_telemetry_t;

/* Predictive analytics structure */
typedef struct {
    uint32_t device_id;
    char name[64];
    uint32_t predicted_failure;
    uint32_t predicted_maintenance;
    uint32_t anomaly_score;
    char recommendation[128];
} limitless_device_prediction_t;

/* Monitoring API */
int limitless_device_monitoring_init(void);
int limitless_device_monitoring_update(const limitless_device_telemetry_t* telemetry);
int limitless_device_monitoring_get_stats(uint32_t device_id, limitless_device_telemetry_t* stats);
int limitless_device_monitoring_list(limitless_device_telemetry_t* stats, int max_devices);

/* Predictive analytics API */
int limitless_device_predict(uint32_t device_id, limitless_device_prediction_t* prediction);
int limitless_device_predict_all(limitless_device_prediction_t* predictions, int max_devices);

/* Enterprise integration API */
int limitless_device_export_stats(const char* destination);
int limitless_device_export_predictions(const char* destination);

#endif /* LIMITLESS_DEVICE_MONITORING_H */
