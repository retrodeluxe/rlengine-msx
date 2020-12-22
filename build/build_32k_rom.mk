#
# Build a 32K ROM
#
include $(BUILD_SYSTEM)/boot.mk
include $(BUILD_SYSTEM)/engine.mk

built_rom_ihx = $(LOCAL_BUILD_OUT_BIN)/$(LOCAL_ROM_NAME).ihx
built_rom_bin = $(LOCAL_BUILD_OUT_BIN)/$(LOCAL_ROM_NAME).bin
built_rom_32k = $(LOCAL_BUILD_OUT_ROM)/$(LOCAL_ROM_NAME).rom

all: $(built_rom_32k)

# Build local sourcess
#
BUILT_LOCAL_SRC_FILES := $(patsubst %.c, $(LOCAL_BUILD_OUT_BIN)/%.rel, $(LOCAL_SRC_FILES))

$(BUILT_LOCAL_SRC_FILES): $(LOCAL_BUILD_OUT_BIN)/%.rel: $(LOCAL_BUILD_SRC)/%.c
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_BIN)
	$(call print_cc, rom, $^)
	$(hide) $(CROSS_CC) $(ENGINE_CFLAGS) -c -o $@ $^

# Link with Engine and 32k bootstrap
#
$(built_rom_ihx) : $(BUILT_LOCAL_SRC_FILES) $(BUILT_BOOTSTRAP_32K) | $(BUILT_ENGINE_LIB)
	@echo "-mwxuy" > $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-i ${@}" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-b _BOOT=0x4000" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-b _CODE=0x4042" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-b _HOME=0x6000" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-b _DATA=0xC000" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-b _CODE_PAGE_2=0x8000" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-l z80" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-l rdl_engine" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo $^ | tr ' ' '\n' >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-e" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	$(call print_ld, ihx, $@)
	$(hide) $(CROSS_LD) -n -k $(SDCC_LIB) -k $(BUILD_OUT_BIN)  -f $(LOCAL_BUILD_OUT_BIN)/rom32.lnk

$(built_rom_bin) : $(built_rom_ihx) | $(HEX2BIN)
	$(call print_pack, bin, $@)
	$(hide) cd $(LOCAL_BUILD_OUT_BIN) && $(HEX2BIN) -e bin $(notdir $^)

# Generate the actual ROM
#
$(built_rom_32k) : $(built_rom_bin)
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_ROM)
	$(call print_pack, rom32k, $@)
	$(hide) tr "\000" "\377" < /dev/zero | (dd ibs=1k count=32 of=$@ ) > /dev/null 2>&1
	$(hide) (dd if=$^ of=$@ conv=notrunc status=noxfer) > /dev/null 2>&1
