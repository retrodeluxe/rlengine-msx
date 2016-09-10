# Build Engine 
#
ENGINE_ROOT := $(TOP)/engine
ENGINE_SRC_FILES := $(wildcard $(ENGINE_ROOT)/*.c)

export BUILT_ENGINE := $(patsubst $(ENGINE_ROOT)/%.c,$(BUILD_OUT_BIN)/%.rel,$(ENGINE_SRC_FILES))

$(BUILT_ENGINE): $(BUILD_OUT_BIN)/%.rel: $(ENGINE_ROOT)/%.c
	@mkdir -p $(BUILD_OUT_BIN)
	$(CROSS_CC) -c -o $@ $^ $(ENGINE_CFLAGS)