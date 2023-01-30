///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"
#include "bi_uint128_t.h"


///////////////////////////////////////////////////////////////////////////////
//  Helpers
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  bi_idiv10: divides the integer in-place by 10, returning the remainder.
 *
 *             TODO: use twodigits instead.
 ******************************************************************************/
uint8_t
bi_idiv10(bi_t a)
{
    bi_uint128_t current, Q, R;
    bi_uint128_t ten = {10, 0};
    bi_uint128_t base = {0, 1};
    int n_digits = ABS(a->n_digits);
    uint8_t carry = 0;

    for (int i = n_digits - 1; i >= 0; i--)
    {
        current = bi_uint128_t_add64(bi_uint128_t_mul64(base, carry),
                                     a->digits[i]);
        bi_uint128_t_div(current, ten, &Q, &R);
        a->digits[i] = Q.lo;
        carry = R.lo;
    }

    /* Decrement the number of digits if the most significant digit is 0. */
    if (n_digits > 1 && a->digits[n_digits - 1] == 0) n_digits--;

    a->n_digits = (a->n_digits >= 0) ? n_digits : -n_digits;

    return carry;
}


///////////////////////////////////////////////////////////////////////////////
//  Integer to String Conversion
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  bi_to_str: returns a null-terminated string of the base-10 representation
 *             of the integer.
 *
 *             The caller is responsible for freeing the string returned.
 ******************************************************************************/
char *
bi_to_str(const bi_t a)
{
    uint8_t carry;
    char *buf, *str;
    bi_t copy;
    size_t half_len, buf_size, idx, lower_bound;

    if (a->n_digits == 0)
    {
        buf = _malloc(2);
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* copy = |a| */
    bi_prep(copy);
    bi_set(copy, a);
    bi_abs(copy, copy);

    buf_size = _bi_decimal_length(a, &lower_bound);
    if (buf_size == 0)
    {
        if (lower_bound == 0)
        {
            fprintf(stderr, "%s(): too many digits!\n", __func__);
            exit(EXIT_FAILURE);
        }
        else
        {
            /* UNLIKELY we ever get here in practice. Currently, this would
             * make buf_size == 0x9a209aaa3ad1a bytes, which is about
             * 2_525_222 gigabytes! */
            buf_size = lower_bound;
        }
    }

    idx = 0;

    if (a->n_digits < 0)
    {
        buf_size += 2;
        buf = _malloc(buf_size);
        buf[idx++] = '-';
    }
    else
    {
        buf_size++;
        buf = _malloc(buf_size);
    }

    do {
        if (idx == buf_size - 1)
        {
            fprintf(stdout, "%s(): reallocating!\n", __func__);
            char *new_buf;
            size_t new_buf_size = (buf_size + (buf_size >> 2) + 3) & ~(size_t)3;

            if (new_buf_size < buf_size)
            {
                /* UNLIKELY we ever get here in practice. Note that if
                 * buf_size == 0 initially and lower_bound == 0x9a209aaa3ad1a
                 * (as what would be returned by _bi_decimal_length() when
                 * n_bits >  DBL_MAX_INT in _decimal_length.c) and a size_t is
                 * 64-bits, we would only enter here when buf_size ==
                 * 0xe27844e4c76652e8. That's about 15_198_131_091 gigabytes!
                 * We won't bother.
                 */
                fprintf(stderr, "%s(): too many digits!\n", __func__);
                exit(EXIT_FAILURE);
            }

            new_buf = _realloc(buf, new_buf_size);
            buf = new_buf;
            buf_size = new_buf_size;
        }

        carry = bi_idiv10(copy);
        buf[idx++] = '0' + carry;
    } while (copy->digits[copy->n_digits - 1]);

    /* Terminating null-character. */
    buf[idx] = '\0';

    if (a->n_digits < 0)
    {
        /* Make `str` point to first base10 digit (excl. sign). */
        str = buf + 1;
        /* Make idx represent number of included base10 digits (excl. sign). */
        idx--;
    }
    else
    {
        str = buf;
    }

    /* Reverse the string `str` in-place. */
    half_len = idx / 2;
    for (size_t i = 0; i < half_len; i++)
    {
        char temp = str[i];
        str[i] = str[idx - 1 - i];
        str[idx - 1 - i] = temp;
    }

    bi_free(copy);

    return buf;
}
