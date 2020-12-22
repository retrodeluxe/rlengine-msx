# Rules tools
#
TOOLS_ROOT := $(TOP)/tools

export HEX2BIN := $(BUILD_OUT_TOOLS)/hex2bin
export TGA2H := $(BUILD_OUT_TOOLS)/tga2header
export PNG2H := $(BUILD_OUT_TOOLS)/png2header
export TMU2H := $(BUILD_OUT_TOOLS)/tmu2header
export XXD   := $(BUILD_OUT_TOOLS)/xxd
export HEX2ROM := $(BUILD_OUT_TOOLS)/hex2rom
export GRAPHX := $(BUILD_OUT_TOOLS)/graphx

export BUILT_TOOLS := $(HEX2BIN) $(TGA2H) $(TMU2H) $(XXD) $(GRAPHX) $(PNG2H)

all: $(TGA2H) $(HEX2BIN) $(TMU2H) $(XXD) $(PNG2H)

$(TGA2H): $(TOOLS_ROOT)/tga2header.c
	$(call print_host_cc, tools, $@)
	$(hide) $(HOSTCC) $^ -o $@

$(PNG2H): $(TOOLS_ROOT)/png2header.c
	$(call print_host_cc, tools, $@)
	$(hide) $(HOSTCC) $^ -lpng -o $@

$(HEX2BIN): $(TOOLS_ROOT)/hex2bin.c
	$(call print_host_cc, tools, $@)
	$(hide) $(HOSTCC) $^ -o $@

$(TMU2H): $(TOOLS_ROOT)/tmu2header.c
	$(call print_host_cc, tools, $@)
	$(hide) $(HOSTCC) $^ -o $@

$(XXD): $(TOOLS_ROOT)/xxd.c
	$(call print_host_cc, tools, $@)
	$(hide) $(HOSTCC) $^ -o $@

$(HEX2ROM): $(TOOLS_ROOT)/hex2rom.c
	$(call print_host_cc, tools, $@)
	$(hide) $(HOSTCC) $^ -o $@

$(GRAPHX): $(TOOLS_ROOT)/graphx.c
	$(call print_host_cc, tools, $@)
	$(hide) $(HOSTCC) $^ -o $@ -lm -lSDL2
