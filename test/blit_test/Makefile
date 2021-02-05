include ../build.mk

MACHINE := msx2
BANKED_CALLS := enabled
BUILD_TYPE := debug
include $(SETUP_BUILD)

include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NAME := blit_test
LOCAL_SRC_FILES := blit_test.c data1.c
include $(BUILD_ROM_ASCII8)
