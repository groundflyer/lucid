cmake_minimum_required(VERSION 3.9)

project(lucid CXX)

# Compiler with C++17 support is required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check some C++17 features
include(cmake/Checks.cmake)

# External dependencies
# Threads
set(CMAKE_THREAD_PREFER_PTHREAD 1)
set(THREADS_PREFER_PTHREAD_FLAG 1)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
link_libraries(${OPENGL_gl_LIBRARY})

# GLFW3
find_package(glfw3 3.2 REQUIRED)
link_libraries(glfw)
add_definitions(-DGLFW_INCLUDE_GLCOREARB -DGLFW_INCLUDE_GLEXT -DGL_GLEXT_PROTOTYPES)

# Default Build Type is Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Options
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  option(USE_LIBCXX "Use LibC++ when compiling with clang" OFF)
endif()
option(USE_DOUBLE_PRECISION "Use double precision floating-point format" OFF)
option(BUILD_TESTS "Build tests" OFF)

# Use GCC/Clang pipe
add_compile_options(-Wall -Wpedantic -Wextra -pipe)

# Optimize for the host machine
add_compile_options($<$<CONFIG:Release>:-O3>)
add_compile_options($<$<CONFIG:Release>:-DNDEBUG>)
add_compile_options($<$<CONFIG:Release>:-march=native>)
add_compile_options($<$<CONFIG:Release>:-ffast-math>)

# Link-time optimization
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  include(CheckIPOSupported)
  check_ipo_supported(RESULT do_lto OUTPUT lto_error LANGUAGES CXX)
  if(do_lto)
    message(STATUS "Interprocedural optimization enabled")
    set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(STATUS "Interprocedural optimization is not supported: ${lto_error}")
  endif()
endif()

# Don't optimize Debug build
add_compile_options($<$<CONFIG:Debug>:-O0>)
add_compile_options($<$<CONFIG:Debug>:-g>)

# Colored output from Clang
add_compile_options($<$<CXX_COMPILER_ID:Clang>:-fcolor-diagnostics>)

# Print vectorization statistics
if(CMAKE_VERBOSE_MAKEFILE)
  add_compile_options($<$<CXX_COMPILER_ID:GNU>:-fopt-info-vec>)
  add_compile_options($<$<CXX_COMPILER_ID:Clang>:-Rpass=loop-vectorize>)
endif()

# Set floating point precision
set(_precision "single")
if(USE_DOUBLE_PRECISION)
  set(_precision "double")
  add_definitions(-D_DOUBLE_PRECISION)
endif()
message(STATUS "Floating-point format: ${_precision} precision")

# spdlog
include_directories(spdlog/include)

add_subdirectory(src)
