#
# ASCII8 banked ROM test (up to 2048K)
include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

## Specify manually the distribution of files on each page
## -- limit per page is 8K
LOCAL_ROM_NUM_PAGES := 2
LOCAL_PAGE_1_SRC_FILES := code0.c
LOCAL_PAGE_2_SRC_FILES := code1.c

LOCAL_ROM_NAME := linker_test
LOCAL_SRC_FILES := linker_test.c
include $(BUILD_ROM_ASCII8_BANKSWITCH)
