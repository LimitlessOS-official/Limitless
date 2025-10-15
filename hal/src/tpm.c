#include "hal/include/tpm.h"
#include <string.h> // For memcpy

// Simulated TPM MMIO space
static uint8_t tpm_mmio_space[4096];
static tpm_driver_state_t tpm_state;

// --- Low-level MMIO access functions ---
static uint8_t tpm_read8(uint32_t offset) {
    return tpm_mmio_space[offset];
}

static uint32_t tpm_read32(uint32_t offset) {
    return *(uint32_t*)&tpm_mmio_space[offset];
}

static void tpm_write8(uint32_t offset, uint8_t value) {
    tpm_mmio_space[offset] = value;
}

static void tpm_write32(uint32_t offset, uint32_t value) {
    *(uint32_t*)&tpm_mmio_space[offset] = value;
}

// --- TPM Locality Management ---
static bool tpm_request_locality(int locality) {
    if (tpm_state.locality != (uint8_t)-1 && tpm_state.locality != locality) {
        return false; // Already owned by another locality
    }
    tpm_write8(TPM_ACCESS_REG, TPM_ACCESS_REQUEST_USE);
    uint8_t access = tpm_read8(TPM_ACCESS_REG);
    if ((access & (TPM_ACCESS_VALID | TPM_ACCESS_ACTIVE_LOCALITY)) == (TPM_ACCESS_VALID | TPM_ACCESS_ACTIVE_LOCALITY)) {
        tpm_state.locality = locality;
        return true;
    }
    return false;
}

static void tpm_release_locality() {
    if (tpm_state.locality != (uint8_t)-1) {
        tpm_write8(TPM_ACCESS_REG, TPM_ACCESS_ACTIVE_LOCALITY);
        tpm_state.locality = (uint8_t)-1;
    }
}

// --- Core Driver Functions ---

/**
 * @brief Detects the presence of a TPM 2.0 device.
 * @return True if a TPM is detected, false otherwise.
 */
bool tpm_detect(void) {
    // Simulate finding a TPM by writing a dummy DID_VID
    *(uint32_t*)&tpm_mmio_space[TPM_DID_VID_REG] = 0x1AE015D1; // QEMU TPM

    uint32_t did_vid = tpm_read32(TPM_DID_VID_REG);
    if (did_vid == 0xFFFFFFFF || did_vid == 0) {
        tpm_state.is_present = false;
        return false;
    }
    
    tpm_state.vendor_id = did_vid & 0xFFFF;
    tpm_state.device_id = did_vid >> 16;
    tpm_state.is_present = true;
    tpm_state.is_initialized = false;
    tpm_state.locality = (uint8_t)-1;
    
    return true;
}

/**
 * @brief Initializes the TPM device.
 * @return 0 on success, -1 on failure.
 */
int tpm_init(void) {
    if (!tpm_state.is_present) {
        if (!tpm_detect()) {
            return -1;
        }
    }

    if (!tpm_request_locality(0)) {
        return -1;
    }

    // Set TPM to ready state
    tpm_write32(TPM_STS_REG, TPM_STS_COMMAND_READY);

    if (tpm_startup() != 0) {
        tpm_release_locality();
        return -1;
    }

    if (tpm_self_test() != 0) {
        tpm_release_locality();
        return -1;
    }

    tpm_state.is_initialized = true;
    return 0;
}

/**
 * @brief Transmits a command to the TPM and receives a response.
 * @param cmd_buf Buffer containing the command.
 * @param cmd_size Size of the command.
 * @param res_buf Buffer to store the response.
 * @param res_size Pointer to store the size of the response.
 * @return 0 on success, -1 on failure.
 */
int tpm_transmit(const uint8_t* cmd_buf, uint32_t cmd_size, uint8_t* res_buf, uint32_t* res_size) {
    if (!tpm_state.is_initialized || tpm_state.locality == (uint8_t)-1) {
        return -1;
    }

    // Simplified FIFO write
    for (uint32_t i = 0; i < cmd_size; ++i) {
        tpm_write8(TPM_DATA_FIFO_REG + i, cmd_buf[i]);
    }

    // Tell TPM command is ready
    tpm_write32(TPM_STS_REG, TPM_STS_GO);

    // --- Simulation of TPM processing ---
    // A real driver would poll the status register. Here we'll process the command directly.
    tpm2_header_t* cmd_hdr = (tpm2_header_t*)cmd_buf;
    tpm2_header_t* res_hdr = (tpm2_header_t*)res_buf;
    
    // Default success response
    res_hdr->tag = TPM2_ST_NO_SESSIONS;
    res_hdr->command_code = 0; // Success
    res_hdr->length = sizeof(tpm2_header_t);

    // Simulate specific command responses
    if (cmd_hdr->command_code == TPM2_CC_STARTUP) {
        // No extra data for startup success
    } else if (cmd_hdr->command_code == TPM2_CC_SELF_TEST) {
        // No extra data for self-test success
    } else if (cmd_hdr->command_code == TPM2_CC_PCR_EXTEND) {
        res_hdr->length += 5; // updateCount + pcrUpdateCounter
        // Dummy data for response
        res_buf[sizeof(tpm2_header_t)] = 1; // updateCount
        *(uint32_t*)&res_buf[sizeof(tpm2_header_t) + 1] = 1; // pcrUpdateCounter
    }
    
    *res_size = res_hdr->length;
    // --- End Simulation ---

    // Simplified FIFO read
    for (uint32_t i = 0; i < *res_size; ++i) {
        res_buf[i] = tpm_read8(TPM_DATA_FIFO_REG + i);
    }

    // Set TPM back to ready
    tpm_write32(TPM_STS_REG, TPM_STS_COMMAND_READY);

    return 0;
}

/**
 * @brief Sends a TPM2_Startup command.
 * @return 0 on success, -1 on failure.
 */
int tpm_startup(void) {
    uint8_t cmd_buf[12];
    tpm2_header_t* header = (tpm2_header_t*)cmd_buf;
    header->tag = TPM2_ST_NO_SESSIONS;
    header->length = sizeof(cmd_buf);
    header->command_code = TPM2_CC_STARTUP;
    *(uint16_t*)(cmd_buf + sizeof(tpm2_header_t)) = TPM2_SU_CLEAR;

    uint8_t res_buf[12];
    uint32_t res_size = sizeof(res_buf);
    if (tpm_transmit(cmd_buf, sizeof(cmd_buf), res_buf, &res_size) != 0) {
        return -1;
    }

    tpm2_header_t* res_hdr = (tpm2_header_t*)res_buf;
    return res_hdr->command_code == 0 ? 0 : -1;
}

/**
 * @brief Sends a TPM2_SelfTest command.
 * @return 0 on success, -1 on failure.
 */
int tpm_self_test(void) {
    uint8_t cmd_buf[11];
    tpm2_header_t* header = (tpm2_header_t*)cmd_buf;
    header->tag = TPM2_ST_NO_SESSIONS;
    header->length = sizeof(cmd_buf);
    header->command_code = TPM2_CC_SELF_TEST;
    cmd_buf[sizeof(tpm2_header_t)] = 1; // fullTest = YES

    uint8_t res_buf[10];
    uint32_t res_size = sizeof(res_buf);
    if (tpm_transmit(cmd_buf, sizeof(cmd_buf), res_buf, &res_size) != 0) {
        return -1;
    }

    tpm2_header_t* res_hdr = (tpm2_header_t*)res_buf;
    return res_hdr->command_code == 0 ? 0 : -1;
}

/**
 * @brief Extends a PCR with a new measurement.
 * @param pcr_index The index of the PCR to extend.
 * @param digest The digest to extend the PCR with.
 * @param digest_len The length of the digest.
 * @return 0 on success, -1 on failure.
 */
int tpm_pcr_extend(uint32_t pcr_index, const uint8_t* digest, uint32_t digest_len) {
    uint8_t cmd_buf[sizeof(tpm2_header_t) + 4 + 4 + 32]; // Header + pcr_handle + auth_size + digest
    tpm2_header_t* header = (tpm2_header_t*)cmd_buf;
    header->tag = TPM2_ST_NO_SESSIONS;
    header->length = sizeof(cmd_buf);
    header->command_code = TPM2_CC_PCR_EXTEND;

    *(uint32_t*)(cmd_buf + sizeof(tpm2_header_t)) = pcr_index;
    *(uint32_t*)(cmd_buf + sizeof(tpm2_header_t) + 4) = 0; // Auth session size
    memcpy(cmd_buf + sizeof(tpm2_header_t) + 8, digest, digest_len);

    uint8_t res_buf[sizeof(tpm2_header_t) + 5];
    uint32_t res_size = sizeof(res_buf);
    if (tpm_transmit(cmd_buf, sizeof(cmd_buf), res_buf, &res_size) != 0) {
        return -1;
    }

    tpm2_header_t* res_hdr = (tpm2_header_t*)res_buf;
    return res_hdr->command_code == 0 ? 0 : -1;
}
