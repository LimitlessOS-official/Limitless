#ifndef POLYVEC_H
#define POLYVEC_H

#include "poly.h"
#include "kyber_params.h"

typedef struct {
    poly vec[KYBER_K];
} polyvec;


void polyvec_compress(unsigned char *r, const polyvec *a);
void polyvec_decompress(polyvec *r, const unsigned char *a);

void polyvec_tobytes(unsigned char *r, const polyvec *a);
void polyvec_frombytes(polyvec *r, const unsigned char *a);

void polyvec_ntt(polyvec *r);
void polyvec_invntt_tomont(polyvec *r);

void polyvec_pointwise_acc_montgomery(poly *r, const polyvec *a, const polyvec *b);

void polyvec_reduce(polyvec *r);
void polyvec_add(polyvec *r, const polyvec *a, const polyvec *b);

#endif
