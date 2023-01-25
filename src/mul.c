///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"
#include "mul.h"

#include <string.h>


///////////////////////////////////////////////////////////////////////////////
//  Multiplication Functions
//
//  Grade-School Multiplication Algorithm:
//
//  Consider integers a = (a_(m-1)...a_0)_B, b = (b_(n-1)...b_0)_B > 0.
//  The product ab = (c_(m+n-1)...c_0)_B has at most (m + n) base B digits.
//
//  (1) For i from 0 to (m + n - 1): set c_i <- 0.
//  (2) For i from 0 to (n - 1):
//      (I) carry <- 0.
//      (II) For j from 0 to (m - 1):
//           Calculate (xy)_B = c_(i + j) + a_j * b_i + carry.
//           c_(i + j) <- y.
//           carry <- x.
//      (III)
//           c_(i + m) <- x.
//  (3) Return (c_(m+n-1)...c_0)_B.
//
//  TODO: for the case that the digits are the same between a and b, apply
//        a multiple-precision squaring algorithm, which can just about halve
//        the number of single-precision multiplications. Also, implement and
//        benchmark other general algorithms that have better asymptotic
//        complexities.
///////////////////////////////////////////////////////////////////////////////
void
bi_mul(bi_t c, const bi_t a, const bi_t b)
{
    int n_c_digits, m, n;
    digit x, y, carry, k;
    digit *p;

    /* Product is zero. */
    if (a->n_digits == 0 || b->n_digits == 0)
    {
        c->n_digits = 0;
        return;
    }

    m = ABS(a->n_digits);
    n = ABS(b->n_digits);

    /* Maximum number of digits in the result. TRUE: n_c_digits >= 2. */
    n_c_digits = m + n;

    /* Memory management plus ensuring result array is cleared. */
    if (c->digits == a->digits || c->digits == b->digits)
    {
        /* If overlapping memory, create a temporary array. */
        p = _calloc(n_c_digits, sizeof(digit));
    }
    else
    {
        if (c->n_alloc >= n_c_digits)
        {
            /* Just clear the array. */
            p = c->digits;
            memset(p, 0, n_c_digits * sizeof(digit));
        }
        else
        {
            /* On my system, realloc() + memset() is faster for builds with
             * optimizations disabled. However, for any nonzero optimization
             * level (with gcc), free() + calloc() is a lot faster. Here, we
             * code for builds that enable optimizations. */
            p = _calloc(n_c_digits, sizeof(digit));

            _free(c->digits);
            c->digits = p;
            c->n_alloc = n_c_digits;
        }
    }

    /* Grade-School Multiplication Algorithm. */
    for (int i = 0; i < n; i++)
    {
        carry = 0;
        for (int j = 0; j < m; j++)
        {
            k = i + j;
            MULT_2DIGITS_x_ADD_2DIGITS(a->digits[j], b->digits[i], p[k], carry,
                                       y, x);
            p[k] = y;
            carry = x;
        }
        p[i + m] = x;
    }

    /* Normalize. */
    while (n_c_digits > 1 && c->digits[n_c_digits - 1] == 0)
    {
        --n_c_digits;
    }

    c->n_digits = (a->n_digits ^ b->n_digits) < 0 ? -n_c_digits : n_c_digits;

    if (c->digits == a->digits || c->digits == b->digits)
    {
        /* Free destination memory and replace it with the temporary array. */
        _free(c->digits);
        c->digits = p;
        c->n_alloc = n_c_digits;
    }
}
