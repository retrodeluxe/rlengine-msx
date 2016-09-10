# Build bootstraps
#
BOOT_ROOT := $(TOP)/boot
BOOT_SRC_FILES := $(wildcard $(BOOT_ROOT)/*.s)

BUILT_BOOTSTRAP := $(patsubst $(BOOT_ROOT)/%.s,$(BUILD_OUT_BIN)/%.rel,$(BOOT_SRC_FILES))

export BUILT_BOOTSTRAP_32K := $(BUILD_OUT_BIN)/boot32k.rel

$(BUILT_BOOTSTRAP): $(BUILD_OUT_BIN)/%.rel: $(BOOT_ROOT)/%.s
	@mkdir -p $(BUILD_OUT_BIN)
	$(CROSS_AS) $(ENGINE_ASFLAGS) $@ $^