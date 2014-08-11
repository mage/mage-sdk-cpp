file(GLOB_RECURSE mage_source *.c*)
file(GLOB mage_header *.h)

add_library(mage SHARED ${mage_source})
add_library(mageStatic STATIC ${mage_source})

set_target_properties(mageStatic PROPERTIES OUTPUT_NAME mage)

target_link_libraries(mage jsonrpc ${CURL_LIBRARIES} ${Readline_LIBRARY})
target_link_libraries(mageStatic jsonrpc ${CURL_LIBRARIES} ${Readline_LIBRARY})

