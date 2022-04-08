cmake_minimum_required(VERSION "3.8")

# decline in-source build
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

# project information
project(
  hello_world
  VERSION "0.1.0"
  DESCRIPTION "A template project for C++ language."
  LANGUAGES "CXX"
)

# define source directories
set(HELLO_WORLD_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

#--------------------------------------------------------------------------------------#
# Build targets
#--------------------------------------------------------------------------------------#

add_executable(hello_world
  "${HELLO_WORLD_SOURCE_DIR}/lesson_01.cpp"
)
target_compile_features(hello_world PRIVATE
  "cxx_std_17"
)
target_compile_options(hello_world PRIVATE
  -Wall
  -Wextra
  $<$<STREQUAL:${CMAKE_BUILD_TYPE},"Release">:"-O2 -march=native">
  $<$<STREQUAL:${CMAKE_BUILD_TYPE},"RelWithDebInfo">:"-g3 -Og -pg">
  $<$<STREQUAL:${CMAKE_BUILD_TYPE},"Debug">:"-g3 -O0 -pg">
)
target_link_libraries(hello_world PRIVATE
  pthread
)
