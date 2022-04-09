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

# define function to add unit tests in the same format
function(ADD_MEMORY_ORDER_PLAYGROUND_TARGET MEMORY_ORDER_PLAYGROUND_TARGET)
  add_executable(${MEMORY_ORDER_PLAYGROUND_TARGET}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/${MEMORY_ORDER_PLAYGROUND_TARGET}.cpp"
  )
  target_compile_features(${MEMORY_ORDER_PLAYGROUND_TARGET} PRIVATE
    "cxx_std_17"
  )
  target_compile_options(${MEMORY_ORDER_PLAYGROUND_TARGET} PRIVATE
    -Wall
    -Wextra
    $<$<STREQUAL:${CMAKE_BUILD_TYPE},"Release">:"-O3 -march=native">
    $<$<STREQUAL:${CMAKE_BUILD_TYPE},"RelWithDebInfo">:"-g3 -Og -pg">
    $<$<STREQUAL:${CMAKE_BUILD_TYPE},"Debug">:"-g3 -O0 -pg">
  )
  target_link_libraries(${MEMORY_ORDER_PLAYGROUND_TARGET} PRIVATE
    pthread
  )
  add_test(NAME ${MEMORY_ORDER_PLAYGROUND_TARGET} COMMAND $<TARGET_FILE:${MEMORY_ORDER_PLAYGROUND_TARGET}>)
endfunction()

ADD_MEMORY_ORDER_PLAYGROUND_TARGET("lesson_01")
ADD_MEMORY_ORDER_PLAYGROUND_TARGET("lesson_02")
