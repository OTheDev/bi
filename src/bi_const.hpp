/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_SRC_BI_CONST_HPP_
#define BI_SRC_BI_CONST_HPP_

#include "bi.hpp"

namespace bi {

#if defined(BI_DIGIT_64_BIT)
using ddigit = unsigned __int128;
constexpr digit digit_c(digit v) { return v; }

constexpr int bi_dwidth = 64;
constexpr int bi_dbits = 64;
constexpr digit bi_dmax = digit_c(0xffffffffffffffff);
constexpr int bi_sizeof_digit = 8;
#else
using ddigit = uint64_t;
constexpr digit digit_c(digit v) { return v; }

constexpr int bi_dwidth = 32;
constexpr int bi_dbits = 32;
constexpr digit bi_dmax = digit_c(0xffffffff);
constexpr int bi_sizeof_digit = 4;
#endif

constexpr auto bi_base = static_cast<ddigit>(1) << bi_dwidth;
constexpr auto dbl_max_int = 0x1fffffffffffffu;  // 2 ** 53 - 1

}  // namespace bi

#endif  // BI_SRC_BI_CONST_HPP_
