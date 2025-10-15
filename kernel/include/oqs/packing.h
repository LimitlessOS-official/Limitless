#ifndef PACKING_H
#define PACKING_H

#include "polyvec.h"
#include "poly.h"
#include "dilithium_params.h"

void pack_pk(unsigned char pk[DILITHIUM_CRYPTO_PUBLICKEYBYTES], const unsigned char rho[32], const polyveck *t1);
void unpack_pk(unsigned char rho[32], polyveck *t1, const unsigned char pk[DILITHIUM_CRYPTO_PUBLICKEYBYTES]);

void pack_sk(unsigned char sk[DILITHIUM_CRYPTO_SECRETKEYBYTES],
             const unsigned char rho[32],
             const unsigned char key[32],
             const unsigned char tr[32],
             const polyvecl *s1,
             const polyveck *s2,
             const polyveck *t0);

void unpack_sk(unsigned char rho[32],
               unsigned char key[32],
               unsigned char tr[32],
               polyvecl *s1,
               polyveck *s2,
               polyveck *t0,
               const unsigned char sk[DILITHIUM_CRYPTO_SECRETKEYBYTES]);

void pack_sig(unsigned char sig[DILITHIUM_CRYPTO_BYTES], const unsigned char c[32], const polyvecl *z, const polyveck *h);
int unpack_sig(unsigned char c[32], polyvecl *z, polyveck *h, const unsigned char sig[DILITHIUM_CRYPTO_BYTES]);

#endif
