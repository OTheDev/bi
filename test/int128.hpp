/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_TEST_INT128_HPP_
#define BI_TEST_INT128_HPP_

#include <algorithm>
#include <charconv>
#include <concepts>
#include <stdexcept>
#include <string>

#if defined(__SIZEOF_INT128__)

namespace bi_int128 {

template <typename T>
concept is_int128 = std::is_same<T, __int128>::value ||
                    std::is_same<T, unsigned __int128>::value;

template <is_int128 T>
std::string to_string(T value) {
  if (value == 0)
    return "0";

  std::string result;

  bool negative = std::is_same<T, __int128>::value && value < 0;
  unsigned __int128 uvalue =
      negative ? -static_cast<unsigned __int128>(value) : value;

  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  while (uvalue != 0) {
    int digit = static_cast<int>(uvalue % 10);
    result.push_back(static_cast<char>('0' + digit));
    uvalue /= 10;
  }
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

  if (negative) {
    result.push_back('-');
  }

  std::reverse(result.begin(), result.end());

  return result;
}

template <is_int128 T>
T from_string(const std::string& str) {
  T result = 0;
  // NOLINTNEXTLINE
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
  if (ec != std::errc()) {
    throw std::invalid_argument("");
  }
  return result;
}

unsigned __int128 operator"" _u128(const char* str) {
  return from_string<unsigned __int128>(str);
}

__int128 operator"" _i128(const char* str) {
  return from_string<__int128>(str);
}

}  // namespace bi_int128

#endif  // __SIZEOF_INT128__

#endif  // BI_TEST_INT128_HPP_
