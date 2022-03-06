include ../build.mk

CONFIG_BUILD_TYPE := debug
CONFIG_MACHINE := msx2
CONFIG_BANKED_CALLS := enabled

include $(SETUP_BUILD)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_ROM_NAME := celeste
LOCAL_SRC_FILES := celeste.c init.c anim.c scene.c gfx.c map.c data2.c data3.c \
	pico8sfx.c

include $(BUILD_ROM_ASCII8)
