cmake_minimum_required(VERSION 3.9)

project(lucid CXX)

# Compiler with C++17 support is required
set(CMAKE_CXX_STANDARD 17)

# Default Build Type is Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Options
option(USE_DOUBLE_PRECISION "Use double precision floating-point format" OFF)
option(BUILD_TESTS "Build and run tests" OFF)

# Use GCC/Clang pipe
set(CMAKE_CXX_FLAGS "-Wall -Wpedantic -Wextra -pipe")

# Optimize for the host machine
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -march=native -ffast-math")

# Print vectorization statistics
if(CMAKE_VERBOSE_MAKEFILE)
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fopt-info-vec")
  elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Rpass=loop-vectorize")
  endif()
endif()

# Link-time optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT do_lto OUTPUT lto_error LANGUAGES CXX)
if(do_lto)
  message(STATUS "Interprocedural optimization enabled")
  set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(STATUS "Interprocedural optimization is not supported: ${lto_error}")
endif()

# Don't optimize Debug build
set(CMAKE_CXX_FLAGS_DEBUG -g -O0 -D_GLIBCXX_DEBUG)

# Use Clang with libc++ and colorize output
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -stlib=libc++ -fcolor-diagnostics)
  set(CMAKE_EXE_LINKER_FLAGS -lc++abi)
endif()

# Set floating point precision
set(_precision "single")
if(USE_DOUBLE_PRECISION)
  set(_precision "double")
  add_definitions(-D_DOUBLE_PRECISION)
endif()
message(STATUS "Floating-point format: ${_precision} precision")

add_subdirectory(src)
