///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Negate (to = -a)
///////////////////////////////////////////////////////////////////////////////
void
bi_negate(bi_t to, const bi_t a)
{
    if (to == a)
    {
        to->n_digits = -a->n_digits;
    }
    else
    {
        int abs_size_a = ABS(a->n_digits);

        /* If t->n_alloc < abs_size_a, malloc or realloc depending if 0 or
         * nonzero number of elements already allocated. */
        to->digits = BI_REALLOC_IF_NECESSARY(to, abs_size_a);

        /* Copy a's digits array to to's digits array */
        BI_DCOPY(to->digits, a->digits, abs_size_a);

        to->n_digits = -a->n_digits;
    }
}
