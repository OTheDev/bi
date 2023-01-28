///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"

#include <math.h>


///////////////////////////////////////////////////////////////////////////////
//  Functions to Estimate the Number of Decimal Digits Needed to Represent
//  The Integer.
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  _n_decimal_digits: returns an estimate of the number of decimal digits
 *                     required to represent any binary number that has
 *                     `n_bits` bits.
 *
 *                     TODO: ensure there are no overflow issues.
 ******************************************************************************/
size_t
_n_decimal_digits(unsigned long n_bits)
{
    /* 0.30103 is approximately log10(2) (slightly larger too) */
    return ceil(n_bits * 0.30103);
}


/******************************************************************************
 *  _bi_decimal_length: returns an estimate of the number of decimal digits
 *                      required to represent the integer.
 *
 *                      There are algorithms to determine the exact decimal
 *                      length, such as by repeatedly dividing the integer
 *                      by 10. However, the goal is to obtain an efficient
 *                      estimate of the number of digits required for
 *                      bi_to_str() that almost always returns a count
 *                      sufficient to represent its decimal version.
 *
 *                      TODO: adjust after resolving TODO in _n_decimal_digits.
 ******************************************************************************/
size_t
_bi_decimal_length(const bi_t a)
{
    return _n_decimal_digits(bi_bit_length(a));
}
