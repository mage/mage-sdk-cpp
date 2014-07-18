BIN_DIRECTORY = ./bin
BUILD_DIRECTORY = ./build

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
	LIBCURL_DYLIB = /usr/lib64/libcurl.so
endif
ifeq ($(UNAME_S),Darwin)
	LIBCURL_DYLIB = /usr/lib/libcurl.dylib
endif

CC = g++
CFLAGS  = -g -Wall -std=c++0x -DHTTP_CONNECTOR

INCLUDES = -I ./src -I ./vendor/libjson-rpc-cpp/src

LFLAGS = -L ./vendor/libjson-rpc-cpp/build/out -L ./build
LIBS = -ljsonrpc -lmage -ldl $(LIBCURL_DYLIB)

LIB_SRCS = $(wildcard ./src/*.cpp)
LIB_OBJS = $(addprefix ./build/,$(notdir $(LIB_SRCS:.cpp=.o)))

BIN_SRCS = $(wildcard ./src/bin/*.cpp)
BIN = $(addprefix ./bin/,$(notdir $(BIN_SRCS:.cpp=)))

EXAMPLES_SRCS = $(wildcard ./examples/*.cpp)
EXAMPLES_BIN = $(EXAMPLES_SRCS:.cpp=)

all: bin

#
# If you want a library to build
# on iOS, Android, etc...
#
platforms: ios ios-unity android

clean-ios:
	rm -rf platforms/ios/build/Release-iphoneos

clean-ios-unity:
	rm -rf platforms/ios/build/UnityRelease-iphoneos

clean-android:
	rm -rf ./platforms/android/obj

ios: platforms/ios/build/Release-iphoneos/libmage-sdk.a

platforms/ios/build/Release-iphoneos/libmage-sdk.a:
	xcodebuild ONLY_ACTIVE_ARCH=NO \
		-project "./platforms/ios/mage-sdk.xcodeproj" \
		-configuration "Release"

ios-unity: platforms/ios/build/UnityRelease-iphoneos/libmage-sdk.a

platforms/ios/build/UnityRelease-iphoneos/libmage-sdk.a:
	xcodebuild ONLY_ACTIVE_ARCH=NO \
		-project "./platforms/ios/mage-sdk.xcodeproj" \
		-configuration "UnityRelease"

android:
	cd platforms/android && $(MAKE)

#
# Build libraries for OS X, Linux
#
lib: vendors dirs build/libmage.a

vendors:
	cd ./vendor/libjson-rpc-cpp/build && cmake .. && make

build/libmage.a: $(LIB_OBJS)
	ar rcs ./build/libmage.a $(LIB_OBJS)

build/%.o: src/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

dirs: dirs-bin dirs-build

dirs-bin:
	mkdir -p $(BIN_DIRECTORY)

dirs-build:
	mkdir -p $(BUILD_DIRECTORY)

bin: lib $(BIN)

bin/%: $(BIN_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) $< -o $@

examples: clean-examples lib $(EXAMPLES_BIN)

clean-examples:
	find ./examples -depth 1 -not -iname "*.cpp" -exec rm -rf {} \; || true

examples/%:
	$(CC) $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) $@.cpp -o $@

clean: clean-bin clean-build clean-vendor

clean-vendor:
	find ./vendor/libjson-rpc-cpp/build/src/*/CMakeFiles -name "*.o" -exec rm {} \;
	rm -rf ./vendor/libjson-rpc-cpp/build/out/*

clean-bin:
	rm -rf $(BIN_DIRECTORY)

clean-build:
	rm -rf $(BUILD_DIRECTORY)
