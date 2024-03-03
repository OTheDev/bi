/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_INCLUDE_IMPL_BI_INL_
#define BI_INCLUDE_IMPL_BI_INL_

#include "bi.hpp"

#include <utility>

namespace bi {

/**
 *  @name Accessors for internal representation
 */
///@{

inline size_t bi_t::capacity() const noexcept { return vec_.capacity(); }

inline size_t bi_t::size() const noexcept { return vec_.size(); }

inline bool bi_t::negative() const noexcept { return negative_; }

inline std::span<const digit> bi_t::digits() const {
  if (vec_.size() == 0) {
    return std::span<const digit>();
  }
  return std::span<const digit>(vec_.data(), vec_.size());
}

///@}

/**
 *  @name Other
 */
///@{

inline void bi_t::swap(bi_t& other) noexcept {
  using std::swap;
  swap(this->vec_, other.vec_);
  swap(this->negative_, other.negative_);
}

inline void bi_t::negate() noexcept {
  if (size() != 0) {
    negative_ = !negative_;
  }
}

inline int bi_t::sign() const noexcept {
  if (size() == 0) {
    return 0;
  }
  return negative_ ? -1 : 1;
}

inline bool bi_t::odd() const noexcept {
  return size() == 0 ? false : vec_[0] & 1;
}

inline bool bi_t::even() const noexcept { return !odd(); }

///@}

}  // namespace bi

#endif  // BI_INCLUDE_IMPL_BI_INL_
