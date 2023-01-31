///////////////////////////////////////////////////////////////////////////////
//  Tests for the `bi` arbitrary precision integer library.
//
//  This file is still very much in its infancy and the tests can be improved.
//
//  To test a function named `X` from "ap.c", the convention followed is to
//  create a parameterless function `test_X` with a `bool` return type (a
//  return value of `true` indicates the test was successful):
//
//      bool
//      test_X(void)
//      {
//          INIT_TEST();
//          /* tests go here */
//          return true;
//      }
//
//  In order for the test function to actually be run, the function must be
//  "registered for testing" by appending the function to the `test_functions`
//  array (of pointers to parameterless functions with a return type of `bool`).
//
//  In the future, the tests for a function will be in their own separate files,
//  with multiple test functions, modularizing the tests, as it quickly
//  becomes difficult to maintain. This should also help create more meaningful
//  tests with more coverage of the possibilities, which still needs to be done
//  for the existing tests.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "bi_internal.h"
#include "bi_uint128_t.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


///////////////////////////////////////////////////////////////////////////////
//  Base
///////////////////////////////////////////////////////////////////////////////
/* All test functions must make INIT_TEST(); the first statement. */
#define INIT_TEST()                             \
    do {                                        \
        printf("%s(): running...\n", __func__); \
    } while (0)

#define INDENT "    "

/* Success if `v` is 'truthy', Fail otherwise. In the event of success,
 * assertion does nothing. In the event of failure, assertion causes function
 * this function-like macro is called in to return `false`, indicating that
 * the test failed.
 */
#define ASSERT(v)                                                         \
    do {                                                                  \
        if (!(v)) {                                                       \
            fprintf(stdout, INDENT "Assertion failed: %s, "               \
                    "function %s, line %d\n", (#v), __func__, __LINE__);  \
            return false;                                                 \
        }                                                                 \
    } while (0)

#define ASSERT_M(...)                                                     \
    do {                                                                  \
        bool _args[] = {__VA_ARGS__};                                     \
        size_t _args_count = sizeof(_args) / sizeof(bool);                \
        for (size_t i = 0; i < _args_count; i++) {                        \
            if (!(_args[i])) {                                            \
                fprintf(stdout, INDENT "Assertion failed: %s, "           \
                        "function %s, line %d\n", #__VA_ARGS__, __func__, \
                        __LINE__);                                        \
                return false;                                             \
            }                                                             \
        }                                                                 \
    } while (0)


///////////////////////////////////////////////////////////////////////////////
//  Helpers for Random Input
///////////////////////////////////////////////////////////////////////////////
void
bi_create(bi_t x, size_t n_digits)
{
    x->n_alloc = n_digits;
    x->n_digits = n_digits;

    if (n_digits)
    {
        x->digits = (digit *)malloc(n_digits * sizeof(digit));
    } else
    {
        x->digits = NULL;
    }
}

void
bi_create_random(bi_t x, int n_digits, bool positive)
{
    x->n_alloc = n_digits;
    x->n_digits = n_digits;

    if (n_digits != 0)
    {
        x->digits = (digit *)malloc(n_digits * sizeof(digit));
        for (int i = 0; i < n_digits; i++)
        {
            x->digits[i] = rand() & BI_MASK;
        }
        BI_NORMALIZE(x, n_digits);

        if (!positive)
        {
            x->n_digits = -x->n_digits;
        }
    }
    else
    {
        x->digits = NULL;
    }
}


///////////////////////////////////////////////////////////////////////////////
//  Test Functions
///////////////////////////////////////////////////////////////////////////////
bool
test_bi_set(void)
{
    INIT_TEST();

    bi_t a, b, copy, zero;

    /* Test setting an integer with value 0 and no allocated digits to an
     * integer with multiple digits. */
    bi_prep_umax(a, BI_MASK);
    bi_prep_umax(b, BI_MASK);
    bi_prep(copy);
    bi_add_abs(a, a, b);
    bi_set(copy, a);
    ASSERT_M(a != copy, a->digits != copy->digits, a->n_digits == 2,
             copy->n_digits == 2, a->digits[0] == copy->digits[0],
             a->digits[1] == copy->digits[1]);

    /* Test setting a value to zero */
    bi_prep(zero);
    bi_set(copy, zero);
    ASSERT_M(copy->n_digits == 0, copy->n_alloc != 0);

    bi_frees(a, b, copy, zero, NULL);

    /* Test setting zero to zero */
    bi_t d, e;
    bi_preps(d, e, NULL);
    bi_set(d, e);
    ASSERT_M(d->n_digits == 0, e->n_digits == 0, d->n_alloc == e->n_alloc,
             d->n_alloc == 0, d->digits == e->digits, d->digits == NULL);

    return true;
}

bool
test_bi_mul(void)
{
    INIT_TEST();

    bi_t a, b, c;
    char *s;

    /** Small input **/
    /* 2 * 3 = 6 */
    bi_prep_u32(a, 2);
    ASSERT_M(a->digits[0] == 2, a->n_digits == 1, a->n_alloc == 1);
    bi_prep_u32(b, 3);
    ASSERT_M(b->digits[0] == 3, b->n_digits == 1, b->n_alloc == 1);
    bi_prep(c);
    bi_mul(c, a, b);
    ASSERT_M(c->digits[0] == 6, c->n_digits == 1, c->n_alloc == 2);
    bi_frees(a, b, c, NULL);

    /* 2 * -3 = -6 */
    bi_prep_u32(a, 2);
    bi_prep_i32(b, -3);
    bi_prep(c);
    bi_mul(c, a, b);
    ASSERT_M(c->digits[0] == 6, c->n_digits == -1);
    bi_frees(a, b, c, NULL);

    bi_prep_umax(a, 923048209329);
    bi_prep_umax(b, 3920849232);
    bi_prep(c);
    bi_mul(c, a, b);
    s = bi_to_str(c);
    ASSERT_M(!strcmp(s, "3619132862646584885328"), c->n_digits == 2);
    free(s);
    /* Need to check "in-place" multiplication too */
    bi_mul(a, a, b);
    s = bi_to_str(a);
    /* FIXME: Something werid is going on here. Sometimes this succeeds
     * sometimes fails. */
    ASSERT_M(!strcmp(s, "3619132862646584885328"), a->n_digits == 2);
    bi_frees(a, b, c, NULL); free(s);

    /** Random Input **/
    int random_n_a;
    bi_t res_1, res_2, counter, one, zero;
    bi_prep_u32(zero, 0);
    bi_prep_u32(one, 1);
    srand(time(NULL));

    for (int i = 0; i < 1000; i++)
    {
        // clock_t start, end;
        // double time_elapsed;
        int multiplier = rand() % 65000;
        bi_t mult;

        bi_preps(res_1, res_2, NULL);
        bi_prep_u32(mult, multiplier);

        random_n_a = rand() % 10;
        bi_create_random(a, random_n_a, true);
        // bi_create_random(b, random_n_b, true);

        /* res_1 = |a| * |b| */
        // bi_mul(res_1, a, b);
        bi_mul(res_1, a, mult);

        // start = clock();
        bi_prep(counter);
        // bi_set(counter, b);
        bi_set(counter, mult);
        /*  |a| * |b| = |a| + |a| + ... + |a| (b times)*/
        while (bi_cmp(counter, zero))
        {
            bi_add_abs(res_2, res_2, a);
            bi_sub_abs(counter, counter, one);
        }
        // end = clock();
        // time_elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;

        if (bi_cmp(res_2, res_1) != 0)
        {
            printf("i = %d\n", i);
            printf("Test failed!\n");
            printf("a = ");
            bi_print(a);
            printf("multiplier = ");
            bi_print(mult);
            printf("res_1 = ");
            bi_print(res_1);
            printf("res_2 = ");
            bi_print(res_2);

            ASSERT(bi_cmp(res_2, res_1) == 0);
        }

        bi_frees(a, mult, res_1, res_2, NULL);
    }

    return 1;
}

bool
test_bi_irshift(void)
{
    INIT_TEST();

    bi_t a, b;
    char *s;

    bi_prep_umax(a, BI_MASK);
    bi_prep_umax(b, BI_MASK);
    bi_add_abs(a, a, b);
    s = bi_to_str(a);
    ASSERT_M(!strcmp(s, "36893488147419103230"), a->n_digits == 2); free(s);
    bi_rshift(a, a, 64);
    ASSERT_M(a->digits[0] == 1, a->n_digits == 1);
    bi_frees(a, b, NULL);

    bi_prep_umax(a, 923048209329);
    bi_prep_umax(b, 3920849232);
    bi_mul(a, a, b);
    s = bi_to_str(a);
    /* For the same compiled files, sometimes this test fails but then succeeds
     * when it is rerun. Something off is going on. */
    ASSERT(!strcmp(s, "3619132862646584885328"));
    ASSERT(a->n_digits >= 2);
    bi_rshift(a, a, 1);
    free(s);
    s = bi_to_str(a);
    ASSERT_M(!strcmp(s, "1809566431323292442664"), a->n_digits == 2);
    free(s);
    bi_rshift(a, a, 21);
    s = bi_to_str(a);
    ASSERT_M(!strcmp(s, "862868514691969"), a->n_digits == 1);
    free(s);
    bi_rshift(a, a, 50);
    ASSERT_M(a->digits[0] == 0, a->n_digits == 0);
    s = bi_to_str(a);
    ASSERT_M(!strcmp(s, "0"), a->n_digits == 0);
    bi_frees(a, b, NULL); free(s);

    bi_prep(a);
    bi_rshift(a, a, 1);
    ASSERT(a->n_digits == 0);
    bi_free(a);

    return 1;
}

bool
test_bi_lshift(void)
{
    INIT_TEST();

    bi_t a;

    /* Test 0*/
    bi_prep(a);
    bi_lshift(a, a, 1);
    ASSERT(a->n_digits == 0);
    bi_free(a);

    bi_prep_u32(a, 0);
    bi_lshift(a, a, 1);
    ASSERT(a->n_digits == 0);
    bi_free(a);

    /* Powers of 2 that fit in a digit */
    bi_prep_u32(a, 1);
    for (int i = 1; i < BI_SHIFT - 1; i++)
    {
        bi_lshift(a, a, 1);
        ASSERT_M(a->n_digits == 1, a->digits[0] == ((digit)1 << i));
    }
    bi_free(a);

    return 1;
}

bool
test_bi_divide_qr(void)
{
    INIT_TEST();

    bi_t a, b, q, r;
    char *qs, *rs;

    /* Prepare q, r, a, b */
    bi_preps(q, r, NULL);
    bi_prep_u32(a, 3232);
    bi_prep_u32(b, 10);
    bi_lshift(a, a, 80);
    bi_lshift(b, b, 70);
    /* a / b */
    bi_divide_qr(q, r, a, b);
    qs = bi_to_str(q);
    rs = bi_to_str(r);
    ASSERT_M(!strcmp(qs, "330956"), !strcmp(rs, "9444732965739290427392"));
    bi_frees(a, b, q, r, NULL); free(qs); free(rs);

    /* Prepare q, r, a, b */
    bi_preps(q, r, NULL);
    bi_prep_u32(a, 309329420);
    bi_prep_u32(b, 4294967290);
    /* a / b */
    bi_divide_qr(q, r, a, b);
    ASSERT_M(q->n_digits == 0, r->digits[0] == 309329420, r->n_digits == 1);
    bi_frees(q, r, a, b, NULL);

    bi_preps(q, r, NULL);
    for (int i = 0; i < 3000; i++)
    {
        for (int j = 1; j < 3000; j++)
        {
            digit quotient, remainder;
            quotient = i / j;
            remainder = i % j;
            bi_prep_u32(a, i); bi_prep_u32(b, j);
            bi_divide_qr(q, r, a, b);

            ASSERT_M(q->n_digits == 0 || q->digits[0] == quotient);
            ASSERT_M(r->n_digits == 0 || r->digits[0] == remainder);

            bi_frees(a, b, NULL);
        }
    }

    /* Divide by Zero */
    // bi_preps(q, r, a, b, NULL);
    // bi_divide(q, r, a, b);

    return 1;
}

bool
test_bi_divide_by_digit(void)
{
    INIT_TEST();
    /* Quotient and Numerator */
    bi_t Q, N;
    /* Remainder */
    digit R;

    bi_prep_umax(N, BI_MASK);
    bi_prep(Q);
    bi_divide_by_digit(Q, &R, N, BI_MASK / 2);
    /* True irrespective of the possible bases for this library */
    ASSERT_M(Q->digits[0] == 2, Q->n_digits == 1, R == 1);
    bi_frees(Q, N, NULL);

    return 1;
}

bool
test_bi_sub_abs(void)
{
    INIT_TEST();

    bi_t a, b, c;
    char *s;

    /** Test zeros **/
    bi_prep(a);
    bi_prep_umax(b, BI_MASK);
    bi_prep(c);
    /* c = 0 - b */
    bi_sub_abs(c, a, b);
    ASSERT_M(c->n_digits == -1, c->digits[0] == BI_MASK, c->n_alloc == 1);
    /* c = b - 0 */
    bi_sub_abs(c, b, a);
    ASSERT_M(c->n_digits == 1, c->digits[0] == BI_MASK, c->n_alloc == 1);
    /* b -= 0, a -= b (a = 0) */
    bi_sub_abs(b, b, a);
    ASSERT_M(b->n_digits == 1, b->digits[0] == BI_MASK, b->n_alloc == 1);
    bi_sub_abs(a, a, b);
    ASSERT_M(a->n_digits == -1, a->digits[0] == BI_MASK, a->n_alloc == 1);
    bi_frees(a, b, c, NULL);

    /** Small numbers **/
    bi_prep(c);
    bi_prep_i32(a, -3232);
    bi_prep_u32(b, 10);
    /* c = |a| - |b| */
    bi_sub_abs(c, a, b);
    ASSERT_M(c->digits[0] == 3222, c->n_digits == 1);
    /* c = |b| - |a| */
    bi_sub_abs(c, b, a);
    ASSERT_M(c->digits[0] == 3222, c->n_digits == -1);
    bi_frees(a, b, c, NULL);

    /** Slightly larger numbers **/
    /* Example 1*/
    bi_prep(c);
    bi_prep_umax(a, 2390329032);
    bi_prep_umax(b, 32903202);
    bi_sub_abs(c, a, b);
    s = bi_to_str(c);
    ASSERT(!strcmp(s, "2357425830")); free(s);
    bi_sub_abs(c, b, a);
    s = bi_to_str(c);
    ASSERT(!strcmp(s, "-2357425830")); free(s);
    bi_frees(a, b, c, NULL);

    /** Multi-digit numbers **/
    bi_prep_umax(a, UINT64_C(32904410910932));
    /* a <<= 172 */
    bi_lshift(a, a, 172);
    bi_prep_umax(b, 65555);
    /* b <<= 60 */
    bi_lshift(b, b, 60);
    bi_prep(c);
    /* c = |a| - |b| */
    bi_sub_abs(c, a, b);
    s = bi_to_str(c);
    ASSERT(!strcmp(s, "19697602732743042985117518369422140289313164712161595036"
                      "1229524992")); free(s);
    /* c = |c| - |c| */
    bi_sub_abs(c, c, c);
    ASSERT_M(c->n_digits == 0);
    /* c = |b| - |a| */
    bi_sub_abs(c, b, a);
    s = bi_to_str(c);
    ASSERT(!strcmp(s, "-1969760273274304298511751836942214028931316471216159503"
                      "61229524992")); free(s);
    bi_frees(a, b, c, NULL);

    /** Another test **/
    bi_create(a, 5);
    bi_create(b, 3);
    bi_create(c, 5);
    a->digits[0] = 9;
    a->digits[1] = 7;
    a->digits[2] = 5;
    a->digits[3] = 3;
    a->digits[4] = 1;
    a->n_digits = 5;
    b->digits[0] = 3;
    b->digits[1] = 2;
    b->digits[2] = 1;
    b->n_digits = 3;
    bi_sub_abs(c, a, b);
    ASSERT(c->digits[0] == 6);
    ASSERT(c->digits[1] == 5);
    ASSERT(c->digits[2] == 4);
    ASSERT(c->digits[3] == 3);
    ASSERT(c->digits[4] == 1);
    ASSERT(c->n_digits == 5);
    bi_frees(a, b, c, NULL);

    /** Random Input **/
    bi_t res_1, res_2;
    int i;
    int random_n_a, random_n_b;
    srand(time(NULL));

    for (i = 0; i < 10000; i++)
    {
        random_n_a = rand() % 100;
        random_n_b = rand() % 100;

        bi_preps(res_1, res_2, NULL);
        bi_create_random(a, random_n_a, true);
        bi_create_random(b, random_n_b, true);

        /* res_1 = |a| + |b| */
        bi_add_abs(res_1, a, b);
        /* res_2 = res_1 - |b| = |a| */
        bi_sub_abs(res_2, res_1, b);

        if (bi_cmp(res_2, a) != 0)
        {
            printf("Test failed!\n");
            printf("a = ");
            bi_print(a);
            printf("b = ");
            bi_print(b);

            ASSERT(bi_cmp(res_2, a) == 0);
        }

        bi_frees(a, b, res_1, res_2, NULL);
    }

    return 1;
}


/******************************************************************************
 *  String -> Integer and Integer -> String
 ******************************************************************************/
bool
test_bi_prep_str(void)
{
    INIT_TEST();

    bi_t a;
    char *s;

    /* Test Zeros */
    bi_prep_str(a, "0");
    ASSERT_M(a->n_digits == 0, a->n_alloc == 1);
    bi_free(a);
    bi_prep_str(a, "-0");
    ASSERT_M(a->n_digits == 0, a->n_alloc == 1);
    bi_free(a);
    bi_prep_str(a, "   0");
    ASSERT_M(a->n_digits == 0, a->n_alloc == 1);
    bi_free(a);
    bi_prep_str(a, "        -0");
    ASSERT_M(a->n_digits == 0, a->n_alloc == 1);
    bi_free(a);

    /* Test invalid string. In this case, no allocation and a is cleared. */
    printf("You may ignore \"error parsing string\" messages for test suite!"
           "\n");
    bi_prep_str(a, "     ");
    ASSERT_M(a->n_digits == 0, a->n_alloc == 0, a->digits == NULL);
    bi_free(a);
    bi_prep_str(a, "-");
    ASSERT_M(a->n_digits == 0, a->n_alloc == 0, a->digits == NULL);
    bi_free(a);

    /* Large numbers */
    bi_prep_str(a, "-23090394209304223424243243242524524522513563532152352141"
                   "292839482908490389048920384093829048134130905970709280329");
    s = bi_to_str(a);
    ASSERT(!strcmp("-23090394209304223424243243242524524522513563532152352141"
                   "292839482908490389048920384093829048134130905970709280329",
                   s));
    free(s); bi_free(a);

    bi_prep_str(a, "4090492094029049090592049502495042249584290582");
    s = bi_to_str(a);
    ASSERT(!strcmp("4090492094029049090592049502495042249584290582", s));
    free(s); bi_free(a);

    /* Test conversion for all valid 16-bit unsigned integers (except for 0 and
     * UINT16_MAX). Two extra characters for null and for negative sign for
     * signed integer test after this one.*/
    char buf[7];
    for (uint16_t i = 1; i < UINT16_MAX; i++)
    {
        sprintf(buf, "%" PRIu16, i);
        bi_prep_str(a, buf);
        ASSERT_M(a->digits[0] == i, a->n_digits == 1);
        bi_free(a);
    }

    /* Test conversion for all valid 16-bit signed integers */
    for (int16_t i = INT16_MIN; i < 0; i++)
    {
        sprintf(buf, "%" PRId16, i);
        bi_prep_str(a, buf);
        ASSERT_M(a->digits[0] == (digit)ABS((int32_t)i), a->n_digits == -1);
        bi_free(a);
    }
    for (int16_t i = 1; i < INT16_MAX; i++)
    {
        sprintf(buf, "%" PRId16, i);
        bi_prep_str(a, buf);
        ASSERT_M(a->digits[0] == (digit)i, a->n_digits == 1);
        bi_free(a);
    }

    /* Test conversion for all valid 32-bit unsigned integers. I have verified
     * this test passes. */
  #ifdef TEST_PEDANTIC
    char buf2[11];
    for (uint32_t i = 0; i < UINT32_MAX; i++)
    {
        sprintf(buf2, "%" PRIu32, i);
        bi_prep_str(a, buf2);
        ASSERT(a->digits[0] == i);
        bi_free(a);
    }
  #endif

    return 1;
}

bool
test_bi_to_str(void)
{
    INIT_TEST();

    bi_t z, a, b;
    char *s;
    bi_preps(z, a, b, NULL);

    /* 1 */
    bi_prep_umax(z, BI_MASK);

    s = bi_to_str(z);
  #if BI_SHIFT == 64
    ASSERT(!strcmp(s, "18446744073709551615"));
  #elif BI_SHIFT == 32
    ASSERT(!strcmp(s, "4294967295"));
  #endif
    bi_free(z); free(s);

    /* 2 */
    bi_prep_umax(z, BI_MASK);
    bi_prep_u32(a, 1);
    bi_add_abs(z, z, a);

    s = bi_to_str(z);
  #if BI_SHIFT == 64
    ASSERT(!strcmp(s, "18446744073709551616"));
  #elif BI_SHIFT == 32
    ASSERT(!strcmp(s, "4294967296"));
  #endif
    bi_frees(z, a, NULL); free(s);

    /* 3 */
    bi_prep_umax(z, 329032081341332);
    s = bi_to_str(z);
    ASSERT(!strcmp(s, "329032081341332"));
    bi_free(z); free(s);

    /* 4 */
    bi_prep_umax(z, 2393201001);
    s = bi_to_str(z);
    ASSERT(!strcmp(s, "2393201001"));
    bi_free(z); free(s);

    /* 5 */
    bi_prep_umax(z, 49209202323);
    bi_prep_umax(a, 20390293348203983);
    bi_mul(b, z, a);
    s = bi_to_str(b);
    ASSERT(!strcmp(s, "1003390070797090888121452509"));
    bi_frees(z, a, b, NULL); free(s);

    /* 6 */
    bi_prep_i32(z, -329032);
    s = bi_to_str(z);
    ASSERT(!strcmp(s, "-329032"));
    bi_free(z); free(s);

    /* Test all valid signed integer values */
    char buf[7];
    for (int16_t i = INT16_MIN; i < INT16_MAX; i++)
    {
        bi_prep_i32(a, i);
        s = bi_to_str(a);

        sprintf(buf, "%" PRId16, i);

        ASSERT(!strcmp(s, buf));

        bi_free(a); free(s);
    }

    return true;
}


/******************************************************************************
 *  Bits
 ******************************************************************************/
bool
test_bit_length_digit(void)
{
    INIT_TEST();

    ASSERT(bit_length_digit(0) == 1);
    ASSERT(bit_length_digit(1) == 1);

  #if BI_SHIFT == 64
    ASSERT(bit_length_digit(BI_MASK) == 64);
  #elif BI_SHIFT == 32
    ASSERT(bit_length_digit(BI_MASK) == 32);
  #endif

    /* Need to watch for overflow here. Didn't bother storing BI_MASK / 2. */
    for (uint64_t x = 1; x < BI_MASK / 2; x *= 2)
    {
        uint8_t len = bit_length_digit(x);
        ASSERT_M(((uint64_t)1 << (len - 1)) <= x, x < ((uint64_t)1 << len));
    }

    return true;
}

bool
test_bi_get_bit(void)
{
    INIT_TEST();

    bi_t a;
    bi_prep_i32(a, 1);
    bool bit_set, next_bit_set, prev_bit_set;

    bit_set = bi_get_bit(a, 0);
    next_bit_set = bi_get_bit(a, 1);
    ASSERT_M(bit_set, !next_bit_set);
    bi_lshift(a, a, 1);
    for (int i = 1; i < 65000; i++)
    {
        bit_set = bi_get_bit(a, i);
        next_bit_set = bi_get_bit(a, i + 1);
        prev_bit_set = bi_get_bit(a, i - 1);
        ASSERT_M(bit_set, !next_bit_set, !prev_bit_set);
        bi_lshift(a, a, 1);
    }
    bi_free(a);

    /* Test Zero */
    bi_prep(a);
    bit_set = bi_get_bit(a, 0);
    ASSERT(!bit_set);
    bit_set = bi_get_bit(a, 53042);
    ASSERT(!bit_set);

    return 1;
}

bool
test_bi_set_bit(void)
{
    INIT_TEST();

    bi_t a, zero;
    bi_prep(zero);
    bi_prep(a);

    for (int i = 0; i < BI_SHIFT; i++)
    {
        bi_set_bit(a, i);
        ASSERT_M(a->digits[0] == (digit)pow(2, i), a->n_digits == 1);
        bi_set(a, zero);
    }

    return 1;
}


/******************************************************************************
 *  bi_uint128_t
 ******************************************************************************/
bool
test_uint128_add(void)
{
    INIT_TEST();

    bi_uint128_t a = { .lo = 0xffffffffffffffff, .hi = 0};
    bi_uint128_t b = { .lo = 9, .hi = 0};

    bi_uint128_t c = bi_uint128_t_add(a, b);

    ASSERT_M(c.hi == 1, c.lo == 8);

    c = bi_uint128_t_add(b, a);

    ASSERT_M(c.hi == 1, c.lo == 8);

    /* Check add zero */
    b.lo = 0;
    b.hi = 0;
    bi_uint128_t_iadd(&c, b);
    ASSERT_M(c.hi == 1, c.lo == 8);
    c = bi_uint128_t_add(a, b);
    ASSERT_M(c.lo == 0xffffffffffffffff, c.hi == 0);

    /* Check wraparound behavior. Expect UADD(a, b) = a + b - 2^w since
     * a + b >= 2^w.
     */
    a.lo = a.hi = b.lo = b.hi = 0xffffffffffffffff;
    c = bi_uint128_t_add(a, b);

    ASSERT_M(c.lo == 0xfffffffffffffffe, c.hi == 0xffffffffffffffff);

    return 1;
}

bool
test_uint128_mul(void)
{
    INIT_TEST();

    /* Check wraparound behavior. */
    bi_uint128_t a = { .lo = 0xffffffffffffffff, .hi = 0xffffffffffffffff };
    bi_uint128_t b = { .lo = 2, .hi = 0 };
    bi_uint128_t c = bi_uint128_t_mul(a, b);
    ASSERT_M(c.lo == 0xfffffffffffffffe, c.hi == 0xffffffffffffffff);

    /* Check mult by zero. */
    b.lo = b.hi = 0;
    c = bi_uint128_t_mul(a, b);
    ASSERT_M(c.lo == 0, c.hi == 0);
    c = bi_uint128_t_mul(b, a);
    ASSERT_M(c.lo == 0, c.hi == 0);

    /* Check small numbers. */
    a.lo = 2; a.hi = 0;
    b.lo = 3; b.hi = 0;
    c = bi_uint128_t_mul(a, b);
    ASSERT_M(c.lo == 6, c.hi == 0);

    /* Check larger numbers. TODO: add large scale test against uint64_t
     * multiplication. */
    a.lo = UINT64_C(8125818854588488327); a.hi = 0;
    b.lo = UINT64_C(6911272311421472271); b.hi = 0;
    c = bi_uint128_t_mul(a, b);
    ASSERT_M(c.lo == UINT64_C(3313038996805578729),
             c.hi == UINT64_C(3044425977448415193));


    a.lo = 0xffffffffffffffff; a.hi = 0;
    b.lo = 0xffffffffffffffff; b.hi = 0;
    c = bi_uint128_t_mul(a, b);
    ASSERT_M(c.lo == 1, c.hi == UINT64_C(18446744073709551614));

    uint64_t r64;
    a.hi = 0; b.hi = 0;
    for (size_t i = 0; i < 65000; i++)
    {
        for (size_t j = i; j < 65000; j++)
        {
            uint64_t r64 = (uint64_t)i * j;

            a.lo = i;
            b.lo = j;
            c = bi_uint128_t_mul(a, b);

            ASSERT(r64 == c.lo);
        }
    }

    uint64_t i = 0xffffffff, j = 0;
    while (UMUL64_OVERFLOW(&r64, i, j) == 0)
    {
        uint64_t third_check = i * j;
        a.lo = i;
        b.lo = j++;
        c = bi_uint128_t_mul(a, b);
        ASSERT_M(r64 == c.lo, r64 == third_check);
    }
    /* 0xffffffff * 4294967297 = (2 ** 64 - 1) ==> with increment j is: */
    ASSERT(j == 4294967298);

    return 1;
}

bool
test_uint128_div(void)
{
    INIT_TEST();

    /* (2 ** 128 - 1) */
    bi_uint128_t n = { .lo = 0xffffffffffffffff, .hi = 0xffffffffffffffff };
    bi_uint128_t d = { .lo = 0x8df4a0ebf42176fe, .hi = 0};
    bi_uint128_t q, r;

    bi_uint128_t_div(n, d, &q, &r);

    ASSERT_M(q.hi == 1, q.lo == 0xcdaa60f629f5d72f, r.lo == 0x2192b27c83d5d55d,
             r.hi == 0);

    d.lo = 0x3cb14b5ead8e2a1c;
    d.hi = 0xff133ee1f10e26db;

    bi_uint128_t_div(n, d, &q, &r);

    ASSERT_M(q.lo == 1, q.hi == 0, r.lo == 0xc34eb4a15271d5e3,
             r.hi == 0xecc11e0ef1d924);

    d.lo = 1;
    d.hi = 0;

    bi_uint128_t_div(n, d, &q, &r);

    ASSERT_M(q.lo == 0xffffffffffffffff, q.hi == 0xffffffffffffffff, r.lo == 0,
             r.hi == 0);

    d.lo = 0xffffffffffffffff;
    d.hi = 0xffffffffffffffff;

    bi_uint128_t_div(n, d, &q, &r);

    ASSERT_M(q.lo == 1, q.hi == 0, r.lo == 0, r.hi == 0);

    /* Uncomment these lines to test division by 0. */
    // d.lo = 0;
    // d.hi = 0;
    // bi_uint128_t_div(n, d, &q, &r);

    return 1;
}


/******************************************************************************
 *  Test Preparing/Initializing Integers
 ******************************************************************************/
bool
test_prep(void)
{
    INIT_TEST();

    bi_t a;
    bi_prep(a);
    ASSERT_M(a->n_digits == 0, a->n_alloc == 0, a->digits == NULL);
    bi_free(a);

    return 1;
}

bool
test_prep_u32(void)
{
    INIT_TEST();

    bi_t a;

    /* Test zero */
    bi_prep_u32(a, 0);
    ASSERT_M(a->n_digits == 0, a->n_alloc == 0, a->digits == NULL);
    bi_free(a);

    /* Test small values */
    for (uint32_t i = 1; i < UINT16_MAX; i++) {
        bi_prep_u32(a, i);
        ASSERT_M(a->digits[0] == i, a->n_digits == 1, a->n_alloc == 1);
        bi_free(a);
    }

    /* Test large values */
    for (uint32_t i = 0; i < UINT16_MAX; i++) {
        bi_prep_u32(a, UINT32_MAX - i);
        ASSERT_M(a->digits[0] == (UINT32_MAX - i), a->n_digits == 1,
                 a->n_alloc == 1);
        bi_free(a);
    }

    return 1;
}

bool
test_prep_i32(void)
{
    INIT_TEST();

    bi_t a;

    /* Test zero */
    bi_prep_i32(a, 0);
    ASSERT_M(a->n_digits == 0, a->n_alloc == 0, a->digits == NULL);
    bi_free(a);

    /* Test small absolute values */
    for (int32_t i = INT16_MIN; i < 0; i++) {
        bi_prep_i32(a, i);
        ASSERT_M(a->digits[0] == (uint32_t)-i, a->n_digits == -1);
        bi_free(a);
    }

    for (int32_t i = 1; i <= INT16_MAX; i++) {
        bi_prep_i32(a, i);
        ASSERT_M(a->digits[0] == (uint32_t)i, a->n_digits == 1);
        bi_free(a);
    }

    /* Test large absolute values */
    for (uint16_t i = 0; i < UINT16_MAX; i++) {
        bi_prep_i32(a, INT32_MAX - i);
        ASSERT_M(a->digits[0] == (digit)(INT32_MAX - i), a->n_digits == 1,
                 a->n_alloc == 1);
        bi_free(a);
    }

    for (uint16_t i = 0; i < UINT16_MAX; i++) {
        bi_prep_i32(a, INT32_MIN + i);
        ASSERT_M(a->digits[0] == (digit)-(stwodigits)(INT32_MIN + i),
                 a->n_digits == -1, a->n_alloc == 1);
        bi_free(a);
    }

    return 1;
}

bool
test_prep_u64(void)
{
    INIT_TEST();

    bi_t a;

    /* Test zero */
    bi_prep_u64(a, 0);
    ASSERT_M(a->n_digits == 0, a->n_alloc == 0, a->digits == NULL);
    bi_free(a);

    /* Test small values */
    for (uint16_t i = 1; i < UINT16_MAX; i++) {
        bi_prep_u64(a, i);
        ASSERT_M(a->n_digits == 1, a->n_alloc == 1, a->digits[0] == i);
        bi_free(a);
    }

    /* Test large values */
  #if BI_SHIFT == 64
    for (uint16_t i = 0; i < UINT16_MAX; i++) {
        bi_prep_u64(a, UINT64_MAX - i);
        ASSERT_M(a->n_digits == 1, a->n_alloc == 1,
                 a->digits[0] == UINT64_MAX - i);
        bi_free(a);
    }
  #elif BI_SHIFT == 32
    /* TODO. */
  #endif

    return 1;
}

bool
test_prep_i64(void)
{
    INIT_TEST();

    bi_t a;

    /* Test zero */
    bi_prep_i64(a, 0);
    ASSERT_M(a->n_digits == 0, a->n_alloc == 0, a->digits == NULL);
    bi_free(a);

    /* Test small absolute values */
    for (int32_t i = INT16_MIN; i < 0; i++) {
        bi_prep_i64(a, i);
        ASSERT_M(a->digits[0] == (uint32_t)-i, a->n_digits == -1);
        bi_free(a);
    }
    for (int32_t i = 1; i <= INT16_MAX; i++) {
        bi_prep_i64(a, i);
        ASSERT_M(a->digits[0] == (uint32_t)i, a->n_digits == 1);
        bi_free(a);
    }

    /* Test large absolute values */
  #if BI_SHIFT == 64
    for (uint16_t i = 0; i < UINT16_MAX; i++) {
        bi_prep_i64(a, INT64_MAX - i);
        ASSERT_M(a->digits[0] == (digit)(INT64_MAX - (int64_t)i),
                 a->n_digits == 1, a->n_alloc == 1);
        bi_free(a);
    }

    for (uint16_t i = 0; i < UINT16_MAX; i++) {
        bi_prep_i64(a, INT64_MIN + i);
        ASSERT_M(a->digits[0] == (digit)-(stwodigits)(INT64_MIN + i),
                 a->n_digits == -1, a->n_alloc == 1);
        bi_free(a);
    }
  #elif BI_SHIFT == 32
    /* TODO. */
  #endif

    return 1;
}


/******************************************************************************
 *  Test void bi_abs(bi_t to, const bi_t a): implements to = |a|.
 ******************************************************************************/
bool
test_bi_abs(void)
{
    INIT_TEST();

    bi_t a, b;
    int asize;
    char *s;

    /** Single-digit integer **/
    /* Prepare integers */
    bi_prep_i32(a, -320923);
    bi_prep(b);

    ASSERT_M(a->digits[0] == 320923, a->n_digits == -1, a->n_alloc == 1);
    ASSERT_M(b->digits == NULL, b->n_digits == 0, b->n_alloc == 0);

    /* Test b = |a| when b->digits != a->digits */
    bi_abs(b, a);
    ASSERT_M(b->digits[0] == 320923, b->n_digits == 1, b->n_alloc == 1);

    /* Test a = |a|. */
    bi_abs(a, a);
    ASSERT_M(a->digits[0] == 320923, a->n_digits == 1, a->n_alloc == 1);

    /* Free integers */
    bi_frees(a, b, NULL);

    /** Multi-digit integer **/
    /* Prepare integers */
    bi_prep_i64(a, -93029322093223);
    bi_lshift(a, a, 92);
    bi_prep(b);
    asize = ABS(a->n_digits);

    /* Ensure a as expected */
    s = bi_to_str(a);
    ASSERT(!strcmp(s, "-460658890587107103073360601277668742135808")); free(s);
    ASSERT(a->n_digits <= -2);

    /* Test b = |a| */
    bi_abs(b, a);
    s = bi_to_str(b);
    ASSERT(!strcmp(s, "460658890587107103073360601277668742135808")); free(s);
    ASSERT(b->n_digits >= 2);

    for (int i = 0; i < asize; i++) {
        ASSERT(a->digits[i] == b->digits[i]);
    }

    /* Test a = |a| */
    bi_abs(a, a);
    s = bi_to_str(a);
    ASSERT(!strcmp(s, "460658890587107103073360601277668742135808")); free(s);
    ASSERT(a->n_digits >= 2);

    for (int i = 0; i < asize; i++) {
        ASSERT(a->digits[i] == b->digits[i]);
    }

    /* Free integers */
    bi_frees(a, b, NULL);

    return 1;
}


///////////////////////////////////////////////////////////////////////////////
//  Register the Test Functions
///////////////////////////////////////////////////////////////////////////////
bool (*test_functions[])(void) = {
    test_uint128_div,
    test_uint128_mul,
    test_uint128_add,
    test_bi_mul,
    test_bi_set,
    test_bit_length_digit,
    test_bi_to_str,
    test_bi_irshift,
    test_bi_lshift,
    test_bi_divide_qr,
    test_bi_sub_abs,
    test_bi_divide_by_digit,
    test_bi_prep_str,
    test_bi_get_bit,
    test_bi_set_bit,
    test_prep,
    test_prep_u32,
    test_prep_i32,
    test_prep_u64,
    test_prep_i64,
    test_bi_abs
};

void
test(void)
{
    size_t n_functions = sizeof(test_functions) / sizeof(test_functions[0]);
    size_t n_successful = 0;

    printf("\nn_tests: %zu\n", n_functions);
    for (size_t i = 0; i < n_functions; i++)
    {
        if (test_functions[i]()) {
            n_successful++;
            printf(INDENT "Success!\n");
        } else {
            printf(INDENT "Failure!\n");
        }

        printf("Completed: %zu Success: %zu Failure: %zu\n",
               i + 1, n_successful, i + 1 - n_successful);
    }
    putchar('\n');
}


///////////////////////////////////////////////////////////////////////////////
//  Main
///////////////////////////////////////////////////////////////////////////////
int
main(void)
{
    test();
}
