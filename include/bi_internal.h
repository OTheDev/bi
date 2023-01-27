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
 * The minimum of INT_MAX, (ULONG_MAX / BI_DIGIT_BITS), SIZE_MAX.
 * In all three cases, should have LONG_MAX > BI_MAX_DIGITS. */
#if INT_MAX <= (ULONG_MAX / BI_DIGIT_BITS) && INT_MAX <= SIZE_MAX
    /* ==> sizeof(long) > sizeof(int) */
    #define BI_MAX_DIGITS INT_MAX
#elif (ULONG_MAX / BI_DIGIT_BITS) <= SIZE_MAX
    /* True: INT_MAX > (ULONG_MAX / BI_DIGIT_BITS) or INT_MAX > SIZE_MAX. */
    #define BI_MAX_DIGITS (ULONG_MAX / BI_DIGIT_BITS)
#else
    #define BI_MAX_DIGITS SIZE_MAX
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

/* Make sure ABS(b->n_digits) refers to the number of digits needed to
 * represent the integer. */
void bi_normalize(bi_t b);

/* Return the number of bits required to represent the digit. */
uint8_t bit_length_digit(digit number);

/* Return an estimate of the number of decimal digits required to represent
 * the integer or 0 if the bit length of the integer is ULONG_MAX. */
size_t _bi_decimal_length(const bi_t a);

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
#define MAX(a, b) ((a) > (b) ? (a) : (b))


/******************************************************************************
 *  BI_NORMALIZE: see bi_normalize(). This macro is more flexible as it doesn't
 *                need to compute ABS(b->n_digits), which is normally already
 *                computed by routines prior to normalization.
 ******************************************************************************/
#define BI_NORMALIZE(b, abs_n_digits)                                       \
    do {                                                                    \
        while ((abs_n_digits) > 0 && (b)->digits[(abs_n_digits) - 1] == 0)  \
        {                                                                   \
            (abs_n_digits)--;                                               \
        }                                                                   \
        (b)->n_digits = ((b)->n_digits) ? (abs_n_digits) : -(abs_n_digits); \
    } while (0)

#define BI_NORMALIZE_POSITIVE(b, abs_n_digits, positive)                    \
    do {                                                                    \
        while ((abs_n_digits) > 0 && (b)->digits[(abs_n_digits) - 1] == 0)  \
        {                                                                   \
            (abs_n_digits)--;                                               \
        }                                                                   \
        (b)->n_digits = (positive) ? (abs_n_digits) : -(abs_n_digits);      \
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

/* If a >= 0, a >>= shift_by.
 * Otherwise, (-1) * (|a| >>= shift_by). */
void bi_irshift(bi_t a, bi_bitcount_t shift_by);

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
