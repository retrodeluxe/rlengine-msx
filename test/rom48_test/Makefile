#
# 48k test rom
include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

# files to be placed in low ram in page0 (up to 16k)
# bios needs to be switched before accessing
# anything in here
LOCAL_BANKED_SRC_FILES := swap.c

LOCAL_ROM_NAME := rom48_test
LOCAL_SRC_FILES := rom48_test.c
include $(BUILD_ROM_48K)
