#
# Build an ASCII8 ROM (up to 2048K)
#
include $(BUILD_SYSTEM)/boot.mk
include $(BUILD_SYSTEM)/engine.mk

built_rom_ihx = $(LOCAL_BUILD_OUT_BIN)/$(LOCAL_ROM_NAME).ihx
built_rom_bin = $(LOCAL_BUILD_OUT_BIN)/$(LOCAL_ROM_NAME).bin
built_rom_1Mb = $(LOCAL_BUILD_OUT_ROM)/$(LOCAL_ROM_NAME).rom

all: $(built_rom_1Mb)

# Extract pages from local source
#
CODE_PAGES := 2 # Engine uses CODE_PAGE 2 for banked functions
CODE_PAGES += $(shell grep -R CODE_PAGE $(LOCAL_SRC_FILES) | grep -oE '[^ ]+$$' | sort | uniq | tr '\n' ' ')
DATA_PAGES := $(shell grep -R DATA_PAGE $(LOCAL_SRC_FILES) | grep -oE '[^ ]+$$' | sort | uniq | tr '\n' ' ')

# Build local sources
#
BUILT_LOCAL_SRC_FILES := $(patsubst %.c, $(LOCAL_BUILD_OUT_BIN)/%.rel, $(LOCAL_SRC_FILES))

$(BUILT_LOCAL_SRC_FILES): $(LOCAL_BUILD_OUT_BIN)/%.rel: $(LOCAL_BUILD_SRC)/%.c | resources
	@mkdir -p $(LOCAL_BUILD_OUT_BIN)
	$(call print_cc, local, $^)
	$(hide) $(CROSS_CC) $(ROM_CFLAGS) -c -o $@ $^

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
	@echo "-b _CODE=0x7C00" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	$(hide) $(foreach page,$(CODE_PAGES),$(call emit_link_code_page,$(page)))
	$(hide) $(foreach page,$(DATA_PAGES),$(call emit_link_data_page,$(page)))
	@echo "-b _HOME=0x4120" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-b _DATA=0xC000" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-l z80" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-l rdl_engine" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo $^ | tr ' ' '\n' >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-e" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	$(call print_ld, ihx, $@)
	$(hide) $(CROSS_LD) -n -k $(SDCC_LIB) -k $(BUILD_OUT_BIN) -f $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk

$(built_rom_bin) : $(built_rom_ihx) | $(HEX2ROM)
	$(call print_pack, bin, $@)
	$(hide) cd $(LOCAL_BUILD_OUT_BIN) && $(HEX2ROM) -e bin $(notdir $^)

## Fill in the binary into a ROM of standard size (256Kb = 2Mbit)
##
$(built_rom_1Mb) : $(built_rom_bin) | $(BUILT_ROM_PAGES)
	@mkdir -p $(LOCAL_BUILD_OUT_ROM)
	$(call print_pack, rom2Mb, $@)
	$(hide) tr "\000" "\377" < /dev/zero | (dd ibs=1k count=256 of=$@) > /dev/null 2>&1
	$(hide) (dd if=$^ of=$@ conv=notrunc) > /dev/null 2>&1

run: $(built_rom_1Mb)
	$(hide) $(OPENMSX) -extb debugdevice -machine msx1 -carta $^

run2: $(built_rom_1Mb)
	$(hide) $(OPENMSX) -extb debugdevice -machine msx2 -carta $^

runR: $(built_rom_1Mb)
	$(hide) $(OPENMSX) -extb debugdevice -machine Panasonic_FS-A1GT -carta $^
