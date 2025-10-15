#ifndef OQS_SHA3_H
#define OQS_SHA3_H

#include <stdint.h>
#include <stddef.h>

#define OQS_SHA3_RATE_1088 1088
#define OQS_SHA3_RATE_1344 1344

// Context for SHA3
typedef struct {
    uint64_t s[25];
    unsigned int pos;
} OQS_SHA3_CTX;

// SHAKE128
void OQS_SHA3_shake128_absorb(OQS_SHA3_CTX *state, const uint8_t *in, size_t inlen);
void OQS_SHA3_shake128_squeezeblocks(uint8_t *out, size_t nblocks, OQS_SHA3_CTX *state);
void OQS_SHA3_shake128(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen);

// SHAKE256
void OQS_SHA3_shake256_absorb(OQS_SHA3_CTX *state, const uint8_t *in, size_t inlen);
void OQS_SHA3_shake256_squeezeblocks(uint8_t *out, size_t nblocks, OQS_SHA3_CTX *state);
void OQS_SHA3_shake256(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen);

#endif // OQS_SHA3_H
