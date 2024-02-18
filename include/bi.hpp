/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_INCLUDE_BI_HPP_
#define BI_INCLUDE_BI_HPP_

#include <algorithm>
#include <cassert>
#include <compare>
#include <iostream>
#include <limits>
#include <span>
#include <string>
#include <utility>

#include "bi_config.hpp"
#include "bi_digit_vector.hpp"

// TODO: modularize through the use of more files and separate implementation
// from interface.

namespace bi {

using bi_config::bi_base;
using bi_config::bi_bitcount_t;
using bi_config::bi_dbits;
using bi_config::bi_dmax;
using bi_config::bi_dwidth;
using bi_config::dbl_max_int;
using bi_config::ddigit;
using bi_config::digit;

/**
 *  @brief Arbitrary-precision integer type and related functions.
 *
 *  An instance of `bi_t` represents an arbitrary precision integer.
 *
 *  The implementation has several design goals, including, but not limited
 *  to:
 *  1. **Memory safety**, through the use of smart pointers or standard library
 *     containers.
 *  2. **Strong exception safety**. Some operations may throw, but if they do,
 *     original values should remain intact.
 *
 *  @throw std::bad_alloc Throws in case of memory allocation failure.
 *  @throw bi::overflow_error Throws if an operation expects the result to
 *  require a digit vector with `size()` exceeding `max_size()`. See
 *  [internals](@ref internals) below.
 *  @throw bi::division_by_zero Throws if a division by zero attempt is
 *  detected.
 *  @throw std::invalid_argument Throws in a string constructor if an invalid
 *  string or argument is provided.
 *
 *  The custom exception classes derive from `std::runtime_error`. To catch and
 *  handle these specific exceptions, `#include` the `"bi_exceptions.hpp"`
 *  header (in addition to `"bi.hpp"`).
 *
 *  ## Internals
 *
 *  The representation of a `bi_t` consists of a digit vector and a boolean
 *  indicating if the integer is negative. The integer is represented internally
 *  as a base \f$ 2^{n} \f$ integer where \f$ n \f$ is typically 32 bits. An
 *  element of the digit vector, i.e. a digit, is typically a `uint32_t`. The
 *  digit vector stores the magnitude of the integer, with least significant
 *  digits first. Counts of bits are represented by the `bi_bitcount_t` type,
 *  which is an `unsigned long`. Counts of bytes for dynamically allocated
 *  memory expects a `size_t` type. Thus, the digit vector is theoretically
 *  constrained to be less than or equal to `SIZE_MAX / bi_sizeof_digit`, where
 *  `bi_sizeof_digit` represents the number of bytes in a digit (typically 4).
 *  Additionally, because counts of bits use the `unsigned long` type, we also
 *  constrain the maximum size of the vector to be less than or equal to
 *  `ULONG_MAX / bi_dbits`, where `bi_dbits` is the number of bits in the digit
 *  data type (typically 32). Together, these constraints determine `max_size()`
 *  for the internal digit vector. An operation that expects a result to exceed
 *  `max_size()` throws `bi::overflow_error`.
 */

class BI_API bi_t {
 public:
  // Constructors
  bi_t() noexcept;
  template <std::integral T>
  bi_t(T);  // NOLINT(runtime/explicit)
  bi_t(const bi_t&);
  bi_t(bi_t&& other) noexcept;
  explicit bi_t(const std::string&);
  explicit bi_t(const char*);

  ~bi_t() = default;

  // Assignment operators
  bi_t& operator=(const bi_t&);
  bi_t& operator=(bi_t&& other) noexcept;
  template <std::integral T>
  bi_t& operator=(T);

  // Unary operators
  bi_t operator+() const;
  bi_t operator-() const;

  // Increment and decrement
  bi_t& operator++();
  bi_t operator++(int);
  bi_t& operator--();
  bi_t operator--(int);

  // Multiplicative operators
  bi_t operator*(const bi_t&) const;
  bi_t operator/(const bi_t&) const;
  bi_t operator%(const bi_t&) const;
  bi_t& operator*=(const bi_t&);
  bi_t& operator/=(const bi_t&);
  bi_t& operator%=(const bi_t&);
  std::pair<bi_t, bi_t> div(const bi_t&) const;

  // Additive operators
  bi_t operator+(const bi_t&) const;
  bi_t operator-(const bi_t&) const;
  bi_t& operator+=(const bi_t&);
  bi_t& operator-=(const bi_t&);

  // Shift operators
  bi_t operator<<(bi_bitcount_t shift) const;
  bi_t operator>>(bi_bitcount_t shift) const;
  bi_t& operator<<=(bi_bitcount_t shift);
  bi_t& operator>>=(bi_bitcount_t shift);

  // Comparisons
  std::strong_ordering operator<=>(const bi_t& other) const noexcept;
  bool operator==(const bi_t&) const noexcept;

  // Bitwise operators
  bi_t operator~() const;
  bi_t operator&(const bi_t&) const;
  bi_t operator|(const bi_t&) const;
  bi_t operator^(const bi_t&) const;
  bi_t& operator&=(const bi_t&);
  bi_t& operator|=(const bi_t&);
  bi_t& operator^=(const bi_t&);

  // Comparison with integral types
  template <std::integral T>
  bool operator<(T) const noexcept;
  template <std::integral T>
  bool operator>(T) const noexcept;
  template <std::integral T>
  bool operator<=(T) const noexcept;
  template <std::integral T>
  bool operator>=(T) const noexcept;
  template <std::integral T>
  bool operator==(T) const noexcept;
  template <std::integral T>
  bool operator!=(T) const noexcept;

  // Conversion operators
  explicit operator bool() const noexcept;

  // Bits
  bi_bitcount_t bit_length() const noexcept;
  bool test_bit(bi_bitcount_t) const noexcept;
  bi_t& set_bit(bi_bitcount_t);

  // Accessors for internal representation
  size_t capacity() const noexcept;
  size_t size() const noexcept;
  bool negative() const noexcept;
  std::span<const digit> digits() const;
  void print_internal(std::ostream& os = std::cout) const noexcept;

  /* Other */
  // Swap: there is also a non-member swap function in the same namespace
  void swap(bi_t&) noexcept;

  // Integer to string
  std::string to_string() const;

  void negate() noexcept;
  int sign() const noexcept;
  bool odd() const noexcept;
  bool even() const noexcept;

  /* Friends */
  // Non-member operator overloads for relational and equality operators also
  // implemented for when T is on the LHS.
  template <std::integral T>
  friend bool operator<(T lhs, const bi_t& rhs) noexcept;
  template <std::integral T>
  friend bool operator>(T lhs, const bi_t& rhs) noexcept;
  template <std::integral T>
  friend bool operator<=(T lhs, const bi_t& rhs) noexcept;
  template <std::integral T>
  friend bool operator>=(T lhs, const bi_t& rhs) noexcept;
  template <std::integral T>
  friend bool operator==(T lhs, const bi_t& rhs) noexcept;
  template <std::integral T>
  friend bool operator!=(T lhs, const bi_t& rhs) noexcept;

  BI_API friend std::ostream& operator<<(std::ostream& os, const bi_t& x);

 private:
  digit_vector vec_;
  bool negative_;

  // vector modifiers
  void reserve_(size_t new_capacity);
  void resize_(size_t new_size);
  digit& operator[](size_t index);
  const digit& operator[](size_t index) const;
  void resize_unsafe_(size_t new_size);
  void trim_trailing_zeros() noexcept;

  // vector iterators
  auto begin() noexcept { return vec_.begin(); }
  auto begin() const noexcept { return vec_.begin(); }
  auto end() noexcept { return vec_.end(); }
  auto end() const noexcept { return vec_.end(); }
  auto rbegin() noexcept { return vec_.rbegin(); }
  auto rbegin() const noexcept { return vec_.rbegin(); }
  auto rend() noexcept { return vec_.rend(); }
  auto rend() const noexcept { return vec_.rend(); }

  // comparisons
  static int cmp_abs(const bi_t&, const bi_t&) noexcept;
  static int cmp(const bi_t&, const bi_t&) noexcept;
  template <std::integral T>
  static int cmp(const bi_t&, T) noexcept;

  // initializing
  template <std::integral T>
  static void init_one_digit(bi_t&, T);
  template <std::integral T>
  static void init_atleast_one_digit(bi_t&, T);
  void init_string(const std::string& str);

  // arithmetic
  static void sub_abs_gt(bi_t& result, const bi_t& a, const bi_t& b);
  static void add_abs(bi_t& result, const bi_t& a, const bi_t& b);
  static void sub_abs(bi_t& result, const bi_t& a, const bi_t& b);
  static void add(bi_t& result, const bi_t& a, const bi_t& b);
  static void sub(bi_t& result, const bi_t& a, const bi_t& b);
  static void mul(bi_t& result, const bi_t& a, const bi_t& b);
  void imul1add1(digit v, digit k);
  static void left_shift(bi_t& result, const bi_t& a, bi_bitcount_t shift);
  static void right_shift(bi_t& result, const bi_t& a, bi_bitcount_t shift);
  static void div_algo_binary(bi_t& q, bi_t& r, const bi_t& n, const bi_t& d);
  static void div_algo_single(bi_t& q, bi_t& r, const bi_t& n,
                              const bi_t& d) noexcept;
  static void div_algo_knuth(bi_t& q, bi_t& r, const bi_t& n, const bi_t& d);
  static void divide(bi_t& q, bi_t& r, const bi_t& n, const bi_t& d);

  enum class BitwiseOperation { AND, OR, XOR };
  template <BitwiseOperation Op>
  static void bitwise_operation_impl(bi_t& res, const bi_t& a, const bi_t& b);
  template <BitwiseOperation Op>
  static bi_t bitwise_operation(const bi_t& a, const bi_t& b);
  template <BitwiseOperation Op>
  bi_t& ibitwise_operation(const bi_t& other);

  // increment/decrement helpers
  void increment_abs();
  void decrement_abs();

  // to_string() helpers
  uint8_t idiv10() noexcept;
  size_t decimal_length() const;

  // misc.
  template <DigitIterator InputIterator>
  bi_t(InputIterator first, InputIterator last, bool negative = false);
};

BI_API std::ostream& operator<<(std::ostream&, const bi_t&);

BI_API void swap(bi_t& a, bi_t& b) noexcept;
BI_API bi_t operator"" _bi(const char* str);
BI_API bi_t abs(const bi_t& value);

/**
 *  @name Accessors for internal representation
 */
///@{

/// Return the number of digits the allocated storage can hold.
inline size_t bi_t::capacity() const noexcept { return vec_.capacity(); }

/// Return the number of digits used. Instance represents `0` iff `size() == 0`.
inline size_t bi_t::size() const noexcept { return vec_.size(); }

/**
 *  @brief Return a read-only span of the digits stored in the internal digit
 *  vector, with least significant digits first. If the integer is zero, an
 *  empty span is returned.
 *
 *  @warning Modifying the integer after the span is returned may invalidate the
 *  span.
 */
inline std::span<const digit> bi_t::digits() const {
  if (vec_.size() == 0) {
    return std::span<const digit>();
  }
  return std::span<const digit>(vec_.data(), vec_.size());
}

/// Reture `true` if the integer is (strictly) negative, false otherwise.
inline bool bi_t::negative() const noexcept { return negative_; }

///@}

/**
 *  @name Internal modifiers for the digit vector
 */
///@{

inline void bi_t::reserve_(size_t new_capacity) { vec_.reserve(new_capacity); }

inline void bi_t::resize_(size_t new_size) { vec_.resize(new_size); }

inline digit& bi_t::operator[](size_t index) { return vec_[index]; }

inline const digit& bi_t::operator[](size_t index) const { return vec_[index]; }

inline void bi_t::resize_unsafe_(size_t new_size) {
  assert(new_size <= vec_.capacity());
  vec_.resize_unsafe(new_size);
}

inline void bi_t::trim_trailing_zeros() noexcept {
  size_t new_size = vec_.size();
  while (new_size > 0 && vec_[new_size - 1] == 0) {
    --new_size;
  }
  vec_.resize(new_size);
  if (new_size == 0) {
    negative_ = false;
  }
}

///@}

template <std::integral T>
void bi_t::init_one_digit(bi_t& x, T value) {
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
void bi_t::init_atleast_one_digit(bi_t& x, T value) {
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

/**
 *  @brief Constructor for integral types.
 */
template <std::integral T>
bi_t::bi_t(T value) {
  using UnsignedT = typename std::make_unsigned<T>::type;

  if (value == 0) {
    vec_ = digit_vector();
    negative_ = false;
    return;
  }

  if constexpr (std::numeric_limits<T>::max() <= bi_dmax) {
    bi_t::init_one_digit(*this, value);
  } else {
    bi_t::init_atleast_one_digit(*this, value);
  }
}

/**
 *  @brief Constructs a `bi_t` from the range `[first, last)` (providing the
 *  digits of the integer, with least significant digits first) and a boolean
 *  flag indicating whether the number is negative (defaults to false).
 */
template <DigitIterator InputIterator>
bi_t::bi_t(InputIterator first, InputIterator last, bool negative)
    : vec_(first, last), negative_(negative) {
  trim_trailing_zeros();
}

/**
 *  @name Assignment operators
 */
///@{

/**
 *  @brief Assign a builtin integral type value to a `bi_t`.
 */
template <std::integral T>
bi_t& bi_t::operator=(T value) {
  using UnsignedT = typename std::make_unsigned<T>::type;

  UnsignedT uvalue;
  if (value == 0) {
    resize_(0);
    negative_ = false;
    return *this;
  } else if (value < 0) {
    uvalue = static_cast<UnsignedT>(0) - static_cast<UnsignedT>(value);
    negative_ = true;
  } else {
    uvalue = value;
    negative_ = false;
  }

  if constexpr (std::numeric_limits<T>::max() <= bi_dmax) {
    resize_(1);
    vec_[0] = static_cast<digit>(uvalue);
  } else {
    UnsignedT temp = uvalue;

    size_t n_digits = 1;
    while (temp >>= bi_dwidth) {
      ++n_digits;
    }

    resize_(n_digits);
    for (size_t i = 0; uvalue != 0; ++i) {
      vec_[i] = static_cast<digit>(uvalue);
      uvalue >>= bi_dwidth;
    }
  }

  return *this;
}

///@}

template <std::integral T>
int bi_t::cmp(const bi_t& a, T b) noexcept {
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
 *  @name Comparison with integral types
 *  @brief Efficient comparison between `bi_t`s and any standard integral type
 *  `T`, with no implicit conversions from `T`s to `bi_t`s (and hence no memory
 *  allocation).
 *  @note These apply for when a `bi_t` is on the LHS and a `T` is on the RHS.
 *  Non-member operator overloads for relational and equality operators are also
 *  implemented for when `T` is on the LHS. See friend functions.
 */
///@{

template <std::integral T>
bool bi_t::operator==(T rhs) const noexcept {
  return bi_t::cmp(*this, rhs) == 0;
}

template <std::integral T>
bool bi_t::operator!=(T rhs) const noexcept {
  return bi_t::cmp(*this, rhs) != 0;
}

template <std::integral T>
bool bi_t::operator<(T rhs) const noexcept {
  return bi_t::cmp(*this, rhs) < 0;
}

template <std::integral T>
bool bi_t::operator>(T rhs) const noexcept {
  return bi_t::cmp(*this, rhs) > 0;
}

template <std::integral T>
bool bi_t::operator<=(T rhs) const noexcept {
  return bi_t::cmp(*this, rhs) <= 0;
}

template <std::integral T>
bool bi_t::operator>=(T rhs) const noexcept {
  return bi_t::cmp(*this, rhs) >= 0;
}

///@}

/**
 *  @name Comparison with integral types: non-member operator overloads
 */
///@{

template <std::integral T>
bool operator==(T lhs, const bi_t& rhs) noexcept {
  return bi_t::cmp(rhs, lhs) == 0;
}

template <std::integral T>
bool operator!=(T lhs, const bi_t& rhs) noexcept {
  return bi_t::cmp(rhs, lhs) != 0;
}

template <std::integral T>
bool operator<(T lhs, const bi_t& rhs) noexcept {
  return bi_t::cmp(rhs, lhs) > 0;
}

template <std::integral T>
bool operator>(T lhs, const bi_t& rhs) noexcept {
  return bi_t::cmp(rhs, lhs) < 0;
}

template <std::integral T>
bool operator<=(T lhs, const bi_t& rhs) noexcept {
  return bi_t::cmp(rhs, lhs) >= 0;
}

template <std::integral T>
bool operator>=(T lhs, const bi_t& rhs) noexcept {
  return bi_t::cmp(rhs, lhs) <= 0;
}

///@}

/**
 *  @name Other
 */
///@{

/**
 *  @brief Swap the contents of this `bi_t` object with another `bi_t` object.
 *  @complexity O(1)
 */
inline void bi_t::swap(bi_t& other) noexcept {
  using std::swap;
  swap(this->vec_, other.vec_);
  swap(this->negative_, other.negative_);
}

/**
 *  @brief Return an `int` indicating the sign of the number: `-1` for negative,
 *  `0` for zero, `1` for positive.
 *  @complexity O(1)
 */
inline int bi_t::sign() const noexcept {
  if (size() == 0) {
    return 0;
  }
  return negative_ ? -1 : 1;
}

/**
 *  @brief Negates the integer in place.
 *  @complexity O(1)
 */
inline void bi_t::negate() noexcept {
  if (size() != 0) {
    negative_ = !negative_;
  }
}

/**
 *  @brief Return `true` if this integer is odd, `false` otherwise.
 *  @complexity O(1)
 */
inline bool bi_t::odd() const noexcept {
  return size() == 0 ? false : (*this)[0] & 1;
}

/**
 *  @brief Return `true` if this integer is even, `false` otherwise.
 *  @complexity O(1)
 */
inline bool bi_t::even() const noexcept { return !odd(); }

///@}

/// @private
inline digit_vector to_twos_complement(const digit_vector& vec) {
  const size_t vec_size = vec.size();
  digit_vector result;
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
inline void to_twos_complement_in_place(digit_vector& vec) noexcept {
  const size_t vec_size = vec.size();

  bool carry = true;
  for (size_t i = 0; i < vec_size; ++i) {
    const digit sum = ~vec[i] + carry;
    carry = sum < static_cast<uint8_t>(carry);
    vec[i] = sum;
  }
}

/// @private
template <bi_t::BitwiseOperation Op>
void bi_t::bitwise_operation_impl(bi_t& result, const bi_t& x, const bi_t& y) {
  size_t max_size = std::max(x.size(), y.size());

  const digit_vector& lhs_digits =
      x.negative_ ? to_twos_complement(x.vec_) : x.vec_;
  const digit_vector& rhs_digits =
      y.negative_ ? to_twos_complement(y.vec_) : y.vec_;

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
template <bi_t::BitwiseOperation Op>
bi_t bi_t::bitwise_operation(const bi_t& a, const bi_t& b) {
  bi_t result;
  bitwise_operation_impl<Op>(result, a, b);
  return result;
}

/// @private
template <bi_t::BitwiseOperation Op>
bi_t& bi_t::ibitwise_operation(const bi_t& other) {
  bitwise_operation_impl<Op>(*this, *this, other);
  return *this;
}

}  // namespace bi

#endif  // BI_INCLUDE_BI_HPP_
