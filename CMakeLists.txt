cmake_minimum_required(VERSION 2.8)
project(mage-sdk-cpp)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

find_package(Readline REQUIRED)
include_directories(${Readline_INCLUDE_DIR})

SET(HTTP_CONNECTOR YES CACHE BOOL "Include HTTP connector inside library")
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/libjson-rpc-cpp/src/jsonrpc)

if (HTTP_CONNECTOR)
  ADD_DEFINITIONS(-DHTTP_CONNECTOR)
endif()

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
  COMMAND xcodebuild ONLY_ACTIVE_ARCH=NO -project "${CMAKE_SOURCE_DIR}/platforms/ios/mage-sdk.xcodeproj" -configuration "Release"
)

add_custom_target(
  ios
  DEPENDS platforms/ios/build/Release-iphoneos/libmage-sdk.a
)

add_custom_target(
  clean-ios
  COMMAND rm -rf ${CMAKE_SOURCE_DIR}/platforms/ios/build/Release-iphoneos
)

add_custom_command(
  OUTPUT platforms/ios/build/UnityRelease-iphoneos/libmage-sdk.a
  COMMAND xcodebuild ONLY_ACTIVE_ARCH=NO -project "${CMAKE_SOURCE_DIR}/platforms/ios/mage-sdk.xcodeproj" -configuration "UnityRelease"
)

add_custom_target(
  ios-unity
  DEPENDS platforms/ios/build/UnityRelease-iphoneos/libmage-sdk.a
)

add_custom_target(
  clean-ios-unity
  COMMAND rm -rf ${CMAKE_SOURCE_DIR}/platforms/ios/build/UnityRelease-iphoneos
)

add_custom_target(
  android
  COMMAND cd ${CMAKE_SOURCE_DIR}/platforms/android && \${MAKE}
)

add_custom_target(
  clean-android
  COMMAND rm -rf ${CMAKE_SOURCE_DIR}/platforms/android/obj 
)

add_custom_target(
  android-unity
  COMMAND cd ${CMAKE_SOURCE_DIR}/platforms/android-unity && \${MAKE}
)

add_custom_target(
  clean-android-unity
  COMMAND rm -rf ${CMAKE_SOURCE_DIR}/platforms/android-unity/libs
)

add_custom_target(
  platforms
  DEPENDS android android-unity ios ios-unity
)

add_custom_target(
  lint
  COMMAND python ../tools/cpplint.py --filter=-whitespace/tab,-legal,-build,-readability/streams,-runtime/explicit,-whitespace/indent --linelength=120 src/*.{h,cpp} ../src/bin/*.cpp ../examples/*.cpp
)

add_subdirectory(src)
add_subdirectory(src/bin)
add_subdirectory(examples)

