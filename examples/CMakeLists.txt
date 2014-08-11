file(GLOB_RECURSE examples_source *.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/examples)

set (_allExamples "")

foreach(file ${examples_source})
  string(REGEX REPLACE "^.*/([^/]*)\\.cpp$" "\\1" executable_name ${file})
  add_executable(examples_${executable_name} EXCLUDE_FROM_ALL ${file})
  target_link_libraries(examples_${executable_name} mage jsonrpc)
  list (APPEND _allExamples examples_${executable_name})
endforeach()

add_custom_target(
  examples
  DEPENDS ${_allExamples}
)

