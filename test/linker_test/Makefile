include ../build.mk
include $(SETUP_BUILD)
#
# ASCII8 banked ROM test (up to 2048K)
BUILD_TYPE := debug
include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NAME := linker_test
LOCAL_SRC_FILES := code0.c code1.c data.c linker_test.c
include $(BUILD_ROM_ASCII8_BANKSWITCH)
