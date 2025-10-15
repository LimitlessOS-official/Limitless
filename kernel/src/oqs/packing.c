#include "packing.h"
#include "polyvec.h"
#include "poly.h"
#include "dilithium_params.h"
#include <string.h>

/*************************************************
* Name:        pack_pk
*
* Description: Bit-pack public key pk = (rho, t1).
*
* Arguments:   - uint8_t pk[]: output byte array
*              - const uint8_t rho[]: byte array containing rho
*              - const polyveck *t1: pointer to vector t1
**************************************************/
void pack_pk(unsigned char pk[DILITHIUM_CRYPTO_PUBLICKEYBYTES],
             const unsigned char rho[32],
             const polyveck *t1)
{
  unsigned int i;

  memcpy(pk, rho, 32);
  pk += 32;

  for(i=0;i<DILITHIUM_K;i++)
    polyt1_pack(pk + i*DILITHIUM_POLYT1_PACKEDBYTES, &t1->vec[i]);
}

/*************************************************
* Name:        unpack_pk
*
* Description: Unpack public key pk = (rho, t1).
*
* Arguments:   - const unsigned char rho[]: output byte array for rho
*              - const polyveck *t1: pointer to output vector t1
*              - uint8_t pk[]: byte array containing bit-packed pk
**************************************************/
void unpack_pk(unsigned char rho[32],
               polyveck *t1,
               const unsigned char pk[DILITHIUM_CRYPTO_PUBLICKEYBYTES])
{
  unsigned int i;

  memcpy(rho, pk, 32);
  pk += 32;

  for(i=0;i<DILITHIUM_K;i++)
    polyt1_unpack(&t1->vec[i], pk + i*DILITHIUM_POLYT1_PACKEDBYTES);
}

/*************************************************
* Name:        pack_sk
*
* Description: Bit-pack secret key sk = (rho, key, tr, s1, s2, t0).
*
* Arguments:   - uint8_t sk[]: output byte array
*              - const uint8_t rho[]: byte array containing rho
*              - const uint8_t key[]: byte array containing key
*              - const uint8_t tr[]: byte array containing tr
*              - const polyvecl *s1: pointer to vector s1
*              - const polyveck *s2: pointer to vector s2
*              - const polyveck *t0: pointer to vector t0
**************************************************/
void pack_sk(unsigned char sk[DILITHIUM_CRYPTO_SECRETKEYBYTES],
             const unsigned char rho[32],
             const unsigned char key[32],
             const unsigned char tr[32],
             const polyvecl *s1,
             const polyveck *s2,
             const polyveck *t0)
{
  unsigned int i;

  memcpy(sk, rho, 32);
  sk += 32;

  memcpy(sk, key, 32);
  sk += 32;

  memcpy(sk, tr, 32);
  sk += 32;

  for(i=0;i<DILITHIUM_L;i++)
    polyeta_pack(sk + i*DILITHIUM_POLYETA_PACKEDBYTES, &s1->vec[i]);
  sk += DILITHIUM_L*DILITHIUM_POLYETA_PACKEDBYTES;

  for(i=0;i<DILITHIUM_K;i++)
    polyeta_pack(sk + i*DILITHIUM_POLYETA_PACKEDBYTES, &s2->vec[i]);
  sk += DILITHIUM_K*DILITHIUM_POLYETA_PACKEDBYTES;

  for(i=0;i<DILITHIUM_K;i++)
    polyt0_pack(sk + i*DILITHIUM_POLYT0_PACKEDBYTES, &t0->vec[i]);
}

/*************************************************
* Name:        unpack_sk
*
* Description: Unpack secret key sk = (rho, key, tr, s1, s2, t0).
*
* Arguments:   - const uint8_t rho[]: output byte array for rho
*              - const uint8_t key[]: output byte array for key
*              - const uint8_t tr[]: output byte array for tr
*              - const polyvecl *s1: pointer to output vector s1
*              - const polyveck *s2: pointer to output vector s2
*              - const polyveck *t0: pointer to output vector t0
*              - uint8_t sk[]: byte array containing bit-packed sk
**************************************************/
void unpack_sk(unsigned char rho[32],
               unsigned char key[32],
               unsigned char tr[32],
               polyvecl *s1,
               polyveck *s2,
               polyveck *t0,
               const unsigned char sk[DILITHIUM_CRYPTO_SECRETKEYBYTES])
{
  unsigned int i;

  memcpy(rho, sk, 32);
  sk += 32;

  memcpy(key, sk, 32);
  sk += 32;

  memcpy(tr, sk, 32);
  sk += 32;

  for(i=0;i<DILITHIUM_L;i++)
    polyeta_unpack(&s1->vec[i], sk + i*DILITHIUM_POLYETA_PACKEDBYTES);
  sk += DILITHIUM_L*DILITHIUM_POLYETA_PACKEDBYTES;

  for(i=0;i<DILITHIUM_K;i++)
    polyeta_unpack(&s2->vec[i], sk + i*DILITHIUM_POLYETA_PACKEDBYTES);
  sk += DILITHIUM_K*DILITHIUM_POLYETA_PACKEDBYTES;

  for(i=0;i<DILITHIUM_K;i++)
    polyt0_unpack(&t0->vec[i], sk + i*DILITHIUM_POLYT0_PACKEDBYTES);
}

/*************************************************
* Name:        pack_sig
*
* Description: Bit-pack signature sig = (c, z, h).
*
* Arguments:   - uint8_t sig[]: output byte array
*              - const uint8_t c[]: byte array containing c
*              - const polyvecl *z: pointer to vector z
*              - const polyveck *h: pointer to vector h
**************************************************/
void pack_sig(unsigned char sig[DILITHIUM_CRYPTO_BYTES],
              const unsigned char c[32],
              const polyvecl *z,
              const polyveck *h)
{
  unsigned int i, j, k;

  memcpy(sig, c, 32);
  sig += 32;

  for(i=0;i<DILITHIUM_L;i++)
    polyz_pack(sig + i*DILITHIUM_POLYZ_PACKEDBYTES, &z->vec[i]);
  sig += DILITHIUM_L*DILITHIUM_POLYZ_PACKEDBYTES;

  /* Encode h */
  k=0;
  for(i=0;i<DILITHIUM_K;i++) {
    for(j=0;j<DILITHIUM_N;j++)
      if(h->vec[i].coeffs[j] != 0)
        sig[k++] = j;

    sig[DILITHIUM_OMEGA + i] = k;
  }
}

/*************************************************
* Name:        unpack_sig
*
* Description: Unpack signature sig = (c, z, h).
*
* Arguments:   - uint8_t c[]: output byte array for c
*              - polyvecl *z: pointer to output vector z
*              - polyveck *h: pointer to output vector h
*              - const uint8_t sig[]: byte array containing bit-packed sig
*
* Returns 1 in case of malformed signature; otherwise 0.
**************************************************/
int unpack_sig(unsigned char c[32],
               polyvecl *z,
               polyveck *h,
               const unsigned char sig[DILITHIUM_CRYPTO_BYTES])
{
  unsigned int i, j, k;

  memcpy(c, sig, 32);
  sig += 32;

  for(i=0;i<DILITHIUM_L;i++)
    polyz_unpack(&z->vec[i], sig + i*DILITHIUM_POLYZ_PACKEDBYTES);
  sig += DILITHIUM_L*DILITHIUM_POLYZ_PACKEDBYTES;

  /* Decode h */
  k = 0;
  for(i=0;i<DILITHIUM_K;i++) {
    for(j=0;j<DILITHIUM_N;j++)
      h->vec[i].coeffs[j] = 0;

    if(sig[DILITHIUM_OMEGA + i] < k || sig[DILITHIUM_OMEGA + i] > DILITHIUM_OMEGA)
      return 1;

    for(j=k;j<sig[DILITHIUM_OMEGA + i];j++) {
      /* Coefficients are ordered for strong unforgeability */
      if(j > k && sig[j] <= sig[j-1]) return 1;
      h->vec[i].coeffs[sig[j]] = 1;
    }
    k = sig[DILITHIUM_OMEGA + i];
  }

  /* Extra indices are zero for strong unforgeability */
  for(j=k;j<DILITHIUM_OMEGA;j++)
    if(sig[j])
      return 1;

  return 0;
}
