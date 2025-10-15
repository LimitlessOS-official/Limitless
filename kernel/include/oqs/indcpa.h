#ifndef INDCPA_H
#define INDCPA_H

#include "polyvec.h"
#include "poly.h"

void indcpa_keypair(unsigned char *pk, unsigned char *sk);

void indcpa_enc(unsigned char *c,
                const unsigned char *m,
                const unsigned char *pk,
                const unsigned char *coins);

void indcpa_dec(unsigned char *m,
                const unsigned char *c,
                const unsigned char *sk);


void indcpa_pack_pk(unsigned char *r, const polyvec *pk, const unsigned char *seed);
void indcpa_unpack_pk(polyvec *pk, unsigned char *seed, const unsigned char *packedpk);

void indcpa_pack_sk(unsigned char *r, const polyvec *sk);
void indcpa_unpack_sk(polyvec *sk, const unsigned char *packedsk);

void indcpa_gen_matrix(polyvec *a, const unsigned char *seed, int transposed);

#endif
