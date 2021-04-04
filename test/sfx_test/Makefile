include ../build.mk

CONFIG_BUILD_TYPE := debug
include $(SETUP_BUILD)
#
# misc test rom
include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NAME := sfx_test
LOCAL_SRC_FILES := sfx_test.c
include $(BUILD_ROM_32K)
