include(CheckCXXCompilerFlag)

# Use Clang with libc++ and colorize output
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  option(USE_LIBCXX "Use LibC++ when compiling with clang" ON)
  # Colored output from Clang
  add_compile_options($<$<CXX_COMPILER_ID:Clang>:-fcolor-diagnostics>)
  check_cxx_compiler_flag("-stdlib=libc++ -lc++abi" LibCxx)

  if(USE_LIBCXX AND LibCxx)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
    add_link_options(-stdlib=libc++ -lc++abi)
  endif()
endif()
