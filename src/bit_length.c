///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"

#include <limits.h> /* ULONG_MAX, CHAR_BIT */


///////////////////////////////////////////////////////////////////////////////
//  Bit Length
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  bit_length_digit: Given a `digit`, return the number of bits required to
 *                    represent it.
 ******************************************************************************/
static uint8_t
_bit_length_digit(digit number)
{
    uint8_t n_bits = 1;
    while (number >>= 1)
    {
        n_bits++;
    }
    return n_bits;
}

static uint8_t
_bit_length_u32(uint32_t number)
{
    uint8_t n_bits = 1;
    if ((number & 0xffff0000) != 0) {
        number >>= 16;
        n_bits += 16;
    }
    if ((number & 0xff00) != 0) {
        number >>= 8;
        n_bits += 8;
    }
    if ((number & 0xf0) != 0) {
        number >>= 4;
        n_bits += 4;
    }
    if ((number & 0xc) != 0) {
        number >>= 2;
        n_bits += 2;
    }
    if ((number & 0x2) != 0) {
        n_bits += 1;
    }
    return n_bits;
}

static uint8_t
_bit_length_u64(uint64_t number)
{
    uint8_t n_bits = 1;
    if ((number & 0xffffffff00000000) != 0)
    {
        number >>= 32;
        n_bits += 32;
    }
    if ((number & 0xffff0000) != 0)
    {
        number >>= 16;
        n_bits += 16;
    }
    if ((number & 0xff00) != 0)
    {
        number >>= 8;
        n_bits += 8;
    }
    if ((number & 0xf0) != 0)
    {
        number >>= 4;
        n_bits += 4;
    }
    if ((number & 0xc) != 0)
    {
        number >>= 2;
        n_bits += 2;
    }
    if ((number & 0x2) != 0)
    {
        n_bits += 1;
    }
    return n_bits;
}

uint8_t
bit_length_digit(digit number)
{
    static int tot_bits = sizeof(digit) * CHAR_BIT;

    if (tot_bits == 64)
    {
        return _bit_length_u64(number);
    }
    else if (tot_bits == 32)
    {
        return _bit_length_u32(number);
    }
    else
    {
        return _bit_length_digit(number);
    }
}


/******************************************************************************
 *  bi_bit_length: return the number of bits required to represent its absolute
 *                 value.
 *
 *                 In the event that an unsigned long (ULONG_MAX >= 2**32 - 1)
 *                 is not wide enough to count the number of bits, returns
 *                 ULONG_MAX.
 ******************************************************************************/
unsigned long
bi_bit_length(const bi_t a)
{
    unsigned long abs_ndigits = ABS(a->n_digits);

    if (abs_ndigits > ULONG_MAX / BI_SHIFT)
    {
        return ULONG_MAX;
    }

    return (abs_ndigits - 1) * BI_SHIFT +                                     \
           bit_length_digit(a->digits[abs_ndigits - 1]);
}
