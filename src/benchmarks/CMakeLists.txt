find_package(Git)
if(Git_FOUND)
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE _REPO_HASH)
  string(REPLACE "\n" "" REPO_HASH ${_REPO_HASH})
  execute_process(COMMAND ${GIT_EXECUTABLE} diff-index --quiet HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE REPO_HAS_CHANGES)
  if(REPO_HAS_CHANGES)
    string(APPEND REPO_HASH "*")
  endif()
endif()

file(GLOB test_srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

add_custom_target(benchmarks)

foreach(src ${test_srcs})
  string(REPLACE ".cpp" "" name ${src})
  string(PREPEND name "bm_")

  add_executable(${name} ${src})

  target_link_libraries(${name} PRIVATE fmt-header-only)

  if(${name} MATCHES "bm_mp_.+")
    target_link_libraries(${name} PRIVATE Threads::Threads)
  endif()
  target_compile_definitions(${name} PRIVATE -DREPO_HASH="${REPO_HASH}")
  add_dependencies(benchmarks ${name})
endforeach()
