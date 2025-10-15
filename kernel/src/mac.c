#include "mac.h"
#include <string.h>
#include <stddef.h>

// --- Data Structures ---

// Storage for security context strings and their corresponding SIDs
static security_context_string_t sid_map[MAX_SIDS];
static uint32_t next_sid = 1; // SID 0 is reserved for "unlabeled"

// The Access Vector Cache (AVC) - stores policy rules for fast lookup
// A real AVC is a complex hash table. We'll use a simple array for this simulation.
static mac_policy_rule_t avc[MAX_MAC_POLICIES];
static uint32_t avc_rule_count = 0;

// --- Core Functions ---

/**
 * @brief Initializes the MAC framework.
 */
void mac_init(void) {
    memset(sid_map, 0, sizeof(sid_map));
    memset(avc, 0, sizeof(avc));
    next_sid = 1;
    avc_rule_count = 0;

    // Create the initial "unlabeled" SID
    sid_map[0].sid = 0;
    strcpy(sid_map[0].context, "unlabeled");
}

/**
 * @brief Converts a security context string to a SID.
 *        If the context doesn't exist, a new SID is created.
 * @param context The security context string (e.g., "system:process:init").
 * @return The corresponding SID, or 0 if the table is full.
 */
sid_t mac_context_to_sid(const char* context) {
    // First, search if the context already exists
    for (uint32_t i = 0; i < next_sid; ++i) {
        if (strcmp(sid_map[i].context, context) == 0) {
            return sid_map[i].sid;
        }
    }

    // If not found, create a new entry
    if (next_sid >= MAX_SIDS) {
        return 0; // SID table full, cannot create new context
    }

    sid_map[next_sid].sid = next_sid;
    strncpy(sid_map[next_sid].context, context, sizeof(sid_map[next_sid].context) - 1);
    
    return next_sid++;
}

/**
 * @brief Converts a SID back to its string representation.
 * @param sid The SID to convert.
 * @return A pointer to the security context string, or "invalid" if not found.
 */
const char* mac_sid_to_context(sid_t sid) {
    if (sid < next_sid) {
        return sid_map[sid].context;
    }
    return "invalid";
}

/**
 * @brief Loads a set of policy rules into the kernel's AVC.
 * @param rules An array of policy rules.
 * @param num_rules The number of rules in the array.
 * @return 0 on success, -1 on failure (e.g., AVC is full).
 */
int mac_load_policy(const mac_policy_rule_t* rules, uint32_t num_rules) {
    if (!rules || num_rules == 0) {
        return -1;
    }

    for (uint32_t i = 0; i < num_rules; ++i) {
        if (avc_rule_count >= MAX_MAC_POLICIES) {
            return -1; // AVC full
        }
        avc[avc_rule_count] = rules[i];
        avc[avc_rule_count].active = true;
        avc_rule_count++;
    }

    return 0;
}

/**
 * @brief Computes an access decision based on the loaded policy.
 *        This simulates a lookup in the Access Vector Cache (AVC).
 * @param source_sid The SID of the subject (e.g., a process).
 * @param target_sid The SID of the object (e.g., a file).
 * @param object_class The class of the object.
 * @param requested_permission The permission being requested.
 * @return 0 if access is granted, -1 if denied.
 */
int mac_compute_avc(sid_t source_sid, sid_t target_sid, security_class_t object_class, uint32_t requested_permission) {
    // In a real system, this would be a highly optimized hash table lookup.
    // Here, we do a linear scan of the rules.
    for (uint32_t i = 0; i < avc_rule_count; ++i) {
        if (avc[i].active &&
            avc[i].source_sid == source_sid &&
            avc[i].target_sid == target_sid &&
            avc[i].object_class == object_class) {
            
            // Rule matches. Check if the requested permission is allowed by the rule's permission bitmask.
            if ((avc[i].permissions & requested_permission) == requested_permission) {
                return 0; // Access granted
            }
        }
    }

    // If no specific rule is found, access is denied by default.
    return -1; // Access denied
}
