/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_INCLUDE_BI_CONFIG_HPP_
#define BI_INCLUDE_BI_CONFIG_HPP_

#include <climits>
#include <cstddef>
#include <cstdint>
#include <limits>

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

#if defined(__SIZEOF_INT128__)
#define HAS_UINT128 1  // NOLINT
#endif

// TODO: Both options work, but CI tests suggest that 32-bit digits are faster
// (for what is tested). However, this needs to be investigated/benchmarked on
// the operation level. CI continues to test both versions. Because int128 is
// typically implemented through 64-bit integers, maybe omitting the 64-bit case
// altogether is best. I suspect that for 32-bit (64-bit) systems, 16-bit digits
// are optimal, so that 32-bit (64-bit) arithmetic is fast.
#if defined(BI_FORCE_64_BIT) && defined(HAS_UINT128)
#define BI_DIGIT_64_BIT
#elif defined(BI_FORCE_32_BIT)
#define BI_DIGIT_32_BIT
// #elif (defined(__x86_64__) || defined(_WIN64) || defined(__LP64__) ||      \
//        defined(__aarch64__)) &&                                            \
//     defined(HAS_UINT128)
// #define BI_DIGIT_64_BIT
#else
#define BI_DIGIT_32_BIT
#endif

namespace bi_config {

#if defined(BI_DIGIT_64_BIT)
using digit = uint64_t;
using ddigit = unsigned __int128;
constexpr digit digit_c(digit v) { return v; }

constexpr int bi_dwidth = 64;
constexpr int bi_dbits = 64;
constexpr digit bi_dmax = digit_c(0xffffffffffffffff);
constexpr int bi_sizeof_digit = 8;
#else
using digit = uint32_t;
using ddigit = uint64_t;
constexpr digit digit_c(digit v) { return v; }

constexpr int bi_dwidth = 32;
constexpr int bi_dbits = 32;
constexpr digit bi_dmax = digit_c(0xffffffff);
constexpr int bi_sizeof_digit = 4;
#endif

constexpr auto bi_base = static_cast<ddigit>(1) << bi_dwidth;

using bi_bitcount_t = unsigned long;

// Want size() such that
// (1) size() <= (SIZE_MAX / BI_SIZEOF_DIGIT) := A
// (2) size() <= (ULONG_MAX / BI_DBITS) := B
// Set BI_MAX_DIGITS to min(A, B)
constexpr size_t compute_max_digits() {
  size_t a = std::numeric_limits<size_t>::max() / bi_sizeof_digit;
  bi_bitcount_t b = std::numeric_limits<bi_bitcount_t>::max() / bi_dbits;

  return a < b ? a : static_cast<size_t>(b);
}

constexpr size_t bi_max_digits = compute_max_digits();

constexpr auto dbl_max_int = 0x1fffffffffffffu;  // 2 ** 53 - 1

#ifdef BI_DEBUG
constexpr bool range_check = true;
#else
constexpr bool range_check = false;
#endif

}  // namespace bi_config

#endif  // BI_INCLUDE_BI_CONFIG_HPP_
