///////////////////////////////////////////////////////////////////////////////
//  A Simple Portable (Incomplete) Implementation of 128-bit Unsigned
//  Arithmetic that can be used as a fallback.
//
//  Compiling with optimizations enabled (e.g. gcc with -O2) is encouraged.
//  The speedup is significant.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  Wrappers (start)
///////////////////////////////////////////////////////////////////////////////
#ifndef BI_UINT128_T_H
#define BI_UINT128_T_H

#ifdef __cplusplus
    extern "C" {
#endif


///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include <stdbool.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <assert.h>


///////////////////////////////////////////////////////////////////////////////
// bi_uint128_t type
///////////////////////////////////////////////////////////////////////////////
typedef struct bi_uint128_t {
    /* Lowest 64 bits */
    uint64_t lo;
    /* Highest 64 bits */
    uint64_t hi;
} bi_uint128_t;


///////////////////////////////////////////////////////////////////////////////
//  Helpful Primitives
//
//  C Standard (C11, §6.2.5): "[a] computation involving unsigned operands
//  can never overflow, because a result that cannot be represented by the
//  resulting unsigned integer type is reduced modulo the number that is one
//  greater than the largest value that can be represented by the resulting
//  type." In other words,
//
//  UADD(a, b) = a + b if a + b < 2^w, a + b - 2^w if a + b >= 2^w.
//  USUB(a, b) = a - b if a - b >= 0, a - b + 2^w if a - b < 0.
//
//  Fact: UADD(a, b) < a <==> there is overflow caused by the addition.
//  Proof: (==>) Assume there is overflow. By definition, a + b >= 2^w.
//         This implies UADD(a, b) = a + (b - 2^w) < a (since b - 2^w < 0).
//         (<==) Assume there is no overflow. By definition, a + b < 2^w.
//               Thus, UADD(a, b) = a + b >= a (since b >= 0). By logical
//               equivalence of the contrapositive, if (UADD(a, b) < a) then
//               there is overflow. QED.
//
//  Fact: (a < b) <==> there is overflow caused by the subtraction.
//  Proof: By definition, there is overflow iff a - b < 0 <==> a < b. QED.
///////////////////////////////////////////////////////////////////////////////
/* In the following bit manipulation/extraction macros, zero-based indexing is
 * assumed for i. */
#define U64_SET_BIT(x, i)  ((x) |= UINT64_C(1) << (i))
#define U128_SET_BIT(x, i) (((i) < 64) ? (((x).lo) |= UINT64_C(1) << (i)) : \
                                         (((x).hi) |= UINT64_C(1) << (i)))

#define U64_GET_BIT(x, i)  (bool)((x) & (UINT64_C(1) << (i)))
#define U128_GET_BIT(x, i) (bool)(((i) < 64) ?                  \
                           (((x).lo) & (UINT64_C(1) << (i))) :  \
                           (((x).hi) & (UINT64_C(1) << (i))))

/* General Routines */
#define UADD_OVERFLOW(r_p, a, b, overflow_p)  \
    do {                                      \
        *(r_p) = (a) + (b);                   \
        *(overflow_p) = *(r_p) < (a);         \
    } while (0)

#define USUB_OVERFLOW(r_p, a, b, overflow_p)  \
    do {                                      \
        *(r_p) = (a) - (b);                   \
        *(overflow_p) = (a) < (b);            \
    } while (0)

#define UMUL_OVERFLOW(r_p, a, b, overflow_p)        \
    do {                                            \
        *(r_p) = (a) * (b);                         \
        *(overflow_p) = (a) && *(r_p) / (a) != (b); \
    } while (0)


/******************************************************************************
 *  UADD64_OVERFLOW: adds a and b, storing the computational result in the
 *                   object pointed to by r_p.
 *
 *                   Returns 1 if overflow occurred, 0 otherwise.
 ******************************************************************************/
static inline bool
UADD64_OVERFLOW(uint64_t *r_p, uint64_t a, uint64_t b)
{
    *r_p = a + b;
    return *r_p < a;
}


/******************************************************************************
 *  UMUL64_OVERFLOW: multiplies a by b, storing the computational result in
 *                   the object pointed to be r_p.
 *
 *                   Returns 1 if overflow occurred, 0 otherwise.
 ******************************************************************************/
static inline bool
UMUL64_OVERFLOW(uint64_t *r_p, uint64_t a, uint64_t b)
{
    *r_p = a * b;
    return a && *r_p / a != b;
}


/******************************************************************************
 *  USUB64_OVERFLOW: subtracts b from a, storing the computational result in
 *                   the object pointed to be r_p.
 *
 *                   Returns 1 if overflow occurred, 0 otherwise.
 ******************************************************************************/
static inline bool
USUB64_OVERFLOW(uint64_t *r_p, uint64_t a, uint64_t b)
{
    *r_p = a - b;
    return b > a;
}


/******************************************************************************
 *  UMUL64_128: multiply two 64-bit unsigned integers, storing the result in
 *              a 128-bit unsigned integer stored as two 64-bit unsigned
 *              integers.
 *
 *              This is grade-school multiplication.
 *
 *              Note that the product of two digits (each digit 32 bits) plus
 *              two digits is at most:
 *
 *                  (2^w - 1)(2^w - 1) + 2(2^w - 1) = 2^(2w) - 1.
 *
 *              Thus, the product of two digits plus two digits in base 2^w
 *              will fit in one base 2^(2w) digit.
 ******************************************************************************/
static inline bi_uint128_t
UMUL64_128(const uint64_t a, const uint64_t b)
{
    #define MASK32 0xffffffff

    bi_uint128_t r;
    uint64_t a0 = a & MASK32;
    uint64_t a1 = a >> 32;
    uint64_t b0 = b & MASK32;
    uint64_t b1 = b >> 32;

    uint64_t a0b0 = a0 * b0;
    uint64_t a0b1 = a0 * b1;
    uint64_t a1b0 = a1 * b0;
    uint64_t a1b1 = a1 * b1;

    uint64_t mid = a1b0 + (a0b0 >> 32) + (a0b1 & MASK32);

    r.lo = (a0b0 & MASK32) | (mid << 32);
    r.hi = a1b1 + (mid >> 32) + (a0b1 >> 32);

    return r;

    #undef MASK32
}


///////////////////////////////////////////////////////////////////////////////
//  Functions
//
//  Results are "computational" in that all the unsigned integer overflow
//  rules of the C Standard apply as if bi_uint128_t were a standard type.
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  Addition/Multiplication
 ******************************************************************************/
/* Returns a + b. */
static inline bi_uint128_t
bi_uint128_t_add(bi_uint128_t a, bi_uint128_t b)
{
    a.hi += UADD64_OVERFLOW(&a.lo, a.lo, b.lo);
    a.hi += b.hi;

    return a;
}

/* Performs (*a) += b. */
static inline void
bi_uint128_t_iadd(bi_uint128_t *a, bi_uint128_t b)
{
    *a = bi_uint128_t_add(*a, b);
}

/* Returns a - b. */
static inline bi_uint128_t
bi_uint128_t_sub(bi_uint128_t a, bi_uint128_t b)
{
    a.hi -= USUB64_OVERFLOW(&a.lo, a.lo, b.lo);
    a.hi -= b.hi;
    return a;
}

/* Performs (*a) -= b. */
static inline void
bi_uint128_t_isub(bi_uint128_t *a, bi_uint128_t b)
{
    *a = bi_uint128_t_sub(*a, b);
}

/* Returns a * b. */
static inline bi_uint128_t
bi_uint128_t_mul(bi_uint128_t a, bi_uint128_t b)
{
    bi_uint128_t r;

    r = UMUL64_128(a.lo, b.lo);
    r.hi += a.hi * b.lo + a.lo * b.hi;
    return r;
}

/* Performs (*a) *= b. */
static inline void
bi_uint128_t_imul(bi_uint128_t *a, bi_uint128_t b)
{
    *a = bi_uint128_t_mul(*a, b);
}


/******************************************************************************
 *  Comparisons
 ******************************************************************************/
/* (a < b) */
static inline int
bi_uint128_t_lt(bi_uint128_t a, bi_uint128_t b)
{
    return (a.hi < b.hi) ? 1 : ( (a.hi == b.hi) ? (a.lo < b.lo) : 0);
}

/* (a <= b) <==> not (b < a) */
static inline int
bi_uint128_t_lte(bi_uint128_t a, bi_uint128_t b)
{
    return !bi_uint128_t_lt(b, a);
}

/* (a == b) */
static inline int
bi_uint128_t_eq(bi_uint128_t a, bi_uint128_t b)
{
    return a.lo == b.lo && a.hi == b.hi;
}

/* (a != b) <==> not (a == b) */
static inline int
bi_uint128_t_neq(bi_uint128_t a, bi_uint128_t b)
{
    return !bi_uint128_t_eq(a, b);
}

/* (a > b) <==> (b < a) */
static inline int
bi_uint128_t_gt(bi_uint128_t a, bi_uint128_t b)
{
    return bi_uint128_t_lt(b, a);
}

/* (a >= b) <==> not (a < b) */
static inline int
bi_uint128_t_gte(bi_uint128_t a, bi_uint128_t b)
{
    return !bi_uint128_t_lt(a, b);
}


/******************************************************************************
 *  Bit-Shifting
 ******************************************************************************/
/* a << b */
static inline bi_uint128_t
bi_uint128_t_lshift(bi_uint128_t a, bi_uint128_t b)
{
    if (b.lo == 0) return a;
    if (b.lo < 64)
    {
        a.hi = (a.hi << b.lo) | (a.lo >> (64 - b.lo));
        a.lo <<= b.lo;
    }
    else
    {
        a.hi = a.lo << (b.lo - 64);
        a.lo = 0;
    }
    return a;
}

/* a <<= b */
static inline void
bi_uint128_t_ilshift(bi_uint128_t *a, bi_uint128_t b)
{
    if (b.lo == 0) return;
    if (b.lo < 64)
    {
        a->hi = (a->hi << b.lo) | (a->lo >> (64 - b.lo));
        a->lo <<= b.lo;
    }
    else
    {
        a->hi = a->lo << (b.lo - 64);
        a->lo = 0;
    }
}

/* a <<= 1 */
static inline void
bi_uint128_t_ilshift1(bi_uint128_t *a)
{
    a->hi = (a->hi << 1) | (a->lo >> 63);
    a->lo <<= 1;
}

/* a >> b */
static inline bi_uint128_t
bi_uint128_t_rshift(bi_uint128_t a, bi_uint128_t b)
{
    if (b.lo == 0) return a;
    if (b.lo < 64)
    {
        a.lo = (a.lo >> b.lo) | (a.hi << (64 - b.lo));
        a.hi >>= b.lo;
    }
    else
    {
        a.lo = a.hi >> (b.lo - 64);
        a.hi = 0;
    }
    return a;
}

/* a >>= b */
static inline void
bi_uint128_t_irshift(bi_uint128_t *a, bi_uint128_t b)
{
    if (b.lo == 0) return;
    if (b.lo < 64)
    {
        a->lo = (a->lo >> b.lo) | (a->hi << (64 - b.lo));
        a->hi >>= b.lo;
    }
    else
    {
        a->lo = a->hi >> (b.lo - 64);
        a->hi = 0;
    }
}


/******************************************************************************
 *  Division: divide N by D, placing the quotient in Q and the remainder in R.
 *
 *  Binary Long Division Algorithm
 *  --------------------------------------------------------------------------
 *  Let n denote the number of bits in N. In what follows, X(i) denotes the
 *  ith bit of X (zero-based indexing; zero gives the LSB).
 *
 *  (1) if D = 0 then handle division by zero (or not).
 *  (2) Q := 0; R := 0.
 *  (3) for i from (n - 1) to 0:
 *      (3)(I)   R := R << 1.
 *      (3)(II)  R(0) := N(i).
 *      (3)(III) if R >= D then
 *               (a) R := R - D.
 *               (b) Q(i) := 1.
 *  (4) Return Q and R.
 ******************************************************************************/
static inline void
bi_uint128_t_div(bi_uint128_t N, bi_uint128_t D, bi_uint128_t *Q,
                 bi_uint128_t *R)
{
    /* (1) */
    if (D.lo == 0 && D.hi == 0)
    {
        /* TODO: set global error indicator instead? */
        fprintf(stderr, "bi_uint128_t_div(): integer division by zero attempt "
                        "detected. Manually raising SIGFPE. \n");
        /* SIGFPE: "an erroneaous arithmetic operation, such as zero divide or
         * an operation resulting in overflow" (C11, §7.14). */
        raise(SIGFPE);
    }

    /* (2) */
    Q->lo = 0;
    Q->hi = 0;
    R->lo = 0;
    R->hi = 0;

    /* (3) */
    for (int i = 127; i >= 0; i--) {
        /* (3)(I) */
        bi_uint128_t_ilshift1(R);
        /* (3)(II). You may be wondering why we do not perform R(0) when N(i)
         * is zero. R(0) is guaranteed to be zero at this point due to (3)(I).
         */
        if (U128_GET_BIT(N, i)) {
            U128_SET_BIT(*R, 0);
        }
        /* (3)(III) */
        if (bi_uint128_t_gte(*R, D))
        {
            /* (3)(III)(a) */
            bi_uint128_t_isub(R, D);
            /* (3)(III)(b) */
            U128_SET_BIT(*Q, i);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
//  Other Utilities
///////////////////////////////////////////////////////////////////////////////
static inline bi_uint128_t
bi_uint128_t_add64(bi_uint128_t a, uint64_t b)
{
    a.hi += UADD64_OVERFLOW(&a.lo, a.lo, b);
    return a;
}

static inline bi_uint128_t
bi_uint128_t_mul64(bi_uint128_t a, uint64_t b)
{
    bi_uint128_t tmp = {b, 0};
    return bi_uint128_t_mul(a, tmp);
}


///////////////////////////////////////////////////////////////////////////////
//  Wrappers (end)
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
    }
#endif

#endif /* MUL_H */
