///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Copy
///////////////////////////////////////////////////////////////////////////////
void bi_set(bi_t to, const bi_t from)
{
    int from_n_digits = ABS(from->n_digits);

    /* If to->n_alloc < from_n_digits, reallocate and return new digit vector.
     * Otherwise, return to->digits. */
    to->digits = BI_REALLOC_IF_NECESSARY(to, from_n_digits);

    /* Copy `from_n_digits` elements from from->digits to to->digits. */
    BI_DCOPY(to->digits, from->digits, from_n_digits);

    to->n_digits = from->n_digits;
}
