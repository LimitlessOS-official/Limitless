#include "pqc.h"
#include "kyber.h"
#include "dilithium.h"
#include "randombytes.h"
#include "sha3.h"

int pqc_init(void) {
    // This function can be used to initialize the random number generator
    // or perform other setup tasks.
    unsigned char seed[48];
    randombytes(seed, 48);
    shake256_absorb(NULL, seed, 48);
    return 0;
}

// Kyber KEM functions
int pqc_kyber_keypair(unsigned char *pk, unsigned char *sk) {
    return crypto_kem_keypair(pk, sk);
}

int pqc_kyber_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk) {
    return crypto_kem_enc(ct, ss, pk);
}

int pqc_kyber_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk) {
    return crypto_kem_dec(ss, ct, sk);
}

// Dilithium signature functions
int pqc_dilithium_keypair(unsigned char *pk, unsigned char *sk) {
    return crypto_sign_keypair(pk, sk);
}

int pqc_dilithium_sign(unsigned char *sm, unsigned long long *smlen, const unsigned char *m, unsigned long long mlen, const unsigned char *sk) {
    return crypto_sign_signature(sm, smlen, m, mlen, sk);
}

int pqc_dilithium_verify(unsigned char *m, unsigned long long *mlen, const unsigned char *sm, unsigned long long smlen, const unsigned char *pk) {
    return crypto_sign_open(m, mlen, sm, smlen, pk);
}
