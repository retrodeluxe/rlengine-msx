include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_CODE_START_PAGE := 2
LOCAL_ROM_CODE_END_PAGE := 5
LOCAL_ROM_DATA_START_PAGE := 6
LOCAL_ROM_DATA_END_PAGE := 12

LOCAL_ROM_NAME := game_test
LOCAL_SRC_FILES := main.c logic.c anim.c scene.c init.c swap1.c swap2.c swap3.c swap4.c swap5.c swap6.c swap7.c
include $(BUILD_ROM_ASCII8_BANKSWITCH)
