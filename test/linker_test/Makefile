include ../build.mk
#
# ASCII8 banked ROM test (up to 2048K)
CONFIG_BUILD_TYPE := debug
CONFIG_BANKED_CALLS := enabled
include $(SETUP_BUILD)

include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NAME := linker_test
LOCAL_SRC_FILES := code0.c code1.c data.c linker_test.c
include $(BUILD_ROM_ASCII8)
