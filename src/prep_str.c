///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"

#include <math.h>
#include <ctype.h>


///////////////////////////////////////////////////////////////////////////////
//  Lookup Table for "From String" Function
///////////////////////////////////////////////////////////////////////////////
/* BI_DECSHIFT == max{e in N ∪ {0}: 10^e fits in a digit}.
 * POWERS_OF_TEN[i] == 10^i.
 */
#if BI_DECSHIFT == 9
    static digit POWERS_OF_TEN[10] = {
        DIGIT_C(1),
        DIGIT_C(10),
        DIGIT_C(100),
        DIGIT_C(1000),
        DIGIT_C(10000),
        DIGIT_C(100000),
        DIGIT_C(1000000),
        DIGIT_C(10000000),
        DIGIT_C(100000000),
        DIGIT_C(1000000000)
    };

#elif BI_DECSHIFT == 19
    static digit POWERS_OF_TEN[20] = {
        DIGIT_C(1),
        DIGIT_C(10),
        DIGIT_C(100),
        DIGIT_C(1000),
        DIGIT_C(10000),
        DIGIT_C(100000),
        DIGIT_C(1000000),
        DIGIT_C(10000000),
        DIGIT_C(100000000),
        DIGIT_C(1000000000),
        DIGIT_C(10000000000),
        DIGIT_C(100000000000),
        DIGIT_C(1000000000000),
        DIGIT_C(10000000000000),
        DIGIT_C(100000000000000),
        DIGIT_C(1000000000000000),
        DIGIT_C(10000000000000000),
        DIGIT_C(100000000000000000),
        DIGIT_C(1000000000000000000),
        DIGIT_C(10000000000000000000)
    };
#endif


///////////////////////////////////////////////////////////////////////////////
//  From String
//
//  The largest integer representable by an n digit integer in base b is
//      b^n - 1.
//
//  To find the number of base b digits needed to represent any m-digit
//  integer in base c, we want the smallest natural number n such that
//      b^n - 1             >= c^m - 1
//      b^n                 >= c^m
//      (b^n)/(c^m)         >= 1
//      log(b^n) - log(c^m) >= 0
//      n                   >= m log(c) / log(b)
//
//  n = ceil(m log(c) / log(b)).
//
//  When base is 2^64, we can't portably rely on a double, so we precompute it.
//  In the extremely rare case we do not allocate enough digits for z, no
//  problem, we reallocate.
///////////////////////////////////////////////////////////////////////////////
void
bi_prep_str(bi_t z, const char *str)
{
    /* Number of base (BI_MASK + 1) digits in result integer. */
    int n_digits;
    /* `scan` and `str` will be combined to get the number of decimal digits
     * in the string (`scan` - `str`). */
    const char *scan;
    bool negative = false;
  /* log(10) / log(2^BI_SHIFT) where 2^BI_SHIFT is the base. These are slight
   * overestimates to be safe. */
  #if BI_SHIFT == 64
    static const double conversion_ratio = 0.052;
  #elif BI_SHIFT == 32
    static const double conversion_ratio = 0.104;
  #endif

    /* Allow leading whitespace. */
    while (isspace(*str))
    {
        str++;
    }

    /* Allow plus/minus sign to precede first decimal digit. */
    if (*str == '+')
    {
        str++;
    }
    else if (*str == '-')
    {
        str++;
        negative = true;
    }

    /* `str` should now be pointing to the first decimal digit. */
    scan = str;

    while ('0' <= *scan && *scan <= '9')
    {
        scan++;
    }

    if (scan == str)
    {
        fprintf(stderr, "bi_prep_str(): error parsing string.\n");

        z->n_alloc = 0;
        z->n_digits = 0;
        z->digits = NULL;

        return;
    }

    /* `scan` should now be pointing to the first nondecimal digit. */

    /* Number of base (BI_MASK + 1) digits required to represent any base 10
     * number with `scan - str` base 10 digits. */
    n_digits = (int)ceil( (scan - str) * (conversion_ratio) );

    z->digits = _malloc(n_digits * sizeof(digit));
    z->n_alloc = n_digits;
    z->n_digits = 0;

    while (str < scan)
    {
        int e;
        digit *ptr_z, *ptr_zend;
        twodigits carry, shift_factor;

        carry = *str++ - '0';
        e = 1;
        while (e < BI_DECSHIFT && str < scan)
        {
            carry = (twodigits)(carry * 10 + (*str++ - '0'));
            e++;
        }

        shift_factor = POWERS_OF_TEN[e];
        ptr_z = z->digits;

        ptr_zend = ptr_z + z->n_digits;

        while (ptr_z < ptr_zend)
        {
            carry += (twodigits)*ptr_z * shift_factor;
            *ptr_z = (digit)(carry & BI_MASK);
            carry >>= BI_SHIFT;

            ptr_z++;
        }

        if (carry)
        {
            if (z->n_digits < n_digits)
            {
                *ptr_z = (digit)carry;
                z->n_digits++;
            }
            else
            {
                n_digits++;

                fprintf(stdout, "bi_prep_str(): reallocating!\n");

                z->digits = _realloc(z->digits, n_digits * sizeof(digit));
                z->digits[n_digits - 1] = (digit)carry;
                z->n_alloc = n_digits;

                z->n_digits++;
            }
        }
    }

    BI_NORMALIZE_POSITIVE(z, z->n_digits, true);

    if (negative)
    {
        z->n_digits = -z->n_digits;
    }
}
