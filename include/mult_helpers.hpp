/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_INCLUDE_MULT_HELPERS_HPP_
#define BI_INCLUDE_MULT_HELPERS_HPP_

#include <cstdint>
#include <utility>
#if __cplusplus >= 202002L
#include <concepts>
#endif

namespace mult_helpers {

/**
 *  @brief Performs (rhi, rlo)_B = (a)_B * (b)_B.
 *
 *  (2^w - 1)(2^w - 1) = 2^(2w) - 2^(w + 1) + 1 <= 2^(2w) - 1. Thus, the result
 *  of multiplying two base 2^w digits is guaranteed to need at most two base
 *  2^w digits to represent it.
 */
#if __cplusplus >= 202002L
template <std::unsigned_integral T>
#else
template <typename T, typename = std::enable_if_t<std::is_unsigned<T>::value>>
#endif
constexpr std::pair<T, T> mult2(T a, T b);

// Specialization for uint64_t
template <>
constexpr std::pair<uint64_t, uint64_t> mult2(uint64_t a, uint64_t b) {
#ifdef __SIZEOF_INT128__
  unsigned __int128 res = static_cast<unsigned __int128>(a) * b;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  return {static_cast<uint64_t>(res >> 64), static_cast<uint64_t>(res)};
#else
  constexpr uint64_t MASK32 = 0xffffffff;

  uint64_t a0 = (a)&MASK32;
  uint64_t a1 = (a) >> 32;
  uint64_t b0 = (b)&MASK32;
  uint64_t b1 = (b) >> 32;
  uint64_t a0b0 = a0 * b0;
  uint64_t a0b1 = a0 * b1;
  uint64_t a1b0 = a1 * b0;
  uint64_t a1b1 = a1 * b1;
  uint64_t mid = a1b0 + (a0b0 >> 32) + (a0b1 & MASK32);

  return {a1b1 + (mid >> 32) + (a0b1 >> 32), (a0b0 & MASK32) | (mid << 32)};
#endif
}

// Specialization for uint32_t
template <>
constexpr std::pair<uint32_t, uint32_t> mult2(uint32_t a, uint32_t b) {
  uint64_t res = static_cast<uint64_t>(a) * b;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  return {static_cast<uint32_t>(res >> 32), static_cast<uint32_t>(res)};
}

/**
 *  @brief Performs (rhi, rlo)_B = (y)_B * (x)_B + (a)_B + (b)_B.
 *
 *  (2^w - 1)(2^w - 1) + 2(2^w - 1) = 2^(2w) - 1. Thus, the result of
 *  multiplying two base 2^w digits and adding two base 2^w digits is guaranteed
 *  to need at most two digits to represent it.
 */
#if __cplusplus >= 202002L
template <std::unsigned_integral T>
#else
template <typename T, typename = std::enable_if_t<std::is_unsigned<T>::value>>
#endif
constexpr std::pair<T, T> mult2_add2(T y, T x, T a, T b) {
  auto [rhi, rlo] = mult2(y, x);
  rlo += a;
  rhi += rlo < a;
  rlo += b;
  rhi += rlo < b;
  return {rhi, rlo};
}

}  // namespace mult_helpers

#endif  // BI_INCLUDE_MULT_HELPERS_HPP_
