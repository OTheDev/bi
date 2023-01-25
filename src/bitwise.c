///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  bi_get_bit: get bit i of a (zero-based indexing), without regard for sign
 *              (i.e. as if a were nonnegative, but preserving its sign).
 ******************************************************************************/
bool
bi_get_bit(bi_t a, unsigned long i)
{
    int asize = ABS(a->n_digits);
    int digit_idx = i / BI_DIGIT_BITS;

    if (asize <= digit_idx)
    {
        return 0;
    }

    return (a->digits[digit_idx] >> (i % BI_DIGIT_BITS)) & 1;
}


/******************************************************************************
 *  bi_set_bit: set bit i of a (zero-based indexing), without regard for sign
 *              (i.e. as if a were nonnegative, but preserving its sign).
 *              i >= bi_bit_length(a) is permitted.
 ******************************************************************************/
void
bi_set_bit(bi_t a, unsigned long i)
{
    int asize, digit_idx, bit_idx;
    digit mask;

    asize = ABS(a->n_digits);

    digit_idx = i / BI_DIGIT_BITS;
    bit_idx = i % BI_DIGIT_BITS;

    mask = DIGIT_C(1) << bit_idx;

    if (digit_idx < asize)
    {
        a->digits[digit_idx] |= mask;
    }
    else
    {
        a->digits = bi_realloc(a, digit_idx + 1);
        a->n_digits = (a->n_digits < 0) ? -(digit_idx + 1) : digit_idx + 1;

        BI_MEMSET(a->digits + asize, digit_idx - asize, 0);

        a->digits[digit_idx] = mask;
    }
}
