include ../build.mk

CONFIG_MACHINE := msx2
CONFIG_BANKED_CALLS := enabled
CONFIG_BUILD_TYPE := debug

include $(SETUP_BUILD)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NAME := blit_test
LOCAL_SRC_FILES := blit_test.c data1.c
include $(BUILD_ROM_ASCII8)
