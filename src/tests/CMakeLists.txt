include(CTest)

file(GLOB test_srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

foreach(src ${test_srcs})
  string(REPLACE ".cpp" "" name ${src})
  string(PREPEND name "test_")

  add_executable(${name} ${src})

  if(${name} STREQUAL "test_viewport")
    target_include_directories(${name} PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(${name} PUBLIC ${GL_LIBS})
    target_compile_definitions(${name} PUBLIC ${GL_DEFS})
  elseif(${name} STREQUAL "test_dispatcher")
    target_link_libraries(${name} PRIVATE fmt-header-only Threads::Threads)
    add_test(NAME ${name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  else()
    add_test(NAME ${name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  endif()

  target_link_libraries(${name} PRIVATE fmt-header-only)

endforeach()
