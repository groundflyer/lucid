include(CTest)

file(GLOB test_srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

add_custom_target(tests)

foreach(src ${test_srcs})
  string(REPLACE ".cpp" "" name ${src})
  string(PREPEND name "test_")

  add_executable(${name} ${src})

  if(${name} STREQUAL "test_viewport")
    target_link_libraries(${name} PRIVATE nanogui ${NANOGUI_EXTRA_LIBS})
  elseif(${name} STREQUAL "test_dispatcher")
    target_link_libraries(${name} PRIVATE fmt-header-only Threads::Threads)
    add_test(NAME ${name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  else()
    add_test(NAME ${name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  endif()

  target_link_libraries(${name} PRIVATE fmt-header-only)
  add_dependencies(tests ${name})
endforeach()
