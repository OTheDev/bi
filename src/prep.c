///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"  /* bi_t */

#include <stdarg.h>


/******************************************************************************
 *  bi_prep: initializes the bi_t variable, setting its value to zero.
 ******************************************************************************/
void
bi_prep(bi_t a)
{
    /* Signifies bi has no associated dynamically-allocated memory. */
    a->n_alloc = 0;
    /* This is how we encode the value 0. */
    a->n_digits = 0;
    a->digits = NULL;
}


/******************************************************************************
 *  bi_preps: prepares a NULL-terminated list of bi_t variables, setting their
 *            value to zero.
 ******************************************************************************/
void
bi_preps(bi_t a, ...)
{
    va_list args;

    va_start(args, a);
    while (a != NULL)
    {
        a->n_alloc = a->n_digits = 0;
        a->digits = NULL;
        /* Can't pass bi_t here. */
        a = va_arg(args, _bi_struct *);
    }
    va_end(args);
}
