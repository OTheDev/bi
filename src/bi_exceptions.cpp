/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#include "bi_exceptions.hpp"

namespace bi {

/**
 *  @class exception
 *  @headerfile "bi_exceptions.hpp"
 *  @brief Base exception class for the `bi` library.
 *
 *  This class serves as the root for all exception types specific to the `bi`
 *  library, providing a common base from which all other custom exceptions
 *  derive. It extends `std::runtime_error`.
 */

/**
 *  @class overflow_error
 *  @headerfile "bi_exceptions.hpp"
 *  @brief Throws if an operation expects the result to require a digit vector
 *  with `size()` exceeding `max_size()`. See the [internals](@ref internals)
 *  section.
 */

/**
 *  @class division_by_zero
 *  @headerfile "bi_exceptions.hpp"
 *  @brief Throws if a division by zero attempt is detected.
 */

#ifdef __cpp_lib_source_location
exception::exception(const std::string& message,
                     const std::source_location& location)
    : std::runtime_error(message + "Thrown at " + location.file_name() + ":" +
                         std::to_string(location.line()) + " in function " +
                         location.function_name()) {}
#else
exception::exception(const std::string& message)
    : std::runtime_error(message) {}
#endif

}  // namespace bi
