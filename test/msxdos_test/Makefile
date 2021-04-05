include ../build.mk
include $(SETUP_BUILD)
#
# hello msx rom
include $(CONFIG_ROM)

LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)

LOCAL_COM_NAME := hello_msx
LOCAL_SRC_FILES := hello_msx.c
include $(BUILD_MSXDOS_COM)
