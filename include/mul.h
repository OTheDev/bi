///////////////////////////////////////////////////////////////////////////////
//  Wrappers (start)
///////////////////////////////////////////////////////////////////////////////
#ifndef MUL_H
#define MUL_H

#ifdef __cplusplus
    extern "C" {
#endif


///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Multiplication Helpers
//
//  Even though unsigned __int128 is supported on Apple clang version 13.1.6
//  (clang-1316.0.21.2.5) on my arm64-apple-darwin21.6.0 (M1 Macbook Pro),
//  it is no faster than the portable MULT_2DIGITS. This is true both without
//  and with optimizations.
//
//  Note: (2^w - 1)(2^w - 1) = 2^(2w) - 2^(w + 1) + 1 < 2^(2w) - 1. Thus, the
//        result of multiplying two base 2^w digits is guaranteed to need at
//        most two base 2^w digits to represent it.
//
//        Similarly, (2^w - 1)(2^w - 1) + 2(2^w - 1) = 2^(2w) - 1. Thus, the
//        result of multiplying two base 2^w digits and adding two base 2^w
//        digits is guaranteed to need at most two digits to represent it.
///////////////////////////////////////////////////////////////////////////////
#define MASK32 0xffffffff

#if BI_SHIFT == 64
    /* Performs (rhi, rlo)_B = (a)_B * (b)_B. */
    #define MULT_2DIGITS(a, b, rlo, rhi)                          \
        do {                                                      \
            uint64_t a0 = (a) & MASK32;                           \
            uint64_t a1 = (a) >> 32;                              \
            uint64_t b0 = (b) & MASK32;                           \
            uint64_t b1 = (b) >> 32;                              \
            uint64_t a0b0 = a0 * b0;                              \
            uint64_t a0b1 = a0 * b1;                              \
            uint64_t a1b0 = a1 * b0;                              \
            uint64_t a1b1 = a1 * b1;                              \
            uint64_t mid = a1b0 + (a0b0 >> 32) + (a0b1 & MASK32); \
            (rlo) = (a0b0 & MASK32) | (mid << 32);                \
            (rhi) = a1b1 + (mid >> 32) + (a0b1 >> 32);            \
        } while (0)

    #ifdef __SIZEOF_INT128__
        #define MULT_2DIGITS_NATIVE(a, b, rlo, rhi)                   \
            do {                                                      \
                unsigned __int128 res = (unsigned __int128)(a) * (b); \
                (rlo) = (uint64_t)res;                                \
                (rhi) = (uint64_t)(res >> 64);                        \
            } while (0)
    #endif

#elif BI_SHIFT == 32
    /* The easy case */
    #error "Multiplication for 32-bit unsigned integers not yet implemented"
#endif

/* Performs (rhi, rlo)_B = (y)_B * (x)_B + (a)_B + (b)_B. */
#define MULT_2DIGITS_x_ADD_2DIGITS(y, x, a, b, rlo, rhi)  \
    do {                                                  \
        MULT_2DIGITS((y), (x), (rlo), (rhi));             \
        (rlo) += (a);                                     \
        (rhi) += ((rlo) < (a));                           \
        (rlo) += (b);                                     \
        (rhi) += ((rlo) < (b));                           \
    } while (0)

/* Performs (rhi, rlo)_B = (y)_B * (x)_B + (a)_B. */
#define MULT_2DIGITS_x_ADD_1DIGIT(y, x, a, rlo, rhi)  \
    do {                                              \
        MULT_2DIGITS((y), (x), (rlo), (rhi));         \
        (rlo) += (a);                                 \
        (rhi) += ((rlo) < (a));                       \
    } while (0)


///////////////////////////////////////////////////////////////////////////////
//  Wrappers (end)
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
    }
#endif

#endif /* MUL_H */
