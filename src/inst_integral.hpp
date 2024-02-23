/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_SRC_INST_INTEGRAL_HPP_
#define BI_SRC_INST_INTEGRAL_HPP_

/*
C++20 Standard (6.8.1, pp. 70-71)
-------------------------------------------------------------------------------
Standard Signed Integer Types       |   Minimum width N
-------------------------------------------------------------------------------
  signed char                       |   8
  short int                         |   16
  int                               |   16
  long int                          |   32
  long long int                     |   64
Standard Unsigned Integer Types
  unsigned char
  unsigned short int
  unsigned int
  unsigned long int
  unsigned long long int
Extension Signed/Unsigned Integer Types (GCC, Clang)
  __int128
  unsigned __int128
*/

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define BI_EMPTY

#if defined(__SIZEOF_INT128__)
#define BI_INST_TEMPLATE_FOR_EXTENSION_INTEGRAL_TYPES(FUNC, END)               \
  template FUNC(__int128) END;                                                 \
  template FUNC(unsigned __int128) END;

#define BI_INST_TEMPLATE_FOR_EXTENSION_INTEGRAL_TYPES_ARG(FUNC, ARG)           \
  template FUNC(__int128, ARG);                                                \
  template FUNC(unsigned __int128, ARG);

#define BI_INST_TEMPLATE_FOR_EXTENSION_INTEGRAL_TYPES_CONV(FUNC, END)          \
  template FUNC __int128() END;                                                \
  template FUNC unsigned __int128() END;
#else
#define BI_INST_TEMPLATE_FOR_EXTENSION_INTEGRAL_TYPES(FUNC, END)
#define BI_INST_TEMPLATE_FOR_EXTENSION_INTEGRAL_TYPES_ARG(FUNC, ARG)
#define BI_INST_TEMPLATE_FOR_EXTENSION_INTEGRAL_TYPES_CONV(FUNC, END)
#endif

#define BI_INST_TEMPLATE_FOR_INTEGRAL_TYPES(FUNC, END)                         \
  template FUNC(signed char) END;                                              \
  template FUNC(short) END;                                                    \
  template FUNC(int) END;                                                      \
  template FUNC(long) END;                                                     \
  template FUNC(long long) END;                                                \
  template FUNC(unsigned char) END;                                            \
  template FUNC(unsigned short) END;                                           \
  template FUNC(unsigned) END;                                                 \
  template FUNC(unsigned long) END;                                            \
  template FUNC(unsigned long long) END;                                       \
  BI_INST_TEMPLATE_FOR_EXTENSION_INTEGRAL_TYPES(FUNC, END)

#define BI_INST_TEMPLATE_FOR_INTEGRAL_TYPES_ARG(FUNC, ARG)                     \
  template FUNC(signed char, ARG);                                             \
  template FUNC(short, ARG);                                                   \
  template FUNC(int, ARG);                                                     \
  template FUNC(long, ARG);                                                    \
  template FUNC(long long, ARG);                                               \
  template FUNC(unsigned char, ARG);                                           \
  template FUNC(unsigned short, ARG);                                          \
  template FUNC(unsigned, ARG);                                                \
  template FUNC(unsigned long, ARG);                                           \
  template FUNC(unsigned long long, ARG);                                      \
  BI_INST_TEMPLATE_FOR_EXTENSION_INTEGRAL_TYPES_ARG(FUNC, ARG)

#define BI_INST_TEMPLATE_FOR_INTEGRAL_TYPES_CONV(FUNC, END)                    \
  template FUNC signed char() END;                                             \
  template FUNC short() END;                                                   \
  template FUNC int() END;                                                     \
  template FUNC long() END;                                                    \
  template FUNC long long() END;                                               \
  template FUNC unsigned char() END;                                           \
  template FUNC unsigned short() END;                                          \
  template FUNC unsigned() END;                                                \
  template FUNC unsigned long() END;                                           \
  template FUNC unsigned long long() END;                                      \
  BI_INST_TEMPLATE_FOR_EXTENSION_INTEGRAL_TYPES_CONV(FUNC, END)

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif  // BI_SRC_INST_INTEGRAL_HPP_
