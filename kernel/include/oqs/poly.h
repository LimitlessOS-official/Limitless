#ifndef POLY_H
#define POLY_H

#include <stdint.h>
#include "kyber_params.h"

/*
 * Elements of R_q = Z_q[X]/(X^n + 1). Represents a polynomial
 * of degree < 256 whose coefficients are in Z_3329.
 */
typedef struct {
    int16_t coeffs[KYBER_N];
} poly;

void poly_compress(unsigned char *r, const poly *a);
void poly_decompress(poly *r, const unsigned char *a);

void poly_tobytes(unsigned char *r, const poly *a);
void poly_frombytes(poly *r, const unsigned char *a);

void poly_frommsg(poly *r, const unsigned char msg[32]);
void poly_tomsg(unsigned char msg[32], const poly *a);

void poly_getnoise(poly *r, const unsigned char *seed, unsigned char nonce);

void poly_ntt(poly *r);
void poly_invntt_tomont(poly *r);
void poly_basemul_montgomery(poly *r, const poly *a, const poly *b);
void poly_tomont(poly *r);

void poly_reduce(poly *r);
void poly_add(poly *r, const poly *a, const poly *b);
void poly_sub(poly *r, const poly *a, const poly *b);

#endif
