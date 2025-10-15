#ifndef DILITHIUM_H
#define DILITHIUM_H

#include "dilithium_params.h"

#define crypto_sign_keypair DILITHIUM_NAMESPACE(keypair)
int crypto_sign_keypair(unsigned char *pk, unsigned char *sk);

#define crypto_sign_signature DILITHIUM_NAMESPACE(signature)
int crypto_sign_signature(unsigned char *sig, unsigned long long *siglen,
                          const unsigned char *m, unsigned long long mlen,
                          const unsigned char *sk);

#define crypto_sign_verify DILITHIUM_NAMESPACE(verify)
int crypto_sign_verify(const unsigned char *sig, unsigned long long siglen,
                       const unsigned char *m, unsigned long long mlen,
                       const unsigned char *pk);

#define crypto_sign_open DILITHIUM_NAMESPACE(open)
int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                     const unsigned char *sm, unsigned long long smlen,
                     const unsigned char *pk);

#endif
