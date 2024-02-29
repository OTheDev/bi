/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#include <gtest/gtest.h>

#include <array>
#include <charconv>
#include <cmath>
#include <functional>
#include <limits>
#include <random>
#include <string>

#include "bi.hpp"
#include "bi_exceptions.hpp"
#include "constants.hpp"
#include "int128.hpp"
#include "uints.hpp"

namespace {

using bi::bi_dmax;
using bi::bi_dwidth;
using bi::bi_t;
using bi::ddigit;
using bi::digit;
using bi::operator""_bi;
// We can safely ignore MSVC warning C4455 in this instance
using std::literals::string_literals::operator""s;
#if defined(BI_DIGIT_64_BIT)
using sdigit = int64_t;
using sddigit = __int128;
#else
using sdigit = int32_t;
using sddigit = int64_t;
#ifdef __SIZEOF_INT128__
using qdigit = unsigned __int128;
using sqdigit = __int128;
constexpr qdigit qdigit_min = std::numeric_limits<qdigit>::min();
constexpr qdigit qdigit_max = std::numeric_limits<qdigit>::max();
constexpr sqdigit sqdigit_min = std::numeric_limits<sqdigit>::min();
constexpr sqdigit sqdigit_max = std::numeric_limits<sqdigit>::max();
#endif
#endif

constexpr digit digit_min = std::numeric_limits<digit>::min();
constexpr digit digit_max = std::numeric_limits<digit>::max();
constexpr sdigit sdigit_min = std::numeric_limits<sdigit>::min();
constexpr sdigit sdigit_max = std::numeric_limits<sdigit>::max();
constexpr ddigit ddigit_min = std::numeric_limits<ddigit>::min();
constexpr ddigit ddigit_max = std::numeric_limits<ddigit>::max();
constexpr sddigit sddigit_min = std::numeric_limits<sddigit>::min();
constexpr sddigit sddigit_max = std::numeric_limits<sddigit>::max();

class BITest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

template <typename T>
std::string integral_type_name() {
  if constexpr (std::is_same_v<T, int>)
    return "int";
  else if constexpr (std::is_same_v<T, long>)
    return "long";
  else if constexpr (std::is_same_v<T, long long>)
    return "long long";
  else if constexpr (std::is_same_v<T, unsigned int>)
    return "unsigned int";
  else if constexpr (std::is_same_v<T, unsigned long>)
    return "unsigned long";
  else if constexpr (std::is_same_v<T, unsigned long long>)
    return "unsigned long long";
  else if constexpr (std::is_same_v<T, int32_t>)
    return "int32_t";
  else if constexpr (std::is_same_v<T, int64_t>)
    return "int64_t";
  else if constexpr (std::is_same_v<T, intmax_t>)
    return "intmax_t";
  else if constexpr (std::is_same_v<T, uint32_t>)
    return "uint32_t";
  else if constexpr (std::is_same_v<T, uint64_t>)
    return "uint64_t";
  else if constexpr (std::is_same_v<T, uintmax_t>)
    return "uintmax_t";
  else
    return "Unknown Type";
}

template <typename T, bool Assign>
void test_integer_value(T value, const std::string& expected_str) {
  bi_t x(value);

  EXPECT_EQ(x.to_string(), expected_str)
      << "Failed for type: " << integral_type_name<T>() << " (" << sizeof(T)
      << ")" << std::endl;

  if constexpr (Assign) {
    // Assignment from initialized nonzero value
    x = value;
    EXPECT_EQ(x.to_string(), expected_str);

    // Assignment from default-initialized value
    bi_t y;
    y = value;
    EXPECT_EQ(y.to_string(), expected_str);
  }
}

template <typename T, bool Assign>
void test_integer() {
  T min = std::numeric_limits<T>::min();
  T max = std::numeric_limits<T>::max();

  test_integer_value<T, Assign>(0, "0");
  test_integer_value<T, Assign>(min, std::to_string(min));
  test_integer_value<T, Assign>(max, std::to_string(max));
}

template <typename T, typename Operator>
void test_binary_operator(Operator op) {
  std::random_device rdev;
  std::mt19937_64 rng(rdev());
  std::uniform_int_distribution<T> udist(std::numeric_limits<T>::min(),
                                         std::numeric_limits<T>::max());
  for (int16_t i = 0; i < INT16_MAX; ++i) {
    T a_in = udist(rng), b_in = udist(rng);
    bi_t a = a_in, b = b_in;
    ASSERT_TRUE(op(a, b) == op(a_in, b_in));
  }
}

enum class BinOp { Add, Sub, Mul };

template <typename T, BinOp op_type>
void test_binop_overflow() {
  std::random_device rdev;
  std::mt19937_64 rng(rdev());
  std::uniform_int_distribution<T> udist(std::numeric_limits<T>::min(),
                                         std::numeric_limits<T>::max());

  for (int16_t i = 0; i < INT16_MAX; ++i) {
    T a_in = udist(rng), b_in = udist(rng);
    bi_t a = a_in, b = b_in;

    if constexpr (op_type == BinOp::Add) {
      auto [result, overflow] = uints::uadd_overflow(a_in, b_in);
      if (!overflow) {
        ASSERT_EQ(a + b, result);
      }
    } else if constexpr (op_type == BinOp::Sub) {
      auto [result, overflow] = uints::usub_overflow(a_in, b_in);
      if (!overflow) {
        ASSERT_EQ(a - b, result);
      }
    } else if constexpr (op_type == BinOp::Mul) {
      auto [result, overflow] = uints::umul_overflow(a_in, b_in);
      if (!overflow) {
        ASSERT_EQ(a * b, result);
      }
    }
  }
}

TEST_F(BITest, DefaultConstructor) {
  bi_t x;
  EXPECT_EQ(x, 0);
  EXPECT_EQ(x.size(), 0);
  EXPECT_TRUE(!x.negative());
  EXPECT_EQ(x.to_string(), "0");
  EXPECT_EQ(x.capacity(), 0);
}

TEST_F(BITest, TestIntegralTypes) {
  auto test_all = [](auto... args) {
    (test_integer<decltype(args), false>(), ...);
    (test_integer<decltype(args), true>(), ...);
  };

  test_all(0, 0l, 0ll, 0u, 0ul, 0ull,
#ifdef INT32_MAX
           int32_t{},
#endif
#ifdef INT64_MAX
           int64_t{},
#endif
#ifdef UINT32_MAX
           uint32_t{},
#endif
#ifdef UINT64_MAX
           uint64_t{},
#endif
           intmax_t{}, uintmax_t{});
}

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

/*  Tests constructing from small and large unsigned and signed integers for
 *  both 32-bit digits and 64-bit digits. Verifies that
 *    (1) size() is as expected.
 *    (2) capacity() >= size().
 *    (3) a.to_string() == std::to_string(built_in_integer).
 *    (4) the internal representation of digits is as expected via the digits()
 *        span.
 *    (5) a == built_in_integer (which doesn't cast the RHS to bi_t; tests
 *        efficient comparisons).
 *  For the 32-bit case, multiple digit (2) bi_t integers are also tested
 *  through large `uint64_t`s and `int64_t`s.
 */
TEST_F(BITest, TestConstructFromIntegrals) {
  // Small unsigned
  for (uint16_t i = 1; i < UINT16_MAX; ++i) {
    bi_t a{i};
    ASSERT_EQ(a.size(), 1);
    ASSERT_GE(a.capacity(), a.size());
    ASSERT_EQ(a.digits()[0], i);
  }

  // Large unsigned
  for (uint16_t i = 0; i < UINT16_MAX; ++i) {
    bi_t a{UINT64_MAX - i};
    if constexpr (bi_dwidth == 64) {
      ASSERT_EQ(a.size(), 1);
      ASSERT_EQ(a.digits()[0], UINT64_MAX - i);
    } else if (bi_dwidth == 32) {
      ASSERT_EQ(a.size(), 2);
      ASSERT_EQ(a.digits()[1], bi_dmax);
      ASSERT_EQ(a.digits()[0], bi_dmax - i);
    }
    ASSERT_GE(a.capacity(), a.size());
    ASSERT_EQ(a.to_string(), std::to_string(UINT64_MAX - i));
    ASSERT_EQ(a, UINT64_MAX - i);
  }

  // Signed ints, small absolute value
  for (int32_t i = INT16_MIN; i < 0; ++i) {
    bi_t a{i};
    ASSERT_EQ(a.negative(), true);
    ASSERT_EQ(a.size(), 1);
    ASSERT_GE(a.capacity(), a.size());
    ASSERT_EQ(a.digits()[0], -i);
    ASSERT_EQ(a, i);
  }
  for (int32_t i = 1; i <= INT16_MAX; ++i) {
    bi_t a{i};
    ASSERT_EQ(a.negative(), false);
    ASSERT_EQ(a.size(), 1);
    ASSERT_GE(a.capacity(), a.size());
    ASSERT_EQ(a.digits()[0], i);
    ASSERT_EQ(a, i);
  }

  // Signed ints, large absolute value
  for (uint16_t i = 0; i < UINT16_MAX; ++i) {
    bi_t a{INT64_MAX - i};
    if constexpr (bi_dwidth == 64) {
      ASSERT_EQ(a.size(), 1);
      ASSERT_EQ(a.digits()[0], INT64_MAX - i);
    } else if (bi_dwidth == 32) {
      ASSERT_EQ(a.size(), 2);
      ASSERT_EQ(a.digits()[1], (static_cast<uint32_t>(1) << 31) - 1);
      ASSERT_EQ(a.digits()[0], bi_dmax - i);
    }
    ASSERT_GE(a.capacity(), a.size());
    ASSERT_TRUE(a == INT64_MAX - i);
    ASSERT_EQ(a.to_string(), std::to_string(INT64_MAX - i));
  }
  for (uint16_t i = 0; i < UINT16_MAX; ++i) {
    bi_t a{INT64_MIN + i};
    if constexpr (bi_dwidth == 64) {
      ASSERT_EQ(a.size(), 1);
      ASSERT_EQ(a.digits()[0], -static_cast<uint64_t>(INT64_MIN + i));
    } else if (bi_dwidth == 32) {
      if (i == 0) {
        ASSERT_EQ(a.digits()[1], static_cast<uint32_t>(1) << 31);
        ASSERT_EQ(a.digits()[0], i);
      } else {
        ASSERT_EQ(a.digits()[1], (static_cast<uint32_t>(1) << 31) - 1);
        ASSERT_EQ(a.digits()[0], bi_dmax - i + 1);
      }
    }
    ASSERT_GE(a.capacity(), a.size());
    ASSERT_TRUE(a == INT64_MIN + i);
    ASSERT_EQ(a.to_string(), std::to_string(INT64_MIN + i));
  }
}

// Supports builtin integers plus __int128 / unsigned __int128 if the
// implementation supports it.
template <typename T>
std::string bltin_int_to_string(T value) {
#if defined(__SIZEOF_INT128__)
  if constexpr (std::is_same<T, __int128>::value ||
                std::is_same<T, unsigned __int128>::value) {
    return bi_int128::to_string(value);
  } else {
#endif
    return std::to_string(value);
#if defined(__SIZEOF_INT128__)
  }
#endif
}

template <std::integral T>
std::string to_string_base(T number, int base) {
  std::array<char, 129> buffer{};

  // auto result = std::to_chars(buffer.begin(), buffer.end(), number, base);
  auto result =
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      std::to_chars(&buffer[0], &buffer[0] + buffer.size(), number, base);
  if (result.ec != std::errc()) {
    throw std::runtime_error("Failed to convert number to string.");
  }

  // std::string str(buffer.begin(), result.ptr);
  std::string str(&buffer[0], result.ptr);

  return str;
}

void test_construct_from_string(int base) {
  ASSERT_EQ(bi_t("0", base), 0);
  ASSERT_EQ(bi_t(to_string_base(digit_max, base), base), digit_max);
  ASSERT_EQ(bi_t(to_string_base(ddigit_max, base), base), ddigit_max);
  ASSERT_EQ(bi_t(to_string_base(sdigit_min, base), base), sdigit_min);
  ASSERT_EQ(bi_t(to_string_base(sdigit_max, base), base), sdigit_max);
  ASSERT_EQ(bi_t(to_string_base(sddigit_min, base), base), sddigit_min);
  ASSERT_EQ(bi_t(to_string_base(sddigit_max, base), base), sddigit_max);
#if defined(__SIZEOF_INT128__) && defined(BI_DIGIT_32_BIT)
  ASSERT_EQ(bi_t(to_string_base(qdigit_max, base), base), qdigit_max);
  ASSERT_EQ(bi_t(to_string_base(sqdigit_min, base), base), sqdigit_min);
  ASSERT_EQ(bi_t(to_string_base(sqdigit_max, base), base), sqdigit_max);
#endif

  std::random_device rdev;
  std::mt19937 rng(rdev());
  std::uniform_int_distribution<sddigit> udist(sddigit_min, sddigit_max);
  std::uniform_int_distribution<int16_t> udist_16(INT16_MIN, INT16_MAX);
  for (int16_t i = 0; i < INT16_MAX / 2; ++i) {
    sddigit rv = udist(rng);
    std::string s = to_string_base(rv, base);
    ASSERT_EQ(bi_t(s, base), rv);

    int16_t rv_16 = udist_16(rng);
    s = to_string_base(rv_16, base);
    ASSERT_EQ(bi_t(s, base), rv_16);
  }
}

TEST_F(BITest, ConstructFromString) {
  EXPECT_EQ(bi_t{"0"s}, 0);
  EXPECT_EQ(bi_t{"-0"s}, 0);
  EXPECT_EQ(bi_t{"+0"s}, 0);
  EXPECT_EQ(bi_t{"     0 "}, 0);
  EXPECT_EQ(bi_t{"      -0"}, 0);

  EXPECT_EQ(bi_t{"987"s}, 987);
  EXPECT_EQ(bi_t{"-987"s}, -987);
  EXPECT_EQ(bi_t{"+987"s}, 987);
  EXPECT_EQ(bi_t{"  -987"}, -987);

  EXPECT_EQ(bi_t{"+00100"}, 100);
  EXPECT_EQ(bi_t{"+000000"}, 0);
  EXPECT_EQ(bi_t{"    00009876"}, 9876);

  auto str = "999909090093232329302932309230930923230992094029424204"s;
  bi_t large{str};
  EXPECT_EQ(large.to_string(), str);
  auto strn = "-9999090900932323293029323092309309232309920940294242"s;
  bi_t large_n{strn};
  EXPECT_EQ(large_n.to_string(), strn);

  EXPECT_THROW(bi_t{""}, std::invalid_argument);
  EXPECT_THROW(bi_t{nullptr}, std::invalid_argument);
  EXPECT_THROW(bi_t{"  -"}, std::invalid_argument);
  EXPECT_THROW(bi_t invalid{"      -"}, std::invalid_argument);
  EXPECT_THROW(bi_t{"     "}, std::invalid_argument);
  EXPECT_THROW(bi_t{""}, std::invalid_argument);

  EXPECT_THROW(bi_t("0", -1), std::invalid_argument);
  EXPECT_THROW(bi_t("0", 0), std::invalid_argument);
  EXPECT_THROW(bi_t("0", 1), std::invalid_argument);
  EXPECT_THROW(bi_t("0", 37), std::invalid_argument);

  for (int i = 2; i <= 36; ++i) {
    test_construct_from_string(i);
  }
}

// Note: this is defined in terms of the string constructor
TEST_F(BITest, UserDefinedLiteral) {
  bi_t pos = 123456789012345678901234567890123456789043909809801329009092930_bi;
  EXPECT_EQ(pos.to_string(),
            "123456789012345678901234567890123456789043909809801329009092930"s);

  bi_t neg = -12345678901234567890123456789012345678909098909809802340982349_bi;
  EXPECT_EQ(neg.to_string(),
            "-12345678901234567890123456789012345678909098909809802340982349"s);

  bi_t zero = 0_bi;
  EXPECT_EQ(zero.to_string(), "0"s);
}

TEST_F(BITest, CopyConstructor) {
  bi_t original(1234);
  bi_t copy = original;

  EXPECT_EQ(copy, original);
  EXPECT_TRUE(!copy.negative());

  original = 5678;
  EXPECT_NE(copy, original);
  EXPECT_EQ(original, 5678);

  bi_t a(-1234);
  bi_t a_copy = a;
  EXPECT_EQ(a, a_copy);

  bi_t b;
  bi_t b_copy = b;
  EXPECT_EQ(b, b_copy);
}

TEST_F(BITest, CopyAssignmentOperator) {
  bi_t original(1234);
  bi_t copy;
  copy = original;

  EXPECT_EQ(copy, original);
  EXPECT_EQ(copy, 1234);

  bi_t zero;
  copy = zero;
  EXPECT_EQ(copy, zero);
  EXPECT_EQ(copy, 0);

  bi_t zero_2;
  zero_2 = zero;
  EXPECT_EQ(zero_2, zero);
  EXPECT_EQ(zero_2, 0);

  original = -5000;
  copy = original;
  EXPECT_EQ(copy, original);
  EXPECT_EQ(copy, -5000);

  original = ddigit_max;
  copy = original;
  EXPECT_EQ(copy, original);
  EXPECT_EQ(copy, ddigit_max);

  original = sddigit_min;
  copy = original;
  EXPECT_EQ(copy, original);
  EXPECT_EQ(copy, sddigit_min);
}

TEST_F(BITest, MoveConstructor) {
  bi_t original(1234);
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
  bi_t moved(std::move(original));
  EXPECT_EQ(moved, 1234);
  EXPECT_TRUE(!moved.negative());
  // Post-move state of a `bi` object is equivalent to it being 0
  EXPECT_EQ(original, 0);

  bi_t a(-1234);
  bi_t a_moved(std::move(a));
  EXPECT_EQ(a_moved, -1234);
  EXPECT_TRUE(a_moved.negative());

  bi_t b;
  bi_t b_moved(std::move(b));
  EXPECT_EQ(b_moved, 0);
  EXPECT_EQ(b_moved.size(), 0);
  EXPECT_TRUE(!b_moved.negative());
}

TEST_F(BITest, MoveAssignmentOperator) {
  bi_t original(1234);
  bi_t moved;
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
  moved = std::move(original);

  EXPECT_EQ(moved, 1234);
  EXPECT_EQ(original, 0);
}

TEST_F(BITest, UnaryPlus) {
  bi_t a(10);
  bi_t b(-10);
  bi_t zero;

  EXPECT_EQ(+a, 10);
  EXPECT_EQ(+b, -10);
  EXPECT_EQ(+zero, 0);
}

TEST_F(BITest, UnaryMinus) {
  bi_t a(10);
  bi_t b(-10);
  bi_t zero;

  EXPECT_EQ(-a, -10);
  EXPECT_EQ(-b, 10);
  EXPECT_EQ(-zero, 0);
}

TEST_F(BITest, Sign) {
  bi_t zero = 0_bi;
  bi_t positive = 123_bi;
  bi_t negative = -123_bi;

  EXPECT_EQ(zero.sign(), 0);
  EXPECT_EQ(positive.sign(), 1);
  EXPECT_EQ(negative.sign(), -1);
}

TEST_F(BITest, Negate) {
  bi_t zero = 0_bi;
  bi_t number = 123_bi;

  // Negation of zero
  zero.negate();
  EXPECT_EQ(zero.sign(), 0);
  EXPECT_EQ(zero, 0);

  // Negation of a positive number
  number.negate();
  EXPECT_EQ(number.sign(), -1);
  EXPECT_EQ(number, -123);

  // Negation of a negative number
  number.negate();
  EXPECT_EQ(number.sign(), 1);
  EXPECT_EQ(number, 123);

  // Double negation should bring it back to original
  number.negate();
  EXPECT_EQ(number.sign(), -1);
  number.negate();
  EXPECT_EQ(number.sign(), 1);
  EXPECT_EQ(number, 123);
}

TEST_F(BITest, EqualityOperator) {
  bi_t x(10);
  bi_t y(10);
  bi_t z(-10);
  bi_t default_constructed;

  EXPECT_TRUE(x == y);
  EXPECT_FALSE(x == z);
  EXPECT_FALSE(x == default_constructed);

  test_binary_operator<sddigit>([](auto lhs, auto rhs) { return lhs == rhs; });
}

TEST_F(BITest, InequalityOperator) {
  bi_t x(10);
  bi_t y(10);
  bi_t z(-10);
  bi_t default_constructed;

  EXPECT_FALSE(x != y);
  EXPECT_TRUE(x != z);
  EXPECT_TRUE(x != default_constructed);

  test_binary_operator<sddigit>([](auto lhs, auto rhs) { return lhs != rhs; });
}

TEST_F(BITest, LessThanOperator) {
  bi_t x(10);
  bi_t y(-10);
  bi_t default_constructed;

  EXPECT_TRUE(y < x);
  EXPECT_FALSE(x < y);
  EXPECT_TRUE(default_constructed < x);
  EXPECT_FALSE(x < default_constructed);

  test_binary_operator<sddigit>([](auto lhs, auto rhs) { return lhs < rhs; });
}

TEST_F(BITest, GreaterThanOperator) {
  bi_t x(10);
  bi_t y(-10);
  bi_t default_constructed;

  EXPECT_FALSE(y > x);
  EXPECT_TRUE(x > y);
  EXPECT_FALSE(default_constructed > x);
  EXPECT_TRUE(x > default_constructed);

  test_binary_operator<sddigit>([](auto lhs, auto rhs) { return lhs > rhs; });
}

TEST_F(BITest, LessThanOrEqualOperator) {
  bi_t x(10);
  bi_t y(10);
  bi_t z(-10);
  bi_t default_constructed;

  EXPECT_TRUE(x <= y);
  EXPECT_FALSE(x <= z);
  EXPECT_TRUE(z <= x);
  EXPECT_TRUE(default_constructed <= x);
  EXPECT_FALSE(x <= default_constructed);

  test_binary_operator<sddigit>([](auto lhs, auto rhs) { return lhs <= rhs; });
}

TEST_F(BITest, GreaterThanOrEqualOperator) {
  bi_t x(10);
  bi_t y(10);
  bi_t z(-10);
  bi_t default_constructed;

  EXPECT_TRUE(x >= y);
  EXPECT_TRUE(x >= z);
  EXPECT_FALSE(z >= x);
  EXPECT_FALSE(default_constructed >= x);
  EXPECT_TRUE(x >= default_constructed);

  test_binary_operator<sddigit>([](auto lhs, auto rhs) { return lhs >= rhs; });
}

TEST_F(BITest, Addition) {
  bi_t a(10);
  bi_t b(-5);
  bi_t zero;

  EXPECT_EQ(a + b, 5);
  EXPECT_EQ(b + a, 5);
  EXPECT_EQ(a + zero, 10);
  EXPECT_EQ(zero + a, 10);
  EXPECT_EQ(b + zero, -5);
  EXPECT_EQ(zero + b, -5);
  EXPECT_EQ(a + a, 20);
  EXPECT_EQ(b + b, -10);

  // Important edge case to test
  bi_t c{ddigit_max};
  bi_t d = c;
  if constexpr (bi_dwidth == 64) {
    EXPECT_EQ((d + c).to_string(), "680564733841876926926749214863536422910");
  } else if constexpr (bi_dwidth == 32) {
    EXPECT_EQ((d + c).to_string(), "36893488147419103230");
  }

  std::random_device rdev;
  std::mt19937_64 rng(rdev());

  std::uniform_int_distribution<sdigit> udist_sdigit(sdigit_min, sdigit_max);
  for (int16_t i = 0; i < INT16_MAX; ++i) {
    sdigit a_in = udist_sdigit(rng), b_in = udist_sdigit(rng);
    a = a_in;
    b = b_in;
    ASSERT_EQ(a + b, static_cast<sddigit>(a_in) + b_in);
  }

  test_binop_overflow<ddigit, BinOp::Add>();
}

TEST_F(BITest, Subtraction) {
  bi_t a(10);
  bi_t b(-5);
  bi_t zero;

  EXPECT_EQ(a - b, 15);
  EXPECT_EQ(b - a, -15);
  EXPECT_EQ(a - zero, 10);
  EXPECT_EQ(zero - a, -10);
  EXPECT_EQ(zero - b, 5);
  EXPECT_EQ(b - zero, -5);
  EXPECT_EQ(a - a, 0);
  EXPECT_EQ(b - b, 0);

  std::random_device rdev;
  std::mt19937_64 rng(rdev());

  std::uniform_int_distribution<sdigit> udist_sdigit(sdigit_min, sdigit_max);
  for (int16_t i = 0; i < INT16_MAX; ++i) {
    sdigit a_in = udist_sdigit(rng), b_in = udist_sdigit(rng);
    a = a_in;
    b = b_in;
    ASSERT_EQ(a - b, static_cast<sddigit>(a_in) - b_in);
  }

  test_binop_overflow<ddigit, BinOp::Sub>();
}

TEST_F(BITest, Multiplication) {
  bi_t a(10);
  bi_t b(-5);
  bi_t zero;

  EXPECT_EQ(a * b, -50);
  EXPECT_EQ(b * a, -50);
  EXPECT_EQ(a * zero, 0);
  EXPECT_EQ(a * a, 100);
  EXPECT_EQ(b * b, 25);

  std::random_device rdev;
  std::mt19937_64 rng(rdev());

  std::uniform_int_distribution<digit> udist_digit(digit_min, digit_max);
  for (int16_t i = 0; i < INT16_MAX; ++i) {
    digit rval = udist_digit(rng);

    bi_t r{rval};
    bi_t mask_bi_t{bi_dmax};
    ASSERT_TRUE(r * mask_bi_t == rval * static_cast<ddigit>(bi_dmax));

    ddigit maskp2 = static_cast<ddigit>(bi_dmax) + 2;
    bi_t maskp2_bi_t{maskp2};
    ASSERT_TRUE(r * maskp2_bi_t == rval * maskp2);
  }

  std::uniform_int_distribution<sdigit> udist_sdigit(sdigit_min, sdigit_max);
  for (int16_t i = 0; i < INT16_MAX; ++i) {
    sdigit rval{udist_sdigit(rng)};
    bi_t r{rval};
    bi_t mask_bi_t{bi_dmax};
    ASSERT_TRUE(r * mask_bi_t == rval * static_cast<sddigit>(bi_dmax));
  }

  a = ddigit_max;
  b = a;
  if constexpr (bi_dwidth == 64) {
    EXPECT_EQ((a * b).to_string(),
              "1157920892373161954235709850086879078525894199317986871125308347"
              "93049593217025");
  } else if (bi_dwidth == 32) {
    EXPECT_EQ((a * b).to_string(), "340282366920938463426481119284349108225");
  }

  b = sddigit_min;
  if constexpr (bi_dwidth == 64) {
    EXPECT_EQ((a * b).to_string(),
              "-578960446186580977117854925043439539264648511493598127879971047"
              "00240680714240");
  } else if (bi_dwidth == 32) {
    EXPECT_EQ((a * b).to_string(), "-170141183460469231722463931679029329920");
  }

  test_binop_overflow<ddigit, BinOp::Mul>();
}

TEST_F(BITest, Division) {
  bi_t a(10);
  bi_t b(-5);
  bi_t c(2);

  EXPECT_EQ(a / c, 5);
  EXPECT_EQ(b / c, -2);
  EXPECT_EQ(a, 10);
  EXPECT_EQ(b, -5);
  EXPECT_EQ(c, 2);
  EXPECT_EQ(a / b, -2);
}

TEST_F(BITest, DivisionAssignment) {
  bi_t a(10);
  bi_t b(-5);
  a /= b;
  EXPECT_EQ(a, -2);

  bi_t c(49);
  c /= c;
  EXPECT_EQ(c, 1);
}

TEST_F(BITest, Modulus) {
  bi_t a(10);
  bi_t b(3);

  EXPECT_EQ(a % b, 1);
}

TEST_F(BITest, ModulusAssignment) {
  bi_t a(10);
  bi_t b(3);
  a %= b;
  EXPECT_EQ(a, 1);

  bi_t c(49);
  c %= c;
  EXPECT_EQ(c, 0);
}

TEST_F(BITest, DivisionAndRemainder) {
  bi_t a(10);
  bi_t b(3);
  bi_t zero;
  auto result = a.div(b);

  EXPECT_EQ(result.first, 3);
  EXPECT_EQ(result.second, 1);
  EXPECT_THROW(a.div(zero), bi::division_by_zero);

  auto [qt, rm] = bi_t{10}.div(bi_t{-2});
  EXPECT_EQ(qt, -5);
  EXPECT_EQ(rm, 0);

  a = sddigit_min;
  b = -1;
  auto [quot, rem] = a.div(b);
  EXPECT_EQ(quot, static_cast<ddigit>(1) + sddigit_max);
  EXPECT_EQ(rem, 0);

  ddigit a_in = 13565672763181344623u, b_in = 10964129492588451979u;
  a = a_in;
  b = b_in;
  std::tie(quot, rem) = a.div(b);
  EXPECT_EQ(quot, a_in / b_in);
  EXPECT_EQ(rem, a_in % b_in);

  /* If bi_dbits == 32, TRUE:
   * q_hat * vpp > bi_base * r_hat + u_norm[j+n-2]) and q_hat < bi_base */
  bi_t ap{"23763499325903112001635915745003616971301114662694927266435717575054"
          "0350033099851627590"};
  bi_t bp{"62391207566730956436059735556895094403209083705277492693463432131493"
          "682000515"};
  auto [q, r] = ap.div(bp);
  EXPECT_EQ(q, 3808789772);
  EXPECT_EQ(
      r,
      bi_t("1613724566691726467990941007309394463279649607168892419209105494691"
           "7820895010"));

  /* If bi_dbits == 32, TRUE:
   * q_hat * vpp > bi_base * r_hat + u_norm[j+n-2]) and q_hat == bi_base */
  a = 1208925820177561948258300_bi;
  b = 281474976841724_bi;
  std::tie(q, r) = a.div(b);
  EXPECT_EQ(q, 4294967295_bi);
  EXPECT_EQ(r, 281474976841720_bi);

  // bi_dbits == 32: triggers the "Add back" step of Knuth Algo D
  a = 1188654551471331072704702840834_bi;
  b = 77371252455336267181195265_bi;
  std::tie(q, r) = a.div(b);
  EXPECT_EQ(q, 15362_bi);
  EXPECT_EQ(r, 77371252455336267181179904_bi);

  std::random_device rdev;
  std::mt19937_64 rng(rdev());

  std::uniform_int_distribution<sddigit> udist_sddigit(sddigit_min,
                                                       sddigit_max);
  for (int16_t i = 0; i < INT16_MAX; ++i) {
    sddigit a_in = udist_sddigit(rng), b_in = udist_sddigit(rng);
    a = a_in;
    b = b_in;

    if (b == 0) {
      ASSERT_THROW(a.div(b), bi::division_by_zero);
      continue;
    }

    // Most negative value divided by -1 won't be representable in sddigit.
    // We tested this case above.
    if (a == sddigit_min && b == -1) {
      continue;
    }

    auto [quot, rem] = a.div(b);
    ASSERT_EQ(quot, a_in / b_in)
        << "Quo mismatch for a_in: " << bltin_int_to_string(a_in)
        << ", b_in: " << bltin_int_to_string(b_in);
    ASSERT_EQ(rem, a_in % b_in)
        << "Rem mismatch for a_in: " << bltin_int_to_string(a_in)
        << ", b_in: " << bltin_int_to_string(b_in);
  }

#if defined(__SIZEOF_INT128__) && !defined(BI_DIGIT_64_BIT)
  std::uniform_int_distribution<sqdigit> udist_sqdigit(sqdigit_min,
                                                       sqdigit_max);
  for (int16_t i = 0; i < INT16_MAX; ++i) {
    sqdigit a_in = udist_sqdigit(rng), b_in = udist_sqdigit(rng);
    a = a_in;
    b = b_in;

    if (b == 0) {
      ASSERT_THROW(a.div(b), bi::division_by_zero);
      continue;
    }

    if (a == sqdigit_min && b == -1) {
      continue;
    }

    auto [quot, rem] = a.div(b);
    ASSERT_EQ(quot, a_in / b_in)
        << "Quo mismatch for a_in: " << bltin_int_to_string(a_in)
        << ", b_in: " << bltin_int_to_string(b_in);
    ASSERT_EQ(rem, a_in % b_in)
        << "Rem mismatch for a_in: " << bltin_int_to_string(a_in)
        << ", b_in: " << bltin_int_to_string(b_in);
  }
#endif
}

TEST_F(BITest, LeftShiftOperator) {
  EXPECT_EQ(bi_t{} << 1, 0);
  EXPECT_EQ(bi_t{} << bi_dwidth, 0);
  EXPECT_EQ(bi_t{} << 0, 0);

  EXPECT_EQ(bi_t{4} << 2, 16);
  EXPECT_EQ(bi_t{-4} << 2, -16);
  EXPECT_EQ(bi_t{4} << 0, 4);
  EXPECT_EQ(bi_t{-4} << 0, -4);

  // Powers of 2 that fit in two digits
  bi_t one{1};
  for (int i = 0; i < bi_dwidth * 2; ++i) {
    ASSERT_EQ(one << i, static_cast<ddigit>(1) << i);
  }
}

TEST_F(BITest, LeftShiftAssignment) {
  EXPECT_EQ(bi_t{} <<= 1, 0);
  EXPECT_EQ(bi_t{} <<= bi_dwidth, 0);
  EXPECT_EQ(bi_t{} <<= 0, 0);

  EXPECT_EQ(bi_t{4} <<= 2, 16);
  EXPECT_EQ(bi_t{-4} <<= 2, -16);
  EXPECT_EQ(bi_t{4} <<= 0, 4);
  EXPECT_EQ(bi_t{-4} <<= 0, -4);

  // Powers of 2 that fit in two digits
  bi_t one{1};
  for (int i = 0; i < bi_dwidth * 2; ++i) {
    if (i == 0) {
      ASSERT_EQ(one <<= i, static_cast<ddigit>(1) << i);
      continue;
    }
    ASSERT_EQ(one <<= 1, static_cast<ddigit>(1) << i);
  }
}

TEST_F(BITest, RightShiftOperator) {
  EXPECT_EQ(bi_t{} >> 1, 0);

  bi_t a{static_cast<ddigit>(bi_dmax) * 2};
  EXPECT_EQ(a >> bi_dwidth, 1);

  a = bi_t{"3619132862646584885328"};
  EXPECT_EQ(a >> 1, bi_t{"1809566431323292442664"});

  bi_t positive(16);
  EXPECT_EQ(positive >> 3, 2);
  EXPECT_EQ(positive >> 0, 16);
  EXPECT_EQ(positive >> (bi_dwidth * 2), 0);

  bi_t negative(-16);
  EXPECT_EQ(negative >> 2, -4);
  EXPECT_EQ(negative >> 0, -16);
  EXPECT_EQ(negative >> (bi_dwidth * 2), -1);

  negative = -1;
  EXPECT_EQ(negative >> 0, -1);
  EXPECT_EQ(negative >> 1, -1);
  EXPECT_EQ(negative >> (bi_dwidth + 1), -1);

  std::random_device rdev;
  std::mt19937_64 rng(rdev());

  std::uniform_int_distribution<sddigit> udist(sddigit_min, sddigit_max);
  for (int16_t i = INT16_MIN; i < INT16_MAX; ++i) {
    sddigit r = udist(rng);
    for (int shift = 0; shift <= 2 * bi_dwidth - 1; ++shift) {
      ASSERT_EQ(bi_t{r} >> shift, r >> shift)
          << "Shift = " << shift << ", r = " << bltin_int_to_string(r)
          << std::endl;
    }
  }
}

TEST_F(BITest, RightShiftAssignment) {
  EXPECT_EQ(bi_t{} >>= 1, 0);

  bi_t a{static_cast<ddigit>(bi_dmax) * 2};
  EXPECT_EQ(a >>= bi_dwidth, 1);

  a = bi_t{"3619132862646584885328"};
  EXPECT_EQ(a >>= 1, bi_t{"1809566431323292442664"});
  EXPECT_EQ(a >>= 21, bi_t{"862868514691969"});
  EXPECT_EQ(a.bit_length(), 50);
  EXPECT_EQ(a >>= 50, 0);

  bi_t b(16);
  b >>= 3;
  EXPECT_EQ(b, 2);

  bi_t c(4);
  c >>= 4;
  EXPECT_EQ(c, 0);
}

TEST_F(BITest, AdditionAssignment) {
  bi_t a(10);
  bi_t b(-5);
  bi_t o1(99090), o2(9909032932);
  a += b;
  EXPECT_EQ(a, 5);
  EXPECT_EQ(b, -5);

  o1 += o2;
  EXPECT_EQ(o1, 9909132022);

  bi_t c(7);
  c += c;
  EXPECT_EQ(c, 14);
}

TEST_F(BITest, SubtractionAssignment) {
  bi_t a(10);
  bi_t b(-5);
  a -= b;
  EXPECT_EQ(a, 15);
  EXPECT_EQ(b, -5);

  bi_t c(7);
  c -= c;
  EXPECT_EQ(c, 0);
}

TEST_F(BITest, MultiplicationAssignment) {
  bi_t a(10);
  bi_t b(-5);
  a *= b;
  EXPECT_EQ(a, -50);
  EXPECT_EQ(b, -5);

  a = ddigit_max;
  a *= sddigit_min;
  if constexpr (bi_dwidth == 64) {
    EXPECT_EQ(a.to_string(),
              "-578960446186580977117854925043439539264648511493598127879971047"
              "00240680714240");
  } else if (bi_dwidth == 32) {
    EXPECT_EQ(a.to_string(), "-170141183460469231722463931679029329920");
  }

  // Self-multiplication
  bi_t c(7);
  c *= c;
  EXPECT_EQ(c, 49);

  a = ddigit_max;
  a *= a;
  if constexpr (bi_dwidth == 64) {
    EXPECT_EQ(a.to_string(),
              "1157920892373161954235709850086879078525894199317986871125308347"
              "93049593217025");
  } else if (bi_dwidth == 32) {
    EXPECT_EQ(a.to_string(), "340282366920938463426481119284349108225");
  }

  b = sddigit_min;
  b *= b;
  if constexpr (bi_dwidth == 64) {
    EXPECT_EQ(b.to_string(),
              "2894802230932904885589274625217197696331749616641014100986439600"
              "1978282409984");
  } else if (bi_dwidth == 32) {
    EXPECT_EQ(b.to_string(), "85070591730234615865843651857942052864");
  }
}

TEST_F(BITest, BitLength) {
  bi_t positive(16);
  bi_t negative(-16);
  bi_t zero;
  bi_t one{1};

  EXPECT_EQ(positive.bit_length(), 5);
  EXPECT_EQ(negative.bit_length(), 5);
  EXPECT_EQ(zero.bit_length(), 0);
  EXPECT_EQ(one.bit_length(), 1);

  positive = UINT64_MAX;
  negative = INT64_MIN;
  EXPECT_EQ(positive.bit_length(), 64);
  EXPECT_EQ(negative.bit_length(), 64);
}

TEST_F(BITest, TestBit) {
  // Zero
  EXPECT_FALSE(bi_t{}.test_bit(0));
  EXPECT_FALSE(bi_t{}.test_bit(42040));

  // '1010'
  bi_t x(10);
  EXPECT_FALSE(x.test_bit(0));
  EXPECT_TRUE(x.test_bit(1));
  EXPECT_FALSE(x.test_bit(2));
  EXPECT_TRUE(x.test_bit(3));

  bi_t one{1};
  EXPECT_TRUE(one.test_bit(0));
  EXPECT_FALSE(one.test_bit(1));
  one <<= 1;
  for (int i = 1; i < 65000; ++i) {
    ASSERT_FALSE(one.test_bit(i - 1));
    ASSERT_TRUE(one.test_bit(i));
    ASSERT_FALSE(one.test_bit(i + 1));
    one <<= 1;
  }
}

TEST_F(BITest, SetBit) {
  EXPECT_EQ(bi_t{}.set_bit(0), 1);
  EXPECT_EQ(bi_t{10}.set_bit(2), 14);

  for (int i = 0; i < bi_dwidth; ++i) {
    ASSERT_EQ(bi_t{}.set_bit(i), static_cast<digit>(std::pow(2.0, i)));
  }
}

void test_to_string_base(int b) {
  ASSERT_EQ(bi_t{}.to_string(b), to_string_base(0, b));
  ASSERT_EQ(bi_t{digit_max}.to_string(b), to_string_base(digit_max, b));
  ASSERT_EQ(bi_t{ddigit_max}.to_string(b), to_string_base(ddigit_max, b));
  ASSERT_EQ(bi_t{sdigit_min}.to_string(b), to_string_base(sdigit_min, b));
  ASSERT_EQ(bi_t{sdigit_max}.to_string(b), to_string_base(sdigit_max, b));
  ASSERT_EQ(bi_t{sddigit_min}.to_string(b), to_string_base(sddigit_min, b));
  ASSERT_EQ(bi_t{sddigit_max}.to_string(b), to_string_base(sddigit_max, b));

  ASSERT_EQ(bi_t{-static_cast<sddigit>(digit_max)}.to_string(b),
            to_string_base(-static_cast<sddigit>(digit_max), b));
  ASSERT_EQ(bi_t{static_cast<ddigit>(digit_max) + 1}.to_string(b),
            to_string_base(static_cast<ddigit>(digit_max) + 1, b));

#if defined(__SIZEOF_INT128__) && defined(BI_DIGIT_32_BIT)
  ASSERT_EQ(bi_t(qdigit_max).to_string(b), to_string_base(qdigit_max, b));
  ASSERT_EQ(bi_t(sqdigit_min).to_string(b), to_string_base(sqdigit_min, b));
  ASSERT_EQ(bi_t(sqdigit_max).to_string(b), to_string_base(sqdigit_max, b));
#endif

  std::random_device rdev;
  std::mt19937_64 rng(rdev());
  std::uniform_int_distribution<sddigit> udist(sddigit_min, sddigit_max);
  std::uniform_int_distribution<digit> udist_digit(digit_min, digit_max);

  const auto mn = INT16_MIN / 8;
  const auto mx = INT16_MAX / 8;
  for (int16_t i = mn; i < mx; ++i) {
    sddigit r = udist(rng);
    digit r_digit = udist_digit(rng);

    ASSERT_EQ(bi_t{r}.to_string(b), to_string_base(r, b));
    ASSERT_EQ(bi_t{r_digit}.to_string(b), to_string_base(r_digit, b));
    ASSERT_EQ(bi_t{i}.to_string(b), to_string_base(i, b));
  }
}

TEST_F(BITest, ToString) {
  EXPECT_THROW(bi_t("0", -1), std::invalid_argument);
  EXPECT_THROW(bi_t("0", 0), std::invalid_argument);
  EXPECT_THROW(bi_t("0", 1), std::invalid_argument);
  EXPECT_THROW(bi_t("0", 37), std::invalid_argument);

  for (int i = 2; i <= 36; ++i) {
    test_to_string_base(i);
  }
}

TEST_F(BITest, SwapMethod) {
  bi_t x(123);
  bi_t y(-456);
  bi_t original_x = x;
  bi_t original_y = y;

  x.swap(y);

  EXPECT_EQ(x, original_y);
  EXPECT_EQ(y, original_x);
}

TEST_F(BITest, NonMemberSwapFunction) {
  bi_t x(123);
  bi_t y(-456);
  bi_t original_x = x;
  bi_t original_y = y;

  swap(x, y);

  EXPECT_EQ(x, original_y);
  EXPECT_EQ(y, original_x);
}

TEST_F(BITest, OutputStreamOperator) {
  bi_t x(123);
  bi_t y(-456);
  bi_t zero(0);
  std::ostringstream oss;

  oss << x << " " << y << " " << zero;
  EXPECT_EQ(oss.str(), "123 -456 0");
}

class CompareWithIntegral : public testing::Test {
 protected:
  // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes)
  bi_t zero;
  bi_t positive = 123456789_bi;
  bi_t negative = -987654321_bi;
  bi_t tdigit = static_cast<ddigit>(bi_dmax) + 242092;
  bi_t tdigit_n = -(static_cast<sddigit>(bi_dmax) + 242092);
  // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes)

  void SetUp() override {}
  void TearDown() override {}
};

// int bi_t::cmp(const bi_t& a, T b) const noexcept
TEST_F(CompareWithIntegral, TestInternalCMP) {
  // a
  EXPECT_EQ(zero, 0);

  // b
  EXPECT_GT(positive, 0);
  EXPECT_LT(negative, 0);

  // c
  EXPECT_LT(bi_t(-500), 1409209);

  // d
  EXPECT_GT(zero, -1409209);
  EXPECT_GT(positive, -1409209);

  // e
  EXPECT_LT(bi_t(), 32902);
  EXPECT_TRUE(bi_t(42920) < static_cast<ddigit>(bi_dmax) + 2920);
  EXPECT_TRUE(bi_t(-42920) > -(static_cast<sddigit>(bi_dmax) + 2920));

  // f
  EXPECT_GT(tdigit, 3293);
  EXPECT_LT(tdigit_n, -42092);

  // g
  EXPECT_TRUE(tdigit < static_cast<ddigit>(bi_dmax) + 342093);
  EXPECT_TRUE(tdigit_n > -(static_cast<sddigit>(bi_dmax) + 342093));

  // h
  EXPECT_TRUE(tdigit > static_cast<ddigit>(bi_dmax) + 2920);
  EXPECT_TRUE(tdigit_n < -(static_cast<sddigit>(bi_dmax) + 2920));

  // i
  EXPECT_TRUE(bi_t(bi_dmax) == bi_dmax);
  EXPECT_TRUE(-bi_t(bi_dmax) == -static_cast<sddigit>(bi_dmax));
  EXPECT_TRUE(bi_t(sddigit_min) == sddigit_min);
  EXPECT_TRUE(bi_t(ddigit_max) == ddigit_max);
}

TEST_F(CompareWithIntegral, TestInternalCMPReversed) {
  // a
  EXPECT_EQ(0, zero);

  // b
  EXPECT_LT(0, positive);
  EXPECT_GT(0, negative);

  // c
  EXPECT_GT(1409209, bi_t(-500));

  // d
  EXPECT_LT(-1409209, zero);
  EXPECT_LT(-1409209, positive);

  // e
  EXPECT_GT(32902, bi_t());
  EXPECT_TRUE(static_cast<ddigit>(bi_dmax) + 2920 > bi_t(42920));
  EXPECT_TRUE(-(static_cast<sddigit>(bi_dmax) + 2920) < bi_t(-42920));

  // f
  EXPECT_LT(3293, tdigit);
  EXPECT_GT(-42092, tdigit_n);

  // g
  EXPECT_TRUE(static_cast<ddigit>(bi_dmax) + 342093 > tdigit);
  EXPECT_TRUE(-(static_cast<sddigit>(bi_dmax) + 342093) < tdigit_n);

  // h
  EXPECT_TRUE(static_cast<ddigit>(bi_dmax) + 2920 < tdigit);
  EXPECT_TRUE(-(static_cast<sddigit>(bi_dmax) + 2920) > tdigit_n);

  // i
  EXPECT_TRUE(bi_dmax == bi_t(bi_dmax));
  EXPECT_TRUE(-static_cast<sddigit>(bi_dmax) == -bi_t(bi_dmax));
  EXPECT_TRUE(sddigit_min == bi_t(sddigit_min));
  EXPECT_TRUE(ddigit_max == bi_t(ddigit_max));
}

class IncrementDecrementTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}

  template <bool Increment>
  void test_random_values_boundary() {
    std::random_device rdev;
    std::mt19937 rng(rdev());
    std::uniform_int_distribution<ddigit> udist_pos(bi_dmax - 100,
                                                    bi_dmax + 100);
    std::uniform_int_distribution<sdigit> udist_zero(-100, 100);
    std::uniform_int_distribution<sddigit> udist_neg(
        -static_cast<sddigit>(bi_dmax) - 100,
        -static_cast<sddigit>(bi_dmax) + 100);
    for (int16_t i = 0; i < INT16_MAX; ++i) {
      ddigit rv_pos = udist_pos(rng);
      bi_t r = rv_pos;
      if constexpr (Increment) {
        ASSERT_EQ(++r, ++rv_pos);
      } else {
        ASSERT_EQ(--r, --rv_pos);
      }

      sdigit rv_zero = udist_zero(rng);
      r = rv_zero;
      if constexpr (Increment) {
        ASSERT_EQ(++r, ++rv_zero);
      } else {
        ASSERT_EQ(--r, --rv_zero);
      }

      sddigit rv_neg = udist_neg(rng);
      r = rv_neg;
      if constexpr (Increment) {
        ASSERT_EQ(++r, ++rv_neg);
      } else {
        ASSERT_EQ(--r, --rv_neg);
      }
    }
  }
};

TEST_F(IncrementDecrementTest, PrefixIncrement) {
  bi_t x{10};
  bi_t& ref{++x};
  EXPECT_EQ(x, 11);
  EXPECT_EQ(&ref, &x);

  bi_t y;
  bi_t& ref_y{++y};
  EXPECT_EQ(y, 1);
  EXPECT_EQ(&ref_y, &y);

  bi_t z{-10};
  bi_t& ref_z{++z};
  EXPECT_EQ(z, -9);
  EXPECT_EQ(&ref_z, &z);

  bi_t a{bi_dmax};
  bi_t& ref_a{++a};
  EXPECT_EQ(a, static_cast<ddigit>(bi_dmax) + 1);
  EXPECT_EQ(&ref_a, &a);

  bi_t b{-static_cast<sddigit>(bi_dmax) - 1};
  bi_t& ref_b{++b};
  EXPECT_EQ(b, -static_cast<sddigit>(bi_dmax));
  EXPECT_EQ(&ref_b, &b);

  // Bit representation: 111...1
  a = ddigit_max;
  ++a;
  if constexpr (bi_dwidth == 64) {
    EXPECT_EQ(a.to_string(), "340282366920938463463374607431768211456");
  } else {
    EXPECT_EQ(a.to_string(), "18446744073709551616");
  }
  --a;
  EXPECT_EQ(a, ddigit_max);

  test_random_values_boundary<true>();
}

TEST_F(IncrementDecrementTest, PrefixDecrement) {
  bi_t x{10};
  bi_t& ref{--x};
  EXPECT_EQ(x, 9);
  EXPECT_EQ(&ref, &x);

  bi_t y;
  bi_t& ref_y{--y};
  EXPECT_EQ(y, -1);
  EXPECT_EQ(&ref_y, &y);

  bi_t z{-10};
  bi_t& ref_z{--z};
  EXPECT_EQ(z, -11);
  EXPECT_EQ(&ref_z, &z);

  bi_t a{static_cast<ddigit>(bi_dmax) + 1};
  bi_t& ref_a{--a};
  EXPECT_EQ(a, bi_dmax);
  EXPECT_EQ(&ref_a, &a);

  bi_t b{-static_cast<sddigit>(bi_dmax)};
  bi_t& ref_b{--b};
  EXPECT_EQ(b, -static_cast<sddigit>(bi_dmax) - 1);
  EXPECT_EQ(&ref_b, &b);

  a = ddigit_max;
  --a;
  EXPECT_EQ(a, ddigit_max - 1);
  ++a;
  EXPECT_EQ(a, ddigit_max);
  a.negate();
  --a;
  if constexpr (bi_dwidth == 64) {
    EXPECT_EQ(a.to_string(), "-340282366920938463463374607431768211456");
  } else {
    EXPECT_EQ(a.to_string(), "-18446744073709551616");
  }

  test_random_values_boundary<false>();
}

// The logic of postfix ops are contained within the prefix ops
TEST_F(IncrementDecrementTest, PostfixIncrement) {
  bi_t x{10};
  bi_t old_value{x++};
  EXPECT_EQ(old_value, 10);
  EXPECT_EQ(x, 11);

  bi_t y;
  bi_t old_y{y++};
  EXPECT_EQ(old_y, 0);
  EXPECT_EQ(y, 1);

  bi_t z{-10};
  bi_t old_z{z++};
  EXPECT_EQ(old_z, -10);
  EXPECT_EQ(z, -9);
}

TEST_F(IncrementDecrementTest, PostfixDecrement) {
  bi_t x{10};
  bi_t old_value{x--};
  EXPECT_EQ(old_value, 10);
  EXPECT_EQ(x, 9);

  bi_t y;
  bi_t old_y{y--};
  EXPECT_EQ(old_y, 0);
  EXPECT_EQ(y, -1);

  bi_t z{-10};
  bi_t old_z{z--};
  EXPECT_EQ(old_z, -10);
  EXPECT_EQ(z, -11);
}

TEST_F(BITest, Abs) {
  bi_t result;

  bi_t pos{123};
  result = bi::abs(pos);
  EXPECT_EQ(result, pos);

  bi_t neg{-123};
  result = bi::abs(neg);
  EXPECT_EQ(result, bi_t{123});

  bi_t zero;
  result = bi::abs(zero);
  EXPECT_EQ(result, zero);
}

TEST_F(BITest, OperatorBool) {
  bi_t zero;
  EXPECT_FALSE(zero);

  bi_t pos{9};
  EXPECT_TRUE(pos);

  bi_t neg{-9};
  EXPECT_TRUE(neg);
}

TEST_F(BITest, OperatorUnsignedIntegral) {
  bi_t x = 0;
  EXPECT_EQ(static_cast<unsigned int>(x), 0);

  x = 123;
  EXPECT_EQ(static_cast<unsigned int>(x), 123);

  x = -123;
  EXPECT_EQ(static_cast<unsigned int>(x), static_cast<unsigned int>(-123));

  x = ddigit_max;
  EXPECT_EQ(static_cast<ddigit>(x), ddigit_max);

  x = sddigit_min;
  EXPECT_EQ(static_cast<ddigit>(x), static_cast<ddigit>(sddigit_min));
  EXPECT_EQ(static_cast<digit>(x), static_cast<digit>(sddigit_min));
}

TEST_F(BITest, OperatorSignedIntegral) {
  bi_t x = 0;
  EXPECT_EQ(static_cast<int>(x), 0);

  x = 123;
  EXPECT_EQ(static_cast<int>(x), 123);

  x = -123;
  EXPECT_EQ(static_cast<int>(x), -123);

  x = ddigit_max;
  EXPECT_EQ(static_cast<sddigit>(x), static_cast<sddigit>(ddigit_max));
  EXPECT_EQ(static_cast<sdigit>(x), static_cast<sdigit>(ddigit_max));

  x = sddigit_min;
  EXPECT_EQ(static_cast<sddigit>(x), static_cast<sddigit>(sddigit_min));
  EXPECT_EQ(static_cast<sdigit>(x), static_cast<sdigit>(sddigit_min));

  x = sddigit_max;
  EXPECT_EQ(static_cast<sddigit>(x), static_cast<sddigit>(sddigit_max));
  EXPECT_EQ(static_cast<sdigit>(x), static_cast<sdigit>(sddigit_max));

  x = std::numeric_limits<long>::min();
  EXPECT_EQ(static_cast<long>(x), std::numeric_limits<long>::min());
}

TEST_F(BITest, EvenOrOdd) {
  bi_t zero;
  EXPECT_TRUE(zero.even());
  EXPECT_FALSE(zero.odd());

  bi_t a{bi_dmax};
  EXPECT_TRUE(a.odd());
  EXPECT_FALSE(a.even());

  a = static_cast<ddigit>(bi_dmax) + 1;
  EXPECT_TRUE(a.even());
  EXPECT_FALSE(a.odd());

  a = -static_cast<sddigit>(bi_dmax);
  EXPECT_TRUE(a.odd());
  EXPECT_FALSE(a.even());

  --a;
  EXPECT_TRUE(a.even());
  EXPECT_FALSE(a.odd());
}

class BitwiseOperatorsTest : public ::testing::Test {
 protected:
  bi_t zero;
  bi_t pos{12345};
  bi_t neg{-6789};

  template <typename Operation>
  void TestBitwiseOperation(Operation op) {
    std::random_device rdev;
    std::mt19937_64 rng(rdev());
    std::uniform_int_distribution<sddigit> udist(sddigit_min, sddigit_max);
    std::uniform_int_distribution<sddigit> udist_small(-25, 25);
    std::uniform_int_distribution<sddigit> udist_digit(
        bi_dmax - 25, static_cast<sddigit>(bi_dmax) + 25);

#if defined(__SIZEOF_INT128__) && defined(BI_DIGIT_64_BIT)
    using bi_int128::operator""_i128;
    sddigit lft = 116996355089234619366281923321514370394_i128;
    bi_t lft_bi{lft};
    EXPECT_EQ(op(lft_bi, bi_t{-11}), op(lft, -11));
#endif

    auto check_op = [&op](sddigit& lhs, const sddigit& rhs) -> bool {
      bi_t lhs_bi{lhs};
      bool result = op(lhs_bi, bi_t{rhs}) == op(lhs, rhs);
      if (!result) {
        std::cerr << "Failure with lhs: " << bltin_int_to_string(lhs)
                  << ", rhs: " << bltin_int_to_string(rhs) << std::endl;
        std::cerr << "In " << op(lhs_bi, bi_t{rhs})
                  << " == " << bltin_int_to_string(op(lhs, rhs)) << std::endl;
      }
      return result;
    };

    for (int16_t i = 0; i < INT16_MAX; ++i) {
      sddigit ra = udist(rng), rb = udist(rng);
      sddigit ra_small = udist_small(rng), rb_small = udist_small(rng);
      sddigit ra_digit = udist_digit(rng), rb_digit = udist_digit(rng);

      ASSERT_TRUE(check_op(ra, rb));
      ASSERT_TRUE(check_op(ra_small, rb_small));
      ASSERT_TRUE(check_op(ra_digit, rb_digit));

      ASSERT_TRUE(check_op(ra, ra_small));
      ASSERT_TRUE(check_op(ra, ra_digit));
      ASSERT_TRUE(check_op(ra_small, ra_digit));
    }
  }
};

TEST_F(BitwiseOperatorsTest, BitwiseAND) {
  EXPECT_EQ(pos & pos, 12345);
  EXPECT_EQ(neg & neg, -6789);
  EXPECT_EQ(zero & pos, 0);
  EXPECT_EQ(zero & neg, 0);
  EXPECT_EQ(pos & neg, 8249);

  TestBitwiseOperation(std::bit_and<>());
}

TEST_F(BitwiseOperatorsTest, BitwiseOR) {
  EXPECT_EQ(pos | pos, 12345);
  EXPECT_EQ(neg | neg, -6789);
  EXPECT_EQ(zero | pos, 12345);
  EXPECT_EQ(zero | neg, -6789);
  EXPECT_EQ(pos | neg, -2693);

  TestBitwiseOperation(std::bit_or<>());
}

TEST_F(BitwiseOperatorsTest, BitwiseXOR) {
  EXPECT_EQ(pos ^ pos, 0);
  EXPECT_EQ(neg ^ neg, 0);
  EXPECT_EQ(zero ^ pos, 12345);
  EXPECT_EQ(zero ^ neg, -6789);
  EXPECT_EQ(pos ^ neg, -10942);

  TestBitwiseOperation(std::bit_xor<>());
}

TEST_F(BitwiseOperatorsTest, BitwiseANDAssignment) {
  bi_t res;

  res = pos;
  EXPECT_EQ(res &= pos, 12345);

  res = neg;
  EXPECT_EQ(res &= neg, -6789);

  res = pos;
  EXPECT_EQ(res &= neg, 8249);

  res = zero;
  EXPECT_EQ(res &= pos, 0);

  res = zero;
  EXPECT_EQ(res &= neg, 0);

  auto and_assignment = [](auto& lhs, const auto& rhs) -> auto& {
    return lhs &= rhs;
  };
  TestBitwiseOperation(and_assignment);
}

TEST_F(BitwiseOperatorsTest, BitwiseORAssignment) {
  bi_t res;

  res = pos;
  EXPECT_EQ(res |= pos, 12345);

  res = neg;
  EXPECT_EQ(res |= neg, -6789);

  res = zero;
  EXPECT_EQ(res |= pos, 12345);

  res = zero;
  EXPECT_EQ(res |= neg, -6789);

  res = pos;
  EXPECT_EQ(res |= neg, -2693);

  auto or_assignment = [](auto& lhs, const auto& rhs) -> auto& {
    return lhs |= rhs;
  };
  TestBitwiseOperation(or_assignment);
}

TEST_F(BitwiseOperatorsTest, BitwiseXORAssignment) {
  bi_t res;

  res = pos;
  EXPECT_EQ(res ^= pos, 0);

  res = neg;
  EXPECT_EQ(res ^= neg, 0);

  res = zero;
  EXPECT_EQ(res ^= pos, 12345);

  res = zero;
  EXPECT_EQ(res ^= neg, -6789);

  res = pos;
  EXPECT_EQ(res ^= neg, -10942);

  auto xor_assignment = [](auto& lhs, const auto& rhs) -> auto& {
    return lhs ^= rhs;
  };
  TestBitwiseOperation(xor_assignment);
}

TEST_F(BitwiseOperatorsTest, UnaryComplement) {
  bi_t a;
  EXPECT_EQ(~a, -1);

  a = 1;
  EXPECT_EQ(~a, -2);

  a = bi_dmax;
  EXPECT_EQ(~a, -static_cast<sddigit>(bi_dmax) - 1);
  EXPECT_EQ(~a, ~static_cast<sddigit>(bi_dmax));

  std::random_device rdev;
  std::mt19937_64 rng(rdev());
  std::uniform_int_distribution<sddigit> udist(sddigit_min, sddigit_max);
  std::uniform_int_distribution<sddigit> udist_mdigit(bi_dmax / 4, bi_dmax / 2);
  std::uniform_int_distribution<sddigit> udist_msddigit(sddigit_max / 4,
                                                        sddigit_max / 2);
  std::uniform_int_distribution<sddigit> udist_msddigit_neg(sddigit_min / 2,
                                                            sddigit_min / 4);
  for (int i = INT16_MIN; i < INT16_MAX; ++i) {
    std::vector<sddigit> vec = {udist(rng),
                                static_cast<sddigit>(bi_dmax) + i,
                                udist_mdigit(rng),
                                udist_msddigit(rng),
                                udist_msddigit_neg(rng),
                                i};
    for (const auto& r : vec) {
      ASSERT_EQ(~bi_t{r}, ~r);
    }
  }
}

TEST_F(BITest, WithinIntegral) {
  bi_t x;

  EXPECT_TRUE(x.within<digit>());

  x = digit_max;
  EXPECT_TRUE(x.within<digit>());
  EXPECT_FALSE(x.within<sdigit>());

  x += 1;
  EXPECT_FALSE(x.within<digit>());

  x = -static_cast<sddigit>(digit_max);
  EXPECT_FALSE(x.within<digit>());
  EXPECT_TRUE(x.within<sddigit>());
  x -= 1;
  EXPECT_TRUE(x.within<sddigit>());

  x = ddigit_max;
  EXPECT_TRUE(x.within<ddigit>());
  EXPECT_FALSE(x.within<digit>());

  // From documentation
  x = std::numeric_limits<int32_t>::max();
  bool fits_in_int32 = x.within<int32_t>();  // true
  bool fits_in_int16 = x.within<int16_t>();  // false
  EXPECT_TRUE(fits_in_int32);
  EXPECT_FALSE(fits_in_int16);
}

TEST_F(BITest, AssignString) {
  bi_t x;

  x = "0"s;
  EXPECT_EQ(x, 0);

  x = "3239"s;
  EXPECT_EQ(x, 3239);

  x = "-3239"s;
  EXPECT_EQ(x, -3239);

  EXPECT_THROW(x = "", std::invalid_argument);
  EXPECT_THROW(x = nullptr, std::invalid_argument);
  EXPECT_THROW(x = "  -", std::invalid_argument);

  auto str = "999909090093232329302932309230930923230992094029424204"s;
  x = str;
  EXPECT_EQ(x.to_string(), str);

  auto strn = "-9999090900932323293029323092309309232309920940294242"s;
  x = strn;
  EXPECT_EQ(x.to_string(), strn);

  std::random_device rdev;
  std::mt19937 rng(rdev());

  EXPECT_EQ(x = bltin_int_to_string(sddigit_min), sddigit_min);
  EXPECT_EQ(x = bltin_int_to_string(ddigit_max), ddigit_max);

  std::uniform_int_distribution<sddigit> udist(sddigit_min, sddigit_max);
  std::uniform_int_distribution<int16_t> udist_16(
      std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
  for (int16_t i = 0; i < INT16_MAX; ++i) {
    sddigit rv = udist(rng);
    std::string s = bltin_int_to_string(rv);
    ASSERT_EQ(x = s, rv);

    int16_t rv_16 = udist_16(rng);
    s = bltin_int_to_string(rv_16);
    ASSERT_EQ(x = s, rv_16);
  }
}

TEST_F(BITest, ConstructAndAssignFromDouble) {
  bi_t x;

  auto test_doub = [&x]<typename T>(const double& value, const T& exp) {
    ASSERT_EQ(bi_t{value}, exp) << "Failure in constructor test: bi_t{" << value
                                << "} vs. expected: " << exp;
    ASSERT_EQ(x = value, exp) << "Failure in assignment test: x = " << value
                              << " vs. expected: " << exp;
  };  // NOLINT(readability/braces)

  /* Special values */
  double min_double = std::numeric_limits<double>::min();
  double max_double = std::numeric_limits<double>::max();
  double lowest_double = std::numeric_limits<double>::lowest();
  double zero = 0.0;
  double mzero = -0.0;
  double nan = std::numeric_limits<double>::quiet_NaN();
  double snan = std::numeric_limits<double>::signaling_NaN();
  double inf = std::numeric_limits<double>::infinity();
  double minf = -std::numeric_limits<double>::infinity();
  double subnormal_double = std::numeric_limits<double>::denorm_min();
  double max_int = 9007199254740992.0;       // 2^{53}
  double max_int_neg = -9007199254740992.0;  // -2^{53}
  test_doub(min_double, 0);
  bi_t bi_max_double = max_double;
  test_doub(max_double,
            bi_t{"1797693134862315708145274237317043567980705675258449965989174"
                 "7680315726078002853876058955863276687817154045895351438246423"
                 "4321326889464182768467546703537516986049910576551282076245490"
                 "0903893289440758685084551339423045832369032229481658085593321"
                 "2334827479782620414472316873817718091929988125040402618412485"
                 "8368"});
  bi_max_double.negate();
  test_doub(lowest_double, bi_max_double);
  test_doub(zero, 0);
  test_doub(mzero, 0);
  EXPECT_THROW(bi_t{nan}, bi::from_float);
  EXPECT_THROW(x = nan, bi::from_float);
  EXPECT_THROW(bi_t{snan}, bi::from_float);
  EXPECT_THROW(x = snan, bi::from_float);
  EXPECT_THROW(bi_t{inf}, bi::from_float);
  EXPECT_THROW(x = inf, bi::from_float);
  EXPECT_THROW(bi_t{minf}, bi::from_float);
  EXPECT_THROW(x = minf, bi::from_float);
  test_doub(subnormal_double, 0);
  test_doub(max_int, bi_t{"9007199254740992"});
  test_doub(max_int_neg, bi_t{"-9007199254740992"});

  /* Misc. */
  test_doub(9876.54321, 9876);
  test_doub(-9876.54321, -9876);
  test_doub(0.987654321, 0);
  test_doub(0.999999999, 0);
  test_doub(-0.999999999, 0);
  test_doub(1e-109, 0);
  test_doub(1e109,
            bi_t{"9999999999999999818508707188399807864717650964328171247958398"
                 "369899072554380053298205803424393137676263358464"});

  /* Random */
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_real_distribution<double> dis_0(max_int_neg, max_int);
  std::uniform_real_distribution<double> dis_1(-100.0, 100.0);

  for (int16_t i = 0; i < INT16_MAX; ++i) {
    double random_double = dis_0(gen);
    int64_t int_val = static_cast<int64_t>(random_double);
    test_doub(random_double, int_val);

    random_double = dis_1(gen);
    int_val = static_cast<int64_t>(random_double);
    test_doub(random_double, int_val);
  }
}

TEST_F(BITest, ConvertToDouble) {
  /* Special values */
  double zero = 0.0;
  double max_int = 9007199254740992.0;       // 2^{53}
  double max_int_neg = -9007199254740992.0;  // -2^{53}
  double min_double = std::numeric_limits<double>::min();
  double max_double = std::numeric_limits<double>::max();
  double lowest_double = std::numeric_limits<double>::lowest();
  double subnormal_double = std::numeric_limits<double>::denorm_min();
  double inf = std::numeric_limits<double>::infinity();
  double minf = -std::numeric_limits<double>::infinity();

  auto test_db = [](const bi_t& bi_value, auto expected) {
    EXPECT_EQ(static_cast<double>(bi_value), static_cast<double>(expected));
  };

  test_db(bi_t{zero}, zero);
  test_db(bi_t{max_int}, max_int);
  test_db(bi_t{max_int_neg}, max_int_neg);
  test_db(bi_t{min_double}, 0.0);
  test_db(bi_t{lowest_double}, lowest_double);
  test_db(bi_t{subnormal_double}, 0.0);

  bi_t bi_max_double = max_double;
  test_db(bi_max_double, max_double);
  test_db(bi_max_double + 1, max_double + 1.0);
  test_db(bi_max_double * 987654321, max_double * 987654321);
  test_db(bi_max_double * bi_max_double, max_double * max_double);
  test_db(bi_max_double * bi_max_double, inf);
  test_db(bi_max_double * -bi_max_double, minf);

  test_db(bi_t{digit_max}, digit_max);
  test_db(bi_t{ddigit_max}, ddigit_max);
#if defined(__SIZEOF_INT128__) && !defined(BI_DIGIT_64_BIT)
  test_db(bi_t{qdigit_max}, qdigit_max);
  test_db(bi_t{sqdigit_min}, sqdigit_min);
#endif

  // Misc.
  test_db(bi_t{-1.0}, -1.0);
  test_db(bi_t{1.0}, 1.0);
  test_db(bi_t{-987654321.0}, -987654321.0);
  test_db(bi_t{987654321.0}, 987654321.0);

  /* Random */
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_real_distribution<double> dis_0(max_int_neg, max_int);
  std::uniform_real_distribution<double> dis_1(-100.0, 100.0);
  std::uniform_int_distribution<uint64_t> dis_2(bi::dbl_max_int, UINT64_MAX);
#if defined(__SIZEOF_INT128__) && !defined(BI_DIGIT_64_BIT)
  std::uniform_int_distribution<sqdigit> dis_3(sqdigit_min, sqdigit_max);
#endif
  std::uniform_int_distribution<sddigit> dis_4(sddigit_min, sddigit_max);

  for (int16_t i = 0; i < INT16_MAX; ++i) {
    double random_double = dis_0(gen);
    int64_t int_val = static_cast<int64_t>(random_double);
    ASSERT_EQ(static_cast<double>(bi_t{random_double}), int_val);

    random_double = dis_1(gen);
    int_val = static_cast<int64_t>(random_double);
    ASSERT_EQ(static_cast<double>(bi_t{random_double}), int_val);

    uint64_t uint_val = dis_2(gen);
    ASSERT_EQ(static_cast<double>(bi_t{uint_val}),
              static_cast<double>(uint_val));

#if defined(__SIZEOF_INT128__) && !defined(BI_DIGIT_64_BIT)
    sqdigit qval = dis_3(gen);
    ASSERT_DOUBLE_EQ(static_cast<double>(bi_t{qval}),
                     static_cast<double>(qval));
#endif

    sddigit sval = dis_4(gen);
    ASSERT_DOUBLE_EQ(static_cast<double>(bi_t{sval}),
                     static_cast<double>(sval));
  }
}

TEST_F(BITest, CompareToDouble) {
  double zero = 0.0;
  double max_int = 9007199254740992.0;       // 2^{53}
  double max_int_neg = -9007199254740992.0;  // -2^{53}
  double min_double = std::numeric_limits<double>::min();
  double max_double = std::numeric_limits<double>::max();
  double lowest_double = std::numeric_limits<double>::lowest();
  double subnormal_double = std::numeric_limits<double>::denorm_min();
  double inf = std::numeric_limits<double>::infinity();
  double minf = -std::numeric_limits<double>::infinity();
  double nan = std::numeric_limits<double>::quiet_NaN();
  double snan = std::numeric_limits<double>::signaling_NaN();

  bi_t bi_max_double{max_double};

  EXPECT_EQ(bi_t{}, zero);
  EXPECT_TRUE(bi_t{} < min_double);
  EXPECT_TRUE(bi_t{} < subnormal_double);
  EXPECT_TRUE(bi_t{} > lowest_double);

  // Infinity should be larger than all ints
  EXPECT_TRUE(bi_max_double < inf);
  EXPECT_TRUE(bi_t{} < inf);

  // -Infinity should be less than all ints
  EXPECT_TRUE(-bi_max_double > minf);
  EXPECT_TRUE(bi_t{} > minf);

  // Boundary around max_double
  EXPECT_EQ(bi_t{max_double}, max_double);
  EXPECT_TRUE(++bi_t{max_double} > max_double);
  EXPECT_TRUE(--bi_t{max_double} < max_double);
  EXPECT_EQ(bi_t{-max_double}, -max_double);
  EXPECT_TRUE(++bi_t{-max_double} > -max_double);
  EXPECT_TRUE(--bi_t{-max_double} < -max_double);

  /* IEEE 754: NaN compared to another floating point number x (where x can be
   * finite, an infinite, or NaN) evaluates to false with >=, <=, >, <, ==,
   * but true when NaN != x is evaluated. */
  bi_t one{1};
  EXPECT_TRUE(one != nan);
  EXPECT_TRUE(one != snan);
  EXPECT_FALSE(one == nan);
  EXPECT_FALSE(one == snan);
  EXPECT_FALSE(one < nan);
  EXPECT_FALSE(one < snan);
  EXPECT_FALSE(one <= nan);
  EXPECT_FALSE(one <= snan);
  EXPECT_FALSE(one > nan);
  EXPECT_FALSE(one > snan);
  EXPECT_FALSE(one >= nan);
  EXPECT_FALSE(one >= snan);

  // Comparison with infinities above triggers the case where bi_base_dbl <= dbl
  // in the implementation. Similarly, this does too:
  EXPECT_TRUE(bi_t{ddigit_max} <= static_cast<double>(ddigit_max));

  EXPECT_TRUE(bi_t{sddigit_min} >= static_cast<double>(sddigit_min));
  EXPECT_TRUE(bi_t{sdigit_min} >= static_cast<double>(sdigit_min));
  EXPECT_TRUE(bi_t{digit_max} <= static_cast<double>(digit_max));

  // *it < cur: one iteration (the max_double ones above cover > 1 iteration)
  EXPECT_TRUE(bi_t{1} << bi_dwidth < std::ldexp(1.0, bi_dwidth + 1));
  EXPECT_TRUE((bi_t{1} << (bi_dwidth * 2)) <
              std::ldexp(1.0, bi_dwidth * 2 + 1));

  // *it > cur: one iteration (the max_double ones above cover > 1 iteration)
  EXPECT_TRUE(bi_t{1} << (bi_dwidth + 2) > std::ldexp(1.0, bi_dwidth + 1));
  EXPECT_TRUE((bi_t{1} << (bi_dwidth * 2 + 2)) >
              std::ldexp(1.0, bi_dwidth * 2 + 1));

  // Large is the first number possible that will trigger the z.size() >= upper
  // branch of the impl.
  constexpr auto upper = bi::bi_cmp_dbl_size_upper;
  constexpr auto shift = bi::bi_dbits * (upper - 1);  // Should be 1024
  bi_t large = bi_t{1} << shift;
  if (bi::bi_dbits == 32) {
    EXPECT_EQ(large.size(), 33);
  } else if (bi::bi_dbits == 64) {
    EXPECT_EQ(large.size(), 17);
  }
  EXPECT_TRUE(large > std::numeric_limits<double>::max());
  EXPECT_TRUE((large + 1) > std::numeric_limits<double>::max());
  // This will not trigger the z.size() >= upper branch
  EXPECT_TRUE((large - 1) > std::numeric_limits<double>::max());

  // Misc.
  EXPECT_TRUE(bi_t{5} > 4.9);
  EXPECT_TRUE(4.9 < bi_t{5});
  EXPECT_TRUE(bi_t{1} > -1.0);
  EXPECT_TRUE(bi_t{-1} < 1.0);

  // Random
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_real_distribution<double> dis(max_int_neg, max_int);
  for (int16_t i = 0; i < INT16_MAX; ++i) {
    const double rand_double = dis(gen);
    const int64_t i64 = static_cast<int64_t>(rand_double);
    const bi_t rand_bi{rand_double};

    EXPECT_TRUE(rand_bi == i64);

    if (rand_double < 0) {
      EXPECT_TRUE(rand_bi >= rand_double);
    } else {
      EXPECT_TRUE(rand_bi <= rand_double);
    }
  }
}

TEST_F(BITest, Exponentiation) {
  bi_t zero;
  bi_t one{1};
  bi_t mone{-1};
  bi_t two{2};
  bi_t dmax = bi_t{ddigit_max};
  bi_t sdmin = bi_t{sddigit_min};

  // Zero base
  EXPECT_EQ(bi_t::pow(zero, 9), 0);
  EXPECT_EQ(bi_t::pow(zero, bi_t{"987654321"}), 0);

  // Zero exponent
  EXPECT_EQ(bi_t::pow(zero, 0), 1);
  EXPECT_EQ(bi_t::pow(dmax, 0), 1);
  EXPECT_EQ(bi_t::pow(sdmin, 0), 1);

  // Negative exponent (only relevant for the bi_t ** bi_t overload)
  EXPECT_THROW(bi_t::pow(zero, bi_t{-1}), std::invalid_argument);
  EXPECT_THROW(bi_t::pow(dmax, bi_t{-9}), std::invalid_argument);
  EXPECT_THROW(bi_t::pow(sdmin, bi_t{"-90932093239"}), std::invalid_argument);

  // Negative base, odd exponent (should keep negative)
  EXPECT_EQ(bi_t::pow(mone, 1), -1);
  EXPECT_EQ(bi_t::pow(mone, 3), -1);
#if defined(BI_DIGIT_32_BIT)
  EXPECT_EQ(bi_t::pow(sdmin, 23),
            bi_t{"-155762789823805819489644210841387463844061115915499499458344"
                 "5145898846278001462648694944985132025705960729951317997322375"
                 "2880850475164269288340008276174542366683242849014366832921220"
                 "1902149561345719251656394893099439238152225978695343860366892"
                 "2855433927712300945575766753370369126991084298521781809546945"
                 "4992653894994728712678545239681773161633112497585462598498727"
                 "9633462629005911338101835943481319191828108807880410326670273"
                 "13956749312"});
#else
  EXPECT_EQ(bi_t::pow(sdmin, 11),
            bi_t{"-345862871284476501831149277335936051622902444958594924036764"
                 "4166080576879632652362184119765613545163153674691520749911733"
                 "4856931716223259006470787726815846167401342301538062679980223"
                 "7019475639957997729415406269691677905502804565730165765306863"
                 "3580937091736686607336729867002420861757015609953617850773500"
                 "6954279854950959266887234288394055518341102523261959774677993"
                 "72494786181018418826967585199690251403572745123158556672"});
#endif

  // Negative base, even exponent (should make positive)
  EXPECT_EQ(bi_t::pow(mone, 2), 1);
  EXPECT_EQ(bi_t::pow(mone, 4), 1);
#if defined(BI_DIGIT_32_BIT)
  EXPECT_EQ(bi_t::pow(sdmin, 28), bi_t{"103971031169538340124421817778820199111"
                                       "80732225837567918598261456490667957"
                                       "015521630261134407217646372387167989961"
                                       "15356897447811088851009681763948496"
                                       "343306415110762436966946115370052740363"
                                       "23602361682111083622989307588713647"
                                       "881955005902365403198712787692241264106"
                                       "28252477896572557128589413489281176"
                                       "975553284484518608538073046398294341654"
                                       "39526506373257315741880485191194993"
                                       "469873589860721480400028774502622633631"
                                       "46478527498944936048377329839250155"
                                       "330892874094061087045864641276073782007"
                                       "83285961953855832464802521215743516"
                                       "11841914863616"});
#else
  EXPECT_EQ(bi_t::pow(sdmin, 12),
            bi_t{"5884551823537677218213175307686313752226790156117404666405456"
                 "6383429135536909257828835083596370753165611542145529492304391"
                 "1722895213612872307034420871771155140432944184967095891304218"
                 "2416214598820423689300263232562493520160899900169583464653563"
                 "2518676704483431208512637438314337930112998334818456244772225"
                 "8390861598931052720472224612197939074726704000940161375418674"
                 "8738956656814407995761242366638628565689079572358151137624542"
                 "29942217871394772181885927817216"});
#endif

  // Test guard branch for bi_t ** bi_bitcount_t overload
  EXPECT_EQ(bi_t::pow(zero, bi::max_bits), 0);
  EXPECT_EQ(bi_t::pow(one, bi::max_bits), 1);
  EXPECT_EQ(bi_t::pow(mone, bi::max_bits), 1);       // even
  EXPECT_EQ(bi_t::pow(mone, bi::max_bits + 1), -1);  // odd
  EXPECT_THROW(bi_t::pow(two, bi::max_bits), bi::overflow_error);
  EXPECT_THROW(bi_t::pow(two, bi::max_bits + 1), bi::overflow_error);

  // Test guard branch for bi_t ** bi_t overload
  bi_t bi_max_bits = bi::max_bits;
  EXPECT_EQ(bi_t::pow(zero, bi_max_bits), 0);
  EXPECT_EQ(bi_t::pow(one, bi_max_bits), 1);
  EXPECT_EQ(bi_t::pow(mone, bi_max_bits), 1);       // even
  EXPECT_EQ(bi_t::pow(mone, bi_max_bits + 1), -1);  // odd
  EXPECT_THROW(bi_t::pow(two, bi_max_bits), bi::overflow_error);
  EXPECT_THROW(bi_t::pow(two, bi_max_bits + 1), bi::overflow_error);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
