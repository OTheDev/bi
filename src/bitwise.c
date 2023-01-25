///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Getting/Setting a Bit
//
//  Note: when getting/setting a bit, these functions act as if the integer
//        is nonnegative with the least significant bit at index 0. When
//        setting a bit, the resulting integer preserves its sign (after its
//        absolute value has been changed, if at all).
//
//        i >= bi_bit_length(a) is permitted.
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  bi_get_bit: get/test bit i of a.
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
 *  bi_set_bit: set bit i of a.
 ******************************************************************************/
void
bi_set_bit(bi_t a, unsigned long i)
{
    int asize, digit_idx;

    asize = ABS(a->n_digits);
    digit_idx = i / BI_DIGIT_BITS;

    if (digit_idx < asize)
    {
        a->digits[digit_idx] |= DIGIT_C(1) << (i % BI_DIGIT_BITS);
    }
    else
    {
        a->digits = bi_realloc(a, digit_idx + 1);
        a->n_digits = (a->n_digits < 0) ? -(digit_idx + 1) : digit_idx + 1;

        BI_MEMSET(a->digits + asize, digit_idx - asize, 0);

        a->digits[digit_idx] = DIGIT_C(1) << (i % BI_DIGIT_BITS);
    }
}
