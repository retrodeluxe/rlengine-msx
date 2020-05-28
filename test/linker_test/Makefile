#
# ASCII8 banked ROM test (up to 2048K)
include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)


# I think this can be detected with a tool
LOCAL_ROM_CODE_START_PAGE := 2
LOCAL_ROM_CODE_END_PAGE := 3
LOCAL_ROM_DATA_START_PAGE := 4
LOCAL_ROM_DATA_END_PAGE := 4

LOCAL_ROM_NAME := linker_test
LOCAL_SRC_FILES := code0.c code1.c data.c linker_test.c
include $(BUILD_ROM_ASCII8_BANKSWITCH)
