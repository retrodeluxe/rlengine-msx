include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NUM_PAGES := 6
LOCAL_PAGE_1_SRC_FILES := swap1.c
LOCAL_PAGE_2_SRC_FILES := swap2.c
LOCAL_PAGE_3_SRC_FILES := swap3.c
LOCAL_PAGE_4_SRC_FILES := swap4.c
LOCAL_PAGE_5_SRC_FILES := swap5.c
LOCAL_PAGE_6_SRC_FILES := swap6.c

LOCAL_ROM_NAME := game_test
LOCAL_SRC_FILES := main.c logic.c anim.c scene.c
include $(BUILD_ROM_ASCII8)
