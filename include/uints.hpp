/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_INCLUDE_UINTS_HPP_
#define BI_INCLUDE_UINTS_HPP_

#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

namespace uints {

template <std::unsigned_integral T, std::integral U = bool>
void uaddc(T& r, T a, T b, U& carry) noexcept {
  T temp = a + carry;
  r = b + temp;
  carry = (r < b) || (temp < a);
}

template <std::unsigned_integral T, std::integral U = bool>
void usubb(T& r, T a, T b, U& borrow) noexcept {
  T temp = a - b;
  r = temp - borrow;
  borrow = (r > temp) || (temp > a);
}

template <std::unsigned_integral T>
bool uadd_overflow(T& r, T a, T b) noexcept {
  r = a + b;
  return r < a;
}

template <std::unsigned_integral T>
bool usub_overflow(T& r, T a, T b) noexcept {
  r = a - b;
  return a < b;
}

template <std::unsigned_integral T>
bool umul_overflow(T& r, T a, T b) noexcept {
  r = a * b;
  return a && r / a != b;
}

template <std::unsigned_integral T, std::integral U = bool>
void uadd_overflow(T& result, T a, T b, U& overflow) noexcept {
  overflow = uadd_overflow(result, a, b);
}

template <std::unsigned_integral T, std::integral U = bool>
void usub_overflow(T& result, T a, T b, U& overflow) noexcept {
  overflow = usub_overflow(result, a, b);
}

template <std::unsigned_integral T, std::integral U = bool>
void umul_overflow(T& result, T a, T b, U& overflow) noexcept {
  overflow = umul_overflow(result, a, b);
}

template <std::unsigned_integral T>
std::pair<T, bool> uadd_overflow(T a, T b) noexcept {
  T r = a + b;
  return {r, r < a};
}

template <std::unsigned_integral T>
std::pair<T, bool> usub_overflow(T a, T b) noexcept {
  return {a - b, a < b};
}

template <std::unsigned_integral T>
std::pair<T, bool> umul_overflow(T a, T b) noexcept {
  T r = a * b;
  return {r, a && r / a != b};
}

/**
 *  @brief Return the number of bits required to represent a value of type T,
 *  where T is an unsigned integral type.
 */
template <std::unsigned_integral T>
uint8_t bit_length(T number) noexcept {
  constexpr size_t width = sizeof(T) * CHAR_BIT;
  static_assert(width <= std::numeric_limits<uint8_t>::max());

  uint8_t n_bits = 1;

  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  if constexpr (width == 32 || width == 64) {
    if constexpr (width == 64) {
      constexpr T mask64_upper = 0xffffffff00000000;

      if ((number & mask64_upper) != 0) {
        number >>= 32;
        n_bits += 32;
      }
    }

    constexpr T mask32_upper = 0xffff0000;
    constexpr T mask16_upper = 0xff00;
    constexpr T mask8_upper = 0xf0;
    constexpr T mask4_upper = 0xc;
    constexpr T mask2_upper = 0x2;

    if ((number & mask32_upper) != 0) {
      number >>= 16;
      n_bits += 16;
    }
    if ((number & mask16_upper) != 0) {
      number >>= 8;
      n_bits += 8;
    }
    if ((number & mask8_upper) != 0) {
      number >>= 4;
      n_bits += 4;
    }
    if ((number & mask4_upper) != 0) {
      number >>= 2;
      n_bits += 2;
    }
    if ((number & mask2_upper) != 0) {
      n_bits += 1;
    }
  } else {
    while (number >>= 1) {
      n_bits++;
    }
  }
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

  return n_bits;
}

/**
 *  Any integer with absolute value less than 2 ** 53 can be exactly represented
 *  in an IEEE 754 double.
 *
 *  An n-bit unsigned integer can represent values in the range [0, 2 ** n - 1].
 *
 *  Method 1:
 *    return bit_length(value) <= double_precision; [double_precision is 53]
 *
 *  Method 2:
 *    return value <= dbl_max_int;                  [dbl_max_int is 2 ** 53 - 1]
 */
template <std::unsigned_integral T>
constexpr bool has_double_exact(T value) noexcept {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  static_assert(sizeof(uint64_t) * CHAR_BIT == 64,
                "uint64_t must be available");
  // Static assertion ensures there will be an integer type that can hold this
  // literal constant on the RHS.
  constexpr auto dbl_max_int = 0x1fffffffffffffu;  // 2 ** 53 - 1

  return value <= dbl_max_int;
}

}  // namespace uints

#endif  // BI_INCLUDE_UINTS_HPP_
