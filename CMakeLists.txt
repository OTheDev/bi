cmake_minimum_required(VERSION 3.15)

project(bi VERSION 0.1)

include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)
if(result)
  set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(WARNING "IPO is not supported: ${output}")
endif()

add_library(bi_compiler_flags INTERFACE)
target_compile_features(bi_compiler_flags INTERFACE cxx_std_20)
set(gcc_like "$<COMPILE_LANG_AND_ID:C,CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc "$<COMPILE_LANG_AND_ID:C,CXX,MSVC>")
target_compile_options(
  bi_compiler_flags INTERFACE
  "$<${gcc_like}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc}:$<BUILD_INTERFACE:-W3>>"
)

add_library(bi_common_defs INTERFACE)
target_compile_definitions(
  bi_common_defs INTERFACE 
  BI_API_EXPORTS
  $<$<BOOL:${BI_FORCE_64_BIT}>:BI_FORCE_64_BIT>
  $<$<BOOL:${BI_FORCE_32_BIT}>:BI_FORCE_32_BIT>
)

if (APPLE)
  set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
elseif (UNIX)
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build the tests" ON)
option(BI_FORCE_64_BIT "Force 64-bit digit type" OFF)
option(BI_FORCE_32_BIT "Force 32-bit digit type" OFF)
if(BI_FORCE_64_BIT AND BI_FORCE_32_BIT)
  message(FATAL_ERROR "BI_FORCE_64_BIT and BI_FORCE_32_BIT cannot both be set.")
endif()

enable_testing()

add_subdirectory(src)
if (BUILD_TESTS)
  add_subdirectory(test)
endif()

install(
  DIRECTORY include/
  DESTINATION include
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/bi-config.cmake"
  INSTALL_DESTINATION "lib/cmake/bi"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/bi-config_version.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
  "${CMAKE_CURRENT_BINARY_DIR}/bi-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/bi-config_version.cmake"
  DESTINATION "lib/cmake/bi"
)

install(
  TARGETS
  bi_compiler_flags
  bi_common_defs
  EXPORT bi_targets
)

install(
  EXPORT bi_targets
  FILE bi-targets.cmake
  NAMESPACE bi::
  DESTINATION lib/cmake/bi
)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
include(CPack)
