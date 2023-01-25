///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Comparisons
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  bi_cmp: if a < b, return a negative number;
 *          if a == b, return zero;
 *          if a > b, return a positive number.
 ******************************************************************************/
int
bi_cmp(const bi_t a, const bi_t b)
{
    if (a->n_digits > b->n_digits)
    {
        return 1;
    }
    else if (a->n_digits < b->n_digits)
    {
        return -1;
    }
    else
    {
        int i = ABS(a->n_digits);
        while (--i >= 0)
        {
            if (a->digits[i] != b->digits[i])
            {
                if (a->n_digits < 0)
                {
                    return (a->digits[i] > b->digits[i]) ? -1 : 1;
                }
                else
                {
                    return a->digits[i] > b->digits[i] ? 1 : -1;
                }
            }
        }
        return 0;
    }
}
