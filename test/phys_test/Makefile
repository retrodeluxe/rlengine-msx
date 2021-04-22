include ../build.mk

CONFIG_BUILD_TYPE := debug
CONFIG_BANKED_CALLS := enabled
include $(SETUP_BUILD)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NAME := phys_test
LOCAL_SRC_FILES := phys_test.c data.c
include $(BUILD_ROM_ASCII8)
