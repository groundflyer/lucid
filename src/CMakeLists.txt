include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(lucid "lucid.cpp")
target_include_directories(lucid PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(lucid PUBLIC Threads::Threads ${GL_LIBS} fmt-header-only)
target_compile_definitions(lucid PUBLIC ${GL_DEFS})

if(BUILD_TESTS)
  message(STATUS "Building tests")
  enable_testing()
  add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
  message(STATUS "Building benchmarks")
  add_subdirectory(benchmarks)
endif()
