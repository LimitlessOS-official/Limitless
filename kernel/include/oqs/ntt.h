#ifndef NTT_H
#define NTT_H

#include <stdint.h>
#include "kyber_params.h"

extern int16_t ntt_zetas[256];
extern int16_t ntt_zetas_inv[256];

void ntt(int16_t r[256]);
void invntt(int16_t r[256]);

void basemul(int16_t r[2], const int16_t a[2], const int16_t b[2], int16_t zeta);

#endif
