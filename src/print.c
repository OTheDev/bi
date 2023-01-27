///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"

#include <stdlib.h>


///////////////////////////////////////////////////////////////////////////////
//  Printing
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  bi_print: print the base-10 representation of the integer plus a newline
 *            to standard output.
 ******************************************************************************/
void
bi_print(const bi_t a)
{
    char *s = bi_to_str(a);
    fprintf(stdout, "%s\n", s);
    free(s);
}


/******************************************************************************
 *  bi_print_internal: prints the integer in the form
 *
 *                     d_p * 2**(BI_SHIFT * p) + ... + d_0 * 2**(BI_SHIFT * 0),
 *
 *                     plus a newline, to standard output. Useful for debugging
 *                     and understanding the internal representation of the
 *                     integer.
 ******************************************************************************/
void
bi_print_internal(const bi_t a)
{
    int i;

    if (a->n_digits == 0) {
        printf("0 * 2**(%d * 0)\n", BI_SHIFT);
        return;
    }

    i = ABS(a->n_digits) - 1;

    if (a->n_digits < 0) {
        putchar('-');
    }

    printf("(%" BI_FSPEC " * 2**(%d * %d)", a->digits[i], BI_SHIFT, i);

    while (--i >= 0) {
        printf(" + %" BI_FSPEC " * 2**(%d * %d)", a->digits[i], BI_SHIFT, i);
    }

    printf(")\n");
}
