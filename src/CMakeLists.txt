include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} "lucid.cpp")
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads ${GL_LIBS} fmt-header-only)
target_compile_definitions(${PROJECT_NAME} PUBLIC ${GL_DEFS})

if(BUILD_TESTS)
  message(STATUS "Building tests")
  enable_testing()
  add_subdirectory(tests)
endif()
