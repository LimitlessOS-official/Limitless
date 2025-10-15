/*
 * LimitlessOS Mobile & IoT Integration Header
 * Device connectivity, edge computing support, and seamless ecosystem synchronization
 */

#ifndef MOBILE_IOT_INTEGRATION_H
#define MOBILE_IOT_INTEGRATION_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <netinet/in.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

/* Maximum limits */
#define MAX_MOBILE_DEVICES 50
#define MAX_IOT_DEVICES 500
#define MAX_EDGE_NODES 100
#define MAX_SYNC_SESSIONS 200
#define MAX_PROTOCOLS 20
#define MAX_MESH_NODES 1000
#define MAX_DEVICE_GROUPS 100
#define MAX_DISCOVERY_SERVICES 50

/* Device types */
typedef enum {
    DEVICE_SMARTPHONE = 0,
    DEVICE_TABLET,
    DEVICE_LAPTOP,
    DEVICE_DESKTOP,
    DEVICE_SMARTWATCH,
    DEVICE_FITNESS_TRACKER,
    DEVICE_SMART_TV,
    DEVICE_SMART_SPEAKER,
    DEVICE_SMART_HOME_HUB,
    DEVICE_SMART_CAMERA,
    DEVICE_SMART_DOORBELL,
    DEVICE_SMART_THERMOSTAT,
    DEVICE_SMART_LIGHT,
    DEVICE_SMART_LOCK,
    DEVICE_SMART_SENSOR,
    DEVICE_IOT_GATEWAY,
    DEVICE_EDGE_COMPUTE,
    DEVICE_DRONE,
    DEVICE_VEHICLE,
    DEVICE_INDUSTRIAL_IOT,
    DEVICE_MEDICAL_DEVICE,
    DEVICE_WEARABLE,
    DEVICE_APPLIANCE,
    DEVICE_ROUTER,
    DEVICE_ACCESS_POINT,
    DEVICE_MAX
} device_type_t;

/* Connection protocols */
typedef enum {
    PROTOCOL_WIFI = 0,
    PROTOCOL_BLUETOOTH,
    PROTOCOL_ZIGBEE,
    PROTOCOL_ZWAVE,
    PROTOCOL_THREAD,
    PROTOCOL_MATTER,
    PROTOCOL_LORA,
    PROTOCOL_NFC,
    PROTOCOL_USB,
    PROTOCOL_ETHERNET,
    PROTOCOL_CELLULAR_4G,
    PROTOCOL_CELLULAR_5G,
    PROTOCOL_SATELLITE,
    PROTOCOL_MESH,
    PROTOCOL_PROPRIETARY,
    PROTOCOL_MAX
} connection_protocol_t;

/* Device capabilities */
typedef enum {
    CAP_VOICE_CONTROL = 0x01,
    CAP_TOUCH_INPUT = 0x02,
    CAP_DISPLAY_OUTPUT = 0x04,
    CAP_AUDIO_PLAYBACK = 0x08,
    CAP_VIDEO_CAPTURE = 0x10,
    CAP_SENSORS = 0x20,
    CAP_ACTUATORS = 0x40,
    CAP_GPS = 0x80,
    CAP_ACCELEROMETER = 0x100,
    CAP_GYROSCOPE = 0x200,
    CAP_MAGNETOMETER = 0x400,
    CAP_HEART_RATE = 0x800,
    CAP_TEMPERATURE = 0x1000,
    CAP_HUMIDITY = 0x2000,
    CAP_PRESSURE = 0x4000,
    CAP_LIGHT_SENSOR = 0x8000,
    CAP_PROXIMITY = 0x10000,
    CAP_BIOMETRIC = 0x20000,
    CAP_SECURE_ELEMENT = 0x40000,
    CAP_EDGE_COMPUTING = 0x80000
} device_capabilities_t;

/* Security levels */
typedef enum {
    SECURITY_NONE = 0,
    SECURITY_BASIC,
    SECURITY_ENHANCED,
    SECURITY_ENTERPRISE,
    SECURITY_MILITARY,
    SECURITY_MAX
} security_level_t;

/* Synchronization types */
typedef enum {
    SYNC_FILES = 0,
    SYNC_CONTACTS,
    SYNC_CALENDAR,
    SYNC_PHOTOS,
    SYNC_MUSIC,
    SYNC_VIDEOS,
    SYNC_DOCUMENTS,
    SYNC_SETTINGS,
    SYNC_PASSWORDS,
    SYNC_BOOKMARKS,
    SYNC_NOTES,
    SYNC_TASKS,
    SYNC_HEALTH_DATA,
    SYNC_LOCATION_DATA,
    SYNC_APP_DATA,
    SYNC_CUSTOM,
    SYNC_MAX
} sync_type_t;

/* Edge computing capabilities */
typedef enum {
    EDGE_INFERENCE = 0x01,
    EDGE_TRAINING = 0x02,
    EDGE_STORAGE = 0x04,
    EDGE_NETWORKING = 0x08,
    EDGE_ANALYTICS = 0x10,
    EDGE_CACHING = 0x20,
    EDGE_LOAD_BALANCING = 0x40,
    EDGE_SECURITY = 0x80,
    EDGE_ORCHESTRATION = 0x100
} edge_capabilities_t;

/* Device information */
typedef struct {
    uint32_t device_id;
    char name[128];
    char manufacturer[64];
    char model[64];
    char serial_number[64];
    char firmware_version[32];
    char hardware_version[32];
    
    device_type_t type;
    connection_protocol_t primary_protocol;
    connection_protocol_t protocols[MAX_PROTOCOLS];
    uint32_t protocol_count;
    
    /* Network information */
    char mac_address[18];
    char ip_address[46]; /* IPv6 compatible */
    uint16_t port;
    char hostname[256];
    
    /* Capabilities */
    device_capabilities_t capabilities;
    security_level_t security_level;
    edge_capabilities_t edge_caps;
    
    /* Status */
    bool connected;
    bool authenticated;
    bool trusted;
    time_t last_seen;
    time_t first_discovered;
    
    /* Performance metrics */
    uint32_t latency_ms;
    uint32_t bandwidth_kbps;
    uint8_t signal_strength; /* 0-100 */
    uint8_t battery_level;   /* 0-100, 255 if N/A */
    
    /* Location */
    double latitude;
    double longitude;
    double altitude;
    float accuracy_meters;
    
    /* Power management */
    bool low_power_mode;
    uint32_t sleep_interval; /* seconds */
    time_t next_wake;
    
    pthread_mutex_t lock;
} mobile_iot_device_t;

/* Device group */
typedef struct {
    uint32_t group_id;
    char name[128];
    char description[256];
    
    uint32_t device_ids[50];
    uint32_t device_count;
    
    /* Group settings */
    bool auto_discovery;
    security_level_t min_security;
    uint32_t sync_interval; /* seconds */
    
    /* Synchronization settings */
    sync_type_t sync_types[SYNC_MAX];
    uint32_t sync_type_count;
    bool bidirectional_sync;
    bool conflict_resolution;
    
    time_t created;
    time_t last_updated;
    
    pthread_mutex_t lock;
} device_group_t;

/* Edge compute node */
typedef struct {
    uint32_t node_id;
    char name[128];
    char location[256];
    
    /* Hardware specs */
    uint32_t cpu_cores;
    uint64_t ram_mb;
    uint64_t storage_gb;
    uint32_t gpu_units;
    
    /* Capabilities */
    edge_capabilities_t capabilities;
    uint32_t max_containers;
    uint32_t max_functions;
    
    /* Current state */
    uint32_t active_containers;
    uint32_t active_functions;
    uint32_t cpu_usage;      /* 0-100 */
    uint32_t memory_usage;   /* 0-100 */
    uint32_t storage_usage;  /* 0-100 */
    
    /* Network */
    char management_ip[46];
    uint16_t management_port;
    uint32_t bandwidth_mbps;
    
    /* Status */
    bool online;
    time_t last_heartbeat;
    uint32_t uptime_seconds;
    
    pthread_mutex_t lock;
} edge_node_t;

/* Synchronization session */
typedef struct {
    uint32_t session_id;
    uint32_t source_device_id;
    uint32_t target_device_id;
    sync_type_t sync_type;
    
    /* Session state */
    bool active;
    time_t started;
    time_t last_activity;
    
    /* Progress */
    uint64_t total_items;
    uint64_t synced_items;
    uint64_t failed_items;
    uint64_t bytes_transferred;
    
    /* Status */
    char status_message[256];
    uint32_t error_count;
    bool completed;
    
    pthread_mutex_t lock;
} sync_session_t;

/* Mesh network node */
typedef struct {
    uint32_t node_id;
    char node_address[64];
    
    /* Mesh information */
    uint32_t parent_id;
    uint32_t children[20];
    uint32_t child_count;
    uint32_t hop_count;
    
    /* Routing */
    uint32_t routing_table[100];
    uint32_t route_count;
    
    /* Performance */
    uint8_t link_quality;
    uint32_t packet_loss;
    uint32_t throughput;
    
    time_t last_update;
    bool active;
} mesh_node_t;

/* Discovery service */
typedef struct {
    uint32_t service_id;
    char service_name[64];
    char service_type[32];
    uint16_t port;
    
    /* Discovery method */
    connection_protocol_t protocol;
    char discovery_data[512];
    
    /* Callbacks */
    void (*device_discovered)(mobile_iot_device_t *device);
    void (*device_lost)(uint32_t device_id);
    
    bool enabled;
    time_t last_scan;
    uint32_t scan_interval;
    
    pthread_mutex_t lock;
} discovery_service_t;

/* Ecosystem configuration */
typedef struct {
    /* Discovery settings */
    bool auto_discovery;
    uint32_t discovery_interval; /* seconds */
    bool bluetooth_discovery;
    bool wifi_discovery;
    bool upnp_discovery;
    bool mdns_discovery;
    
    /* Security settings */
    security_level_t default_security;
    bool require_authentication;
    bool require_encryption;
    char security_key[256];
    
    /* Synchronization settings */
    bool auto_sync;
    uint32_t sync_interval; /* seconds */
    bool sync_on_connection;
    bool sync_on_change;
    uint64_t max_sync_size; /* bytes */
    
    /* Edge computing settings */
    bool edge_enabled;
    uint32_t edge_heartbeat_interval;
    uint32_t edge_health_check_interval;
    char edge_registry_url[256];
    
    /* Mesh networking */
    bool mesh_enabled;
    char mesh_network_id[64];
    uint32_t mesh_channel;
    uint32_t mesh_max_hops;
    
    /* Performance settings */
    uint32_t connection_timeout; /* seconds */
    uint32_t retry_count;
    uint32_t max_concurrent_syncs;
    uint64_t bandwidth_limit; /* bytes/sec */
    
} ecosystem_config_t;

/* Main mobile & IoT system */
typedef struct {
    /* Device registry */
    mobile_iot_device_t devices[MAX_MOBILE_DEVICES + MAX_IOT_DEVICES];
    uint32_t device_count;
    
    device_group_t groups[MAX_DEVICE_GROUPS];
    uint32_t group_count;
    
    /* Edge computing */
    edge_node_t edge_nodes[MAX_EDGE_NODES];
    uint32_t edge_node_count;
    
    /* Synchronization */
    sync_session_t sync_sessions[MAX_SYNC_SESSIONS];
    uint32_t session_count;
    
    /* Mesh networking */
    mesh_node_t mesh_nodes[MAX_MESH_NODES];
    uint32_t mesh_node_count;
    uint32_t local_node_id;
    
    /* Discovery services */
    discovery_service_t discovery_services[MAX_DISCOVERY_SERVICES];
    uint32_t discovery_service_count;
    
    /* Configuration */
    ecosystem_config_t config;
    
    /* Threading */
    pthread_t discovery_thread;
    pthread_t sync_thread;
    pthread_t edge_thread;
    pthread_t mesh_thread;
    pthread_t maintenance_thread;
    bool threads_running;
    
    /* Synchronization */
    pthread_mutex_t system_lock;
    pthread_cond_t sync_available;
    
    /* Statistics */
    uint64_t devices_discovered;
    uint64_t sync_sessions_completed;
    uint64_t bytes_synchronized;
    uint64_t edge_tasks_processed;
    time_t start_time;
    
    bool initialized;
} mobile_iot_system_t;

/* Core functions */
int mobile_iot_system_init(void);
int mobile_iot_system_cleanup(void);
int mobile_iot_load_config(const char *config_file);

/* Device management */
int device_register(const mobile_iot_device_t *device_info);
int device_unregister(uint32_t device_id);
int device_update_status(uint32_t device_id, bool connected);
int device_authenticate(uint32_t device_id, const char *credentials);
int device_get_info(uint32_t device_id, mobile_iot_device_t *device_info);
int device_enumerate(mobile_iot_device_t *devices, uint32_t max_devices, uint32_t *count);

/* Device groups */
int device_group_create(const char *name, const char *description);
int device_group_add_device(uint32_t group_id, uint32_t device_id);
int device_group_remove_device(uint32_t group_id, uint32_t device_id);
int device_group_configure_sync(uint32_t group_id, sync_type_t sync_types[], uint32_t count);

/* Discovery */
int discovery_start_scan(connection_protocol_t protocol);
int discovery_stop_scan(connection_protocol_t protocol);
int discovery_register_callback(void (*device_found)(mobile_iot_device_t *device));
int discovery_configure_service(const char *service_name, connection_protocol_t protocol);

/* Synchronization */
int sync_start_session(uint32_t source_device, uint32_t target_device, sync_type_t type);
int sync_stop_session(uint32_t session_id);
int sync_get_progress(uint32_t session_id, uint64_t *total, uint64_t *completed);
int sync_configure_automatic(uint32_t device_id, sync_type_t types[], uint32_t count);

/* Edge computing */
int edge_node_register(const char *name, const char *management_ip, uint16_t port);
int edge_node_unregister(uint32_t node_id);
int edge_deploy_function(uint32_t node_id, const char *function_name, const char *code);
int edge_execute_function(uint32_t node_id, const char *function_name, const char *input);
int edge_get_node_status(uint32_t node_id, edge_node_t *status);

/* Mesh networking */
int mesh_initialize_network(const char *network_id, uint32_t channel);
int mesh_join_network(const char *network_id, const char *parent_address);
int mesh_leave_network(void);
int mesh_send_message(uint32_t target_node, const void *data, size_t size);
int mesh_broadcast_message(const void *data, size_t size);

/* Security */
int security_pair_device(uint32_t device_id, const char *pin);
int security_trust_device(uint32_t device_id, bool trusted);
int security_generate_key_pair(char *public_key, char *private_key);
int security_encrypt_data(const void *data, size_t size, const char *key, void *encrypted);

/* Ecosystem management */
int ecosystem_get_status(char *status_json, size_t max_size);
int ecosystem_optimize_connections(void);
int ecosystem_balance_load(void);
int ecosystem_migrate_services(uint32_t from_node, uint32_t to_node);

/* Data synchronization */
int sync_files(uint32_t device_id, const char *source_path, const char *dest_path);
int sync_contacts(uint32_t device_id, bool upload, bool download);
int sync_calendar(uint32_t device_id, const char *calendar_name);
int sync_photos(uint32_t device_id, const char *album_name);
int sync_settings(uint32_t device_id, const char *app_name);

/* Remote control */
int remote_execute_command(uint32_t device_id, const char *command);
int remote_transfer_file(uint32_t device_id, const char *local_path, const char *remote_path);
int remote_stream_screen(uint32_t device_id, const char *stream_url);
int remote_control_input(uint32_t device_id, const char *input_type, const char *data);

/* Analytics */
int analytics_device_usage(uint32_t device_id, char *report, size_t max_size);
int analytics_sync_performance(uint32_t session_id, char *metrics, size_t max_size);
int analytics_edge_utilization(char *report, size_t max_size);
int analytics_mesh_topology(char *topology, size_t max_size);

/* Utility functions */
const char *device_type_name(device_type_t type);
const char *protocol_name(connection_protocol_t protocol);
const char *security_level_name(security_level_t level);
const char *sync_type_name(sync_type_t type);
bool device_has_capability(const mobile_iot_device_t *device, device_capabilities_t capability);

#endif /* MOBILE_IOT_INTEGRATION_H */