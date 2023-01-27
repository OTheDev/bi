///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"

#include <stdlib.h>


///////////////////////////////////////////////////////////////////////////////
//  Routines
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 *  bi_lshift: result = (a << shift) if a >= 0;
 *             result = (-1) * (|a| << shift), if a < 0.
 *****************************************************************************/
void
bi_lshift(bi_t result, const bi_t a, bi_bitcount_t shift)
{
    unsigned i, j, a_size, shift_bits;
    unsigned long result_size, shift_digits;
    twodigits sum;
    digit *p;

    if (a->n_digits == 0)
    {
        result->n_digits = 0;
        return;
    }

    if (shift == 0)
    {
        if (result->digits != a->digits)
        {
            bi_set(result, a);
        }
        return;
    }

    shift_digits = shift / BI_SHIFT;
    shift_bits = shift % BI_SHIFT;
    a_size = ABS(a->n_digits);

    if (shift_bits)
    {
        /* shift_digits + 1 will never overflow */
        result_size = a_size + (shift_digits + 1);
    }
    else
    {
        result_size = a_size + shift_digits;
    }

    if (result_size < a_size || result_size > BI_MAX_DIGITS)
    {
        fprintf(stderr, "bi_lshift(): overflow! \n");
        exit(EXIT_FAILURE);
    }

    /* True: result_size <= BI_MAX_DIGITS where BI_MAX_DIGITS :=
     * min(INT_MAX, (ULONG_MAX/BI_SHIFT), (SIZE_MAX/sizeof(digit)). */

    /* Memory Management. */
    if (result->digits == a->digits)
    {
        /* Overlapping memory. Create temporary array. */
        p = _malloc(result_size * sizeof(digit));
    }
    else
    {
        if (result_size > (unsigned)result->n_alloc)
        {
            result->digits = _realloc(result->digits,
                                      result_size * sizeof(digit));
            result->n_alloc = result_size;
        }

        p = result->digits;
    }

    for (i = 0; i < shift_digits; i++)
    {
        p[i] = 0;
    }

    sum = 0;
    for (j = 0; j < a_size; i++, j++)
    {
        sum |= (twodigits)a->digits[j] << shift_bits;
        p[i] = (digit)(sum & BI_MASK);
        sum >>= BI_SHIFT;
    }

    if (shift_bits)
    {
        p[result_size - 1] = (digit)sum;
    }

    if (result->digits == a->digits)
    {
        _free(result->digits);
        result->digits = p;
        result->n_alloc = result_size;
    }

    BI_NORMALIZE(result, result_size);
}
