#include <string.h>
#include "dilithium.h"
#include "randombytes.h"
#include "sha3.h"
#include "poly.h"
#include "polyvec.h"
#include "packing.h"
#include "dilithium_params.h"

/*************************************************
* Name:        crypto_sign_keypair
*
* Description: Generates public and private key.
*
* Arguments:   - uint8_t *pk: pointer to output public key (allocated
*                             array of DILITHIUM_CRYPTO_PUBLICKEYBYTES bytes)
*              - uint8_t *sk: pointer to output private key (allocated
*                             array of DILITHIUM_CRYPTO_SECRETKEYBYTES bytes)
*
* Returns 0 (success)
**************************************************/
int crypto_sign_keypair(unsigned char *pk, unsigned char *sk)
{
  unsigned char seedbuf[2*32 + 32];
  unsigned char *rho, *rhoprime, *key;
  polyvecl mat[DILITHIUM_K];
  polyvecl s1, s1hat;
  polyveck s2, t1, t0;

  /* Get randomness for rho, rhoprime and key */
  randombytes(seedbuf, 32);
  shake256(seedbuf, 2*32 + 32, seedbuf, 32);
  rho = seedbuf;
  rhoprime = rho + 32;
  key = rhoprime + 32;

  /* Expand matrix */
  polyvec_matrix_expand(mat, rho);

  /* Sample short vectors s1 and s2 */
  polyvecl_uniform_eta(&s1, rhoprime, 0);
  polyveck_uniform_eta(&s2, rhoprime, DILITHIUM_L);

  /* Matrix-vector multiplication */
  s1hat = s1;
  polyvecl_ntt(&s1hat);
  polyvec_matrix_pointwise_montgomery(&t1, mat, &s1hat);
  polyveck_invntt_tomont(&t1);

  /* Add error vector s2 */
  polyveck_add(&t1, &t1, &s2);

  /* Extract t1 and write public key */
  polyveck_power2round(&t1, &t0, &t1);
  pack_pk(pk, rho, &t1);

  /* Write secret key */
  pack_sk(sk, rho, key, &t0, &s1, &s2);

  return 0;
}

/*************************************************
* Name:        crypto_sign_signature
*
* Description: Computes signature.
*
* Arguments:   - uint8_t *sig:   pointer to output signature (of length DILITHIUM_CRYPTO_BYTES)
*              - size_t *siglen: pointer to output length of signature
*              - uint8_t *m:     pointer to message to be signed
*              - size_t mlen:    length of message
*              - uint8_t *sk:    pointer to bit-packed secret key
*
* Returns 0 (success)
**************************************************/
int crypto_sign_signature(unsigned char *sig,
                          unsigned long long *siglen,
                          const unsigned char *m,
                          unsigned long long mlen,
                          const unsigned char *sk)
{
  unsigned char seedbuf[3*32 + 2*32];
  unsigned char *rho, *key, *mu;
  unsigned char *rhoprime, *tr;
  uint16_t nonce = 0;
  polyvecl mat[DILITHIUM_K], s1, y, z;
  polyveck t0, s2, w1, w0, h;
  poly cp;
  shake256_context state;

  rho = seedbuf;
  key = seedbuf + 32;
  mu = seedbuf + 2*32;
  rhoprime = seedbuf + 3*32;
  tr = rhoprime + 32;
  unpack_sk(rho, key, tr, &s1, &s2, &t0, sk);

  /* Compute mu = CRH(tr, msg) */
  shake256_init(&state);
  shake256_absorb(&state, tr, 32);
  shake256_absorb(&state, m, mlen);
  shake256_finalize(&state);
  shake256_squeeze(mu, 32, &state);

rej:
  /* Sample intermediate vector y */
  polyvecl_uniform_gamma1(&y, rhoprime, nonce++);

  /* Matrix-vector multiplication */
  z = y;
  polyvecl_ntt(&z);
  polyvec_matrix_expand(mat, rho);
  polyvec_matrix_pointwise_montgomery(&w1, mat, &z);
  polyveck_invntt_tomont(&w1);

  /* Decompose w and call the random oracle */
  polyveck_decompose(&w1, &w0, &w1);
  polyveck_pack_w1(sig, &w1);

  shake256_init(&state);
  shake256_absorb(&state, mu, 32);
  shake256_absorb(&state, sig, DILITHIUM_K*DILITHIUM_POLYW1_PACKEDBYTES);
  shake256_finalize(&state);
  shake256_squeeze(sig, 32, &state);
  poly_challenge(&cp, sig);

  /* Compute z, reject if it reveals secret */
  polyvecl_ntt(&s1);
  poly_ntt(&cp);
  polyvecl_pointwise_poly_montgomery(&z, &cp, &s1);
  polyvecl_invntt_tomont(&z);
  polyvecl_add(&z, &z, &y);
  polyvecl_reduce(&z);
  if(polyvecl_chknorm(&z, DILITHIUM_GAMMA1 - DILITHIUM_BETA))
    goto rej;

  /* Check that subtracting cs2 does not change high bits of w and call signature oracle */
  polyveck_ntt(&s2);
  polyveck_pointwise_poly_montgomery(&h, &cp, &s2);
  polyveck_invntt_tomont(&h);
  polyveck_sub(&w0, &w0, &h);
  polyveck_reduce(&w0);
  if(polyveck_chknorm(&w0, DILITHIUM_GAMMA2 - DILITHIUM_BETA))
    goto rej;

  /* Compute hints for w1 */
  polyveck_pointwise_poly_montgomery(&h, &cp, &t0);
  polyveck_invntt_tomont(&h);
  polyveck_reduce(&h);
  if(polyveck_chknorm(&h, DILITHIUM_GAMMA2))
    goto rej;

  polyveck_add(&w0, &w0, &h);
  polyveck_make_hint(&h, &w0, &w1);

  /* Write signature */
  pack_sig(sig, sig, &z, &h);
  *siglen = DILITHIUM_CRYPTO_BYTES;
  return 0;
}

/*************************************************
* Name:        crypto_sign_verify
*
* Description: Verifies signature.
*
* Arguments:   - uint8_t *sig:  pointer to bit-packed signature
*              - size_t siglen: length of signature
*              - uint8_t *m:    pointer to message
*              - size_t mlen:   length of message
*              - uint8_t *pk:   pointer to bit-packed public key
*
* Returns 0 if signature is valid, 1 otherwise.
**************************************************/
int crypto_sign_verify(const unsigned char *sig,
                       unsigned long long siglen,
                       const unsigned char *m,
                       unsigned long long mlen,
                       const unsigned char *pk)
{
  unsigned char rho[32], mu[32];
  polyvecl mat[DILITHIUM_K], z;
  polyveck t1, w1, h;
  poly cp;
  shake256_context state;

  if(siglen != DILITHIUM_CRYPTO_BYTES)
    return -1;

  unpack_pk(rho, &t1, pk);
  if(unpack_sig(&cp, &z, &h, sig))
    return -1;
  if(polyvecl_chknorm(&z, DILITHIUM_GAMMA1 - DILITHIUM_BETA))
    return -1;

  /* Compute mu = CRH(H(rho, t1), msg) */
  shake256(mu, 32, pk, DILITHIUM_CRYPTO_PUBLICKEYBYTES);
  shake256_init(&state);
  shake256_absorb(&state, mu, 32);
  shake256_absorb(&state, m, mlen);
  shake256_finalize(&state);
  shake256_squeeze(mu, 32, &state);

  /* Matrix-vector multiplication */
  polyvec_matrix_expand(mat, rho);
  polyvecl_ntt(&z);
  polyvec_matrix_pointwise_montgomery(&w1, mat, &z);

  /* Use public key to re-compute w1 */
  poly_challenge(&cp, &cp);
  poly_ntt(&cp);
  polyveck_shiftl(&t1);
  polyveck_ntt(&t1);
  polyveck_pointwise_poly_montgomery(&w1, &cp, &t1);
  polyveck_sub(&w1, &w1, &w1);
  polyveck_invntt_tomont(&w1);

  /* Reconstruct w1 */
  polyveck_use_hint(&w1, &w1, &h);
  polyveck_pack_w1(mu, &w1);

  /* Call random oracle */
  shake256_init(&state);
  shake256_absorb(&state, mu, 32 + DILITHIUM_K*DILITHIUM_POLYW1_PACKEDBYTES);
  shake256_finalize(&state);
  shake256_squeeze(mu, 32, &state);

  /* Compare matrices */
  if(memcmp(&cp, mu, 32))
    return -1;

  return 0;
}

/*************************************************
* Name:        crypto_sign_open
*
* Description: Verify signed message.
*
* Arguments:   - uint8_t *m:      pointer to output message (allocated
*                               array with smlen bytes), can be equal to sm
*              - size_t *mlen:    pointer to output length of message
*              - uint8_t *sm:     pointer to signed message
*              - size_t smlen:    length of signed message
*              - uint8_t *pk:     pointer to bit-packed public key
*
* Returns 0 if signed message could be verified, 1 otherwise.
**************************************************/
int crypto_sign_open(unsigned char *m,
                     unsigned long long *mlen,
                     const unsigned char *sm,
                     unsigned long long smlen,
                     const unsigned char *pk)
{
  unsigned long long i;

  if(smlen < DILITHIUM_CRYPTO_BYTES)
    goto badsig;

  *mlen = smlen - DILITHIUM_CRYPTO_BYTES;
  if(crypto_sign_verify(sm + *mlen, DILITHIUM_CRYPTO_BYTES, sm, *mlen, pk))
    goto badsig;
  else {
    /* All good, copy msg, return 0 */
    for(i=0;i<*mlen;i++)
      m[i] = sm[i];
    return 0;
  }

badsig:
  /* Signature verification failed */
  *mlen = -1;
  for(i=0;i<smlen;i++)
    m[i] = 0;

  return -1;
}
