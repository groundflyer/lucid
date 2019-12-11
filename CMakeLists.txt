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

# OpenGL
find_package(OpenGL REQUIRED)

# Default Build Type is Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Options
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  option(USE_LIBCXX "Use LibC++ when compiling with clang" OFF)
endif()
option(USE_DOUBLE_PRECISION "Use double precision floating-point format" OFF)
option(BUILD_TESTS "Build tests" OFF)

# Use GCC/Clang pipe
add_compile_options(-Wall -Wpedantic -Wextra -pipe -fno-rtti)

# Optimize for the host machine
add_compile_options($<$<CONFIG:Release>:-march=native>)
add_compile_options($<$<CONFIG:Release>:-ffast-math>)

# Link-time optimization
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  include(CheckIPOSupported)
  check_ipo_supported(RESULT HAS_IPO OUTPUT IPO_ERROR LANGUAGES CXX)
  if(HAS_IPO)
    option(ENABLE_IPO "Enable interprocedural optimization" OFF)
    if(ENABLE_IPO)
      message(STATUS "Interprocedural optimization enabled")
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
  else()
    message(STATUS "Interprocedural optimization is not supported: ${IPO_ERROR}")
  endif()
endif()

# RelWithDebInfo
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
add_compile_options($<$<CONFIG:RelWithDebInfo>:-march=native>)
add_compile_options($<$<CONFIG:RelWithDebInfo>:-ffast-math>)

# Colored output from Clang
add_compile_options($<$<CXX_COMPILER_ID:Clang>:-fcolor-diagnostics>)

# Set floating point precision
set(_precision "single")
if(USE_DOUBLE_PRECISION)
  set(_precision "double")
  add_definitions(-D_DOUBLE_PRECISION)
endif()
message(STATUS "Floating-point format: ${_precision} precision")

# GLFW3
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
set(GL_LIBS ${OPENGL_gl_LIBRARY} glfw)
set(GL_DEFS "-DGLFW_INCLUDE_GLCOREARB -DGLFW_INCLUDE_GLEXT -DGL_GLEXT_PROTOTYPES")

# fmt
set(BUILD_SHARED_LIBS FALSE)
add_definitions(-DFMT_USE_GRISU=1)
add_subdirectory(fmt EXCLUDE_FROM_ALL)

# Concurrent Queue
include_directories(concurrentqueue)

add_subdirectory(src)
