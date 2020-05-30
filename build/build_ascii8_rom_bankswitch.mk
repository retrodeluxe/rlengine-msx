#
# Build an ASCII8 ROM (up to 2048K)
#
include $(BUILD_SYSTEM)/tools.mk
include $(BUILD_SYSTEM)/boot.mk
include $(BUILD_SYSTEM)/engine.mk

built_rom_ihx = $(LOCAL_BUILD_OUT_BIN)/$(LOCAL_ROM_NAME).ihx
built_rom_bin = $(LOCAL_BUILD_OUT_BIN)/$(LOCAL_ROM_NAME).bin
built_rom_1Mb = $(LOCAL_BUILD_OUT_ROM)/$(LOCAL_ROM_NAME).rom

all: $(built_rom_1Mb)

CODE_PAGES := $(shell seq $(LOCAL_ROM_CODE_START_PAGE) $(LOCAL_ROM_CODE_END_PAGE))
DATA_PAGES := $(shell seq $(LOCAL_ROM_DATA_START_PAGE) $(LOCAL_ROM_DATA_END_PAGE))

# Build local sources
#
BUILT_LOCAL_SRC_FILES := $(patsubst %.c, $(LOCAL_BUILD_OUT_BIN)/%.rel, $(LOCAL_SRC_FILES))

$(BUILT_LOCAL_SRC_FILES): $(LOCAL_BUILD_OUT_BIN)/%.rel: $(LOCAL_BUILD_SRC)/%.c
	@mkdir -p $(LOCAL_BUILD_OUT_BIN)
	$(CROSS_CC) $(ENGINE_CFLAGS_BANKED) -c -o $@ $^

## Build IHX on a 24bit address space containing all code and data
##
### should not be any code in _CODE, it should be either banked or non-banked

define emit_link_code_page
echo "-b _CODE_PAGE_${1}=0x$(shell printf '%x' $1)8000" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk;
endef

define emit_link_data_page
echo "-b _DATA_PAGE_${1}=0x$(shell printf '%x' $1)A000" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk;
endef

$(built_rom_ihx) : $(BUILT_LOCAL_SRC_FILES) $(BUILT_BOOTSTRAP_ASCII8) $(BUILT_LOCAL_PAGES_SRC_FILES) | $(BUILT_ENGINE_LIB)
	@echo "-mwxuy" > $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-i ${@}" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-b _BOOT=0x4000" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-b _CODE=0x7A00" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	$(foreach page,$(CODE_PAGES),$(call emit_link_code_page,$(page)))
	$(foreach page,$(DATA_PAGES),$(call emit_link_data_page,$(page)))
	@echo "-b _HOME=0x40B5" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-b _DATA=0xC000" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-l z80" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-l rdl_engine" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo $^ | tr ' ' '\n' >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-e" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	$(CROSS_LD) -k $(SDCC_LIB) -k $(BUILD_OUT_BIN) -f $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk

$(built_rom_bin) : $(built_rom_ihx) | $(HEX2ROM)
	cd $(LOCAL_BUILD_OUT_BIN) && $(HEX2ROM) -e bin $(notdir $^)

## Fill in the binary into a ROM of standard size (128Kb = 1Mbit)
##
$(built_rom_1Mb) : $(built_rom_bin) | $(BUILT_ROM_PAGES)
	@mkdir -p $(LOCAL_BUILD_OUT_ROM)
	tr "\000" "\377" < /dev/zero | dd ibs=1k count=128 of=$@
	dd if=$^ of=$@ conv=notrunc
