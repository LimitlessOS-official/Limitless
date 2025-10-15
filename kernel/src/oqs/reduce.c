#include "reduce.h"
#include "kyber_params.h"

/*************************************************
* Name:        montgomery_reduce
*
* Description: Montgomery reduction; given a 32-bit integer a, computes
*              a*2^-16 mod q.
*
* Arguments:   - int32_t a: input integer to be reduced; has to be in {-q2^15,...,q2^15-1}
*
* Returns:     a*2^-16 mod q
**************************************************/
int16_t montgomery_reduce(int32_t a)
{
  int32_t t;
  int16_t u;

  u = a * QINV;
  t = (int32_t)u * KYBER_Q;
  t = a - t;
  t >>= 16;
  return t;
}

/*************************************************
* Name:        barrett_reduce
*
* Description: Barrett reduction; given a 16-bit integer a, computes
*              a mod q in constant time.
*
* Arguments:   - int16_t a: input integer to be reduced
*
* Returns:     a mod q
**************************************************/
int16_t barrett_reduce(int16_t a) {
    int32_t t;
    const int32_t v = ((1U << 26) + KYBER_Q / 2) / KYBER_Q;

    t = v * a;
    t >>= 26;
    t *= KYBER_Q;
    return a - t;
}
