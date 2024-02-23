/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_SRC_CONSTANTS_HPP_
#define BI_SRC_CONSTANTS_HPP_

#include <climits>
#include <limits>

#include "bi.hpp"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
static_assert(CHAR_BIT == 8);

namespace bi {

#if defined(BI_DIGIT_64_BIT)
using digit = uint64_t;
using ddigit = unsigned __int128;
#else
using digit = uint32_t;
using ddigit = uint64_t;
#endif

constexpr digit digit_c(digit v) { return v; }

constexpr unsigned bi_sizeof_digit = sizeof(digit);
constexpr unsigned bi_dwidth = CHAR_BIT * sizeof(digit);
constexpr unsigned bi_dbits = bi_dwidth;
constexpr digit bi_dmax = std::numeric_limits<digit>::max();
constexpr auto bi_base = static_cast<ddigit>(1) << bi_dwidth;

using bi_bitcount_t = unsigned long;

constexpr auto dbl_max_int = 0x1fffffffffffffu;  // 2 ** 53 - 1

}  // namespace bi

#endif  // BI_SRC_CONSTANTS_HPP_
