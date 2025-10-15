#ifndef TPM_H
#define TPM_H

#include <stdint.h>
#include <stdbool.h>

// TPM 2.0 Registers (simplified for simulation)
#define TPM_ACCESS_REG      0x00
#define TPM_STS_REG         0x18
#define TPM_DATA_FIFO_REG   0x24
#define TPM_DID_VID_REG     0xF00

// TPM_ACCESS_REG bits
#define TPM_ACCESS_ACTIVE_LOCALITY  (1 << 5)
#define TPM_ACCESS_REQUEST_USE      (1 << 1)
#define TPM_ACCESS_VALID            (1 << 7)

// TPM_STS_REG bits
#define TPM_STS_COMMAND_READY   (1 << 6)
#define TPM_STS_VALID           (1 << 7)
#define TPM_STS_DATA_AVAIL      (1 << 0)
#define TPM_STS_GO              (1 << 5)
#define TPM_STS_BURST_COUNT_MASK 0xFFFF00

// TPM Commands and Response Codes
#define TPM2_ST_NO_SESSIONS 0x8001
#define TPM2_CC_STARTUP     0x00000144
#define TPM2_CC_SELF_TEST   0x00000143
#define TPM2_CC_PCR_EXTEND  0x00000182
#define TPM2_SU_CLEAR       0x0000

// TPM2 Header
typedef struct {
    uint16_t tag;
    uint32_t length;
    uint32_t command_code;
} __attribute__((packed)) tpm2_header_t;

// TPM Driver State
typedef struct {
    bool is_present;
    bool is_initialized;
    uint32_t vendor_id;
    uint32_t device_id;
    uint8_t locality;
} tpm_driver_state_t;

// Public Function Prototypes
bool tpm_detect(void);
int tpm_init(void);
int tpm_transmit(const uint8_t* cmd_buf, uint32_t cmd_size, uint8_t* res_buf, uint32_t* res_size);
int tpm_startup(void);
int tpm_self_test(void);
int tpm_pcr_extend(uint32_t pcr_index, const uint8_t* digest, uint32_t digest_len);

#endif // TPM_H
