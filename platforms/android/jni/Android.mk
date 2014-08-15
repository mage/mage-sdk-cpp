LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := mage-sdk
LOCAL_MODULE_FILENAME := libmage

MAGE_SRC_DIR := $(LOCAL_PATH)/../../../src
LIBJSONRPC_SRC_DIR := $(LOCAL_PATH)/../../../vendor/libjson-rpc-cpp/src

LOCAL_SRC_FILES := $(MAGE_SRC_DIR)/exceptions.cpp \
				   $(MAGE_SRC_DIR)/rpc.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/client.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/clientconnector.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/connectors/mongoose.c \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/connectors/httpclient.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/connectors/httpserver.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/errors.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/exception.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/json/json_reader.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/json/json_value.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/json/json_writer.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/procedure.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/rpcprotocolclient.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/rpcprotocolserver.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/server.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/serverconnector.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/specificationparser.cpp \
				   $(LIBJSONRPC_SRC_DIR)/jsonrpc/specificationwriter.cpp

LOCAL_C_INCLUDES := $(MAGE_SRC_DIR)
LOCAL_C_INCLUDES += $(LIBJSONRPC_SRC_DIR)
LOCAL_C_INCLUDES += $(LIBJSONRPC_SRC_DIR)/jsonrpc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../externals/curl/include/android

LOCAL_LDFLAGS := $(LOCAL_PATH)/../../externals/curl/prebuilt/android/armeabi/libcurl.a
LOCAL_LDLIBS  := -lz -ldl

LOCAL_WHOLE_STATIC_LIBRARIES += curl_static
LOCAL_STATIC_LIBRARIES := curl_static_prebuilt

LOCAL_CPPFLAGS := -fexceptions -std=c++11 -DHTTP_CONNECTOR

include $(BUILD_STATIC_LIBRARY)
