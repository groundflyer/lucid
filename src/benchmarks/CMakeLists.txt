file(GLOB test_srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

foreach(src ${test_srcs})
  string(REPLACE ".cpp" "" name ${src})
  string(APPEND name "_benchmark")

  add_executable(${name} ${src})

  target_link_libraries(${name} PRIVATE fmt-header-only)

  message(STATUS "Found ${name}")
endforeach()
