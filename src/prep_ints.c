///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"


///////////////////////////////////////////////////////////////////////////////
//  Macros
///////////////////////////////////////////////////////////////////////////////
/* Assumes val an unsigned integer with maximum value <= BI_MASK */
#define PREP_ONE_DIGIT(a, val)                \
    do {                                      \
        (a)->digits = _malloc(sizeof(digit)); \
        (a)->n_digits = (a)->n_alloc = 1;     \
        (a)->digits[0] = (val) & BI_MASK;     \
    } while (0)

/* Assumes val a signed or unsigned integer with maximum magnitude <= BI_MASK */
#define PREP_ONE_DIGIT_SIGNED(a, val)               \
    do {                                            \
        (a)->digits = _malloc(sizeof(digit));       \
        if ((val) < 0)                              \
        {                                           \
            (a)->n_digits = -1;                     \
            (a)->n_alloc = 1;                       \
            (a)->digits[0] = ((-(val)) & BI_MASK);  \
        }                                           \
        else                                        \
        {                                           \
            (a)->n_digits = (a)->n_alloc = 1;       \
            (a)->digits[0] = (val) & BI_MASK;       \
        }                                           \
    } while (0)

/* Assumes val an unsigned integer with maximum value > BI_MASK */
#define PREP_ATLEAST_ONE_DIGIT(a, val, tmp, i)                \
    do {                                                      \
        (a)->n_digits = 1;                                    \
        (tmp) = (val);                                        \
        while ((tmp) >>= BI_SHIFT)                            \
            (a)->n_digits++;                                  \
        (a)->digits = _malloc((a)->n_digits * sizeof(digit)); \
        (a)->n_alloc = (a)->n_digits;                         \
        (i) = 0;                                              \
        while ((val) != 0)                                    \
        {                                                     \
            (a)->digits[(i)++] = (val) & BI_MASK;             \
            (val) >>= BI_SHIFT;                               \
        }                                                     \
    } while (0)

/* Assumes val an unsigned integer with maximum value > BI_MASK */
#define PREP_ATLEAST_ONE_DIGIT_SIGNED(a, val, tmp, i)         \
    do {                                                      \
        if ((val) >= 0)                                       \
        {                                                     \
            PREP_ATLEAST_ONE_DIGIT((a), (val), (tmp), (i));   \
            return;                                           \
        }                                                     \
        (a)->n_digits = 1;                                    \
        (tmp) = (val) = -(val);                               \
        while ((tmp) >>= BI_SHIFT)                            \
            (a)->n_digits++;                                  \
        (a)->digits = _malloc((a)->n_digits * sizeof(digit)); \
        (a)->n_alloc = (a)->n_digits;                         \
        (a)->n_digits = -((a)->n_digits);                     \
        (i) = 0;                                              \
        while ((val) != 0)                                    \
        {                                                     \
            (a)->digits[(i)++] = (val) & BI_MASK;             \
            (val) >>= BI_SHIFT;                               \
        }                                                     \
    } while (0)


///////////////////////////////////////////////////////////////////////////////
//  Functions
///////////////////////////////////////////////////////////////////////////////
void
bi_prep_umax(bi_t a, uintmax_t val)
{
    if (val == 0)
    {
        bi_prep(a);
        return;
    }

  #if UINTMAX_T <= BI_MASK
    PREP_ONE_DIGIT(a, val);
  #else
    int i;
    uintmax_t tmp;
    PREP_ATLEAST_ONE_DIGIT(a, val, tmp, i);
  #endif
}

void
bi_prep_u64(bi_t a, uint64_t val)
{
    if (val == 0)
    {
        bi_prep(a);
        return;
    }

  #if UINT64_MAX <= BI_MASK
    PREP_ONE_DIGIT(a, val);
  #else
    int i;
    uint64_t tmp;
    PREP_ATLEAST_ONE_DIGIT(a, val, tmp, i);
  #endif
}

void
bi_prep_u32(bi_t a, uint32_t val)
{
    if (val == 0)
    {
        bi_prep(a);
        return;
    }

  #if UINT32_MAX <= BI_MASK
    PREP_ONE_DIGIT(a, val);
  #else
    int i;
    uint32_t tmp;
    PREP_ATLEAST_ONE_DIGIT(a, val, tmp, i);
  #endif
}

void
bi_prep_imax(bi_t a, intmax_t val)
{
    if (val == 0)
    {
        bi_prep(a);
        return;
    }

  #if INTMAX_T <= BI_MASK
    PREP_ONE_DIGIT(a, val);
  #else
    int i;
    uintmax_t tmp;
    PREP_ATLEAST_ONE_DIGIT_SIGNED(a, val, tmp, i);
  #endif
}

void
bi_prep_i32(bi_t a, int32_t val)
{
    if (val == 0)
    {
        bi_prep(a);
        return;
    }

  #if INT32_MAX <= BI_MASK
    PREP_ONE_DIGIT_SIGNED(a, val);
  #else
    int i;
    uint32_t tmp;
    PREP_ATLEAST_ONE_DIGIT_SIGNED(a, val, tmp, i);
  #endif
}

void
bi_prep_i64(bi_t a, int64_t val)
{
    if (val == 0)
    {
        bi_prep(a);
        return;
    }

  #if INT64_MAX <= BI_MASK
    PREP_ONE_DIGIT_SIGNED(a, val);
  #else
    int i;
    uint64_t tmp;
    PREP_ATLEAST_ONE_DIGIT_SIGNED(a, val, tmp, i);
  #endif
}
