# LimitlessOS Device API & Monitoring

## Overview
LimitlessOS provides enterprise-grade device management, monitoring, and analytics APIs for user-space applications. These APIs enable persistent device registry, event notification, state recovery, real-time telemetry, predictive analytics, and integration with enterprise monitoring systems.

## Key APIs
- **Device Registry**: Persistent device info, state recovery, enumeration
- **Event Notification**: Subscribe to device events (add/remove/change/power/error)
- **Monitoring**: Real-time telemetry (power, health, throughput, latency, temperature, battery)
- **Predictive Analytics**: Failure prediction, maintenance scheduling, anomaly detection
- **Enterprise Integration**: Export stats and predictions to external systems

## Example Usage
```c
#include "limitless_device_api.h"
#include "limitless_device_monitoring.h"

// Initialize registry and monitoring
limitless_device_registry_init();
limitless_device_monitoring_init();

// List all devices
limitless_device_info_t devices[128];
int count = limitless_device_registry_list(devices, 128);
for (int i = 0; i < count; i++) {
    printf("Device: %s (ID: %u)\n", devices[i].name, devices[i].device_id);
}

// Subscribe to device events
void on_device_event(const limitless_device_event_t* event) {
    printf("Device event: %s (Type: %d)\n", event->device_name, event->type);
}
limitless_device_event_subscribe(on_device_event);

// Get telemetry and prediction
limitless_device_telemetry_t stats;
limitless_device_monitoring_get_stats(devices[0].device_id, &stats);
limitless_device_prediction_t prediction;
limitless_device_predict(devices[0].device_id, &prediction);
printf("Prediction: %s\n", prediction.recommendation);
```

## API Reference
See `limitless_device_api.h` and `limitless_device_monitoring.h` for full function documentation.

## Integration
- Export stats/predictions to enterprise systems via `limitless_device_export_stats` and `limitless_device_export_predictions`.
- Extend predictive logic for custom analytics.

## Support
For more information, see the LimitlessOS developer documentation or contact enterprise support.
