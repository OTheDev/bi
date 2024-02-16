/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_SRC_H__HPP_
#define BI_SRC_H__HPP_

#include <algorithm>
#include <limits>
#include <string>

#include "bi_.hpp"

namespace bi {

struct h_ {
  using dvector = bi_t::dvector;

  // comparisons
  static int cmp_abs(const bi_t&, const bi_t&) noexcept;
  static int cmp(const bi_t&, const bi_t&) noexcept;
  template <std::integral T>
  static int cmp(const bi_t&, T) noexcept;

  // initializing
  template <std::integral T>
  static void init_one_digit(bi_t&, T);
  template <std::integral T>
  static void init_atleast_one_digit(bi_t&, T);
  static void init_string(bi_t&, const std::string& str);

  // increment/decrement
  static void increment_abs(bi_t&);
  static void decrement_abs(bi_t&);

  // arithmetic
  static void add_abs(bi_t& result, const bi_t& a, const bi_t& b);
  static void add(bi_t& result, const bi_t& a, const bi_t& b);
  static void sub_abs_gt(bi_t& result, const bi_t& a, const bi_t& b);
  static void sub_abs(bi_t& result, const bi_t& a, const bi_t& b);
  static void sub(bi_t& result, const bi_t& a, const bi_t& b);
  static void imul1add1(bi_t&, digit v, digit k);
  static void mul(bi_t& result, const bi_t& a, const bi_t& b);
  static void div_algo_binary(bi_t& q, bi_t& r, const bi_t& n, const bi_t& d);
  static void div_algo_single(bi_t& q, bi_t& r, const bi_t& n,
                              const bi_t& d) noexcept;
  static void div_algo_knuth(bi_t& q, bi_t& r, const bi_t& n, const bi_t& d);
  static void divide(bi_t& q, bi_t& r, const bi_t& n, const bi_t& d);

  // bits
  static void left_shift(bi_t& result, const bi_t& a, bi_bitcount_t shift);
  static void right_shift(bi_t& result, const bi_t& a, bi_bitcount_t shift);
  enum class BitwiseOperation { AND, OR, XOR };
  template <BitwiseOperation Op>
  static void bitwise_operation_impl(bi_t& res, const bi_t& a, const bi_t& b);
  template <BitwiseOperation Op>
  static bi_t bitwise_operation(const bi_t& a, const bi_t& b);
  template <BitwiseOperation Op>
  static bi_t& ibitwise_operation(bi_t&, const bi_t& other);

  // to string
  static uint8_t idiv10(bi_t&) noexcept;
  static size_t decimal_length(const bi_t&);

  static dvector to_twos_complement(const dvector& vec);
  static void to_twos_complement_in_place(dvector& vec) noexcept;
};

template <std::integral T>
int h_::cmp(const bi_t& a, T b) noexcept {
  if (b == 0) {
    if (a.size() == 0) {
      return 0;  // a
    }
    return a.negative() ? -1 : 1;  // b
  }

  using UnsignedT = typename std::make_unsigned<T>::type;
  const bool b_negative = b < 0;
  const UnsignedT unsigned_b =
      b_negative ? -static_cast<UnsignedT>(b) : static_cast<UnsignedT>(b);

  if (a.negative() && !b_negative) {
    return -1;  // c
  }
  if (!a.negative() && b_negative) {
    return 1;  // d
  }

  size_t n_b_digits = 0;
  if constexpr (std::numeric_limits<UnsignedT>::max() <= bi_dmax) {
    n_b_digits = (unsigned_b != 0) ? 1 : 0;
  } else {
    UnsignedT temp_b = unsigned_b;
    while (temp_b != 0) {
      temp_b >>= bi_dwidth;
      n_b_digits++;
    }
  }

  const size_t a_size = a.size();
  if (a_size < n_b_digits) {
    return a.negative() ? 1 : -1;  // e
  }

  if (a_size > n_b_digits) {
    return a.negative() ? -1 : 1;  // f
  }

  for (size_t i = n_b_digits; i-- > 0;) {
    const digit a_digit = a[i];
    const digit b_digit = static_cast<digit>(unsigned_b >> (bi_dwidth * i));

    if (a_digit < b_digit) {
      return a.negative() ? 1 : -1;  // g
    }
    if (a_digit > b_digit) {
      return a.negative() ? -1 : 1;  // h
    }
  }
  return 0;  // i
}

template <std::integral T>
void h_::init_one_digit(bi_t& x, T value) {
  x.resize_(1);

  if (value >= 0) {
    x[0] = static_cast<digit>(value);
    x.negative_ = false;
  } else {
    x[0] = static_cast<digit>(0) - static_cast<digit>(value);
    x.negative_ = true;
  }
}

template <std::integral T>
void h_::init_atleast_one_digit(bi_t& x, T value) {
  using UnsignedT = typename std::make_unsigned<T>::type;

  size_t size = 1;
  UnsignedT temp, uvalue;

  if (value >= 0) {
    uvalue = value;
    x.negative_ = false;
  } else {
    uvalue = static_cast<UnsignedT>(0) - static_cast<UnsignedT>(value);
    x.negative_ = true;
  }

  temp = uvalue;

  while (temp >>= bi_dwidth) {
    ++size;
  }

  x.resize_(size);

  for (size_t i = 0; uvalue != 0; ++i) {
    x[i] = static_cast<digit>(uvalue);
    uvalue >>= bi_dwidth;
  }
}

/// @private
inline bi_t::dvector h_::to_twos_complement(const dvector& vec) {
  const size_t vec_size = vec.size();
  dvector result;
  result.resize(vec_size);

  bool carry = true;
  for (size_t i = 0; i < vec_size; ++i) {
    const digit sum = ~vec[i] + carry;
    carry = sum < static_cast<uint8_t>(carry);
    result[i] = sum;
  }

  return result;
}

/// @private
inline void h_::to_twos_complement_in_place(dvector& vec) noexcept {
  const size_t vec_size = vec.size();

  bool carry = true;
  for (size_t i = 0; i < vec_size; ++i) {
    const digit sum = ~vec[i] + carry;
    carry = sum < static_cast<uint8_t>(carry);
    vec[i] = sum;
  }
}

/// @private
template <h_::BitwiseOperation Op>
void h_::bitwise_operation_impl(bi_t& result, const bi_t& x, const bi_t& y) {
  size_t max_size = std::max(x.size(), y.size());

  const dvector& lhs_digits = x.negative_ ? to_twos_complement(x.vec_) : x.vec_;
  const dvector& rhs_digits = y.negative_ ? to_twos_complement(y.vec_) : y.vec_;

  bool result_negative;  // NOLINT(cppcoreguidelines-init-variables)
  if constexpr (Op == BitwiseOperation::AND) {
    result_negative = x.negative_ && y.negative_;
  } else if constexpr (Op == BitwiseOperation::OR) {
    result_negative = x.negative_ || y.negative_;
  } else if constexpr (Op == BitwiseOperation::XOR) {
    result_negative = x.negative_ != y.negative_;
    ++max_size;
  }

  result.resize_(max_size);

  for (size_t i = 0; i < result.size(); ++i) {
    const digit lhs_digit =
        i < lhs_digits.size() ? lhs_digits[i] : (x.negative_ ? bi_dmax : 0);
    const digit rhs_digit =
        i < rhs_digits.size() ? rhs_digits[i] : (y.negative_ ? bi_dmax : 0);

    if constexpr (Op == BitwiseOperation::AND) {
      result[i] = lhs_digit & rhs_digit;
    } else if constexpr (Op == BitwiseOperation::OR) {
      result[i] = lhs_digit | rhs_digit;
    } else if constexpr (Op == BitwiseOperation::XOR) {
      result[i] = lhs_digit ^ rhs_digit;
    }
  }

  result.negative_ = result_negative;

  if (result.negative_) {
    to_twos_complement_in_place(result.vec_);
  }

  result.trim_trailing_zeros();
}

/// @private
template <h_::BitwiseOperation Op>
bi_t h_::bitwise_operation(const bi_t& a, const bi_t& b) {
  bi_t result;
  bitwise_operation_impl<Op>(result, a, b);
  return result;
}

/// @private
template <h_::BitwiseOperation Op>
bi_t& h_::ibitwise_operation(bi_t& x, const bi_t& other) {
  bitwise_operation_impl<Op>(x, x, other);
  return x;
}

}  // namespace bi

#endif  // BI_SRC_H__HPP_
