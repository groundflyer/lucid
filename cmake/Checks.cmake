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

# Use Clang with libc++ and colorize output
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND USE_LIBCXX)
  check_cxx_compiler_flag("-stdlib=libc++ -lc++abi" LibCxx)

  if(LibCxx)
    add_compile_options(-stdlib=libc++)
    set(CMAKE_EXE_LINKER_FLAGS "-lc++abi")
    message(STATUS "Using LLVM libc++")
  endif()
endif()
