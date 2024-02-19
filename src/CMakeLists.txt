add_library(
  bi
  bi.cpp
  bi_exceptions.cpp
)

# include(CheckIPOSupported)
# check_ipo_supported(RESULT result OUTPUT output)
# if(result)
#   set_target_properties(bi PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
# else()
#   message(WARNING "IPO is not supported: ${output}")
# endif()

set_target_properties(
  bi PROPERTIES
  POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS}
)

target_link_libraries(
  bi PUBLIC
  bi_compiler_flags
  bi_common_defs
  $<$<BOOL:${UNIX}>:m>  # Link the math library
)

target_include_directories(
  bi PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

install(
  TARGETS bi
  EXPORT bi_targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)
