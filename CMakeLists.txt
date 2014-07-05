cmake_minimum_required(VERSION 2.8)
project(mage-sdk-cpp)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/libjson-rpc-cpp/src/jsonrpc)

include_directories(${CMAKE_SOURCE_DIR}/vendor/libjson-rpc-cpp/src)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/bin)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_CXX_FLAGS         "-std=c++0x ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS           "-std=c++0x ${CMAKE_C_FLAGS}")

SET(CMAKE_CXX_FLAGS_DEBUG   "-std=c++0x -Wall -g ${CMAKE_CXX_FLAGS_DEBUG}")
SET(CMAKE_C_FLAGS_DEBUG     "-std=c++0x -Wall -g ${CMAKE_C_FLAGS_DEBUG}")

add_custom_command(
  OUTPUT platforms/ios/build/Release-iphoneos/libmage-sdk.a
  COMMAND xcodebuild ONLY_ACTIVE_ARCH=NO -project "./platforms/ios/mage-sdk.xcodeproj" -configuration "Release"
)

add_custom_target(
  ios
  DEPENDS platforms/ios/build/Release-iphoneos/libmage-sdk.a
)

add_custom_target(
  android
  COMMAND cd ${CMAKE_SOURCE_DIR}/platforms/android && \${MAKE}
)

add_custom_target(
  platforms
  DEPENDS android ios
)

add_subdirectory(src)
add_subdirectory(src/bin)
add_subdirectory(examples)

