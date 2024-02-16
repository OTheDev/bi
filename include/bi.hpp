/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_INCLUDE_BI_HPP_
#define BI_INCLUDE_BI_HPP_

#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <utility>

#if defined(_WIN32)
#if defined(BI_API_EXPORTS)
#define BI_API __declspec(dllexport)
#else
#define BI_API __declspec(dllimport)
#endif
#else
#define BI_API
#endif

// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
static_assert(sizeof(double) * CHAR_BIT == 64, "64-bit double is assumed.");
static_assert(-1 == ~0, "Two's complement representation assumed.");

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

class bi_t;

/* Public API */
BI_API std::ostream& operator<<(std::ostream&, const bi_t&);

BI_API void swap(bi_t& a, bi_t& b) noexcept;
BI_API bi_t operator"" _bi(const char* str);
BI_API bi_t abs(const bi_t& value);

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

  // Relational operators
  bool operator<(const bi_t&) const noexcept;
  bool operator>(const bi_t&) const noexcept;
  bool operator<=(const bi_t&) const noexcept;
  bool operator>=(const bi_t&) const noexcept;

  // Equality operators
  bool operator==(const bi_t&) const noexcept;
  bool operator!=(const bi_t&) const noexcept;

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

  // Other
  void swap(bi_t&) noexcept;
  std::string to_string() const;
  void negate() noexcept;
  int sign() const noexcept;
  bool odd() const noexcept;
  bool even() const noexcept;

  // Friends
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
  /// @private
  class dvector {
   public:
    using iterator = digit*;
    using const_iterator = const digit*;
    using riterator = std::reverse_iterator<digit*>;
    using const_riterator = std::reverse_iterator<const digit*>;

    ~dvector() noexcept;

    dvector() noexcept;
    dvector(const dvector& other);
    dvector(dvector&& other) noexcept;
    dvector& operator=(const dvector& other);
    dvector& operator=(dvector&& other) noexcept;

    template <typename DigitIterator>
    dvector(DigitIterator first, DigitIterator last);

    size_t size() const noexcept;
    size_t capacity() const noexcept;
    digit* data() noexcept;
    const digit* data() const noexcept;
    size_t max_size() const noexcept;

    digit& operator[](size_t index);
    const digit& operator[](size_t index) const;
    void resize(size_t new_size);
    void reserve(size_t new_capacity);
    void push_back(const digit& value);

    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    riterator rbegin() noexcept;
    const_riterator rbegin() const noexcept;
    riterator rend() noexcept;
    const_riterator rend() const noexcept;

    void resize_unsafe(size_t new_size);

   private:
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
    std::unique_ptr<digit[]> digits_;
    size_t size_{0};
    size_t capacity_{0};

    friend class h_;
  };

  digit& operator[](size_t index);
  const digit& operator[](size_t index) const;
  auto begin() noexcept;
  auto begin() const noexcept;
  auto end() noexcept;
  auto end() const noexcept;
  auto rbegin() noexcept;
  auto rbegin() const noexcept;
  auto rend() noexcept;
  auto rend() const noexcept;
  void reserve_(size_t);
  void resize_(size_t);

  void resize_unsafe_(size_t new_size);
  void trim_trailing_zeros() noexcept;

  /// @cond
  friend class h_;
  /// @endcond

  dvector vec_;
  bool negative_;
};

}  // namespace bi

#endif  // BI_INCLUDE_BI_HPP_
