include ../build.mk

CONFIG_BUILD_TYPE := debug
CONFIG_MACHINE := msx2
#CONFIG_BANKED_CALLS := enabled

include $(SETUP_BUILD)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_BANK0_SRC_FILES := gfx.c map.c
LOCAL_BANK3_SRC_FILES := data2.c

LOCAL_ROM_NAME := celeste
LOCAL_SRC_FILES := celeste.c init.c anim.c scene.c
include $(BUILD_ROM_64K)
