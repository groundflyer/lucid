include(CTest)

file(GLOB test_srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

foreach(src ${test_srcs})
  string(REPLACE ".cpp" "" name ${src})

  add_executable(${name} ${src})

  if(${name} STREQUAL "viewport")
    target_include_directories(${name} PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(${name} PUBLIC ${GL_LIBS})
    target_compile_definitions(${name} PUBLIC ${GL_DEFS})
  elseif(${name} STREQUAL "dispatcher")
    target_link_libraries(${name} PRIVATE fmt-header-only Threads::Threads)
  else()
    target_link_libraries(${name} PRIVATE fmt-header-only)
  endif()

  add_test(NAME ${name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${name})
  message(STATUS "Added test: ${name}")
endforeach()
