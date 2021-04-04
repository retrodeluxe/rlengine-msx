include ../build.mk

# game test is very tight in terms of how code fits on each
# ROM page, enabling debug mode breaks the build currently
# due to the addition of asserts
CONFIG_BUILD_TYPE := release
CONFIG_BANKED_CALLS := enabled
include $(SETUP_BUILD)

include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NAME := game_test
LOCAL_SRC_FILES := main.c logic.c anim.c anim2.c scene.c init.c \
	data1.c data2.c data3.c data4.c data5.c data6.c data7.c data8.c \
	data9.c data10.c data11.c data12.c data13.c data14.c
include $(BUILD_ROM_ASCII8)
