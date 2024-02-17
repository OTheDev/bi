/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_SRC_BI__HPP_
#define BI_SRC_BI__HPP_

#include <cassert>

#include "bi.hpp"
#include "bi_const.hpp"

namespace bi {

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
