#ifndef PQC_H
#define PQC_H

#include "kyber.h"
#include "dilithium.h"

// Function to initialize the PQC library
int pqc_init(void);

// Kyber KEM functions
int pqc_kyber_keypair(unsigned char *pk, unsigned char *sk);
int pqc_kyber_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk);
int pqc_kyber_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk);

// Dilithium signature functions
int pqc_dilithium_keypair(unsigned char *pk, unsigned char *sk);
int pqc_dilithium_sign(unsigned char *sm, unsigned long long *smlen, const unsigned char *m, unsigned long long mlen, const unsigned char *sk);
int pqc_dilithium_verify(unsigned char *m, unsigned long long *mlen, const unsigned char *sm, unsigned long long smlen, const unsigned char *pk);

#endif
