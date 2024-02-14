add_library(
  bi
  bi.cpp
)

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
