cmake_minimum_required(VERSION 3.9)

project(lucid CXX)

# Compiler with C++17 support is required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check some C++17 features
include(cmake/Checks.cmake)

# Default Build Type is Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

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

# Options
option(USE_DOUBLE_PRECISION "Use double precision floating-point format" OFF)
option(BUILD_TESTS "Build tests" OFF)

# Use GCC/Clang pipe
set(CMAKE_CXX_FLAGS "-Wall -Wpedantic -Wextra -pipe")

# Optimize for the host machine
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -march=native -ffast-math")

# Print vectorization statistics
if(CMAKE_VERBOSE_MAKEFILE)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fopt-info-vec")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Rpass=loop-vectorize")
  endif()
endif()

# Don't optimize Debug build
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Set floating point precision
set(_precision "single")
if(USE_DOUBLE_PRECISION)
  set(_precision "double")
  add_definitions(-D_DOUBLE_PRECISION)
endif()
message(STATUS "Floating-point format: ${_precision} precision")

add_subdirectory(src)
