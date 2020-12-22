# Build Engine
#
ENGINE_ROOT := $(TOP)/engine
ENGINE_SRC_FILES := $(wildcard $(ENGINE_ROOT)/*.c)

export BUILT_ENGINE := $(patsubst $(ENGINE_ROOT)/%.c,$(BUILD_OUT_BIN)/%.rel,$(ENGINE_SRC_FILES))
export BUILT_ENGINE_LIB := $(BUILD_OUT_BIN)/rdl_engine.lib

$(BUILT_ENGINE): $(BUILD_OUT_BIN)/%.rel: $(ENGINE_ROOT)/%.c
	$(hide) mkdir -p $(BUILD_OUT_BIN)
	$(hide) $(CROSS_CC) -c -o $@ $^ $(ENGINE_CFLAGS)

$(BUILT_ENGINE_LIB): $(BUILT_ENGINE)
	$(hide) mkdir -p $(BUILD_OUT_BIN)
	$(hide) $(CROSS_AR) $(ENGINE_LDFLAGS) $@ $^
