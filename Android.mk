LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ue4dumper
LOCAL_ARM_MODE := arm
LOCAL_ARCH := arm64-v8a

LOCAL_SRC_FILES := \
    src/main.cpp \
    src/memory/process_memory.cpp \
    src/memory/pattern_scanner.cpp \
    src/dumper/dumper.cpp \
    src/utils/logger.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/src

LOCAL_CFLAGS := -Wall -Wextra -O2 -fPIE -fPIC -DANDROID
LOCAL_CPPFLAGS := -std=c++17

LOCAL_LDLIBS := -llog -landroid

include $(BUILD_EXECUTABLE)
