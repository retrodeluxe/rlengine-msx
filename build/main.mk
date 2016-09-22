#
# Common definitions
export BUILD_SYSTEM := $(TOP)/build
export RLE_TEST  = $(TOP)test
export RLE_TOOLS = $(TOP)tools

# Output tree
export BUILD_OUT = $(TOP)out
export BUILD_OUT_BIN = $(BUILD_OUT)/bin
export BUILD_OUT_TOOLS = $(BUILD_OUT)/tools

# Set default target
.PHONY: rle
DEFAULT_GOAL := rle
$(DEFAULT_GOAL):

export MAKEFLAGS :=

# warning 59: function must return value, in low level asm functions,
#             we directly set hl with return value without using local vars
#  
export ENGINE_CFLAGS  := -mz80 --std-c99 --fno-omit-frame-pointer --disable-warning 59 -I $(TOP)/engine/include
export ENGINE_LDFLAGS := --no-std-crt0 --use-stdout
export ENGINE_ASFLAGS := -plosff

export ARCH := $(shell uname -p)
export SDCC_ROOT := $(TOP)/prebuilts/$(ARCH)/sdcc_3.6.0
export CROSS_CC := $(SDCC_ROOT)/bin/sdcc
export CROSS_AS := $(SDCC_ROOT)/bin/sdasz80
export CROSS_LD := $(SDCC_ROOT)/bin/sdldz80
export SDCC_LIB := $(SDCC_ROOT)/share/sdcc/lib/z80

export HOSTCC	:= gcc-4.9
export TILED2H  := $(RLE_TOOLS)/map2header.py

# Build Commands
#
export CONFIG_ROM := $(BUILD_SYSTEM)/config_rom.mk
export BUILD_ROM := $(BUILD_SYSTEM)/build_rom.mk
export BUILD_ROM_32K := $(BUILD_SYSTEM)/build_32k_rom.mk
export BUILD_RESOURCES := $(BUILD_SYSTEM)/build_resources.mk

# Create output tree
.PHONY: outdirs
outdirs:
	@mkdir -p $(BUILD_OUT)
	@mkdir -p $(BUILD_OUT_BIN)
	@mkdir -p $(BUILD_OUT_TOOLS)
rle: outdirs

# Build tests
#
.PHONY: test
test:
	$(MAKE) -C $(RLE_TEST) all
rle: test

clean:
	rm -Rf $(TOP)/out
	$(MAKE) -C $(RLE_TEST) $(MAKEFLAGS) clean
