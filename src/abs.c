///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Absolute Value (to = |a|)
///////////////////////////////////////////////////////////////////////////////
void
bi_abs(bi_t to, const bi_t a)
{
    if (to == a)
    {
        to->n_digits = ABS(to->n_digits);
    }
    else
    {
        int abs_size_a = ABS(a->n_digits);

        /* If t->n_alloc < abs_size_a, malloc or realloc depending if 0 or
         * nonzero number of elements already allocated. */
        to->digits = bi_realloc_if_necessary(to, abs_size_a);

        /* Copy a's digits array to to's digits array */
        bi_dcopy(to->digits, a->digits, abs_size_a);

        to->n_digits = abs_size_a;
    }
}
