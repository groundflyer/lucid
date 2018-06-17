include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(io)

if(BUILD_TESTS)
  message(STATUS "Building tests")
  enable_testing()
  add_subdirectory(tests)
endif()
