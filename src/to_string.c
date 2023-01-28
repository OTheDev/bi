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
 ******************************************************************************/
char *
bi_to_str(const bi_t a)
{
    uint8_t carry;
    size_t half_len, n_base10_estimate, n_base10_included, buf_size, idx;
    char *buf, *str;
    bi_t copy;

    if (a->n_digits == 0)
    {
        buf = _malloc(2);
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    bi_prep(copy);

    /* Returns an estimate of the number of base-10 digits required. Almost
     * always, this will be correct. In the extremely rare case that an
     * underestimate is provided, no problem, this function detects this and
     * reallocates. */
    n_base10_estimate = _bi_decimal_length(a);

    /* Current position in the buffer. */
    idx = 0;

    if (a->n_digits < 0)
    {
        /* +2 for sign and terminating null character. */
        buf_size = n_base10_estimate + 2;
        buf = _malloc(buf_size);
        buf[idx++] = '-';
        str = buf + 1;
    }
    else
    {
        /* +1 for terminating null character. */
        buf_size = n_base10_estimate + 1;
        buf = _malloc(buf_size);
        str = buf;
    }

    n_base10_included = 0;
    bi_set(copy, a);

    do {
        if (n_base10_included == n_base10_estimate)
        {
            char *new_buf;
            buf_size += 2;
            n_base10_estimate += 2;
            new_buf = _realloc(buf, buf_size);
            buf = new_buf;
        }

        carry = bi_idiv10(copy);
        buf[idx++] = '0' + carry;
        n_base10_included++;
    } while (copy->digits[ABS(copy->n_digits) - 1]);

    buf[idx] = '\0';

    half_len = n_base10_included / 2;
    for (size_t i = 0; i < half_len; i++)
    {
        char temp = str[i];
        str[i] = str[n_base10_included - 1 - i];
        str[n_base10_included - 1 - i] = temp;
    }

    bi_free(copy);

    return buf;
}
