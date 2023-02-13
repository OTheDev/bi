///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Addition/Subtraction
//      The algorithms employed here makes use of one important fact guaranteed
//      by the C Standard.
//
//      From the C11 Standard (§6.2.5), "[a] computation involving unsigned
//      operands can never overflow, because a result that cannot be
//      represented by the resulting unsigned integer type is reduced modulo
//      the number that is one greater than the largest value that can be
//      represented by the resulting type."
//
//      Consider adding/subtracting two unsigned integers with width `w`. Let
//      UADD(a, b) denote the (computational) sum and USUB(a, b) the difference
//      between a and b. Then
//          UADD(a, b) = a + b,       if a + b < 2^w
//                       a + b - 2^w, if a + b >= 2^w,
//
//          USUB(a, b) = a - b        if a - b >= 0
//                       a - b + 2^w  if a - b < 0.
//
//      Theorem: UADD(a, b) < a iff there is overflow.
//      Proof:
//      (1) Assume there is overflow (i.e. a + b >= 2^w). Then by definition
//          UADD(a, b) = a + b - 2^w < a + 0 = a (since b < 2^w).
//          Thus,
//              (overflow) ==> (UADD(a, b) < a).
//      (2) Assume there is no overflow. Then UADD(a, b) = a + b >= a (since
//          b >= 0). By logical equivalence of the contrapositive, we have
//          that
//              (UADD(a, b) < a) ==> overflow is true. QED.
//
//      By symmetry, there is overflow iff UADD(a, b) < b.
//
//      Theorem: (a < b) <==> overflow caused by the subtraction of a from b.
//      Proof: By definition, there is overflow iff a - b < 0 <==> a < b. QED.
///////////////////////////////////////////////////////////////////////////////
/* Sets c to |a| + |b|. |a->n_digits| >= |b->n_digits| is assumed.
 * Fact: if a (m digits), b (n digits) are nonnegative integers, a + b has
 *       at most max(m, n) + 1 digits.
 */
#define ADD(c, a, b, abs_size_a, abs_size_b)                                  \
    do {                                                                      \
        /* Temporary variable in case c overlaps with a or b */               \
        digit tmp;                                                            \
        uint8_t carry;                                                        \
        /* This sum should never overflow as reasonable to assume INT_MAX <   \
           UINT_MAX. */                                                       \
        unsigned i, max_digits = (unsigned)abs_size_a + 1;                    \
        if (max_digits > BI_MAX_DIGITS)                                       \
        {                                                                     \
            BI_ON_OVERFLOW();                                                 \
            return;                                                           \
        }                                                                     \
        if ((unsigned)c->n_alloc < max_digits)                                \
        {                                                                     \
            bi_realloc(c, max_digits);                                        \
        }                                                                     \
        carry = 0;                                                            \
        for (i = 0; i < abs_size_b; i++)                                      \
        {                                                                     \
            tmp = a->digits[i] + b->digits[i] + carry;                        \
            /* FIXME: if b->digits[i] == BI_MASK and carry == 1, this code    \
             * fails. */                                                      \
            carry = tmp < a->digits[i];                                       \
            c->digits[i] = tmp;                                               \
        }                                                                     \
        while (i < abs_size_a)                                                \
        {                                                                     \
            tmp = a->digits[i] + carry;                                       \
            carry = tmp < carry;                                              \
            c->digits[i++] = tmp;                                             \
        }                                                                     \
        if (carry)                                                            \
        {                                                                     \
            c->digits[i] = carry;                                             \
            c->n_digits = max_digits;                                         \
        }                                                                     \
        else                                                                  \
        {                                                                     \
            BI_NORMALIZE_NONNEG(c, abs_size_a);                               \
        }                                                                     \
    } while (0)

/* Performs |a| - |b|, storing the result into c, which may overlap with either
 * a or b. Assumes a > b.
 * Fact: if a (m digits), b (n digits) are nonnegative integers with m >= n,
 * then a - b has at most m digits.
 */
#define SUB(c, a, b, abs_size_a, abs_size_b)                    \
    do {                                                        \
        /* Temporary variable in case c overlaps with a or b */ \
        digit tmp;                                              \
        int i;                                                  \
        bool borrow = 0;                                        \
        if (c->n_alloc < abs_size_a)                            \
            bi_realloc(c, abs_size_a);                          \
        for (i = 0; i < abs_size_b; i++)                        \
        {                                                       \
            tmp = a->digits[i] - b->digits[i] - borrow;         \
            borrow = tmp > a->digits[i];                        \
            c->digits[i] = tmp;                                 \
        }                                                       \
        while (i < abs_size_a)                                  \
        {                                                       \
            tmp = a->digits[i] - borrow;                        \
            borrow = tmp > a->digits[i];                        \
            c->digits[i++] = tmp;                               \
        }                                                       \
    } while (0)


///////////////////////////////////////////////////////////////////////////////
//  Functions that Add/Subtract Absolute Values of Two Numbers
///////////////////////////////////////////////////////////////////////////////
/* Returns |a| + |b| */
void
bi_add_abs(bi_t to, const bi_t a, const bi_t b)
{
    unsigned a_n, b_n;

    a_n = ABS(a->n_digits);
    b_n = ABS(b->n_digits);

    if (a_n >= b_n)
    {
        ADD(to, a, b, a_n, b_n);
    }
    else
    {
        ADD(to, b, a, b_n, a_n);
    }
}

/* Returns |a| - |b| */
void
bi_sub_abs(bi_t to, const bi_t a, const bi_t b)
{
    int a_n, b_n;

    a_n = ABS(a->n_digits);
    b_n = ABS(b->n_digits);

    if (a_n == b_n)
    {
        int j;

        /* Consider the value of j after this for loop is executed.
         * (j >= 0) ==> j is the first index in digits array (starting from the
         * most significant digit) at which the digits are not the same between
         * a and b.
         * (j < 0) ==> all digits the same ==> |a| - |b| = 0.
         */
        for (j = a_n - 1; j >= 0 && a->digits[j] == b->digits[j]; j--)
            ;

        if (j < 0)
        {
            to->n_digits = 0;
            return;
        }

        if (a->digits[j] < b->digits[j])
        {
            SUB(to, b, a, b_n, a_n);
            BI_NORMALIZE_NEG(to, b_n);
        }
        else
        {
            SUB(to, a, b, a_n, b_n);
            BI_NORMALIZE_NONNEG(to, a_n);
        }
    }
    else if (a_n > b_n)
    {
        /* a_n > b_n ==> |a| - |b| > 0 */
        SUB(to, a, b, a_n, b_n);
        BI_NORMALIZE_NONNEG(to, a_n);

    }
    else
    {
        /* a_n < b_n ==> |a| - |b| < 0. Note |a| - |b| = -(|b| - |a|) always. */
        SUB(to, b, a, b_n, a_n);
        BI_NORMALIZE_NEG(to, b_n);
    }
}


///////////////////////////////////////////////////////////////////////////////
//  General Add/Subtract Functions
///////////////////////////////////////////////////////////////////////////////
/* r = a + b */
void
bi_add(bi_t r, const bi_t a, const bi_t b)
{
    if (a->n_digits < 0)
    {
        if (b->n_digits < 0)
        {
            /* a < 0, b < 0 ==> a = -|a|, b = -|b|. ==> a + b = -(|a| + |b|). */
            bi_add_abs(r, a, b);
            r->n_digits = -(r->n_digits);
        }
        else
        {
            /* a < 0, b >= 0 ==> a = -|a|, b = |b| ==> a + b = |b| - |a|. */
            bi_sub_abs(r, b, a);
        }
    }
    else
    {
        if (b->n_digits < 0)
        {
            /* a >= 0, b < 0 ==> a = |a|, b = -|b| ==> a + b = |a| - |b|. */
            bi_sub_abs(r, a, b);
        }
        else
        {
            /* a >= 0, b >= 0 ==> a = |a|, b = |b| ==> a + b = |a| + |b|. */
            bi_add_abs(r, a, b);
        }
    }
}

/* r = a - b */
void
bi_sub(bi_t r, const bi_t a, const bi_t b)
{
    if (a->n_digits < 0)
    {
        if (b->n_digits < 0)
        {
            /* a < 0, b < 0 ==> a = -|a|, b = -|b| ==> a - b = |b| - |a|. */
            bi_sub_abs(r, b, a);
        }
        else
        {
            /* a < 0, b >= 0 ==> a = -|a|, b = |b| ==> a - b = -(|a| + |b|). */
            bi_add_abs(r, a, b);
            r->n_digits = -(r->n_digits);
        }
    }
    else
    {
        if (b->n_digits < 0)
        {
            /* a >= 0, b < 0 ==> a = |a|, b = -|b| ==> a - b = |a| + |b|. */
            bi_add_abs(r, a, b);
        }
        else
        {
            /* a >= 0, b >= 0 ==> a = |a|, b = |b| ==> a - b = |a| - |b|. */
            bi_sub_abs(r, a, b);
        }
    }
}
