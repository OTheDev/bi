/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_SRC_BI__HPP_
#define BI_SRC_BI__HPP_

#include <cassert>

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

inline digit& bi_t::operator[](size_t index) { return vec_[index]; }
inline const digit& bi_t::operator[](size_t index) const { return vec_[index]; }

inline auto bi_t::begin() noexcept { return vec_.begin(); }
inline auto bi_t::begin() const noexcept { return vec_.begin(); }
inline auto bi_t::end() noexcept { return vec_.end(); }
inline auto bi_t::end() const noexcept { return vec_.end(); }
inline auto bi_t::rbegin() noexcept { return vec_.rbegin(); }
inline auto bi_t::rbegin() const noexcept { return vec_.rbegin(); }
inline auto bi_t::rend() noexcept { return vec_.rend(); }
inline auto bi_t::rend() const noexcept { return vec_.rend(); }

/**
 *  @name Internal modifiers for the digit vector
 */
///@{

inline void bi_t::reserve_(size_t new_capacity) { vec_.reserve(new_capacity); }

inline void bi_t::resize_(size_t new_size) { vec_.resize(new_size); }

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

}  // namespace bi

#endif  // BI_SRC_BI__HPP_
