#include "capability.h"
#include <stddef.h>
#include <string.h>

// --- Data Structures ---

// For simplicity, we'll have a static array to store capability sets for each security context.
// A real implementation might store this within the process's credential struct.
#define MAX_SECURITY_CONTEXTS 1024 // Should match the definition in security_framework.c
static capability_set_t context_capabilities[MAX_SECURITY_CONTEXTS];

// The root/kernel capability set, possessing all capabilities.
static capability_set_t root_caps;

// --- Core Functions ---

/**
 * @brief Initializes the capability subsystem.
 */
void capability_init(void) {
    // Clear all existing capability sets
    memset(context_capabilities, 0, sizeof(context_capabilities));

    // Initialize the root capability set to have all capabilities in all sets.
    // A 64-bit value of all 1s.
    cap_mask_t all_caps = ~((cap_mask_t)0);

    root_caps.permitted = all_caps;
    root_caps.effective = all_caps;
    root_caps.inheritable = all_caps;

    // The kernel's context (ID 1) gets the root capabilities.
    // Note: context_id 0 is invalid.
    if (MAX_SECURITY_CONTEXTS > 1) {
        context_capabilities[1] = root_caps;
    }
}

/**
 * @brief Retrieves the capability set for a given security context.
 * @param context_id The ID of the security context.
 * @return A pointer to the capability set, or NULL if the ID is invalid.
 */
capability_set_t* capability_get(uint32_t context_id) {
    if (context_id == 0 || context_id >= MAX_SECURITY_CONTEXTS) {
        return NULL;
    }
    return &context_capabilities[context_id];
}

/**
 * @brief Sets the capability set for a given security context.
 * @param context_id The ID of the security context.
 * @param caps The new capability set to apply.
 * @return 0 on success, -1 on failure.
 */
int capability_set(uint32_t context_id, const capability_set_t* caps) {
    if (context_id == 0 || context_id >= MAX_SECURITY_CONTEXTS || !caps) {
        return -1;
    }
    context_capabilities[context_id] = *caps;
    return 0;
}

/**
 * @brief Checks if a security context has a specific capability in its effective set.
 * @param context_id The ID of the security context.
 * @param cap The capability to check for.
 * @return true if the capability is present, false otherwise.
 */
bool has_capability(uint32_t context_id, security_capability_t cap) {
    if (context_id == 0 || context_id >= MAX_SECURITY_CONTEXTS || cap >= CAP_LAST_CAP) {
        return false;
    }
    cap_mask_t cap_bit = (cap_mask_t)1 << cap;
    return (context_capabilities[context_id].effective & cap_bit) != 0;
}

/**
 * @brief Raises a capability in the effective set.
 *        The capability must be present in the permitted set.
 * @param context_id The ID of the security context.
 * @param cap The capability to raise.
 * @return 0 on success, -1 on failure.
 */
int capability_raise(uint32_t context_id, security_capability_t cap) {
    if (context_id == 0 || context_id >= MAX_SECURITY_CONTEXTS || cap >= CAP_LAST_CAP) {
        return -1;
    }
    cap_mask_t cap_bit = (cap_mask_t)1 << cap;
    // Check if the capability is in the permitted set before raising it
    if ((context_capabilities[context_id].permitted & cap_bit) != 0) {
        context_capabilities[context_id].effective |= cap_bit;
        return 0;
    }
    return -1; // Not permitted
}

/**
 * @brief Lowers a capability from the effective set.
 * @param context_id The ID of the security context.
 * @param cap The capability to lower.
 * @return 0 on success, -1 on failure.
 */
int capability_lower(uint32_t context_id, security_capability_t cap) {
    if (context_id == 0 || context_id >= MAX_SECURITY_CONTEXTS || cap >= CAP_LAST_CAP) {
        return -1;
    }
    cap_mask_t cap_bit = (cap_mask_t)1 << cap;
    context_capabilities[context_id].effective &= ~cap_bit;
    return 0;
}
