///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Routines
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 *  bi__rshift: Shifts digit vector `to_shift[0:size]` `shift_by` bits to the
 *              right, placing the result in `result` (which may be the same
 *              as `to_shift`). Returns the number of digits in `result`,
 *              which may be zero, indicating the value of the result is
 *              zero.
 *
 *              `size` > 0 assumed.
 *****************************************************************************/
int
bi__rshift(digit *result, digit *to_shift, int size, bi_bitcount_t shift_by)
{
    int size_result, hi, shift_bits;
    unsigned long shift_digits;
    twodigits sum;

    shift_digits = shift_by / BI_SHIFT;
    shift_bits = shift_by % BI_SHIFT;

    if ((unsigned) size <= shift_digits)
    {
        return 0;
    }

    size_result = size - shift_digits;

    /* TRUE: size_result >= 1. */

    hi = BI_SHIFT - shift_bits;

    sum = to_shift[shift_digits] >> shift_bits;

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
 *  bi_rshift: If a >= 0, equivalent to r = a >> shift_by.
 *             Otherwise, equivalent to r = (-1) * (|a| >> shift_by).
 ******************************************************************************/
void
bi_rshift(bi_t r, const bi_t a, bi_bitcount_t shift_by)
{
    int size_result, high, shift_bits, size;
    unsigned long shift_digits;
    twodigits sum;

    shift_digits = shift_by / BI_SHIFT;
    shift_bits = shift_by % BI_SHIFT;

    size = ABS(a->n_digits);

    if ((unsigned) size <= shift_digits)
    {
        r->n_digits = 0;
        return;
    }

    size_result = size - shift_digits;

    if (r->n_alloc < size_result)
    {
        r->digits = bi_realloc(r, size_result);
    }

    high = BI_SHIFT - shift_bits;

    sum = a->digits[shift_digits] >> shift_bits;
    for (int i = 0, j = shift_digits + 1; j < size; i++, j++)
    {
        sum += (twodigits)a->digits[j] << high;
        r->digits[i] = (digit)(sum & BI_MASK);
        sum >>= BI_SHIFT;
    }

    r->digits[size_result - 1] = (digit)sum;

    BI_NORMALIZE(r, size_result);

    return;
}
