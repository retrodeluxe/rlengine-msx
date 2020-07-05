include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_CODE_START_PAGE := 2
LOCAL_ROM_CODE_END_PAGE := 7
LOCAL_ROM_DATA_START_PAGE := 8
LOCAL_ROM_DATA_END_PAGE := 20

LOCAL_ROM_NAME := game_test
LOCAL_SRC_FILES := main.c logic.c anim.c anim2.c scene.c init.c \
	data1.c data2.c data3.c data4.c data5.c data6.c data7.c data8.c \
	data9.c data10.c data11.c data12.c data13.c
include $(BUILD_ROM_ASCII8_BANKSWITCH)
