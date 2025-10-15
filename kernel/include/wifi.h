/**
 * WiFi Driver Framework Header
 * Complete 802.11a/b/g/n/ac/ax (WiFi 6/6E) support
 */

#pragma once
#include "kernel.h"

#define MAX_WIFI_DEVICES 64

#ifdef __cplusplus
extern "C" {
#endif

/* WiFi Standards */
typedef enum {
    WIFI_STD_80211A = 0,    // 5 GHz, 54 Mbps
    WIFI_STD_80211B,        // 2.4 GHz, 11 Mbps
    WIFI_STD_80211G,        // 2.4 GHz, 54 Mbps
    WIFI_STD_80211N,        // 2.4/5 GHz, 600 Mbps (WiFi 4)
    WIFI_STD_80211AC,       // 5 GHz, 3.5 Gbps (WiFi 5)
    WIFI_STD_80211AX,       // 2.4/5/6 GHz, 9.6 Gbps (WiFi 6/6E)
} wifi_standard_t;

/* WiFi Security Types */
typedef enum {
    WIFI_SEC_NONE = 0,
    WIFI_SEC_WEP,
    WIFI_SEC_WPA_PSK,
    WIFI_SEC_WPA2_PSK,
    WIFI_SEC_WPA3_PSK,
    WIFI_SEC_WPA2_ENTERPRISE,
    WIFI_SEC_WPA3_ENTERPRISE,
    WIFI_SEC_WPS_PIN,
    WIFI_SEC_WPS_PBC,
} wifi_security_t;

/* WiFi Frequency Bands */
typedef enum {
    WIFI_BAND_2GHZ = 0,
    WIFI_BAND_5GHZ,
    WIFI_BAND_6GHZ,
} wifi_band_t;

/* WiFi Connection State */
typedef enum {
    WIFI_STATE_DISCONNECTED = 0,
    WIFI_STATE_SCANNING,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_AUTHENTICATING,
    WIFI_STATE_ASSOCIATING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_DISCONNECTING,
    WIFI_STATE_ROAMING,
    WIFI_STATE_ERROR,
} wifi_state_t;

/* WiFi Power Management */
typedef enum {
    WIFI_PM_ACTIVE = 0,         // Full power mode
    WIFI_PM_POWER_SAVE,         // Basic power save
    WIFI_PM_DEEP_SLEEP,         // Deep sleep mode
    WIFI_PM_ULTRA_LOW_POWER,    // Ultra low power
} wifi_power_mode_t;

/* WiFi Statistics */
typedef struct wifi_stats {
    uint64_t tx_packets;
    uint64_t rx_packets;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    uint32_t tx_errors;
    uint32_t rx_errors;
    uint32_t dropped_packets;
    int8_t signal_strength;    // RSSI in dBm
} wifi_stats_t;

/* WiFi Channel Information */
typedef struct wifi_channel {
    uint8_t number;
    uint16_t frequency;         // MHz
    wifi_band_t band;
    bool disabled;
    bool radar_detection;       // DFS channel
    int8_t max_power;          // dBm
} wifi_channel_t;

/* WiFi Network (SSID) */
typedef struct wifi_network {
    char ssid[33];              // Network name (max 32 chars + null)
    uint8_t bssid[6];           // MAC address of AP
    wifi_channel_t channel;     // Channel info
    int8_t rssi;                // Signal strength (dBm)
    wifi_security_t security;   // Security type
    wifi_standard_t standard;   // WiFi standard
    bool hidden;                // Hidden SSID
    uint16_t beacon_interval;   // Beacon interval (ms)
    uint64_t timestamp;         // Last seen timestamp
    uint8_t* ie_data;          // Information Elements
    uint16_t ie_length;        // IE data length
} wifi_network_t;

/* WiFi Capabilities */
typedef struct wifi_capabilities {
    wifi_standard_t max_standard;
    bool dual_band;             // 2.4 + 5 GHz
    bool tri_band;              // 2.4 + 5 + 6 GHz
    uint8_t max_spatial_streams; // MIMO streams
    bool mu_mimo;               // Multi-user MIMO
    bool beamforming;           // Beamforming support
    bool ldpc;                  // LDPC error correction
    bool short_gi;              // Short guard interval
    bool he_support;            // 802.11ax (WiFi 6)
    bool wpa3_support;          // WPA3 support
    bool ofdma;                 // OFDMA (WiFi 6)
    uint16_t max_ampdu_length;  // Max A-MPDU length
    uint16_t max_amsdu_length;  // Max A-MSDU length
} wifi_capabilities_t;

/* WiFi Firmware */
typedef struct wifi_firmware {
    const char* name;
    const uint8_t* data;
    uint32_t size;
    uint32_t version;
    bool loaded;
} wifi_firmware_t;

/* Forward declaration */
struct wifi_device;

/* WiFi Device */
typedef struct wifi_device {
    uint32_t id;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t revision_id;
    char name[64];
    char driver_name[32];
    uint8_t mac_address[6];
    
    /* Hardware info */
    uint32_t pci_bus;
    uint32_t pci_device;
    uint32_t pci_function;
    vaddr_t mmio_base;
    uint32_t mmio_size;
    uint32_t irq_line;
    
    /* Capabilities */
    wifi_capabilities_t capabilities;
    
    /* Current configuration */
    wifi_power_mode_t power_mode;
    bool regulatory_domain_set;
    char country_code[3];
    
    /* State */
    wifi_state_t state;
    wifi_network_t* connected_network;
    bool enabled;
    bool initialized;
    
    /* Scan results */
    wifi_network_t* scan_results;
    uint32_t scan_count;
    uint32_t max_scan_results;
    
    /* Supported channels */
    wifi_channel_t* channels;
    uint32_t channel_count;
    
    /* Firmware */
    wifi_firmware_t firmware;
    
    /* Statistics */
    uint64_t tx_packets;
    uint64_t rx_packets;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    uint64_t tx_errors;
    uint64_t rx_errors;
    uint64_t beacon_loss_count;
    uint64_t retry_count;
    
    /* Performance metrics */
    uint32_t link_quality;      // 0-100%
    int8_t noise_level;         // dBm
    uint32_t tx_bitrate;        // Mbps
    uint32_t rx_bitrate;        // Mbps
    
    /* Driver callbacks */
    status_t (*init)(struct wifi_device*);
    status_t (*shutdown)(struct wifi_device*);
    status_t (*scan)(struct wifi_device*);
    status_t (*connect)(struct wifi_device*, wifi_network_t*, const char* password);
    status_t (*disconnect)(struct wifi_device*);
    status_t (*send)(struct wifi_device*, void* packet, uint32_t length);
    status_t (*set_power_mode)(struct wifi_device*, wifi_power_mode_t mode);
    status_t (*set_channel)(struct wifi_device*, wifi_channel_t* channel);
    status_t (*get_signal_info)(struct wifi_device*, int8_t* rssi, uint32_t* quality);
    void (*irq_handler)(struct wifi_device*);
    
    /* Lock for thread safety */
    spinlock_t lock;
} wifi_device_t;

/* Core WiFi API */
status_t wifi_init(void);
status_t wifi_register_device(wifi_device_t* device);

/* Device management */
wifi_device_t* wifi_get_primary(void);
wifi_device_t* wifi_get_device(uint32_t index);
uint32_t wifi_get_count(void);

/* Network operations */
status_t wifi_scan(wifi_device_t* device);
status_t wifi_connect(wifi_device_t* device, const char* ssid, const char* password, wifi_security_t security);
status_t wifi_connect_secure(wifi_device_t* device, const char* ssid, 
                           wifi_security_t security, const char* password);
status_t wifi_disconnect(wifi_device_t* device);

/* Data transmission */
status_t wifi_send(wifi_device_t* device, void* packet, uint32_t length);

/* Configuration */
status_t wifi_set_power_mode(wifi_device_t* device, wifi_power_mode_t mode);
status_t wifi_get_signal_info(wifi_device_t* device, int8_t* rssi, uint32_t* quality);
status_t wifi_get_stats(wifi_device_t* device, wifi_stats_t* stats);

/* Internal functions */
status_t wifi_setup_channels(wifi_device_t* device);

/* Generic WiFi driver functions */
status_t generic_wifi_shutdown(wifi_device_t* device);
status_t generic_wifi_scan(wifi_device_t* device);
status_t generic_wifi_connect(wifi_device_t* device, wifi_network_t* network, const char* password);
status_t generic_wifi_disconnect(wifi_device_t* device);
status_t generic_wifi_send(wifi_device_t* device, void* packet, uint32_t length);
status_t generic_wifi_set_power_mode(wifi_device_t* device, wifi_power_mode_t mode);
status_t generic_wifi_set_channel(wifi_device_t* device, wifi_channel_t* channel);
status_t generic_wifi_get_signal_info(wifi_device_t* device, int8_t* rssi, uint32_t* quality);

/* Vendor-specific driver initialization */
status_t intel_wifi_init(wifi_device_t* device);
status_t realtek_wifi_init(wifi_device_t* device);
status_t broadcom_wifi_init(wifi_device_t* device);
status_t atheros_wifi_init(wifi_device_t* device);
status_t mediatek_wifi_init(wifi_device_t* device);
status_t marvell_wifi_init(wifi_device_t* device);

#ifdef __cplusplus
}
#endif