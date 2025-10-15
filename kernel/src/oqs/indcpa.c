#include <string.h>
#include "indcpa.h"
#include "polyvec.h"
#include "poly.h"
#include "ntt.h"
#include "randombytes.h"
#include "sha3.h"
#include "kyber_params.h"

/*************************************************
* Name:        indcpa_gen_matrix
*
* Description: Deterministically generates a matrix A (or A^T)
*              from a seed. Entries of the matrix are polynomials
*              that look uniformly random.
*
* Arguments:   - polyvec *a:          pointer to ouptput matrix A
*              - const unsigned char *seed: pointer to input seed
*              - int transposed:      boolean deciding whether A or A^T
*                                     is generated
**************************************************/
void indcpa_gen_matrix(polyvec *a, const unsigned char *seed, int transposed)
{
  unsigned int i, j;
  unsigned char buf[SHAKE128_RATE + 2];
  shake128_context state;

  for(i=0;i<KYBER_K;i++) {
    for(j=0;j<KYBER_K;j++) {
      uint16_t ctr = 0;
      if(transposed)
        shake128_absorb(&state, seed, 32, i, j);
      else
        shake128_absorb(&state, seed, 32, j, i);

      shake128_squeezeblocks(buf, 1, &state);

      int k=0;
      while(ctr < KYBER_N)
      {
        uint16_t val = buf[k] | ((uint16_t)buf[k+1] << 8);
        if(val < 3*KYBER_Q)
        {
            a[i].vec[j].coeffs[ctr] = val;
            ctr++;
        }
        k+=2;
        if(k > SHAKE128_RATE)
        {
            shake128_squeezeblocks(buf, 1, &state);
            k = 0;
        }
      }
    }
  }
}


/*************************************************
* Name:        indcpa_keypair
*
* Description: Generates public and private key for the CPA-secure
*              public-key encryption scheme underlying Kyber
*
* Arguments:   - unsigned char *pk: pointer to output public key (of length KYBER_INDCPA_PUBLICKEYBYTES)
*              - unsigned char *sk: pointer to output private key (of length KYBER_INDCPA_SECRETKEYBYTES)
**************************************************/
void indcpa_keypair(unsigned char *pk, unsigned char *sk)
{
  polyvec a[KYBER_K], e, pkpv, skpv;
  unsigned char buf[64];
  unsigned char *publicseed = buf;
  unsigned char *noiseseed = buf+32;
  int i;
  unsigned char nonce = 0;

  randombytes(buf, 32);
  sha3_512(buf, buf, 32);

  indcpa_gen_matrix(a, publicseed, 0);

  for(i=0;i<KYBER_K;i++)
    poly_getnoise(&skpv.vec[i], noiseseed, nonce++);

  for(i=0;i<KYBER_K;i++)
    poly_getnoise(&e.vec[i], noiseseed, nonce++);

  polyvec_ntt(&skpv);
  polyvec_ntt(&e);

  // matrix-vector multiplication
  for(i=0;i<KYBER_K;i++) {
    polyvec_pointwise_acc_montgomery(&pkpv.vec[i], &a[i], &skpv);
    poly_tomont(&pkpv.vec[i]);
  }

  polyvec_add(&pkpv, &pkpv, &e);
  polyvec_reduce(&pkpv);

  indcpa_pack_sk(sk, &skpv);
  indcpa_pack_pk(pk, &pkpv, publicseed);
}


/*************************************************
* Name:        indcpa_enc
*
* Description: Encryption function of the CPA-secure
*              public-key encryption scheme underlying Kyber.
*
* Arguments:   - unsigned char *c:          pointer to output ciphertext (of length KYBER_INDCPA_BYTES)
*              - const unsigned char *m:    pointer to input message (of length 32 bytes)
*              - const unsigned char *pk:   pointer to input public key (of length KYBER_INDCPA_PUBLICKEYBYTES)
*              - const unsigned char *coin: pointer to input random coins (of length 32 bytes)
**************************************************/
void indcpa_enc(unsigned char *c,
               const unsigned char *m,
               const unsigned char *pk,
               const unsigned char *coins)
{
  polyvec sp, pkpv, ep, at[KYBER_K], b;
  poly v, k, epp;
  unsigned char seed[32];
  int i;
  unsigned char nonce = 0;

  indcpa_unpack_pk(&pkpv, seed, pk);

  poly_frommsg(&k, m);

  indcpa_gen_matrix(at, seed, 1);

  for(i=0;i<KYBER_K;i++)
    poly_getnoise(sp.vec+i, coins, nonce++);

  for(i=0;i<KYBER_K;i++)
    poly_getnoise(ep.vec+i, coins, nonce++);

  poly_getnoise(&epp, coins, nonce++);

  polyvec_ntt(&sp);

  // matrix-vector multiplication
  for(i=0;i<KYBER_K;i++)
    polyvec_pointwise_acc_montgomery(b.vec+i, at+i, &sp);

  polyvec_pointwise_acc_montgomery(&v, &pkpv, &sp);

  polyvec_invntt_tomont(&b);
  poly_invntt_tomont(&v);

  polyvec_add(&b, &b, &ep);
  poly_add(&v, &v, &epp);
  poly_add(&v, &v, &k);

  polyvec_reduce(&b);
  poly_reduce(&v);

  polyvec_compress(c, &b);
  poly_compress(c+KYBER_POLYVEC_COMPRESSED_BYTES, &v);
}

/*************************************************
* Name:        indcpa_dec
*
* Description: Decryption function of the CPA-secure
*              public-key encryption scheme underlying Kyber.
*
* Arguments:   - unsigned char *m:        pointer to output decrypted message (of length 32 bytes)
*              - const unsigned char *c:  pointer to input ciphertext (of length KYBER_INDCPA_BYTES)
*              - const unsigned char *sk: pointer to input secret key (of length KYBER_INDCPA_SECRETKEYBYTES)
**************************************************/
void indcpa_dec(unsigned char *m,
               const unsigned char *c,
               const unsigned char *sk)
{
  polyvec b, skpv;
  poly v, mp;

  polyvec_decompress(&b, c);
  poly_decompress(&v, c+KYBER_POLYVEC_COMPRESSED_BYTES);

  indcpa_unpack_sk(&skpv, sk);

  polyvec_ntt(&b);
  polyvec_pointwise_acc_montgomery(&mp, &skpv, &b);
  poly_invntt_tomont(&mp);

  poly_sub(&mp, &v, &mp);
  poly_reduce(&mp);

  poly_tomsg(m, &mp);
}


void indcpa_pack_pk(unsigned char *r, const polyvec *pk, const unsigned char *seed)
{
  polyvec_tobytes(r, pk);
  memcpy(r+KYBER_POLYVEC_BYTES, seed, 32);
}

void indcpa_unpack_pk(polyvec *pk, unsigned char *seed, const unsigned char *packedpk)
{
  polyvec_frombytes(pk, packedpk);
  memcpy(seed, packedpk+KYBER_POLYVEC_BYTES, 32);
}

void indcpa_pack_sk(unsigned char *r, const polyvec *sk)
{
  polyvec_tobytes(r, sk);
}

void indcpa_unpack_sk(polyvec *sk, const unsigned char *packedsk)
{
  polyvec_frombytes(sk, packedsk);
}
