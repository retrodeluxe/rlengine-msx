include ../build.mk
include $(SETUP_BUILD)
#
# ASCII8 banked ROM test (up to 2048K)
include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

## Specify manually the distribution of files on each page
## -- limit per page is 8K
LOCAL_ROM_NUM_PAGES := 2
LOCAL_PAGE_1_SRC_FILES := swap.c
LOCAL_PAGE_2_SRC_FILES := swap0.c

LOCAL_ROM_NAME := rom256_test
LOCAL_SRC_FILES := rom256_test.c
include $(BUILD_ROM_ASCII8)
