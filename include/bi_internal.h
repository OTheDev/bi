///////////////////////////////////////////////////////////////////////////////
//  Wrappers (start)
///////////////////////////////////////////////////////////////////////////////
#ifndef BI_H
#define BI_H

#ifdef __cplusplus
    extern "C" {
#endif


///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>


///////////////////////////////////////////////////////////////////////////////
//  Assumptions
///////////////////////////////////////////////////////////////////////////////
/* Assume implementation has mostly-compliant IEEE-754 64-bit doubles. */
_Static_assert(sizeof(double) * CHAR_BIT == 64, "64-bit double is assumed.");

/* The C Standard permits (1) sign and magnitude : -1 encoded as 100...001.
 *                        (2) two's complement   : -1 encoded as 111...111.
 *                        (3) ones' complement   : -1 encoded as 111...110. */
_Static_assert(-1 == ~0, "Two's complement representation assumed.");

/* This assumption is nonrestrictive. Technically, it is hypothetically
 * possible, for example, that a CPU only supports signed but not unsigned
 * arithmetic. An int might have N - 1 value bits and 1 sign bit and an
 * unsigned int may have the same memory layout but with the sign bit treated
 * as a padding bit, in which case INT_MAX == UINT_MAX. Note that the Standard
 * guarantees that if an int has M value bits and an unsigned int has N value
 * bits then M <= N. */
_Static_assert(INT_MAX < UINT_MAX, "INT_MAX < UINT_MAX assumed.");


///////////////////////////////////////////////////////////////////////////////
//  Main
///////////////////////////////////////////////////////////////////////////////
#ifdef __SIZEOF_INT128__
    #define HAS_UINT128
#endif

#if (defined(__x86_64__) || defined(_WIN64) || defined(__LP64__) ||          \
     defined(__aarch64__)) && defined(HAS_UINT128)
    typedef uint64_t digit;
    typedef unsigned __int128 twodigits;
    typedef int64_t sdigit;
    typedef __int128 stwodigits;

    #define BI_DIGIT_BITS 64
    #define BI_SHIFT 64
    #define BI_MASK UINT64_C(0xffffffffffffffff)
    #define BI_FSPEC PRIu64
    #define BI_SIZEOF_DIGIT 8
    #define DIGIT_C(v) UINT64_C(v)

     /* max{e in N ∪ {0}: 10^e fits in a digit} */
    #define BI_DECSHIFT 19
#else
    typedef uint32_t digit;
    typedef uint64_t twodigits;
    typedef int32_t sdigit;
    typedef int64_t stwodigits;

    #define BI_DIGIT_BITS 32
    #define BI_SHIFT 32
    #define BI_MASK UINT32_C(0xffffffff)
    #define BI_FSPEC PRIu32
    #define BI_SIZEOF_DIGIT 4
    #define DIGIT_C(v) UINT32_C(v)

    /* max{e in N ∪ {0}: 10^e fits in a digit} */
    #define BI_DECSHIFT 9
#endif

typedef struct
{
    /* Number of allocated members in the array of `digit`s. */
    int n_alloc;
    /* Number of used members in the array of `digit`s. Also encodes the sign
     * of the integer. negative <==> ->n_digits < 0, zero <==> ->n_digits == 0,
     * positive <==> ->n_digits > 0. */
    int n_digits;
    /* The digits array, representing the absolute value of the integer.
     * Least significant digit is the element with index 0. */
    digit *digits;
} _bi_struct;

typedef _bi_struct bi_t[1];

/* Counts of digits. */
typedef long bi_ssize_t;
/* Counts of bits. */
typedef unsigned long bi_bitcount_t;

/* Maximum number of digits.
 * Consider allocating n_digits digits for an integer using malloc().
 * The number of bytes needed for n_digits digits is n_digits * sizeof(digit).
 * Since malloc() accepts a size_t argument, need n_digits * sizeof(digit) to
 * be less than or equal to SIZE_MAX. We also need n_digits to be representable
 * by an int ==> n_digits <= INT_MAX required. Lastly, as the bit-level
 * functions accept an unsigned long as the bit index, we will require
 * n_digits <= ULONG_MAX / BI_DIGIT_BITS. In summary, we want n_digits s.t.
 * (1) n_digits <= (SIZE_MAX / BI_SIZEOF_DIGIT) := A
 * (2) n_digits <= INT_MAX := B
 * (3) n_digits <= (ULONG_MAX / BI_DIGIT_BITS) := C
 * Set BI_MAX_DIGITS to min(A, B, C).
 */
#if INT_MAX <= (ULONG_MAX / BI_DIGIT_BITS) &&                     \
    INT_MAX <= (SIZE_MAX / BI_SIZEOF_DIGIT)
    /* B <= C and B <= A ==> min(A, B, C) == B. */
    #define BI_MAX_DIGITS ( (unsigned) INT_MAX)
#elif (ULONG_MAX / BI_DIGIT_BITS) <= (SIZE_MAX / BI_SIZEOF_DIGIT)
    /* TRUE: B > C or B > A. Case 1: B > C and C <= A ==> min(A, B, C) == C.
     * Case 2: B > A and C <= A ==> min(A, B, C) == C. In either case,
     * min(A, B, C) == C. */
    #define BI_MAX_DIGITS ( (unsigned long) (ULONG_MAX / BI_DIGIT_BITS) )
#else
    /* min(A, B, C) != B and min(A, B, C) != C ==> min(A, B, C) == A. */
    #define BI_MAX_DIGITS ( (size_t) (SIZE_MAX / BI_SIZEOF_DIGIT) )
#endif


///////////////////////////////////////////////////////////////////////////////
//  Private
///////////////////////////////////////////////////////////////////////////////
/* Root memory allocation/deallocation functions - same as C standard library
 * functions but if memory allocation/reallocation fails, the program
 * terminates. _recalloc() is custom; same as _realloc() but clears any extra
 * memory allocated. */
void *_malloc(size_t size);
void *_calloc(size_t nmemb, size_t size);
void *_realloc(void *ptr, size_t size);
void _free(void *ptr);
void *_recalloc(void *ptr, size_t new_size, size_t old_size);

/*  Make `ptr->digits` have space for `size` `digit`s. This may modify both
 * `ptr->digits` and/or `ptr->n_alloc` but does not change `ptr->n_digits`. */
void *bi_realloc(bi_t ptr, bi_ssize_t size);

/* Prints the integer in its base 2^(BI_SHIFT) representation. */
void bi_print_internal(const bi_t a);

/* Return the number of bits required to represent the digit. */
uint8_t bit_length_digit(digit number);

/* Return an estimate of the number of decimal digits required to represent
 * the integer or 0 if it could not be determined. If 0 is returned and if
 * the object pointed to by lower_bound is also 0, the number of base10
 * digits required would exceed SIZE_MAX - 2; if this object has a value
 * other than 0, this value can be used as a large lower bound for the
 * base10 length. */
size_t _bi_decimal_length(const bi_t a, size_t *lower_bound);

/* a %= 10: divide a in-place by 10, returning the remainder. */
uint8_t bi_idiv10(bi_t a);

/* Divide N by digit D, storing the quotient in Q and remainder in the object
 * pointed to by R. */
void bi_divide_by_digit(bi_t Q, digit *R, bi_t N, digit D);

/* to = |a| + |b| */
void bi_add_abs(bi_t to, const bi_t a, const bi_t b);
/* to = |a| - |b| */
void bi_sub_abs(bi_t to, const bi_t a, const bi_t b);


///////////////////////////////////////////////////////////////////////////////
//  Private Macros and Useful Routines
///////////////////////////////////////////////////////////////////////////////
#define ABS(x) ((x) < 0 ? -(x) : (x))


/******************************************************************************
 *  BI_ON_OVERFLOW: called before an attempt is made to allocate more digits
 *                  than BI_MAX_DIGITS. The default behavior is to print an
 *                  error message to stderr and terminate the program.
 ******************************************************************************/
#define BI_ON_OVERFLOW()                                        \
    do {                                                        \
        fprintf(stderr, "%s(): overflow error!\n", __func__);   \
        exit(EXIT_FAILURE);                                     \
    } while (0)


/******************************************************************************
 *  BI_NORMALIZE: given a bi_t variable `b` and int `asize` (representing an
 *                estimate of the number of digits needed for the integer),
 *                set `b->n_digits` to the actual number of digits needed.
 ******************************************************************************/
#define BI_NORMALIZE(b, asize)                                    \
    do {                                                          \
        while ((asize) > 0 && (b)->digits[(asize) - 1] == 0)      \
        {                                                         \
            (asize)--;                                            \
        }                                                         \
        (b)->n_digits = ((b)->n_digits < 0) ? -(asize) : (asize); \
    } while (0)

#define BI_NORMALIZE_NEG(b, asize)                            \
    do {                                                      \
        while ((asize) > 0 && (b)->digits[(asize) - 1] == 0)  \
        {                                                     \
            (asize)--;                                        \
        }                                                     \
        (b)->n_digits = -(asize);                             \
    } while (0)

#define BI_NORMALIZE_NONNEG(b, asize)                         \
    do {                                                      \
        while ((asize) > 0 && (b)->digits[(asize) - 1] == 0)  \
        {                                                     \
            (asize)--;                                        \
        }                                                     \
        (b)->n_digits = (asize);                              \
    } while (0)


/******************************************************************************
 *  bi_realloc_if_necessary: call bi_realloc() if a->n_alloc < what is needed.
 *                           otherwise, just return the digits array of a.
 *
 *                           Name a little too verbose but clear.
 ******************************************************************************/
static inline digit *
bi_realloc_if_necessary(bi_t a, int n)
{

    return a->n_alloc < n ? (digit *)bi_realloc(a, n) : a->digits;
}

#define BI_REALLOC_IF_NECESSARY(a, n)                                   \
    ((a)->n_alloc < (n) ? (digit *)bi_realloc((a), (n)) : (a)->digits)


/******************************************************************************
 *  bi_dcopy: copy `n_digits` elements from the `from` array to the `to` array,
 *            ([0, n_digits - 1]).
 ******************************************************************************/
static inline void
bi_dcopy(digit *to, digit *from, int n_digits)
{
    for (int i = 0; i < n_digits; i++)
    {
        to[i] = from[i];
    }
}

#define BI_DCOPY(to, from, n_digits)          \
    do {                                      \
        for (int i = 0; i < (n_digits); i++)  \
        {                                     \
            (to)[i] = (from)[i];              \
        }                                     \
    } while (0)


/******************************************************************************
 *  BI_MEMSET: set destination[0],...,destination[n_digits - 1] to value.
 ******************************************************************************/
static inline void
bi_memset(digit *destination, int n_digits, digit value)
{
    for (int i = 0; i < n_digits; i++)
    {
        destination[i] = value;
    }
}

#define BI_MEMSET(destination, n_digits, value) \
    do {                                        \
        for (int i = 0; i < (n_digits); i++)    \
        {                                       \
            (destination)[i] = (value);         \
        }                                       \
    } while (0)


/******************************************************************************
 *  Unsigned Integer Addition/Subtraction/Multiplication.
 *      The object pointed to by overflow_p will evaluate to 1 if overflow
 *      occurred, 0 otherwise. *r_p, a, b assumed to be of the same type.
 ******************************************************************************/
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
 *  BI_ADDC: add with carry (ADC). Given r, a, b of unsigned integer type
 *           'type' and carry an unsigned integer that is either 1 or 0, set r
 *           equal to the computation a + b + carry and set carry to 1 or 0
 *           depending on whether the addition "overflowed" or not.
 ******************************************************************************/
#define BI_ADDC(r, a, b, carry, type)   \
    do {                                \
        type _tmp = a + carry;          \
        r = b + _tmp;                   \
        carry = (r < b) || (_tmp < a);  \
    } while (0)


/******************************************************************************
 *  BI_SUBB: subtract with borrow (SBB). Given r, a, b of unsigned integer type
 *           'type' and borrow an unsigned integer that is either 1 or 0, set r
 *           equal to the computation a - b - borrow and set carry to 1 or 0
 *           depending on whether the subtraction "overflowed" or not.
 ******************************************************************************/
#define BI_SUBB(r, a, b, borrow, type)      \
    do {                                    \
        type _tmp = a - b;                  \
        r = _tmp - borrow;                  \
        borrow = (r > _tmp) || (_tmp > a);  \
    } while (0)


/* Functionally equivalent to BI_ADDC(), BI_SUBB() but generally slower (unless
 * optimizations are enabled, in which case they are virtually equivalent).
 * An earlier version of add.c gives a proof of why this algorithm works.
 * BI_ADDC(), BI_SUBB() are not only faster, but also more readable/intuitive
 * and so the choice feels obvious. */
#define BI_ADDC_V2(r, a, b, carry)            \
    do {                                      \
        r = a + b + carry;                    \
        carry = (r < a) || (carry && r == a); \
    } while (0)

#define BI_SUBB_V2(r, a, b, borrow)             \
    do {                                        \
        r = a - b - borrow;                     \
        borrow = (r > a) || (borrow && r == a); \
    } while (0)


///////////////////////////////////////////////////////////////////////////////
//  Public
///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
 *  FREE memory occupied by integer.
 ******************************************************************************/
/* Call when already-prepared big integer no longer needed. */
void bi_free(bi_t a);
void bi_frees(bi_t a, ...);


/******************************************************************************
 *  PREPARE and SET the integer (zero by default).
 *
 *  Do not call prep_* functions on a bi_t variable already prepared. If a
 *  previously prepared variable has been freed, prep_* functions may be called
 *  on it again.
 ******************************************************************************/
void bi_prep(bi_t a);
void bi_preps(bi_t a, ...);

void bi_prep_i32(bi_t a, int32_t val);
void bi_prep_i64(bi_t a, int64_t val);
void bi_prep_imax(bi_t a, intmax_t val);

void bi_prep_u32(bi_t a, uint32_t val);
void bi_prep_u64(bi_t a, uint64_t val);
void bi_prep_umax(bi_t a, uintmax_t val);

void bi_prep_str(bi_t a, const char *str);


/*****************************************************************************
 *  SET an already-prepared integer to a value.
 *****************************************************************************/
/* Makes `to` a deepcopy of `from` such that they represent the same integer. */
void bi_set(bi_t to, const bi_t from);


/*****************************************************************************
 *  COMPARISONS
 *****************************************************************************/
/* Return value negative, zero, positive depending if a < b, a == b, a > b. */
int bi_cmp(const bi_t a, const bi_t b);


/*****************************************************************************
 *  ARITHMETIC
 *****************************************************************************/
/* to = a + b */
void bi_add(bi_t to, const bi_t a, const bi_t b);
/* to = a - b */
void bi_sub(bi_t to, const bi_t a, const bi_t b);
/* to = a * b */
void bi_mul(bi_t to, const bi_t a, const bi_t b);

/* to = |a| */
void bi_abs(bi_t to, const bi_t a);
/* to = -a */
void bi_negate(bi_t to, const bi_t a);


/*****************************************************************************
 *  DIVISION
 *
 *  Caution: as with normal integer division in C, it is the responsibility
 *           of the caller to allow or disallow division by zero. These
 *           functions will divide by zero if division by zero is attempted.
 *****************************************************************************/
/* quotient = a / b (integer division -- truncation towards zero),
 * remainder = a % b. (a/b)*b + a%b = a always. */
void bi_divide_qr(bi_t quotient, bi_t remainder, bi_t a, bi_t b);


/*****************************************************************************
 *  INTEGER to STRING
 *****************************************************************************/
char *bi_to_str(const bi_t a);


/*****************************************************************************
 *  BITS
 *****************************************************************************/
/* Return the number of bits required to represent its absolute value. */
bi_bitcount_t bi_bit_length(const bi_t a);

/* If a >= 0, result = a << shift_by.
 * Otherwise, result = (-1) * (|a| << shift_by). */
void bi_lshift(bi_t result, const bi_t a, bi_bitcount_t shift_by);

/* If a >= 0, result = a >> shift_by.
 * Otherwise, result = (-1) * (|a| >> shift_by). */
void bi_rshift(bi_t result, const bi_t a, bi_bitcount_t shift_by);

/** Zero-based indexing applies. i >= bi_bit_length(a) permitted. These
 ** functions act as if a is nonnegative; a's sign is preserved. **/
/* Get/Test bit `i` of `a`. */
bool bi_get_bit(bi_t a, bi_bitcount_t i);
/* Set bit `i` of `a`. */
void bi_set_bit(bi_t a, bi_bitcount_t i);


/*****************************************************************************
 *  PRINTING
 *****************************************************************************/
/* Print the base-10 representation of the integer plus a newline to stdout. */
void bi_print(const bi_t a);


///////////////////////////////////////////////////////////////////////////////
//  Wrappers (end)
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
    }
#endif

#endif /* BI_H */
