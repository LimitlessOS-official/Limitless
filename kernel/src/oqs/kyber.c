#include <string.h>
#include "kyber.h"
#include "indcpa.h"
#include "sha3.h"
#include "randombytes.h"
#include "kyber_params.h"

// KYBER CCA-KEM

/*************************************************
* Name:        crypto_kem_keypair
*
* Description: Generates public and private key
*              for CCA-secure Kyber key encapsulation mechanism
*
* Arguments:   - unsigned char *pk: pointer to output public key (of length KYBER_PUBLICKEYBYTES)
*              - unsigned char *sk: pointer to output private key (of length KYBER_SECRETKEYBYTES)
*
* Returns 0 (success)
**************************************************/
int crypto_kem_keypair(unsigned char *pk, unsigned char *sk)
{
  size_t i;
  indcpa_keypair(pk, sk);
  for(i=0;i<KYBER_INDCPA_PUBLICKEYBYTES;i++)
    sk[i+KYBER_INDCPA_SECRETKEYBYTES] = pk[i];
  sha3_256(sk+KYBER_SECRETKEYBYTES-64, pk, KYBER_PUBLICKEYBYTES);
  randombytes(sk+KYBER_SECRETKEYBYTES-32, 32); /* Value z for pseudo-random output on reject */
  return 0;
}

/*************************************************
* Name:        crypto_kem_enc
*
* Description: Generates cipher text and shared
*              secret for given public key
*
* Arguments:   - unsigned char *ct:       pointer to output cipher text (of length KYBER_CIPHERTEXTBYTES)
*              - unsigned char *ss:       pointer to output shared secret (of length 32)
*              - const unsigned char *pk: pointer to input public key (of length KYBER_PUBLICKEYBYTES)
*
* Returns 0 (success)
**************************************************/
int crypto_kem_enc(unsigned char *ct,
                   unsigned char *ss,
                   const unsigned char *pk)
{
  unsigned char kr[64];
  unsigned char buf[64];

  randombytes(buf, 32);
  sha3_256(buf, buf, 32); /* Don't release system RNG output */

  sha3_256(buf+32, pk, KYBER_PUBLICKEYBYTES); /* Multitarget countermeasure for coins + contributory KEM */
  sha3_512(kr, buf, 64);

  indcpa_enc(ct, buf, pk, kr+32); /* coins are in kr+32 */

  sha3_256(kr+32, ct, KYBER_CIPHERTEXTBYTES); /* overwrite coins in kr with H(c) */
  sha3_256(ss, kr, 64); /* hash concatenation of pre-k and H(c) to k */
  return 0;
}


/*************************************************
* Name:        crypto_kem_dec
*
* Description: Generates shared secret for given
*              cipher text and private key
*
* Arguments:   - unsigned char *ss:       pointer to output shared secret (of length 32)
*              - const unsigned char *ct: pointer to input cipher text (of length KYBER_CIPHERTEXTBYTES)
*              - const unsigned char *sk: pointer to input private key (of length KYBER_SECRETKEYBYTES)
*
* Returns 0 for sucess or 1 for failure
*
* On failure, ss will contain a pseudo-random value.
**************************************************/
int crypto_kem_dec(unsigned char *ss,
                   const unsigned char *ct,
                   const unsigned char *sk)
{
  int fail;
  unsigned char cmp[KYBER_CIPHERTEXTBYTES];
  unsigned char buf[64];
  unsigned char kr[64];
  const unsigned char *pk = sk+KYBER_INDCPA_SECRETKEYBYTES;

  indcpa_dec(buf, ct, sk);

  /* Multitarget countermeasure for coins + contributory KEM */
  memcpy(buf+32, sk+KYBER_SECRETKEYBYTES-64, 32);
  sha3_512(kr, buf, 64);

  indcpa_enc(cmp, buf, pk, kr+32);

  fail = memcmp(ct, cmp, KYBER_CIPHERTEXTBYTES);

  sha3_256(kr+32, ct, KYBER_CIPHERTEXTBYTES); /* overwrite coins in kr with H(c) */

  /* Overwrite pre-k with z on re-encryption failure */
  cmov(kr, sk+KYBER_SECRETKEYBYTES-32, 32, fail);

  sha3_256(ss, kr, 64); /* hash concatenation of pre-k and H(c) to k */

  return fail;
}
