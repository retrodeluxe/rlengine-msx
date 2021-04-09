#
# Build an MSXDOS COM executable
#
include $(BUILD_SYSTEM)/boot.mk
include $(BUILD_SYSTEM)/engine.mk

built_com_ihx = $(LOCAL_BUILD_OUT_BIN)/$(LOCAL_COM_NAME).ihx
built_com_bin = $(LOCAL_BUILD_OUT_BIN)/$(LOCAL_COM_NAME).bin
built_com_exe = $(LOCAL_BUILD_OUT_COM)/$(LOCAL_COM_NAME).com

ENGINE_CFLAGS += -DMSXDOS

all: $(built_com_exe)

# Build local sourcess
#
BUILT_LOCAL_SRC_FILES := $(patsubst %.c, $(LOCAL_BUILD_OUT_BIN)/%.rel, $(LOCAL_SRC_FILES))

$(BUILT_LOCAL_SRC_FILES): $(LOCAL_BUILD_OUT_BIN)/%.rel: $(LOCAL_BUILD_SRC)/%.c
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_BIN)
	$(call print_cc, local, $^)
	$(hide) $(CROSS_CC) $(COM_CFLAGS) -c -o $@ $^

# Link with Engine and MSXDOS bootstrap
#
$(built_com_ihx) : $(BUILT_LOCAL_SRC_FILES) $(BUILT_BOOTSTRAP_MSXDOS) | $(BUILT_ENGINE_LIB)
	@echo "-mwxuy" > $(LOCAL_BUILD_OUT_BIN)/com.lnk
	@echo "-i ${@}" >> $(LOCAL_BUILD_OUT_BIN)/com.lnk
	@echo "-b _CODE=0x0178" >> $(LOCAL_BUILD_OUT_BIN)/com.lnk
	@echo "-b _CODE_PAGE_2=0x4000" >> $(LOCAL_BUILD_OUT_BIN)/com.lnk
	@echo "-b _BOOT=0x0100" >> $(LOCAL_BUILD_OUT_BIN)/com.lnk
	@echo "-b _DATA=0xC000" >> $(LOCAL_BUILD_OUT_BIN)/com.lnk
	@echo "-l z80" >> $(LOCAL_BUILD_OUT_BIN)/com.lnk
	@echo "-l rdl_engine" >> $(LOCAL_BUILD_OUT_BIN)/com.lnk
	@echo $^ | tr ' ' '\n' >> $(LOCAL_BUILD_OUT_BIN)/com.lnk
	@echo "-e" >> $(LOCAL_BUILD_OUT_BIN)/com.lnk
	$(call print_ld, ihx, $@)
	$(hide) $(CROSS_LD) -n -k $(SDCC_LIB) -k $(BUILD_OUT_BIN)  -f $(LOCAL_BUILD_OUT_BIN)/com.lnk

$(built_com_bin) : $(built_com_ihx) | $(HEX2BIN)
	$(call print_pack, bin, $@)
	$(hide) cd $(LOCAL_BUILD_OUT_BIN) && $(HEX2BIN) -e bin $(notdir $^)

# Generate the actual ROM
#
$(built_com_exe) : $(built_com_bin)
	$(hide) mkdir -p $(LOCAL_BUILD_OUT_COM)
	$(call print_pack, com_exe, $@)
	$(hide) mv $^ $@
