///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Functions
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  bi_normalize: make sure ABS(b->n_digits) actually refers to the number of
 *                digits needed to represent the integer. Starts by checking
 *                b->digits[ABS(b->n_digits)-1] is zero until it finds the
 *                highest order nonzero digit.
 ******************************************************************************/
void
bi_normalize(bi_t b)
{
    int unchanged;
    int abs_n_digits = (unchanged = ABS(b->n_digits));
    while (abs_n_digits > 0 && b->digits[abs_n_digits - 1] == 0)
    {
        abs_n_digits--;
    }

    if (unchanged != abs_n_digits)
        b->n_digits = (b->n_digits) ? abs_n_digits : -abs_n_digits;
}
