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

/**
 *  @brief Base exception class for the `bi` library.
 *
 *  This class serves as the root for all exception types specific to the `bi`
 *  library, providing a common base from which all other custom exceptions
 *  derive. It extends `std::runtime_error`.
 */
class exception : public std::runtime_error {
 public:
#ifdef __cpp_lib_source_location
  exception(const std::string& message, const std::source_location& location =
                                            std::source_location::current())
      : std::runtime_error(message + "Thrown at " + location.file_name() + ":" +
                           std::to_string(location.line()) + " in function " +
                           location.function_name()) {}
#else
  explicit exception(const std::string& message)
      : std::runtime_error(message) {}
#endif
};

/**
 *  @brief Throws if an operation expects the result to require a digit vector
 *  with `size()` exceeding `max_size()`. See the [internals](@ref internals)
 *  section.
 */
class overflow_error : public exception {
 public:
  using exception::exception;
};

/**
 *  @brief Throws if a division by zero attempt is detected.
 */
class division_by_zero : public exception {
 public:
  using exception::exception;
};

}  // namespace bi

#endif  // BI_INCLUDE_BI_EXCEPTIONS_HPP_
