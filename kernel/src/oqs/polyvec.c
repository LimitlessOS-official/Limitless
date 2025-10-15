#include "polyvec.h"
#include "poly.h"
#include "kyber_params.h"

/*************************************************
* Name:        polyvec_compress
*
* Description: Compress and serialize a vector of polynomials
*
* Arguments:   - unsigned char *r: pointer to output byte array (needs space for KYBER_POLYVEC_COMPRESSED_BYTES)
*              - const polyvec *a: pointer to input vector of polynomials
**************************************************/
void polyvec_compress(unsigned char *r, const polyvec *a)
{
  int i,j,k;

  polyvec_reduce((polyvec *)a);

  uint16_t t[4];
  for(i=0;i<KYBER_K;i++)
  {
    for(j=0;j<KYBER_N/4;j++)
    {
      for(k=0;k<4;k++)
        t[k] = ((((uint32_t)a->vec[i].coeffs[4*j+k] << 10) + KYBER_Q/2)/KYBER_Q) & 0x3ff;

      r[5*j+0] = (t[0] >> 0);
      r[5*j+1] = (t[0] >> 8) | (t[1] << 2);
      r[5*j+2] = (t[1] >> 6) | (t[2] << 4);
      r[5*j+3] = (t[2] >> 4) | (t[3] << 6);
      r[5*j+4] = (t[3] >> 2);
    }
    r += 320;
  }
}

/*************************************************
* Name:        polyvec_decompress
*
* Description: De-serialize and decompress a vector of polynomials;
*              approximate inverse of polyvec_compress
*
* Arguments:   - polyvec *r:       pointer to output vector of polynomials
*              - unsigned char *a: pointer to input byte array (of length KYBER_POLYVEC_COMPRESSED_BYTES)
**************************************************/
void polyvec_decompress(polyvec *r, const unsigned char *a)
{
  int i,j;
  for(i=0;i<KYBER_K;i++)
  {
    for(j=0;j<KYBER_N/4;j++)
    {
      r->vec[i].coeffs[4*j+0] = ( (a[5*j+0] >> 0) | ((uint32_t)a[5*j+1] << 8) ) & 0x3FF;
      r->vec[i].coeffs[4*j+1] = ( (a[5*j+1] >> 2) | ((uint32_t)a[5*j+2] << 6) ) & 0x3FF;
      r->vec[i].coeffs[4*j+2] = ( (a[5*j+2] >> 4) | ((uint32_t)a[5*j+3] << 4) ) & 0x3FF;
      r->vec[i].coeffs[4*j+3] = ( (a[5*j+3] >> 6) | ((uint32_t)a[5*j+4] << 2) ) & 0x3FF;
    }
    a += 320;
  }

  for(i=0;i<KYBER_K;i++)
  {
    for(j=0;j<KYBER_N;j++)
      r->vec[i].coeffs[j] = ((uint32_t)r->vec[i].coeffs[j] * KYBER_Q + 512) >> 10;
  }
}

/*************************************************
* Name:        polyvec_tobytes
*
* Description: Serialize a vector of polynomials
*
* Arguments:   - unsigned char *r: pointer to output byte array (needs space for KYBER_POLYVECBYTES)
*              - const polyvec *a: pointer to input vector of polynomials
**************************************************/
void polyvec_tobytes(unsigned char *r, const polyvec *a)
{
  unsigned int i;
  for(i=0;i<KYBER_K;i++)
    poly_tobytes(r+i*KYBER_POLY_BYTES, &a->vec[i]);
}

/*************************************************
* Name:        polyvec_frombytes
*
* Description: De-serialize a vector of polynomials;
*              inverse of polyvec_tobytes
*
* Arguments:   - polyvec *r:       pointer to output vector of polynomials
*              - unsigned char *a: pointer to input byte array (of length KYBER_POLYVECBYTES)
**************************************************/
void polyvec_frombytes(polyvec *r, const unsigned char *a)
{
  unsigned int i;
  for(i=0;i<KYBER_K;i++)
    poly_frombytes(&r->vec[i], a+i*KYBER_POLY_BYTES);
}

/*************************************************
* Name:        polyvec_ntt
*
* Description: Apply forward NTT to all elements of a vector of polynomials
*
* Arguments:   - polyvec *r: pointer to in/output vector of polynomials
**************************************************/
void polyvec_ntt(polyvec *r)
{
  unsigned int i;
  for(i=0;i<KYBER_K;i++)
    poly_ntt(&r->vec[i]);
}

/*************************************************
* Name:        polyvec_invntt_tomont
*
* Description: Apply inverse NTT to all elements of a vector of polynomials
*              and multiply by Montgomery factor 2^16
*
* Arguments:   - polyvec *r: pointer to in/output vector of polynomials
**************************************************/
void polyvec_invntt_tomont(polyvec *r)
{
  unsigned int i;
  for(i=0;i<KYBER_K;i++)
    poly_invntt_tomont(&r->vec[i]);
}

/*************************************************
* Name:        polyvec_pointwise_acc_montgomery
*
* Description: Pointwise multiply elements of a and b and accumulate into r
*
* Arguments: - poly *r:          pointer to output polynomial
*            - const polyvec *a: pointer to first input vector of polynomials
*            - const polyvec *b: pointer to second input vector of polynomials
**************************************************/
void polyvec_pointwise_acc_montgomery(poly *r, const polyvec *a, const polyvec *b)
{
  unsigned int i;
  poly t;

  poly_basemul_montgomery(r, &a->vec[0], &b->vec[0]);
  for(i=1;i<KYBER_K;i++) {
    poly_basemul_montgomery(&t, &a->vec[i], &b->vec[i]);
    poly_add(r, r, &t);
  }

  poly_reduce(r);
}

/*************************************************
* Name:        polyvec_reduce
*
* Description: Apply Barrett reduction to all coefficients of a vector of polynomials
*
* Arguments:   - polyvec *r: pointer to in/output vector of polynomials
**************************************************/
void polyvec_reduce(polyvec *r)
{
  unsigned int i;
  for(i=0;i<KYBER_K;i++)
    poly_reduce(&r->vec[i]);
}

/*************************************************
* Name:        polyvec_add
*
* Description: Add vectors of polynomials
*
* Arguments: - polyvec *r:       pointer to output vector of polynomials
*            - const polyvec *a: pointer to first input vector of polynomials
*            - const polyvec *b: pointer to second input vector of polynomials
**************************************************/
void polyvec_add(polyvec *r, const polyvec *a, const polyvec *b)
{
  unsigned int i;
  for(i=0;i<KYBER_K;i++)
    poly_add(&r->vec[i], &a->vec[i], &b->vec[i]);
}
