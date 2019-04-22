include(CTest)

file(GLOB test_srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

foreach(src ${test_srcs})
  string(REPLACE ".cpp" "" name ${src})

  if(${name} STREQUAL "image")
    add_executable(${name} $<TARGET_OBJECTS:io_image> ${src})
  else()
    add_executable(${name} ${src})
  endif()

  add_test(NAME ${name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${name})
  target_link_libraries(${name} PRIVATE fmt-header-only)
  message(STATUS "Added test: ${name}")
endforeach()
