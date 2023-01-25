///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"

#include <stdarg.h>


///////////////////////////////////////////////////////////////////////////////
//  Deallocating Memory
///////////////////////////////////////////////////////////////////////////////
/* Frees the dynamically allocated memory of `a`. */
void
bi_free(bi_t a)
{
    if (a->n_alloc)
    {
        _free(a->digits);
    }
}

/* Frees the dynamically allocated memory of the NULL-terminated list of bi_t
 * variables. */
void
bi_frees(bi_t a, ...)
{
    va_list args;
    va_start(args, a);
    while (a != NULL)
    {
        if (a->n_alloc)
        {
            _free(a->digits);
        }
        /* Can't pass bi_t here. */
        a = va_arg(args, _bi_struct *);
    }
    va_end(args);
}
