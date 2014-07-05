file(GLOB_RECURSE examples_source *.c*)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/examples)

foreach(file ${examples_source})
  string(REGEX REPLACE "^.*/([a-zA-Z0-9]*)\\.cpp$" "\\1" executable_name ${file})
  add_executable(${executable_name} EXCLUDE_FROM_ALL ${file})
  target_link_libraries(${executable_name} mage jsonrpc)
endforeach()

