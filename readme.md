# bi
A BigInt implementation in C.

This has been a big interest of mine ever since transitioning to higher-level
languages than C (my first language) that have built-in support for arbitrary
precision integers.

The current API is presented below. Many more functions will be added and
modifications will be made to improve the efficiency and robustness of these
functions. These changes will be made after comprehensive tests have been made
for the currently available functions.

# API

Big integers are stored in a `bi_t` variable. Internally, a `bi_t` variable is
a one-element array of a thin structure that defines the integer.

`bi_t` variables are stack-allocated:

```C
bi_t a;
```

## Initializing/Finalizing

### Initialization/Preparation

Before using a `bi_t` variable, it must be **prepared** or **initialized**.
The following functions are available for this purpose:

```C
/* Prepare and set the integer to 0 */
void bi_prep(bi_t a);
/* Prepare a NULL-terminated, comma-separated list of integers and set their
 * value to 0 */
void bi_preps(bi_t a, ...);

/* Prepare 'a', setting its value to 'val' */
void bi_prep_i32(bi_t a, int32_t val);
void bi_prep_i64(bi_t a, int64_t val);

void bi_prep_u32(bi_t a, uint32_t val);
void bi_prep_u64(bi_t a, uint64_t val);

/* Prepare 'a', setting its value to the integer 'str' represents. It is
 * assumed 'str' contains characters representing base-10 digits, possibly
 * preceded by a minus sign. */
void bi_prep_str(bi_t a, const char *str);
```

Once prepared, one may use any of the functions below on it. Note that if you
free an already-prepared variable, it must be prepared again if you would like
to re-use it.

### Free

One of these functions should be called on an already-prepared `bi_t` variable
once it is no longer needed:
```C
/* Free the dynamically-allocated memory 'a' uses */
void bi_free(bi_t a);

/* Free the dynamically-allocated memory used by a NULL-terminated,
 * comma-separated list of bi_t variables */
void bi_frees(bi_t a, ...);
```

## Using the Integers
Once a `bi_t` variable has been **prepared**, it can be included as an argument
to any one of the functions below. Once **freed**, it must be **prepared**
again if one would like to reuse the `bi_t` variable.

Unless stated otherwise, the same `bi_t`
variable can appear both on the left-hand-side (LHS) or right-hand-side (RHS) of
a binary operation. For example, `bi_add(a, a, a)` performs `a = a + a`.

### Set

Internally `bi_set()` makes `to` a deepcopy of `from` such that they represent
the same integer (if `to == from`, this is a no-op).
```C
/* to = from */
void bi_set(bi_t to, const bi_t from);
```

### Comparisons
```C
/* Return value negative, zero, positive, depending if a < b, a == b, a > b */
int bi_cmp(const bi_t a, const bi_t b);
```

### Arithmetic
```C
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
```

### Division
```C
/* quotient  = a / b (integer division -- truncation towards zero)
 * remainder = a % b.
 * TRUE: (a / b) * b + a % b == a */
void bi_divide_qr(bi_t quotient, bi_t remainder, bi_t a, bi_t b);
```

### Integer to String

This function returns a string representing the base-10 representation of the
integer. The caller is responsible for freeing the object the pointer refers
to once the caller is done with it.
```C
char *bi_to_str(const bi_t a);
```

### Bits
Counts of bits are represented by the `bi_bitcount_t` type, which is just an
alias for `unsigned long`. For `bi_get_bit()` and `bi_set_bit()`, zero-based
indexing applies and `i >= bi_bit_length(a)` is permitted. These functions
act as if `a` is nonnegative but preserve `a`'s sign in the result.

```C
/* Return the number of bits required to represent its absolute value */
bi_bitcount_t bi_bit_length(const bi_t a);

/* If a >= 0, result = a << shift_by.
 * Otherwise, result = (-1) * (|a| << shift_by). */
void bi_lshift(bi_t result, const bi_t a, bi_bitcount_t shift_by);

/* If a >= 0, result = a >> shift_by;
 * Otherwise, result = (-1) * (|a| >> shift_by). */
void bi_rshift(bi_t result, const bi_t a, bi_bitcount_t shift_by);

/* Test bit i of a */
bool bi_get_bit(bi_t a, bi_bitcount_t i);
/* Set bit i of a */
void bi_set_bit(bi_t a, bi_bitcount_t i);
```

### Printing
Print the base-10 representation of the integer plus a newline to standard
output.
```C
void bi_print(const bi_t a);
```
