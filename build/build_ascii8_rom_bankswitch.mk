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

define page-files
BUILT_LOCAL_PAGE_$(1)_SRC_FILES := $$(patsubst %.c, $$(LOCAL_BUILD_OUT_BIN)/%.rel, $$(LOCAL_PAGE_$(1)_SRC_FILES))
BUILT_LOCAL_PAGES_SRC_FILES += $$(BUILT_LOCAL_PAGE_$(1)_SRC_FILES)
BUILT_ROM_PAGE_$(1) := $$(LOCAL_BUILD_OUT_BIN)/$$(LOCAL_ROM_NAME)_$(1).rom
BUILT_ROM_BIN_PAGE_$(1) := $$(LOCAL_BUILD_OUT_BIN)/$$(LOCAL_ROM_NAME)_$(1).bin
BUILT_ROM_IHX_PAGE_$(1) := $$(LOCAL_BUILD_OUT_BIN)/$$(LOCAL_ROM_NAME)_$(1).ihx
BUILT_ROM_PAGES += $$(BUILT_ROM_PAGE_$(1))
endef

define build-ascii8-page
$$(BUILT_LOCAL_PAGE_$(1)_SRC_FILES): $$(LOCAL_BUILD_OUT_BIN)/%.rel: $$(LOCAL_BUILD_SRC)/%.c
	@mkdir -p $$(LOCAL_BUILD_OUT_BIN)
	$$(CROSS_CC) $$(ENGINE_CFLAGS_BANKED) -bo $(1) -c -o $$@ $$^
endef

define build-rom-page
$$(BUILT_ROM_PAGE_$(1)): $$(BUILT_ROM_BIN_PAGE_$(1))
	@mkdir -p $$(LOCAL_BUILD_OUT_ROM)
	tr "\000" "\377" < /dev/zero | dd ibs=1k count=8 of=$$@
	dd if=$$^ of=$$@ conv=notrunc
endef

define build-rom-bin-page
$$(BUILT_ROM_BIN_PAGE_$(1)): $$(BUILT_ROM_IHX_PAGE_$(1)) | $$(HEX2BIN)
	cd $$(LOCAL_BUILD_OUT_BIN) && $$(HEX2BIN) -e bin $$(notdir $$^)
endef

define build-rom-ihx-page
$$(BUILT_ROM_IHX_PAGE_$(1)): $$(BUILT_LOCAL_PAGE_$(1)_SRC_FILES)
	@echo "-mwxuy" > $$(LOCAL_BUILD_OUT_BIN)/tmp.lnk
	@echo "-i $${@}" >> $$(LOCAL_BUILD_OUT_BIN)/tmp.lnk
	@echo "-b _CODE_$(1)=0x$(1)A000" >> $$(LOCAL_BUILD_OUT_BIN)/tmp.lnk
	@echo "-l z80" >> $$(LOCAL_BUILD_OUT_BIN)/tmp.lnk
	@echo $$^ | tr ' ' '\n' >> $$(LOCAL_BUILD_OUT_BIN)/tmp.lnk
	@echo "-e" >> $$(LOCAL_BUILD_OUT_BIN)/tmp.lnk
	$(CROSS_LD) -k $$(SDCC_LIB) -f $$(LOCAL_BUILD_OUT_BIN)/tmp.lnk
endef

ROM_PAGES := $(shell seq 1 $(LOCAL_ROM_NUM_PAGES))

$(foreach page,$(ROM_PAGES),$(eval $(call page-files,$(page))))
$(foreach page,$(ROM_PAGES),$(eval $(call build-ascii8-page,$(page))))
$(foreach page,$(ROM_PAGES),$(eval $(call build-rom-page,$(page))))
$(foreach page,$(ROM_PAGES),$(eval $(call build-rom-bin-page,$(page))))
$(foreach page,$(ROM_PAGES),$(eval $(call build-rom-ihx-page,$(page))))

# Build local sourcess
#
BUILT_LOCAL_SRC_FILES := $(patsubst %.c, $(LOCAL_BUILD_OUT_BIN)/%.rel, $(LOCAL_SRC_FILES))

$(BUILT_LOCAL_SRC_FILES): $(LOCAL_BUILD_OUT_BIN)/%.rel: $(LOCAL_BUILD_SRC)/%.c
	@mkdir -p $(LOCAL_BUILD_OUT_BIN)
	$(CROSS_CC) $(ENGINE_CFLAGS_BANKED) -c -o $@ $^

## Everything needs to be linked together; all mapped pages overlap over 0xA000 - 0xBFFF
##
$(built_rom_ihx) : $(BUILT_LOCAL_SRC_FILES) $(BUILT_BOOTSTRAP_ASCII8) $(BUILT_LOCAL_PAGES_SRC_FILES) | $(BUILT_ENGINE_LIB)
	@echo "-mwxuy" > $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-i ${@}" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-b _BOOT=0x4000" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-b _CODE=0x406C" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	$(foreach page,$(ROM_PAGES),echo "-b _CODE_${page}=0x${page}A000" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk;)
	@echo "-b _HOME=0xB000" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-b _DATA=0xC000" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-l z80" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo $^ | tr ' ' '\n' >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	@echo "-e" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk
	$(CROSS_LD) -k $(SDCC_LIB) -k $(BUILD_OUT_BIN) -f $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk

#@echo "-l rdl_engine" >> $(LOCAL_BUILD_OUT_BIN)/rom_ascii8.lnk

$(built_rom_bin) : $(built_rom_ihx) | $(HEX2BIN)
	cd $(LOCAL_BUILD_OUT_BIN) && $(HEX2BIN) -e bin $(notdir $^)

# Generate the actual ROM by aseembling the pieces.
#
$(built_rom_1Mb) : $(built_rom_bin) | $(BUILT_ROM_PAGES)
	@mkdir -p $(LOCAL_BUILD_OUT_ROM)
	tr "\000" "\377" < /dev/zero | dd ibs=1k count=128 of=$@
	dd if=$^ of=$@ conv=notrunc
	$(foreach page,$(ROM_PAGES),dd if=${BUILT_ROM_PAGE_$(page)} of=$@ seek=$(shell expr $(page) + 3) bs=8192 conv=notrunc,sync;)
