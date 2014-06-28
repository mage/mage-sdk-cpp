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
CFLAGS  = -g -Wall -std=c++0x

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

platorms: android

android:
	cd platforms/android && $(MAKE)

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

examples: lib $(EXAMPLES_BIN)

examples/%: $(EXAMPLES_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) $< -o $@

clean: clean-bin clean-build clean-vendor

clean-vendor:
	find ./vendor/libjson-rpc-cpp/build/src/*/CMakeFiles -name "*.o" -exec rm {} \;
	rm -rf ./vendor/libjson-rpc-cpp/build/out/*

clean-bin:
	rm -rf $(BIN_DIRECTORY)

clean-build:
	rm -rf $(BUILD_DIRECTORY)
