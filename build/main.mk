#
# Common definitions
export TOP := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))/../
export BUILD_SYSTEM := $(TOP)/build
export RLE_TEST  = $(TOP)test
export RLE_ROMS = $(TOP)roms
export RLE_TOOLS = $(TOP)tools

export hide:= @

# Output tree
export BUILD_OUT = $(TOP)out
export BUILD_OUT_BIN = $(BUILD_OUT)/bin
export BUILD_OUT_TOOLS = $(BUILD_OUT)/tools

# Set default target
.PHONY: rle
DEFAULT_GOAL := all
$(DEFAULT_GOAL):

export MAKEFLAGS :=

# warning 59: function must return value, in low level asm functions,
#             we directly set hl with return value without using local vars
# warning 196: pointer target lost const qualifier
#             we need to store ROM data as const, but is is assigned as non const
#
export ENGINE_LDFLAGS := -rc
export ENGINE_ASFLAGS := -plosff

export ARCH := $(shell uname)

ifeq ($(ARCH), Darwin)
export SDCC_ROOT := $(TOP)/prebuilts/darwin/sdcc_3.8.5
else
export SDCC_ROOT := $(TOP)/prebuilts/x86_64/sdcc_3.8.5
endif
export CROSS_CC := $(SDCC_ROOT)/bin/sdcc
export CROSS_AS := $(SDCC_ROOT)/bin/sdasz80
export CROSS_LD := $(SDCC_ROOT)/bin/sdldz80
export CROSS_AR := $(SDCC_ROOT)/bin/sdar
export CROSS_LD := $(SDCC_ROOT)/bin/sdldz80
export SDCC_LIB := $(SDCC_ROOT)/share/lib/z80
export SDCC_INCLUDE := $(SDCC_ROOT)/share/include

export ENGINE_CFLAGS := -mz80
export ROM_CFLAGS := -mz80

ifeq ($(MACHINE),msx2)
	ENGINE_CFLAGS += -DMSX2
endif

ifeq ($(BANKED_CALLS),enabled)
	ENGINE_CFLAGS += --model-large -DBANKED_CALLS
	ROM_CFLAGS += --model-large -DBANKED_CALLS
endif

ifeq ($(BUILD_TYPE),release)
	ENGINE_CFLAGS += -DNDEBUG --max-allocs-per-node 100000
	ROM_CFLAGS += -DNDEBUG --max-allocs-per-node 100000
endif

ENGINE_CFLAGS += --std-c99 --opt-code-speed --fno-omit-frame-pointer --disable-warning 59 --disable-warning 196 -I $(TOP)/engine/include -I $(SDCC_INCLUDE)
ROM_CFLAGS += --std-c99 --opt-code-speed --fno-omit-frame-pointer --disable-warning 59 --disable-warning 196 -I $(TOP)/engine/include -I $(SDCC_INCLUDE)

export HOSTCC	:= gcc

# Build Commands
#
export CONFIG_ROM := $(BUILD_SYSTEM)/config_rom.mk
export BUILD_ROM := $(BUILD_SYSTEM)/build_rom.mk
export BUILD_ROM_32K := $(BUILD_SYSTEM)/build_32k_rom.mk
export BUILD_ROM_48K := $(BUILD_SYSTEM)/build_48k_rom.mk
export BUILD_ROM_ASCII8 := $(BUILD_SYSTEM)/build_ascii8_rom.mk
export BUILD_RESOURCES := $(BUILD_SYSTEM)/build_resources.mk

# Create output tree
.PHONY: outdirs
outdirs:
	@mkdir -p $(BUILD_OUT)
	@mkdir -p $(BUILD_OUT_BIN)
	@mkdir -p $(BUILD_OUT_TOOLS)
all: outdirs

# Build tests
#
.PHONY: test
test:
	$(MAKE) -C $(RLE_TEST) all
rle: #test

# Build ROMS
#
.PHONY: roms
roms:
	$(MAKE) -C $(RLE_ROMS) all
rle: #roms

clean:
	$(hide) rm -Rf $(TOP)/out
	$(hide) $(MAKE) -C $(RLE_TEST) clean_rom
	$(hide) $(MAKE) -C $(RLE_ROMS) clean_rom
