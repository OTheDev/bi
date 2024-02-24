/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_INCLUDE_BI_EXCEPTIONS_HPP_
#define BI_INCLUDE_BI_EXCEPTIONS_HPP_

#include <version>
#ifdef __cpp_lib_source_location
#include <source_location>
#endif
#include <stdexcept>
#include <string>

namespace bi {

class exception : public std::runtime_error {
 public:
#ifdef __cpp_lib_source_location
  exception(const std::string& message, const std::source_location& location =
                                            std::source_location::current());
#else
  explicit exception(const std::string& message);
#endif
};

class overflow_error : public exception {
 public:
  using exception::exception;
};

class division_by_zero : public exception {
 public:
  using exception::exception;
};

class from_float : public exception {
 public:
  using exception::exception;
};

}  // namespace bi

#endif  // BI_INCLUDE_BI_EXCEPTIONS_HPP_
