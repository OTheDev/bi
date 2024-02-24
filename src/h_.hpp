/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_SRC_H__HPP_
#define BI_SRC_H__HPP_

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <limits>
#include <string>
#include <utility>

#include "bi.hpp"
#include "bi_exceptions.hpp"
#include "constants.hpp"
#include "uints.hpp"

/// @defgroup algorithms Algorithms

namespace bi {

/// @private
struct h_ {
  // increment/decrement
  static void increment_abs(bi_t& x);
  static void decrement_abs(bi_t& x);

  // additive
  static void add_abs(bi_t& result, const bi_t& a, const bi_t& b);
  static void add(bi_t& result, const bi_t& a, const bi_t& b);
  static void sub_abs_gt(bi_t& result, const bi_t& a, const bi_t& b);
  static void sub_abs(bi_t& result, const bi_t& a, const bi_t& b);
  static void sub(bi_t& result, const bi_t& a, const bi_t& b);

  // multiplicative
  static void imul1add1(bi_t& x, digit v, digit k);
  static void mul(bi_t& result, const bi_t& a, const bi_t& b);
  static void div_algo_single(bi_t& q, bi_t& r, const bi_t& n,
                              const bi_t& d) noexcept;
  static void div_algo_binary(bi_t& q, bi_t& r, const bi_t& n, const bi_t& d);
  static void div_algo_knuth(bi_t& q, bi_t& r, const bi_t& n, const bi_t& d);
  static void divide(bi_t& q, bi_t& r, const bi_t& n, const bi_t& d);

  // bits
  static void left_shift(bi_t& result, const bi_t& a, bi_bitcount_t shift);
  static void right_shift(bi_t& result, const bi_t& a, bi_bitcount_t shift);
  enum class BitwiseOperation { AND, OR, XOR };
  template <BitwiseOperation Op>
  static void bitwise_operation_impl(bi_t& res, const bi_t& a, const bi_t& b);
  template <BitwiseOperation Op>
  static bi_t bitwise_operation(const bi_t& a, const bi_t& b);
  template <BitwiseOperation Op>
  static bi_t& ibitwise_operation(bi_t&, const bi_t& other);

  // comparisons
  static int cmp_abs(const bi_t&, const bi_t&) noexcept;
  static int cmp(const bi_t&, const bi_t&) noexcept;
  template <std::integral T>
  static int cmp(const bi_t& a, T b) noexcept;

  // to_string()
  static uint8_t idiv10(bi_t& x) noexcept;
  static size_t decimal_length(const bi_t& x);

  // initializing
  template <std::integral T>
  static void init_one_digit(bi_t& x, T value);
  template <std::integral T>
  static void init_atleast_one_digit(bi_t& x, T value);
  static void init_string(bi_t& x, const std::string& str);

  // misc.
  static dvector to_twos_complement(const dvector& vec);
  static void to_twos_complement_in_place(dvector& vec) noexcept;

  // double
  static void assign_from_double(bi_t&, double);
};

void h_::increment_abs(bi_t& x) {
  if (x.size() == 0 || x[x.size() - 1] == std::numeric_limits<digit>::max()) {
    x.reserve_(x.size() + 1);
  }

  if (x.size() == 0) {
    x.resize_(1);
    x[0] = 1;
    return;
  }

  size_t i = 0;
  bool carry = true;

  while (carry && i < x.size()) {
    if (x[i] == std::numeric_limits<digit>::max()) {
      x[i] = 0;
    } else {
      ++x[i];
      carry = false;
    }
    ++i;
  }

  if (carry) {
    x.vec_.push_back(1);
  }
}

void h_::decrement_abs(bi_t& x) {
  if (x.size() == 0) {
    x.resize_(1);
    x[0] = 1;
    x.negative_ = true;
    return;
  }

  size_t i = 0;
  bool borrow = true;

  while (borrow && i < x.size()) {
    if (x[i] == 0) {
      // Borrow from the next more significant digit
      x[i] = std::numeric_limits<digit>::max();
    } else {
      --x[i];
      borrow = false;
    }
    ++i;
  }

  x.trim_trailing_zeros();
}

/**
 *  @internal
 *  @page add Addition/Subtraction
 *  @ingroup algorithms
 *  Knuth Algorithm A (Vol. 2, 4.3.1, p. 266)
 *  ***
 *  **Input**:
 *  \f[
 *    (u_{n-1} \cdots u_{0})_{b}, (v_{n-1} \cdots v_{0})_{b} \geq 0
 *  \f]
 *
 *  **Output**: radix-b sum
 *  \f[
 *    (w_{n} \cdots w_{0})_{b}, \; w_{n} \in \{0, 1\}
 *  \f]
 *
 *  1. Set \f$ j \leftarrow 0, \; k \leftarrow 0 \f$.
 *
 *  2. Set
 *  \f{align}{
 *    t     &\leftarrow u_{j} + v_{j} + k                                     \\
 *    w_{j} &\leftarrow t \bmod b                                             \\
 *    k     &\leftarrow \left\lfloor \frac{t}{b} \right\rfloor
 *      \Longleftrightarrow k \leftarrow t \geq b
 *  \f}
 *
 *  3. Increase \f$ j \f$ by one. If \f$ j < n \f$, go to (2); else, set \f$
 *  w_{n} \leftarrow k \f$ and terminate.
 *
 *  The use of the temporary \f$ t \f$ in (2) permits the same storage locations
 *  for both input and output (Exercise 30, p. 283). *This applies for algorithm
 *  S as well*.
 *
 *  Knuth Algorithm S (Vol. 2, 4.3.1, p. 267)
 *  ***
 *  **Input**:
 *  \f[
 *    (u_{n-1} \cdots u_{0})_{b} \geq (v_{n-1} \cdots v_{0})_{b} \geq 0
 *  \f]
 *
 *  **Output**: radix-b difference
 *  \f[
 *    (w_{n-1} \cdots w_{0})_{b}
 *  \f]
 *
 *  1. Set \f$ j \leftarrow 0, \; k \leftarrow 0 \f$.
 *
 *  2. Set
 *  \f{align}{
 *    t     &\leftarrow u_{j} - v_{j} + k                                     \\
 *    w_{j} &\leftarrow t \bmod b                                             \\
 *    k     &\leftarrow \left\lfloor \frac{t}{b} \right\rfloor
 *  \f}
 *
 *  3. Increase \f$ j \f$ by one. If \f$ j < n \f$, go to (2); else, terminate.
 *  @endinternal
 */
/**
 *  @name Helpers for addition/subtraction
 *  @note Overlap between `r`, `x`, and `y` is permitted.
 */
///@{

/// |x| + |y|.
void h_::add_abs(bi_t& r, const bi_t& x, const bi_t& y) {
  // Algorithm assumes x.size() >= y.size()
  const bi_t& large = x.size() >= y.size() ? x : y;
  const bi_t& small = x.size() >= y.size() ? y : x;

  r.reserve_(large.size() + 1);

  digit temp{};
  bool carry = 0;
  size_t i = 0;
  for (; i < small.size(); ++i) {
    uints::uaddc(temp, large[i], small[i], carry);
    r[i] = temp;
  }

  while (i < large.size()) {
    temp = large[i] + carry;
    carry = temp < static_cast<uint8_t>(carry);  // MSVC warning C4804
    r[i++] = temp;
  }

  r[i] = carry;
  r.resize_(large.size() + 1);
  r.trim_trailing_zeros();
  r.negative_ = false;
}

/// x + y.
void h_::add(bi_t& r, const bi_t& x, const bi_t& y) {
  if (x.negative()) {
    if (y.negative()) {
      // x < 0, y < 0 ==> x = -|x|, y = -|y|. ==> x + y = -(|x| + |y|)
      add_abs(r, x, y);
      r.negative_ = true;
    } else {
      // x < 0, y >= 0 ==> x = -|x|, y = |y| ==> x + y = |y| - |x|
      sub_abs(r, y, x);
    }
  } else {
    if (y.negative()) {
      // x >= 0, y < 0 ==> x = |x|, y = -|y| ==> x + y = |x| - |y|
      sub_abs(r, x, y);
    } else {
      // x >= 0, y >= 0 ==> x = |x|, y = |y| ==> x + y = |x| + |y|
      add_abs(r, x, y);
    }
  }
}

/// |x| - |y|, assuming |x| > |y|.
void h_::sub_abs_gt(bi_t& r, const bi_t& x, const bi_t& y) {
  assert(x.size() >= y.size());

  r.reserve_(x.size());

  digit temp{};
  bool borrow = 0;
  size_t i = 0;
  for (; i < y.size(); ++i) {
    uints::usubb(temp, x[i], y[i], borrow);
    r[i] = temp;
  }
  while (i < x.size()) {
    temp = x[i] - borrow;
    borrow = temp > x[i];
    r[i++] = temp;
  }

  r.resize_(x.size());
  r.trim_trailing_zeros();
  r.negative_ = false;
}

/// |x| - |y|.
void h_::sub_abs(bi_t& r, const bi_t& x, const bi_t& y) {
  if (x.size() == y.size()) {
    const auto [it_x, it_y] = std::mismatch(x.rbegin(), x.rend(), y.rbegin());

    if (it_x == x.rend()) {
      // x, y have the same digits (or both are zero)
      r.resize_(0);
      r.negative_ = false;
      return;
    }

    if (*it_y > *it_x) {
      sub_abs_gt(r, y, x);
      r.negative_ = true;
    } else {
      sub_abs_gt(r, x, y);
    }
  } else if (x.size() > y.size()) {
    sub_abs_gt(r, x, y);
  } else {
    sub_abs_gt(r, y, x);
    r.negative_ = true;
  }
}

/// x - y.
void h_::sub(bi_t& r, const bi_t& x, const bi_t& y) {
  if (x.negative()) {
    if (y.negative()) {
      // x < 0, y < 0 ==> x = -|x|, y = -|y| ==> x - y = |y| - |x|
      sub_abs(r, y, x);
    } else {
      // x < 0, y >= 0 ==> x = -|x|, y = |y| ==> x - y = -(|x| + |y|)
      add_abs(r, x, y);
      r.negative_ = true;
    }
  } else {
    if (y.negative()) {
      // x >= 0, y < 0 ==> x = |x|, y = -|y| ==> x - y = |x| + |y|
      add_abs(r, x, y);
    } else {
      // x >= 0, y >= 0 ==> x = |x|, y = |y| ==> x - y = |x| - |y|
      sub_abs(r, x, y);
    }
  }
}

///@}

/**
 *  @internal
 *  @page mul Multiplication
 *  @ingroup algorithms
 *  Knuth Algorithm M (Vol. 2, 4.3.1, pp. 268-269)
 *  ***
 *  **Input**: \f$ (u_{m-1} \ldots u_{0})_{b},
 *                 (v_{n-1} \ldots v_{0})_{b} \geq 0 \f$
 *
 *  **Output**: radix-b product \f$ (w_{m+n-1} \ldots w_{0})_{b} \f$
 *
 *  1. Set \f$ w_{m-1}, \ldots , w_{0} \f$ to 0. Set \f$ j \leftarrow 0 \f$.
 *
 *  2. Set \f$ i \leftarrow 0, k \leftarrow 0 \f$.
 *
 *  3. Set (in order from top to bottom)
 *  \f{align}{
 *    t         &\leftarrow u_{i} \times v_{j} + w_{i + j} + k                \\
 *    w_{i + j} &\leftarrow t \bmod b                                         \\
 *    k         &\leftarrow \left\lfloor \frac{t}{b} \right\rfloor
 *  \f}
 *
 *  4. Increase \f$ i \f$ by one. If \f$ i < m \f$, go to (3); else, set \f$
 *  w_{j + m} \leftarrow k \f$.
 *
 *  5. Increase \f$ j \f$ by one. If \f$ j < n \f$, go to (2); else, terminate.
 *
 *  Algorithm permits \f$ v_{j} \f$ to be in the same memory location as
 *  \f$ w_{j + n} \f$ (Exercise 30, p. 627) but, in general, special handling is
 *  needed for overlapping memory (e.g. through the use of temporaries).
 *  @endinternal
 */
/**
 *  @private
 *  @brief Multiply this integer in place by a digit and optionally, add a digit
 *  to this integer.
 *  @note This function does not trim trailing zeros in the digit vector.
 *  @details
 *  Knuth Algorithm M (above), but recognizing
 *  (1) j is always 0
 *  (2) w_{i + j} = w_{i} is always zero when forming t.
 *  (3) we can safely make the initial carry nonzero to implement adding a
 *      digit.
 *  @complexity O(n)
 */
inline void h_::imul1add1(bi_t& x, digit v, digit k = 0) {
  for (auto& d : x.vec_) {
    const ddigit t = static_cast<ddigit>(d) * v + k;
    k = t >> bi_dwidth;         // floor(t / 2^{bi_dwidth})
    d = static_cast<digit>(t);  // t mod 2^{bi_dwidth}
  }

  if (k) {
    x.vec_.push_back(k);
  }
}

/**
 *  @brief Performs `result = a * b`.
 *  @note mult_helpers.hpp proves that multiplying any two digits followed by
 *  adding any two digits to the multiplication result, never overflows double
 *  the width of a digit.
 */
void h_::mul(bi_t& result, const bi_t& a, const bi_t& b) {
  if (a.size() == 0 || b.size() == 0) {
    result.resize_(0);
    result.negative_ = false;
    return;
  }

  const size_t m = a.size();
  const size_t n = b.size();

  const auto [n_result_digits, overflow] = uints::uadd_overflow(m, n);
  if (overflow) {
    throw overflow_error("");
  }

  const bool overlap = &result == &a || &result == &b;

  bi_t temp;
  bi_t& target = overlap ? temp : result;

  target.resize_(n_result_digits);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  std::fill(target.begin(), target.begin() + m, 0);

  for (size_t j = 0; j < n; ++j) {
    digit k = 0;
    for (size_t i = 0; i < m; ++i) {
      const ddigit t = static_cast<ddigit>(a[i]) * b[j] +
                       static_cast<ddigit>(target[i + j]) + k;
      k = t >> bi_dwidth;                     // floor(t / 2^{bi_dwidth})
      target[i + j] = static_cast<digit>(t);  // t mod 2^{bi_dwidth}
    }
    target[j + m] = k;
  }

  target.trim_trailing_zeros();
  target.negative_ = a.negative() != b.negative();

  if (overlap) {
    result.swap(target);
  }
}

/**
 *  @internal
 *  @page div Division by single-precision integer
 *  @ingroup algorithms
 *  Knuth Exercise 16 (Vol. 2, 4.3.1, p. 282)
 *  ***
 *  **Input**: \f$ u = (u_{n-1} \ldots u_{0})_{b} \geq 0 \; \text{and} \;
 *                 v \in (0, b-1] \f$
 *
 *  **Output**: radix-b
 *  \f{align}{
 *    \text{quotient}&:
 *      \left\lfloor \frac{u}{v} \right\rfloor = (w_{n-1} \cdots w_{0})_{b}   \\
 *    \text{remainder}&: u \bmod v = r
 *  \f}
 *
 *  1. Set \f$ r \leftarrow 0, \; j \leftarrow n - 1 \f$.
 *
 *  2. Set
 *  \f{align}{
 *    w_{j} &\leftarrow \left\lfloor \frac{(rb + u_{j})}{v} \right\rfloor     \\
 *    r     &\leftarrow (rb + u_{j}) \bmod v
 *  \f}
 *
 *  3. Decrease \f$ j \f$ by 1. If \f$ j \geq 0 \f$, go to (2); else, terminate.
 *  @endinternal
 */
/**
 *  NOTE: Assumes space and size have already been set for q and r.
 */
void h_::div_algo_single(bi_t& q, bi_t& r, const bi_t& u,
                         const bi_t& v) noexcept {
  ddigit rem{0};

  for (size_t j = u.size() - 1; j < std::numeric_limits<size_t>::max(); --j) {
    const ddigit temp = (rem << bi_dwidth) | u[j];  // rem * bi_base + u[j]
    q[j] = temp / v[0];
    rem = temp % v[0];
  }

  r[0] = static_cast<digit>(rem);

  q.trim_trailing_zeros();
  r.trim_trailing_zeros();
}

/**
 *  @internal
 *  @page division Division - Binary Long Division
 *  @ingroup algorithms
 *  Let \f$ n \f$ denote the number of bits in numerator \f$ u \f$. Let \f$
 *  X(i) \f$ denote the \f$ ith \f$ bit of X (zero-based indexing; zero gives
 *  the LSB). This algorithm assumes that \f$ u \geq 0, \; v > 0 \f$.
 *
 *  1. Set \f$ q \leftarrow 0, \; r \leftarrow 0 \f$.
 *  2. Set \f$ i \leftarrow n - 1 \f$.
 *  3. Set \f$ r \leftarrow r << 1 \f$.
 *  4. Set \f$ r(0) \leftarrow u(i) \f$.
 *  5. If \f$ r \geq v \f$, set
 *  \f{align}{
 *    r    &\leftarrow r - v                                                  \\
 *    q(i) &\leftarrow 1
 *  \f}
 *  6. Decrease \f$ i \f$ by one. If \f$ i \geq 0 \f$, go to (3); else,
 *  terminate.
 *  @endinternal
 */
/**
 *  NOTE: In div_algo_binary(), (1) should already have been accounted for and
 *  space (no need for size, but it's okay if it's been set) should already have
 *  been allocated for q, r. Moreover, unlike some multiple-precision division
 *  algorithms, this algorithm works for v.size() == 1 too.
 */
void h_::div_algo_binary(bi_t& q, bi_t& r, const bi_t& u, const bi_t& v) {
  // (2)
  q = 0;
  r = 0;

  // (3)
  for (unsigned long i = u.bit_length() - 1; i < ULONG_MAX; --i) {
    // (3)(I)
    r <<= 1;

    // (3)(II)
    if (u.test_bit(i)) {
      r.set_bit(0);
    }

    // (3)(III)
    if (h_::cmp_abs(r, v) >= 0) {
      // (3)(III)(a)
      h_::sub_abs(r, r, v);
      // (3)(III)(b)
      q.set_bit(i);
    }
  }
}

/**
 *  @internal
 *  @page knuth_d Division - Knuth Algorithm D
 *  @ingroup algorithms
 *  Knuth Algorithm D (Vol. 2, 4.3.1, pp. 272-273) with Execise 37 (p. 283).
 *  ***
 *  **Input**: \f$ u = (u_{m+n-1} \cdots u_{0})_{b},
 *                 v = (v_{n-1} \cdots v_{0})_{b} \geq 0, \;
 *                 v_{n-1} \neq 0, \; n > 1 \f$
 *
 *  **Output**: radix-b
 *  \f{align}{
 *    \text{quotient}&:
 *      \left\lfloor \frac{u}{v} \right\rfloor = (q_{m} \cdots q_{0})_{b}     \\
 *    \text{remainder}&: u \bmod v = (r_{n-1} \cdots r_{0})_{b}
 *  \f}
 *
 *  1. Normalize. Assuming \f$ b \f$ is a power of 2, set \f$ d \leftarrow 2^{e}
 *  \f$ such that \f$ b > 2^{e} \times v_{n-1} \geq \frac{b}{2} \f$. Then set
 *  \f{align}{
 *    (u_{m+n} \cdots u_{0})_{b} &\text{ to } (u_{m+n-1} \cdots u_{0})_{b}
 *      \times d                                                              \\
 *    (v_{n-1} \cdots v_{0})_{b} &\text{ to } (v_{n-1} \cdots v_{0})_{b}
 *      \times d
 *  \f}
 *  This can be achieved through bit shifting.
 *
 *  2. Set \f$ j \leftarrow m \f$.
 *
 *  3. Set
 *  \f{align}{
 *    \hat{q} &\leftarrow \left\lfloor \frac{u_{j+n}b + u_{j+n-1}}{v_{n-1}}
 *      \right\rfloor                                                         \\
 *    \hat{r} &\leftarrow (u_{j+n}b + u_{j+n-1}) \bmod v_{n-1}
 *  \f}
 *  Now, if \f$ \hat{q} = b \f$ or \f$ \hat{q}v_{n-2} > b\hat{r} + u_{j+n-2}
 *  \f$, then decrease \f$ \hat{q} \f$ by 1, increase \f$ \hat{r} \f$ by \f$
 *  v_{n-1} \f$ and repeat this test if \f$ \hat{r} < b \f$.
 *
 *  4. Replace \f$ (u_{j+n} \cdots u_{j})_{b} \f$ by
 *  \f[
 *    (u_{j+n} \cdots u_{j})_{b} - \hat{q}(v_{n-1} \cdots v_{0})_{b}
 *  \f]
 *  The digits \f$ (u_{j+n},\ldots,u_{j}) \f$ should be kept positive. If the
 *  result of this step is actually negative, \f$ (u_{j+n} \cdots u_{j})_{b} \f$
 *  should be left as the true value plus \f$ b^{n+1} \f$, namely, as the \f$ b
 *  \f$'s complement of the true value and a borrow to the left should be
 *  remembered.
 *
 *  5. Set \f$ q_{j} \leftarrow \hat{q} \f$. If the result of (4) was negative,
 *  go to (6); otherwise, go to (7).
 *
 *  6. Decrease \f$ q_{j} \f$ by 1 and add \f$ (0v_{n-1} \cdots v_{0})_{b} \f$
 *  to \f$ (u_{j+n} \cdots u_{j})_{b} \f$. A carry will occur to the left of
 *  \f$ u_{j+n} \f$ and it should be ignored.
 *
 *  7. Decrease \f$ j \f$ by one. Then, if \f$ j \geq 0 \f$, go to (3).
 *
 *  8. \f$ (q_{m} \cdots q_{0})_{b} \f$ is the quotient and the remainder is the
 *  result of \f$ (u_{n-1} \cdots u_{0})_{b} \f$ divided by \f$ d \f$.
 *  @endinternal
 */
void h_::div_algo_knuth(bi_t& q, bi_t& r, const bi_t& u, const bi_t& v) {
#if defined(BI_DIGIT_64_BIT)
  using sddigit = __int128;
#else
  using sddigit = int64_t;
#endif

  const size_t m = u.size();
  const size_t n = v.size();

  /* (1) Normalize */
  // Find e such that b > 2^{e} * v_{n-1} >= b / 2
  constexpr digit b_half = bi_base >> 1;
  const digit v_msd = v[n - 1];
  int e = 0;
  while ((v_msd << e) < b_half) {
    ++e;
  }
  const unsigned compl_e = bi_dwidth - e;

  bi_t u_norm, v_norm;
  u_norm.resize_(m + 1);
  v_norm.resize_(n);

  // u_norm set to u * 2^{e}. Cast to ddigit as e may be 0 which would be UB
  u_norm[0] = u[0] << e;
  for (size_t i = 1; i < m; ++i) {
    u_norm[i] = (u[i] << e) | (static_cast<ddigit>(u[i - 1]) >> compl_e);
  }
  u_norm[m] = static_cast<ddigit>(u[m - 1]) >> compl_e;

  // v_norm set to v * 2^{e}
  v_norm[0] = v[0] << e;
  for (size_t i = 1; i < n; ++i) {
    v_norm[i] = (v[i] << e) | (static_cast<ddigit>(v[i - 1]) >> compl_e);
  }

  const digit vp = v_norm[n - 1], vpp = v_norm[n - 2];
  /* (2) Initialize j. Also (7) Loop on j */
  for (size_t j = m - n; j < std::numeric_limits<size_t>::max(); --j) {
    /* (3) Calculate q_hat */
    const ddigit tmp = u_norm[j + n] * bi_base + u_norm[j + n - 1];
    ddigit q_hat = tmp / vp;
    ddigit r_hat = tmp % vp;

    while (q_hat == bi_base ||
           q_hat * vpp > bi_base * r_hat + u_norm[j + n - 2]) {
      --q_hat;
      r_hat += vp;
      if (r_hat >= bi_base)
        break;
    }

    /* (4) Multiply and subtract */
    sddigit b{};
    for (size_t i = 0; i < n; ++i) {
      const ddigit product = q_hat * v_norm[i];
      const sddigit stmp =
          static_cast<sddigit>(u_norm[j + i]) - static_cast<digit>(product) - b;
      u_norm[j + i] = static_cast<digit>(stmp);
      b = static_cast<sddigit>(product >> bi_dbits) - (stmp >> bi_dbits);
    }
    const bool neg{u_norm[j + n] < b};
    u_norm[j + n] = static_cast<digit>(u_norm[j + n] - b);

    /* (5) Test remainder */
    q[j] = q_hat;

    if (neg) {
      /* (6) Add back */
      // Decrease q_{j} by 1
      --q[j];

      // Add (0v_{n-1}...v_{0})_{b} to (u_{j+n}...u_{j})_{b}
      digit c = 0;
      for (size_t i = 0; i < n; ++i) {
        const ddigit tmp = (static_cast<ddigit>(u_norm[j + i]) + v_norm[i]) + c;
        u_norm[j + i] = tmp;
        c = tmp >> bi_dwidth;
      }
      // A carry will occur to the left of u_{j+n} and it should be ignored
      u_norm[j + n] += c;
      assert(u_norm[j + n] == 0);
    }
  }

  /* (8) Unnormalize */
  // (u_{n-1}...u_{0})_{b} / 2^e
  const size_t last_idx = n - 1;
  r[last_idx] = u_norm[last_idx] >> e;
  for (size_t i = last_idx; i-- > 0;) {
    r[i] = (static_cast<ddigit>(u_norm[i + 1]) << compl_e) | (u_norm[i] >> e);
  }

  q.trim_trailing_zeros();
  r.trim_trailing_zeros();
}

/**
 *  @brief `Q = N / D, R = N % D`, in one pass.
 *  @throw `bi::division_by_zero` if the divisor is zero.
 *
 *  (ISO/IEC 2020, 7.6.6, C++):
 *    The binary / operator yields the quotient, and the binary % operator
 *    yields the remainder from the division of the first expression by the
 *    second. If the second operand of / or % is zero the behavior is undefined.
 *    For integral operands the / operator yields the algebraic quotient with
 *    any fractional part discarded; if the quotient a/b is representable in the
 *    type of the result, (a/b)*b + a%b is equal to a; otherwise, the behavior
 *    of both a/b and a%b is undefined.
 */
void h_::divide(bi_t& Q, bi_t& R, const bi_t& N, const bi_t& D) {
  if (D.size() == 0) {
    throw division_by_zero("Division by zero attempt.");
  }

  const size_t size_N = N.size();
  const size_t size_D = D.size();

  // |N| < |D| case
  if ((size_N == size_D && N[size_N - 1] < D[size_D - 1]) || size_N < size_D) {
    Q = 0;  // |N| < |D| ==> |N| / |D| < 1 ==> Q computes to 0
    R = N;  // Computation (a/b)*b + a%b should equal a ==> a%b is a
    return;
  }

  // TRUE: size_N >= size_D > 0

  // Ensure enough space in Q and R
  const size_t size_Q = size_N - size_D + 1;
  Q.resize_(size_Q);
  R.resize_(size_D);

  // Unsigned integer division algorithms
  if (size_D == 1) {
    // Knuth (Vol. 2, 4.3.1, p. 272) recommends using the algorithm used in
    // div_algo_single() when size_D is 1.
    div_algo_single(Q, R, N, D);
  } else {
    div_algo_knuth(Q, R, N, D);
  }

  Q.negative_ = D.negative() ^ N.negative();
  R.negative_ = R.size() > 0 && N.negative();
}

/**
 *  @name Shift operators helpers
 *  @note Both `left_shift` and `right_shift` support both `&result == &x` and
 *  `&result != &x`.
 */
///@{

/**
 *  From ISO/IEC 2020 (C++), "[t]he value of `E1 << E2` is the unique value
 *  congruent to `E1 * 2^{E2}` modulo `2^{N}` , where `N` is the width of the
 *  type of the result... E1 is left-shifted E2 bit positions; vacated bits are
 *  zero-filled."
 */
void h_::left_shift(bi_t& result, const bi_t& x, bi_bitcount_t n_bits) {
  if (x.size() == 0) {
    result.resize_unsafe_(0);
    result.negative_ = false;
    return;
  }

  if (n_bits == 0) {
    result = x;  // Copy assignment avoids copying if &result == &x
    return;
  }

  const size_t size_x = x.size();
  const bi_bitcount_t digit_shift = n_bits / bi_dwidth;
  const unsigned bit_shift = n_bits % bi_dwidth;

  const size_t size_result = size_x + digit_shift + (bit_shift ? 1 : 0);
  if (size_result < size_x) {
    throw overflow_error("Result size exceeds SIZE_MAX.");
  }

  // Create a temporary vector if `result` and `x` overlap
  dvector temp_vec;
  dvector& target_vec = (&result == &x) ? temp_vec : result.vec_;

  target_vec.resize(size_result);

  size_t i = 0;
  // Zero-fill vacated digits
  for (; i < digit_shift; ++i) {
    target_vec[i] = 0;
  }

  const auto compl_bit_shift = bi_dwidth - bit_shift;

  digit carry = 0;
  for (size_t j = 0; j < size_x; ++j, ++i) {
    const digit current = x[j];
    target_vec[i] = (current << bit_shift) | carry;
    carry = bit_shift ? (current >> compl_bit_shift) : 0;
  }

  if (bit_shift) {
    target_vec[i] = carry;
  }

  if (&result == &x) {
    result.vec_ = std::move(temp_vec);
  }

  result.trim_trailing_zeros();
  result.negative_ = x.negative();
}

/**
 *  From ISO/IEC 2020 (C++), "[t]he value of `E1 >> E2` is `E1/2^{E2}, rounded
 *  down. [Note: E1 is right-shifted E2 bit positions. Right-shift on signed
 *  integral types is an arithmetic right shift, which performs sign-extension.
 *  —end note]".
 *
 *  @note No temporary is needed for in-place right-shifting.
 */
void h_::right_shift(bi_t& result, const bi_t& x, bi_bitcount_t n_bits) {
  // TODO: maybe set `result = x` for special cases x.size() == 0 || n_bits == 0
  const size_t size_x = x.size();

  const bi_bitcount_t digit_shift = n_bits / bi_dwidth;
  const unsigned bit_shift = n_bits % bi_dwidth;

  if (size_x <= digit_shift) {
    if (x.negative()) {
      // Set result to -1
      result.resize_(1);
      result[0] = 1;
      result.negative_ = true;
    } else {
      // Set result to 0
      result.resize_unsafe_(0);
      result.negative_ = false;
    }
    return;
  }

  result.resize_(size_x - digit_shift);

  if (bit_shift == 0) {
    for (size_t i = 0; i < result.size(); ++i) {
      result[i] = x[i + digit_shift];
    }
  } else {
    const unsigned compl_bit_shift = bi_dwidth - bit_shift;

    for (size_t i = 0, k = digit_shift + 1; i < result.size(); ++i, ++k) {
      result[i] = x[k - 1] >> bit_shift;
      if (k < size_x) {
        result[i] |= x[k] << compl_bit_shift;
      }
    }
  }

  result.trim_trailing_zeros();
  result.negative_ = x.negative();

  // At this point, the result is trunc(x / 2^{n_bits}) for all x

  // Adjust for floor division for negative numbers
  if (x.negative()) {
    bool subtract_one = false;
    if (bit_shift > 0) {
      const digit mask = (static_cast<digit>(1) << bit_shift) - 1;
      if ((x[digit_shift] & mask) != 0) {
        subtract_one = true;
      }
    }
    if (!subtract_one && digit_shift > 0) {
      for (size_t i = 0; i < digit_shift; ++i) {
        if (x[i] != 0) {
          subtract_one = true;
          break;
        }
      }
    }
    if (subtract_one) {
      --result;
    }
  }
}

///@}

/// @private
template <h_::BitwiseOperation Op>
void h_::bitwise_operation_impl(bi_t& result, const bi_t& x, const bi_t& y) {
  size_t max_size = std::max(x.size(), y.size());

  const dvector& lhs_digits = x.negative_ ? to_twos_complement(x.vec_) : x.vec_;
  const dvector& rhs_digits = y.negative_ ? to_twos_complement(y.vec_) : y.vec_;

  bool result_negative;  // NOLINT(cppcoreguidelines-init-variables)
  if constexpr (Op == BitwiseOperation::AND) {
    result_negative = x.negative_ && y.negative_;
  } else if constexpr (Op == BitwiseOperation::OR) {
    result_negative = x.negative_ || y.negative_;
  } else if constexpr (Op == BitwiseOperation::XOR) {
    result_negative = x.negative_ != y.negative_;
    ++max_size;
  }

  result.resize_(max_size);

  for (size_t i = 0; i < result.size(); ++i) {
    const digit lhs_digit =
        i < lhs_digits.size() ? lhs_digits[i] : (x.negative_ ? bi_dmax : 0);
    const digit rhs_digit =
        i < rhs_digits.size() ? rhs_digits[i] : (y.negative_ ? bi_dmax : 0);

    if constexpr (Op == BitwiseOperation::AND) {
      result[i] = lhs_digit & rhs_digit;
    } else if constexpr (Op == BitwiseOperation::OR) {
      result[i] = lhs_digit | rhs_digit;
    } else if constexpr (Op == BitwiseOperation::XOR) {
      result[i] = lhs_digit ^ rhs_digit;
    }
  }

  result.negative_ = result_negative;

  if (result.negative_) {
    to_twos_complement_in_place(result.vec_);
  }

  result.trim_trailing_zeros();
}

/// @private
template <h_::BitwiseOperation Op>
bi_t h_::bitwise_operation(const bi_t& a, const bi_t& b) {
  bi_t result;
  bitwise_operation_impl<Op>(result, a, b);
  return result;
}

/// @private
template <h_::BitwiseOperation Op>
bi_t& h_::ibitwise_operation(bi_t& x, const bi_t& other) {
  bitwise_operation_impl<Op>(x, x, other);
  return x;
}

// Assumes x, y >= 0
int h_::cmp_abs(const bi_t& x, const bi_t& y) noexcept {
  if (x.size() > y.size()) {
    return 1;
  } else if (x.size() < y.size()) {
    return -1;
  } else {
    for (size_t i = x.size(); i-- > 0;) {
      if (x.vec_[i] > y.vec_[i]) {
        return 1;
      } else if (x.vec_[i] < y.vec_[i]) {
        return -1;
      }
    }
    return 0;
  }
}

/// Return negative, zero, or positive depending if x < y, x == y, or x > y.
int h_::cmp(const bi_t& x, const bi_t& y) noexcept {
  if (!x.negative() && y.negative()) {
    // x >= 0, y < 0
    return 1;
  } else if (x.negative() && !y.negative()) {
    // x < 0, y >= 0
    return -1;
  } else if (x.negative() && y.negative()) {
    // x, y < 0
    if (x.size() > y.size()) {
      return -1;
    } else if (x.size() < y.size()) {
      return 1;
    } else {
      for (size_t i = x.size(); i-- > 0;) {
        if (x.vec_[i] < y.vec_[i]) {
          return 1;
        } else if (x.vec_[i] > y.vec_[i]) {
          return -1;
        }
      }
      return 0;
    }
  } else {
    // x, y >= 0
    return h_::cmp_abs(x, y);
  }
}

template <std::integral T>
int h_::cmp(const bi_t& a, T b) noexcept {
  if (b == 0) {
    if (a.size() == 0) {
      return 0;  // a
    }
    return a.negative() ? -1 : 1;  // b
  }

  using UnsignedT = typename std::make_unsigned<T>::type;
  const bool b_negative = b < 0;
  const UnsignedT unsigned_b =
      b_negative ? -static_cast<UnsignedT>(b) : static_cast<UnsignedT>(b);

  if (a.negative() && !b_negative) {
    return -1;  // c
  }
  if (!a.negative() && b_negative) {
    return 1;  // d
  }

  size_t n_b_digits = 0;
  if constexpr (std::numeric_limits<UnsignedT>::max() <= bi_dmax) {
    n_b_digits = (unsigned_b != 0) ? 1 : 0;
  } else {
    UnsignedT temp_b = unsigned_b;
    while (temp_b != 0) {
      temp_b >>= bi_dwidth;
      n_b_digits++;
    }
  }

  const size_t a_size = a.size();
  if (a_size < n_b_digits) {
    return a.negative() ? 1 : -1;  // e
  }

  if (a_size > n_b_digits) {
    return a.negative() ? -1 : 1;  // f
  }

  for (size_t i = n_b_digits; i-- > 0;) {
    const digit a_digit = a[i];
    const digit b_digit = static_cast<digit>(unsigned_b >> (bi_dwidth * i));

    if (a_digit < b_digit) {
      return a.negative() ? 1 : -1;  // g
    }
    if (a_digit > b_digit) {
      return a.negative() ? -1 : 1;  // h
    }
  }
  return 0;  // i
}

/**
 *  @name Private helpers for `to_string()`.
 */
///@{

/// Divides the integer in place by 10, returning the remainder.
uint8_t h_::idiv10(bi_t& x) noexcept {
  constexpr ddigit base = static_cast<ddigit>(bi_dmax) + 1;
  uint8_t carry = 0;

  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  for (size_t i = x.size(); i-- > 0;) {
    const ddigit current = (base * carry) + x.vec_[i];
    x.vec_[i] = static_cast<digit>(current / 10);
    carry = static_cast<uint8_t>(current % 10);
  }
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

  x.trim_trailing_zeros();
  return carry;
}

/**
 *  @brief Return an estimate of the number of decimal digits required to
 *  represent this integer.
 *
 *  Given an n-digit integer in base b, the largest integer representable is
 *  \f[
 *    b^{n} - 1
 *  \f]
 *
 *  The minimum number of base b digits, n, required to represent any m-digit
 *  base c integer is such that:
 *  \f{align}{
 *    b^{n} - 1                 & >= c^{m} - 1                                \\
 *    b^{n}                     & >= c^{m}                                    \\
 *    \frac{b^{n}}{c^{m}}       & >= 1                                        \\
 *    \log(b^{n}) - \log(c^{m}) & >= 0                                        \\
 *    n                         & >= m \cdot \frac{\log(c)}{\log(b)}
 *  \f}
 *
 *  Use
 *  \f[
 *    n = \left\lceil m \cdot \frac{\log(c)}{\log(b)} \right\rceil
 *  \f]
 *
 *  Thus, the minimum number of base 10 digits required to represent any m-digit
 *  base 2 integer is:
 *  \f[
 *    n = \left\lceil m * \log_{10}(2) \right\rceil
 *  \f]
 *
 *  @throw overflow_error Thrown when the estimated number of decimal digits is
 *  beyond the theoretical or practical limit for representation, indicating an
 *  unmanageable or impractical size for the corresponding decimal string.
 */
size_t h_::decimal_length(const bi_t& x) {
  // TODO: a different exception is probably more appropriate in this func
  constexpr const char* s = "Decimal digit estimation exceeds practical limit.";
  constexpr double log10_2 = 0.30103;  // log10(2)

  const bi_bitcount_t bitlen = x.bit_length();
  if (bitlen > dbl_max_int) {
    throw overflow_error(s);
  }

  const bi_bitcount_t r = static_cast<bi_bitcount_t>(
      std::floor(static_cast<double>(bitlen) * log10_2) + 1);
  if (r > std::numeric_limits<size_t>::max() - 2) {
    throw overflow_error(s);
  }

  return r;
}

///@}

template <std::integral T>
void h_::init_one_digit(bi_t& x, T value) {
  x.resize_(1);

  if (value >= 0) {
    x[0] = static_cast<digit>(value);
    x.negative_ = false;
  } else {
    x[0] = static_cast<digit>(0) - static_cast<digit>(value);
    x.negative_ = true;
  }
}

template <std::integral T>
void h_::init_atleast_one_digit(bi_t& x, T value) {
  using UnsignedT = typename std::make_unsigned<T>::type;

  size_t size = 1;
  UnsignedT temp, uvalue;

  if (value >= 0) {
    uvalue = value;
    x.negative_ = false;
  } else {
    uvalue = static_cast<UnsignedT>(0) - static_cast<UnsignedT>(value);
    x.negative_ = true;
  }

  temp = uvalue;

  while (temp >>= bi_dwidth) {
    ++size;
  }

  x.resize_(size);

  for (size_t i = 0; uvalue != 0; ++i) {
    x[i] = static_cast<digit>(uvalue);
    uvalue >>= bi_dwidth;
  }
}

// TODO: move this somewhere else.
/**
 *  @internal
 *  @page string_to_integer String to Integer
 *  @ingroup algorithms
 *
 *  String to integer conversion algorithms (e.g. used by `std::strtoul`,
 *  `std::from_chars`, and company) tend to follow a similar structure.
 *
 *  In the C/C++ Standards, many string to integer conversion functions have an
 *  integer `base` argument, accepting a base in \f$ [2, 36] \f$. The expected
 *  string of characters consists of digits and/or letters, together,
 *  representing the integer in the provided base. When processing the string of
 *  digits, characters '0' to '9' are mapped to the integer values 0 to 9, and
 *  the 26 letters in the alphabet, `a` (or `A`) to `z` (or `Z`), are mapped to
 *  the values 10 to 35:
 *
 *  \f{align}{
 *    a & \mapsto 10   \\
 *    b & \mapsto 11   \\
 *      & ...     \\
 *    z & \mapsto 35
 *  \f}
 *
 *  Suppose we have a string of \f$ m \f$ characters, \f$ (ch_{m-1}, \ldots,
 *  ch_{0}) \f$, where each character represents a digit in some base \f$ b \in
 *  [2, 36] \f$. (Since \f$ b > 1 \f$, there exists unique coefficients \f$
 *  a_{m-1}, \ldots, a_{0} \f$ such that the integer is equal to \f$ a_{m-1}
 *  \cdot b^{m-1} + \cdots + a_{0} \cdot b^{0}) \f$. The resulting integer
 *  can be represented as:
 *
 *  \f[
 *    (a_{m-1} \cdots a_{0})_{b} = a_{m-1} \cdot b^{m-1} + \cdots + a_{0} \cdot
 *                                 b^{0}
 *  \f]
 *
 *  1. **Set result to 0**. \f$ \text{result} = 0 \f$
 *
 *  2. **Iterate over the characters**. For each character in \f$ (ch_{m-1},
 *  \ldots, ch_{0}) \f$:
 *
 *    (a) **Convert the character to the base b digit it represents**. Call this
 *    value \f$ a_{j} \f$, where \f$ a_{j} \in \{0, 1, ..., b - 1\} \f$. \f$
 *    a_{j} \f$ is the "current digit".
 *
 *    (b) **Multiply and add**.
 *    \f[
 *        \text{result} = \text{result} \cdot b + a_{j}
 *    \f]
 *
 *  Writing out the value of the result at the end of each iteration:
 *
 *  \f{align}{
 *    k = 0&:
 *      \text{result} = a_{m - 1}                                             \\
 *    k = 1&:
 *      \text{result} = a_{m - 1} \cdot b + a_{m-2}                           \\
 *    k = 2&:
 *      \text{result} = a_{m - 1} \cdot b^{2} + a_{m - 2} \cdot b + a_{m - 3} \\
 *    &\ldots                                                                 \\
 *    k = m - 1&:
 *      \text{result} = a_{m - 1} \cdot b^{m - 1} + \cdots + a_{0} \cdot b^{0}
 *  \f}
 *
 *  It's clear we would be able to prove by induction that this holds generally.
 *
 *  @note
 *  Step (2)(a).
 *  Characters \f$ ch_{j} \f$ in the range ['0', '9'] have values
 *  \f[
 *    a_{j} = ch_{j} - \text{'0'}
 *  \f]
 *  The fact that characters corresponding to decimal digits have consecutive
 *  integer values is guaranteed by the C/C++ standards. In most systems,
 *  lowercase and uppercase letters also have consecutive integer values, but
 *  this is not guaranteed by the standards. In systems that do make their
 *  values consecutive, we can use the relationships below. However, for
 *  maximizing portability, this implementation *does not* assume that their
 *  values are consecutive. <br><br>
 *  Characters in the range ['a', 'z'] have values
 *  \f[
 *    a_{j} = ch_{j} - \text{'a'} + 10
 *  \f]
 *  Character in the range ['A', 'Z'] have values
 *  \f[
 *    a_{j} = ch_{j} - \text{'A'} + 10
 *  \f]
 *
 *  Once we start dealing with many very large strings, analysis and benchmarks
 *  show that we can do a lot better by processing the string of alphanumerics
 *  in batches.
 *
 *  Let \f$ e \f$ denote the exponent corresponding to the highest power of base
 *  \f$ b \in [2, 36] \f$ that fits in a base \f$ \text{bi_base} > b \f$ digit
 *  [\ref e_example "1"]. By definition, any \f$ e \f$-digit base-b integer
 *
 *  \f[
 *    \phi_{e - 1}\cdot b^{e - 1} + \cdots + \phi_{0} \cdot b^{0}
 *  \f]
 *
 *  can be represented by a single base-`bi_base` digit. The exponent \f$ e \f$
 *  can be referred to as the **maximum batch size** (i.e. the maximum number of
 *  characters in a batch).
 *
 *  1. **Set result to 0**. \f$ \text{result} = 0 \f$.
 *
 *  2. **Set k to 0**. \f$ k = 0 \f$. Define \f$ k \f$ as a zero-based index for
 *  the characters in a string of \f$ m \f$ alphanumerics. Valid indices are in
 *  the set \f$ [0, m - 1] \f$.
 *
 *  3. **Iterate over the characters**.
 *
 *    (a) If \f$ k \f$ is equal to \f$ m \f$, **return** \f$ \text{result} \f$.
 *
 *    (b) **Set batch value to 0**. \f$ \text{batch_value} = 0 \f$.
 *
 *    (c) **Calculate how many characters to batch**: \f$ \lambda \f$. This
 *    should be the minimum of \f$ e \f$ and the number of characters remaining
 *    in the string that have not yet been processed.
 *    \f[
 *      \lambda \equiv \min \{e, \; m - k\}
 *    \f]
 *
 *    (d) **Calculate the batch value**. Use the algorithm specified above. That
 *    is, for each \f$ j \in (0, 1, \ldots, \lambda - 1) \f$, obtain the base-b
 *    digit that the current character maps to (call this \f$ \text{cur_digit}
 *    \f$), calculate
 *    \f[
 *      \text{batch_value} = \text{batch_value} * b + \text{cur_digit}
 *    \f]
 *
 *    and then increase \f$ k \f$ by 1.
 *
 *    At the end of this loop, the batch value represents a \f$ \lambda \f$
 *    digit base-b integer or a single base \f$ b^{\lambda} \f$ digit.
 *
 *    (e) **Multiply and add**.
 *    \f[
 *      \text{result} = \text{result} \cdot b^{\lambda} + \text{batch_value}
 *    \f]
 *
 *  To see that this works, it helps to write out a few iterations.
 *
 *  If \f$ m \leq e \f$, \f$ \text{result} = \text{batch_value} \f$ and we are
 *  done.
 *
 *  If \f$ m \gt e \f$, there's at least one full batch and some residual. Let's
 *  consider this case.
 *
 *  After processing the **first substring**:
 *
 *  \f{align}{
 *  \text{result}
 *    &= 0 \cdot b^{e} + (a_{m-1}b^{e-1} + \cdots + a_{m-e}b^{0})             \\
 *    &= a_{m-1}b^{e-1} + \cdots + a_{m-e}b^{0}
 *  \f}
 *
 *  Suppose \f$ m - k \ge e\f$, so that we have another full batch. The result
 *  after processing the **second substring** is:
 *
 *  \f{align}{
 *  \text{result}
 *    &= (a_{m-1}b^{e-1} + \cdots + a_{m-e}b^{0}) \cdot b^{e} +
 *       (a_{m-e-1}b^{e-1} + \cdots + a_{m-2e}b^{0})                          \\
 *    &= a_{m-1}b^{2e-1} + \cdots + a_{m-e}b^{e} + \cdots + a_{m-2e}b^{0}
 *  \f}
 *
 *  Suppose \f$ m - k \ge e\f$ a total of \f$ \Omega \f$ times. After processing
 *  the \f$ \Omega\text{th} \f$ **substring**:
 *
 *  \f{align}{
 *  \text{result}
 *    &= a_{m-1}b^{\Omega e - 1} + \cdots + a_{m-\Omega e}b^{0}
 *  \f}
 *
 *  Suppose \f$ l \equiv m - k < e \f$. This will be the last substring. After
 *  processing the last substring:
 *
 *  \f{align}{
 *  \text{result}
 *    &= (a_{m-1}b^{\Omega e-1} + \cdots + a_{m-\Omega e}b^{0})\cdot b^{l} +
 *       (a_{m - \Omega e - 1}b^{l-1} + \cdots + a_{0}b^{0})                  \\
 *    &= a_{m-1}b^{(\Omega e +l)-1} + \cdots + a_{0}b^{0}                     \\
 *    &= a_{m-1}b^{m-1} + \cdots + a_{0}b^{0}
 *  \f}
 *
 *  Above, \f$ \Omega e + l \f$ is none other than the number of characters in
 *  the string of alphanumerics, which, by definition, is \f$ m \f$.
 *
 *  @anchor e_example 1. For example, if \f$ \text{bi_base} = 2^{64} \f$, the
 *  highest power of 10 that fits in a base-`bi_base` digit is \f$ 10^{19} \f$.
 *  \f$ e \f$ is then 19.
 *  @endinternal
 */
/**
 *  In the comment block above `decimal_length()`, it is shown how to calculate
 *  the minimum number of base-b digits, \f$ n \f$, required to represent any
 *  integer with \f$ m \f$ base-c digits.
 *
 *  We can find an upper bound another way as well.
 *
 *  Let \f$ e \f$ denote the largest exponent such that \f$ c^{e} \f$ fits in
 *  one base-b digit. By construction, any e-digit base-c number will fit in a
 *  base-b digit, but not every \f$ (e + 1) \f$ digit base-c number will. By
 *  dividing the number of base-c digits in an integer, \f$ m \f$, by \f$ e \f$,
 *  and rounding up the result of the division to the nearest integer, we obtain
 *  an upperbound on the number of base-b digits, \f$ n f$, needed to represent
 *  any m-digit base-c integer.
 */
/**
 *  @name Private helpers for string constructors
 */
///@{

constexpr auto pow2_8 = 1 << 8;

constexpr std::array<uint8_t, pow2_8> create_char_to_b36_map() {
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  constexpr auto n_digits = 10;
  constexpr auto n_letters = 26;
  constexpr std::array<char, n_letters> lowercase{
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
  constexpr std::array<char, n_letters> uppercase{
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
      'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

  std::array<uint8_t, pow2_8> map{};
  map.fill(0xFF);

  for (uint8_t i = 0; i < n_digits; ++i) {
    map.at(static_cast<uint8_t>('0') + i) = i;
  }

  for (uint8_t i = 0; i < n_letters; ++i) {
    map.at(static_cast<uint8_t>(lowercase.at(i))) = 10 + i;
    map.at(static_cast<uint8_t>(uppercase.at(i))) = 10 + i;
  }

  return map;
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

constexpr auto char_to_int_map = create_char_to_b36_map();

constexpr uint8_t char_to_b36(char ch) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  return char_to_int_map[static_cast<uint8_t>(ch)];
}

// Calculate 10 ** n
constexpr digit pow10(size_t n) {
  digit result = 1;
  for (size_t i = 0; i < n; ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    result *= 10;
  }
  return result;
}

// Generate an array of powers of ten
template <size_t... Indices>
constexpr auto make_powers_of_ten(std::index_sequence<Indices...>) {
  return std::array<digit, sizeof...(Indices)>{pow10(Indices)...};
}

// If digit <==> uint32_t (uint64_t), 10^{9} (10^{19}) is the highest power of
// 10 that fits in it.
constexpr unsigned max_batch_size = (bi_dwidth == 64) ? 19 : 9;

constexpr auto powers_of_ten =
    make_powers_of_ten(std::make_index_sequence<max_batch_size + 1>{});

void h_::init_string(bi_t& x, const std::string& s) {
  // std::stoi and company allow leading whitespace and a plus/minus sign. We
  // follow suit.

  // Allow leading whitespace
  auto it = std::find_if_not(s.begin(), s.end(),
                             [](char ch) { return std::isspace(ch); });

  // Allow plus/minus sign to precede first decimal digit
  x.negative_ = false;
  if (it != s.end()) {
    if (*it == '-') {
      x.negative_ = true;
      ++it;
    } else if (*it == '+') {
      ++it;
    }
  }

  const auto start_digit = it;
  it = std::find_if_not(start_digit, s.end(),
                        [](char ch) { return std::isdigit(ch); });

  if (start_digit == it) {
    throw std::invalid_argument("Invalid string format.");
  }

  size_t n_base10 = std::distance(start_digit, it);  // it - start_digit
  const size_t n_digits = uints::div_ceil(n_base10, max_batch_size);

  x.reserve_(n_digits);
  x.resize_unsafe_(0);

  for (auto dec_it = start_digit; dec_it < it;) {
    /* We could replace all the code in the body of this loop with just
     * `imul1add1(10, *dec_it++ - '0');` and the end result will be the same.
     * However, it is more efficient to batch some base-10 digits together. */
    // Initialize batch value
    digit batch = 0;

    // Calculate how many base-10 digits we can process in this batch
    const size_t remaining = std::distance(dec_it, it);
    const size_t digits_in_batch =
        std::min(remaining, static_cast<size_t>(max_batch_size));

    // Convert batch substring to integer value
    for (size_t j = 0; j < digits_in_batch; ++j, ++dec_it) {
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
      batch = (*dec_it - '0') + batch * 10;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    h_::imul1add1(x, powers_of_ten[digits_in_batch], batch);
  }

  x.trim_trailing_zeros();
}

///@}

/// @private
inline dvector h_::to_twos_complement(const dvector& vec) {
  const size_t vec_size = vec.size();
  dvector result;
  result.resize(vec_size);

  bool carry = true;
  for (size_t i = 0; i < vec_size; ++i) {
    const digit sum = ~vec[i] + carry;
    carry = sum < static_cast<uint8_t>(carry);
    result[i] = sum;
  }

  return result;
}

/// @private
inline void h_::to_twos_complement_in_place(dvector& vec) noexcept {
  const size_t vec_size = vec.size();

  bool carry = true;
  for (size_t i = 0; i < vec_size; ++i) {
    const digit sum = ~vec[i] + carry;
    carry = sum < static_cast<uint8_t>(carry);
    vec[i] = sum;
  }
}

/**
 *  From C++20 (7.3.10, p. 93): "A prvalue of a floating-point type can be
 *  converted to a prvalue of an integer type. The conversion truncates; that
 *  is, the fractional part is discarded. The behavior is undefined if the
 *  truncated value cannot be represented in the destination type."
 */
void h_::assign_from_double(bi_t& x, double d) {
  if (std::isnan(d) || std::isinf(d)) {
    throw from_float(
        "Conversion error: NaN or infinity cannot be converted to an integer.");
  }

  if (d > -1 && d < 1) {
    x = 0;
    return;
  }

  const bool neg = d < 0;
  if (neg) {
    d = -d;
  }

  size_t n_digits{1};
  while (bi_base_dbl <= d) {
    // Equiv. to `d /= bi_base_dbl;`, but multiplication is generally cheaper
    d *= bi_base_dbl_reciprocal;
    ++n_digits;
  }

  x.resize_(n_digits);

  for (size_t i = n_digits; i-- > 0;) {
    x[i] = static_cast<digit>(d);
    d = (d - x[i]) * bi_base_dbl;
  }

  x.negative_ = neg;
  x.trim_trailing_zeros();
}

}  // namespace bi

#endif  // BI_SRC_H__HPP_
