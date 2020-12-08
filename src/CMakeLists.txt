include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(lucid "lucid.cpp")
target_link_libraries(lucid PUBLIC Threads::Threads fmt-header-only nanogui ${NANOGUI_EXTRA_LIBS})

if(BUILD_TESTS)
  message(STATUS "Building tests")
  enable_testing()
  add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
  message(STATUS "Building benchmarks")
  add_subdirectory(benchmarks)
endif()
