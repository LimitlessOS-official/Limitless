#include <string.h>
#include "poly.h"
#include "ntt.h"
#include "reduce.h"
#include "sha3.h"
#include "kyber_params.h"

/*************************************************
* Name:        poly_compress
*
* Description: Compression and subsequent serialization of a polynomial
*
* Arguments:   - unsigned char *r: pointer to output byte array (of length KYBER_POLYCOMPRESSED_BYTES)
*              - const poly *a:    pointer to input polynomial
**************************************************/
void poly_compress(unsigned char *r, const poly *a)
{
  unsigned int i,j;
  uint8_t t[8];

  poly_reduce((poly *)a);

  for(i=0;i<KYBER_N/8;i++) {
    for(j=0;j<8;j++)
      t[j] = ((((uint32_t)a->coeffs[8*i+j] << 4) + KYBER_Q/2)/KYBER_Q) & 15;

    r[0] = t[0] | (t[1] << 4);
    r[1] = t[2] | (t[3] << 4);
    r[2] = t[4] | (t[5] << 4);
    r[3] = t[6] | (t[7] << 4);
    r += 4;
  }
}

/*************************************************
* Name:        poly_decompress
*
* Description: De-serialization and subsequent decompression of a polynomial;
*              approximate inverse of poly_compress
*
* Arguments:   - poly *r:                pointer to output polynomial
*              - const unsigned char *a: pointer to input byte array (of length KYBER_POLYCOMPRESSED_BYTES)
**************************************************/
void poly_decompress(poly *r, const unsigned char *a)
{
  unsigned int i;
  for(i=0;i<KYBER_N/2;i++)
  {
    r->coeffs[2*i+0] = (((uint32_t)(a[0] & 15) * KYBER_Q) + 8) >> 4;
    r->coeffs[2*i+1] = (((uint32_t)(a[0] >> 4) * KYBER_Q) + 8) >> 4;
    a += 1;
  }
}


/*************************************************
* Name:        poly_tobytes
*
* Description: Serialization of a polynomial
*
* Arguments:   - unsigned char *r: pointer to output byte array (needs space for KYBER_POLYBYTES)
*              - const poly *a:    pointer to input polynomial
**************************************************/
void poly_tobytes(unsigned char *r, const poly *a)
{
  unsigned int i;
  uint16_t t0, t1;

  poly_reduce((poly *)a);

  for(i=0;i<KYBER_N/2;i++) {
    t0 = a->coeffs[2*i];
    t1 = a->coeffs[2*i+1];
    r[3*i+0] = (t0 >> 0);
    r[3*i+1] = (t0 >> 8) | (t1 << 4);
    r[3*i+2] = (t1 >> 4);
  }
}

/*************************************************
* Name:        poly_frombytes
*
* Description: De-serialization of a polynomial;
*              inverse of poly_tobytes
*
* Arguments:   - poly *r:                pointer to output polynomial
*              - const unsigned char *a: pointer to input byte array (of KYBER_POLYBYTES bytes)
**************************************************/
void poly_frombytes(poly *r, const unsigned char *a)
{
  unsigned int i;
  for(i=0;i<KYBER_N/2;i++)
  {
    r->coeffs[2*i]   = ((a[3*i+0] >> 0) | ((uint16_t)a[3*i+1] << 8)) & 0xFFF;
    r->coeffs[2*i+1] = ((a[3*i+1] >> 4) | ((uint16_t)a[3*i+2] << 4)) & 0xFFF;
  }
}

/*************************************************
* Name:        poly_frommsg
*
* Description: Convert 32-byte message to polynomial
*
* Arguments:   - poly *r:                  pointer to output polynomial
*              - const unsigned char *msg: pointer to input message
**************************************************/
void poly_frommsg(poly *r, const unsigned char msg[32])
{
  unsigned int i,j;
  int16_t mask;

  for(i=0;i<32;i++)
  {
    for(j=0;j<8;j++)
    {
      mask = -(int16_t)((msg[i] >> j)&1);
      r->coeffs[8*i+j] = mask & ((KYBER_Q+1)/2);
    }
  }
}

/*************************************************
* Name:        poly_tomsg
*
* Description: Convert polynomial to 32-byte message
*
* Arguments:   - unsigned char *msg: pointer to output message
*              - const poly *a:      pointer to input polynomial
**************************************************/
void poly_tomsg(unsigned char msg[32], const poly *a)
{
  unsigned int i,j;
  uint16_t t;

  poly_reduce((poly *)a);

  for(i=0;i<32;i++)
  {
    msg[i] = 0;
    for(j=0;j<8;j++)
    {
      t = (((((uint16_t)a->coeffs[8*i+j] << 1) + KYBER_Q/2)/KYBER_Q) & 1);
      msg[i] |= t << j;
    }
  }
}

/*************************************************
* Name:        poly_getnoise
*
* Description: Sample a polynomial deterministically from a seed and a nonce,
*              with output polynomial close to centered binomial distribution
*              with parameter KYBER_ETA
*
* Arguments:   - poly *r:                   pointer to output polynomial
*              - const unsigned char *seed: pointer to input seed (of length 32 bytes)
*              - unsigned char nonce:       one-byte nonce
**************************************************/
void poly_getnoise(poly *r, const unsigned char *seed, unsigned char nonce)
{
    unsigned char buf[KYBER_ETA1*KYBER_N/4];
    unsigned char extseed[33];
    unsigned int i, j;
    uint32_t t0, t1;

    memcpy(extseed, seed, 32);
    extseed[32] = nonce;

    shake256(buf, sizeof(buf), extseed, 33);

    for (i = 0; i < KYBER_N / 4; i++) {
        t0 = buf[i];
        t1 = buf[i] >> 4;

        for (j = 0; j < 4; j++) {
            r->coeffs[4 * i + j] = 0;
            r->coeffs[4 * i + j] += (t0 >> j) & 1;
            r->coeffs[4 * i + j] -= (t1 >> j) & 1;
        }
    }
}


/*************************************************
* Name:        poly_ntt
*
* Description: Computes number-theoretic transform (NTT) of a polynomial in place;
*              inputs assumed to be in normal order, output in bitreversed order
*
* Arguments:   - poly *r: pointer to in/output polynomial
**************************************************/
void poly_ntt(poly *r)
{
  ntt(r->coeffs);
  poly_reduce(r);
}

/*************************************************
* Name:        poly_invntt_tomont
*
* Description: Computes inverse of NTT in place;
*              inputs assumed to be in bitreversed order, output in normal order
*
* Arguments:   - poly *r: pointer to in/output polynomial
**************************************************/
void poly_invntt_tomont(poly *r)
{
  invntt(r->coeffs);
}

/*************************************************
* Name:        poly_basemul_montgomery
*
* Description: Multiplication of two polynomials in NTT domain
*
* Arguments:   - poly *r:       pointer to output polynomial
*              - const poly *a: pointer to first input polynomial
*              - const poly *b: pointer to second input polynomial
**************************************************/
void poly_basemul_montgomery(poly *r, const poly *a, const poly *b)
{
  unsigned int i;
  for(i=0;i<KYBER_N/4;i++) {
    basemul(&r->coeffs[4*i], &a->coeffs[4*i], &b->coeffs[4*i], ntt_zetas[64+i]);
    basemul(&r->coeffs[4*i+2], &a->coeffs[4*i+2], &b->coeffs[4*i+2], -ntt_zetas[64+i]);
  }
}

/*************************************************
* Name:        poly_tomont
*
* Description: Inplace conversion of all coefficients of a polynomial
*              from normal domain to Montgomery domain
*
* Arguments:   - poly *r: pointer to in/output polynomial
**************************************************/
void poly_tomont(poly *r)
{
  unsigned int i;
  const int16_t f = (1ULL << 32) % KYBER_Q;
  for(i=0;i<KYBER_N;i++)
    r->coeffs[i] = montgomery_reduce((int32_t)r->coeffs[i]*f);
}

/*************************************************
* Name:        poly_reduce
*
* Description: Applies Barrett reduction to all coefficients of a polynomial
*              for details of the Barrett reduction see Luca et al.,
*              "A note on scalar multiplication with fixed-base combs"
*
* Arguments:   - poly *r: pointer to in/output polynomial
**************************************************/
void poly_reduce(poly *r)
{
  unsigned int i;
  for(i=0;i<KYBER_N;i++)
    r->coeffs[i] = barrett_reduce(r->coeffs[i]);
}


/*************************************************
* Name:        poly_add
*
* Description: Add two polynomials
*
* Arguments: - poly *r:       pointer to output polynomial
*            - const poly *a: pointer to first input polynomial
*            - const poly *b: pointer to second input polynomial
**************************************************/
void poly_add(poly *r, const poly *a, const poly *b)
{
  unsigned int i;
  for(i=0;i<KYBER_N;i++)
    r->coeffs[i] = a->coeffs[i] + b->coeffs[i];
}

/*************************************************
* Name:        poly_sub
*
* Description: Subtract two polynomials
*
* Arguments: - poly *r:       pointer to output polynomial
*            - const poly *a: pointer to first input polynomial
*            - const poly *b: pointer to second input polynomial
**************************************************/
void poly_sub(poly *r, const poly *a, const poly *b)
{
  unsigned int i;
  for(i=0;i<KYBER_N;i++)
    r->coeffs[i] = a->coeffs[i] - b->coeffs[i];
}
