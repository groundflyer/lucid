include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(image)

add_executable(${PROJECT_NAME} $<TARGET_OBJECTS:io_image> "lucid.cpp")

if(BUILD_TESTS)
  message(STATUS "Building tests")
  enable_testing()
  add_subdirectory(tests)
endif()
