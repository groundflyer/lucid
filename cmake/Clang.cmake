include(CheckCXXCompilerFlag)

# Use Clang with libc++ and colorize output
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND USE_LIBCXX)
  check_cxx_compiler_flag("-stdlib=libc++ -lc++abi" LibCxx)

  if(LibCxx)
    add_compile_options(-stdlib=libc++)
    add_link_options(-stdlib=libc++ -lc++abi)
    message(STATUS "Using LLVM libc++")
  endif()
endif()
