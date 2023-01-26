///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"

#include <stdlib.h>
#include <string.h>


///////////////////////////////////////////////////////////////////////////////
//  Root Memory Allocation/Deallocation Functions
//
//  These are simply wrappers for malloc(), calloc(), realloc(), and free().
//  They have the same interface, except that, if allocation/reallocation
//  fails, the program terminates.
///////////////////////////////////////////////////////////////////////////////
void *
_malloc(size_t size)
{
    void *r = malloc(size);
    if (r == NULL)
    {
        fprintf(stderr, "_malloc(): failed allocating memory.\n");
        exit(EXIT_FAILURE);
    }
    return r;
}

void *
_calloc(size_t nmemb, size_t size)
{
    void *r = calloc(nmemb, size);
    if (r == NULL)
    {
        fprintf(stderr, "_calloc(): failed allocating memory.\n");
        exit(EXIT_FAILURE);
    }
    return r;
}

void *
_realloc(void *ptr, size_t size)
{
    void *r = realloc(ptr, size);
    if (r == NULL)
    {
        fprintf(stderr, "_realloc(): failed reallocating memory.\n");
        exit(EXIT_FAILURE);
    }
    return r;
}

void
_free(void *ptr)
{
    free(ptr);
}

void *
_recalloc(void *ptr, size_t new_size, size_t old_size)
{
    void *r = realloc(ptr, new_size);

    if (r == NULL)
    {
        fprintf(stderr, "_recalloc(): failed reallocating memory.\n");
        exit(EXIT_FAILURE);
    }

    if (new_size > old_size)
    {
        /* C Standard (C11, §7.22.4.1): "[a]ny bytes in the new object beyond
         * the size of the old object have indeterminate values."
         */

        /* Clear any extra memory that was allocated.  */
        memset((char *)r + old_size, 0, new_size - old_size);
    }

    return r;
}


///////////////////////////////////////////////////////////////////////////////
//  Allocator/Reallocator
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  bi_realloc: sets the number of allocated digits to `size`. Returns a
 *              pointer to the new digits array (which may have the same value
 *              as the pointer to the old digits array).
 *
 *              May modify `->digits` and `->n_alloc` but never `->n_digits`.
 ******************************************************************************/
void *
bi_realloc(bi_t ptr, bi_ssize_t size)
{
    digit *digits;

    if (size < 1)
    {
        size = 1;
    }

    if (size > BI_MAX_DIGITS)
    {
        fprintf(stderr, "bi_realloc(): failure.\n");
        exit(EXIT_FAILURE);
    }

    /* FIXME: the multiplication can overflow. */
    if (ptr->n_alloc == 0)
    {
        digits = _malloc(size * sizeof(digit));
    }
    else
    {
        digits = _realloc(ptr->digits, size * sizeof(digit));
    }

    ptr->digits = digits;
    ptr->n_alloc = size;
    return (void *)digits;
}
