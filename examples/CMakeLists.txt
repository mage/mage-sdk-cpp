file(GLOB_RECURSE examples_source *.c*)

foreach(file ${examples_source})
  string(REGEX REPLACE "^.*/([a-zA-Z0-9]*)\\.cpp$" "\\1" executable_name ${file})
  add_executable(${executable_name} ${file})
  target_link_libraries(${executable_name} mage jsonrpc)
endforeach()

