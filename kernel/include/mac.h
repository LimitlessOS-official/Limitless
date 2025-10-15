#ifndef MAC_H
#define MAC_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_MAC_POLICIES 256
#define MAX_SEC_LEVELS 16
#define MAX_CATEGORIES 64
#define MAX_SIDS 512

// Forward declaration
struct security_context;

// Security Identifier (SID)
typedef uint32_t sid_t;

// Security Context: A string representation of a security identity (e.g., "system:process:init")
typedef struct security_context_string {
    sid_t sid;
    char context[128];
} security_context_string_t;

// Security Level (for MLS)
typedef struct security_level {
    uint8_t sensitivity;
    uint64_t categories; // Bitmask for up to 64 categories
} security_level_t;

// MAC Policy Rule
typedef struct mac_policy_rule {
    sid_t source_sid;
    sid_t target_sid;
    uint16_t object_class;
    uint32_t permissions; // Bitmask of allowed permissions
    bool active;
} mac_policy_rule_t;

// Object classes for MAC
typedef enum {
    SEC_CLASS_PROCESS,
    SEC_CLASS_FILE,
    SEC_CLASS_SOCKET,
    SEC_CLASS_IPC,
    // ... other classes
} security_class_t;

// Core MAC functions
void mac_init(void);
sid_t mac_context_to_sid(const char* context);
const char* mac_sid_to_context(sid_t sid);
int mac_load_policy(const mac_policy_rule_t* rules, uint32_t num_rules);
int mac_compute_avc(sid_t source_sid, sid_t target_sid, security_class_t object_class, uint32_t requested_permission);

#endif // MAC_H
