include(CheckCXXSourceCompiles)
include(CheckCXXCompilerFlag)

set(CMAKE_REQUIRED_FLAGS -std=c++17)

# Check for Typename in template
check_cxx_source_compiles("template <template <typename> typename T> struct foo {}; int main(){return 0;}" TypenameInTemplate)

if(NOT TypenameInTemplate)
  message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION} doesn't support typename in a template (N4051)")
endif()

# Check for Structured bindings
check_cxx_source_compiles("struct foo{int a; char b;}; int main(){auto [a, b] = foo{}; return 0;}" StructuredBindings)

if(NOT StructuredBindings)
  message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION} doesn't support structured bindings (P0217R3)")
endif()

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

# Use Clang with libc++ and colorize output
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  check_cxx_compiler_flag("-stdlib=libc++ -lc++abi" LibCxx)

  if(LibCxx)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "-lc++abi")
    message(STATUS "Using LLVM libc++")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
endif()
