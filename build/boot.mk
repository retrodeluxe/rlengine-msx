# Build bootstraps
#
BOOT_ROOT := $(TOP)/boot
BOOT_SRC_FILES := $(wildcard $(BOOT_ROOT)/*.s)

BUILT_BOOTSTRAP := $(patsubst $(BOOT_ROOT)/%.s,$(BUILD_OUT_BIN)/%.rel,$(BOOT_SRC_FILES))

export BUILT_BOOTSTRAP_32K := $(BUILD_OUT_BIN)/boot32k.rel
export BUILT_BOOTSTRAP_48K := $(BUILD_OUT_BIN)/boot48k.rel
export BUILT_BOOTSTRAP_ASCII8 := $(BUILD_OUT_BIN)/bootASCII8.rel
export BUILT_BOOTSTRAP_MSXDOS := $(BUILD_OUT_BIN)/bootMSXDOS.rel

$(BUILT_BOOTSTRAP): $(BUILD_OUT_BIN)/%.rel: $(BOOT_ROOT)/%.s
	$(hide) mkdir -p $(BUILD_OUT_BIN)
	$(hide) $(CROSS_AS) $(ENGINE_ASFLAGS) $@ $^
