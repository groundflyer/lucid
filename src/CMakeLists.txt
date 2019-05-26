include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(image)

add_executable(${PROJECT_NAME} "lucid.cpp")
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} Threads::Threads ${OPENGL_gl_LIBRARY} glfw fmt-header-only)
target_compile_definitions(${PROJECT_NAME} PUBLIC -DGLFW_INCLUDE_GLCOREARB -DGLFW_INCLUDE_GLEXT -DGL_GLEXT_PROTOTYPES)

if(BUILD_TESTS)
  message(STATUS "Building tests")
  enable_testing()
  add_subdirectory(tests)
endif()
