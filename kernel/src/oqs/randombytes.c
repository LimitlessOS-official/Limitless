#include "randombytes.h"
#include "sha3.h"
#include <stddef.h>

// For a real kernel, this would be a proper entropy source.
// We will simulate it for this implementation.
static void get_kernel_entropy(unsigned char *seed, size_t len) {
    // In a real OS, this would collect entropy from various sources:
    // - Hardware RNG (like RDRAND)
    // - Interrupt timing jitter
    // - Mouse/keyboard input timings
    // - Network packet arrival times
    // For now, we use a fixed seed to ensure deterministic behavior for this example,
    // but in a real system, this MUST be replaced with a true entropy source.
    for(size_t i = 0; i < len; ++i) {
        seed[i] = (unsigned char)i;
    }
}

static shake256_context rng_state;
static int rng_initialized = 0;

static void initialize_rng(void) {
    unsigned char seed[48];
    get_kernel_entropy(seed, 48);
    shake256_absorb(&rng_state, seed, 48);
    rng_initialized = 1;
}

void randombytes(unsigned char *out, unsigned long long outlen) {
    if (!rng_initialized) {
        initialize_rng();
    }
    shake256_squeezeblocks(out, outlen / SHAKE256_RATE, &rng_state);

    unsigned long long rem = outlen % SHAKE256_RATE;
    if (rem > 0) {
        unsigned char last_block[SHAKE256_RATE];
        shake256_squeezeblocks(last_block, 1, &rng_state);
        for (unsigned long long i = 0; i < rem; ++i) {
            out[outlen - rem + i] = last_block[i];
        }
    }
}
