///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Routines
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 *  Shifts digit vector `to_shift[0:size]` `shift_by` bits to the right,
 *  placing the result in `result` (which may be the same as `to_shift`).
 *  Returns the size of result, which may be zero, indicating the value of
 *  the result is zero.
 *****************************************************************************/
unsigned
bi__rshift(digit *result, digit *to_shift, unsigned size, unsigned long shift_by)
{
    int size_result, hi, shift_bits;
    unsigned long shift_digits;
    twodigits sum;

    shift_digits = shift_by / BI_SHIFT;
    shift_bits = shift_by % BI_SHIFT;

    size_result = size - shift_digits;

    /* FIXME: this will never be negative. */
    if (size_result <= 0)
    {
        return 0;
    }

    /* TRUE: size_result >= 1. */

    hi = BI_SHIFT - shift_bits;

    sum = to_shift[shift_digits];
    sum >>= shift_bits;

    /* FIXME: cmp of integers of different size. */
    for (int i = 0, j = shift_digits + 1; j < size; i++, j++)
    {
        sum += (twodigits)to_shift[j] << hi;
        result[i] = (digit)(sum & BI_MASK);
        sum >>= BI_SHIFT;
    }

    result[size_result - 1] = (digit)sum;

    return (sum != 0) ? size_result : --size_result;
}


/******************************************************************************
 *  If a >= 0, equivalent to (a >>= shift_by).
 *  Otherwise, equivalent to (-1) * (|a| >>= shift_by).
 ******************************************************************************/
void
bi_irshift(bi_t a, unsigned long shift_by)
{
    int size = ABS(a->n_digits);

    /* Right-shift and get new size. */
    size = bi__rshift(a->digits, a->digits, size, shift_by);

    a->n_digits = (a->n_digits >= 0) ? size : -size;
}
