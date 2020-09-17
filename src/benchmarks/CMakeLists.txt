file(GLOB test_srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

foreach(src ${test_srcs})
  string(REPLACE ".cpp" "" name ${src})
  string(APPEND name "_benchmark")

  add_executable(${name} ${src})

  target_link_libraries(${name} PRIVATE fmt-header-only)
  target_compile_options(${name} PRIVATE "-save-temps=cwd" "-Rpass=vector*")

  add_test(NAME ${name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${name})
  message(STATUS "Found ${name}")
endforeach()
