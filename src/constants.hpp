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

// TODO: maybe remove dependency on ddigit in the future
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
constexpr double bi_base_dbl =
    2.0 * static_cast<double>(static_cast<digit>(1) << (bi_dbits - 1));
constexpr double bi_base_dbl_reciprocal = 1.0 / bi_base_dbl;

using bi_bitcount_t = unsigned long;

constexpr auto dbl_max_int = 0x20000000000000u;  // 2 ** 53

/*
 *  TRUE: std::ldexp(std::numeric_limits<double>::max(), -1024) < 1  and
 *        std::ldexp(std::numeric_limits<double>::max(), -1023) >= 1
 *  What x makes bi_dbits * (x - 1) >= 1024?
 *    x >= (1024 / bi_dbits + 1) := upper   [note that 1024 % bi_dbits == 0]
 *
 *  constexpr auto upper = 1024 / bi_dbits + 1;
 */
constexpr int find_upper() {
  double dbl = std::numeric_limits<double>::max();
  int x = 1;

  while (dbl >= 1.0) {
    // Equiv. to dbl /= bi_base_dbl
    dbl *= bi_base_dbl_reciprocal;
    ++x;
  }

  return x;
}

constexpr auto bi_cmp_dbl_size_upper = find_upper();

constexpr auto max_size = dvector::max_size();
constexpr bi_bitcount_t max_bits = max_size * bi_dwidth;

// If both operands of * have size() >= karatsuba_threshold, then use karatsuba
constexpr auto karatsuba_threshold = 60;

}  // namespace bi

#endif  // BI_SRC_CONSTANTS_HPP_
