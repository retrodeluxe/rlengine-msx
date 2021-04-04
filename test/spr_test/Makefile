include ../build.mk

CONFIG_BUILD_TYPE := release
CONFIG_MACHINE := msx2
include $(SETUP_BUILD)

include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NAME := spr_test
LOCAL_SRC_FILES := spr_test.c
include $(BUILD_ROM_32K)
