///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"

#include <math.h>
#include <assert.h>


///////////////////////////////////////////////////////////////////////////////
//  Macro Constants and Assumptions
///////////////////////////////////////////////////////////////////////////////
/* 2^53 */
#define DBL_MAX_INT 0x20000000000000u
/* Approximately (slightly larger than) log10(2). */
#define LOG10_2 0.30103

/* Assume the implementation has mostly-compliant IEEE-754 64-bit doubles. This
 * is almost always true. */
static_assert(sizeof(double) * CHAR_BIT == 64, "64-bit double is assumed.");


///////////////////////////////////////////////////////////////////////////////
//  Functions to Estimate the Number of Decimal Digits Needed to Represent
//  The Integer.
//
//  Ultimately, the functions in this file are intended to serve as helpers for
//  bi_to_str().
//
//  The below is definitely messy and there probably is a way to show that this
//  can be greatly simplified, but it at least provides some assurances.
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  _n_decimal_digits: returns an estimate of the number of decimal digits
 *                     required to represent any binary number that has
 *                     `n_bits` bits or 0 if it could not be determined.
 *
 *                     If 0 is returned, the caller should check the value of
 *                     the object pointed to by lower_bound; if it is also 0,
 *                     then the estimate would exceed SIZE_MAX - 2; otherwise,
 *                     use the value of the object pointed to by lower_bound as
 *                     a large lower bound / initial estimate.
 ******************************************************************************/
#define SAFE(n_bits, lower_bound)                       \
    do {                                                \
        unsigned long n = floor(n_bits * LOG10_2) + 1;  \
        if (n <= SIZE_MAX - 2)                          \
        {                                               \
            return n;                                   \
        }                                               \
        else                                            \
        {                                               \
            *lower_bound = 0;                           \
            return 0;                                   \
        }                                               \
    } while (0)

size_t
_n_decimal_digits(bi_bitcount_t n_bits, size_t *lower_bound)
{
  #if ULONG_MAX <= DBL_MAX_INT
    /* Any `n_bits` safely fits in an IEEE-754 64-bit double. */
    SAFE(n_bits, lower_bound);
  #else
    if (n_bits <= DBL_MAX_INT)
    {
        SAFE(n_bits, lower_bound);
    }
    else
    {
        /* TRUE: ULONG_MAX >= n_bits > DBL_MAX_INT. Unlikely we would ever get
         * here in practice. 0x9a209aaa3ad1a == floor(DBL_MAX_INT * LOG10_2) + 1
         * bytes is about 2_525_222 gigabytes! */
      #if 0x9a209aaa3ad1aUL <= SIZE_MAX - 2
        *lower_bound = 0x9a209aaa3ad1aUL;
      #else
        *lower_bound = 0;
      #endif
        return 0;
    }
  #endif
}


/******************************************************************************
 *  _bi_decimal_length: returns an estimate of the number of decimal digits
 *                      required to represent an integer or 0 if it could not
 *                      be determined.
 *
 *                      If 0 is returned, the caller should check the value of
 *                      the object pointed to by lower_bound; if it is also 0,
 *                      then the estimate would exceed SIZE_MAX - 2; otherwise,
 *                      use the value of the object pointed to by lower_bound as
 *                      a large lower bound / initial estimate.
 ******************************************************************************/
size_t
_bi_decimal_length(const bi_t a, size_t *lower_bound)
{
    size_t lb;
    size_t ret;

    ret = _n_decimal_digits(bi_bit_length(a), &lb);
    if (ret == 0)
    {
        *lower_bound = lb;
    }
    return ret;
}
