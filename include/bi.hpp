/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_INCLUDE_BI_HPP_
#define BI_INCLUDE_BI_HPP_

#include <algorithm>
#include <cassert>
#include <climits>
#include <compare>
#include <iostream>
#include <span>
#include <string>
#include <utility>

#include "bi_digit_vector.impl.hpp"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
static_assert(sizeof(double) * CHAR_BIT == 64, "64-bit double is assumed.");
static_assert(-1 == ~0, "Two's complement representation assumed.");

#if defined(_WIN32)
#if defined(BI_API_EXPORTS)
#define BI_API __declspec(dllexport)
#else
#define BI_API __declspec(dllimport)
#endif
#else
#define BI_API
#endif

#if defined(BI_FORCE_64_BIT) && defined(__SIZEOF_INT128__)
#define BI_DIGIT_64_BIT
#elif defined(BI_FORCE_32_BIT)
#define BI_DIGIT_32_BIT
#else
#define BI_DIGIT_32_BIT
#endif

namespace bi {

#if defined(BI_DIGIT_64_BIT)
using digit = uint64_t;
#else
using digit = uint32_t;
#endif

using bi_bitcount_t = unsigned long;
using dvector = digit_vector<digit, bi_bitcount_t>;

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
  std::strong_ordering operator<=>(const bi_t&) const noexcept;
  bool operator==(const bi_t&) const noexcept;
  template <std::integral T>
  std::strong_ordering operator<=>(T) const noexcept;
  template <std::integral T>
  bool operator==(T) const noexcept;

  // Bitwise operators
  bi_t operator~() const;
  bi_t operator&(const bi_t&) const;
  bi_t operator|(const bi_t&) const;
  bi_t operator^(const bi_t&) const;
  bi_t& operator&=(const bi_t&);
  bi_t& operator|=(const bi_t&);
  bi_t& operator^=(const bi_t&);

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

  // Other
  void swap(bi_t&) noexcept;
  std::string to_string() const;
  void negate() noexcept;
  int sign() const noexcept;
  bool odd() const noexcept;
  bool even() const noexcept;

  // Friends
  BI_API friend std::ostream& operator<<(std::ostream& os, const bi_t& x);

 private:
  dvector vec_;
  bool negative_;

  void reserve_(size_t new_capacity);
  void resize_(size_t new_size);
  digit& operator[](size_t index);
  const digit& operator[](size_t index) const;
  void resize_unsafe_(size_t new_size);
  void trim_trailing_zeros() noexcept;

  auto begin() noexcept { return vec_.begin(); }
  auto begin() const noexcept { return vec_.begin(); }
  auto end() noexcept { return vec_.end(); }
  auto end() const noexcept { return vec_.end(); }
  auto rbegin() noexcept { return vec_.rbegin(); }
  auto rbegin() const noexcept { return vec_.rbegin(); }
  auto rend() noexcept { return vec_.rend(); }
  auto rend() const noexcept { return vec_.rend(); }

  /// @cond
  friend struct h_;
  /// @endcond
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

/// Reture `true` if the integer is (strictly) negative, false otherwise.
inline bool bi_t::negative() const noexcept { return negative_; }

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
 *  @brief Negates the integer in place.
 *  @complexity O(1)
 */
inline void bi_t::negate() noexcept {
  if (size() != 0) {
    negative_ = !negative_;
  }
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

}  // namespace bi

#endif  // BI_INCLUDE_BI_HPP_
