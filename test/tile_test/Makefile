include ../build.mk
include $(SETUP_BUILD)

include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NAME := tile_test
LOCAL_SRC_FILES := tile_test.c
include $(BUILD_ROM_32K)
