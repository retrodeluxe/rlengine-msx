include ../build.mk

MACHINE := msx2
include $(SETUP_BUILD)

include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NAME := bitmap_test
LOCAL_SRC_FILES := bitmap_test.c
include $(BUILD_ROM_32K)
