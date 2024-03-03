/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#include "bi.hpp"

#include <compare>
#include <iostream>
#include <version>
#if defined(__cpp_lib_format)
#include <format>
#define HAS_STD_FORMAT
#endif

#include "h_.hpp"
#include "inst_integral.hpp"
#include "uints.hpp"

namespace bi {

/**
 *  @class bi_t
 *  @headerfile "bi.hpp"
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
 *  @throw bi::from_float Throws when attempting to convert a NaN or infinity to
 *  a `bi_t`.
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
 *  constrained to be less than or equal to `SIZE_MAX / sizeof(digit)`.
 *  Additionally, because counts of bits use the `unsigned long` type, we also
 *  constrain the maximum size of the vector to be less than or equal to
 *  `ULONG_MAX / (CHAR_BIT * sizeof(digit))`, where the denominator is the
 *  number of bits in the digit data type (typically 32). Together, these
 *  constraints determine `max_size()` for the internal digit vector. An
 *  operation that expects a result to exceed `max_size()` throws
 *  `bi::overflow_error`.
 */

/**
 *  @brief Default constructor. The integer is initialized to zero and no memory
 *  allocation occurs.
 *  @complexity O(1)
 */
bi_t::bi_t() noexcept : negative_(false), vec_() {}

/**
 *  @brief Constructor for integral types.
 */
template <std::integral T>
bi_t::bi_t(T value) {
  using UnsignedT = typename std::make_unsigned<T>::type;

  if (value == 0) {
    vec_ = dvector();
    negative_ = false;
    return;
  }

  if constexpr (std::numeric_limits<T>::max() <= bi_dmax) {
    h_::init_one_digit(*this, value);
  } else {
    h_::init_atleast_one_digit(*this, value);
  }
}

/// Copy constructor.
bi_t::bi_t(const bi_t& other) : vec_(other.vec_), negative_(other.negative_) {}

/**
 *  @brief Move constructor. After the move, `other` is left in a valid state
 *  representing the integer 0.
 */
bi_t::bi_t(bi_t&& other) noexcept
    : vec_(std::move(other.vec_)), negative_(other.negative_) {
  other.negative_ = false;
}

/**
 *  @name Construct from a string
 *  @brief Construct an integer from a string representing a base-`base`
 *  integer. These are `explicit` constructors.
 *  @throw std::invalid_argument Throws if a parsing error occurs, if a null
 *  pointer is provided, or if an invalid base is provided.
 *  @details Allows leading whitespace and/or a plus/minus sign before the first
 *  base-`base` digit. `base` must be an integer in \f$ [2, 36] \f$ (by default,
 *  it is `10`).
 *
 *  Examples:
 *  @code
 *  bi_t a{"98765"};      // 98765
 *  bi_t b{"-98765"};     // -98765
 *  bi_t c{"  -6789"};    // -6789
 *  bi_t empty{""};       // Throws std::invalid_argument
 *  bi_t nll{nullptr};    // Throws std::invalid_argument
 *  bi_t invalid{"  -"};  // Throws std::invalid_argument
 *  @endcode
 */
///@{

// clang-tidy says the "constructor does not initialize these fields: negative_"
// but init_string() does. We can ignore this warning.
// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init)

bi_t::bi_t(const std::string& s, int base) {
  // vec_ implicitly default-initialized
  h_::init_string(*this, s, base);
}

bi_t::bi_t(const char* s, int base) {
  if (s == nullptr) {
    throw std::invalid_argument("Null string pointer provided.");
  }
  h_::init_string(*this, std::string(s), base);
}

bi_t::bi_t(double d) { h_::assign_from_double(*this, d); }

// NOLINTEND(cppcoreguidelines-pro-type-member-init)

///@}

/**
 *  @name Assignment operators
 */
///@{

/// Copy assignment operator.
bi_t& bi_t::operator=(const bi_t& other) {
  if (this != &other) {
    vec_ = other.vec_;
    negative_ = other.negative_;
  }
  return *this;
}

/**
 *  @brief Move assignment operator. After the move, `other` is left in a valid
 *  state representing the integer 0.
 */
bi_t& bi_t::operator=(bi_t&& other) noexcept {
  if (this != &other) {
    vec_ = std::move(other.vec_);
    negative_ = other.negative_;
    other.negative_ = false;
  }
  return *this;
}

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

bi_t& bi_t::operator=(const std::string& s) {
  h_::init_string(*this, s);
  return *this;
}

bi_t& bi_t::operator=(const char* s) {
  if (s == nullptr) {
    throw std::invalid_argument("Null string pointer provided.");
  }
  h_::init_string(*this, std::string(s));
  return *this;
}

bi_t& bi_t::operator=(double d) {
  h_::assign_from_double(*this, d);
  return *this;
}

///@}

/**
 *  @name Unary operators
 */
///@{

bi_t bi_t::operator+() const { return *this; }

bi_t bi_t::operator-() const {
  bi_t ret(*this);
  if (size() != 0) {
    ret.negative_ = !this->negative_;
  }
  return ret;
}

///@}

/**
 *  @name Increment and decrement
 */
///@{

bi_t& bi_t::operator++() {
  // return *this += bi_t{1};
  if (!negative()) {
    h_::increment_abs(*this);
  } else {
    h_::decrement_abs(*this);
    if (size() == 0) {
      negative_ = false;
    }
  }
  return *this;
}

bi_t bi_t::operator++(int) {
  bi_t temp{*this};
  ++(*this);
  return temp;
}

bi_t& bi_t::operator--() {
  // return *this -= bi_t{1};
  if (!negative()) {
    h_::decrement_abs(*this);
  } else {
    h_::increment_abs(*this);
  }
  return *this;
}

bi_t bi_t::operator--(int) {
  bi_t temp{*this};
  --(*this);
  return temp;
}

///@}

/**
 *  @name Multiplicative operators
 *  @throw bi::division_by_zero Throws if a division by zero attempt is
 *  detected.
 */
///@{

/// @complexity \f$ O(n^{\log_{2}(3)}) \approx O(n^{1.58}) \f$
bi_t bi_t::operator*(const bi_t& other) const {
  bi_t ret;
  h_::mul(ret, *this, other);
  return ret;
}

/// @complexity \f$ O(m \cdot n) \f$
bi_t bi_t::operator/(const bi_t& other) const {
  bi_t quot, rem;
  h_::divide(quot, rem, *this, other);
  return quot;
}

/// @complexity \f$ O(m \cdot n) \f$
bi_t bi_t::operator%(const bi_t& other) const {
  bi_t quot, rem;
  h_::divide(quot, rem, *this, other);
  return rem;
}

/// @complexity \f$ O(n^{\log_{2}(3)}) \approx O(n^{1.58}) \f$
bi_t& bi_t::operator*=(const bi_t& other) {
  h_::mul(*this, *this, other);
  return *this;
}

/// @complexity \f$ O(m \cdot n) \f$
bi_t& bi_t::operator/=(const bi_t& other) {
  bi_t quot, rem;
  h_::divide(quot, rem, *this, other);
  swap(quot);
  return *this;
}

/// @complexity \f$ O(m \cdot n) \f$
bi_t& bi_t::operator%=(const bi_t& other) {
  bi_t quot, rem;
  h_::divide(quot, rem, *this, other);
  swap(rem);
  return *this;
}

/**
 *  @brief Computes both the quotient and remainder of division of this `bi_t`
 *  object by another `bi_t` object and returns both the quotient and the
 *  remainder as a pair.
 *  @param other The divisor.
 *  @return A pair of `bi_t` objects where the `first` element is the quotient
 *  and the `second` element is the remainder.
 *  @complexity \f$ O(m \cdot n) \f$ where \f$ m \f$ is the `size()` of the
 *  dividend and \f$ n \f$ is the `size()` of the divisor.
 */
std::pair<bi_t, bi_t> bi_t::div(const bi_t& other) const {
  bi_t quot, rem;
  h_::divide(quot, rem, *this, other);
  return std::make_pair(std::move(quot), std::move(rem));
}

///@}

/**
 *  @name Additive operators
 */
///@{

/// @complexity \f$ O(n) \f$
bi_t bi_t::operator+(const bi_t& other) const {
  bi_t ret;
  h_::add(ret, *this, other);
  return ret;
}

/// @complexity \f$ O(n) \f$
bi_t bi_t::operator-(const bi_t& other) const {
  bi_t ret;
  h_::sub(ret, *this, other);
  return ret;
}

/// @complexity \f$ O(n) \f$
bi_t& bi_t::operator+=(const bi_t& other) {
  h_::add(*this, *this, other);
  return *this;
}

/// @complexity \f$ O(n) \f$
bi_t& bi_t::operator-=(const bi_t& other) {
  h_::sub(*this, *this, other);
  return *this;
}

///@}

/**
 *  @name Shift operators
 *  @note Prior to the C++20 Standard, the behavior of `<<` and `>>` is
 *  undefined or implementation-defined if the left operand is negative. In
 *  contrast, the C++20 Standard defines the behavior of `<<` and `>>` for both
 *  negative and nonnegative left operands. This implementation conforms to the
 *  C++20 Standard (except for any wrap-around behavior, as the `bi_t` is not a
 *  fixed-width integer) and makes the result of `<<` equivalent to  \f$ x
 *  \times 2^{shift} \f$ and the result of `>>` equivalent to \f$ \left\lfloor
 *  \frac{x}{2^{\text{shift}}} \right\rfloor \f$.
 *  @param shift An `unsigned long` representing the number of bit positions to
 *  shift by.
 */
///@{

/**
 *  @brief Return a new integer representing the integer left-shifted `shift`
 *  bit positions with vacated bits zero-filled.
 *  @details Mathematically, the value of the resulting integer is
 *  \f[
 *    x \times 2^{\text{shift}}
 *  \f]
 *  where \f$ x \f$ is the big integer.
 */
bi_t bi_t::operator<<(bi_bitcount_t shift) const {
  bi_t ret;
  h_::left_shift(ret, *this, shift);
  return ret;
}

/**
 *  @brief Return a new integer representing the integer right-shifted `shift`
 *  bit positions. This is an arithmetic right shift with sign extension.
 *  @details Mathematically, the value of the resulting integer is \f$
 *  \frac{x}{2^{\text{shift}}} \f$, rounded down:
 *  \f[
 *    \left\lfloor \frac{x}{2^{\text{shift}}} \right\rfloor
 *  \f]
 *  where \f$ x \f$ is the big integer.
 */
bi_t bi_t::operator>>(bi_bitcount_t shift) const {
  bi_t ret;
  h_::right_shift(ret, *this, shift);
  return ret;
}

bi_t& bi_t::operator<<=(bi_bitcount_t shift) {
  h_::left_shift(*this, *this, shift);
  return *this;
}

bi_t& bi_t::operator>>=(bi_bitcount_t shift) {
  h_::right_shift(*this, *this, shift);
  return *this;
}

///@}

/**
 *  @name Comparisons
 */
///@{

std::strong_ordering bi_t::operator<=>(const bi_t& other) const noexcept {
  return h_::cmp(*this, other) <=> 0;
}

bool bi_t::operator==(const bi_t& other) const noexcept {
  return h_::cmp(*this, other) == 0;
}

///@}

/**
 *  @name Comparisons with integral types
 *  @brief Efficient comparison between `bi_t`s and any standard integral type
 *  `T`, with no implicit conversions from `T`s to `bi_t`s (and hence no memory
 *  allocation).
 */
///@{

template <std::integral T>
std::strong_ordering bi_t::operator<=>(T rhs) const noexcept {
  return h_::cmp(*this, rhs) <=> 0;
}

template <std::integral T>
bool bi_t::operator==(T rhs) const noexcept {
  return h_::cmp(*this, rhs) == 0;
}

///@}

/**
 *  @name Comparisons with floats
 *  @brief Compare a `bi_t` to a floating-point number.
 *
 *  @details
 *  These member functions handle the case where a `bi_t` is on the left-hand
 *  side (LHS) and a `double` is on the right-hand side (RHS). There are also
 *  non-member functions that facilitate comparisons where the `double` is on
 *  the LHS.
 *
 *  These comparisons are designed to be consistent with IEEE 754, handling
 *  special cases like NaNs and infinities.
 *
 *  **NaN**:
 *  - Operators `==`, `<`, `<=`, `>`, and `>=` return `false` when comparing
 *    with NaN.
 *  - Operator `!=` returns `true` when comparing with NaN.
 *
 *  **Infinities**:
 *  - Positive infinity is treated as greater than any `bi_t` number.
 *  - Negative infinity is treated as less than any `bi_t` number.
 *
 *  @note The spaceship operator (`<=>`) is not used here due to the unordered
 *  nature of NaN in IEEE 754. Instead, each comparison operator is explicitly
 *  defined to handle NaN appropriately. In comparisons of `bi_t`s with `bi_t`s
 *  and `bi_t`s with integral types, the spaceship operator is used, which,
 *  combined with `operator==`, implicitly allows comparisons using `==`,
 *  `!=`, `<`, `<=`, `>`, and `>=`, both when a `bi_t` is on the LHS and on the
 *  RHS. Here, in order to handle NaN appropriately, we explicitly define all
 *  of `==`, `!=`, `<`, `<=`, `>`, and `>=`, both for when a `bi_t` is on the
 *  LHS and RHS.
 */
///@{

// Only these two needed if we leave comparisons undefined for NaN and `bi_t`
// std::strong_ordering bi_t::operator<=>(double other) const noexcept {
//   return h_::cmp(*this, other) <=> 0;
// }

// bool bi_t::operator==(double other) const noexcept {
//   return h_::cmp(*this, other) == 0;
// }

/// @complexity O(1)
bool bi_t::operator==(double other) const noexcept {
  return !std::isnan(other) && h_::cmp(*this, other) == 0;
}

/// @complexity O(1)
bool bi_t::operator!=(double other) const noexcept { return !(*this == other); }

/// @complexity O(1)
bool bi_t::operator<(double other) const noexcept {
  return !std::isnan(other) && h_::cmp(*this, other) < 0;
}

/// @complexity O(1)
bool bi_t::operator<=(double other) const noexcept {
  return !std::isnan(other) && h_::cmp(*this, other) <= 0;
}

/// @complexity O(1)
bool bi_t::operator>(double other) const noexcept {
  return !std::isnan(other) && h_::cmp(*this, other) > 0;
}

/// @complexity O(1)
bool bi_t::operator>=(double other) const noexcept {
  return !std::isnan(other) && h_::cmp(*this, other) >= 0;
}

///@}

bool operator==(double lhs, const bi_t& rhs) noexcept { return rhs == lhs; }
bool operator!=(double lhs, const bi_t& rhs) noexcept { return rhs != lhs; }
bool operator<(double lhs, const bi_t& rhs) noexcept { return rhs > lhs; }
bool operator<=(double lhs, const bi_t& rhs) noexcept { return rhs >= lhs; }
bool operator>(double lhs, const bi_t& rhs) noexcept { return rhs < lhs; }
bool operator>=(double lhs, const bi_t& rhs) noexcept { return rhs <= lhs; }

/**
 *  @name Bitwise operators
 *  @note These operators perform bitwise operations on integers using two's
 *  complement representation (with sign extension).
 */
///@{

/**
 *  @brief Unary complement operator. Return a new integer representing the
 *  ones' complement of this integer.
 */
bi_t bi_t::operator~() const { return -(*this) - 1; }

/**
 *  Mathematically, given two integers \f$ x, y \f$ with coefficients \f$ x_{i},
 *  y_{i} \f$ of their binary representation, the result of \f$ x \; \& \; y \f$
 *  (bitwise AND) is an integer \f$ r \f$ with base-2 coefficients \f$ r_{i} \f$
 *  such that
 *  \f[
 *    r_{i} = 1 \Longleftrightarrow x_{i} = 1 \wedge y_{i} = 1
 *  \f]
 */
bi_t bi_t::operator&(const bi_t& other) const {
  return h_::bitwise_operation<h_::BitwiseOperation::AND>(*this, other);
}

/**
 *  Mathematically, given two integers \f$ x, y \f$ with coefficients \f$ x_{i},
 *  y_{i} \f$ of their binary representation, the result of \f$ x \; | \; y \f$
 *  (bitwise inclusive OR) is an integer \f$ r \f$ with base-2 coefficients \f$
 *  r_{i} \f$ such that
 *  \f[
 *    r_{i} = 1 \Longleftrightarrow x_{i} = 1 \lor y_{i} = 1
 *  \f]
 */
bi_t bi_t::operator|(const bi_t& other) const {
  return h_::bitwise_operation<h_::BitwiseOperation::OR>(*this, other);
}

/**
 *  Mathematically, given two integers \f$ x, y \f$ with coefficients \f$ x_{i},
 *  y_{i} \f$ of their binary representation, the result of \f$ x \; ^\wedge \;
 *  y \f$ (bitwise exclusive OR) is an integer \f$ r \f$ with base-2
 *  coefficients \f$ r_{i} \f$ such that
 *  \f[
 *    r_{i} = 1 \Longleftrightarrow (x_{i} = 1 \wedge y_{i} = 0) \lor
 *                                  (x_{i} = 0 \wedge y_{i} = 1)
 *  \f]
 */
bi_t bi_t::operator^(const bi_t& other) const {
  return h_::bitwise_operation<h_::BitwiseOperation::XOR>(*this, other);
}

bi_t& bi_t::operator&=(const bi_t& other) {
  return h_::ibitwise_operation<h_::BitwiseOperation::AND>(*this, other);
}

bi_t& bi_t::operator|=(const bi_t& other) {
  return h_::ibitwise_operation<h_::BitwiseOperation::OR>(*this, other);
}

bi_t& bi_t::operator^=(const bi_t& other) {
  return h_::ibitwise_operation<h_::BitwiseOperation::XOR>(*this, other);
}

///@}

/**
 *  @name Conversion operators
 */
///@{

/**
 *  @brief `true` if and only if this integer is nonzero.
 *
 *  Example:
 *  @code
 *  bi_t x;
 *  if (x) {
 *    std::cout << "x is nonzero!" << std::endl;
 *  } else {
 *    std::cout << "x is zero!" << std::endl;
 *  }
 *  @endcode
 *  The output is
 *  @code
 *  x is zero!
 *  @endcode
 *  @complexity O(1)
 */
bi_t::operator bool() const noexcept { return size() == 0 ? false : true; }

/**
 *  @brief Converts a `bi_t` object to an integral type T.
 *
 *  As with the C++20 Standard (7.3.8, p. 93):
 *  > [T]he result is the unique value of the destination type that is congruent
 *  > to the source integer modulo \f$ 2^{N} \f$, where \f$ N \f$ is the width
 *  > of the destination type.
 */
template <std::integral T>
bi_t::operator T() const noexcept {
  if (size() == 0) {
    return 0;
  }

  constexpr unsigned t_bits = CHAR_BIT * sizeof(T);
  constexpr bool t_is_signed = std::is_signed<T>::value;

  T ret = 0;
  if constexpr (t_bits > bi_dbits) {
    constexpr size_t max_digits_for_t = uints::div_ceil(t_bits, bi_dbits);
    size_t n_digits = std::min(size(), max_digits_for_t);

    for (size_t i = n_digits - 1; i < SIZE_MAX; --i) {
      ret = (ret << bi_dbits) | (*this)[i];
    }
  } else {
    ret = (*this)[0];
  }

  if constexpr (t_is_signed) {
    if (negative()) {
      ret = -ret;
    }
  } else {
    if (negative()) {
      ret = ~ret + 1;
    }
  }

  return ret;
}

bi_t::operator double() const noexcept {
  if (size() == 0) {
    return 0.0;
  }

  double result = 0.0;

  for (size_t i = size(); i-- > 0;) {
    // First operand to addition same as `std::ldexp(result, bi_dbits)`
    result = result * bi_base_dbl + static_cast<double>((*this)[i]);
  }

  if (negative_) {
    result = -result;
  }

  return result;
}

///@}

/**
 *  @name Bits
 */
///@{

/**
 *  @brief If nonzero, return the number of bits required to represent its
 *  absolute value. Otherwise (i.e. the integer is zero), return `0`.
 */
bi_bitcount_t bi_t::bit_length() const noexcept {
  if (size() == 0) {
    return 0;
  }

  return (size() - 1) * static_cast<bi_bitcount_t>(bi_dbits) +
         uints::bit_length(vec_[size() - 1]);
}

/// Test bit `i`, acting as if the integer is nonnegative.
bool bi_t::test_bit(bi_bitcount_t i) const noexcept {
  const bi_bitcount_t digit_idx = i / bi_dbits;

  if (size() <= digit_idx) {
    return 0;
  }

  return (vec_[digit_idx] >> (i % bi_dbits)) & 1;
}

/**
 *  @brief Set bit `i`, acting as if the integer is nonnegative, but preserving
 *  its original sign in the result.
 */
bi_t& bi_t::set_bit(bi_bitcount_t i) {
  const size_t n = size();
  const bi_bitcount_t digit_idx = i / bi_dbits;

  if (digit_idx < n) {
    vec_[digit_idx] |= static_cast<digit>(1) << (i % bi_dbits);
  } else {
    resize_(digit_idx + 1);
    std::fill_n(&vec_[n], digit_idx - n, 0);

    vec_[digit_idx] = static_cast<digit>(1) << (i % bi_dbits);
  }
  return *this;
}

///@}

/**
 *  @name Accessors for internal representation
 */
///@{

/**
 *  @fn size_t bi_t::capacity() const noexcept
 *  @brief Return the number of digits the allocated storage can hold.
 *  @complexity O(1)
 */

/**
 *  @fn size_t bi_t::size() const noexcept
 *  @brief Return the number of digits used. Instance represents `0` iff
 *  `size() == 0`.
 *  @complexity O(1)
 */

/**
 *  @fn bool bi_t::negative() const noexcept
 *  @brief Return `true` if the integer is (strictly) negative, false otherwise.
 *  @complexity O(1)
 */

/**
 *  @fn std::span<const digit> bi_t::digits() const
 *  @brief Return a read-only span of the digits stored in the internal digit
 *  vector, with least significant digits first. If the integer is zero, an
 *  empty span is returned.
 *
 *  @warning Modifying the integer after the span is returned may invalidate the
 *  span.
 *  @complexity O(1)
 */

/**
 *  Prints the integer in the form
 *  @code
 *  (d_p * 2**(bi_dbits * p) + ... + d_0 * 2**(bi_dbits * 0))
 *  @endcode
 *  followed by a newline. Outputs to the specified `std::ostream` or to
 *  standard output (`std::cout`) by default. If the integer is negative, the
 *  output will be preceded by a minus sign (`-`). `bi_dbits` is typically
 *  32.
 *
 *  Useful for understanding the internal representation of the integer.
 *
 *  @param os The output stream to which the internal representation is printed.
 *  If not specified, defaults to standard output (`std::cout`).
 *  @complexity O(n)
 */
void bi_t::print_internal(std::ostream& os) const noexcept {
  if (size() == 0) {
#if defined(HAS_STD_FORMAT)
    os << std::format("0 * 2**({} * 0)\n", bi_dwidth);
#else
    os << "0 * 2**(" << bi_dwidth << " * 0)\n";
#endif
    return;
  }

  if (negative()) {
    os << '-';
  }

  const size_t i = size() - 1;
#if defined(HAS_STD_FORMAT)
  os << std::format("({} * 2**({} * {})", vec_[i], bi_dwidth, i);
#else
  os << "(" << vec_[i] << " * 2**(" << bi_dwidth << " * " << i << ")";
#endif
  for (size_t j = i; j-- > 0;) {
#if defined(HAS_STD_FORMAT)
    os << std::format(" + {} * 2**({} * {})", vec_[j], bi_dwidth, j);
#else
    os << " + " << vec_[j] << " * 2**(" << bi_dwidth << " * " << j << ")";
#endif
  }

  os << ")\n";
}

///@}

/**
 *  @name Other
 */
///@{

/**
 *  @fn void bi_t::swap(bi_t& other) noexcept
 *  @brief Swap the contents of this `bi_t` object with another `bi_t` object.
 *  @complexity O(1)
 */

/**
 *  @brief Return a `string` containing the base-`base` representation of the
 *  integer, where `base` must be an integer in \f$ [2, 36] \f$ (by default,
 *  `base` is `10`).
 *
 *  Examples:
 *  @code
 *  bi_t x;
 *  std::string s = x.to_string();  // s == "0"
 *  x = 65535;
 *  s = x.to_string();              // s == "65535"
 *  x = -32768;
 *  s = x.to_string();              // s == "-32768"
 *  @endcode
 */
std::string bi_t::to_string(int base) const {
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  static constexpr auto base_digits = "0123456789abcdefghijklmnopqrstuvwxyz";

  if (base <= 1 || base > 36) {
    throw std::invalid_argument("base argument must be in [2, 36]");
  }

  if (size() == 0) {
    return "0";
  }

  bi_t copy = *this;
  std::string result;
  const size_t estimate = h_::base_length(*this, base);
  result.reserve(estimate + negative_);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  const auto [max_batch_size, divisor] = base_mbs[base];

  while (copy.size()) {
    digit remainder = h_::div_algo_digit(copy, copy, divisor);

    for (unsigned i = 0; i < max_batch_size; ++i) {
      if (remainder == 0 && copy.size() == 0) {
        break;
      }

      digit current_digit = remainder % base;
      remainder /= base;

      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      result.push_back(base_digits[current_digit]);
    }
  }

  if (negative_) {
    result.push_back('-');
  }

  std::reverse(result.begin(), result.end());

  return result;
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

/**
 *  @fn void bi_t::negate() noexcept
 *  @brief Negates the integer in place.
 *  @complexity O(1)
 */

/**
 *  @fn int bi_t::sign() const noexcept
 *  @brief Return an `int` indicating the sign of the number: `-1` for negative,
 *  `0` for zero, `1` for positive.
 *  @complexity O(1)
 */

/**
 *  @fn bool bi_t::odd() const noexcept
 *  @brief Return `true` if this integer is odd, `false` otherwise.
 *  @complexity O(1)
 */

/**
 *  @fn bool bi_t::even() const noexcept
 *  @brief Return `true` if this integer is even, `false` otherwise.
 *  @complexity O(1)
 */

/**
 *  @brief Return `true` if this integer fits in an integral type T, `false`
 *  otherwise.
 *
 *  For `bi_t` object `x` and integral type `T`, `x.within<T>()` evaluates to
 *  @code
 *  x >= std::numeric_limits<T>::min() && x <= std::numeric_limits<T>::max()
 *  @endcode
 *
 *  Example:
 *  @code{.cpp}
 *  bi_t x = std::numeric_limits<int32_t>::max();
 *  bool fits_in_int32 = x.within<int32_t>();  // true
 *  bool fits_in_int16 = x.within<int16_t>();  // false
 *  @endcode
 */
template <std::integral T>
bool bi_t::within() const noexcept {
  return (*this) >= std::numeric_limits<T>::min() &&
         (*this) <= std::numeric_limits<T>::max();
}

///@}

/**
 *  @name Exponentiation
 */
///@{

bi_t bi_t::pow(const bi_t& base, bi_bitcount_t exp) {
  if (exp == 0) {
    return 1;
  }
  // Guard against attempts to exponentiate when we know it will lead to
  // bi::overflow_error. If base == 2 and exp > 0, the result of 2 ** exp will
  // have a bit length of exp + 1 bits. Big integers are constrained such that
  // their bit length is less than or equal to bi::max_bits. Thus, we require
  // that (exp + 1 <= bi::max_bits) <==> (exp <= bi::max_bits - 1).
  if (exp >= bi::max_bits) {
    if (base == 0 || base == 1) {
      return base;
    }
    if (base == -1) {
      return exp % 2 == 0 ? 1 : -1;
    }
    throw overflow_error("");
  }
  return h_::expo_left_to_right(base, exp);
}

bi_t bi_t::pow(const bi_t& base, const bi_t& exp) {
  if (exp < 0) {
    throw std::invalid_argument("Negative exponents are not supported.");
  }
  if (exp == 0) {
    return 1;
  }
  if (exp >= bi::max_bits) {
    if (base == 0 || base == 1) {
      return base;
    }
    if (base == -1) {
      return exp.even() ? 1 : -1;
    }
    throw overflow_error("");
  }
  return h_::expo_left_to_right(base, static_cast<bi_bitcount_t>(exp));
}

///@}

/**
 *  @brief Outputs the base-10 (decimal) representation of a `bi_t` object to
 *  a standard output stream.
 *
 *  This overload of the stream insertion operator allows instances of `bi_t`
 *  to be sent to standard output streams like `std::cout`. The `bi_t` object
 *  is converted to a string via `to_string()` before being output.
 *
 *  @param os The output stream to which the data is sent.
 *  @param x The `bi_t` object to be output.
 *  @return A reference to the output stream, to support chaining of `<<`
 *  operators.
 */
std::ostream& operator<<(std::ostream& os, const bi_t& x) {
  return os << x.to_string();
}

/**
 *  @brief Swap the contents of `a` with `b`.
 *  @relates bi_t
 *  @complexity O(1)
 */
void swap(bi_t& a, bi_t& b) noexcept { a.swap(b); }

/**
 *  @brief User-defined literal (UDL) operator for creating `bi_t` objects.
 *  @return `bi_t(s)`.
 *  @relates bi_t
 *
 *  Example:
 *  @code
 *  // Equivalent to bi_t fib{"123581321345589144"};
 *  bi_t fib = 123581321345589144_bi;
 *  @endcode
 */
bi_t operator"" _bi(const char* s) { return bi_t(s); }

/**
 *  @brief Return a new integer representing the absolute value of the argument
 *  integer.
 *  @relates bi_t
 */
bi_t abs(const bi_t& value) {
  if (value.negative()) {
    bi_t result = value;
    result.negate();
    return result;
  }
  return value;
}

/// @cond
BI_INST_TEMPLATE_FOR_INTEGRAL_TYPES(bi_t::bi_t, BI_EMPTY);
BI_INST_TEMPLATE_FOR_INTEGRAL_TYPES(bi_t& bi_t::operator=, BI_EMPTY);
BI_INST_TEMPLATE_FOR_INTEGRAL_TYPES(std::strong_ordering bi_t::operator<=>,
                                    const noexcept);
BI_INST_TEMPLATE_FOR_INTEGRAL_TYPES(bool bi_t::operator==, const noexcept);

BI_INST_TEMPLATE_FOR_INTEGRAL_TYPES_CONV(bi_t::operator, const noexcept);

BI_INST_TEMPLATE_FOR_INTEGRAL_TYPES_NOARG(bool bi_t::within, const noexcept);
/// @endcond

};  // namespace bi
