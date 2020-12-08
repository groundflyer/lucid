cmake_minimum_required(VERSION 3.16)

project(Lucid CXX)

# Compiler with C++20 support is required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Check for required compiler features
include(CxxFeatures)

# Clang-related checks
include(Clang)

# Default Build Type is Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Options
option(USE_DOUBLE_PRECISION "Use double precision floating-point format" OFF)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BUILD_DOC "Build documentation" OFF)
option(ENABLE_IPO "Enable interprocedural optimization" OFF)

add_compile_options(-Wall -Wpedantic -Wextra)
# Optimize for the host machine
add_compile_options($<$<CONFIG:Release>:-march=native>)
add_compile_options($<$<CONFIG:Release>:-ffast-math>)

# RelWithDebInfo
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
add_compile_options($<$<CONFIG:RelWithDebInfo>:-march=native>)
add_compile_options($<$<CONFIG:RelWithDebInfo>:-ffast-math>)
add_compile_options($<$<CONFIG:RelWithDebInfo>:-O3>)

# Link-time optimization
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  include(CheckIPOSupported)
  check_ipo_supported(RESULT HAS_IPO OUTPUT IPO_ERROR LANGUAGES CXX)
  if(HAS_IPO)
    if(ENABLE_IPO)
      message(STATUS "Interprocedural optimization enabled")
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
  else()
    message(STATUS "Interprocedural optimization is not supported")
  endif()
endif()

# Set floating point precision
set(_precision "single")
if(USE_DOUBLE_PRECISION)
  set(_precision "double")
  add_definitions(-D_DOUBLE_PRECISION)
endif()
message(STATUS "Floating-point format: ${_precision} precision")

# Threads
set(CMAKE_THREAD_PREFER_PTHREAD 1)
set(THREADS_PREFER_PTHREAD_FLAG 1)
find_package(Threads REQUIRED)

# NanoGUI
set(NANOGUI_BUILD_SHARED  OFF CACHE BOOL "" FORCE)
set(NANOGUI_BUILD_EXAMPLE ON CACHE BOOL "" FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL "" FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL "" FORCE)
add_subdirectory(nanogui EXCLUDE_FROM_ALL)
add_definitions(${NANOGUI_EXTRA_DEFS})
include_directories(${NANOGUI_EXTRA_INCS})

# fmt
set(BUILD_SHARED_LIBS FALSE)
add_definitions(-DFMT_USE_GRISU=1)
add_subdirectory(fmt EXCLUDE_FROM_ALL)

# Concurrent Queue
include_directories(concurrentqueue)

# range-v3
include_directories(range-v3/include)

# static_math
include_directories(static_math/include)

# docs
if(BUILD_DOC)
  find_package(Doxygen REQUIRED dot)
  if(DOXYGEN_FOUND)
    message(STATUS "Building documentation")

    set(_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set(DOC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/doc)
    set(BIB_DIR ${DOC_DIR}/bib)

    set(DOXYGEN_PROJECT_BRIEF "Experimental renderer")
    set(DOXYGEN_FILE_PATTERNS *.cpp *.hpp *.md)
    set(DOXYGEN_EXTRACT_LOCAL_CLASSES NO)
    set(DOXYGEN_HIDE_UNDOC_CLASSES YES)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_INCLUDE_GRAPH NO)
    set(DOXYGEN_INCLUDED_BY_GRAPH NO)
    set(DOXYGEN_VERBATIM_HEADERS NO)
    set(DOXYGEN_DIRECTORY_GRAPH NO)
    set(DOXYGEN_DOT_IMAGE_FORMAT svg)
    set(DOXYGEN_INTERACTIVE_SVG YES)
    set(DOXYGEN_TOC_EXPAND YES)
    set(DOXYGEN_USE_MATHJAX YES)
    set(DOXYGEN_CITE_BIB_FILES
      ${BIB_DIR}/Duff2017Basis.bib
      ${BIB_DIR}/other.bib
      ${BIB_DIR}/LD04ERQIT_citation.bib)
    doxygen_add_docs(doc ${_SRC_DIR} ${DOC_DIR} ALL
      COMMENT "Generating documentation with Doxygen"
      WORKING_DIRECTORY ${_SRC_DIR})
  else()
    message(SEND_ERROR "Doxygen not found. Can't build docs.")
  endif()
endif()

add_subdirectory(src)
