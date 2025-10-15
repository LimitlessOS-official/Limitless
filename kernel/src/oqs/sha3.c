/*
 * Based on the public domain implementation of the Keccak-p permutation.
 */

#include <oqs/sha3.h>
#include <string.h>

#define ROL64(a, n) (((a) << (n)) | ((a) >> (64 - (n))))

static const uint64_t KeccakF_RoundConstants[24] = {
    0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
    0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
    0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
    0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
    0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
    0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
    0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
    0x8000000000008080, 0x0000000080000001, 0x8000000080008008
};

static void KeccakF1600_StatePermute(uint64_t *state) {
    int round;
    uint64_t C[5], D;

    for (round = 0; round < 24; ++round) {
        // Theta
        for (int i = 0; i < 5; ++i) {
            C[i] = state[i] ^ state[i + 5] ^ state[i + 10] ^ state[i + 15] ^ state[i + 20];
        }
        for (int i = 0; i < 5; ++i) {
            D = C[(i + 4) % 5] ^ ROL64(C[(i + 1) % 5], 1);
            for (int j = 0; j < 25; j += 5) {
                state[i + j] ^= D;
            }
        }

        // Rho and Pi
        uint64_t temp = state[1];
        state[1] = ROL64(state[6], 44);
        state[6] = ROL64(state[9], 20);
        state[9] = ROL64(state[22], 61);
        state[22] = ROL64(state[14], 39);
        state[14] = ROL64(state[20], 18);
        state[20] = ROL64(state[2], 62);
        state[2] = ROL64(state[12], 43);
        state[12] = ROL64(state[13], 25);
        state[13] = ROL64(state[19], 8);
        state[19] = ROL64(state[23], 56);
        state[23] = ROL64(state[15], 41);
        state[15] = ROL64(state[4], 27);
        state[4] = ROL64(state[24], 14);
        state[24] = ROL64(state[21], 2);
        state[21] = ROL64(state[8], 55);
        state[8] = ROL64(state[16], 45);
        state[16] = ROL64(state[5], 36);
        state[5] = ROL64(state[3], 28);
        state[3] = ROL64(state[18], 21);
        state[18] = ROL64(state[17], 15);
        state[17] = ROL64(state[11], 10);
        state[11] = ROL64(state[7], 6);
        state[7] = ROL64(state[10], 3);
        state[10] = ROL64(temp, 1);

        // Chi
        for (int j = 0; j < 25; j += 5) {
            uint64_t t0 = state[j + 0], t1 = state[j + 1];
            state[j + 0] ^= (~t1) & state[j + 2];
            state[j + 1] ^= (~state[j + 2]) & state[j + 3];
            state[j + 2] ^= (~state[j + 3]) & state[j + 4];
            state[j + 3] ^= (~state[j + 4]) & t0;
            state[j + 4] ^= (~t0) & t1;
        }

        // Iota
        state[0] ^= KeccakF_RoundConstants[round];
    }
}

static void sha3_absorb(OQS_SHA3_CTX *state, const uint8_t *in, size_t inlen, size_t rate) {
    size_t i;
    while (inlen > 0) {
        size_t absorb_len = (inlen < (rate / 8) - state->pos) ? inlen : (rate / 8) - state->pos;
        for (i = 0; i < absorb_len; ++i) {
            state->s[state->pos + i] ^= (uint64_t)in[i] << (8 * (state->pos % 8));
        }
        in += absorb_len;
        inlen -= absorb_len;
        state->pos += absorb_len;

        if (state->pos == rate / 8) {
            KeccakF1600_StatePermute(state->s);
            state->pos = 0;
        }
    }
}

static void sha3_finalize(OQS_SHA3_CTX *state, size_t rate, uint8_t p) {
    state->s[state->pos] ^= (uint64_t)p << (8 * (state->pos % 8));
    state->s[(rate / 8) - 1] ^= 0x8000000000000000;
    KeccakF1600_StatePermute(state->s);
    state->pos = 0;
}

static void sha3_squeeze(uint8_t *out, size_t outlen, OQS_SHA3_CTX *state, size_t rate) {
    size_t i;
    while (outlen > 0) {
        if (state->pos == rate / 8) {
            KeccakF1600_StatePermute(state->s);
            state->pos = 0;
        }
        size_t squeeze_len = (outlen < (rate / 8) - state->pos) ? outlen : (rate / 8) - state->pos;
        for (i = 0; i < squeeze_len; ++i) {
            out[i] = (state->s[state->pos + i] >> (8 * ((state->pos + i) % 8))) & 0xFF;
        }
        out += squeeze_len;
        outlen -= squeeze_len;
        state->pos += squeeze_len;
    }
}

void OQS_SHA3_shake128_absorb(OQS_SHA3_CTX *state, const uint8_t *in, size_t inlen) {
    sha3_absorb(state, in, inlen, OQS_SHA3_RATE_1344);
}

void OQS_SHA3_shake128_squeezeblocks(uint8_t *out, size_t nblocks, OQS_SHA3_CTX *state) {
    sha3_squeeze(out, nblocks * (OQS_SHA3_RATE_1344 / 8), state, OQS_SHA3_RATE_1344);
}

void OQS_SHA3_shake128(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen) {
    OQS_SHA3_CTX state;
    memset(&state, 0, sizeof(state));
    sha3_absorb(&state, in, inlen, OQS_SHA3_RATE_1344);
    sha3_finalize(&state, OQS_SHA3_RATE_1344, 0x1F);
    sha3_squeeze(out, outlen, &state, OQS_SHA3_RATE_1344);
}

void OQS_SHA3_shake256_absorb(OQS_SHA3_CTX *state, const uint8_t *in, size_t inlen) {
    sha3_absorb(state, in, inlen, OQS_SHA3_RATE_1088);
}

void OQS_SHA3_shake256_squeezeblocks(uint8_t *out, size_t nblocks, OQS_SHA3_CTX *state) {
    sha3_squeeze(out, nblocks * (OQS_SHA3_RATE_1088 / 8), state, OQS_SHA3_RATE_1088);
}

void OQS_SHA3_shake256(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen) {
    OQS_SHA3_CTX state;
    memset(&state, 0, sizeof(state));
    sha3_absorb(&state, in, inlen, OQS_SHA3_RATE_1088);
    sha3_finalize(&state, OQS_SHA3_RATE_1088, 0x1F);
    sha3_squeeze(out, outlen, &state, OQS_SHA3_RATE_1088);
}
