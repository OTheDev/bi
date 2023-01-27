///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Division Routines
//
//  "The result of the / operator is the quotient from the division of the
//   first operand by the second; the result of the % operator is the
//   remainder. In both operations, if the value of the second operand is zero,
//   the behavior is undefined...When integers are divided, the result of the
//   / operator is the algebraic quotient with any fractional part discarded.
//   If the quotient a/b is representable, the expression (a/b)*b + a%b shall
//   equal a; otherwise, the behavior of both a/b and a%b is undefined"
//   (C11, §6.5.5).
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  bi_divide_by_digit: divide N by D, storing the quotient in Q (bi_t) and
 *                      remainder in the digit object pointed to by R, without
 *                      regard for the sign.
 ******************************************************************************/
void
bi_divide_by_digit(bi_t Q, digit *R, bi_t N, digit D)
{
    twodigits tmp;
    int size = ABS(N->n_digits);

    if (Q->n_alloc < size)
    {
        Q->digits = _realloc(Q->digits, size * sizeof(digit));
        Q->n_alloc = size;
    }

    *R = 0;

    for (int i = size - 1; i >= 0; i--)
    {
        tmp = ((twodigits)*R << BI_SHIFT) | N->digits[i];
        Q->digits[i] = (digit)(tmp / D);
        *R = tmp % D;
    }

    BI_NORMALIZE_NONNEG(Q, size);
}


/******************************************************************************
 *  bi_divide_qr: Q = N / D, R = N % D, in one pass.
 *
 *               As with standard C, it is the responsibility of the caller to
 *               allow or disallow division by zero. In the event that the
 *               divisor is zero, division by zero is attempted, in order to
 *               be consistent with normal integer division in C.
 *
 *               TODO: this is implemented using binary long division.
 *                     Implement and benchmark alternative algorithms such as
 *                     Knuth's Algorithm D.
 *
 *  Binary Long Division Algorithm
 *  --------------------------------------------------------------------------
 *  Let n denote the number of bits in N. Let X(i) denote the ith bit of X
 *  (zero-based indexing; zero gives the LSB).
 *
 *  (1) if D = 0, then handle division by zero.
 *  (2) Q := 0; R := 0.
 *  (3) for i from (n - 1) to 0:
 *      (3)(I)   R := R << 1.
 *      (3)(II)  R(0) := N(i).
 *      (3)(III) if R >= D then
 *               (a) R := R - D.
 *               (b) Q(i) := 1.
 *  (4) Return Q and R.
 ******************************************************************************/
void
bi_divide_qr(bi_t Q, bi_t R, bi_t N, bi_t D)
{
    int size_N, size_D, size_Q;

    /* (1). Check Division By Zero */
    if (D->n_digits == 0)
    {
        /* Avoid division by zero warning by GCC. */
        int x = 0;
        fprintf(stderr, "bi_divide_qr(): integer division by zero attempt "
                        "detected. Manually dividing by zero.\n");
        x = 1 / x;

        return;
    }

    size_N = ABS(N->n_digits);
    size_D = ABS(D->n_digits);

    /* Handle |N| < |D| case */
    if (size_N < size_D || (size_N == size_D &&
                            N->digits[size_N - 1] < D->digits[size_D - 1]))
    {
        /* |N| < |D| ==> mathematically |N| / |D| < 1 ==> |N| / |D| = 0. */
        Q->n_digits = 0;

        /* Computation (a/b)*b + a%b should equal a. Thus a%b is a. */
        bi_set(R, N);

        return;
    }

    /* TRUE: size_N >= size_D > 0 */

    /* TODO?: can use division by digit routine above to handle size_D == 1
     * case. */

    /* Ensure there is enough space in Q and R. This reallocation shouldn't
     * be necessary as the functions called within the loop should reallocate
     * when necessary. On the one hand this should save us from multiple calls
     * to realloc() (both within this function and subsequently). On the other
     * hand, sometimes this overallocates. I am definitely leaning towards the
     * former. Perhaps benchmarking the differences on random data can be
     * useful. */
    size_Q = size_N - size_D + 1;
    if (Q->n_alloc < size_Q) bi_realloc(Q, size_Q);
    if (R->n_alloc < size_D) bi_realloc(R, size_D);

    /* (2) */
    Q->n_digits = 0;
    R->n_digits = 0;

    /* (3) */
    for (unsigned long i = bi_bit_length(N) - 1; i < ULONG_MAX; i--)
    {
        /* (3)(I). This normalizes R. */
        bi_lshift(R, R, 1);

        /* (3)(II) */
        if (bi_get_bit(N, i))
        {
            /* This normalizes R. */
            bi_set_bit(R, 0);
        }

        /* (3)(III) */
        if (bi_cmp(R, D) >= 0)
        {
            /* (3)(III)(a). This normalizes R. */
            bi_sub(R, R, D);

            /* (3)(III)(b). This normalizes Q. */
            bi_set_bit(Q, i);
        }
    }

    /* a/b == trunc(a/b). */
    if ((N->n_digits ^ D->n_digits) < 0)
    {
        /* Signs of numerator, denominator different ==> quotient negative. */
        Q->n_digits = -Q->n_digits;
    }

    /* a%b is 0 or has the sign of a. */
    if (N->n_digits < 0 && R->n_digits)
    {
        R->n_digits = -R->n_digits;
    }
}
