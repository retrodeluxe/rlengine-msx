include ../build.mk
include $(SETUP_BUILD)

include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NAME := map_test
LOCAL_SRC_FILES := map_test.c
include $(BUILD_ROM_32K)
