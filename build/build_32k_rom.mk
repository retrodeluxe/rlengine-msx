#
# Build a 32K ROM
#
include $(BUILD_SYSTEM)/boot.mk
include $(BUILD_SYSTEM)/engine.mk
include $(BUILD_SYSTEM)/tools.mk

built_rom_ihx = $(LOCAL_BUILD_OUT_BIN)/$(LOCAL_ROM_NAME).ihx
built_rom_bin = $(LOCAL_BUILD_OUT_BIN)/$(LOCAL_ROM_NAME).bin
built_rom_32k = $(LOCAL_BUILD_OUT_ROM)/$(LOCAL_ROM_NAME).rom

all: $(built_rom_32k)

# Build local sourcess
#
BUILT_LOCAL_SRC_FILES := $(patsubst %.c, $(LOCAL_BUILD_OUT_BIN)/%.rel, $(LOCAL_SRC_FILES))

$(BUILT_LOCAL_SRC_FILES): $(LOCAL_BUILD_OUT_BIN)/%.rel: $(LOCAL_BUILD_SRC)/%.c
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_BIN)
	$(hide) $(CROSS_CC) $(ENGINE_CFLAGS) -c -o $@ $^

# Link with Engine and 32k bootstrap
#
$(built_rom_ihx) : $(BUILT_LOCAL_SRC_FILES) $(BUILT_ENGINE) $(BUILT_BOOTSTRAP_32K)
	@echo "-mwxuy" > $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-i ${@}" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-b _BOOT=0x4000" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-b _CODE=0x4042" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-b _HOME=0xB000" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-b _DATA=0xC000" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-l z80" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo $^ | tr ' ' '\n' >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	@echo "-e" >> $(LOCAL_BUILD_OUT_BIN)/rom32.lnk
	$(hide) $(CROSS_LD) -k $(SDCC_LIB) -f $(LOCAL_BUILD_OUT_BIN)/rom32.lnk

$(built_rom_bin) : $(built_rom_ihx) | $(HEX2BIN)
	$(hide) cd $(LOCAL_BUILD_OUT_BIN) && $(HEX2BIN) -e bin $(notdir $^)

# Generate the actual ROM
#
$(built_rom_32k) : $(built_rom_bin)
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_ROM)
	$(hide) tr "\000" "\377" < /dev/zero | dd ibs=1k count=32 of=$@
	$(hide) dd if=$^ of=$@ conv=notrunc
